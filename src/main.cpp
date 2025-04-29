#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

#define SSID_AP "Kunugui"
#define PASSWORD_AP ""

const int saidas[] = {4, 16, 17, 18};
bool estadoSaidas[4] = {false, false, false, false};
AsyncWebServer server(80);

void salvarEstados()
{
  File file = LittleFS.open("/estados.json", "w");
  if (file)
  {
    StaticJsonDocument<100> doc;
    for (int i = 0; i < 4; i++)
      doc["saida" + String(i + 1)] = estadoSaidas[i];
    serializeJson(doc, file);
    file.close();
  }
}

void carregarEstados()
{
  if (LittleFS.exists("/estados.json"))
  {
    File file = LittleFS.open("/estados.json", "r");
    if (file)
    {
      StaticJsonDocument<100> doc;
      if (deserializeJson(doc, file) == DeserializationError::Ok)
      {
        for (int i = 0; i < 4; i++)
        {
          estadoSaidas[i] = doc["saida" + String(i + 1)];
          digitalWrite(saidas[i], estadoSaidas[i] ? HIGH : LOW);
        }
      }
      file.close();
    }
  }
}

void setup()
{
  Serial.begin(115200);
  delay(2000);

  for (int i = 0; i < 4; i++)
  {
    pinMode(saidas[i], OUTPUT);
    digitalWrite(saidas[i], LOW);
  }

  if (!LittleFS.begin(true))
  {
    Serial.println("Falha ao montar LittleFS");
    delay(5000);
    ESP.restart();
  }

  carregarEstados();

  WiFi.softAP(SSID_AP, PASSWORD_AP);
  Serial.print("Access Point iniciado. IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/index.html", "text/html"); });

  server.on("/api/saida", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("id", true) && request->hasParam("estado", true)) {
      int id = request->getParam("id", true)->value().toInt();
      bool estado = request->getParam("estado", true)->value() == "1";
      if (id >= 1 && id <= 4) {
        digitalWrite(saidas[id - 1], estado ? HIGH : LOW);
        estadoSaidas[id - 1] = estado;
        salvarEstados();
        request->send(200, "text/plain", "OK");
        return;
      }
    }
    request->send(400, "text/plain", "Erro"); });

  server.on("/api/estado", HTTP_GET, [](AsyncWebServerRequest *request)
            {
      StaticJsonDocument<100> doc;
      for (int i = 0; i < 4; i++) {
        doc["saida" + String(i + 1)] = estadoSaidas[i];
      }
    
      String json;
      serializeJson(doc, json);
      request->send(200, "application/json", json); });

  server.begin();
}

void loop() {}
