# Backend Flask - Robô Explorador Espacial

Backend em Python usando Flask para receber e armazenar dados dos sensores do robô explorador espacial no InfluxDB Cloud.

## 📋 Estrutura

```
backend/
├── app.py                    # API Flask principal
├── influxdb_service.py       # Serviço de integração com InfluxDB
├── requirements.txt          # Dependências Python
├── .env.example              # Exemplo de configuração
└── README.md                 # Este arquivo
```

## 🚀 Instalação e Configuração

### 1. Criar ambiente virtual

```bash
# Navegar para o diretório backend
cd backend

# Criar ambiente virtual
python3 -m venv venv

# Ativar ambiente virtual
source venv/bin/activate  # Linux/Mac
# OU
venv\Scripts\activate  # Windows
```

### 2. Instalar dependências

```bash
pip install -r requirements.txt
```

### 3. Configurar InfluxDB Cloud

1. Acesse [InfluxDB Cloud](https://cloud2.influxdata.com/signup) e crie uma conta gratuita
2. Crie um bucket chamado `robo_espacial`
3. Gere um API Token com permissões de leitura e escrita
4. Copie o arquivo `.env.example` para `.env`:

```bash
cp .env.example .env
```

5. Edite o arquivo `.env` com suas credenciais:

```env
INFLUXDB_URL=https://us-east-1-1.aws.cloud2.influxdata.com
INFLUXDB_TOKEN=seu_token_aqui
INFLUXDB_ORG=sua_organizacao
INFLUXDB_BUCKET=robo_espacial
PORT=5000
```

### 4. Executar o servidor

```bash
python app.py
```

O servidor estará rodando em: `http://localhost:5000`

## 📡 Endpoints

### `GET /`
Informações da API

### `POST /leituras`
Receber dados dos sensores

**Body:**
```json
{
  "temperatura_c": 24.3,
  "umidade_pct": 55,
  "luminosidade": 723,
  "presenca": 1,
  "probabilidade_vida": 78.0
}
```

### `GET /leituras?limit=100&hours=24`
Consultar últimas leituras

### `GET /leituras/estatisticas?hours=24`
Obter estatísticas (média, mín, máx)

### `GET /health`
Verificar status da API e conexão com InfluxDB

## 🧪 Testar

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
```

## 🔧 Produção

Para produção, use um servidor WSGI como Gunicorn:

```bash
gunicorn -w 4 -b 0.0.0.0:5000 app:app
```

## 📝 Notas

- O arquivo `.env` contém credenciais sensíveis e **NÃO** deve ser commitado no Git
- Use sempre o arquivo `.env.example` como referência
- O InfluxDB Cloud oferece plano gratuito com 30 dias de retenção de dados
