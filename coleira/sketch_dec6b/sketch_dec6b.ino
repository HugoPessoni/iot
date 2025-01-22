#include <Wire.h>
#include <SD_MMC.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h>
#include <Adafruit_MLX90614.h>

// Pinos do LED RGB e botão
const byte ledPins[] = {33, 32, 27}; // Red, Green, Blue
#define BUTTON_PIN 12

// Configuração do MPU6050
#define SDA_PIN 21
#define SCL_PIN 22
Adafruit_MPU6050 mpu;

// Configuração do GPS
#define GPS_RX 25 // RX do GPS conectado ao GPIO25 (TX da ESP32)
#define GPS_TX 26 // TX do GPS conectado ao GPIO26 (RX da ESP32)
HardwareSerial GPS(2); // UART2 para comunicação com o GPS
TinyGPSPlus gps;

// Configuração do sensor de temperatura corporal
#define TEMP_SDA 4
#define TEMP_SCL 16
Adafruit_MLX90614 mlx = Adafruit_MLX90614();

// Configuração do sensor de batimentos cardíacos
#define SENSOR_PIN 36 // Pino conectado ao AO do XD-58C

// Variáveis de controle
bool isCollecting = false; // Estado da coleta
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 300;
unsigned long lastCollectionTime = 0;
const unsigned long collectionInterval = 1000; // Intervalo de 1 segundo

// Arquivo no SD Card
String unifiedFileName;

// Função para ajustar a cor do LED RGB
void setColor(byte r, byte g, byte b) {
  analogWrite(ledPins[0], 255 - r); // Vermelho
  analogWrite(ledPins[1], 255 - g); // Verde
  analogWrite(ledPins[2], 255 - b); // Azul
}

void setup() {
  // Configuração inicial do LED RGB
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    analogWrite(ledPins[i], 0); // Apagar LEDs
  }

  // Configuração do botão
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Inicializar Serial
  Serial.begin(115200);

  // Inicializar MPU6050
  Wire.begin(SDA_PIN, SCL_PIN);
  if (!mpu.begin()) {
    Serial.println("Erro ao inicializar MPU6050!");
  } else {
    Serial.println("MPU6050 inicializado com sucesso!");
  }

  // Inicializar GPS
  GPS.begin(9600, SERIAL_8N1, GPS_RX, GPS_TX);

  // Configurar I2C para o sensor de temperatura corporal
  Wire.begin(TEMP_SDA, TEMP_SCL);
  if (!mlx.begin()) {
    Serial.println("Erro ao inicializar sensor de temperatura!");
  } else {
    Serial.println("Sensor de temperatura inicializado com sucesso!");
  }

  // Configurar sensor de batimentos cardíacos
  pinMode(SENSOR_PIN, INPUT);

  // Inicializar SD Card
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("Erro ao inicializar SD Card!");
    while (1); // Travar se SD falhar
  } else {
    Serial.println("SD Card inicializado com sucesso!");
  }

  Serial.println("Sistema inicializado.");
}

void loop() {
  // Controle do botão com debounce
  if (digitalRead(BUTTON_PIN) == LOW && millis() - lastDebounceTime > debounceDelay) {
    isCollecting = !isCollecting; // Alternar estado da coleta
    lastDebounceTime = millis();

    if (isCollecting) {
      // Gerar o ID para o arquivo unificado
      unifiedFileName = "/data_" + generateID() + ".csv";

      // Criar arquivo e adicionar cabeçalho
      createCSV(unifiedFileName, "Time(ms),Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,Temp_C,Latitude,Longitude,Altitude,Speed,UTC_Time,Date,Sats,Direction,Temperatura_Corporal,BPM");

      Serial.println("Coleta iniciada.");
    } else {
      // Finalizar a coleta e verificar se o arquivo foi salvo
      if (SD_MMC.exists(unifiedFileName.c_str())) {
        blinkLED(0, 255, 0, 5); // LED Verde pisca 5 vezes
        Serial.println("Arquivo salvo com sucesso.");
      } else {
        blinkLED(255, 0, 0, 5); // LED Vermelho pisca 5 vezes
        Serial.println("Erro ao salvar arquivo.");
      }
    }
  }

  // Executar a coleta de dados se estiver ativa
  if (isCollecting && millis() - lastCollectionTime >= collectionInterval) {
    lastCollectionTime = millis();
    collectUnifiedData();
  }

  // **Piscar LED Rosa durante a coleta**
  if (isCollecting) {
    blinkDuringCollection();
  }

  // Processar dados do GPS continuamente
  while (GPS.available()) {
    gps.encode(GPS.read());
  }
}

// Função para gerar um ID aleatório de 5 caracteres
String generateID() {
  String id = "";
  for (int i = 0; i < 5; i++) {
    char c = random(0, 2) ? random('A', 'Z' + 1) : random('0', '9' + 1);
    id += c;
  }
  return id;
}

// Função para criar um arquivo CSV com cabeçalho
void createCSV(String fileName, String header) {
  File file = SD_MMC.open(fileName.c_str(), FILE_WRITE);
  if (file) {
    file.println(header);
    file.close();
  } else {
    Serial.println("Erro ao criar arquivo: " + fileName);
  }
}

// Função para coletar dados unificados de todos os módulos
void collectUnifiedData() {
  String data = String(millis()) + ",";

  // Coletar dados do MPU6050
  Wire.begin(21, 22);
  sensors_event_t a, g, temp;
  if (mpu.getEvent(&a, &g, &temp)) {
    data += String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z) + "," +
            String(g.gyro.x) + "," + String(g.gyro.y) + "," + String(g.gyro.z) + "," +
            String(temp.temperature) + ",";
  } else {
    data += ",,,,,,,";
  }


  // Coletar dados do GPS
  if (gps.location.isValid()) {
    data += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + ",";
  } else {
    data += ",,";
  }
  data += gps.altitude.isValid() ? String(gps.altitude.meters()) + "," : ",";
  data += gps.speed.isValid() ? String(gps.speed.kmph()) + "," : ",";
  data += gps.time.isValid() ? String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "," : ",";
  data += gps.date.isValid() ? String(gps.date.value()) + "," : ",";
  data += gps.satellites.isValid() ? String(gps.satellites.value()) + "," : ",";
  data += gps.course.isValid() ? String(gps.course.deg()) : ",";

  // Coletar temperatura corporal
  Wire.begin(13, 15);
  float tempC = mlx.readObjectTempC();
  if (!isnan(tempC)) {
    data += String(tempC) + ",";
  } else {
    data += ",";
  }

  // Coletar batimentos cardíacos
  int sensorValue = analogRead(SENSOR_PIN);
  int bpm = map(sensorValue, 500, 3500, 40, 80);
  data += String(bpm);

  // Salvar dados no CSV
  appendToFile(unifiedFileName, data);
}

// Função para gravar dados no arquivo CSV
void appendToFile(String fileName, String data) {
  File file = SD_MMC.open(fileName.c_str(), FILE_APPEND);
  if (file) {
    file.println(data);
    file.close();
    Serial.println("Dados gravados: " + data);
  } else {
    Serial.println("Erro ao gravar no arquivo: " + fileName);
  }
}

// Função para piscar o LED RGB durante a coleta
void blinkDuringCollection() {
  setColor(255, 0, 255); // Rosa
  delay(250);
  setColor(0, 0, 0); // Apagar
  delay(250);
}

// Função para piscar o LED RGB um número específico de vezes
void blinkLED(byte r, byte g, byte b, int times) {
  for (int i = 0; i < times; i++) {
    setColor(r, g, b);
    delay(500);
    setColor(0, 0, 0);
    delay(500);
  }
}
