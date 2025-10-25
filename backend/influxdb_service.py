"""
Serviço para integração com InfluxDB Cloud
Responsável por salvar e consultar dados dos sensores
"""

from influxdb_client import InfluxDBClient, Point, WritePrecision
from influxdb_client.client.write_api import SYNCHRONOUS
from datetime import datetime, timedelta


class InfluxDBService:
    """Classe para gerenciar operações com InfluxDB Cloud"""
    
    def __init__(self, url, token, org, bucket):
        """
        Inicializar conexão com InfluxDB
        
        Args:
            url: URL do InfluxDB Cloud (ex: https://us-east-1-1.aws.cloud2.influxdata.com)
            token: Token de autenticação
            org: Nome da organização
            bucket: Nome do bucket para armazenar dados
        """
        self.url = url
        self.token = token
        self.org = org
        self.bucket = bucket
        
        # Criar cliente InfluxDB
        self.client = InfluxDBClient(url=url, token=token, org=org)
        self.write_api = self.client.write_api(write_options=SYNCHRONOUS)
        self.query_api = self.client.query_api()
    
    def salvar_leitura(self, dados):
        """
        Salvar leitura dos sensores no InfluxDB
        
        Args:
            dados: Dicionário com os dados da leitura
            
        Returns:
            bool: True se salvou com sucesso, False caso contrário
        """
        try:
            # Criar point para o InfluxDB
            point = Point("leitura_sensores") \
                .tag("robo", "explorador_espacial") \
                .field("temperatura_c", float(dados['temperatura_c'])) \
                .field("umidade_pct", float(dados['umidade_pct'])) \
                .field("luminosidade", int(dados['luminosidade'])) \
                .field("presenca", int(dados['presenca'])) \
                .field("probabilidade_vida", float(dados['probabilidade_vida']))
            
            # Se houver timestamp, usar ele
            if 'timestamp' in dados:
                timestamp = datetime.fromisoformat(dados['timestamp'].replace('Z', '+00:00'))
                point = point.time(timestamp, WritePrecision.NS)
            
            # Escrever no InfluxDB
            self.write_api.write(bucket=self.bucket, org=self.org, record=point)
            
            return True
            
        except Exception as e:
            print(f"Erro ao salvar leitura: {e}")
            return False
    
    def buscar_leituras(self, limit=100, hours=24):
        """
        Buscar últimas leituras do InfluxDB
        
        Args:
            limit: Número máximo de leituras a retornar
            hours: Buscar leituras das últimas N horas
            
        Returns:
            list: Lista de dicionários com as leituras
        """
        try:
            # Construir query Flux
            query = f'''
            from(bucket: "{self.bucket}")
                |> range(start: -{hours}h)
                |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
                |> pivot(rowKey:["_time"], columnKey: ["_field"], valueColumn: "_value")
                |> sort(columns: ["_time"], desc: true)
                |> limit(n: {limit})
            '''
            
            # Executar query
            result = self.query_api.query(org=self.org, query=query)
            
            # Processar resultados
            leituras = []
            for table in result:
                for record in table.records:
                    leitura = {
                        'timestamp': record.get_time().isoformat(),
                        'temperatura_c': record.values.get('temperatura_c', 0),
                        'umidade_pct': record.values.get('umidade_pct', 0),
                        'luminosidade': record.values.get('luminosidade', 0),
                        'presenca': record.values.get('presenca', 0),
                        'probabilidade_vida': record.values.get('probabilidade_vida', 0)
                    }
                    leituras.append(leitura)
            
            return leituras
            
        except Exception as e:
            print(f"Erro ao buscar leituras: {e}")
            return []
    
    def buscar_estatisticas(self, hours=24):
        """
        Buscar estatísticas das leituras (média, mín, máx)
        
        Args:
            hours: Calcular estatísticas das últimas N horas
            
        Returns:
            dict: Dicionário com as estatísticas
        """
        try:
            # Query para estatísticas de cada campo
            query = f'''
            from(bucket: "{self.bucket}")
                |> range(start: -{hours}h)
                |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
                |> group(columns: ["_field"])
            '''
            
            # Queries individuais para cada estatística
            stats = {}
            campos = ['temperatura_c', 'umidade_pct', 'luminosidade', 'probabilidade_vida']
            
            for campo in campos:
                # Query para média
                query_media = f'''
                from(bucket: "{self.bucket}")
                    |> range(start: -{hours}h)
                    |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
                    |> filter(fn: (r) => r["_field"] == "{campo}")
                    |> mean()
                '''
                
                # Query para mínimo
                query_min = f'''
                from(bucket: "{self.bucket}")
                    |> range(start: -{hours}h)
                    |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
                    |> filter(fn: (r) => r["_field"] == "{campo}")
                    |> min()
                '''
                
                # Query para máximo
                query_max = f'''
                from(bucket: "{self.bucket}")
                    |> range(start: -{hours}h)
                    |> filter(fn: (r) => r["_measurement"] == "leitura_sensores")
                    |> filter(fn: (r) => r["_field"] == "{campo}")
                    |> max()
                '''
                
                # Executar queries
                result_media = self.query_api.query(org=self.org, query=query_media)
                result_min = self.query_api.query(org=self.org, query=query_min)
                result_max = self.query_api.query(org=self.org, query=query_max)
                
                # Processar resultados
                media = None
                minimo = None
                maximo = None
                
                for table in result_media:
                    for record in table.records:
                        media = record.get_value()
                
                for table in result_min:
                    for record in table.records:
                        minimo = record.get_value()
                
                for table in result_max:
                    for record in table.records:
                        maximo = record.get_value()
                
                stats[campo] = {
                    'media': round(media, 2) if media else 0,
                    'minimo': round(minimo, 2) if minimo else 0,
                    'maximo': round(maximo, 2) if maximo else 0
                }
            
            return stats
            
        except Exception as e:
            print(f"Erro ao buscar estatísticas: {e}")
            return {}
    
    def verificar_conexao(self):
        """
        Verificar se a conexão com InfluxDB está funcionando
        
        Returns:
            bool: True se conectado, False caso contrário
        """
        try:
            # Tentar fazer uma query simples
            query = f'buckets() |> filter(fn: (r) => r.name == "{self.bucket}")'
            result = self.query_api.query(org=self.org, query=query)
            return True
        except Exception as e:
            print(f"Erro ao verificar conexão: {e}")
            return False
    
    def fechar(self):
        """Fechar conexão com InfluxDB"""
        self.client.close()
