# Wiring and hardware

> **TODO (author):** the wiring part of this page (below the disassembly) is still yours to write.

## 1. Open the calculator

::: info Source and licence
This section is adapted from **[TI-84 Plus Disassembly](https://www.ifixit.com/Guide/TI-84+Plus+Disassembly/117983)** on iFixit, by martijn muntenaar and 6 other contributors, licensed under [Creative Commons BY-NC-SA](https://creativecommons.org/licenses/by-nc-sa/3.0/). Changes: the text is lightly shortened, the photos are converted to PNG, and step 8 is added (it is not from iFixit). The licence on this section is **BY-NC-SA**, not the GPL used for the code. iFixit does not endorse this project.
:::

**Time:** about 10 minutes. **Difficulty:** moderate.

### What you need

- TR6 Torx security screwdriver
- Phillips #0 screwdriver
- Spudger
- Opening picks (a set of 6 works well)

### Step 1: Flip it over

Be sure the calculator is powered off before you start.

Take off the sliding cover, and flip your calculator to the back side.

![Step 1: slide the cover off](/img/disassembly/step-1-0.png)
![Step 1: flip the calculator to the back](/img/disassembly/step-1-1.png)

### Step 2: Remove the battery cover

Press down on the latch, then remove the battery cover.

![Step 2: press the latch](/img/disassembly/step-2-0.png)
![Step 2: battery cover off](/img/disassembly/step-2-1.png)

### Step 3: Take out the batteries

Remove the batteries. You can use the battery cover as a tray to keep them in.

![Step 3: remove the batteries](/img/disassembly/step-3-0.png)
![Step 3: batteries out](/img/disassembly/step-3-1.png)

### Step 4: Back-up battery plate

Removing the back-up battery will clear your RAM, so make a backup of anything important first.

Use a Phillips #0 to remove the plate over the back-up battery.

![Step 4: back-up battery plate](/img/disassembly/step-4-0.png)
![Step 4: unscrew the plate](/img/disassembly/step-4-1.png)
![Step 4: plate removed](/img/disassembly/step-4-2.png)

### Step 5: Remove the back-up battery

Take out the back-up battery with a screwdriver or the spudger.

![Step 5: back-up battery in place](/img/disassembly/step-5-0.png)
![Step 5: back-up battery removed](/img/disassembly/step-5-1.png)

### Step 6: Remove the back cover screws

Use a T6 Torx screwdriver to remove the six screws holding the back cover on.

![Step 6: six Torx screws](/img/disassembly/step-6.png)

### Step 7: Open the case

Work a pick into the seam at the side, then slide it down the edge until the case opens.

![Step 7: pick in the seam](/img/disassembly/step-7-0.png)
![Step 7: slide the pick down the side](/img/disassembly/step-7-1.png)
![Step 7: case open](/img/disassembly/step-7-2.png)

### Step 8: Remove the metal sheet

Remove the metal sheet from inside the case.

![Step 8: the metal foil sheet lining the inside of the case](/img/disassembly/step-8.png)

> **TODO (author):** add a photo, and say how it comes out (clips, screws or just lifts out) and why it has to go.

*Steps 1 to 7 are adapted from iFixit. Step 8 is specific to this mod.*

To reassemble, follow the steps in reverse order.

### Photos from this build

![The back of the calculator with the battery cover on](/img/disassembly/pic-sides.png)
![The battery compartment open, showing four AAA cells, the back-up battery plate, and a green circuit board visible through the opening above](/img/disassembly/pic-battery.png)
![Side view with the back cover removed, next to the tool kit](/img/disassembly/pic-spudge.png)

## 2. Parts and wiring

From `pcb/cheating-calc.kicad_sch` (the files for ordering the board assembled are in `pcb/jlcpcb/`, see below):

- 1x Seeed Studio **XIAO ESP32S3 Sense** (camera version). The original schematic names a XIAO ESP32C3. The pin names are the same, but only the S3 Sense has a camera.
- 2x **MMBF170** N-channel MOSFETs (SOT-23). One for the tip line, one for the ring line.
- 4x resistors (the schematic shows them only as `R`; fill in the values you used).
- 1x 4-pin header for the link cable: 5V, GND, TIP, RING.
- A link cable or plug for the calculator's link port. TI-83+/84+ calculators use a 2.5 mm stereo port: check yours before buying.

### Solder the XIAO onto the board

JLCPCB places the two MOSFETs (see [ordering the PCB](/ordering-the-pcb)). You solder the rest: the four resistors, the 4-pin header and the XIAO module. The XIAO sits on the right of the board, with its USB-C port at the board's edge and its antenna lead clipped on.

![The XIAO ESP32S3 soldered onto the board, with the resistors and the 4-pin header](/img/soldering-esp32.png)

### Wire it to the calculator

On the left is the calculator's main board, on the right the back case with the mod board in place. Four wires run from the calculator board to the four pins of the header on the mod board (5V, GND, TIP, RING).

![Wiring diagram: four wires from the calculator board to the mod board's 5V, GND, TIP and RING header](/img/wiring-diagram.png)

> **TODO (author):** add a legend for the colours (which wire is 5V, GND, TIP and RING) and say where each one is soldered on the calculator board.

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
