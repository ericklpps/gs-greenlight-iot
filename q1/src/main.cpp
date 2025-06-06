/********************************************************************
 * Projeto: Envio de Dados MQTT com ESP32 - Global Solution 2025
 * Autor: André Tritiack (Modificado por Manus para Green Light)
 * Placa: DOIT ESP32 DEVKIT V1
 * 
 * Descrição:
 * Este projeto conecta o ESP32 a uma rede Wi-Fi e a um Broker MQTT.
 * A cada 10 segundos, envia uma mensagem JSON contendo:
 * - ID do grupo
 * - ID do módulo
 * - IP local
 * - Dados de temperatura e umidade do sensor DHT22
 * - Valor analógico do potenciômetro (0-4095)
 * Além disso, se inscreve em um tópico MQTT para receber comandos
 * e controlar o LED onboard (ex: alerta de temperatura).
 * 
 * Baseado no repositório original:
 * https://github.com/arnaldojr/iot-esp32-wokwi-vscode
 * Professor Arnaldo Viana - FIAP
 ********************************************************************/

//----------------------------------------------------------
// Bibliotecas
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

//----------------------------------------------------------
// Definições e configurações

#define boardLED 2      // LED onboard (usado para alerta)
#define DHTPIN 12       // Pino de dados do DHT
#define DHTTYPE DHT22   // DHT22 (AM2302)
#define POTPIN 34       // Pino do potenciômetro (GPIO34/ADC6)

// Identificadores (Atualizados pelo usuário)
const char* ID        = "553927";
const char* moduleID  = "Esp32-gs";

const char* SSID      = "Wokwi-GUEST";
const char* PASSWORD  = "";

// MQTT Broker (Infraestrutura do Professor)
const char* BROKER_MQTT  = "172.208.54.189";
const int   BROKER_PORT  = 1883;
const char* mqttUser     = "gs2025";
const char* mqttPassword = "q1w2e3r4";

// Tópicos MQTT
#define TOPICO_PUBLISH  "2TDS/esp32/teste" // Tópico para ENVIAR dados
// ***** NOVO: Tópico para RECEBER comandos (ex: controlar LED) *****
// Usamos o ID e moduleID para tornar o tópico único para seu dispositivo
#define TOPICO_SUBSCRIBE "553927/Esp32-gs/comando"

//----------------------------------------------------------
// Variáveis globais

WiFiClient espClient;
PubSubClient MQTT(espClient);
JsonDocument doc;  // Documento JSON dinâmico
char buffer[256];  // Buffer para o JSON serializado
DHT dht(DHTPIN, DHTTYPE);

float temperatura;
float umidade;
int valorPot;      // Valor do potenciômetro

// ***** NOVO: Protótipo da função callback *****
// Precisamos declarar a função antes de usá-la
void callbackMQTT(char* topic, byte* payload, unsigned int length);

//----------------------------------------------------------
// Conexão Wi-Fi (Funções mantidas como no código original)

void initWiFi() {
    WiFi.begin(SSID, PASSWORD);
    Serial.print("Conectando ao Wi-Fi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.print(".");
    }
    Serial.println("\nWi-Fi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("MAC Address: ");
    Serial.println(WiFi.macAddress());
}

void reconectaWiFi() {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Reconectando Wi-Fi...");
        initWiFi();
    }
}

//----------------------------------------------------------
// Conexão MQTT

void initMQTT() {
    MQTT.setServer(BROKER_MQTT, BROKER_PORT);
    // ***** NOVO: Define a função que será chamada quando chegar mensagem *****
    MQTT.setCallback(callbackMQTT);
    
    while (!MQTT.connected()) {
        Serial.println("Conectando ao Broker MQTT...");
        if (MQTT.connect(moduleID, mqttUser, mqttPassword)) {
            Serial.println("Conectado ao Broker!");
            // ***** NOVO: Se inscreve no tópico de comando após conectar *****
            if (MQTT.subscribe(TOPICO_SUBSCRIBE)) {
                Serial.print("Inscrito no tópico: ");
                Serial.println(TOPICO_SUBSCRIBE);
            } else {
                Serial.println("Falha ao se inscrever no tópico de comando");
            }
        } else {
            Serial.print("Falha na conexão. Estado: ");
            Serial.println(MQTT.state());
            delay(2000);
        }
    }
}

void verificaConexoesWiFiEMQTT() {
    reconectaWiFi();
    if (!MQTT.connected()) {
        initMQTT(); // Se reconectar ao MQTT, a inscrição será refeita
    }
    // ***** IMPORTANTE: MQTT.loop() processa mensagens recebidas *****
    // Esta linha é crucial para que a função callback seja chamada!
    MQTT.loop(); 
}

//----------------------------------------------------------
// ***** NOVO: Função Callback para tratar mensagens MQTT recebidas *****

void callbackMQTT(char* topic, byte* payload, unsigned int length) {
    // Converte o payload (bytes) para uma string
    String msg;
    for (int i = 0; i < length; i++) {
        msg += (char)payload[i];
    }
    msg.toUpperCase(); // Converte para maiúsculas para facilitar comparação

    Serial.print("Mensagem recebida no tópico [");
    Serial.print(topic);
    Serial.print("]: ");
    Serial.println(msg);

    // Verifica se a mensagem é para controlar o LED
    if (strcmp(topic, TOPICO_SUBSCRIBE) == 0) { // Confirma se é o tópico correto
        if (msg == "ON") {
            digitalWrite(boardLED, HIGH); // Liga o LED
            Serial.println("LED de Alerta LIGADO");
        } else if (msg == "OFF") {
            digitalWrite(boardLED, LOW);  // Desliga o LED
            Serial.println("LED de Alerta DESLIGADO");
        } else {
            Serial.println("Comando desconhecido para o LED.");
        }
    }
}

//----------------------------------------------------------
// Envio e feedback (Funções mantidas, mas piscaLed pode ser removido se o LED for só para alerta)

void enviaEstadoOutputMQTT() {
    MQTT.publish(TOPICO_PUBLISH, buffer);
    Serial.println("Mensagem publicada com sucesso!");
}

// Esta função pode ser removida ou adaptada, já que o LED agora é controlado pelo Node-RED
// void piscaLed() {
//     digitalWrite(boardLED, HIGH);
//     delay(300);
//     digitalWrite(boardLED, LOW);
// }

//----------------------------------------------------------
// Setup inicial

void setup() {
    Serial.begin(115200);
    pinMode(boardLED, OUTPUT);
    pinMode(POTPIN, INPUT);  
    digitalWrite(boardLED, LOW); // Garante que o LED começa desligado
    dht.begin();
    initWiFi();    
    initMQTT();
}

//----------------------------------------------------------
// Loop principal

void loop() {
    // Verifica e mantém conexões, e processa mensagens recebidas
    verificaConexoesWiFiEMQTT();

    // Leitura dos dados do sensor DHT
    temperatura = dht.readTemperature();
    umidade = dht.readHumidity();

    // Leitura do potenciômetro
    valorPot = analogRead(POTPIN);

    // Limpa o documento JSON para nova utilização
    doc.clear();

    // 1. Identificação
    doc["ID"] = ID;
    doc["Sensor"] = moduleID;

    // 2. Rede
    doc["IP"] = WiFi.localIP().toString();
    doc["MAC"] = WiFi.macAddress();

    // 3. Dados de sensores
    if (!isnan(temperatura) && !isnan(umidade)) {
        doc["Temperatura"] = temperatura;
        doc["Umidade"] = umidade;
    } else {
        doc["Temperatura"] = "Erro na leitura";
        doc["Umidade"] = "Erro na leitura";
    }

    // 4. Dados do potenciômetro
    doc["Potenciometro"] = valorPot;

    // Serializa JSON para string
    serializeJson(doc, buffer);

    // Exibe no monitor serial
    Serial.println(buffer);

    // Envia via MQTT
    enviaEstadoOutputMQTT();

    // Feedback visual (removido ou comentado, pois o LED agora é para alerta)
    // piscaLed(); 

    // Intervalo de envio
    delay(10000);
}

