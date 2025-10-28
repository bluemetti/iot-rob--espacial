# 🤖 Guia de Configuração do ESP32

## 📋 Checklist de Configuração

### 1️⃣ **Configurações WiFi** (Linhas 13-14)
```cpp
const char* ssid = "SEU_WIFI_AQUI";      // ← Nome da sua rede WiFi
const char* password = "SUA_SENHA_AQUI"; // ← Senha do WiFi
```

### 2️⃣ **URL do Backend** (Linha 23)
```cpp
const char* backend_url = "http://10.0.0.129:5000/leituras";
```

**Seu IP atual:** `10.0.0.129`

⚠️ **IMPORTANTE:** O ESP32 e o computador com o Flask precisam estar na **mesma rede WiFi**!

---

## 🚀 Como Fazer Upload do Código

### **1. Abrir Arduino IDE**
- Abra o arquivo: `robo_espacial_ATUALIZADO.ino`

### **2. Configurar a Placa**
- **Ferramentas** → **Placa** → **ESP32 Dev Module**

### **3. Selecionar a Porta**
- **Ferramentas** → **Porta** → Selecione a porta COM do ESP32

### **4. Instalar Bibliotecas Necessárias**
Vá em **Ferramentas** → **Gerenciar Bibliotecas** e instale:
- ✅ `DHT sensor library` (by Adafruit)
- ✅ `Adafruit Unified Sensor`
- ✅ `PubSubClient`
- ✅ `ESP32Servo`
- ✅ `ArduinoJson`
- ✅ `UrlEncode`

### **5. Fazer Upload**
- Clique no botão **Upload** (→)
- Aguarde compilar e fazer upload

---

## 🔍 Verificando Funcionamento

### **1. Abrir Monitor Serial**
- **Ferramentas** → **Monitor Serial**
- **Baud rate:** 115200

### **2. O que você deve ver:**
```
Iniciando Robô Explorador...
Conectando a SEU_WIFI
..........
WiFi conectado!
Endereço IP: 192.168.x.x
Backend URL: http://10.0.0.129:5000/leituras
Sistema pronto!

---- LEITURA DOS SENSORES ----
Temperatura: 25.3 °C
Umidade: 60.0 %
Luz (LDR): 512
Presença detectada!
-------------------------------

✅ Dados enviados! Código: 201
📊 Leitura registrada no InfluxDB Cloud!
```

---

## 📊 Formato dos Dados Enviados

O ESP32 envia este JSON a cada **2 segundos**:

```json
{
  "temperatura_c": 25.3,
  "umidade_pct": 60.0,
  "luminosidade": 512,
  "presenca": 1,
  "probabilidade_vida": 75.0
}
```

---

## 🛠️ Troubleshooting

### ❌ **Erro: WiFi desconectado**
**Solução:** Verifique se o SSID e senha estão corretos

### ❌ **Erro ao enviar. Código: -1**
**Solução:** 
1. Verifique se o Flask está rodando: `python3 app.py`
2. Confirme que o IP está correto
3. ESP32 e PC devem estar na mesma rede

### ❌ **Erro ao enviar. Código: 400**
**Solução:** Verifique se todos os campos estão sendo enviados corretamente

### ❌ **Erro ao enviar. Código: 500**
**Solução:** Problema no backend. Verifique as credenciais do InfluxDB no arquivo `.env`

---

## ✅ Checklist Final

Antes de ligar o robô:

- [ ] WiFi configurado corretamente
- [ ] IP do backend atualizado
- [ ] Servidor Flask rodando (`python3 app.py`)
- [ ] Bibliotecas instaladas
- [ ] Código compilado sem erros
- [ ] Monitor Serial aberto (115200 baud)

---

## 📡 Testando a Conexão

### **No computador, execute:**
```bash
# Iniciar o servidor Flask
cd backend
python3 app.py
```

Você deve ver:
```
* Running on http://10.0.0.129:5000
```

### **No ESP32:**
Quando ligar, deve conectar ao WiFi e começar a enviar dados automaticamente!

### **Verificar dados no InfluxDB:**
1. Acesse: https://us-east-1-1.aws.cloud2.influxdata.com
2. Vá em **Data Explorer**
3. Selecione o bucket `robo_espacial`
4. Escolha `_measurement` = `leitura_sensores`
5. Visualize os gráficos em tempo real! 📊

---

## 🎯 Fluxo Completo

```
ESP32 (Robô Físico)
    ↓ HTTP POST a cada 2s
Flask API (Backend)
    ↓ Salva dados
InfluxDB Cloud
    ↓ Visualização
Dashboard Web
```

---

## 💡 Dicas

- O robô envia dados **automaticamente** a cada 2 segundos
- Não precisa fazer nada, só ligar!
- Os LEDs mostram o estado:
  - 🟢 Verde: Exploração normal
  - 🔴 Vermelho: Alta probabilidade de vida detectada
- Você pode monitorar tudo pelo **Monitor Serial** do Arduino IDE
