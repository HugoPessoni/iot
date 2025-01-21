#include <DHTesp.h>  // Biblioteca para o sensor DHT
#include <WiFi.h> // Biblioteca para a ESP32
#include "ThingSpeak.h" 

DHTesp dht;  // Define o objeto do sensor DHT
int dhtPin = 13;  // Define o pino DHT como 13
unsigned long myChannelNumber = 2688743;
const char *apiKey = "RAOBLZHGF654CC7Y";  // Substitua pela sua chave de API do ThingSpeak
const char *ssid =  "iPhone de Hugo";       // Substitua pelo seu SSID Wi-Fi
const char *pass =  "pessoni13";       // Substitua pela sua senha Wi-Fi
const char* server = "api.thingspeak.com"; // Servidor do ThingSpeak

#define LED_PIN 5   // Pino onde o LED está conectado
#define LED_PIN_ok 27


WiFiClient client;

void setup() {
    Serial.begin(115200); // Inicializa a comunicação serial
    dht.setup(dhtPin, DHTesp::DHT11);  // Inicializa o sensor DHT11 no pino 13
    
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);  // LED desligado inicialmente

    pinMode(LED_PIN_ok, OUTPUT);
    digitalWrite(LED_PIN_ok, LOW); 

    // Conectando ao WiFi
    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi conectado");

    // Conectando ao ThingSpeak
    ThingSpeak.begin(client);
}

void loop() {
    TempAndHumidity data = dht.getTempAndHumidity();  // Leitura de temperatura e umidade

    if (isnan(data.temperature) || isnan(data.humidity)) {
        //Serial.println("Erro ao ler o sensor DHT!");
        return;
    }

    // Exibe os valores de temperatura e umidade no Serial Monitor
    Serial.println("Temperatura: " + String(data.temperature) + "°C");
    Serial.println("Umidade: " + String(data.humidity) + "%");

    // Controle do LED: Pisca se a temperatura for maior que 10 graus Celsius
    if (data.temperature > 32) {
        for (int i = 0; i < 3; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(500);
            digitalWrite(LED_PIN, LOW);
            delay(500);
        }
        ThingSpeak.setField(3, 1);  // Envia o estado do LED como "1" para o campo 3 (piscando)
    } else {
        digitalWrite(LED_PIN, LOW);
        ThingSpeak.setField(3, 0);  // Envia o estado do LED como "0" para o campo 3 (desligado)
    }

    // Define os valores dos campos no ThingSpeak
    ThingSpeak.setField(1, data.temperature);
    ThingSpeak.setField(2, data.humidity);

    // Envia os dados para o ThingSpeak
    int status = ThingSpeak.writeFields(myChannelNumber, apiKey);

    if (status == 200) {
        Serial.println("Dados enviados com sucesso!");
        digitalWrite(LED_PIN_ok, HIGH);
        delay(500);
        digitalWrite(LED_PIN_ok, LOW);
        delay(500);
    } else {
        Serial.println("Falha ao enviar os dados. Código de erro: " + String(status));
    }

    delay(5000);  // Aguarda 5 segundos antes da próxima leitura
    //ThingSpeak.setField(3, 0);
}