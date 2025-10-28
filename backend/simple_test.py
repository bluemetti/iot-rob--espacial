"""
Teste simples e direto - Envia uma leitura e mostra o resultado
"""
import requests
import json
from datetime import datetime

# Dados de teste
dados = {
    "timestamp": datetime.now().strftime("%Y-%m-%dT%H:%M:%S.000Z"),
    "temperatura_c": 25.5,
    "umidade_pct": 60.0,
    "luminosidade": 750,
    "distancia_cm": 120.5
}

print("=" * 60)
print("🧪 TESTE SIMPLES - ROBÔ ESPACIAL")
print("=" * 60)
print("\n📦 Dados a enviar:")
print(json.dumps(dados, indent=2))

print("\n📡 Enviando para: http://localhost:5000/leituras")

try:
    response = requests.post(
        "http://localhost:5000/leituras",
        json=dados,
        timeout=5
    )
    
    print(f"\n📊 Status: {response.status_code}")
    print(f"📄 Resposta:")
    print(json.dumps(response.json(), indent=2))
    
    if response.status_code == 201:
        print("\n✅ SUCESSO! Dados enviados ao InfluxDB Cloud!")
    else:
        print(f"\n❌ Erro: {response.status_code}")
        
except requests.exceptions.ConnectionError:
    print("\n❌ ERRO: Servidor Flask não está rodando!")
    print("   Execute: python3 app.py")
except Exception as e:
    print(f"\n❌ Erro inesperado: {e}")

print("\n" + "=" * 60)
