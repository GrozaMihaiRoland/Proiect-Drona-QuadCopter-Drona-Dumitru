#include <Wire.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "LittleFS.h"
#include <Adafruit_NeoPixel.h>

#define PIN 48
#define NUMPIXELS 1

Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
#define DELAYVAL 10

#define GYRO_ADDR 0x69   // Change to 0x69 if SDO2 is pulled high

// Gyro registers
#define GYRO_CHIP_ID 0x00
#define GYRO_X_LSB   0x02
#define GYRO_X_MSB   0x03
#define GYRO_Y_LSB   0x04
#define GYRO_Y_MSB   0x05
#define GYRO_Z_LSB   0x06
#define GYRO_Z_MSB   0x07
#define GYRO_PWR_CTRL 0x11
#define GYRO_RANGE   0x0F
#define GYRO_BW      0x10

const char* ssid     = "ESP32WiFi";
const char* password = "helloesp32";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

JSONVar readings;

unsigned long lTime;
unsigned long lastSend;

int16_t angX = 0;
int16_t angY = 0;
int16_t angZ = 0;
int16_t R;
int16_t G;
int16_t B;

void setup() {
  Wire.begin(4,5);
  Serial.begin(115200);
  Serial.printf("Hey im alive");

  // Power up the gyro
  Wire.beginTransmission(GYRO_ADDR);
  Wire.write(GYRO_PWR_CTRL);
  Wire.write(0x00); // normal mode
  Wire.endTransmission();

  // Optional: set range (±2000 dps)
  Wire.beginTransmission(GYRO_ADDR);
  Wire.write(GYRO_RANGE);
  Wire.write(0x03); // ±2000 dps
  Wire.endTransmission();

  // Optional: set bandwidth / ODR
  Wire.beginTransmission(GYRO_ADDR);
  Wire.write(GYRO_BW);
  Wire.write(0x07); // 100 Hz ODR, 32 Hz bandwidth as example
  Wire.endTransmission();

  // Check gyro ID
  Wire.beginTransmission(GYRO_ADDR);
  Wire.write(GYRO_CHIP_ID);
  Wire.endTransmission();
  Wire.requestFrom(GYRO_ADDR, 1);
  byte id = Wire.read();
  Serial.print("Gyro Chip ID: 0x");
  Serial.println(id, HEX);

  lTime = millis();

  initWiFi();
  initLittleFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(LittleFS, "/index.html", "text/html");
  });

  server.serveStatic("/", LittleFS, "/");

  // Start server
  server.begin();
  pixels.begin();
}

void loop() {
  unsigned long cTime = millis();
  float dt = (cTime - lTime) / 1000.0;
  int16_t gyroX = read16(GYRO_X_LSB) / 131.1;
  int16_t gyroY = read16(GYRO_Y_LSB) / 131.1;
  int16_t gyroZ = read16(GYRO_Z_LSB) / 131.1;

  angX += gyroX * dt;
  angY += gyroY * dt;
  angZ += gyroZ * dt;

  /*Serial.print(angX); Serial.print(",");
  Serial.print(angY); Serial.print(",");
  Serial.print(angZ); Serial.print(",");
  Serial.print(dt); Serial.println(",");
  delay(20 - 0.02);
  lTime = cTime;*/

  if ((millis() - lastSend) > 100) {
    String sensorReadings = getSensorReadings();
    //Serial.println(sensorReadings);
    notifyClients(sensorReadings);
    lastSend = millis();
  }
  ws.cleanupClients();

  pixels.clear();

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(R, G, B));
    pixels.show();
    delay(DELAYVAL);
  }

}

// Read 16-bit value from LSB register
int16_t read16(byte regLSB) {
  Wire.beginTransmission(GYRO_ADDR);
  Wire.write(regLSB);
  Wire.endTransmission();
  Wire.requestFrom(GYRO_ADDR, 2);

  byte lsb = Wire.read();
  byte msb = Wire.read();
  return (int16_t)((msb << 8) | lsb);
}

void initWiFi() {
  if (!WiFi.softAP(ssid, password)) {
    log_e("Soft AP creation failed.");
    while(1);
  }
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}

String getSensorReadings(){
  readings["x"] = String(angX);
  readings["y"] =  String(angY);
  readings["z"] = String(angZ);

  readings["R"] = R;
  readings["G"] = G;
  readings["B"] = B;
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

void notifyClients(String sensorReadings) {
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
      data[len] = 0; 

      String msg = (char*)data;
      Serial.println(msg);

      JSONVar obj = JSON.parse(msg);

      if (JSON.typeof(obj) == "undefined") {
        Serial.println("JSON parse failed");
        return;
      }

      if (obj.hasOwnProperty("R")) R = int(obj["R"]);
      if (obj.hasOwnProperty("G")) G = int(obj["G"]);
      if (obj.hasOwnProperty("B")) B = int(obj["B"]);
    
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}


void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
  }
}

/*
void loop() {
  WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.println("<!DOCTYPE html>");
            client.println("<html>");
            client.println("<head>");
            client.println("<style>");
            client.println("body { background:#111; color:white; font-family:sans-serif; }");
            client.println("button { padding:20px; font-size:100px; display: flex; flex-direction: column; gap: 10px; justify-content: center; align-items: center;}");
            client.println("h1 { text-align: center; }");
            client.println("</style>");
            client.println("</head>");
            client.println("<body>");
            client.println("<h1>ESP32 LED Control</h1>");
            client.println("<button onclick=\"setColor('G')\">Green</button>");
            client.println("<button onclick=\"setColor('R')\">Red</button>");
            client.println("<button onclick=\"setColor('B')\">Blue</button>");
            client.println("<script>");
            client.println("function setColor(color) {fetch(\"/\" + color);}");
            client.println("</script>");
            client.println("</body>");
            client.println("</html>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        pixels.clear();

        if (currentLine.endsWith("GET /R")) {
          for(int i=0; i<NUMPIXELS; i++) {
            pixels.setPixelColor(i, pixels.Color(255, 0, 0));
            pixels.show();
            delay(DELAYVAL);
          }                  
        }
      
        if (currentLine.endsWith("GET /G")) {
          for(int i=0; i<NUMPIXELS; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 255, 0));
            pixels.show();
            delay(DELAYVAL);
          }            
        }
        if (currentLine.endsWith("GET /B")) {
          for(int i=0; i<NUMPIXELS; i++) {
            pixels.setPixelColor(i, pixels.Color(0, 0, 255));
            pixels.show();
            delay(DELAYVAL);
          }                  
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}

*/
