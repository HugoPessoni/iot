import paho.mqtt.client as mqtt
import time
import random

# Endereço e porta do broker MQTT
broker_address = "192.168.4.1"
broker_port = 1883

# Credenciais MQTT
username = "Hugo"
password = "1234"

# Função para lidar com mensagens recebidas
def on_message(client, userdata, message):
    print("Mensagem recebida:", str(message.payload.decode("utf-8")))

# Cria o cliente MQTT
client = mqtt.Client()

# Define nome de usuário e senha
#client.username_pw_set(username, password)  # comentar caso não esteja usando autenticação

# Atribui a função de tratamento de mensagens
client.on_message = on_message

# Conecta ao broker MQTT
client.connect(broker_address, broker_port)

# Inscreve-se em um tópico
topic = "Python"
client.subscribe(topic)

# Publica mensagens aleatórias periodicamente com diferentes níveis de QoS
while True:
    random_message = f"Mensagem {random.randint(1, 100)}"
    
    print(client.publish(topic, random_message))
    print(f"Mensagem publicada: {random_message}")
    time.sleep(1)  # Publica a cada 1 segundo

# Inicia o loop MQTT
client.loop_forever()