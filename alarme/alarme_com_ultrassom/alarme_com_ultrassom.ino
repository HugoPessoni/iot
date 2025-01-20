#include "Arduino.h"
#include "SD_MMC.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "Audio.h"
#include <ESP32Servo.h>

// Pinos do SDMMC, LEDs, sensor de som, ultrassônico, servo e potenciômetro
#define SD_MMC_CMD 15
#define SD_MMC_CLK 14
#define SD_MMC_D0  2
#define I2S_BCLK   26
#define I2S_DOUT   22
#define I2S_LRC    25
#define LEDS_PIN   27
#define SOUND_SENSOR_PIN 32  // Pino analógico para o detector de som
#define TRIG_PIN 19 // define TrigPin para o ultrassônico
#define ECHO_PIN 18 // define EchoPin para o ultrassônico
#define SERVO_PIN 13  // Pino para controle do servo
#define POT_PIN 34    // Pino para o potenciômetro

// Configuração dos LEDs
#define LEDS_COUNT  8
#define CHANNEL     0
#define SOUND_THRESHOLD 3000  // Limite de detecção de som (ajuste conforme necessário)
#define DISTANCE_MIN_THRESHOLD 3  // Distância mínima em cm para acionar o alarme
#define DISTANCE_MAX_THRESHOLD 20  // Distância máxima em cm para acionar o alarme
#define MEASURE_INTERVAL 500  // Intervalo de medição do som e distância em milissegundos
#define MEASURE_THRESHOLD 3  // Número de medições consecutivas acima do limiar necessárias para ativar

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);
Servo myservo;

Audio audio;
bool isPlaying = false;
unsigned long lastMeasureTime = 0;
int measureCountSound = 0;
int measureCountDistance = 0;
bool alarmTriggered = false;
float timeOut = 700 * 60; 
int soundVelocity = 340; // Velocidade do som em m/s
int potVal;             // Variável para ler o valor do potenciômetro

void setup() {
  Serial.begin(115200);

  // Inicializa o SD_MMC
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
    Serial.println("Falha ao montar o cartão SD");
    return;
  }

  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Nenhum cartão SD_MMC encontrado");
    return;
  }

  uint64_t cardSize = SD_MMC.cardSize() / (1024 * 1024);
  Serial.printf("Tamanho do cartão SD_MMC: %lluMB\n", cardSize);

  // Inicializa os LEDs
  strip.begin();
  strip.setBrightness(80);

  // Inicializa o áudio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(30);

  // Configura os pinos do ultrassônico
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Inicializa o servo
  myservo.setPeriodHertz(50); // Servo padrão a 50Hz
  myservo.attach(SERVO_PIN, 500, 2500); // Anexa o servo ao pino especificado
}

void loop() {
  unsigned long currentMillis = millis();

  // Processa o áudio continuamente
  if (isPlaying) {
    audio.loop();  // Necessário para manter o áudio tocando
    animateLeds(); // Animação dos LEDs enquanto o áudio toca

    // Controle do servo enquanto o alarme está tocando
    static int servoPos = 0;
    static int servoDirection = 1; // 1 para aumentar, -1 para diminuir
    myservo.write(servoPos);
    servoPos += servoDirection * 5; // Mova o servo 5 graus por vez

    if (servoPos >= 180 || servoPos <= 0) {
      servoDirection = -servoDirection; // Inverte a direção ao alcançar os limites
    }

    // Verifica se o áudio terminou
    if (!audio.isRunning()) {
      isPlaying = false;  // Áudio terminou, desliga os LEDs
      turnOffLeds();
      Serial.println("Áudio terminou.");
    }
  }

  // Controle do servo pelo potenciômetro a cada 100ms (somente quando o alarme não está tocando)
  static unsigned long lastServoUpdate = 0;
  if (currentMillis - lastServoUpdate >= 100 && !isPlaying) {
    lastServoUpdate = currentMillis;
    potVal = analogRead(POT_PIN);             
    potVal = map(potVal, 0, 4095, 0, 180); 
    myservo.write(potVal);                  
  }

  // Medição do som e distância a cada intervalo definido
  if (currentMillis - lastMeasureTime >= MEASURE_INTERVAL && !isPlaying) {
    lastMeasureTime = currentMillis;

    // Medição do som
    int soundLevel = analogRead(SOUND_SENSOR_PIN);
    Serial.print("Leitura do sensor de som: ");
    Serial.println(soundLevel);

    if (soundLevel > SOUND_THRESHOLD) {
      measureCountSound++;
      Serial.print("Contagem de medidas de som acima do limiar: ");
      Serial.println(measureCountSound);
    } else {
      measureCountSound = 0;  // Reinicia a contagem se a medida estiver abaixo do limiar
    }

    // Medição da distância
    float distance = getSonar();
    Serial.print("Distância medida: ");
    Serial.print(distance);
    Serial.println(" cm");

    if (distance > DISTANCE_MIN_THRESHOLD && distance < DISTANCE_MAX_THRESHOLD) {
      measureCountDistance++;
      Serial.print("Contagem de medidas de distância dentro do intervalo de disparo: ");
      Serial.println(measureCountDistance);
    } else {
      measureCountDistance = 0;  // Reinicia a contagem se a medida estiver fora do intervalo
    }

    // Se o som foi detectado por mais de 3 medidas consecutivas ou a distância foi violada 3 vezes, ativa o alarme
    if (measureCountSound >= MEASURE_THRESHOLD || measureCountDistance >= MEASURE_THRESHOLD) {
      const char* filePath = "/music/sirene.mp3";
      if (SD_MMC.exists(filePath)) {
        audio.connecttoFS(SD_MMC, filePath);
        isPlaying = true;
        Serial.println("Reproduzindo áudio...");
      } else {
        Serial.println("Arquivo de áudio não encontrado.");
      }
      measureCountSound = 0;  // Reinicia a contagem de medidas de som
      measureCountDistance = 0;  // Reinicia a contagem de medidas de distância
    }
  }
}

float getSonar() {
  unsigned long pingTime;
  float distance;
  // Envia pulso de 10µs para o TrigPin
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  // Espera o retorno do EchoPin e mede o tempo
  pingTime = pulseIn(ECHO_PIN, HIGH, timeOut); 
  // Calcula a distância de acordo com o tempo
  distance = (float)pingTime * soundVelocity / 2 / 10000; 
  return distance; // Retorna a distância medida
}

void animateLeds() {
  static unsigned long lastBlinkTime = 0;
  static int blinkCount = 0;
  static bool isRedPhase = true;

  unsigned long currentMillis = millis();
  if (currentMillis - lastBlinkTime >= 100) {  // Intervalo de 50ms para piscar rapidamente
    lastBlinkTime = currentMillis;
    if (blinkCount < 5) {
      if (isRedPhase) {
        setLedsColor(255, 0, 0);  // Vermelho
      } else {
        setLedsColor(0, 0, 255);  // Azul
      }
      delay(50);  // Mantém o LED aceso por 50ms
      setLedsColor(0, 0, 0);  // Apaga os LEDs
      blinkCount++;
    } else {
      blinkCount = 0;
      isRedPhase = !isRedPhase;  // Troca a fase entre vermelho e azul
    }
  }
}

void setLedsColor(int r, int g, int b) {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, r, g, b);
  }
  strip.show();
}

void turnOffLeds() {
  setLedsColor(0, 0, 0);  // Define todos os LEDs como apagados
}
