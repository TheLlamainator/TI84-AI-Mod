import express from "express";
import { OpenAI } from "openai";
import i264 from "image-to-base64";
import * as Jimp from "jimp";

export async function chatgpt() {
  const routes = express.Router();
  const gpt = new OpenAI();

  // simply answer a question
  routes.get("/ask", async (req, res) => {
    const question = req.query.question ?? "";
    if (Array.isArray(question)) {
      res.sendStatus(400);
      return;
    }

    try {
      const result = await gpt.chat.completions.create({
        messages: [
          { role: "system", content: "Do not use emojis." },
          { role: "user", content: question },
        ],
        model: "gpt-4o",
      });

      res.send(result.choices[0]?.message?.content ?? "no response");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  // solve a math equation from an image
  routes.post("/solve", async (req, res) => {
    try {
      const contentType = req.headers["content-type"];
      console.log("Content-Type:", contentType);

      if (contentType !== "image/jpeg") {
        res.status(400).send(`Bad content-type: ${contentType}`);
        return;
      }

      // Read image data
      const image_data = await Jimp.read(req.body);
      const image_path = "./to_solve.jpg";
      await image_data.writeAsync(image_path);

      // Convert image to Base64
      const encoded_image = await i264(image_path);
      console.log("Encoded Image: ", encoded_image.length, "bytes");
      console.log(encoded_image.substring(0, 100));

      // Generate question
      const question_number = req.query.n ?? "";
      const question = question_number
        ? `What is the answer to question ${question_number}?`
        : "What is the answer to this question?";

      console.log("Prompt:", question);

      // Send request to OpenAI
      const result = await gpt.chat.completions.create({
        messages: [
          {
            role: "system",
            content:
              "You are a helpful math tutor. Answer as accurately and concisely as possible.",
          },
          {
            role: "user",
            content: [
              { type: "text", text: `${question} Do not explain the answer.` },
              {
                type: "image_url",
                image_url: {
                  url: `data:image/jpeg;base64,${encoded_image}`,
                  detail: "high",
                },
              },
            ],
          },
        ],
        model: "gpt-4o",
      });

      res.send(result.choices[0]?.message?.content ?? "no response");
    } catch (e) {
      console.error(e);
      res.sendStatus(500);
    }
  });

  return routes;
}