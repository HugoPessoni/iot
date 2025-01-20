#include "Arduino.h"
#include "SD_MMC.h"
#include "Freenove_WS2812_Lib_for_ESP32.h"
#include "Audio.h"

// Pinos do SDMMC, LEDs e sensor de som
#define SD_MMC_CMD 15
#define SD_MMC_CLK 14
#define SD_MMC_D0  2
#define I2S_BCLK   26
#define I2S_DOUT   22
#define I2S_LRC    25
#define LEDS_PIN   27
#define SOUND_SENSOR_PIN 32  // Alterado para GPIO32 (ADC1_CH4)

// Configuração dos LEDs
#define LEDS_COUNT  8
#define CHANNEL     0
#define SOUND_THRESHOLD 1000  // Limiar ajustado da leitura analógica

Freenove_ESP32_WS2812 strip = Freenove_ESP32_WS2812(LEDS_COUNT, LEDS_PIN, CHANNEL, TYPE_GRB);

Audio audio;
bool isPlaying = false;
unsigned long lastBlinkTime = 0;
int blinkCount = 0;
bool isRedPhase = true;  // Controla se está na fase vermelha ou azul
bool soundDetected = false;

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
  strip.setBrightness(10);

  // Inicializa o áudio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12);
}

void loop() {
  if (!isPlaying && !soundDetected) {
    int soundLevel = analogRead(SOUND_SENSOR_PIN);
    Serial.print("Nível de som detectado: ");
    Serial.println(soundLevel);

    if (soundLevel > SOUND_THRESHOLD) {
      soundDetected = true;
      const char* filePath = "/music/song.mp3";
      if (SD_MMC.exists(filePath)) {
        audio.connecttoFS(SD_MMC, filePath);
        isPlaying = true;
        Serial.println("Reproduzindo áudio...");
      } else {
        Serial.println("Arquivo de áudio não encontrado.");
        soundDetected = false;  // Se não encontrar o arquivo, permite nova detecção
      }
    }
  }

  if (isPlaying) {
    audio.loop();  // Necessário para manter o áudio tocando

    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= 100) {  // Intervalo de 100ms para piscar rapidamente
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

    // Verifica se o áudio terminou
    if (!audio.isRunning()) {
      isPlaying = false;  // Áudio terminou, desliga os LEDs
      turnOffLeds();
      Serial.println("Áudio terminou.");
      soundDetected = false;  // Permite nova detecção de som
    }
  }

  delay(100);  // Delay para evitar leituras excessivas
}

// Função para definir a cor dos LEDs
void setLedsColor(int r, int g, int b) {
  for (int i = 0; i < LEDS_COUNT; i++) {
    strip.setLedColorData(i, r, g, b);
  }
  strip.show();
}

// Função para desligar os LEDs
void turnOffLeds() {
  setLedsColor(0, 0, 0);  // Define todos os LEDs como apagados
}
