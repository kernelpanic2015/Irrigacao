# Irrigação do Kunugui

## Equipe ARR da [Fazenda dos Cogumelos](fazendadoscogumelos.com.br):

Coordenador: Professor [Carlos Abe](fazendadoscogumelos.com.br)

- Enaldo Leite
- Everton Shons
- João Malacrida
- Muriah
- Nicolas Ritton

## ESP32 Controlador de Relés

Controle um módulo de 4 relés usando um ESP32 via servidor web integrado, com acionamento automático e persistência de estado em LittleFS.

---

## 📋 Funcionalidades

- **Controle manual** de 4 relés pela interface web (botões ON/OFF).
- **Acionamento automático** do Relé 1 em intervalos configuráveis (padrão 2 h) e duração configurável (padrão 4 min).
- **Persistência de estado**: os estados dos relés são salvos em LittleFS (`/estados.json`) e restaurados após reboot.
- **Access Point standalone**: ESP32 cria AP “Kunugui” (sem senha).
- **Configuração simples**: altere apenas duas constantes em `main.cpp`:
  - `INTERVALO_AUTO` — intervalo entre ativações automáticas (ms).
  - `TEMPO_LIGADO` — duração de cada ativação (ms).

---

## 📦 Estrutura do Projeto

```
kunugui-esp32-relay-controller/
├── src/
│   └── main.cpp        # Firmware do ESP32
├── data/
│   └── index.html      # Interface web para LittleFS
├── platformio.ini
└── README.md
```

---

## ⚙️ platformio.ini

```ini
[env:esp32dev]
platform      = espressif32
board         = esp32dev
framework     = arduino
monitor_speed = 115200
```

---

## ⚡ main.cpp (resumo)

```cpp
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

// AP Wi-Fi
#define SSID_AP     "Kunugui"
#define PASSWORD_AP ""

// Tempo automático (ajuste para testes)
unsigned long INTERVALO_AUTO = 2UL * 60UL * 60UL * 1000UL; // 2 h
unsigned long TEMPO_LIGADO  = 4UL * 60UL * 1000UL;        // 4 min

const int saidas[]     = {4, 16, 17, 18};
bool estadoSaidas[4]   = {false, false, false, false};
AsyncWebServer server(80);

// ... funções salvarEstados(), carregarEstados(), setup(), loop() ...
```

---

## 🌐 Interface Web

1. Conecte-se à rede Wi-Fi **Kunugui**.
2. Acesse `http://192.168.4.1/` no navegador.
3. Os botões exibem “ON” (verde) ou “OFF” (vermelho) para cada relé.
4. Clique para alternar; o estado será salvo automaticamente.

---

## 🔧 Customização

- **Intervalo automático**: edite `INTERVALO_AUTO` em `main.cpp`.
  - Exemplo para **5 min**:
    ```cpp
    INTERVALO_AUTO = 5UL * 60UL * 1000UL;
    ```
- **Duração do acionamento**: edite `TEMPO_LIGADO`.
  - Exemplo para **1 min**:
    ```cpp
    TEMPO_LIGADO = 1UL * 60UL * 1000UL;
    ```
- **Pinos dos relés**: ajuste o array `saidas[]`.
- **AP SSID/Senha**: altere `SSID_AP` e `PASSWORD_AP`.

---

## 🚀 Build & Upload

1. Clone o repositório:
   ```bash
   git clone https://github.com/SEU_USUARIO/kunugui-esp32-relay-controller.git
   cd kunugui-esp32-relay-controller
   ```
2. Abra no VS Code com PlatformIO.
3. Conecte o ESP32 via USB.
4. Compile e envie:
   ```bash
   pio run --target upload
   ```

---

## 📂 Upload do LittleFS

Para gravar a interface web no sistema de arquivos:

```bash
pio run --target buildfs
pio run --target uploadfs
```

---

## 🛠️ Licença

Este projeto está licenciado sob a **MIT License**. Veja o arquivo [LICENSE](LICENSE) para mais detalhes.

---

> Desenvolvido com ❤️ por [Enaldo Leite](enaldo@yahoo.com)  
> GitHub: https://github.com/SEU_USUARIO/kunugui-esp32-relay-controller
