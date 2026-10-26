#!/bin/sh
# Build the firmware with the same options the Arduino IDE uses for USB uploads, and publish it to your
# server so the calculator's SETTINGS > FIRMWARE can install it.
#
# Usage:   ./publish-firmware.sh <build number>
# Setup:   export SERVER_SSH=user@your-server      (an ssh login that can write to the server folder)
# Optional: export SERVER_DIR=/opt/ti32-server     (where the server lives, default shown)
#           export ARDUINO_CLI=/path/to/arduino-cli (default: arduino-cli from your PATH)
#
# The build number must match `#define FW_VERSION` in esp32.ino, and be higher than the one already on the
# board, otherwise the board says it is up to date.
set -e
VERSION="$1"
[ -n "$VERSION" ] || { echo "usage: $0 <build number>"; exit 1; }
[ -n "$SERVER_SSH" ] || { echo "set SERVER_SSH first, for example: export SERVER_SSH=root@your-server"; exit 1; }
SERVER_DIR="${SERVER_DIR:-/opt/ti32-server}"
CLI="${ARDUINO_CLI:-arduino-cli}"
grep -q "#define FW_VERSION $VERSION\$" esp32.ino || { echo "esp32.ino does not say FW_VERSION $VERSION"; exit 1; }
[ -f secrets.h ] || { echo "secrets.h is missing: copy secrets.example.h to secrets.h and fill it in"; exit 1; }

OUT="${TMPDIR:-/tmp}/ti32-fw-out"
rm -rf "$OUT"
# These options must match the board settings you use when flashing over USB in the Arduino IDE.
"$CLI" compile \
  --fqbn "esp32:esp32:XIAO_ESP32S3:USBMode=hwcdc,CDCOnBoot=default,PartitionScheme=default_8MB,PSRAM=opi,FlashSize=8M,FlashMode=qio,CPUFreq=240,DebugLevel=none" \
  --output-dir "$OUT" .

scp "$OUT/esp32.ino.bin" "$SERVER_SSH:$SERVER_DIR/firmware/latest.bin"
ssh "$SERVER_SSH" "echo $VERSION > $SERVER_DIR/firmware/version.txt"
echo "published build $VERSION"
echo "If the server runs as a separate user, make sure it can read $SERVER_DIR/firmware/*"
