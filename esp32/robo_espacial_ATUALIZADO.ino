// =======================================================================
//   CÓDIGO ATUALIZADO PARA O ROBÔ EXPLORADOR COM SERVOS, MQTT E HTTP
// =======================================================================
// Bibliotecas necessárias
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>

// --- Configurações do Usuário (MODIFIQUE AQUI) ---
const char* ssid = "SEU_WIFI_AQUI";      // <-- ALTERE PARA SEU WIFI
const char* password = "SUA_SENHA_AQUI"; // <-- ALTERE PARA SUA SENHA

String phoneNumber = "557181130658"; 
String apiKey = "5000007"; // <-- COLOQUE A APIKEY QUE VOCÊ RECEBEU NO WHATSAPP

// --- Configurações MQTT ---
const char* mqtt_server = "broker.hivemq.com"; // Broker MQTT público e gratuito
const char* mqtt_topic = "/robo/comandos"; // Tópico para receber comandos

// --- Configurações do Backend Flask ---
const char* backend_url = "http://10.0.0.129:5000/leituras"; // <-- URL ATUALIZADA

// --- Mapeamento de Pinos ---
const int DHT_PIN = 4;
const int LDR_PIN = 34;
const int PIR_PIN = 35;
const int LED_VERDE_PIN = 22;
const int LED_VERMELHO_PIN = 23;
const int SERVO_ESQUERDO_PIN = 14;
const int SERVO_DIREITO_PIN = 12;

// --- Configurações Gerais ---
#define DHT_TYPE DHT11
const int LDR_LUZ_LIMITE = 2000;
const int SERVO_PARADO = 90;
const int SERVO_FRENTE_MAX = 180;
const int SERVO_TRAS_MAX = 0;

// --- Objetos e Variáveis Globais ---
DHT dht(DHT_PIN, DHT_TYPE);
Servo servoEsquerdo;
Servo servoDireito;

WiFiClient espClient;
PubSubClient client(espClient);

float temperatura = 0.0, umidade = 0.0;
int luz = 0;
bool presenca = false;
int probabilidadeVida = 0;
String estadoRobo = "Desligado";

unsigned long tempoAnterior = 0;
const long intervalo = 2000; // Envia dados a cada 2 segundos

// =======================================================================
//              FUNÇÃO DE CALLBACK (CHAMADA PELO MQTT)
// =======================================================================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida no tópico [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String comando = "";
  for (int i = 0; i < length; i++) {
    comando += (char)payload[i];
  }
  Serial.println(comando);

  if (comando == "FRENTE") {
    moverFrente();
  } else if (comando == "TRAS") {
    moverTras();
  } else if (comando == "ESQUERDA") {
    moverEsquerda();
  } else if (comando == "DIREITA") {
    moverDireita();
  } else if (comando == "DESLIGAR") {
    desligarRobo();
  }
}

// =======================================================================
//                            SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\nIniciando Robô Explorador (Versão Servo com MQTT e HTTP)...");

  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_VERDE_PIN, OUTPUT);
  pinMode(LED_VERMELHO_PIN, OUTPUT);

  servoEsquerdo.attach(SERVO_ESQUERDO_PIN);
  servoDireito.attach(SERVO_DIREITO_PIN);
  pararMotores();
  
  dht.begin();
  conectarWiFi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("Sistema pronto!");
  Serial.print("Backend URL: ");
  Serial.println(backend_url);
}

// =======================================================================
//                            LOOP
// =======================================================================
void loop() {
  if (!client.connected()) {
    reconectarMQTT();
  }
  client.loop();

  unsigned long tempoAtual = millis();
  if (tempoAtual - tempoAnterior >= intervalo) {
    tempoAnterior = tempoAtual;
    lerSensores();
    calcularProbabilidade();
    tomarDecisao();
    imprimirStatus();
    enviarDadosBackend(); // Envia dados para o InfluxDB via Flask
  }
}

// =======================================================================
//              FUNÇÕES DE CONTROLE DO MOTOR
// =======================================================================

void moverFrente() {
  Serial.println("Comando Remoto: Frente");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_FRENTE_MAX);
  servoDireito.write(SERVO_FRENTE_MAX);
}

void moverTras() {
  Serial.println("Comando Remoto: Tras");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_TRAS_MAX);
  servoDireito.write(SERVO_TRAS_MAX);
}

void moverEsquerda() {
  Serial.println("Comando Remoto: Esquerda");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_TRAS_MAX);
  servoDireito.write(SERVO_FRENTE_MAX);
}

void moverDireita() {
  Serial.println("Comando Remoto: Direita");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_FRENTE_MAX);
  servoDireito.write(SERVO_TRAS_MAX);
}

void desligarRobo() {
  Serial.println("Comando Remoto: Desligar");
  estadoRobo = "Desligado";
  pararMotores();
}

void pararMotores() {
  servoEsquerdo.write(SERVO_PARADO);
  servoDireito.write(SERVO_PARADO);
}

// =======================================================================
//                       FUNÇÕES AUXILIARES
// =======================================================================

void conectarWiFi() {
  Serial.print("Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("Endereço IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Falha ao conectar WiFi!");
  }
}

void reconectarMQTT() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT Broker...");
    if (client.connect("RoboFisicoSENAI_01")) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic);
      Serial.print("Inscrito no tópico: ");
      Serial.println(mqtt_topic);
    } else {
      Serial.print("falhou, rc=");
      Serial.print(client.state());
      Serial.println(" tentando novamente em 5 segundos");
      delay(5000);
    }
  }
}

// =======================================================================
//                      FUNÇÃO: LER SENSORES
// =======================================================================
void lerSensores() {
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("Erro ao ler DHT11!");
    temperatura = 0.0;
    umidade = 0.0;
  }

  luz = analogRead(LDR_PIN);
  int leituraPIR = digitalRead(PIR_PIN);
  presenca = (leituraPIR == HIGH);

  Serial.println("---- LEITURA DOS SENSORES ----");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  Serial.print("Luz (LDR): ");
  Serial.println(luz);

  if (presenca) {
    Serial.println("Presença detectada!");
  } else {
    Serial.println("Sem presença detectada.");
  }
  Serial.println("-------------------------------");
}

// =======================================================================
//                   FUNÇÃO: CALCULAR PROBABILIDADE DE VIDA
// =======================================================================
void calcularProbabilidade() {
  probabilidadeVida = 0;

  if (temperatura >= 15 && temperatura <= 30) {
    probabilidadeVida += 25;
  }

  if (umidade >= 40 && umidade <= 70) {
    probabilidadeVida += 25;
  }

  if (luz >= LDR_LUZ_LIMITE) {
    probabilidadeVida += 20;
  }

  if (presenca) {
    probabilidadeVida += 30;
  }

  if (probabilidadeVida > 100) probabilidadeVida = 100;
  if (probabilidadeVida < 0) probabilidadeVida = 0;

  Serial.print("Probabilidade de vida: ");
  Serial.print(probabilidadeVida);
  Serial.println(" %");
}

// =======================================================================
//                    FUNÇÃO: TOMAR DECISÃO
// =======================================================================
void tomarDecisao() {
  if (probabilidadeVida > 75) {
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHO_PIN, HIGH);
    estadoRobo = "Alerta";

    Serial.println("⚠️ ALERTA! Alta probabilidade de vida detectada!");
    enviarAlertaWhatsApp();
  } 
  else if (estadoRobo == "Desligado") {
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHO_PIN, HIGH);
  }
  else {
    digitalWrite(LED_VERDE_PIN, HIGH);
    digitalWrite(LED_VERMELHO_PIN, LOW);
    estadoRobo = "Ligado";
    Serial.println("🟢 Exploração normal. Nenhum indício relevante detectado.");
  }
}

// =======================================================================
//                      FUNÇÃO: IMPRIMIR STATUS GERAL
// =======================================================================
void imprimirStatus() {
  Serial.println("========== STATUS DO ROBÔ ==========");
  Serial.print("Temperatura: ");
  Serial.print(temperatura);
  Serial.println(" °C");

  Serial.print("Umidade: ");
  Serial.print(umidade);
  Serial.println(" %");

  Serial.print("Luz: ");
  Serial.println(luz);

  Serial.print("Presença: ");
  Serial.println(presenca ? "Detectada" : "Não detectada");

  Serial.print("Probabilidade de vida: ");
  Serial.print(probabilidadeVida);
  Serial.println(" %");

  Serial.print("Estado do robô: ");
  Serial.println(estadoRobo);
  Serial.println("====================================");
  Serial.println();
}

// =======================================================================
//        FUNÇÃO: ENVIAR DADOS PARA O BACKEND FLASK → INFLUXDB
// =======================================================================
void enviarDadosBackend() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi desconectado. Não foi possível enviar dados.");
    return;
  }

  // Criar JSON com os dados
  StaticJsonDocument<256> doc;
  doc["temperatura_c"] = temperatura;
  doc["umidade_pct"] = umidade;
  doc["luminosidade"] = luz;
  doc["presenca"] = presenca ? 1 : 0;
  doc["probabilidade_vida"] = (float)probabilidadeVida;

  String jsonString;
  serializeJson(doc, jsonString);

  // Enviar via HTTP POST
  HTTPClient http;
  http.begin(backend_url);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000); // Timeout de 5 segundos

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("✅ Dados enviados! Código: ");
    Serial.println(httpResponseCode);
    
    if (httpResponseCode == 201) {
      Serial.println("📊 Leitura registrada no InfluxDB Cloud!");
    } else {
      Serial.print("⚠️ Resposta: ");
      Serial.println(response);
    }
  } else {
    Serial.print("❌ Erro ao enviar. Código: ");
    Serial.println(httpResponseCode);
  }

  http.end();
  
  Serial.println("📤 DADOS ENVIADOS:");
  Serial.println(jsonString);
  Serial.println("------------------");
}

// =======================================================================
//               FUNÇÃO: ENVIAR ALERTA VIA CALLMEBOT (WHATSAPP)
// =======================================================================
void enviarAlertaWhatsApp() {
  if (WiFi.status() == WL_CONNECTED) {
    String mensagem = "🚨 ALERTA! Alta probabilidade de vida detectada. Probabilidade: " + String(probabilidadeVida) + "%";
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber +
                 "&apikey=" + apiKey +
                 "&text=" + urlEncode(mensagem);

    HTTPClient http;
    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      Serial.println("📨 Mensagem WhatsApp enviada!");
    } else {
      Serial.print("❌ Erro WhatsApp. Código: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}
