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

int horaBase = 0, minutoBase = 0;
unsigned long millisBase = 0;

AsyncWebServer server(80);

// --- Salvar e carregar estados das saídas ---
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

// --- Salvar e carregar tempo base ---
void salvarTempo()
{
  File file = LittleFS.open("/tempo.json", "w");
  if (file)
  {
    StaticJsonDocument<100> doc;
    doc["hora"] = horaBase;
    doc["minuto"] = minutoBase;
    doc["millis"] = millisBase;
    serializeJson(doc, file);
    file.close();
  }
}

void carregarTempo()
{
  if (LittleFS.exists("/tempo.json"))
  {
    File file = LittleFS.open("/tempo.json", "r");
    if (file)
    {
      StaticJsonDocument<100> doc;
      if (deserializeJson(doc, file) == DeserializationError::Ok)
      {
        horaBase = doc["hora"];
        minutoBase = doc["minuto"];
        millisBase = doc["millis"];
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
  carregarTempo();

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

  server.on("/api/tempo", HTTP_POST, [](AsyncWebServerRequest *request)
            {
    if (request->hasParam("hora", true) && request->hasParam("minuto", true)) {
      horaBase = request->getParam("hora", true)->value().toInt();
      minutoBase = request->getParam("minuto", true)->value().toInt();
      millisBase = millis();
      salvarTempo();
      request->send(200, "text/plain", "Hora sincronizada");
    } else {
      request->send(400, "text/plain", "Parâmetros inválidos");
    } });

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

void loop()
{
  unsigned long minutosPassados = (millis() - millisBase) / 60000UL;
  int horaAtual = (horaBase * 60 + minutoBase + minutosPassados) / 60 % 24;
  int minutoAtual = (horaBase * 60 + minutoBase + minutosPassados) % 60;

  // Verifica horários exatos
  if ((horaAtual == 8 && minutoAtual == 30) ||
      (horaAtual == 9 && minutoAtual == 30) ||
      (horaAtual == 14 && minutoAtual == 30) ||
      (horaAtual == 15 && minutoAtual == 30))
  {

    digitalWrite(saidas[0], HIGH);
    delay(4 * 60 * 1000); // Liga por 4 minutos
    digitalWrite(saidas[0], LOW);
    delay(60 * 1000); // Aguarda 1 min para evitar repetição no mesmo horário
  }

  delay(1000);
}
