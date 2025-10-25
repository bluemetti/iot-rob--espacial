/*
 * Robô Explorador Espacial - ESP32 Físico
 * 
 * Este código lê os sensores (temperatura, umidade, luminosidade, presença),
 * calcula a probabilidade de vida e envia os dados via HTTP POST para o backend Flask.
 * 
 * Componentes:
 * - ESP32
 * - Sensor DHT22 (temperatura e umidade)
 * - Fotorresistor/LDR (luminosidade)
 * - Sensor PIR (presença)
 * - 2x Motores DC
 * - LED Verde e LED Vermelho
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <DHT.h>
#include <ArduinoJson.h>

// ===== CONFIGURAÇÕES WIFI =====
const char* ssid = "SUA_REDE_WIFI";           // Substitua pelo nome da sua rede WiFi
const char* password = "SUA_SENHA_WIFI";      // Substitua pela senha da sua rede WiFi

// ===== CONFIGURAÇÕES DO SERVIDOR =====
const char* serverUrl = "http://192.168.1.100:5000/leituras";  // Substitua pelo IP do seu computador

// ===== PINOS DOS SENSORES =====
#define DHT_PIN 4           // Pino do sensor DHT22
#define DHT_TYPE DHT22      // Tipo do sensor DHT
#define LDR_PIN 34          // Pino analógico do fotorresistor
#define PIR_PIN 14          // Pino digital do sensor PIR

// ===== PINOS DOS ATUADORES =====
#define LED_VERDE 25        // LED verde (status normal)
#define LED_VERMELHO 26     // LED vermelho (alerta)
#define MOTOR_ESQ_A 27      // Motor esquerdo - pino A
#define MOTOR_ESQ_B 13      // Motor esquerdo - pino B
#define MOTOR_DIR_A 12      // Motor direito - pino A
#define MOTOR_DIR_B 32      // Motor direito - pino B

// ===== OBJETOS =====
DHT dht(DHT_PIN, DHT_TYPE);

// ===== VARIÁVEIS GLOBAIS =====
unsigned long ultimoEnvio = 0;
const unsigned long intervaloEnvio = 2000;  // Enviar a cada 2 segundos

bool roboLigado = true;
float probabilidadeVida = 0.0;

// ===== ESTRUTURA PARA ARMAZENAR LEITURAS =====
struct Leitura {
  float temperatura;
  float umidade;
  int luminosidade;
  int presenca;
  float probabilidade;
};


void setup() {
  Serial.begin(115200);
  Serial.println("\n=== Robô Explorador Espacial Iniciando ===");
  
  // Configurar pinos
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(MOTOR_ESQ_A, OUTPUT);
  pinMode(MOTOR_ESQ_B, OUTPUT);
  pinMode(MOTOR_DIR_A, OUTPUT);
  pinMode(MOTOR_DIR_B, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  
  // Inicializar sensores
  dht.begin();
  
  // Conectar ao WiFi
  conectarWiFi();
  
  // LED verde aceso = robô ligado
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);
  
  Serial.println("Sistema pronto!");
}


void loop() {
  unsigned long agora = millis();
  
  // A cada 2 segundos, ler sensores e enviar dados
  if (agora - ultimoEnvio >= intervaloEnvio) {
    ultimoEnvio = agora;
    
    if (roboLigado) {
      // Ler sensores
      Leitura leitura = lerSensores();
      
      // Calcular probabilidade de vida
      leitura.probabilidade = calcularProbabilidadeVida(leitura);
      
      // Mostrar dados no Serial
      exibirDados(leitura);
      
      // Controlar LEDs baseado na probabilidade
      controlarLEDs(leitura.probabilidade);
      
      // Enviar dados para o servidor
      enviarDados(leitura);
    }
  }
  
  // Aqui você pode adicionar lógica de controle dos motores
  // baseado em comandos recebidos do controle remoto
}


void conectarWiFi() {
  Serial.print("Conectando ao WiFi");
  WiFi.begin(ssid, password);
  
  int tentativas = 0;
  while (WiFi.status() != WL_CONNECTED && tentativas < 20) {
    delay(500);
    Serial.print(".");
    tentativas++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi conectado!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFalha ao conectar WiFi!");
  }
}


Leitura lerSensores() {
  Leitura leitura;
  
  // Ler DHT22 (temperatura e umidade)
  leitura.temperatura = dht.readTemperature();
  leitura.umidade = dht.readHumidity();
  
  // Verificar se a leitura falhou
  if (isnan(leitura.temperatura) || isnan(leitura.umidade)) {
    Serial.println("Erro ao ler DHT22!");
    leitura.temperatura = 0.0;
    leitura.umidade = 0.0;
  }
  
  // Ler fotorresistor (0-4095 no ESP32)
  leitura.luminosidade = analogRead(LDR_PIN);
  
  // Ler sensor PIR (0 ou 1)
  leitura.presenca = digitalRead(PIR_PIN);
  
  return leitura;
}


float calcularProbabilidadeVida(Leitura leitura) {
  float probabilidade = 0.0;
  
  // Temperatura entre 15°C e 30°C → +25%
  if (leitura.temperatura >= 15.0 && leitura.temperatura <= 30.0) {
    probabilidade += 25.0;
  }
  
  // Umidade entre 40% e 70% → +25%
  if (leitura.umidade >= 40.0 && leitura.umidade <= 70.0) {
    probabilidade += 25.0;
  }
  
  // Luminosidade adequada (> 500) → +20%
  if (leitura.luminosidade > 500) {
    probabilidade += 20.0;
  }
  
  // Presença detectada → +30%
  if (leitura.presenca == 1) {
    probabilidade += 30.0;
  }
  
  return probabilidade;
}


void controlarLEDs(float probabilidade) {
  if (probabilidade > 75.0) {
    // ALERTA! Alta probabilidade de vida
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_VERMELHO, HIGH);
  } else {
    // Exploração normal
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_VERMELHO, LOW);
  }
}


void exibirDados(Leitura leitura) {
  Serial.println("\n========== LEITURA DOS SENSORES ==========");
  Serial.print("Temperatura: ");
  Serial.print(leitura.temperatura);
  Serial.println(" °C");
  
  Serial.print("Umidade: ");
  Serial.print(leitura.umidade);
  Serial.println(" %");
  
  Serial.print("Luminosidade: ");
  Serial.println(leitura.luminosidade);
  
  Serial.print("Presença: ");
  Serial.println(leitura.presenca == 1 ? "DETECTADA" : "Não detectada");
  
  Serial.print("Probabilidade de Vida: ");
  Serial.print(leitura.probabilidade);
  Serial.println(" %");
  
  if (leitura.probabilidade > 75.0) {
    Serial.println("🚨 ALERTA! Alta probabilidade de vida detectada!");
  } else {
    Serial.println("✓ Exploração normal. Nenhum indício relevante.");
  }
  Serial.println("==========================================\n");
}


void enviarDados(Leitura leitura) {
  // Verificar conexão WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Tentando reconectar...");
    conectarWiFi();
    return;
  }
  
  // Criar objeto JSON
  StaticJsonDocument<256> doc;
  doc["temperatura_c"] = leitura.temperatura;
  doc["umidade_pct"] = leitura.umidade;
  doc["luminosidade"] = leitura.luminosidade;
  doc["presenca"] = leitura.presenca;
  doc["probabilidade_vida"] = leitura.probabilidade;
  
  // Serializar JSON
  String jsonString;
  serializeJson(doc, jsonString);
  
  // Enviar via HTTP POST
  HTTPClient http;
  http.begin(serverUrl);
  http.addHeader("Content-Type", "application/json");
  
  int httpResponseCode = http.POST(jsonString);
  
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.print("✓ Dados enviados! Código: ");
    Serial.println(httpResponseCode);
    Serial.print("Resposta: ");
    Serial.println(response);
  } else {
    Serial.print("✗ Erro ao enviar dados. Código: ");
    Serial.println(httpResponseCode);
  }
  
  http.end();
}


// ===== FUNÇÕES DE CONTROLE DOS MOTORES =====

void pararMotores() {
  digitalWrite(MOTOR_ESQ_A, LOW);
  digitalWrite(MOTOR_ESQ_B, LOW);
  digitalWrite(MOTOR_DIR_A, LOW);
  digitalWrite(MOTOR_DIR_B, LOW);
}

void moverFrente() {
  digitalWrite(MOTOR_ESQ_A, HIGH);
  digitalWrite(MOTOR_ESQ_B, LOW);
  digitalWrite(MOTOR_DIR_A, HIGH);
  digitalWrite(MOTOR_DIR_B, LOW);
}

void moverTras() {
  digitalWrite(MOTOR_ESQ_A, LOW);
  digitalWrite(MOTOR_ESQ_B, HIGH);
  digitalWrite(MOTOR_DIR_A, LOW);
  digitalWrite(MOTOR_DIR_B, HIGH);
}

void virarEsquerda() {
  digitalWrite(MOTOR_ESQ_A, LOW);
  digitalWrite(MOTOR_ESQ_B, LOW);
  digitalWrite(MOTOR_DIR_A, HIGH);
  digitalWrite(MOTOR_DIR_B, LOW);
}

void virarDireita() {
  digitalWrite(MOTOR_ESQ_A, HIGH);
  digitalWrite(MOTOR_ESQ_B, LOW);
  digitalWrite(MOTOR_DIR_A, LOW);
  digitalWrite(MOTOR_DIR_B, LOW);
}

void desligarRobo() {
  roboLigado = false;
  pararMotores();
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  Serial.println("🔴 Robô DESLIGADO remotamente!");
}
