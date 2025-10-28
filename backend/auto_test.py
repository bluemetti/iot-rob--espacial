"""
Script de teste automático - Envia 10 leituras
"""
import sys
import os
sys.path.append(os.path.dirname(__file__))

from test_simulator import modo_continuo

if __name__ == "__main__":
    print("🚀 Iniciando teste automático com 10 leituras...\n")
    modo_continuo(intervalo=2, total_leituras=10)
