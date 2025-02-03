#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Servo.h>

// ============================
// Configurações Wi-Fi
// ============================
const char* ssid     = "CORREDOR";
const char* password = "351117780";

// ============================
// Configurações do Adafruit IO via MQTT
// ============================
const char* mqtt_server   = "io.adafruit.com";
const int   mqtt_port     = 1883;
const char* aio_username  = "1H3u4goP342es";
const char* aio_key       = "aio_SFSa86452QBL9gL46VYZ8Yvadsf11b7tV4qtP5V";  // Sua chave do Adafruit IO

// Defina os tópicos (feeds) para os comandos
String servoTopic = String(aio_username) + "/feeds/Servo_Control_on_off";
String ledTopic   = String(aio_username) + "/feeds/LED_Control_on_off";

// ============================
// Pinos e Configurações do Servo e LED
// ============================
const int servoPin = 12;      // Pino de sinal do servo
const int ledPin   = 4;       // LED (pino 4)

const int neutralAngle = 90;  // Posição neutra do servo
const int onAngle      = 45;  // Ângulo para acionar o comando "ligar"
const int offAngle     = 135; // Ângulo para acionar o comando "desligar"

Servo myServo;

// ============================
// Variáveis de controle e timers
// ============================
int lastServoCommand = -1;  // Último comando recebido para o servo
bool servoTimerActive = false;
unsigned long servoTimerStart = 0;  // Tempo de início do timer do servo

int lastLEDCommand = -1;    // Último comando recebido para o LED
bool ledTimerActive = false;
unsigned long ledTimerStart = 0;    // Tempo de início do timer do LED

// ============================
// Objetos para Wi-Fi e MQTT
// ============================
WiFiClient wifiClient;
PubSubClient client(wifiClient);

// ============================
// Função para acionar o servo (simula um "pressionamento")
// ============================
void pressButton(int targetAngle) {
  Serial.print("Movendo o servo para o ângulo: ");
  Serial.println(targetAngle);
  myServo.write(targetAngle);
  delay(1000); // Aguarda 1 segundo com o servo na posição de "pressionar"
  myServo.write(neutralAngle);
  Serial.println("Servo retornou à posição neutra.");
}

// ============================
// Callback do MQTT: processa mensagens recebidas
// ============================
void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  
  Serial.print("Mensagem recebida [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Processa mensagem para o Servo
  if (String(topic) == servoTopic) {
    int servoCommand = message.toInt();
    if (servoCommand != lastServoCommand) {
      if (servoCommand == 1) {
        Serial.println("Comando do Servo: LIGAR (pressionar botão para ligar a luz).");
        pressButton(onAngle);
        // Inicia o timer de 60 segundos para desligar automaticamente
        servoTimerActive = true;
        servoTimerStart = millis();
      }
      else if (servoCommand == 0) {
        Serial.println("Comando do Servo: DESLIGAR (pressionar botão para desligar a luz).");
        pressButton(offAngle);
        // Cancela qualquer timer ativo
        servoTimerActive = false;
      }
      lastServoCommand = servoCommand;
    }
  }
  // Processa mensagem para o LED
  else if (String(topic) == ledTopic) {
    int ledCommand = message.toInt();
    if (ledCommand != lastLEDCommand) {
      if (ledCommand == 1) {
        Serial.println("Comando LED: LIGAR");
        digitalWrite(ledPin, HIGH);
        // Inicia timer para desligar o LED após 10 segundos
        ledTimerActive = true;
        ledTimerStart = millis();
      }
      else if (ledCommand == 0) {
        Serial.println("Comando LED: DESLIGAR");
        digitalWrite(ledPin, LOW);
        ledTimerActive = false;
      }
      lastLEDCommand = ledCommand;
    }
  }
}

// ============================
// Função para reconectar ao MQTT (Adafruit IO)
// ============================
void reconnect() {
  // Gere um client ID único baseado no MAC do ESP32
  String clientId = "ESP32Client-" + String((uint32_t)ESP.getEfuseMac(), HEX);
  
  while (!client.connected()) {
    Serial.print("Tentando conectar ao MQTT...");
    if (client.connect(clientId.c_str(), aio_username, aio_key)) {
      Serial.println("Conectado ao MQTT!");
      // Subscreve aos feeds
      client.subscribe(servoTopic.c_str());
      client.subscribe(ledTopic.c_str());
    } else {
      Serial.print("Falha na conexão, rc=");
      Serial.print(client.state());
      Serial.println(". Tentando novamente em 5 segundos.");
      delay(5000);
    }
  }
}

// ============================
// Setup
// ============================
void setup() {
  Serial.begin(115200);
  
  // Configura o pino do LED e garante que ele inicie desligado
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  
  // Inicializa o servo e posiciona-o em neutro
  myServo.attach(servoPin);
  myServo.write(neutralAngle);
  
  // Conecta ao Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Conectando ao Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());
  
  // Configura o MQTT
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ============================
// Loop Principal
// ============================
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Verifica se o timer do LED expirou (10 segundos)
  if (ledTimerActive && (millis() - ledTimerStart >= 10000)) {
    digitalWrite(ledPin, LOW);
    ledTimerActive = false;
    Serial.println("LED desligado automaticamente após 10 segundos.");
    lastLEDCommand = 0;
  }
  
  // Verifica se o timer do servo expirou (60 segundos)
  if (servoTimerActive && (millis() - servoTimerStart >= 60000)) {
    Serial.println("Timer de 60 segundos expirou. Acionando comando de DESLIGAR (servo).");
    pressButton(offAngle);
    servoTimerActive = false;
    lastServoCommand = 0;
  }
}
