/*
 * @brief Abstract Base class for Sound Effects
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
/*
 * modified by W. Voigt for Stereo and Limiter
 */

#pragma once
#include "AudioParameters.h"
#include "PitchShift.h"
#include "AudioLogger.h"
#include "AudioTools/CoreAudio/AudioTypes.h"
#include "AudioTools/CoreAudio/AudioOutput.h"
#include <stdint.h>

namespace audio_tools {

// we use int16_t for our effects
typedef int16_t effect_t;

class AudioEffect {
public:
  AudioEffect() = default;
  virtual ~AudioEffect() = default;

  /// calculates the effect output from the input
  virtual effect_t process(effect_t in) = 0;

  /// sets the effect active/inactive
  virtual void setActive(bool value) { active_flag = value; }

  /// determines if the effect is active
  virtual bool active() { return active_flag; }

  virtual AudioEffect *clone() = 0;

  /// Allows to identify an effect
  int id() { return id_value; }

  /// Allows to identify an effect
  void setId(int id) { this->id_value = id; }

protected:
  bool active_flag = true;
  int id_value = -1;

  void copyParent(AudioEffect *copy) {
    id_value = copy->id_value;
    active_flag = copy->active_flag;
  }

  /// generic clipping method
  int16_t clip(int32_t in, int16_t clipLimit = 32767,
               int16_t resultLimit = 32767) {
    int32_t result = in;
    if (result > clipLimit) {
      result = resultLimit;
    }
    if (result < -clipLimit) {
      result = -resultLimit;
    }
    return result;
  }
};

/**
 * @brief Boost AudioEffect
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 *
 */

class Boost : public AudioEffect, public VolumeSupport {
public:
  /// Boost Constructor: volume 0.1 - 1.0: decrease result; volume >0: increase
  /// result
  Boost(float volume = 1.0) { setVolume(volume); }

  Boost(const Boost &copy) = default;

  effect_t process(effect_t input) {
    if (!active())
      return input;
    int32_t result = volume() * input;
    // clip to int16_t
    return clip(result);
  }

  Boost *clone() { return new Boost(*this); }

};

/**
 * @brief Distortion AudioEffect
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

class Distortion : public AudioEffect {
public:
  /// Distortion Constructor: e.g. use clipThreashold 4990 and maxInput=6500
  Distortion(int16_t clipThreashold = 4990, int16_t maxInput = 6500) {
    p_clip_threashold = clipThreashold;
    max_input = maxInput;
  }

  Distortion(const Distortion &copy) = default;

  void setClipThreashold(int16_t th) { p_clip_threashold = th; }

  int16_t clipThreashold() { return p_clip_threashold; }

  void setMaxInput(int16_t maxInput) { max_input = maxInput; }

  int16_t maxInput() { return max_input; }

  effect_t process(effect_t input) {
    if (!active())
      return input;
    // the input signal is 16bits (values from -32768 to +32768
    // the value of input is clipped to the distortion_threshold value
    return clip(input, p_clip_threashold, max_input);
  }

  Distortion *clone() { return new Distortion(*this); }

protected:
  int16_t p_clip_threashold;
  int16_t max_input;
};

/**
 * @brief Fuzz AudioEffect
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

class Fuzz : public AudioEffect {
public:
  /// Fuzz Constructor: use e.g. effectValue=6.5; maxOut = 300
  Fuzz(float fuzzEffectValue = 6.5, uint16_t maxOut = 300) {
    p_effect_value = fuzzEffectValue;
    max_out = maxOut;
  }

  Fuzz(const Fuzz &copy) = default;

  void setFuzzEffectValue(float v) { p_effect_value = v; }

  float fuzzEffectValue() { return p_effect_value; }

  void setMaxOut(uint16_t v) { max_out = v; }

  uint16_t maxOut() { return max_out; }

  effect_t process(effect_t input) {
    if (!active())
      return input;
    float v = p_effect_value;
    int32_t result = clip(v * input);
    return map(result * v, -32768, +32767, -max_out, max_out);
  }

  Fuzz *clone() { return new Fuzz(*this); }

protected:
  float p_effect_value;
  uint16_t max_out;
};

/**
 * @brief Tremolo AudioEffect
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

class Tremolo : public AudioEffect {
public:
  /// Tremolo constructor -  use e.g. duration_ms=2000; depthPercent=50;
  /// sampleRate=44100
  Tremolo(int16_t duration_ms = 2000, uint8_t depthPercent = 50,
          uint32_t sampleRate = 44100) {
    this->duration_ms = duration_ms;
    this->sampleRate = sampleRate;
    this->p_percent = depthPercent;
    int32_t rate_count = sampleRate * duration_ms / 1000;
    rate_count_half = rate_count / 2;
  }

  Tremolo(const Tremolo &copy) = default;

  void setDuration(int16_t ms) {
    this->duration_ms = ms;
    int32_t rate_count = sampleRate * ms / 1000;
    rate_count_half = rate_count / 2;
  }

  int16_t duration() { return duration_ms; }

  void setDepth(uint8_t percent) { p_percent = percent; }

  uint8_t depth() { return p_percent; }

  effect_t process(effect_t input) {
    if (!active())
      return input;

    // limit value to max 100% and calculate factors
    float tremolo_depth = p_percent > 100 ? 1.0 : 0.01 * p_percent;
    float signal_depth = (100.0 - p_percent) / 100.0;

    float tremolo_factor = tremolo_depth / rate_count_half;
    int32_t out = (signal_depth * input) + (tremolo_factor * count * input);

    // saw tooth shaped counter
    count += inc;
    if (count >= rate_count_half) {
      inc = -1;
    } else if (count <= 0) {
      inc = +1;
    }

    return clip(out);
  }

  Tremolo *clone() { return new Tremolo(*this); }

protected:
  int16_t duration_ms;
  uint32_t sampleRate;
  int32_t count = 0;
  int16_t inc = 1;
  int32_t rate_count_half; // number of samples for on raise and fall
  uint8_t p_percent;
};

/**
 * @brief Delay/Echo AudioEffect. See
 * https://wiki.analog.com/resources/tools-software/sharc-audio-module/baremetal/delay-effect-tutorial
 * Howver the dry value and wet value were replace by the depth parameter.
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class Delay : public AudioEffect {
public:
  /// e.g. depth=0.5, ms=1000, sampleRate=44100
  Delay(uint16_t duration_ms = 1000, float depth = 0.5,
        float feedbackAmount = 1.0, uint32_t sampleRate = 44100) {
    setSampleRate(sampleRate);
    setFeedback(feedbackAmount);
    setDepth(depth);
    setDuration(duration_ms);
  }

  Delay(const Delay &copy) {
    setSampleRate(copy.sampleRate);
    setFeedback(copy.feedback);
    setDepth(copy.depth);
    setDuration(copy.duration);
  };

  void setDuration(int16_t dur) {
    duration = dur;
    updateBufferSize();
  }

  int16_t getDuration() { return duration; }

  void setDepth(float value) {
    depth = value;
    if (depth > 1.0f)
      depth = 1.0f;
    if (depth < 0.0f)
      depth = 0.0f;
  }

  float getDepth() { return depth; }

  void setFeedback(float feed) {
    feedback = feed;
    if (feedback > 1.0f)
      feedback = 1.0f;
    if (feedback < 0.0f)
      feedback = 0.0f;
  }

  float getFeedback() { return feedback; }

  void setSampleRate(int32_t sample) {
    sampleRate = sample;
    updateBufferSize();
  }

  float getSampleRate() { return sampleRate; }

  effect_t process(effect_t input) {
    if (!active())
      return input;

    // Read last audio sample in each delay line
    int32_t delayed_value = buffer[delay_line_index];

    // Mix the above with current audio and write the results back to output
    int32_t out = ((1.0f - depth) * input) + (depth * delayed_value);

    // Update each delay line
    buffer[delay_line_index] = clip(feedback * (delayed_value + input));

    // Finally, update the delay line index
    if (delay_line_index++ >= delay_len_samples) {
      delay_line_index = 0;
    }
    return clip(out);
  }

  Delay *clone() { return new Delay(*this); }

protected:
  Vector<effect_t> buffer{0};
  float feedback = 0.0f, duration = 0.0f, sampleRate = 0.0f, depth = 0.0f;
  size_t delay_len_samples = 0;
  size_t delay_line_index = 0;

  void updateBufferSize() {
    if (sampleRate > 0 && duration > 0) {
      size_t newSampleCount = sampleRate * duration / 1000;
      if (newSampleCount != delay_len_samples) {
        delay_len_samples = newSampleCount;
        buffer.resize(delay_len_samples);
        memset(buffer.data(),0,delay_len_samples*sizeof(effect_t));
        LOGD("sample_count: %u", (unsigned)delay_len_samples);
      }
    }
  }
};

/**
 * @brief ADSR Envelope: Attack, Decay, Sustain and Release.
 * Attack is the time taken for initial run-up oeffect_tf level from nil to
 peak, beginning when the key is pressed.
 * Decay is the time taken for the subsequent run down from the attack level to
 the designated sustainLevel level.
 * Sustain is the level during the main sequence of the sound's duration, until
 the key is released.
 * Release is the time taken for the level to decay from the sustainLevel level
 to zero after the key is released.[4]
 * @ingroup effects
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

class ADSRGain : public AudioEffect {
public:
  ADSRGain(float attack = 0.001, float decay = 0.001, float sustainLevel = 0.5,
           float release = 0.005, float boostFactor = 1.0) {
    this->factor = boostFactor;
    adsr = new ADSR(attack, decay, sustainLevel, release);
  }

  ADSRGain(const ADSRGain &ref) {
    adsr = new ADSR(*(ref.adsr));
    factor = ref.factor;
    copyParent((AudioEffect *)&ref);
  };

  virtual ~ADSRGain() { delete adsr; }

  void setAttackRate(float a) { adsr->setAttackRate(a); }

  float attackRate() { return adsr->attackRate(); }

  void setDecayRate(float d) { adsr->setDecayRate(d); }

  float decayRate() { return adsr->decayRate(); }

  void setSustainLevel(float s) { adsr->setSustainLevel(s); }

  float sustainLevel() { return adsr->sustainLevel(); }

  void setReleaseRate(float r) { adsr->setReleaseRate(r); }

  float releaseRate() { return adsr->releaseRate(); }

  void keyOn(float tgt = 0) { adsr->keyOn(tgt); }

  void keyOff() { adsr->keyOff(); }

  effect_t process(effect_t input) {
    if (!active())
      return input;
    effect_t result = factor * adsr->tick() * input;
    return result;
  }

  bool isActive() { return adsr->isActive(); }

  ADSRGain *clone() { return new ADSRGain(*this); }

protected:
  ADSR *adsr;
  float factor;
};

/**
 * @brief Shifts the pitch by the indicated step size: e.g. 2 doubles the pitch
 * @author Phil Schatzmann
 * @ingroup effects
 * @copyright GPLv3
 */
class PitchShift : public AudioEffect {
public:
  /// Boost Constructor: volume 0.1 - 1.0: decrease result; volume >0: increase
  /// result
  PitchShift(float shift_value = 1.0, int buffer_size = 1000) {
    effect_value = shift_value;
    size = buffer_size;
    buffer.resize(buffer_size);
    buffer.setIncrement(shift_value);
  }

  PitchShift(const PitchShift &ref) {
    size = ref.size;
    effect_value = ref.effect_value;
    buffer.resize(size);
    buffer.setIncrement(effect_value);
  };

  float value() { return effect_value; }

  void setValue(float value) {
    effect_value = value;
    buffer.setIncrement(value);
  }

  effect_t process(effect_t input) {
    if (!active())
      return input;
    buffer.write(input);
    return buffer.read();
  }

  PitchShift *clone() { return new PitchShift(*this); }

protected:
  VariableSpeedRingBuffer<int16_t> buffer;
  float effect_value;
  int size;
};


/**
 * @brief Compressor inspired by https://github.com/YetAnotherElectronicsChannel/STM32_DSP_COMPRESSOR/blob/master/code/Src/main.c
 * @author Phil Schatzmann
 * @ingroup effects
 * @copyright GPLv3
*/
/*
 * modified by W. Voigt for Stereo and Limiter
*/

bool Compressor_Stereo = true;
bool Compressor_Active = false;
float sampleArr[2];

class Compressor : public AudioEffect { 
public:    
    /// Copy Constructor
    Compressor(const Compressor &copy) = default;

    /// Default Constructor
    Compressor(float sampleRate = 44100, float attackMs=5, float releaseMs=200, float holdMs=10, 
               float thresholdPercent=50, float compressionRatio=50){
        
        // Attack -> 5 ms -> 100
        // Release -> 10 ms -> 1000
        
        sample_rate = sampleRate; 
	    current_gain = 1.0f;
        ratio = compressionRatio;
        setThreshold(thresholdPercent);
        setAttack(attackMs);
        setRelease(releaseMs);    
    }

    /// Defines the attack duration in ms
    void setAttack(float attack_ms){
        float attack_samples = sample_rate * (attack_ms / 1000.0);
        attack_coeff = 1 / attack_samples;
        if (attack_coeff > 1.0) attack_coeff = 1.0;
        else if (attack_coeff < 0.0) attack_coeff = 0.0;
    }

    /// Defines the release duration in ms
    void setRelease(float release_ms){
        float release_samples = sample_rate * (release_ms / 1000.0);    
        release_coeff = 1 / release_samples;
        if (release_coeff > 1.0) release_coeff = 1.0;
        else if (release_coeff < 0.0) release_coeff = 0.0;
    }

    /// Defines the threshold in dB
    void setThreshold(float thresholdPercent){
        if (thresholdPercent > 99) thresholdPercent = 99;
        else if (thresholdPercent < 1) thresholdPercent = 1;
        threshold = -0.5 * log10(1 - thresholdPercent / 100);
        if (threshold > 1) threshold = 1;
        else if (threshold < 0) threshold = 0;
    }

    /// Defines the compression ratio from 1 to 100
    void setCompressionRatio(float compressionRatio){
        if (compressionRatio < 1) compressionRatio = 1;
        ratio = compressionRatio;
    }

    /// Processes the sample
    effect_t process(effect_t input) {
        if (!active())
          return input;
        return compress(input);
    }
    
    Compressor *clone() { return new Compressor(*this); }

protected:

    float sample_rate, threshold, ratio, current_gain;
    float attack_coeff, release_coeff;

    float compress(float inSampleF){
        
        float normalized_input = fabs(inSampleF / 32767.0);
        float normalized_output;

        if (normalized_input <= threshold) {
            // Below knee: no compression, output equals input
            normalized_output = normalized_input;
        } else {
            // Above knee: compression at the specified ratio
            normalized_output = threshold + (normalized_input - threshold) / ratio;
        }
        float target_gain;
        if (normalized_input <= 0) target_gain = 1.0;
        else target_gain = normalized_output / normalized_input;
        if (target_gain > 1.0) target_gain = 1.0;
        else if (target_gain < 0.0) target_gain = 0.0; 

        // Smooth the gain with attack and release times
        if (target_gain < current_gain) {
            current_gain = current_gain + (target_gain - current_gain) * attack_coeff;            
            if (current_gain < 0.9) Compressor_Active = true;
        } else { 
            current_gain = current_gain + (target_gain - current_gain) * release_coeff;            
            if (current_gain > 0.95) Compressor_Active = false;
        }
        if (current_gain > 1.0) current_gain = 1.0;
        if (current_gain < 0.0) current_gain = 0.0;

        sampleArr[0] = current_gain * sampleArr[0];
        sampleArr[1] = current_gain * sampleArr[1];
        inSampleF = current_gain * inSampleF;
        return inSampleF;
    }
};


} // namespace audio_tools
