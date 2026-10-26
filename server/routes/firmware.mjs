import express from "express";
import path from "path";
import fs from "fs";
import crypto from "crypto";

// Over-the-air firmware for the ESP32. Protected by a shared secret (FIRMWARE_TOKEN in .env), sent by the
// board as the password of an HTTP Basic login. Without a token set, these routes are switched off.
//   GET /firmware/version      -> the build number of firmware/latest.bin, as plain text
//   GET /firmware/latest.bin   -> the firmware image
export function firmware() {
  const router = express.Router();
  const dir = path.join(process.cwd(), "firmware");
  const token = process.env.FIRMWARE_TOKEN ?? "";

  const digest = (s) => crypto.createHash("sha256").update(s).digest();

  router.use((req, res, next) => {
    if (token.length < 16) {
      res.status(503).send("firmware updates are disabled");
      return;
    }
    const m = (req.headers.authorization ?? "").match(/^Basic (.+)$/i);
    let given = "";
    if (m) {
      const decoded = Buffer.from(m[1], "base64").toString("utf8");
      given = decoded.slice(decoded.indexOf(":") + 1);
    }
    if (!crypto.timingSafeEqual(digest(given), digest(token))) {
      res.sendStatus(401);
      return;
    }
    next();
  });

  router.get("/version", (req, res) => {
    try {
      res.type("text/plain").send(fs.readFileSync(path.join(dir, "version.txt"), "utf8").trim());
    } catch {
      res.sendStatus(404);
    }
  });

  router.get("/latest.bin", (req, res) => {
    res.sendFile(path.join(dir, "latest.bin"), {
      headers: { "Content-Type": "application/octet-stream", "Cache-Control": "no-store" },
    }, (err) => {
      if (err && !res.headersSent) res.sendStatus(404);
    });
  });

  return router;
}
