import express from "express";
import { readFileSync, writeFileSync } from "fs";
import lodash from "lodash";

// Service to ask ChatGPT questions and get responses under 100 characters.

let rooms = {};

const chatFile = "chat.json";

function updateChat(f) {
  writeFileSync(chatFile, JSON.stringify(rooms, null, 2));
  const current = JSON.parse(readFileSync(chatFile, "ascii"));
  rooms = f(current) ?? rooms;
  writeFileSync(chatFile, JSON.stringify(rooms));
}

function formatChat(n) {
  return rooms[n]?.flatMap((x) => lodash.chunk(x, 16).map((x) => x.join("").padEnd(16, " "))) ?? [];
}

const list_len = 7;

export async function chat() {
  const routes = express.Router();

  routes.get("/messages", (req, res) => {
    const page = Number.parseInt(req.query.p ?? 0);
    if (Number.isNaN(page)) {
      res.sendStatus(400);
      return;
    }
    const c = Number.parseInt(req.query.c);
    if (Number.isNaN(c)) {
      res.sendStatus(400);
      return;
    }

    const messages = formatChat(c);
    const p = Math.max(Math.ceil(messages.length / list_len - 1 - page), 0);

    if (p * list_len > messages.length) {
      res.send("");
      return;
    }

    const rangeEnd = Math.min((p + 1) * list_len, messages.length);
    const rangeStart = Math.max(rangeEnd - list_len, 0);

    const chatLines = messages.slice(rangeStart, rangeEnd);

    console.log(chatLines);

    res.send(chatLines.join(""));
  });

  routes.get("/send", (req, res) => {
    const msg = req.query.m;
    console.log({ msg });
    if (typeof msg !== "string") {
      res.sendStatus(400);
      return;
    }
    const c = Number.parseInt(req.query.c);
    if (Number.isNaN(c)) {
      res.sendStatus(400);
      return;
    }

    const id = req.query.id.substring(0, 3);

    updateChat((k) => {
      k[c] ??= [];
      k[c].push(`${id}:${msg}`);
      return k;
    });

    res.sendStatus(200);
  });

      routes.get("/chat-ui", (req, res) => {
        res.send(`
          <!DOCTYPE html>
          <html lang="en">
          <head>
            <meta charset="UTF-8">
            <meta name="viewport" content="width=device-width, initial-scale=1.0">
            <title>Chat UI</title>
            <style>
              body { font-family: Arial, sans-serif; padding: 20px; }
              #chat-box { width: 100%; height: 300px; overflow-y: scroll; border: 1px solid #ddd; padding: 10px; white-space: pre-wrap; }
              #message { width: 80%; padding: 5px; }
              #send { padding: 5px; }
            </style>
          </head>
          <body>
            <h2>Simple Chat</h2>
            <div id="chat-box"></div>
            <input type="text" id="message" placeholder="Type a message..." />
            <button id="send">Send</button>
      
            <script>
              const chatBox = document.getElementById("chat-box");
              const messageInput = document.getElementById("message");
              const sendButton = document.getElementById("send");
              const chatRoom = 0; // Change this as needed
              const userId = "PHN"; // Change this as needed
      
              function fetchMessages() {
                fetch(\`/chats/messages?c=\${chatRoom}&p=0\`)
                  .then(response => {
                    if (!response.ok) {
                      throw new Error('Network response was not ok');
                    }
                    return response.text(); // Get the response as text
                  })
                  .then(text => {
                    console.log('Response text:', text); // Log the response text
                    const messages = text.split(/(CLC:|PHN:)/).filter(Boolean); // Split the text by 'CLC:' or 'PHN:' and filter out empty strings
                    chatBox.innerHTML = ""; // Clear the chat box
                    for (let i = 1; i < messages.length; i += 2) {
                      const prefix = messages[i - 1];
                      const message = messages[i].trim();
                      console.log('Message:', prefix + message); // Log each message
                      const messageDiv = document.createElement("div");
                      messageDiv.textContent = prefix + message; // Add the prefix back to each message and trim any extra spaces
                      chatBox.appendChild(messageDiv);
                    }
                  })
                  .catch(error => {
                    console.error('There was a problem with the fetch operation:', error);
                  });
              }
      
              sendButton.addEventListener("click", () => {
                const msg = messageInput.value.trim();
                if (!msg) return;
      
                fetch(\`/chats/send?c=\${chatRoom}&m=\${encodeURIComponent(msg)}&id=\${userId}\`)
                  .then(response => {
                    if (!response.ok) {
                      throw new Error('Network response was not ok');
                    }
                    messageInput.value = "";
                    fetchMessages();
                  })
                  .catch(error => {
                    console.error('There was a problem with the fetch operation:', error);
                  });
              });
      
              fetchMessages();
              setInterval(fetchMessages, 3000); // Refresh messages every 3 seconds
            </script>
          </body>
          </html>
        `);
    });
  return routes;
}
