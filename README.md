# esp32_Server
Firmware for an esp32 devkitc v4. The program is devided into multiple files:\n\n

wifi_driver: configures a wifi access point and dhcp server\n
http_server: configures a web server and embeddes the files for the webpage in memory, message queue with uart\n
uart_driver: configures uart0 for communication to MockStation (see other repo), message queue with server\n\n

The esp32 is communicating to the MockStation via uart and and provides a web interface for the users.\n
The station can be controlled and progress updates can be viewed by the webpage.
