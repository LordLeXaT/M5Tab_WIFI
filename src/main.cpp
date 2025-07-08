#include <Arduino.h>
#include <M5GFX.h>
#include <M5Unified.h>
#include <WiFi.h>
#include <WebServer.h>
#include <NetworkClient.h>
#include <ESPmDNS.h>

// do not forget to create credentials.h in src directory
#include "credentials.h"
// should contain the following
/*
const char *ssid = "SuperSSDI";
const char *password = "SuperWIFIPassword";
*/

#define SDIO2_CLK GPIO_NUM_12
#define SDIO2_CMD GPIO_NUM_13
#define SDIO2_D0 GPIO_NUM_11
#define SDIO2_D1 GPIO_NUM_10
#define SDIO2_D2 GPIO_NUM_9
#define SDIO2_D3 GPIO_NUM_8
#define SDIO2_RST GPIO_NUM_15

WebServer server(80);

void handleRoot()
{
  server.send(200, "text/plain", "You have done it");
}

void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++)
  {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

M5GFX display;

M5Canvas smain(&display);
M5Canvas stouch(&display);

int ttx = 0;
int tty = 0;

void setup()
{
  display.init();
  display.setFont(&fonts::Font4);
  if (!display.touch())
  {
    display.setTextDatum(textdatum_t::middle_center);
    display.drawString("Touch not found.", display.width() / 2, display.height() / 2);
  }
  display.setEpdMode(epd_mode_t::epd_fastest);
  display.startWrite();
  Serial.begin(115200);

  // start wifi
  WiFi.setPins(SDIO2_CLK, SDIO2_CMD, SDIO2_D0, SDIO2_D1, SDIO2_D2, SDIO2_D3, SDIO2_RST);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // simplified host
  if (MDNS.begin("esp32"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void checkTouch()
{
  lgfx::touch_point_t tp[1];
  int nums = display.getTouchRaw(tp, 1);
  if (nums)
  {
    ttx = int(tp->x);
    tty = int(tp->y);
    smain.setTextColor(RED);
  }
  else
  {
    smain.setTextColor(GREEN);
  }
}

void loop()
{
  server.handleClient();
  checkTouch();
  smain.createSprite(720, 1280);
  smain.drawRect(10, 10, 700, 1260, RED);
  smain.setTextSize(12);
  smain.setCursor(15, 15);
  smain.print(ttx);
  smain.print("/");
  smain.print(tty);
  smain.setTextSize(6);
  smain.setCursor(15, 150);
  smain.print(WiFi.localIP());
  smain.pushSprite(0, 0);
  vTaskDelay(1);
}
