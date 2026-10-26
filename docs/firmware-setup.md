# Flashing the ESP32 and setting up the calculator

> **Files to download:** [firmware files](/downloads#_2-the-esp32-firmware)

You need the hardware built ([wiring](/wiring)) and the server running ([server setup](/server-setup)).

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

The board is a **Seeed Studio XIAO ESP32S3**. The camera add-on is **not required**, but it is **recommended**: the **Sense** version has a small camera board attached, and you need it for the CAMERA feature. Everything else works without it.

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

### The first transfer

A fresh board has nothing to send the calculator yet except this one trick. You tell the board the unlock number, then tell it to send the program, using the calculator's `Send(` command by hand.

Before you start, the board must be wired to the calculator and powered.

1. **Store the password in P.** On the home screen, type `69420`, press **STO>**, then the letter **P** (press **ALPHA**, then the key with P above it), then **ENTER**. `69420` is the default `PASSWORD` in `esp32.ino`. If you changed it there, use your number.
2. **Store 5 in C.** Type `5`, press **STO>**, the letter **C**, then **ENTER**. Command 5 is the one that sends the program.
3. **Open the catalog:** press **2nd** then **0**.
4. **Jump to the S entries.** The catalog is already in letter mode, so **don't press ALPHA**. Just press the key with **S** above it, and the list jumps to the entries starting with S.
5. **Scroll down to `Send(`** and press **ENTER** to paste it on the home screen.
6. **Send P first.** Finish it as `Send(P)` and press **ENTER**. This unlocks the board.
7. **Send C.** Do steps 3 to 5 again, finish it as `Send(C)`, and press **ENTER**.
8. **Press PRGM.** The program `TI32` appears in the list. It can take a few seconds, so if it isn't there at first, wait a moment and open PRGM again.

If `TI32` never appears, check the Serial Monitor. You should see `unlocked: 1`, then `command: 5` and `queued transfer`. If the unlock never happens, check the wiring and the password.

### After the first transfer

Once `TI32` is on the calculator, later updates are done from the calculator itself.

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
