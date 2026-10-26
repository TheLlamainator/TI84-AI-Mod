import express from "express";
import path from "path";

// Serves the calculator launcher program (TI32) as raw variable data: a 2-byte size, then the program.
// The ESP32's SETTINGS > UPDATE downloads this, so the program can change without re-flashing the board.
export function launcher() {
  const router = express.Router();

  router.get("/", (req, res) => {
    res.sendFile(path.join(process.cwd(), "launcher", "launcher.var"), {
      headers: {
        "Content-Type": "application/octet-stream",
        "Cache-Control": "no-store",
      },
    });
  });

  return router;
}
