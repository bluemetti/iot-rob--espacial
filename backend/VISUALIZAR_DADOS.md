# 📊 Como Visualizar os Dados no InfluxDB Cloud

## 🌐 Acesso ao InfluxDB Cloud

### 1️⃣ **Fazer Login**

Acesse: **https://us-east-1-1.aws.cloud2.influxdata.com**

Suas credenciais:
- **Organização**: `bluemetti's Org`
- **Bucket**: `robo_espacial`

### 2️⃣ **Navegar até o Data Explorer**

1. No menu lateral esquerdo, clique em **"Data Explorer"** (ícone de gráfico 📊)
2. Você verá uma interface para construir queries e visualizar dados

## 📈 Visualizar Dados - Opção 1: Interface Gráfica

### Passo a Passo:

1. **Selecionar o Bucket**
   - Na seção "FROM", selecione: `robo_espacial`

2. **Selecionar a Measurement**
   - Em "_measurement", selecione: `leitura_sensores`

3. **Selecionar os Campos (Fields)**
   - Marque os campos que deseja visualizar:
     - ✅ `temperatura_c` (Temperatura em °C)
     - ✅ `umidade_pct` (Umidade em %)
     - ✅ `luminosidade` (Sensor de luz)
     - ✅ `presenca` (Detecção de movimento)
     - ✅ `probabilidade_vida` (% de probabilidade)

4. **Definir o Período**
   - No canto superior direito, selecione o período:
     - "Past 1h" (Última hora)
     - "Past 6h" (Últimas 6 horas)
     - "Past 24h" (Últimas 24 horas)
     - Ou defina um período customizado

5. **Visualizar**
   - Clique no botão **"Submit"** (ou ícone ▶️)
   - Os gráficos aparecerão automaticamente!

## 🔍 Visualizar Dados - Opção 2: Query Flux

### Query Básica - Últimos Dados

Clique em **"Script Editor"** e cole esta query:

```flux
from(bucket: "robo_espacial")
  |> range(start: -1h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "temperatura_c" 
    or r["_field"] == "umidade_pct" 
    or r["_field"] == "luminosidade"
    or r["_field"] == "presenca"
    or r["_field"] == "probabilidade_vida")
```

### Query - Estatísticas (Média, Mín, Máx)

```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "temperatura_c")
  |> aggregateWindow(every: 1h, fn: mean)
```

### Query - Apenas Quando Houver Presença

```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "presenca")
  |> filter(fn: (r) => r["_value"] == 1)
```

### Query - Probabilidade de Vida Alta (>75%)

```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "probabilidade_vida")
  |> filter(fn: (r) => r["_value"] > 75.0)
```

## 📊 Tipos de Visualização

O InfluxDB oferece vários tipos de gráficos:

1. **📈 Line Graph** (Padrão) - Melhor para temperatura, umidade
2. **📊 Bar Chart** - Bom para presença (0/1)
3. **🔢 Single Stat** - Mostra último valor ou média
4. **📉 Gauge** - Indicador visual para probabilidade de vida
5. **🗓️ Heatmap** - Padrões ao longo do tempo
6. **📋 Table** - Visualizar dados brutos

Para mudar o tipo de visualização:
- Clique no menu suspenso acima do gráfico
- Selecione o tipo desejado

## 🎨 Criar um Dashboard

### 1. Criar Novo Dashboard

1. No menu lateral, clique em **"Dashboards"**
2. Clique em **"Create Dashboard"**
3. Dê um nome: `Robô Explorador Espacial`

### 2. Adicionar Células (Cells)

**Cell 1 - Temperatura em Tempo Real:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -1h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "temperatura_c")
```
- Tipo: Line Graph
- Nome: "🌡️ Temperatura"

**Cell 2 - Umidade:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -1h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "umidade_pct")
```
- Tipo: Line Graph
- Nome: "💧 Umidade"

**Cell 3 - Probabilidade de Vida:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -1h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "probabilidade_vida")
  |> last()
```
- Tipo: Gauge
- Nome: "🧬 Probabilidade de Vida"
- Configurar: Min=0, Max=100

**Cell 4 - Detecções de Presença:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "presenca")
  |> filter(fn: (r) => r["_value"] == 1)
  |> count()
```
- Tipo: Single Stat
- Nome: "👤 Detecções de Presença (24h)"

## 🔔 Configurar Alertas (Opcional)

1. Vá em **"Alerts"** no menu lateral
2. Crie um alerta para condições críticas:
   - Temperatura > 35°C
   - Umidade < 30% ou > 80%
   - Probabilidade de vida < 50%

## 💡 Dicas

### ✅ **Melhores Práticas:**
- Use **"Past 1h"** para ver dados recentes
- Ative **"Auto Refresh"** para atualização automática
- Salve suas queries favoritas
- Organize dashboards por funcionalidade

### 🎯 **Queries Úteis:**

**Última leitura de cada sensor:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -1h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> last()
```

**Média das últimas 24 horas:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> mean()
```

**Contar leituras por hora:**
```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "temperatura_c")
  |> aggregateWindow(every: 1h, fn: count)
```

## 📱 Acesso Mobile

O InfluxDB Cloud também funciona em dispositivos móveis!
- Acesse a mesma URL pelo navegador do celular
- Os dashboards são responsivos

## 🔗 Links Rápidos

- **InfluxDB Cloud**: https://us-east-1-1.aws.cloud2.influxdata.com
- **Documentação Flux**: https://docs.influxdata.com/flux/
- **Exemplos de Queries**: https://docs.influxdata.com/influxdb/cloud/query-data/flux/

---

## 🎬 Resumo Rápido

1. Acesse: https://us-east-1-1.aws.cloud2.influxdata.com
2. Clique em **"Data Explorer"**
3. Selecione bucket `robo_espacial`
4. Selecione measurement `leitura_sensores`
5. Marque os campos desejados
6. Clique em **"Submit"**
7. 🎉 Visualize seus dados!
