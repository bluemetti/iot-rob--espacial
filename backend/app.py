"""
API Flask para receber e armazenar dados do robô explorador espacial
Integração com InfluxDB Cloud
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
from datetime import datetime
import os
from dotenv import load_dotenv
from influxdb_service import InfluxDBService

# Carregar variáveis de ambiente
load_dotenv()

app = Flask(__name__)
CORS(app)  # Permitir requisições de outros domínios

# Inicializar serviço do InfluxDB
influx_service = InfluxDBService(
    url=os.getenv('INFLUXDB_URL'),
    token=os.getenv('INFLUXDB_TOKEN'),
    org=os.getenv('INFLUXDB_ORG'),
    bucket=os.getenv('INFLUXDB_BUCKET')
)


@app.route('/', methods=['GET'])
def home():
    """Endpoint raiz - informações da API"""
    return jsonify({
        'nome': 'API Robô Explorador Espacial',
        'versao': '1.0.0',
        'endpoints': {
            'POST /leituras': 'Receber dados dos sensores',
            'GET /leituras': 'Consultar últimas leituras',
            'GET /leituras/estatisticas': 'Obter estatísticas gerais'
        }
    }), 200


@app.route('/leituras', methods=['POST'])
def receber_leitura():
    """
    Endpoint para receber dados dos sensores do ESP32
    Formato esperado:
    {
        "timestamp": "2025-09-02T14:35:00Z",
        "temperatura_c": 24.3,
        "umidade_pct": 55,
        "luminosidade": 723,
        "presenca": 1,
        "probabilidade_vida": 78.0
    }
    """
    try:
        dados = request.get_json()
        
        # Validar dados recebidos
        campos_obrigatorios = [
            'temperatura_c', 'umidade_pct', 'luminosidade', 
            'presenca', 'probabilidade_vida'
        ]
        
        for campo in campos_obrigatorios:
            if campo not in dados:
                return jsonify({
                    'erro': f'Campo obrigatório ausente: {campo}'
                }), 400
        
        # Se não houver timestamp, usar o atual
        if 'timestamp' not in dados:
            dados['timestamp'] = datetime.utcnow().isoformat() + 'Z'
        
        # Salvar no InfluxDB
        sucesso = influx_service.salvar_leitura(dados)
        
        if sucesso:
            return jsonify({
                'mensagem': 'Leitura registrada com sucesso',
                'dados': dados
            }), 201
        else:
            return jsonify({
                'erro': 'Falha ao salvar no banco de dados'
            }), 500
            
    except Exception as e:
        return jsonify({
            'erro': f'Erro ao processar requisição: {str(e)}'
        }), 500


@app.route('/leituras', methods=['GET'])
def consultar_leituras():
    """
    Endpoint para consultar últimas leituras
    Parâmetros de query:
    - limit: número de leituras a retornar (padrão: 100)
    - hours: buscar leituras das últimas N horas (padrão: 24)
    """
    try:
        limit = request.args.get('limit', default=100, type=int)
        hours = request.args.get('hours', default=24, type=int)
        
        # Buscar leituras no InfluxDB
        leituras = influx_service.buscar_leituras(limit=limit, hours=hours)
        
        return jsonify({
            'total': len(leituras),
            'leituras': leituras
        }), 200
        
    except Exception as e:
        return jsonify({
            'erro': f'Erro ao consultar leituras: {str(e)}'
        }), 500


@app.route('/leituras/estatisticas', methods=['GET'])
def obter_estatisticas():
    """
    Endpoint para obter estatísticas das leituras
    Parâmetros de query:
    - hours: calcular estatísticas das últimas N horas (padrão: 24)
    """
    try:
        hours = request.args.get('hours', default=24, type=int)
        
        # Buscar estatísticas no InfluxDB
        stats = influx_service.buscar_estatisticas(hours=hours)
        
        return jsonify({
            'periodo_horas': hours,
            'estatisticas': stats
        }), 200
        
    except Exception as e:
        return jsonify({
            'erro': f'Erro ao calcular estatísticas: {str(e)}'
        }), 500


@app.route('/health', methods=['GET'])
def health_check():
    """Endpoint para verificar status da API e conexão com InfluxDB"""
    try:
        # Verificar conexão com InfluxDB
        conexao_ok = influx_service.verificar_conexao()
        
        if conexao_ok:
            return jsonify({
                'status': 'OK',
                'banco_dados': 'conectado'
            }), 200
        else:
            return jsonify({
                'status': 'ERRO',
                'banco_dados': 'desconectado'
            }), 503
            
    except Exception as e:
        return jsonify({
            'status': 'ERRO',
            'mensagem': str(e)
        }), 503


if __name__ == '__main__':
    # Rodar em modo debug para desenvolvimento
    # Em produção, usar um servidor WSGI como Gunicorn
    port = int(os.getenv('PORT', 5000))
    app.run(host='0.0.0.0', port=port, debug=True)
