#include <WiFi.h>  // Biblioteca WiFi para ESP32
#include <WebServer.h>  // Biblioteca para servidor web/permitindo que ele responda a requisições HTTP 
#include <UltrasonicSensor.h>  // Biblioteca do sensor ultrassônico

// Definir os pinos do sensor ultrassônico
UltrasonicSensor ultrasonic(13, 14);  // Pinos de trigger e echo

// Credenciais da rede Wi-Fi
const char* id = "Ed";
const char* password = "hello@123";

// Isso cria um servidor web no ESP32 que escuta na porta 80
WebServer server(80);

// Definir a temperatura ambiente para calibrar a velocidade do som
int temperature = 25;

// Essa função é chamada sempre que alguém acessa a página principal do servidor
void handleRoot() {
  int distance = ultrasonic.distanceInCentimeters();
  String response = String(distance);
  
  // Adicionar cabeçalho de CORS/permite que qualquer origem (página web, software) acesse o ESP32
  server.sendHeader("Access-Control-Allow-Origin", "*");
  
  // Enviar a resposta com a distância/A resposta HTTP é enviada com o código 200 (sucesso) e o conteúdo é a distância
  server.send(200, "text/plain", response);

  // Imprimir no Serial Monitor para depuração
  Serial.println("Requisição recebida, distância: " + response + " cm");
}

void setup() {
  Serial.begin(115200);

  // Conectar ao Wi-Fi
  WiFi.begin(id, password);
  Serial.println("Conectando ao WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("WiFi conectado!");
  Serial.print("Endereço IP: ");
  Serial.println(WiFi.localIP());

  // Configurar a temperatura ambiente para reduzir erros
  ultrasonic.setTemperature(temperature);

  // Iniciar o servidor web
  server.on("/", handleRoot);  // define que sempre que a página principal do servidor (rota "/") for acessada, a função handleRoot() será executada.
  server.begin();
  Serial.println("Servidor iniciado.");
}

void loop() {
  server.handleClient();  // ela apenas verifica se o servidor web está recebendo requisições. Se houver uma requisição HTTP, o servidor processa a requisição e chama a função apropriada
}