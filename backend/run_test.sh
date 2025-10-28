#!/bin/bash

# Script para testar o sistema completo sem o robô físico

echo "🚀 TESTE COMPLETO DO SISTEMA - ROBÔ ESPACIAL"
echo "=============================================="
echo ""

# Verificar se o Python está instalado
if ! command -v python3 &> /dev/null; then
    echo "❌ Python 3 não encontrado. Por favor, instale o Python 3."
    exit 1
fi

# Verificar se está no diretório correto
if [ ! -f "app.py" ]; then
    echo "❌ Execute este script dentro do diretório backend/"
    exit 1
fi

# Verificar se as dependências estão instaladas
echo "📦 Verificando dependências..."
if [ ! -d "venv" ]; then
    echo "⚠️  Ambiente virtual não encontrado. Criando..."
    python3 -m venv venv
fi

source venv/bin/activate
pip install -q -r requirements.txt

echo "✅ Dependências verificadas"
echo ""

# Iniciar o servidor Flask em background
echo "🌐 Iniciando servidor Flask..."
python3 app.py &
SERVER_PID=$!

# Aguardar servidor iniciar
echo "⏳ Aguardando servidor inicializar..."
sleep 3

# Verificar se o servidor está rodando
if ps -p $SERVER_PID > /dev/null; then
    echo "✅ Servidor rodando (PID: $SERVER_PID)"
else
    echo "❌ Erro ao iniciar o servidor"
    exit 1
fi

echo ""
echo "🤖 Iniciando simulador de robô..."
echo ""

# Executar o simulador
python3 test_simulator.py

# Limpar ao sair
echo ""
echo "🛑 Encerrando servidor..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null

echo "✅ Teste finalizado!"
