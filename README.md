# ESP32 Audio Dynamic Compressor / Limiter
<b>mit digitalem Ein- und Ausgang und Analogausgang, optional mit analogem Eingang,<br>
Web Interface zur Steuerung und OTA Programmierung.

with digital input and output and analogue output, optionally with analogue input, <br>
Web interface for control and OTA programming. English -> scroll down</b>

Ich verwende diese Schaltung zwischen TV und Soundsystem, um die zum Teil sehr hohe Dynamik von Spielfilmen einzuschränken.<br>
Diese Anleitung ist nicht für Anfänger gedacht.<br> Grundlegende Kenntnisse in Programmierung mit der Arduino IDE setze ich voraus.

![Compr_von_oben_kl](https://github.com/user-attachments/assets/a5108c59-f7f9-48c5-bd7a-d229d6821879)

Web Interface<br>
![WebInterface](https://github.com/user-attachments/assets/3b8c5aef-7430-4c4a-8d2b-5738e23c4913) 

<b>Hardware:</b>

ESP32 Wrover Board mit PCM5100A 32bit Stereo DAC 'HiFi-ESP32' https://github.com/sonocotta/esp32-audio-dock<br>
Das Board passt genau in ein Raspberry Pi Gehäuse.<br>
Alternativ kann jedes ESP32 Wrover Board mit 4Mb Flash und 4Mb PSRam verwendet werden (ohne DAC)<br>
ToslinkBee SPDIF zu I2S Converter:<br> 
https://www.audiophonics.fr/en/interface-modules/tinysine-toslinkbee-interface-module-spdif-optical-to-i2s-dir9001-cs8421-24bit-96khz-p-18397.html<br>
Optical Toslink Output Socket on PCB<br>
https://www.audiophonics.fr/en/optical-toslink-plugs/optical-toslink-output-socket-on-pcb-p-17103.html<br>
Optionaler ADC: PCM1802 Stereo-A/D-Wandler, erhältlich bei Amazon

<b>Verdrahtung:</b>
	
<table>
  <tr>
    <th></th>
    <th>I2S DATA (Label)</th>
    <th>I2S CLK (Label)</th>
    <th>I2S WS (Label)</th>
    <th>V+</th>
    <th>GND</th>
</tr>
  <tr>
    <td>PCM5100A (onboard)</td>
    <td>GPIO22</td>
    <td>GPIO26</td>
    <td>GPIO25</td>
    <td>3V3</td>
    <td>GND</td>
  </tr>
  <tr>
    <td>TOSLINKBEE</td>
    <td>GPIO19 (MISO)</td>
    <td>GPIO18 (CLK)</td>
    <td>GPIO14 (RST)</td>
    <td>5V</td>
    <td>GND</td>
  </tr>
  <tr>
    <td>TOSLINK OUT</td>
    <td>GPIO23 (MOSI)</td>
    <td></td>
    <td></td>
    <td>5V</td>
    <td>GND</td>
  </tr>
</table>


<b>Software:</b>

Die Software ist für die Arduino IDE gedacht.<br>
- Compressor4.ino<br>
- CompHtmlServer.h im Compressor4 Ordner<br>
- Audio Library https://github.com/pschatzmann/arduino-audio-tools (thx to pschatzmann for his great audio processing library)<br>

<b>Anleitung:</b>

Leider ist der Dynamic Compressor in der arduino-audio-tools library nur für mono Betrieb ausgelegt.<br>
Für Stereo Betrieb musste ich die files AudioEffects.h and AudioEffect.h modifizieren.<br>
Kopiere die files AudioEffects.h and AudioEffect.h in den Arduino library folder:<br>Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\AudioEffects<br>
Falls du die Original files verwenden möchtest, musst du die Zeilen mit 'Compressor_Stereo' und 'Compressor_Active' in der Compressor4.ino auskommentieren. 
Der Compressor arbeitet dann im mono Betrieb<br>
In CompHtmlServer.h müssen die Wifi Zugangsdaten eingetragen werden. <br>
Alles weitere siehe Compressor4.ino

Die Schaltung tut was sie soll, aber bei hohen Kompressionsraten neigt sie leider zur 'Überkompression', d.h bei lauten Passagen wird das Signal etwas zu stark zurückgeregelt. <br>
Ich empfehle die Einstellung der Ratio etwa bei 50%, das entspricht -6db. <br>
Ideen und Verbesserungsvorschläge sind willkommen :-)

# English:

I use this circuit between TV and sound system to limit the high dynamics of feature films. <br>
These instructions are not intended for beginners. <br>
Basic knowledge of programming with the Arduino IDE is required. <br>

Unfortunately, the Dynamic Compressor in the arduino-audio-tools library is only designed for mono operation. <br>
For stereo operation I had to modify the files AudioEffects.h and AudioEffect.h. <br>
Copy the files AudioEffects.h and AudioEffect.h into the Arduino library folder: <br>
Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\AudioEffects <br>
If you want to use the original files, you must comment out the lines with ‘Compressor_Stereo’ and ‘Compressor_Active’ in Compressor4.ino. <br>
The compressor then works in mono mode. <br>
The Wifi access data must be entered in CompHtmlServer.h. <br>
For everything else, see Compressor4.ino <br>

The circuit does what it should, but at high compression rates it unfortunately tends to ‘overcompress’, i.e. the signal is reduced too much in loud passages. <br>
I recommend setting the ratio to around 50%, which corresponds to -6db. <br>
Ideas and suggestions for improvement are welcome :-)

Translated with DeepL.com (free version)
