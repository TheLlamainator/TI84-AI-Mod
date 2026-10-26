# Ordering the PCB

> **Files to download:** [PCB files](/downloads#_1-the-pcb)

The level-shifter board is made and assembled by **[JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID)**. Everything you need to upload is in [`pcb/jlcpcb/`](https://github.com/TheLlamainator/TI84-AI-Mod/tree/main/pcb/jlcpcb).

Order here: **[jlcpcb.com](https://jlcpcb.com/?from=UNWIJQWZMTXVHID)**

> That is the author's referral link. It may give the author a small reward at no extra cost to you. Ordering from jlcpcb.com directly works exactly the same, and the files are the same either way.

## What to upload

| Step on [JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID) | File | Where |
| --- | --- | --- |
| Gerber files (the PCB itself) | `cheating-calc-gerbers.zip` | `pcb/jlcpcb/` |
| Assembly: BOM | `BOM.csv` | `pcb/jlcpcb/` |
| Assembly: CPL (component placement list) | `CPL.csv` | `pcb/jlcpcb/` |

Upload the zip **as it is**. Don't unzip it.

## Steps

[JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID) changes its website from time to time, so the menu names may differ slightly. The settings below are what the files were prepared for.

1. Go to the [JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID) site through the link above and sign in or create an account.
2. Start a new order and upload `cheating-calc-gerbers.zip` as the Gerber file. The site reads the board and shows a preview. The board is about **72.4 x 22.4 mm**.
3. Settings: **2 layers**, **1.6 mm** thickness. The other options (colour, surface finish) don't affect whether it works. Choose what you like.
4. Turn on **PCB Assembly** and set it to the **top** side.
5. At the assembly step, upload `BOM.csv` and `CPL.csv`.
6. Check the **parts list** the site shows. It should contain one part: the **MMBF170** MOSFET (LCSC part **C130879**), used twice, as Q1 and Q2.
7. Check the **placement preview**. The two MOSFETs should sit next to each other near the resistor pads, with their pins on the pads. `CPL.csv` measures positions from the board's lower-left corner, which is what [JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID)'s assembly expects. If they ever look turned the wrong way, correct the rotation in [JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID)'s tool. Their part library can use a different zero angle for SOT-23 than KiCad does.
8. Finish the order.

## What you still have to solder yourself

[JLCPCB](https://jlcpcb.com/?from=UNWIJQWZMTXVHID) only places the two SMD MOSFETs. You add the rest by hand when the board arrives:

- 4x through-hole pull-up resistors (R1 to R4). See the parts list in [wiring](/wiring) for the value.
- The 4-pin header (J1) for the link cable.
- The **XIAO ESP32S3 Sense** module (U1).

## The files, in case you want to check them

- `cheating-calc-gerbers.zip` contains the 14 gerber and drill files exported from KiCad. The same files are unzipped in `pcb/0.1/`.
- `BOM.csv` has one line: `MMBF170, "Q1,Q2", SOT-23, C130879`.
- `CPL.csv` places Q1 and Q2 on the top side at 180 degrees, with X and Y measured from the board's lower-left corner.
- The KiCad project is in `pcb/` if you want to change the board and export new files.
