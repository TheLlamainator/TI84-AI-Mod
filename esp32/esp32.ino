// Project: TI-32 v0.1
// Author:  ChromaLock
// Date:    2024

#include "./secrets.h"
#include "./launcher.h"
#include <TICL.h>
#include <CBL2.h>
#include <TIVar.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiClient.h>
#include <HTTPClient.h>
#include <UrlEncode.h>
#include <Preferences.h>
#include <HTTPUpdate.h>
#include <esp_ota_ops.h>

#define CAMERA

#ifdef CAMERA
#include <esp_camera.h>
#define CAMERA_MODEL_XIAO_ESP32S3
#include "./camera_pins.h"
#include "./camera_index.h"
#endif

constexpr auto TIP = D1;
constexpr auto RING = D10;
constexpr auto MAXHDRLEN = 16;
constexpr auto MAXDATALEN = 4096;
constexpr auto MAXARGS = 5;
constexpr auto MAXSTRARGLEN = 256;
// Firmware build number. Bump it for every build published to the server (SETTINGS > FIRMWARE compares it
// with the number on the server and only updates when the server's is higher).
#define FW_VERSION 6
#ifndef OTA_TOKEN
#define OTA_TOKEN "" // set in secrets.h; wireless updates are refused without it
#endif

constexpr auto PICSIZE = 756;
constexpr auto PICVARSIZE = PICSIZE + 2;
constexpr auto PASSWORD = 69420;

CBL2 cbl;
Preferences prefs;

// whether or not the user has entered the password
bool unlocked = false;

// Arguments
int currentArg = 0;
char strArgs[MAXARGS][MAXSTRARGLEN];
double realArgs[MAXARGS];

// the command to execute
int command = -1;
// whether or not the operation has completed
bool status = 0;
// whether or not the operation failed
bool error = 0;
// error or success message
char message[MAXSTRARGLEN];
// list data
constexpr auto LISTLEN = 256;
constexpr auto LISTENTRYLEN = 20;
char list[LISTLEN][LISTENTRYLEN];
// http response
constexpr auto MAXHTTPRESPONSELEN = 4096;
char response[MAXHTTPRESPONSELEN];
// image variable (96x63)
uint8_t frame[PICVARSIZE] = {PICSIZE & 0xff, PICSIZE >> 8};
String fullResponse;  // the whole conversation, sent back to the AI as context
String displayText;   // only the newest answer: this is what the calculator pages through
const int PAGE_SIZE = 100;
int PAGE_PAGE = 0; 

void connect();
void disconnect();
void gpt();
void send();
void launcher();
void snap();
void fwupdate();
void solve();
void image_list();
void fetch_image();
void fetch_chats();
void send_chat();
void program_list();
void fetch_program();
void sendPage();
void reply();
void clearChat(); 

struct Command
{
  int id;
  const char *name;
  int num_args;
  void (*command_fp)();
  bool wifi;
};

struct Command commands[] = {
    {0, "connect", 0, connect, false},
    {1, "disconnect", 0, disconnect, false},
    {2, "gpt", 1, gpt, true},
    {4, "send", 2, send, true},
    {5, "launcher", 0, launcher, false},
    {7, "snap", 0, snap, false},
    {8, "solve", 1, solve, true},
    {9, "image_list", 1, image_list, true},
    {10, "fetch_image", 1, fetch_image, true},
    {11, "fetch_chats", 2, fetch_chats, true},
    {12, "send_chat", 2, send_chat, true},
    {13, "program_list", 1, program_list, true},
    {14, "fetch_program", 1, fetch_program, true},
    { 15, "sendPage", 1, sendPage, true },
    { 16, "reply", 1, reply, true },
    { 17, "clearChat", 1, clearChat, true}, 
    { 18, "fwupdate", 0, fwupdate, false },
};

constexpr int NUMCOMMANDS = sizeof(commands) / sizeof(struct Command);
constexpr int MAXCOMMAND = 18; // highest command id in the table above

uint8_t header[MAXHDRLEN];
uint8_t data[MAXDATALEN];

// lowercase letters make strings weird,
// so we have to truncate the string
void fixStrVar(char *str)
{
  int end = strlen(str);
  for (int i = 0; i < end; ++i)
  {
    if (isLowerCase(str[i]))
    {
      --end;
    }
  }
  str[end] = '\0';
}

int onReceived(uint8_t type, enum Endpoint model, int datalen);
int onRequest(uint8_t type, enum Endpoint model, int *headerlen,
              int *datalen, data_callback *data_callback);

void startCommand(int cmd)
{
  command = cmd;
  status = 0;
  error = 0;
  currentArg = 0;
  for (int i = 0; i < MAXARGS; ++i)
  {
    memset(&strArgs[i], 0, MAXSTRARGLEN);
    realArgs[i] = 0;
  }
  strncpy(message, "no command", MAXSTRARGLEN);
}

void setError(const char *err)
{
  Serial.print("ERROR: ");
  Serial.println(err);
  error = 1;
  status = 1;
  command = -1;
  strncpy(message, err, MAXSTRARGLEN);
}

void setSuccess(const char *success)
{
  Serial.print("SUCCESS: ");
  Serial.println(success);
  error = 0;
  status = 1;
  command = -1;
  strncpy(message, success, MAXSTRARGLEN);
}

int sendProgramVariable(const char *name, uint8_t *program, size_t variableSize);

bool camera_sign = false;

// Keep a freshly updated firmware "pending" until setup() has finished (see esp_ota_mark_app_valid_cancel_rollback
// at the end of setup). If the new firmware crashes before then, the bootloader goes back to the previous one.
extern "C" bool verifyRollbackLater()
{
  return true;
}

void setup()
{
  Serial.begin(115200);
   Serial.println("delay");
    delay(2000); 
  Serial.println("[CBL]");
  delay(1000);

  cbl.setLines(TIP, RING);
  cbl.resetLines();
  cbl.setupCallbacks(header, data, MAXDATALEN, onReceived, onRequest);
  // cbl.setVerbosity(true, (HardwareSerial *)&Serial);

  pinMode(TIP, INPUT);
  pinMode(RING, INPUT);

  Serial.println("[preferences]");
  prefs.begin("ccalc", false);
  auto reboots = prefs.getUInt("boots", 0);
  Serial.print("reboots: ");
  Serial.println(reboots);
  prefs.putUInt("boots", reboots + 1);
  prefs.end();

#ifdef CAMERA
  Serial.println("[camera]");

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_UXGA;
  // this needs to be pixformat grayscale in the future
  config.pixel_format = PIXFORMAT_GRAYSCALE; // the calculator screen is 1-bit, so grayscale is all we need
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if (config.pixel_format == PIXFORMAT_JPEG)
  {
    if (psramFound())
    {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    }
    else
    {
      // Limit the frame size when PSRAM is not available
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  }
  else
  {
    // Best option for face detection/recognition
    config.frame_size = FRAMESIZE_240X240;
#if CONFIG_IDF_TARGET_ESP32S3
    config.fb_count = 2;
#endif
  }

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    // No camera board attached (or it failed to start): carry on, only the camera commands are unavailable.
    Serial.printf("Camera init failed with error 0x%x (continuing without the camera)\n", err);
  }
  else
  {
    Serial.println("camera ready");
    camera_sign = true; // Camera initialization check passes

    sensor_t *s = esp_camera_sensor_get();
    // enable grayscale
    s->set_special_effect(s, 2);
  }
#endif

  strncpy(message, "default message", MAXSTRARGLEN);
  delay(100);
  memset(data, 0, MAXDATALEN);
  memset(header, 0, 16);
  Serial.println("[ready]");
  Serial.print("firmware build ");
  Serial.println(FW_VERSION);

  // The new firmware got this far, so confirm it. This cancels the automatic rollback.
  esp_ota_mark_app_valid_cancel_rollback();
}

void (*queued_action)() = NULL;

void loop()
{
  if (queued_action)
  {
    // dont ask me why you need this, but it fails otherwise.
    // probably relates to a CBL2 timeout thing?
    delay(1000);
    Serial.println("executing queued actions");
    // dont ask me
    void (*tmp)() = queued_action;
    queued_action = NULL;
    tmp();
  }
  if (command >= 0 && command <= MAXCOMMAND)
  {
    for (int i = 0; i < NUMCOMMANDS; ++i)
    {
      if (commands[i].id == command && commands[i].num_args == currentArg)
      {
        if (commands[i].wifi && !WiFi.isConnected())
        {
          setError("wifi not connected");
        }
        else
        {
          Serial.print("processing command: ");
          Serial.println(commands[i].name);
          commands[i].command_fp();
        }
      }
    }
  }
  cbl.eventLoopTick();
}

int onReceived(uint8_t type, enum Endpoint model, int datalen)
{
  char varName = header[3];

  Serial.print("unlocked: ");
  Serial.println(unlocked);

  // check for password
  if (!unlocked && varName == 'P')
  {
    auto password = TIVar::realToLong8x(data, model);
    if (password == PASSWORD)
    {
      Serial.println("successful unlock");
      unlocked = true;
      return 0;
    }
    else
    {
      Serial.println("failed unlock");
    }
  }

  if (!unlocked)
  {
    return -1;
  }

  // check for command
  if (varName == 'C')
  {
    if (type != VarTypes82::VarReal)
    {
      return -1;
    }
    int cmd = TIVar::realToLong8x(data, model);
    if (cmd >= 0 && cmd <= MAXCOMMAND)
    {
      Serial.print("command: ");
      Serial.println(cmd);
      startCommand(cmd);
      return 0;
    }
    else
    {
      Serial.print("invalid command: ");
      Serial.println(cmd);
      return -1;
    }
    
  }
      if (varName == 'V') {
    if (type != VarTypes82::VarReal) {
      return -1;
    }
    PAGE_PAGE = TIVar::realToLong8x(data, model);
    Serial.print("Received page number: ");
    Serial.println(PAGE_PAGE);
    if (command >= 0)
    {
      // A command (for example a GPT reply) has been received but has not run yet. The calculator asks for the
      // page straight after sending it, and answering that here would mark the command finished and cancel it.
      // The command sends the page itself when it completes.
      Serial.println("page request while a command is pending: leaving it to the command");
      return 0;
    }
    sendPage();
    return 0;
  }

  if (varName == 'X') {
    Serial.println("Recieved var X");
    if (type != VarTypes82::VarReal) {
      Serial.println("var x not equal vartypes82:varreal");
      return -1;

    }
    Serial.println("Reset fullResponse to empty");
    fullResponse = "";
    displayText = "";
    
    return 0;
  }

  if (currentArg >= MAXARGS)
  {
    Serial.println("argument overflow");
    setError("argument overflow");
    return -1;
  }

  switch (type)
  {
  case VarTypes82::VarString:
    Serial.print("len: ");
    strncpy(strArgs[currentArg++], TIVar::strVarToString8x(data, model).c_str(), MAXSTRARGLEN);
    fixStrVar(strArgs[currentArg - 1]);
    Serial.print("Str");
    Serial.print(currentArg - 1);
    Serial.print(" ");
    Serial.println(strArgs[currentArg - 1]);
    break;
  case VarTypes82::VarReal:
    realArgs[currentArg++] = TIVar::realToFloat8x(data, model);
    Serial.print("Real");
    Serial.print(currentArg - 1);
    Serial.print(" ");
    Serial.println(realArgs[currentArg - 1]);
    break;
  default:
    // maybe set error here?
    return -1;
  }
  return 0;
}

#ifdef CAMERA
// Flip the picture if it comes out upside down or mirrored on your board (set to 1).
#define CAM_VFLIP 0
#define CAM_HMIRROR 0

// Whether Get(Pic1) should take a fresh photo each time (live view) instead of sending the stored frame.
bool liveCamera = false;

// Take a photo and convert it to the calculator picture format: 96 x 63 pixels, 1 bit each, 12 bytes per row,
// top row first, most significant bit first, 1 = black. This is the same layout the server images use.
// The picture is averaged down from the camera frame, contrast-stretched, then dithered with a 4x4 Bayer matrix.
bool capturePic(bool flushStale = true)
{
  static uint8_t gray[64 * 96];
  static const uint8_t bayer[4][4] = {{0, 8, 2, 10}, {12, 4, 14, 6}, {3, 11, 1, 9}, {15, 7, 13, 5}};

  camera_fb_t *fb = nullptr;
  if (flushStale)
  {
    // first shot: throw away a possibly stale buffered frame. Live frames skip this (the camera hands back its latest frame).
    fb = esp_camera_fb_get();
    if (fb)
      esp_camera_fb_return(fb);
  }
  fb = esp_camera_fb_get();
  if (!fb || fb->format != PIXFORMAT_GRAYSCALE)
  {
    if (fb)
      esp_camera_fb_return(fb);
    return false;
  }

  const int sw = fb->width, sh = fb->height;
  // centre crop with the calculator's 96:64 shape
  int cropH = sw * 64 / 96;
  int cropW = sw;
  if (cropH > sh)
  {
    cropH = sh;
    cropW = sh * 96 / 64;
  }
  const int x0 = (sw - cropW) / 2, y0 = (sh - cropH) / 2;

  uint8_t lo = 255, hi = 0;
  for (int y = 0; y < 64; ++y)
  {
    int ya = y0 + y * cropH / 64, yb = y0 + (y + 1) * cropH / 64;
    if (yb <= ya)
      yb = ya + 1;
    for (int x = 0; x < 96; ++x)
    {
      int xa = x0 + x * cropW / 96, xb = x0 + (x + 1) * cropW / 96;
      if (xb <= xa)
        xb = xa + 1;
      uint32_t sum = 0;
      int n = 0;
      for (int yy = ya; yy < yb; ++yy)
        for (int xx = xa; xx < xb; ++xx, ++n)
          sum += fb->buf[yy * sw + xx];
      uint8_t v = n ? sum / n : 0;
      gray[y * 96 + x] = v;
      if (v < lo)
        lo = v;
      if (v > hi)
        hi = v;
    }
  }
  esp_camera_fb_return(fb);

  const int range = max(1, (int)hi - (int)lo);
  // The calculator picture holds rows 1..63 of the 64-row image (row 0 is dropped, as the image tool does).
  memset(frame + 2, 0, PICSIZE);
  for (int row = 0; row < 63; ++row)
  {
    int sy = CAM_VFLIP ? (62 - row) + 1 : row + 1;
    for (int x = 0; x < 96; ++x)
    {
      int sx = CAM_HMIRROR ? 95 - x : x;
      int stretched = ((int)gray[sy * 96 + sx] - lo) * 255 / range;
      int threshold = bayer[row & 3][x & 3] * 16 + 8;
      if (stretched < threshold) // dark pixel -> 1 bit
        frame[2 + row * 12 + (x >> 3)] |= (0x80 >> (x & 7));
    }
  }
  return true;
}
#endif

uint8_t frameCallback(int idx)
{
  return frame[idx];
}

char varIndex(int idx)
{
  return '0' + (idx == 9 ? 0 : (idx + 1));
}

int onRequest(uint8_t type, enum Endpoint model, int *headerlen, int *datalen, data_callback *data_callback)
{
  char varName = header[3];
  char strIndex = header[4];
  char strname[5] = {'S', 't', 'r', varIndex(strIndex), 0x00};
  char picname[5] = {'P', 'i', 'c', varIndex(strIndex), 0x00};
  Serial.print("request for ");
  Serial.println(varName == 0xaa ? strname : varName == 0x60 ? picname
                                                             : (const char *)&header[3]);
  memset(header, 0, sizeof(header));
  switch (varName)
  {
  case 0x60:
    if (type != VarTypes82::VarPic)
    {
      return -1;
    }
#ifdef CAMERA
    if (liveCamera && camera_sign)
      capturePic(false);
#endif
    *datalen = PICVARSIZE;
    TIVar::intToSizeWord(*datalen, &header[0]);
    header[2] = VarTypes82::VarPic;
    header[3] = 0x60;
    header[4] = strIndex;
    *data_callback = frameCallback;
    break;
  case 0xAA:
    if (type != VarTypes82::VarString)
    {
      return -1;
    }
    // TODO right now, the only string variable will be the message, but ill need to allow for other vars later
    *datalen = TIVar::stringToStrVar8x(String(message), data, model);
    TIVar::intToSizeWord(*datalen, header);
    header[2] = VarTypes82::VarString;
    header[3] = 0xAA;
    // send back as same variable that was requested
    header[4] = strIndex;
    *headerlen = 13;
    break;
  case 'E':
    if (type != VarTypes82::VarReal)
    {
      return -1;
    }
    *datalen = TIVar::longToReal8x(error, data, model);
    TIVar::intToSizeWord(*datalen, header);
    header[2] = VarTypes82::VarReal;
    header[3] = 'E';
    header[4] = '\0';
    *headerlen = 13;
    break;
  case 'S':
    if (type != VarTypes82::VarReal)
    {
      return -1;
    }
    *datalen = TIVar::longToReal8x(status, data, model);
    TIVar::intToSizeWord(*datalen, header);
    header[2] = VarTypes82::VarReal;
    header[3] = 'S';
    header[4] = '\0';
    *headerlen = 13;
    break;
  default:
    return -1;
  }
  return 0;
}

int makeRequest(String url, char *result, int resultLen, size_t *len)
{
  memset(result, 0, resultLen);

#ifdef SECURE
  WiFiClientSecure client;
  client.setInsecure();
#else
  WiFiClient client;
#endif
  HTTPClient http;
  http.setAuthorization(HTTP_USERNAME, HTTP_PASSWORD);

  Serial.println(url);
  http.begin(client, url.c_str());

  // Send HTTP GET request
  int httpResponseCode = http.GET();
  Serial.print(url);
  Serial.print(" ");
  Serial.println(httpResponseCode);

  int responseSize = http.getSize();
  WiFiClient *httpStream = http.getStreamPtr();

  Serial.print("response size: ");
  Serial.println(responseSize);

  if (httpResponseCode != 200)
  {
    return httpResponseCode;
  }

  if (httpStream->available() > resultLen)
  {
    Serial.print("response size: ");
    Serial.print(httpStream->available());
    Serial.println(" is too big");
    return -1;
  }

  while (httpStream->available())
  {
    *(result++) = httpStream->read();
  }
  *len = responseSize;

  http.end();

  return 0;
}

void connect()
{
  struct KnownNetwork
  {
    const char *ssid;
    const char *pass;
  };
  static const KnownNetwork known[] = {WIFI_NETWORKS};
  const int numKnown = min((int)(sizeof(known) / sizeof(known[0])), 8);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  // Scan, then try the known networks that are in range, strongest signal first.
  // The networks that ARE visible are printed so a name that does not match exactly (iPhones often use a curly
  // apostrophe) is easy to spot.
  Serial.println("scanning...");
  int found = WiFi.scanNetworks();
  int rssi[8];
  bool visible[8] = {false};
  bool tried[8] = {false};
  for (int k = 0; k < 8; ++k)
    rssi[k] = -1000;
  for (int i = 0; i < found; ++i)
  {
    Serial.print("  saw: ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.println(" dBm)");
    for (int k = 0; k < numKnown; ++k)
      if (WiFi.SSID(i) == String(known[k].ssid) && WiFi.RSSI(i) > rssi[k])
      {
        visible[k] = true;
        rssi[k] = WiFi.RSSI(i);
      }
  }
  WiFi.scanDelete();

  bool anyVisible = false;
  bool wrongPassword = false;
  for (int round = 0; round < numKnown; ++round)
  {
    int best = -1;
    for (int k = 0; k < numKnown; ++k)
      if (visible[k] && !tried[k] && (best < 0 || rssi[k] > rssi[best]))
        best = k;
    if (best < 0)
      break;
    tried[best] = true;
    anyVisible = true;

    Serial.print("trying: ");
    Serial.println(known[best].ssid);
    WiFi.begin(known[best].ssid, known[best].pass);
    unsigned long start = millis();
    wl_status_t last = WL_IDLE_STATUS;
    while (WiFi.status() != WL_CONNECTED)
    {
      wl_status_t st = WiFi.status();
      if (st != last)
      {
        Serial.print("wifi status: ");
        Serial.println((int)st);
        last = st;
      }
      if (st == WL_CONNECT_FAILED)
      {
        wrongPassword = true;
        break;
      }
      if (millis() - start > 15000)
        break;
      delay(100);
    }
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.print("connected to ");
      Serial.print(known[best].ssid);
      Serial.print(", IP: ");
      Serial.println(WiFi.localIP());
      setSuccess("connected");
      return;
    }
    WiFi.disconnect(true);
    delay(100);
  }

  if (!anyVisible)
    setError("ssid not found");
  else if (wrongPassword)
    setError("wrong password");
  else
    setError("connect timed out");
}

void disconnect()
{
  WiFi.disconnect(true);
  setSuccess("disconnected");
}

void clearChat() {
  fullResponse = "";
  displayText = "";
}

void reply() {
  Serial.println("Reply action initiated");
  const char* userReply = strArgs[0];
  Serial.print("prompt: ");
  Serial.println(userReply);
  // Append the user's reply to the existing conversation
  fullResponse += "| User: " + String(userReply) + "| AI: ";
  Serial.print("full response: ");
  Serial.println(fullResponse);
  // Send the updated conversation to the server
  auto url = String(SERVER) + String("/gpt/ask?question=") + urlEncode(fullResponse);
  Serial.println("made url");
  Serial.println(url);

  size_t realsize = 0;
  Serial.println("sending request");
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("error making request");
    return;
  }
  Serial.println("request recieved");

  // Update fullResponse with the new AI response
  fullResponse += String(response);
  displayText = String(response);  // show the new answer from its first page, not the start of the chat

  PAGE_PAGE = 0;  // Reset to first page
  sendPage();
}

void gpt() {
  const char* prompt = strArgs[0];
  Serial.print("prompt: ");
  Serial.println(prompt);

  fullResponse = "User: " + String(prompt) + " | AI: ";

  auto url = String(SERVER) + String("/gpt/ask?question=") + urlEncode(String(prompt));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize)) {
    setError("error making request");
    return;
  }

  fullResponse += String(response);
  displayText = String(response);
  Serial.print("Full response: ");
  Serial.println(fullResponse);

  PAGE_PAGE = 0; 
  sendPage();
}

void sendPage() {
  int start = PAGE_PAGE * PAGE_SIZE;
  String pageContent = displayText.substring(start, min(start + PAGE_SIZE, (int)displayText.length()));
  
  strncpy(message, pageContent.c_str(), MAXSTRARGLEN);
  setSuccess(message);
}

void send()
{
  const char *recipient = strArgs[0];
  const char *message = strArgs[1];
  Serial.print("sending \"");
  Serial.print(message);
  Serial.print("\" to \"");
  Serial.print(recipient);
  Serial.println("\"");
  setSuccess("OK: sent");
}

// The newest launcher downloaded from the server (SETTINGS > UPDATE)
static uint8_t launcherDownload[4096];

// Download url into buf in one go. Returns 0 on success; *len is the number of bytes read.
int downloadBytes(const String &url, uint8_t *buf, size_t cap, size_t *len)
{
#ifdef SECURE
  WiFiClientSecure client;
  client.setInsecure();
#else
  WiFiClient client;
#endif
  HTTPClient http;
  http.setAuthorization(HTTP_USERNAME, HTTP_PASSWORD);
  http.setTimeout(8000);
  if (!http.begin(client, url.c_str()))
    return -1;
  int code = http.GET();
  if (code != 200)
  {
    http.end();
    return code ? code : -1;
  }
  int size = http.getSize();
  if (size <= 2 || (size_t)size > cap)
  {
    http.end();
    return -2;
  }
  WiFiClient *stream = http.getStreamPtr();
  stream->setTimeout(8000);
  size_t got = stream->readBytes(buf, size); // waits for the whole body, not just what has arrived so far
  http.end();
  if ((int)got != size)
    return -3;
  *len = got;
  return 0;
}

// ---- wireless firmware update --------------------------------------------------------------------------------

// Ask the server which firmware build it has. Returns 0 on success.
int fetchServerFirmwareVersion(int *version)
{
  WiFiClient client;
  HTTPClient http;
  http.setAuthorization("ota", OTA_TOKEN);
  http.setTimeout(8000);
  if (!http.begin(client, String(SERVER) + String("/firmware/version")))
    return -1;
  int code = http.GET();
  if (code != 200)
  {
    http.end();
    return code ? code : -1;
  }
  String body = http.getString();
  http.end();
  *version = body.toInt();
  return *version > 0 ? 0 : -2;
}

// Download the firmware and flash it into the spare slot, then reboot into it. The running firmware is left
// alone until the whole download has been written and checked, so a failed download changes nothing.
void _doFirmwareUpdate()
{
  // Give the calculator a few seconds to fetch the "updating" message before this blocks and reboots.
  delay(4000);
  Serial.println("firmware update: downloading");
  WiFiClient client;
  httpUpdate.setAuthorization("ota", OTA_TOKEN);
  httpUpdate.rebootOnUpdate(true);
  httpUpdate.onProgress([](int done, int total) {
    static int lastPct = -10;
    int pct = total ? (done * 100 / total) : 0;
    if (pct >= lastPct + 10)
    {
      lastPct = pct;
      Serial.print("firmware update: ");
      Serial.print(pct);
      Serial.println("%");
    }
  });
  t_httpUpdate_return ret = httpUpdate.update(client, String(SERVER) + String("/firmware/latest.bin"));
  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.print("firmware update failed: ");
    Serial.println(httpUpdate.getLastErrorString());
    break;
  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("firmware update: server had nothing newer");
    break;
  case HTTP_UPDATE_OK:
    Serial.println("firmware update done, rebooting"); // normally rebooted already
    break;
  }
}

// SETTINGS > FIRMWARE. Quick checks here so the calculator gets an answer, then the real download runs after the
// reply has been sent (same pattern as the launcher transfer).
void fwupdate()
{
  if (strlen(OTA_TOKEN) < 16)
  {
    setError("no update password set");
    return;
  }
  if (!WiFi.isConnected())
  {
    setError("not connected, use CONNECT first");
    return;
  }
  int latest = 0;
  int rc = fetchServerFirmwareVersion(&latest);
  if (rc != 0)
  {
    Serial.print("firmware version check failed: ");
    Serial.println(rc);
    setError(rc == 401 ? "update password rejected" : "cannot reach update server");
    return;
  }
  Serial.print("firmware: running ");
  Serial.print(FW_VERSION);
  Serial.print(", server has ");
  Serial.println(latest);
  if (latest <= FW_VERSION)
  {
    char msg[48];
    snprintf(msg, sizeof(msg), "up to date: build %d", FW_VERSION);
    setSuccess(msg);
    return;
  }
  queued_action = _doFirmwareUpdate;
  setSuccess("updating, wait 1 minute and do not unplug");
}

void _sendLauncher()
{
  // Prefer the newest launcher from the server so the program can change without re-flashing the board.
  // If WiFi or the server is unavailable, fall back to the copy built into the firmware.
  if (!WiFi.isConnected())
    connect();
  if (WiFi.isConnected())
  {
    size_t len = 0;
    int rc = downloadBytes(String(SERVER) + String("/launcher"), launcherDownload, sizeof(launcherDownload), &len);
    if (rc == 0 && len > 2 && (launcherDownload[0] | (launcherDownload[1] << 8)) == (int)len - 2)
    {
      Serial.println("sending launcher downloaded from the server");
      sendProgramVariable("TI32", launcherDownload, len);
      return;
    }
    Serial.print("server launcher unavailable (");
    Serial.print(rc);
    Serial.println("), using the built-in copy");
  }
  else
  {
    Serial.println("no WiFi, using the built-in launcher");
  }
  sendProgramVariable("TI32", __launcher_var, __launcher_var_len);
}

void launcher()
{
  // we have to queue this action, since otherwise the transfer fails
  // due to the CBL2 library still using the lines
  queued_action = _sendLauncher;
  setSuccess("queued transfer");
}

void snap()
{
#ifdef CAMERA
  if (!camera_sign)
  {
    setError("camera failed to initialize");
    return;
  }
  if (!capturePic())
  {
    setError("capture failed");
    return;
  }
  liveCamera = true; // later Get(Pic1) requests take a fresh photo each time
  setSuccess("photo taken");
#else
  setError("pictures not supported");
#endif
}

void solve()
{
#ifdef CAMERA
  if (!camera_sign)
  {
    setError("camera failed to initialize");
  }
#else
  setError("pictures not supported");
#endif
}

void image_list()
{
  int page = realArgs[0];
  auto url = String(SERVER) + String("/image/list?p=") + urlEncode(String(page));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize))
  {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void fetch_image()
{
#ifdef CAMERA
  liveCamera = false;
#endif
  memset(frame + 2, 0, 756);
  // fetch image and put it into the frame variable
  int id = realArgs[0];
  Serial.print("id: ");
  Serial.println(id);

  auto url = String(SERVER) + String("/image/get?id=") + urlEncode(String(id));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXHTTPRESPONSELEN, &realsize))
  {
    setError("error making request");
    return;
  }

  if (realsize != PICSIZE)
  {
    Serial.print("response size:");
    Serial.println(realsize);
    setError("bad image size");
    return;
  }

  // load the image
  frame[0] = realsize & 0xff;
  frame[1] = (realsize >> 8) & 0xff;
  memcpy(&frame[2], response, 756);

  setSuccess(response);
}

void fetch_chats()
{
  int room = realArgs[0];
  int page = realArgs[1];
  auto url = String(SERVER) + String("/chats/messages?p=") + urlEncode(String(page)) + String("&c=") + urlEncode(String(room));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize))
  {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void send_chat()
{
  int room = realArgs[0];
  const char *msg = strArgs[1];

  auto url = String(SERVER) +
             String("/chats/send?c=") +
             urlEncode(String(room)) +
             String("&m=") +
             urlEncode(String(msg)) +
             String("&id=") +
             urlEncode(String(CHAT_NAME));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize))
  {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

void program_list()
{
  int page = realArgs[0];
  auto url = String(SERVER) + String("/programs/list?p=") + urlEncode(String(page));

  size_t realsize = 0;
  if (makeRequest(url, response, MAXSTRARGLEN, &realsize))
  {
    setError("error making request");
    return;
  }

  Serial.print("response: ");
  Serial.println(response);

  setSuccess(response);
}

char programName[256];
char programData[4096];
size_t programLength;

void _resetProgram()
{
  memset(programName, 0, 256);
  memset(programData, 0, 4096);
  programLength = 0;
}

void _sendDownloadedProgram()
{
  if (sendProgramVariable(programName, (uint8_t *)programData, programLength))
  {
    Serial.println("failed to transfer requested download");
    Serial.print(programName);
    Serial.print("(");
    Serial.print(programLength);
    Serial.println(")");
  }
  _resetProgram();
}

void fetch_program()
{
  int id = realArgs[0];
  Serial.print("id: ");
  Serial.println(id);

  _resetProgram();

  auto url = String(SERVER) + String("/programs/get?id=") + urlEncode(String(id));

  if (makeRequest(url, programData, 4096, &programLength))
  {
    setError("error making request for program data");
    return;
  }

  size_t realsize = 0;
  auto nameUrl = String(SERVER) + String("/programs/get_name?id=") + urlEncode(String(id));
  if (makeRequest(nameUrl, programName, 256, &realsize))
  {
    setError("error making request for program name");
    return;
  }

  queued_action = _sendDownloadedProgram;

  setSuccess("queued download");
}

/// OTHER FUNCTIONS

int sendProgramVariable(const char *name, uint8_t *program, size_t variableSize)
{
  Serial.print("transferring: ");
  Serial.print(name);
  Serial.print("(");
  Serial.print(variableSize);
  Serial.println(")");

  int dataLength = 0;

  // IF THIS ISNT SET TO COMP83P, THIS DOESNT WORK
  // seems like ti-84s cant silent transfer to each other
  uint8_t msg_header[4] = {COMP83P, RTS, 13, 0};

  uint8_t rtsdata[13] = {variableSize & 0xff, variableSize >> 8, VarTypes82::VarProgram, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  int nameSize = strlen(name);
  if (nameSize == 0)
  {
    return 1;
  }
  memcpy(&rtsdata[3], name, min(nameSize, 8));

  auto rtsVal = cbl.send(msg_header, rtsdata, 13);
  if (rtsVal)
  {
    Serial.print("rts return: ");
    Serial.println(rtsVal);
    return rtsVal;
  }

  cbl.resetLines();
  auto ackVal = cbl.get(msg_header, NULL, &dataLength, 0);
  if (ackVal || msg_header[1] != ACK)
  {
    Serial.print("ack return: ");
    Serial.println(ackVal);
    return ackVal;
  }

  auto ctsRet = cbl.get(msg_header, NULL, &dataLength, 0);
  if (ctsRet || msg_header[1] != CTS)
  {
    Serial.print("cts return: ");
    Serial.println(ctsRet);
    return ctsRet;
  }

  msg_header[1] = ACK;
  msg_header[2] = 0x00;
  msg_header[3] = 0x00;
  ackVal = cbl.send(msg_header, NULL, 0);
  if (ackVal || msg_header[1] != ACK)
  {
    Serial.print("ack cts return: ");
    Serial.println(ackVal);
    return ackVal;
  }

  msg_header[1] = DATA;
  msg_header[2] = variableSize & 0xff;
  msg_header[3] = (variableSize >> 8) & 0xff;
  auto dataRet = cbl.send(msg_header, program, variableSize);
  if (dataRet)
  {
    Serial.print("data return: ");
    Serial.println(dataRet);
    return dataRet;
  }

  ackVal = cbl.get(msg_header, NULL, &dataLength, 0);
  if (ackVal || msg_header[1] != ACK)
  {
    Serial.print("ack data: ");
    Serial.println(ackVal);
    return ackVal;
  }

  msg_header[1] = EOT;
  msg_header[2] = 0x00;
  msg_header[3] = 0x00;
  auto eotVal = cbl.send(msg_header, NULL, 0);
  if (eotVal)
  {
    Serial.print("eot return: ");
    Serial.println(eotVal);
    return eotVal;
  }

  Serial.print("transferred: ");
  Serial.println(name);
  return 0;
}
