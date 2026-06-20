# ESPZeverSolar
Arduino app to communicate directly with a Zever Solar inverter to get current information

## Setup

Install the ESP32 board support and the `TFT_eSPI` library in Arduino IDE.

`TFT_eSPI` must be configured in the library's setup file because the library is
compiled separately from the sketch. 

Update these values at the top of `ESPZeverSolar.ino` before flashing:

```cpp
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

The sketch polls `http://192.168.1.136/home.cgi` every 30 seconds, reads line 11 of the plain text response as watts, and displays it as watts on the LCD.

