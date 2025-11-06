// =======================================================================
//   CÓDIGO PARA ESP32 - VERSÃO CODESPACE (HTTPS)
// =======================================================================
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <ESP32Servo.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <UrlEncode.h>

// --- Configurações do Usuário (MODIFIQUE AQUI) ---
const char* ssid = "SEU_WIFI_AQUI";      // <-- ALTERE
const char* password = "SUA_SENHA_AQUI"; // <-- ALTERE

String phoneNumber = "557181130658"; 
String apiKey = "5000007";

// --- Configurações MQTT ---
const char* mqtt_server = "broker.hivemq.com";
const char* mqtt_topic = "/robo/comandos";

// --- Configurações do Backend (URL PÚBLICA - LOCALTUNNEL) ---
const char* backend_url = "https://plain-clouds-move.loca.lt/leituras";

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
const long intervalo = 5000; // 5 segundos (mais espaçado para HTTPS)

// =======================================================================
//              FUNÇÃO DE CALLBACK (MQTT)
// =======================================================================
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem MQTT [");
  Serial.print(topic);
  Serial.print("]: ");
  
  String comando = "";
  for (int i = 0; i < length; i++) {
    comando += (char)payload[i];
  }
  Serial.println(comando);

  if (comando == "FRENTE") moverFrente();
  else if (comando == "TRAS") moverTras();
  else if (comando == "ESQUERDA") moverEsquerda();
  else if (comando == "DIREITA") moverDireita();
  else if (comando == "DESLIGAR") desligarRobo();
}

// =======================================================================
//                            SETUP
// =======================================================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n🤖 Robô Explorador - Versão Codespace HTTPS");

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

  Serial.println("✅ Sistema pronto!");
  Serial.print("📡 Backend: ");
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
    enviarDadosBackend();
  }
}

// =======================================================================
//              FUNÇÕES DE CONTROLE DO MOTOR
// =======================================================================
void moverFrente() {
  Serial.println("→ Frente");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_FRENTE_MAX);
  servoDireito.write(SERVO_FRENTE_MAX);
}

void moverTras() {
  Serial.println("← Trás");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_TRAS_MAX);
  servoDireito.write(SERVO_TRAS_MAX);
}

void moverEsquerda() {
  Serial.println("↰ Esquerda");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_TRAS_MAX);
  servoDireito.write(SERVO_FRENTE_MAX);
}

void moverDireita() {
  Serial.println("↱ Direita");
  estadoRobo = "Ligado";
  servoEsquerdo.write(SERVO_FRENTE_MAX);
  servoDireito.write(SERVO_TRAS_MAX);
}

void desligarRobo() {
  Serial.println("⏸ Desligar");
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
  Serial.print("📶 Conectando a ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ WiFi conectado!");
    Serial.print("📍 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Falha ao conectar WiFi!");
  }
}

void reconectarMQTT() {
  while (!client.connected()) {
    Serial.print("📡 Conectando MQTT...");
    if (client.connect("RoboEspacial_01")) {
      Serial.println("OK");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("falhou (");
      Serial.print(client.state());
      Serial.println(") - tentando em 5s");
      delay(5000);
    }
  }
}

// =======================================================================
//                      LER SENSORES
// =======================================================================
void lerSensores() {
  temperatura = dht.readTemperature();
  umidade = dht.readHumidity();

  if (isnan(temperatura) || isnan(umidade)) {
    Serial.println("⚠️ Erro DHT11");
    temperatura = 0.0;
    umidade = 0.0;
  }

  luz = analogRead(LDR_PIN);
  presenca = (digitalRead(PIR_PIN) == HIGH);

  Serial.println("─── SENSORES ───");
  Serial.printf("🌡️  %.1f°C | 💧 %.1f%% | 💡 %d | %s\n", 
    temperatura, umidade, luz, presenca ? "👤 Presença" : "🚫 Sem presença");
}

// =======================================================================
//                   CALCULAR PROBABILIDADE
// =======================================================================
void calcularProbabilidade() {
  probabilidadeVida = 0;

  if (temperatura >= 15 && temperatura <= 30) probabilidadeVida += 25;
  if (umidade >= 40 && umidade <= 70) probabilidadeVida += 25;
  if (luz >= LDR_LUZ_LIMITE) probabilidadeVida += 20;
  if (presenca) probabilidadeVida += 30;

  if (probabilidadeVida > 100) probabilidadeVida = 100;
  if (probabilidadeVida < 0) probabilidadeVida = 0;

  Serial.printf("🧬 Prob. Vida: %d%%\n", probabilidadeVida);
}

// =======================================================================
//                    TOMAR DECISÃO
// =======================================================================
void tomarDecisao() {
  if (probabilidadeVida > 75) {
    digitalWrite(LED_VERDE_PIN, LOW);
    digitalWrite(LED_VERMELHO_PIN, HIGH);
    estadoRobo = "Alerta";
    Serial.println("⚠️ ALERTA! Alta prob. de vida!");
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
    Serial.println("🟢 Exploração normal");
  }
}

// =======================================================================
//                      STATUS
// =======================================================================
void imprimirStatus() {
  Serial.println("════════════════");
  Serial.printf("Estado: %s\n", estadoRobo.c_str());
  Serial.println("════════════════\n");
}

// =======================================================================
//        ENVIAR DADOS PARA BACKEND (HTTPS)
// =======================================================================
void enviarDadosBackend() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("⚠️ WiFi offline");
    return;
  }

  // Criar JSON
  StaticJsonDocument<256> doc;
  doc["temperatura_c"] = temperatura;
  doc["umidade_pct"] = umidade;
  doc["luminosidade"] = luz;
  doc["presenca"] = presenca ? 1 : 0;
  doc["probabilidade_vida"] = (float)probabilidadeVida;

  String jsonString;
  serializeJson(doc, jsonString);

  // Cliente HTTPS
  WiFiClientSecure *client = new WiFiClientSecure;
  if(client) {
    client->setInsecure(); // Ignora verificação SSL (OK para testes)

    HTTPClient https;
    
    if (https.begin(*client, backend_url)) {
      https.addHeader("Content-Type", "application/json");
      https.setTimeout(10000); // 10 segundos para HTTPS
      
      Serial.println("📤 Enviando para Codespace...");
      int httpCode = https.POST(jsonString);
      
      if (httpCode > 0) {
        Serial.printf("✅ Resposta: %d\n", httpCode);
        
        if (httpCode == 201) {
          Serial.println("📊 Salvo no InfluxDB Cloud!");
        } else {
          String response = https.getString();
          Serial.println(response);
        }
      } else {
        Serial.printf("❌ Erro: %s\n", https.errorToString(httpCode).c_str());
      }
      
      https.end();
    } else {
      Serial.println("❌ Falha ao conectar HTTPS");
    }
    
    delete client;
  }
  
  Serial.println("📦 Dados: " + jsonString);
  Serial.println("─────────────────\n");
}

// =======================================================================
//               ALERTA WHATSAPP
// =======================================================================
void enviarAlertaWhatsApp() {
  if (WiFi.status() == WL_CONNECTED) {
    String mensagem = "🚨 ALERTA! Probabilidade: " + String(probabilidadeVida) + "%";
    String url = "https://api.callmebot.com/whatsapp.php?phone=" + phoneNumber +
                 "&apikey=" + apiKey +
                 "&text=" + urlEncode(mensagem);

    HTTPClient http;
    http.begin(url);
    int code = http.GET();

    if (code == 200) {
      Serial.println("📨 WhatsApp OK");
    } else {
      Serial.printf("❌ WhatsApp erro: %d\n", code);
    }
    http.end();
  }
}
