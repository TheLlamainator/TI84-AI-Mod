# Flashing the ESP32 and setting up the calculator

You need the hardware built ([wiring.md](wiring.md)) and the server running ([server-setup.md](server-setup.md)).

## 1. Arduino IDE setup

1. Install the **Arduino IDE** (2.x).
2. **File > Preferences**, and add this to *Additional boards manager URLs*:
   `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
3. **Tools > Board > Boards Manager**, search for `esp32` and install **esp32 by Espressif Systems**.
4. Install the libraries:
   - **ArTICL** (provides `TICL.h`, `CBL2.h`, `TIVar.h`). It is not in the Library Manager. Download it from its repository ([github.com/KermMartian/ArTICL](https://github.com/KermMartian/ArTICL)) as a ZIP, then **Sketch > Include Library > Add .ZIP Library...**. If the IDE complains about a missing `library.properties`, unzip it into your `Documents/Arduino/libraries/` folder as a folder named `ArTICL` and restart the IDE.
   - **UrlEncode** by Masayuki Sugahara, from **Sketch > Include Library > Manage Libraries**.

## 2. Configure your secrets

In the `esp32/` folder, copy `secrets.example.h` to `secrets.h` and fill it in:

| Setting | What to put |
| --- | --- |
| `WIFI_SSID`, `WIFI_PASS` | Your WiFi name and password. 2.4 GHz only. On an iPhone hotspot, turn on **Maximize Compatibility** and keep the Personal Hotspot screen open while the board connects. |
| `WIFI_NETWORKS` | The list of networks the board may use. Add more after a comma. It joins the strongest one in range. |
| `SERVER` | Your server, including `http://` and the port, for example `http://203.0.113.5:8080`. Without `http://` the request fails. |
| `CHAT_NAME` | Your name in the chat feature. |
| `OTA_TOKEN` | The update password. Must match `FIRMWARE_TOKEN` in the server's `.env`. |

`secrets.h` is ignored by git. Keep it that way.

## 3. Board settings and first flash (USB)

The board is a **Seeed Studio XIAO ESP32S3**. For the camera you need the **Sense** version, which has a small camera board attached.

In **Tools**, choose:

| Setting | Value |
| --- | --- |
| Board | `XIAO_ESP32S3` |
| PSRAM | **OPI PSRAM** (the camera will not start without it) |
| Partition Scheme | Default with spiffs (3MB APP/1.5MB SPIFFS) |
| USB CDC On Boot | Default |
| Port | the XIAO's COM port |

Open `esp32/esp32.ino` and click **Upload**. If it can't connect, hold the board's **BOOT** button while plugging in USB, then upload again.

Open **Tools > Serial Monitor** at **115200** baud. After a reset you should see `[CBL]`, `[preferences]` and `[ready]`, then `firmware build N`.

Use the **same board settings every time**. Wireless updates are built with these settings, and a mismatch can give you firmware that won't boot.

### Without a camera

The camera is switched on in the sketch with `#define CAMERA`. If the board has no camera attached, it prints `Camera init failed` at start-up and carries on. Everything except the camera works. To remove the camera code entirely, comment out that line.

## 4. Get the program onto the calculator

The menus run from a program on the calculator called `TI32`. The ESP32 sends it over the link cable.

> **TODO (author):** describe how the first transfer is triggered from the calculator with a fresh board, and which button or program starts it. The firmware's `launcher` command (number 5) is what sends the program, and the log shows `queued transfer` when it works.

After that first transfer, updates are done from the calculator itself.

1. Run `TI32` from the PRGM menu.
2. The board ignores the calculator until it receives an unlock number that matches the `PASSWORD` constant in `esp32.ino`. The bundled program sends it for you. If you write your own program, send it first.
3. **SETTINGS > CONNECT** joins WiFi. The Serial Monitor shows a scan, then `trying: <name>` and the result.
4. **SETTINGS > UPDATE** downloads the newest calculator program from your server and sends it. (Without WiFi, or if the server is unreachable, it sends the copy built into the firmware.)

## 5. Using it

The main menu: **SETTINGS, IMAGES, APPS, CHAT, ASK GPT, GPT HISTORY, CAMERA**.

**ASK GPT**

| Key | Does |
| --- | --- |
| ENTER (after typing) | Sends your question. |
| Right / Left arrow | Next / previous page of the answer. |
| ALPHA | Reply to the answer. The whole conversation is sent again. |
| DEL | Clear the conversation. |
| CLEAR | Back to the main menu. |

Letters need ALPHA on the calculator. Use capitals: lowercase letters are cut off.

**CAMERA** takes a picture and keeps refreshing it until you press CLEAR. It needs a Sense board.

## 6. Wireless firmware updates

After the first USB flash you can update the ESP32 without a cable.

1. Change the sketch and raise `FW_VERSION` near the top of `esp32.ino`.
2. Publish it to your server with `publish-firmware.sh` (see the comments at the top of the script for the two settings it needs).
3. On the calculator: **SETTINGS > CONNECT**, then **SETTINGS > FIRMWARE**.
4. The calculator says it is updating. After about a minute the board restarts. Running FIRMWARE again says `up to date: build N`.

**Safety:** an update only switches over after the whole download has been checked and written, so a failed download changes nothing. If a new firmware crashes before the end of start-up, the board automatically goes back to the old one. If something still goes wrong, flash over USB again.

Things that need a USB flash because they are compiled into the firmware: the first install, and changes to `secrets.h`. WiFi details changed in `secrets.h` can still be shipped wirelessly, as long as the board can join some network to download the update.

## Troubleshooting

| Symptom | What to check |
| --- | --- |
| `ssid not found` | The name in `secrets.h` doesn't match exactly, the network is 5 GHz only, or the hotspot is asleep. The Serial Monitor lists every network it can see (`saw: ...`). Compare the names letter by letter. iPhones often use a curly apostrophe. |
| `connect timed out`, with `wifi status: 6` repeated | Usually a wrong password, or a router that refuses the connection (WPA3-only, MAC filtering). Test the password from your phone. |
| `error making request` | Check `SERVER` starts with `http://`. Check the server is up (`curl` from another machine). On a hotspot, run CONNECT again. |
| `cannot reach update server` | The board lost its connection, or the server is down. Run CONNECT, then FIRMWARE. |
| `update password rejected` | `OTA_TOKEN` in `secrets.h` doesn't match `FIRMWARE_TOKEN` on the server. |
| `camera failed to initialize` | PSRAM isn't set to OPI, the camera board isn't seated, or there is no camera. |
| Nothing happens when sending a reply | Make sure the firmware is up to date. Older builds rejected the reply command. |
| `queued transfer` appears but nothing about WiFi | That message is from sending the calculator program, which doesn't need WiFi. It doesn't prove the internet works. |
