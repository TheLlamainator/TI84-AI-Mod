# TI84-AI-Mod

A modified version of the **TI-32** calculator project: an ESP32-S3 board wired to the link port of a TI-84 graphing calculator, giving it WiFi. It adds a camera view, chat with GPT (with replies), several WiFi networks, a self-hosted server, and **wireless firmware updates**.

Based on the original TI-32 project, which is licensed under the GPL v3. See the credits section of the docs.

> **Use at your own risk, and read the rules where you use it.** A calculator with internet access can break exam and classroom rules. This repository is for learning and tinkering. What you do with it is up to you and your school or exam board.

## Documentation

**Full guide: https://thellamainator.github.io/TI84-AI-Mod/**

| Step | Page |
| --- | --- |
| Downloads (PCB files, firmware, server) | [docs/downloads.md](docs/downloads.md) |
| Overview and diagrams | [docs/overview.md](docs/overview.md) |
| 1. Order the PCB | [docs/ordering-the-pcb.md](docs/ordering-the-pcb.md) |
| 2. Wiring and parts | [docs/wiring.md](docs/wiring.md) |
| 3. Server setup | [docs/server-setup.md](docs/server-setup.md) |
| 4. Flash and use it | [docs/firmware-setup.md](docs/firmware-setup.md) |

## Your secrets stay out of git

Copy `esp32/secrets.example.h` to `secrets.h` and `server/.env.example` to `.env`, then fill them in. Both are in `.gitignore`. **Never commit them.**

## Licence

GNU GPL v3, as the original TI-32 project. See [LICENSE](LICENSE) and [NOTICE.md](NOTICE.md).
