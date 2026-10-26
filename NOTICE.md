# Notice

This repository is a modified version of the **TI-32** project. The original is licensed under the GNU General Public License v3 (see `LICENSE`). Its author has taken it down, so no link is given. If you know the original author's name or an archive, please add it here.

## What came from the original

- `pcb/`: the level-shifter board design, gerbers and pictures.
- `source/`: image and program conversion tools, sample images, and the original calculator program sources.
- `esp32/esp32.ino`, `camera_pins.h`, `camera_index.h`: the original sketch and camera pin definitions. The sketch has been heavily modified (below).
- `server/`: the original Express server routes (`chat`, `chatgpt`, `images`, `programs`) and sample images and programs.
- `docs/original-documentation.md`: the original documentation, unchanged.

## What was changed or added in this version

- **Server:**
  - `routes/launcher.mjs`: serves the calculator program so SETTINGS > UPDATE can download it.
  - `routes/firmware.mjs`: serves firmware for wireless updates, protected by a password.
  - The server no longer logs request `Authorization` headers.
  - `routes/programs.mjs` loads its helper only when needed, so the server starts without it.
- **Sketch:**
  - Camera enabled: grayscale capture, 96x63 black-and-white conversion with dithering, and a live view.
  - WiFi: tries a list of known networks, strongest first; fails with clear errors and a timeout instead of waiting forever.
  - Server address now includes `http://`, which the HTTP client needs.
  - GPT replies: a larger command limit so replies are accepted; only the newest answer is paged through.
  - Calculator UPDATE downloads the program from the server, with the built-in copy as a fallback.
  - Wireless firmware updates with automatic rollback if the new firmware crashes during start-up.
- **Calculator program** (`esp32/launcher.h`, `server/launcher/launcher.var`): CAMERA added to the main menu in place of EXIT; FIRMWARE added to SETTINGS; a working camera loop.
- `esp32/secrets.example.h`, `server/.env.example`, `esp32/publish-firmware.sh`, `.gitignore`, this notice, the README and the guides in `docs/`.

## Third-party software

- **ArTICL** (Arduino TI calculator linking library) by Kerm Martian (Cemetech). Install it separately from its own repository. It is not included here.
- **UrlEncode** (Arduino library) by Masayuki Sugahara, from the Arduino Library Manager.
- **Express**, **OpenAI's Node SDK** and other npm packages listed in `server/package.json`, installed with `npm install`.
