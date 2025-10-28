# 🧪 Testando o Sistema sem o Robô Físico

Este diretório contém ferramentas para testar todo o sistema sem precisar do ESP32 físico.

## Opções de Teste

### 1️⃣ Simulador Interativo (Recomendado)

O simulador possui um menu interativo com várias opções:

```bash
cd backend
python3 test_simulator.py
```

**Opções disponíveis:**
- **Teste único**: Envia apenas 1 leitura para testar a conexão
- **Modo contínuo**: Envia leituras a cada 5 segundos (simula o robô real)
- **Modo rápido**: Envia leituras a cada 1 segundo (para testes rápidos)
- **10 leituras**: Envia 10 leituras e para (bom para análises)
- **Modo personalizado**: Configure intervalo e quantidade

### 2️⃣ Script Automático Completo

Inicia o servidor Flask e o simulador automaticamente:

```bash
cd backend
./run_test.sh
```

Este script:
- Verifica as dependências
- Inicia o servidor Flask
- Executa o simulador
- Limpa tudo ao finalizar

### 3️⃣ Teste Manual com cURL

Se preferir testar manualmente, primeiro inicie o servidor:

```bash
cd backend
python3 app.py
```

Em outro terminal, envie dados de teste:

```bash
curl -X POST http://localhost:5000/leituras \
  -H "Content-Type: application/json" \
  -d '{
    "timestamp": "2025-10-28T15:30:00.000Z",
    "temperatura_c": 24.5,
    "umidade_pct": 55.0,
    "luminosidade": 512,
    "distancia_cm": 150.0
  }'
```

### 4️⃣ Consultar Dados

Após enviar leituras, consulte os dados:

```bash
# Últimas leituras
curl http://localhost:5000/leituras

# Estatísticas
curl http://localhost:5000/leituras/estatisticas
```

## 📊 Dados Simulados

O simulador gera dados realistas:
- **Temperatura**: 20-30°C (ambiente controlado)
- **Umidade**: 40-70%
- **Luminosidade**: 0-1023 (valor analógico)
- **Distância**: 2-400 cm (alcance do sensor ultrassônico)

## 🔍 Verificação

Para verificar se os dados estão sendo salvos no InfluxDB Cloud:

1. Acesse: https://us-east-1-1.aws.cloud2.influxdata.com
2. Faça login com suas credenciais
3. Vá em **Data Explorer**
4. Selecione o bucket `robo_espacial`
5. Visualize os gráficos dos dados

## 💡 Dicas

- **Problema de conexão?** Certifique-se que o servidor Flask está rodando (`python3 app.py`)
- **Erro 500?** Verifique as credenciais do InfluxDB no arquivo `.env`
- **Quer mais dados?** Use o modo rápido para popular o banco rapidamente
- **Simulação realista?** Use o modo contínuo de 5 segundos (igual ao robô real)

## 🎯 Fluxo de Teste Recomendado

1. **Primeiro teste**: Use "Teste único" para verificar se tudo funciona
2. **Popular dados**: Use "10 leituras de teste" para ter dados iniciais
3. **Simular operação**: Use "Modo contínuo" para simular o robô em operação
4. **Verificar**: Consulte os dados via API ou InfluxDB Cloud

## 🚨 Troubleshooting

### Erro: "Não foi possível conectar à API"
- O servidor Flask não está rodando
- Execute: `python3 app.py` em outro terminal

### Erro: "Erro ao salvar no InfluxDB"
- Verifique as credenciais no arquivo `.env`
- Teste a conexão com o InfluxDB Cloud

### Erro: "ModuleNotFoundError"
- Instale as dependências: `pip install -r requirements.txt`
- Ou use o ambiente virtual: `source venv/bin/activate`
