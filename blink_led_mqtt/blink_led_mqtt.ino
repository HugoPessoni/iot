#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

// Definições do Wi-Fi
const char* ssid = "LUCINEIDE OI FIBRA";          // Substitua pelo nome da sua rede WiFi
const char* password = "351117780";     // Substitua pela senha da sua rede WiFi

// Definições do MQTT (Cluster HiveMQ Cloud)
const char* mqttServer = "c79bd0399782452e8f95da95931cffe3.s1.eu.hivemq.cloud";  // Substitua pela URL do seu cluster HiveMQ
const int mqttPort = ;                   // Porta segura com SSL/TLS
const char* mqttUser = "hivemq.webclient.1729085867049";  // Nome de usuário do seu cluster HiveMQ
const char* mqttPassword = "23&xCBV4FdIv5mM#;f,k";       // Senha do seu cluster HiveMQ

// Nome do tópico MQTT
const char* topic = "esp32/led";

// Definir o pino do LED (no caso da placa ESP32, pode ser o pino 2)
const int ledPin = 13;

// Variáveis Wi-Fi e MQTT
WiFiClientSecure espClient;  // Cliente WiFi com SSL
PubSubClient client(espClient);

// Função para conectar ao Wi-Fi
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando ao WiFi ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado.");
  Serial.println("Endereço de IP: ");
  Serial.println(WiFi.localIP());
}

// Função chamada quando uma mensagem é recebida no tópico
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  String message;

  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  if (message == "1") {
    digitalWrite(ledPin, HIGH);  // Liga o LED
    client.publish(topic, "LED ligado");
  } else if (message == "0") {
    digitalWrite(ledPin, LOW);   // Desliga o LED
    client.publish(topic, "LED desligado");
  }
}

// Função para se conectar ao broker MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conexão MQTT...");

    // Tentar se conectar com nome de usuário e senha
    if (client.connect("ESP32Client", mqttUser, mqttPassword)) {
      Serial.println("Conectado ao broker MQTT!");

      // Inscrever-se no tópico
      client.subscribe(topic);
    } else {
      Serial.print("Falha na conexão. Erro: ");
      Serial.print(client.state());
      Serial.println(" Tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);
  
  // Iniciar conexão Wi-Fi
  setup_wifi();

  // Configurar o cliente MQTT
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);

  // Definir que o cliente WiFi aceite qualquer certificado (para conexões seguras TLS)
  espClient.setInsecure();  // Use isso se você não estiver validando certificados.
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}