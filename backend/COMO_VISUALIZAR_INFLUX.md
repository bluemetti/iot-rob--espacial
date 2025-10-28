# 📊 Como Visualizar os Dados no InfluxDB Cloud

## ✅ Dados Confirmados!
**25 registros encontrados** no bucket `robo_espacial`

---

## 🔐 Passo 1: Acessar o InfluxDB Cloud

1. Abra seu navegador e acesse:
   ```
   https://us-east-1-1.aws.cloud2.influxdata.com
   ```

2. Faça login com suas credenciais

---

## 📊 Passo 2: Ir para o Data Explorer

1. No menu lateral esquerdo, clique em **"Data Explorer"** (ícone de gráfico 📈)

---

## 🔍 Passo 3: Configurar a Query

### Opção A: Query Builder (Visual)

1. **FROM**: Selecione o bucket `robo_espacial`
2. **Measurement**: Selecione `leitura_sensores`
3. **Fields**: Marque os campos que quer visualizar:
   - ✅ `temperatura_c`
   - ✅ `umidade_pct`
   - ✅ `luminosidade`
   - ✅ `presenca`
   - ✅ `probabilidade_vida`
4. **Time Range**: Selecione **"Past 1h"** ou **"Past 24h"** no canto superior direito
5. Clique em **"Submit"** (botão azul no canto superior direito)

### Opção B: Script Editor (Avançado)

1. Clique em **"Script Editor"** (no topo, ao lado de "Query Builder")
2. Cole esta query:

```flux
from(bucket: "robo_espacial")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> filter(fn: (r) => r["_field"] == "temperatura_c" or 
                       r["_field"] == "umidade_pct" or 
                       r["_field"] == "luminosidade" or
                       r["_field"] == "probabilidade_vida")
```

3. Clique em **"Submit"**

---

## 📈 Passo 4: Visualizar Gráficos

Após executar a query, você verá:

- **Gráfico de linhas** mostrando a evolução dos valores
- **Tabela** com todos os dados
- **Estatísticas** de cada métrica

### Dicas de Visualização:

1. **Trocar tipo de gráfico**: Clique no ícone de gráfico no canto superior direito
   - Line Graph (linhas)
   - Bar Chart (barras)
   - Table (tabela)
   - Gauge (medidor)

2. **Filtrar por campo específico**: 
   - Use o menu dropdown "Fields" para selecionar apenas um campo
   - Exemplo: Ver só a temperatura

3. **Ajustar período**: 
   - Clique no seletor de tempo (canto superior direito)
   - Opções: Past 5m, 15m, 1h, 6h, 24h, 7d, 30d

---

## 🎯 Dados que Você Deve Ver

Com base nos dados salvos, você deve ver:

| Horário | Temperatura | Umidade | Luminosidade | Prob. Vida |
|---------|-------------|---------|--------------|------------|
| 12:32:03 | 25.5°C | 60.0% | 512 | 75.0% |
| 12:33:56 | 29.16°C | 41.4% | 235 | 70.0% |
| 12:33:59 | 29.42°C | 63.1% | 847 | 80.0% |
| 12:34:02 | 25.53°C | 58.5% | 276 | 90.0% |
| 12:34:05 | 28.06°C | 42.9% | 822 | 70.0% |
| 12:34:08 | 20.45°C | 52.4% | 934 | 80.0% |

---

## 🚨 Problemas Comuns

### ❌ "No data to display"

**Solução:**
1. Verifique o **Time Range** (período de tempo)
   - Mude para "Past 1h" ou "Past 24h"
2. Certifique-se de ter selecionado:
   - Bucket: `robo_espacial`
   - Measurement: `leitura_sensores`
3. Clique em **Submit** novamente

### ❌ Não vejo o bucket "robo_espacial"

**Solução:**
1. Verifique se está na organização correta: `bluemetti's Org`
2. Clique no seletor de organização (canto superior esquerdo)
3. Se não aparecer, o bucket pode ter sido criado em outra org

### ❌ Gráfico vazio

**Solução:**
1. Verifique se selecionou os **Fields** corretos
2. Expanda o período de tempo (Time Range)
3. Use a query Flux manual (Opção B acima)

---

## 📱 Criar Dashboard (Opcional)

Para criar um dashboard visual:

1. Vá em **Dashboards** no menu lateral
2. Clique em **"Create Dashboard"**
3. Clique em **"Add Cell"**
4. Configure a query (igual ao Data Explorer)
5. Escolha o tipo de visualização
6. Clique em **"Save"**

Repita para criar múltiplos gráficos!

---

## 🔗 Links Úteis

- **InfluxDB Cloud**: https://us-east-1-1.aws.cloud2.influxdata.com
- **Documentação Flux**: https://docs.influxdata.com/flux/
- **Data Explorer**: Acesso direto pelo menu lateral

---

## ✅ Confirmação de Dados

Execute este comando para confirmar que há dados:

```bash
cd /workspaces/iot-rob--espacial/backend
python3 << 'PYTHON'
from influxdb_service import InfluxDBService
import os
from dotenv import load_dotenv

load_dotenv()

influx = InfluxDBService(
    url=os.getenv('INFLUXDB_URL'),
    token=os.getenv('INFLUXDB_TOKEN'),
    org=os.getenv('INFLUXDB_ORG'),
    bucket=os.getenv('INFLUXDB_BUCKET')
)

query = f'''
from(bucket: "{os.getenv('INFLUXDB_BUCKET')}")
  |> range(start: -24h)
  |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
  |> count()
'''

result = influx.query_api.query(query)
for table in result:
    for record in table.records:
        print(f"✅ Total de registros: {record.get_value()}")
PYTHON
```

**Resultado esperado**: `✅ Total de registros: 25` (ou mais)
