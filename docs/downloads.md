---
title: Downloads
---

<script setup>
import { withBase } from 'vitepress'
</script>

# Downloads

Click a file to download it. Everything here is also in the [repository](https://github.com/TheLlamainator/TI84-AI-Mod), and these files are rebuilt from it every time the site updates.

::: tip Everything at once
[Download the whole project as a zip](https://github.com/TheLlamainator/TI84-AI-Mod/archive/refs/heads/main.zip)
:::

## 1. The PCB

For [ordering the board](/ordering-the-pcb) from JLCPCB. Upload the zip as it is.

| File | What it is |
| --- | --- |
| <a class="dl" :href="withBase('/downloads/cheating-calc-gerbers.zip')" download="cheating-calc-gerbers.zip">cheating-calc-gerbers.zip</a> | Gerber and drill files for the PCB |
| <a class="dl" :href="withBase('/downloads/BOM.csv')" download="BOM.csv">BOM.csv</a> | Bill of materials for assembly |
| <a class="dl" :href="withBase('/downloads/CPL.csv')" download="CPL.csv">CPL.csv</a> | Component placement list for assembly |
| <a class="dl" :href="withBase('/downloads/pcb-kicad-project.zip')" download="pcb-kicad-project.zip">pcb-kicad-project.zip</a> | KiCad project, if you want to change the board |

## 2. The ESP32 firmware

For [flashing the board](/firmware-setup).

| File | What it is |
| --- | --- |
| <a class="dl" :href="withBase('/downloads/esp32-sketch.zip')" download="esp32-sketch.zip">esp32-sketch.zip</a> | The whole Arduino sketch folder, ready to open in the Arduino IDE |
| <a class="dl" :href="withBase('/downloads/secrets.example.h')" download="secrets.example.h">secrets.example.h</a> | Settings template. Save it as `secrets.h` in the sketch folder and fill it in |
| <a class="dl" :href="withBase('/downloads/publish-firmware.sh')" download="publish-firmware.sh">publish-firmware.sh</a> | Script that builds and publishes firmware for wireless updates |

You also need the **ArTICL** library: [download ArTICL (zip)](https://github.com/KermMartian/ArTICL/archive/refs/heads/master.zip), then add it in the Arduino IDE with Sketch, Include Library, Add .ZIP Library.

## 3. The server

For [setting up the server](/server-setup).

| File | What it is |
| --- | --- |
| <a class="dl" :href="withBase('/downloads/server.zip')" download="server.zip">server.zip</a> | The Node.js server, without secrets |
| <a class="dl" :href="withBase('/downloads/env.example')" download="env.example">env.example</a> | Settings template. Save it as `.env` on the server and fill it in |
| <a class="dl" :href="withBase('/downloads/prepare8xp.mjs')" download="prepare8xp.mjs">prepare8xp.mjs</a> | Small helper the program-download route needs. Put it in `/opt/source/` |

::: warning Keep your secrets private
`secrets.h` and `.env` hold your WiFi passwords, API key and update password. They are not in these downloads, and they should never be shared or committed.
:::
