# PowerMeter
Reads the power usage data etc. (every 10 sec.) from the KAMSTRUP power meter, de-crypt and publish all topics to the MQTT server.
This project aims to port the the great work done by Claus Nielsen and others (https://xipher.dk/posts/2020-05-17-using-esp8266-to-monitor-kamstrup-omnipower/)
to the ESP32 ESP-IDF platform.

# Hardware
The Kamstrup powermeter utilizes an 4.15 VDC 75 mA power supply - as the ESP32 WROOM module uses a bit more current, especially during active wifi communication,
a power supply backup system has been constructed including a 1F super-cap and the MIC5249 LDO (see schematic in the 'HW' folder). The MIC5249 features an active low
(open drain) RESET output in order to give the ESP32 module a ferm starting point in relation to the very slow rising of the power supply voltage, - remember to mount a e.g.
100k resistor between Vdd and reset on the ESP32 module. No worring noise rippel or voltage dips has been observed on the 4.15V or the 3.3V power lines.

# Software
The software is based on the ESP-IDF framework on the VSCODE IDE with the PLATFORMIO extention and is inspired by the great work done by Claus Nielsen and others:
(https://xipher.dk/posts/2020-05-17-using-esp8266-to-monitor-kamstrup-omnipower/).
 - The application creates only the 'uart_rx_task' task which listen on the serial port (UART #1) for an mbus message comming from the power meter.
 - The message will get authenticated and decrypted with the two keys supplied (in denmark) by the power company.
 - The decrypted message will get parsed and then published to the MQTT server
 
 In order to save power the ESP32 'light-sleep' mode is utilized for ~6 sec. after the MQTT publish is done. Also for power resons, the CPU clock frequency has been
 has been chosen to 80 MHz. No worrying rippel "noise" or voltage dips has been observed on the 4.15V or the 3.3V supply lines.
 The current SW release has been running for several weeks now without any "hick-ups".

# Integration
There are several individual needs in the code that must to be configured before implementation:
 - Wifi SSID and password (in 'station_example_main.c', line 27 and 28)
 - The URL, username and password for the MQTT server (in 'Mqtt.c', lines 8, 247 and 248)
 - The two private decryption keys (in 'AESdecrypt.c', line 37 and 38)
 
 Make sense to put this in a .h configuration file (backlog).


