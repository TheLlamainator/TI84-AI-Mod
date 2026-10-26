# Wiring and hardware

> **TODO (author):** this section is yours to write. Suggested contents are below, taken from the project's files.

## Parts

From `pcb/cheating-calc.kicad_sch` (the files for ordering the board assembled are in `pcb/jlcpcb/`, see below):

- 1x Seeed Studio **XIAO ESP32S3 Sense** (camera version). The original schematic names a XIAO ESP32C3. The pin names are the same, but only the S3 Sense has a camera.
- 2x **MMBF170** N-channel MOSFETs (SOT-23). One for the tip line, one for the ring line.
- 4x resistors (the schematic shows them only as `R`; fill in the values you used).
- 1x 4-pin header for the link cable: 5V, GND, TIP, RING.
- A link cable or plug for the calculator's link port. TI-83+/84+ calculators use a 2.5 mm stereo port: check yours before buying.

## What the board does

The calculator's TIP and RING link lines work at up to about 5 V and are open-drain: either side pulls the line low to send. The ESP32's pins are 3.3 V. The board is a bidirectional level shifter for each line: one MOSFET plus a pull-up on each side.

The sketch uses `TIP = D1` and `RING = D10` (`esp32/esp32.ino`).

## Pictures

![The built board](/img/built.png)

![Schematic](/img/schematic.png)

![PCB layout](/img/pcb.png)

## Files

- `pcb/cheating-calc.kicad_sch`, `.kicad_pcb`: KiCad design.
- `pcb/0.1/`: gerber and drill files, ready to send to a PCB maker.
- `pcb/jlcpcb/`: everything to upload when ordering the board assembled from JLCPCB. **Full ordering steps: [ordering the PCB](/ordering-the-pcb).** Upload `cheating-calc-gerbers.zip` as the gerber file, then `BOM.csv` and `CPL.csv` at the assembly step. They cover only the two SMD parts (Q1 and Q2, the MMBF170 MOSFETs, LCSC C130879). The resistors, header and XIAO module are soldered by hand. Positions are measured from the board's lower-left corner.
- `pcb/cpl-all-parts.csv`: the placement of every part, for reference or hand assembly.
- `pcb/schematic.png`, `pcb/pcb.png`, `pcb/built.png`: pictures.

## Your build notes go here

- Photos of the finished mod
- How it is mounted
- Soldering notes
- Pinout and cable colours
