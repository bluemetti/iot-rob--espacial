# 🤖 Robô Explorador Espacial - IoT

Projeto de um robô explorador espacial que busca indícios de vida extraterrestre. O robô coleta dados de sensores, calcula probabilidade de vida e envia os dados para um backend Flask com armazenamento em InfluxDB Cloud.

## 📋 Componentes do Projeto

### Hardware
- **ESP32** (2 unidades)
  - ESP32 #1: Controle remoto (simulado no Wokwi)
  - ESP32 #2: Robô físico (laboratório)
- **Sensores:**
  - Sensor de Temperatura e Umidade (DHT22)
  - Fotorresistor (LDR)
  - Sensor de Presença (PIR)
- **Atuadores:**
  - 2x Motores DC
  - 1x LED Verde (status normal)
  - 1x LED Vermelho (alerta)
- **Controle:**
  - Joystick analógico (no Wokwi)

### Software
- **Backend:** Python + Flask
- **Banco de Dados:** InfluxDB Cloud
- **Firmware:** Arduino (C/C++)
- **Versionamento:** Git + GitHub

---

## 🚀 Configuração do Backend

### Pré-requisitos
- Python 3.8 ou superior
- Conta no InfluxDB Cloud (gratuita)
- Git

### 1. Criar conta no InfluxDB Cloud

1. Acesse [https://cloud2.influxdata.com/signup](https://cloud2.influxdata.com/signup)
2. Crie uma conta gratuita
3. Após login, anote as seguintes informações:

#### Obter URL e Organização
- No canto superior esquerdo, clique no nome da sua organização
- Copie o nome da **Organization**
- A **URL** estará no formato: `https://us-east-1-1.aws.cloud2.influxdata.com` (varia por região)

#### Criar Bucket
1. No menu lateral, clique em **Load Data** > **Buckets**
2. Clique em **Create Bucket**
3. Nome: `robo_espacial`
4. Clique em **Create**

#### Obter Token de API
1. No menu lateral, clique em **Load Data** > **API Tokens**
2. Clique em **Generate API Token** > **All Access API Token**
3. Nome: `token_robo_espacial`
4. Clique em **Save**
5. **COPIE O TOKEN** (você não poderá vê-lo novamente!)

### 2. Configurar Backend

```bash
# Navegar para o diretório do backend
cd backend

# Criar ambiente virtual Python
python3 -m venv venv

# Ativar ambiente virtual
source venv/bin/activate  # Linux/Mac
# OU
venv\Scripts\activate  # Windows

# Instalar dependências
pip install -r requirements.txt

# Copiar arquivo de configuração
cp .env.example .env

# Editar o arquivo .env com suas credenciais
nano .env  # ou use seu editor preferido
```

### 3. Editar arquivo `.env`

Abra o arquivo `.env` e preencha com suas credenciais:

```env
INFLUXDB_URL=https://us-east-1-1.aws.cloud2.influxdata.com
INFLUXDB_TOKEN=seu_token_copiado_aqui
INFLUXDB_ORG=sua_organizacao
INFLUXDB_BUCKET=robo_espacial
PORT=5000
```

### 4. Executar Backend

```bash
# Certifique-se de estar no diretório backend com ambiente virtual ativo
python app.py
```

O servidor estará rodando em: `http://localhost:5000`

---

## 📡 Endpoints da API

### `GET /`
Informações sobre a API

**Resposta:**
```json
{
  "nome": "API Robô Explorador Espacial",
  "versao": "1.0.0",
  "endpoints": {
    "POST /leituras": "Receber dados dos sensores",
    "GET /leituras": "Consultar últimas leituras",
    "GET /leituras/estatisticas": "Obter estatísticas gerais"
  }
}
```

### `POST /leituras`
Receber dados dos sensores do ESP32

**Request Body:**
```json
{
  "timestamp": "2025-10-25T14:35:00Z",
  "temperatura_c": 24.3,
  "umidade_pct": 55,
  "luminosidade": 723,
  "presenca": 1,
  "probabilidade_vida": 78.0
}
```

**Resposta (201):**
```json
{
  "mensagem": "Leitura registrada com sucesso",
  "dados": { ... }
}
```

### `GET /leituras`
Consultar últimas leituras

**Parâmetros de Query:**
- `limit` (opcional): Número de leituras (padrão: 100)
- `hours` (opcional): Últimas N horas (padrão: 24)

**Exemplo:**
```
GET /leituras?limit=50&hours=12
```

**Resposta (200):**
```json
{
  "total": 50,
  "leituras": [
    {
      "timestamp": "2025-10-25T14:35:00Z",
      "temperatura_c": 24.3,
      "umidade_pct": 55,
      "luminosidade": 723,
      "presenca": 1,
      "probabilidade_vida": 78.0
    }
  ]
}
```

### `GET /leituras/estatisticas`
Obter estatísticas das leituras

**Parâmetros de Query:**
- `hours` (opcional): Período em horas (padrão: 24)

**Exemplo:**
```
GET /leituras/estatisticas?hours=24
```

**Resposta (200):**
```json
{
  "periodo_horas": 24,
  "estatisticas": {
    "temperatura_c": {
      "media": 24.5,
      "minimo": 18.2,
      "maximo": 30.1
    },
    "umidade_pct": {
      "media": 55.3,
      "minimo": 40.0,
      "maximo": 70.0
    },
    "luminosidade": {
      "media": 650.5,
      "minimo": 100,
      "maximo": 1000
    },
    "probabilidade_vida": {
      "media": 45.2,
      "minimo": 0.0,
      "maximo": 85.0
    }
  }
}
```

### `GET /health`
Verificar status da API e conexão com InfluxDB

**Resposta (200):**
```json
{
  "status": "OK",
  "banco_dados": "conectado"
}
```

---

## 🧪 Testando a API

### Com curl:
```bash
# Testar endpoint raiz
curl http://localhost:5000/

# Enviar leitura de teste
curl -X POST http://localhost:5000/leituras \
  -H "Content-Type: application/json" \
  -d '{
    "temperatura_c": 24.5,
    "umidade_pct": 60,
    "luminosidade": 800,
    "presenca": 1,
    "probabilidade_vida": 75.0
  }'

# Consultar leituras
curl http://localhost:5000/leituras?limit=10

# Ver estatísticas
curl http://localhost:5000/leituras/estatisticas

# Verificar saúde da API
curl http://localhost:5000/health
```

### Com Python:
```python
import requests

# Enviar leitura
dados = {
    "temperatura_c": 24.5,
    "umidade_pct": 60,
    "luminosidade": 800,
    "presenca": 1,
    "probabilidade_vida": 75.0
}

response = requests.post("http://localhost:5000/leituras", json=dados)
print(response.json())
```

---

## 🔧 Configuração do ESP32

O código para o ESP32 enviar dados está na pasta `esp32/`. Consulte o arquivo `esp32/robo_fisico_http.ino` para exemplos de como enviar dados via HTTP POST.

### Configurar WiFi no ESP32:
```cpp
const char* ssid = "SUA_REDE_WIFI";
const char* password = "SUA_SENHA";
const char* serverUrl = "http://SEU_IP:5000/leituras";
```

---

## 📁 Estrutura do Projeto

```
iot-rob--espacial/
├── backend/
│   ├── app.py                    # API Flask principal
│   ├── influxdb_service.py       # Serviço de integração com InfluxDB
│   ├── requirements.txt          # Dependências Python
│   ├── .env.example              # Exemplo de configuração
│   ├── .env                      # Configuração (NÃO VERSIONAR!)
│   └── .gitignore                # Arquivos ignorados pelo Git
├── esp32/
│   ├── controle_remoto_wokwi/    # Código do controle (Wokwi)
│   └── robo_fisico/              # Código do robô (laboratório)
└── README.md                     # Este arquivo
```

---

## 🐛 Troubleshooting

### Erro: "Connection refused" ao conectar no InfluxDB
- Verifique se a URL está correta no `.env`
- Teste a conexão com: `curl http://localhost:5000/health`

### Erro: "Unauthorized"
- Verifique se o token está correto
- Certifique-se de que o token tem permissões de leitura e escrita

### ESP32 não consegue enviar dados
- Verifique se o backend está rodando: `curl http://localhost:5000/`
- Verifique se o ESP32 está na mesma rede ou se o IP está correto
- Use o IP local do computador (ex: `192.168.1.100:5000`)

---

## 📊 Visualizando Dados no InfluxDB

1. Acesse o InfluxDB Cloud: [https://cloud2.influxdata.com](https://cloud2.influxdata.com)
2. Clique em **Data Explorer** no menu lateral
3. Selecione o bucket `robo_espacial`
4. Selecione a measurement `leitura_sensores`
5. Escolha os campos que deseja visualizar (temperatura_c, umidade_pct, etc.)
6. Clique em **Submit** para ver o gráfico

---

## 🎯 Próximos Passos

- [ ] Implementar autenticação na API (JWT)
- [ ] Adicionar dashboard web para visualização em tempo real
- [ ] Implementar buffer local no ESP32 para dados offline
- [ ] Adicionar mais algoritmos de ML para detecção de vida
- [ ] Criar alertas personalizados no InfluxDB

---

## 📝 Licença

Projeto acadêmico - Práticas Integradas: Camada de Serviço

---

## 🔗 Links Úteis

- [Flask Documentation](https://flask.palletsprojects.com/)
- [InfluxDB Cloud](https://docs.influxdata.com/influxdb/cloud/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)
- [Wokwi Simulator](https://wokwi.com/)