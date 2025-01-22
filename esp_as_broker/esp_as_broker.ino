#include <PicoMQTT.h>

#define WIFI_SSID "LIVE TIM_5680_2G"
#define WIFI_PASSWORD "ZUfY6hfT"


/*class MQTT: public PicoMQTT::Server {
    protected:
        PicoMQTT::ConnectReturnCode auth(const char * client_id, const char * username, const char * password) override {
            // aceita apenas IDs de cliente que tenham 3 caracteres ou mais
            if (String(client_id).length() < 3) {    // client_id nunca é NULL
                return PicoMQTT::CRC_IDENTIFIER_REJECTED;
            }

            // aceita apenas conexões se nome de usuário e senha forem fornecidos
            if (!username || !password) {  // username e password podem ser NULL
                // nenhum nome de usuário ou senha fornecidos
                return PicoMQTT::CRC_NOT_AUTHORIZED;
            }

            // aceita duas combinações de usuário/senha
            if (
                ((String(username) == "Hugo") && (String(password) == "1234"))
                || ((String(username) == "Iwens") && (String(password) == "senha123"))) {
                return PicoMQTT::CRC_ACCEPTED;
            }

            // rejeita todas as outras credenciais
            return PicoMQTT::CRC_BAD_USERNAME_OR_PASSWORD;
        }
} mqtt;*/

PicoMQTT::Server mqtt; // caso for usar a autenticação, comente essa linha

void setup() {
  // Configura a serial
  Serial.begin(115200);

  //---------------------------------------------------------------------------
  // Conecta ao WiFi
  Serial.printf("Conectando ao WiFi %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) { delay(1000); }
  Serial.printf("WiFi conectado, IP: %s\n", WiFi.localIP().toString().c_str());
  //---------------------------------------------------------------------------

  /*//---------------------------------------------------------------------------
  // Configura o Wi-Fi no modo Ponto de Acesso (Access Point)
  WiFi.softAP("ESP32_Hotspot", "12345678");  // Substitua pelo seu SSID e senha

  // Exibe o endereço IP da ESP32
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Endereço IP do Ponto de Acesso: ");
  Serial.println(IP);
  //---------------------------------------------------------------------------*/

  // Inscreve-se em um padrão de tópico e anexa uma função de callback
  mqtt.subscribe("#", [](const char* topic, const char* payload) {
    Serial.printf("Mensagem recebida no tópico '%s': %s\n", topic, payload);
  });
  mqtt.begin();
}

void loop() {
  mqtt.loop();
  if (random(1000) == 0){
    mqtt.publish("picomqtt/welcome", "Olá do PicoMQTT!");
  }
}
