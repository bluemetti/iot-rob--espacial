"""
Simulador de Robô Explorador Espacial
Envia dados simulados para a API como se fosse o ESP32 real
"""

import requests
import random
import time
from datetime import datetime
import json

# Configuração da API
API_URL = "http://localhost:5000/leituras"

def gerar_dados_simulados():
    """
    Gera dados de sensores simulados com valores realistas
    """
    # Temperatura: 20-30°C (ambiente espacial controlado)
    temperatura = round(random.uniform(20.0, 30.0), 2)
    
    # Umidade: 40-70% (ambiente controlado)
    umidade = round(random.uniform(40.0, 70.0), 1)
    
    # Luminosidade: 0-1023 (sensor analógico)
    luminosidade = random.randint(0, 1023)
    
    # Presença: 0 ou 1 (sensor PIR)
    presenca = random.randint(0, 1)
    
    # Probabilidade de vida: 0-100% (calculado baseado nos sensores)
    # Condições ideais: temp 22-28°C, umidade 45-65%, luminosidade média
    probabilidade = 50.0  # base
    
    # Ajustar baseado na temperatura
    if 22 <= temperatura <= 28:
        probabilidade += 20
    elif 20 <= temperatura < 22 or 28 < temperatura <= 30:
        probabilidade += 10
    
    # Ajustar baseado na umidade
    if 45 <= umidade <= 65:
        probabilidade += 20
    elif 40 <= umidade < 45 or 65 < umidade <= 70:
        probabilidade += 10
    
    # Ajustar baseado na luminosidade
    if 300 <= luminosidade <= 700:
        probabilidade += 10
    
    probabilidade = min(100.0, max(0.0, probabilidade))  # Limitar entre 0-100
    probabilidade = round(probabilidade, 1)
    
    dados = {
        "timestamp": datetime.now().strftime("%Y-%m-%dT%H:%M:%S.000Z"),
        "temperatura_c": temperatura,
        "umidade_pct": umidade,
        "luminosidade": luminosidade,
        "presenca": presenca,
        "probabilidade_vida": probabilidade
    }
    
    return dados

def enviar_leitura(dados):
    """
    Envia dados para a API
    """
    try:
        response = requests.post(API_URL, json=dados, timeout=5)
        
        if response.status_code == 201:
            print(f"✅ Leitura enviada com sucesso!")
            print(f"   Temp: {dados['temperatura_c']}°C | "
                  f"Umidade: {dados['umidade_pct']}% | "
                  f"Luz: {dados['luminosidade']} | "
                  f"Presença: {dados['presenca']} | "
                  f"Prob.Vida: {dados['probabilidade_vida']}%")
        else:
            print(f"❌ Erro ao enviar: {response.status_code}")
            print(f"   Resposta: {response.text}")
            
        return response.status_code == 201
        
    except requests.exceptions.ConnectionError:
        print("❌ Erro: Não foi possível conectar à API. Certifique-se que está rodando!")
        return False
    except Exception as e:
        print(f"❌ Erro inesperado: {e}")
        return False

def modo_continuo(intervalo=5, total_leituras=None):
    """
    Envia leituras continuamente em intervalos regulares
    
    Args:
        intervalo: Segundos entre cada leitura (padrão: 5)
        total_leituras: Número total de leituras (None = infinito)
    """
    print("\n🤖 SIMULADOR DE ROBÔ ESPACIAL")
    print("=" * 50)
    print(f"📡 Enviando dados para: {API_URL}")
    print(f"⏱️  Intervalo: {intervalo} segundos")
    print(f"📊 Total de leituras: {'∞ (infinito)' if total_leituras is None else total_leituras}")
    print("=" * 50)
    print("\nPressione Ctrl+C para parar\n")
    
    contador = 0
    sucessos = 0
    
    try:
        while True:
            contador += 1
            print(f"\n📍 Leitura #{contador} - {datetime.now().strftime('%H:%M:%S')}")
            
            dados = gerar_dados_simulados()
            if enviar_leitura(dados):
                sucessos += 1
            
            # Parar se atingiu o total desejado
            if total_leituras and contador >= total_leituras:
                break
            
            # Aguardar antes da próxima leitura
            time.sleep(intervalo)
            
    except KeyboardInterrupt:
        print("\n\n🛑 Simulação interrompida pelo usuário")
    
    print("\n" + "=" * 50)
    print(f"📈 Estatísticas:")
    print(f"   Total de leituras: {contador}")
    print(f"   Sucessos: {sucessos}")
    print(f"   Falhas: {contador - sucessos}")
    print("=" * 50)

def modo_teste_unico():
    """
    Envia apenas uma leitura de teste
    """
    print("\n🧪 TESTE ÚNICO")
    print("=" * 50)
    
    dados = gerar_dados_simulados()
    print("\n📦 Dados gerados:")
    print(json.dumps(dados, indent=2))
    
    print(f"\n📡 Enviando para: {API_URL}")
    enviar_leitura(dados)
    print("\n" + "=" * 50)

def menu():
    """
    Menu interativo para escolher o modo de operação
    """
    print("\n" + "=" * 50)
    print("🤖 SIMULADOR DE ROBÔ EXPLORADOR ESPACIAL")
    print("=" * 50)
    print("\nEscolha uma opção:")
    print("1 - Teste único (enviar 1 leitura)")
    print("2 - Modo contínuo (5 segundos entre leituras)")
    print("3 - Modo contínuo rápido (1 segundo entre leituras)")
    print("4 - Enviar 10 leituras de teste")
    print("5 - Modo personalizado")
    print("0 - Sair")
    print("=" * 50)
    
    opcao = input("\n👉 Opção: ").strip()
    
    if opcao == "1":
        modo_teste_unico()
    elif opcao == "2":
        modo_continuo(intervalo=5)
    elif opcao == "3":
        modo_continuo(intervalo=1)
    elif opcao == "4":
        modo_continuo(intervalo=2, total_leituras=10)
    elif opcao == "5":
        try:
            intervalo = int(input("Intervalo em segundos: "))
            total = input("Total de leituras (deixe vazio para infinito): ").strip()
            total_leituras = int(total) if total else None
            modo_continuo(intervalo=intervalo, total_leituras=total_leituras)
        except ValueError:
            print("❌ Valores inválidos!")
    elif opcao == "0":
        print("\n👋 Até logo!")
        return
    else:
        print("\n❌ Opção inválida!")
    
    # Voltar ao menu
    input("\n\nPressione Enter para voltar ao menu...")
    menu()

if __name__ == "__main__":
    try:
        menu()
    except KeyboardInterrupt:
        print("\n\n👋 Até logo!")
