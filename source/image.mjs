// @ts-check
import { exec } from "child_process";

/**
 * Executes a command using child_process.exec
 * @param {string[]} args - The command and arguments to execute
 * @returns {Promise<void>}
 */
async function magic(args) {
  const command = args.join(" ");
  console.log(`Executing command: ${command}`);
  await new Promise((resolve, reject) => {
    exec(command, (err, stdout, stderr) => {
      if (err) {
        console.error(`Command failed: ${stderr}`);
        reject(new Error(`Command failed: ${stderr}`));
        return;
      }
      console.log(`stdout: ${stdout}`);
      console.log(`stderr: ${stderr}`);
      // @ts-ignore
      resolve();
    });
  });
}

/**
 * Converts an image using ImageMagick
 * @param {string} input - The input file path
 * @param {string} output - The output file path
 */
export async function convertImage(input, output) {
  console.time("resize+interpolate");

  await magic([
    "magick", // Use "magick" instead of "convert"
    input,
    "-resize",
    "96x64^", // Resize the image to fill 96x64 while maintaining aspect ratio
    "-gravity",
    "center",
    "-extent",
    "96x64", // Crop or pad the image to exactly 96x64
    "-type",
    "bilevel",
    "-depth",
    "1",
    "-monochrome",
    "-interpolate",
    "Integer",
    "-filter",
    "point",
    "-colors",
    "2",
    "-monochrome",
    "-flip",
    output,
  ]);
  console.timeEnd("resize+interpolate");
  console.log("output:", output);
}