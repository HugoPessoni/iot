#include <Wire.h>
#include <SD_MMC.h>
#include <HardwareSerial.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h>

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

// Variáveis de controle
bool isCollecting = false; // Estado da coleta
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 300;

// Arquivos no SD Card
String baseID;
String accelFileName;
String gpsFileName;

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
      delay(60000);
      // Gerar o mesmo ID para ambos os módulos
      baseID = generateID();
      accelFileName = "/accel_" + baseID + ".csv";
      gpsFileName = "/gps_" + baseID + ".csv";

      // Criar arquivos e adicionar cabeçalhos
      createCSV(accelFileName, "Time(ms),Accel_X,Accel_Y,Accel_Z,Gyro_X,Gyro_Y,Gyro_Z,Temp_C");
      createCSV(gpsFileName, "Time(ms),Latitude,Longitude,Altitude,Speed,UTC_Time,Date,Sats,Direction");

      Serial.println("Coleta iniciada.");
    } else {
      // Finalizar a coleta e verificar se os arquivos foram salvos
      if (saveCheck()) {
        blinkLED(0, 255, 0, 5); // LED Verde pisca 5 vezes
        Serial.println("Arquivos salvos com sucesso.");
      } else {
        blinkLED(255, 0, 0, 5); // LED Vermelho pisca 5 vezes
        Serial.println("Erro ao salvar arquivos.");
      }
    }
  }

  // Executar a coleta de dados se estiver ativa
  if (isCollecting) {
    // Piscar LED Rosa durante a coleta
    blinkDuringCollection();
    collectData();
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

// Função para coletar dados do MPU6050 e GPS
void collectData() {
  // Coletar dados do MPU6050
  String accelData = "";
  sensors_event_t a, g, temp;
  if (mpu.getEvent(&a, &g, &temp)) {
    accelData = String(millis()) + "," + String(a.acceleration.x) + "," +
                String(a.acceleration.y) + "," + String(a.acceleration.z) + "," +
                String(g.gyro.x) + "," + String(g.gyro.y) + "," + String(g.gyro.z) + "," +
                String(temp.temperature);
  } else {
    accelData = String(millis()) + ",,,,,,,";
  }
  appendToFile(accelFileName, accelData);

  // Coletar dados do GPS
  String gpsData = String(millis()) + ",";
  if (gps.location.isValid()) {
    gpsData += String(gps.location.lat(), 6) + "," + String(gps.location.lng(), 6) + ",";
  } else {
    gpsData += ",,";
  }
  gpsData += gps.altitude.isValid() ? String(gps.altitude.meters()) + "," : ",";
  gpsData += gps.speed.isValid() ? String(gps.speed.kmph()) + "," : ",";
  gpsData += gps.time.isValid() ? String(gps.time.hour()) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "," : ",";
  gpsData += gps.date.isValid() ? String(gps.date.value()) + "," : ",";
  gpsData += gps.satellites.isValid() ? String(gps.satellites.value()) + "," : ",";
  gpsData += gps.course.isValid() ? String(gps.course.deg()) : ",";
  appendToFile(gpsFileName, gpsData);
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

// Função para verificar se os arquivos foram salvos
bool saveCheck() {
  return SD_MMC.exists(accelFileName.c_str()) && SD_MMC.exists(gpsFileName.c_str());
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
