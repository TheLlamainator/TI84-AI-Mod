// Copy this file to secrets.h (same folder) and fill in your own values.
// secrets.h is listed in .gitignore. Never commit it: it holds your WiFi passwords and update password.

#ifndef SECRETS_H
#define SECRETS_H

#define HTTP_USERNAME "" // not used
#define HTTP_PASSWORD "" // not used

// The first network. WIFI_NETWORKS below uses these two.
#define WIFI_SSID "your-wifi-name"       // exact name, including capitals and punctuation
#define WIFI_PASS "your-wifi-password"

// Every network the board may use. It joins whichever of these is in range (strongest signal first).
// To add more, put them inside the braces after a comma, for example:
// #define WIFI_NETWORKS { WIFI_SSID, WIFI_PASS }, { "Home WiFi", "home password" }
// The ESP32 only supports 2.4 GHz networks. On an iPhone hotspot, turn on "Maximize Compatibility".
#define WIFI_NETWORKS { WIFI_SSID, WIFI_PASS }

// Where the server runs. Include http:// and the port, with no trailing slash.
#define SERVER "http://YOUR_SERVER_IP:8080"

#define CHAT_NAME "your-name" // your name on the chat program
// #define SECURE // uncomment if SERVER is an https:// address

// Password for wireless firmware updates. Must match FIRMWARE_TOKEN in the server's .env (16+ characters).
#define OTA_TOKEN "change-me-to-a-long-random-string"

#endif // SECRETS_H
