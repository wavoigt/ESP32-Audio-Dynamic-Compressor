# ESP32 Audio Dynamic Compressor
<b>mit digitalem Ein- und Ausgang und Analogausgang, optional mit analogem Eingang,<br>
Web Interface zur Steuerung und OTA Programmierung.<b>

![Compressor_Board_small](https://github.com/user-attachments/assets/570caf30-9aeb-4b0c-b139-afd848521b73)

Requisiten:

ESP32 Wrover Board mit PCM5100A 32bit Stereo DAC 'HiFi-ESP32' https://github.com/sonocotta/esp32-audio-dock<br>
ToslinkBee SPDIF zu I2S Converter:<br> 
https://www.audiophonics.fr/en/interface-modules/tinysine-toslinkbee-interface-module-spdif-optical-to-i2s-dir9001-cs8421-24bit-96khz-p-18397.html<br>
Optical Toslink Output Socket on PCB<br>
https://www.audiophonics.fr/en/optical-toslink-plugs/optical-toslink-output-socket-on-pcb-p-17103.html<br>
Optionaler ADC: PCM1802 Stereo-A/D-Wandler, erhältlich bei Amazon

Software: 
Die Software ist für die Arduino IDE gedacht.
Compressor4.ino
CompHtmlServer.h im Compressor4 Ordner
Audio Library https://github.com/pschatzmann/arduino-audio-tools (thx to pschatzmann for this great audio processing library)

Anleitung:

Leider ist der Dynamic Compressor in der arduino-audio-tools library nur für mono Betrieb ausgelegt.<br>
Für Stereo Betrieb musste ich die files AudioEffects.h and AudioEffect.h modifizieren.<br>
Kopiere die files AudioEffects.h and AudioEffect.h in den Arduino library folder: Arduino\libraries\audio-tools\src\AudioTools\CoreAudio\<br>
Falls du die Original files verwenden möchtest, musst du die Zeilen mit 'Compressor_Stereo' und 'Compressor_Active' in der Compressor4.ino auskommentieren.<br>
