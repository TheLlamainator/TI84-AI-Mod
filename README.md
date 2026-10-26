# ti84-Ai-Mod

A modified version of the **TI-32** calculator project: an ESP32-S3 board wired to the link port of a TI-84 graphing calculator, giving it WiFi. It adds a camera view, chat with GPT (with replies), several WiFi networks, a self-hosted server, and **wireless firmware updates**.

Based on the original TI-32 project, which is licensed under the GPL v3. See [Credits](#credits-and-licence).

> **Use at your own risk, and read the rules where you use it.** A calculator with internet access can break exam and classroom rules. This repository is for learning and tinkering. What you do with it is up to you and your school or exam board.

## What it does

- Connects a TI-84 to your WiFi through an ESP32 on the link port.
- **ASK GPT**: type a question on the calculator, read the answer a page at a time, reply to carry on the conversation.
- **Chat** between devices, **images** shown on the calculator screen, and **apps** (calculator programs) downloaded from your server.
- **Camera**: with a XIAO ESP32S3 **Sense**, take pictures and show a slow live view in black and white. Optional.
- **SETTINGS > UPDATE** replaces the calculator's program from your server.
- **SETTINGS > FIRMWARE** replaces the ESP32's firmware over WiFi, so you don't need a USB cable after the first flash.
- Remembers more than one WiFi network and joins the strongest one in range.

## How it fits together

```
 TI-84  <--link cable-->  level-shifter PCB + XIAO ESP32S3  <--WiFi-->  your server (Node.js)  <-->  OpenAI
```

- The **PCB** (`pcb/`) shifts the calculator's 5 V link signals to the ESP32's 3.3 V.
- The **sketch** (`esp32/`) talks the calculator link protocol and your server.
- The **server** (`server/`) does what the ESP32 can't: it calls OpenAI, stores chat, and serves images, programs, the calculator launcher program and firmware.

## Setting it up

1. **Order the PCB**: [docs/ordering-the-pcb.md](docs/ordering-the-pcb.md) (all files are in `pcb/jlcpcb/`)
2. **Build the hardware**: [docs/wiring.md](docs/wiring.md)
3. **Run the server**: [docs/server-setup.md](docs/server-setup.md)
4. **Flash the ESP32 and put the program on the calculator**: [docs/firmware-setup.md](docs/firmware-setup.md)

## Repository layout

| Folder | Contents |
| --- | --- |
| `esp32/` | The Arduino sketch, camera pins, the compiled calculator program (`launcher.h`), `secrets.example.h` and `publish-firmware.sh`. |
| `server/` | The Node.js server. |
| `pcb/` | KiCad project, gerber files and pictures of the level-shifter board. |
| `source/` | Original tools for converting images and calculator programs, and the original `LAUNCHER.8xp` sources. |
| `docs/` | Guides, plus the original project's documentation. |

## Your secrets stay out of git

The sketch needs a `secrets.h` (WiFi details, server address, update password) and the server needs a `.env` (OpenAI key, update password). Both are listed in `.gitignore`. Copy `esp32/secrets.example.h` to `secrets.h` and `server/.env.example` to `.env`, then fill them in. **Never commit them.**

## Known limits

- The calculator link is slow. Pictures refresh about once a second or two at best, and each answer page holds 100 characters.
- Lowercase letters get cut from the end of text typed on the calculator, so capitals are safest.
- The server speaks plain HTTP. Read the security notes in [docs/server-setup.md](docs/server-setup.md) before exposing it to the internet, and set a spending limit on your OpenAI account.
- `source/programs/LAUNCHER.8xp.txt` is the **original** calculator program. The program actually used by this mod is `esp32/launcher.h` and `server/launcher/launcher.var`, which add the CAMERA and FIRMWARE menu entries.

## Credits and licence

This is a modification of the original **TI-32** project, released under the **GNU General Public License v3**. The original author has since taken the project down. The GPL grants everyone who received the code the right to keep, modify and share it, provided the licence and notices are kept. See [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md) for what came from the original and what was changed here.

The calculator link code uses **ArTICL** by Kerm Martian (Cemetech), installed separately from its own repository.

This repository is licensed under the GPL v3 as well.
