# ESPZeverSolar
Arduino app to communicate directly with a Zever Solar inverter to get current information

## Setup

Install the ESP32 board support and the `TFT_eSPI` library in Arduino IDE.

Update these values at the top of `ESPZeverSolar.ino` before flashing:

```cpp
const char *WIFI_SSID = "YOUR_WIFI_SSID";
const char *WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";
```

The sketch polls `http://192.168.1.136/home.cgi` every 30 seconds, reads line 12 of the plain text response as kilowatts, and displays it as watts on the LCD.

## LCD troubleshooting

The sketch enables LCD backlight pin `4`, which is common for LilyGO T-Display ESP32 boards. If your display stays blank, check your board variant:

- If the serial monitor says the LCD initialized but the screen is dark, update `TFT_BACKLIGHT_PIN` in `ESPZeverSolar.ino`.
- If the backlight is on but nothing is drawn, configure `TFT_eSPI` for your exact LilyGO display model and pinout.
- Open the serial monitor at `115200` baud to confirm the sketch reaches `LCD initialized`.
