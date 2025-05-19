# ESP32 Audio Dynamic Compressor
<b>mit digitalem Ein- und Ausgang und Analogausgang, optional mit analogem Eingang,<br>
Web Interface zur Steuerung und OTA Programmierung.</b>

Ich verwende diese Schaltung zwischen TV und Soundsystem, um die zum Teil sehr hohe Dynamik von Spielfilmen einzuschränken.<br>
Diese Anleitung ist nicht für Anfänger gedacht.<br> Grundlegende Kenntnisse in Programmierung mit der Arduino IDE setze ich voraus.

![Compressor_Board_small](https://github.com/user-attachments/assets/570caf30-9aeb-4b0c-b139-afd848521b73)

Web Interface<br>
![WebInterface](https://github.com/user-attachments/assets/3b8c5aef-7430-4c4a-8d2b-5738e23c4913) 

<b>Requisiten:</b>

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
    <td>PCM1802 (onboard)</td>
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
    <td>GPIO23</td>
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
Alles weitere siehe Compressor4.ino

Die Schaltung tut was sie soll, aber bei hohen Kompressionsraten neigt sie leider zur 'Überkompression', d.h bei lauten Passagen wird das Signal etwas zu stark zurückgeregelt.<br>
Ich empfehle die Einstellung der Ratio etwa bei 50%, das entspricht -6db.<br> 
Ideen und Verbesserungsvorschläge sind willkommen :-)
