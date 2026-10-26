# Server setup

The calculator talks to a small Node.js server over the ESP32's WiFi. The server does the things the ESP32 can't do itself: it asks OpenAI for answers, stores chat messages, serves images and programs, hands out the calculator launcher program, and hosts firmware for wireless updates.

This guide installs it on a small Linux server (tested on Debian 13 with 464 MB of RAM) and keeps it running across reboots. You could run it on any machine the board can reach over WiFi, including a PC on your home network.

## What you need

- A Linux server or PC with a public or local IP address, and root or `sudo` access.
- Node.js 20 or newer. These steps use Node 22.
- An OpenAI API key (for the GPT feature).
- The `server/` folder from this repository.
- `source/prepare8xp.mjs` from this repository: a small helper the program-download route needs (see step 4).

## 1. Install Node.js

Debian's package archive may not have a recent Node, so use the official build from nodejs.org:

```sh
cd /tmp
V=$(curl -fsSL https://nodejs.org/dist/latest-v22.x/SHASUMS256.txt | grep -o "node-v[0-9.]*-linux-x64.tar.xz" | head -1)
curl -fsSLO https://nodejs.org/dist/latest-v22.x/$V
tar -xJf $V -C /usr/local --strip-components=1
rm $V
node -v && npm -v
```

On a Raspberry Pi or other ARM board, swap `linux-x64` for `linux-arm64` (or `armv7l`).

## 2. Create a user and copy the server over

Run the server as its own unprivileged user, not as root.

```sh
useradd --system --home /opt/ti32-server --shell /usr/sbin/nologin ti32-server
mkdir -p /opt/ti32-server
```

From your PC, copy the folder (leave out `node_modules`):

```sh
# from the repository root
tar --exclude=node_modules -cf - -C server . | ssh YOUR_SERVER 'cd /opt/ti32-server && tar -xf -'
```

Then on the server:

```sh
chown -R ti32-server:ti32-server /opt/ti32-server
su -s /bin/sh ti32-server -c "cd /opt/ti32-server && npm install --omit=dev"
```

## 3. Add your secrets

Create `/opt/ti32-server/.env`. This file is private to the server user. It is never committed to git.

```sh
umask 077
cat > /opt/ti32-server/.env <<'EOF'
OPENAI_API_KEY=your-openai-key
FIRMWARE_TOKEN=a-long-random-string-at-least-16-characters
EOF
chown ti32-server:ti32-server /opt/ti32-server/.env
chmod 600 /opt/ti32-server/.env
```

| Variable | What it does |
| --- | --- |
| `OPENAI_API_KEY` | Required. Without it the server crashes on start, because the GPT route can't be created. |
| `FIRMWARE_TOKEN` | The password for wireless firmware updates. Make it long and random (`openssl rand -hex 24`). If it's missing or shorter than 16 characters, the `/firmware` routes switch themselves off and answer 503. |
| `PORT` | Optional. Defaults to 8080. The same number goes in the ESP32's `SERVER` setting. |

## 4. Add the helper file the program route needs

`routes/programs.mjs` loads `../../source/prepare8xp.mjs`, which is outside the `server/` folder. With the server in `/opt/ti32-server`, that path is `/opt/source/prepare8xp.mjs`:

```sh
mkdir -p /opt/source
# copy source/prepare8xp.mjs from this repository into it, for example:
# scp source/prepare8xp.mjs YOUR_SERVER:/opt/source/prepare8xp.mjs
chmod 644 /opt/source/prepare8xp.mjs
```

If you skip this, the server still starts, and downloading a calculator program from the APPS menu fails with HTTP 501. Everything else works.

## 5. Run it as a service

Create `/etc/systemd/system/ti32-server.service`:

```ini
[Unit]
Description=TI-32 server
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=ti32-server
WorkingDirectory=/opt/ti32-server
ExecStart=/usr/local/bin/node index.mjs
Restart=always
RestartSec=5
Environment=NODE_ENV=production
NoNewPrivileges=true
ProtectSystem=full
ReadWritePaths=/opt/ti32-server

[Install]
WantedBy=multi-user.target
```

The `WorkingDirectory` matters: the server finds its `images/`, `programs/`, `launcher/` and `firmware/` folders relative to it, and writes `chat.json` there.

Start it, and make it start on boot:

```sh
systemctl daemon-reload
systemctl enable --now ti32-server
systemctl status ti32-server
journalctl -u ti32-server -f      # live log, Ctrl+C to leave
```

## 6. Check that it works

On the server:

```sh
curl -s localhost:8080/programs/list?p=0
```

You should see a short list of program names. Then from another machine on the same network, replace the address with yours:

```sh
curl -s "http://YOUR_SERVER_IP:8080/programs/list?p=0"
```

If the first works and the second doesn't, something is blocking port 8080: a firewall on the machine, your cloud provider's firewall, or your router.

## What the server provides

| Route | Used for |
| --- | --- |
| `/gpt/ask?question=...` | Asks OpenAI and returns the text. |
| `/chats/...` | Simple chat rooms between devices. Stored in `chat.json`. |
| `/image/list`, `/image/get` | Pictures (96x63, 1 bit) shown from the IMAGES menu. |
| `/programs/list`, `/programs/get` | Calculator programs for the APPS menu. |
| `/launcher` | The calculator program (`TI32`). SETTINGS > UPDATE downloads it from here. |
| `/firmware/version`, `/firmware/latest.bin` | Wireless firmware updates. Needs the `FIRMWARE_TOKEN` password. |

## Updating the calculator program from the server

The program that runs on the calculator is served from `launcher/launcher.var`. To change it, replace that file and press **SETTINGS > UPDATE** on the calculator. No reflashing is needed. The file is the program's raw bytes: a two-byte length, then the tokenised program. It must match the copy built into the firmware (`launcher.h`), which is only used if the server can't be reached.

## Publishing wireless firmware updates

The board checks `/firmware/version` and, if the number there is higher than its own `FW_VERSION`, downloads `/firmware/latest.bin`.

1. Raise `FW_VERSION` in the sketch.
2. Build the firmware. Use exactly the same board options as your IDE uses for USB uploads, or the new firmware may not boot. `arduino-cli compile --output-dir ...` with the same FQBN works well.
3. Copy `esp32.ino.bin` to the server as `/opt/ti32-server/firmware/latest.bin`.
4. Write the new number into `/opt/ti32-server/firmware/version.txt`.
5. On the calculator: **SETTINGS > CONNECT**, then **SETTINGS > FIRMWARE**.

```sh
mkdir -p /opt/ti32-server/firmware
echo 2 > /opt/ti32-server/firmware/version.txt
chown -R ti32-server:ti32-server /opt/ti32-server/firmware
```

Once a board has a firmware with this feature, it never needs a USB cable for code changes again, as long as it can join WiFi. The one exception is WiFi details baked into `secrets.h`: those need a new build.

## Security: read this

- **Port 8080 is plain HTTP.** Anything the board sends, including the firmware password, can be read by someone on the same network path. For a hobby project on your own network that is a fair trade. For a public server, put the server behind HTTPS with a reverse proxy such as nginx or Caddy, and change the board's `SERVER` setting to `https://...` (and enable `SECURE` in `secrets.h`).
- **The GPT route has no login.** Anyone who finds your server's address can use your OpenAI key through it. **Set a monthly spending limit in your OpenAI account before you open the port.** Closing the port to everyone except your own networks is better still.
- Internet scanners will find an open port within hours. In the log you'll see random requests for paths like `/boaform/admin/formLogin`. They get a 404 and are harmless, but they show the port is being probed.
- Never commit `.env` or `secrets.h`. Keep `.env.example` and `secrets.example.h` in the repository instead.

## Troubleshooting

| Symptom | Likely cause |
| --- | --- |
| The service restarts over and over; the log says `OPENAI_API_KEY environment variable is missing` | `.env` is missing, not readable by the service user, or has no key. |
| `ERR_MODULE_NOT_FOUND ... prepare8xp.mjs` in the log | Step 4 was skipped. Add the file as in step 4 (this repository's `routes/programs.mjs` loads it lazily, so a missing file only breaks program downloads). |
| `/firmware/...` returns 503 | `FIRMWARE_TOKEN` is missing or under 16 characters. |
| `/firmware/...` returns 401 | The password on the board (`OTA_TOKEN` in `secrets.h`) doesn't match `FIRMWARE_TOKEN` on the server. |
| Works from the server, not from other machines | Firewall or router blocking port 8080. |
| The board shows "error making request" now and then | Usually the phone hotspot dropped. Run SETTINGS > CONNECT again. |

## Credits and licence

This server is part of a mod of the original **TI-32** project, which is licensed under the **GNU General Public License v3**. This repository keeps that licence. See `LICENSE` and `NOTICE.md` for what was taken from the original and what was changed.
