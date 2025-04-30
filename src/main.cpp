#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

/// @brief Nome da rede Wi-Fi do Access Point.
#define SSID_AP "Kunugui"
/// @brief Senha do Access Point. Vazio = sem senha.
#define PASSWORD_AP ""

// --- Parâmetros de temporização ---
/// @brief Intervalo entre acionamentos automáticos (em milissegundos).
///        Por padrão: 2 horas = 2 * 60 * 60 * 1000.
// unsigned long INTERVALO_AUTO = 2UL * 60UL * 60UL * 1000UL;
unsigned long INTERVALO_AUTO = 1UL * 60UL * 1000UL; // 1 minuto

/// @brief Duração do acionamento automático (em milissegundos).
///        Por padrão: 4 minutos = 4 * 60 * 1000.
// unsigned long TEMPO_LIGADO = 4UL * 60UL * 1000UL;
unsigned long TEMPO_LIGADO = 1UL * 60UL * 1000UL; // 1 minuto de duração, por exemplo

// GPIOs dos relés
const int saidas[] = {4, 16, 17, 18};
/// @brief Estado atual de cada relé (true = ligado).
bool estadoSaidas[4] = {false, false, false, false};

/// @brief Instância do servidor web.
AsyncWebServer server(80);

/**
 * @brief Salva os estados dos relés em LittleFS.
 */
void salvarEstados()
{
  File file = LittleFS.open("/estados.json", "w");
  if (!file)
    return;
  JsonDocument doc;
  for (int i = 0; i < 4; i++)
  {
    doc["saida" + String(i + 1)] = estadoSaidas[i];
  }
  serializeJson(doc, file);
  file.close();
}

/**
 * @brief Carrega os estados dos relés do LittleFS.
 */
void carregarEstados()
{
  if (!LittleFS.exists("/estados.json"))
    return;
  File file = LittleFS.open("/estados.json", "r");
  if (!file)
    return;
  JsonDocument doc;
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

/**
 * @brief Setup inicial do ESP32.
 */
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
  Serial.print("AP iniciado. IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req)
            { req->send(LittleFS, "/index.html", "text/html"); });

  server.on("/api/saida", HTTP_POST, [](AsyncWebServerRequest *req)
            {
    if (req->hasParam("id", true) && req->hasParam("estado", true)) {
      int id = req->getParam("id", true)->value().toInt();
      bool st = req->getParam("estado", true)->value() == "1";
      if (id >= 1 && id <= 4) {
        digitalWrite(saidas[id - 1], st ? HIGH : LOW);
        estadoSaidas[id - 1] = st;
        salvarEstados();
        req->send(200, "text/plain", "OK");
        return;
      }
    }
    req->send(400, "text/plain", "Erro"); });

  server.on("/api/estado", HTTP_GET, [](AsyncWebServerRequest *req)
            {
    JsonDocument doc;
    for (int i = 0; i < 4; i++) {
      doc["saida" + String(i + 1)] = estadoSaidas[i];
    }
    String json;
    serializeJson(doc, json);
    req->send(200, "application/json", json); });

  server.begin();
}

/**
 * @brief Loop principal: gerencia acionamento automático do relé 1.
 */
void loop()
{
  static unsigned long ultimoCiclo = 0;
  static bool releAutoLigado = false;
  static unsigned long tempoInicio = 0;

  unsigned long agora = millis();

  // Aciona a cada INTERVALO_AUTO
  if (!releAutoLigado && (agora - ultimoCiclo >= INTERVALO_AUTO))
  {
    Serial.println("Auto: LIGAR relé 1");
    digitalWrite(saidas[0], HIGH);
    estadoSaidas[0] = true;
    salvarEstados();
    releAutoLigado = true;
    tempoInicio = agora;
  }

  // Desliga após TEMPO_LIGADO
  if (releAutoLigado && (agora - tempoInicio >= TEMPO_LIGADO))
  {
    Serial.println("Auto: DESLIGAR relé 1");
    digitalWrite(saidas[0], LOW);
    estadoSaidas[0] = false;
    salvarEstados();
    releAutoLigado = false;
    ultimoCiclo = agora;
  }

  delay(1000);
}
