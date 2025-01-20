#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include "Audio.h"
#include "FS.h"
#include "SD_MMC.h"

// Pinos para a tela ST7789
#define TFT_DC      27   // Data/Command pin
#define TFT_RST     32   // Reset pin
#define TFT_CS     -1    // CS not used

// Pinos para o SD_MMC e I2S
#define SD_MMC_CMD 15
#define SD_MMC_CLK 14
#define SD_MMC_D0  2
#define I2S_BCLK   26
#define I2S_DOUT   22
#define I2S_LRC    25

// Inicializa a biblioteca Adafruit para o display ST7789
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Inicializa a biblioteca de áudio
Audio audio;

// Pino do decibelímetro
#define DECIBEL_PIN 34

const int sampleWindow = 50;  // Janela de amostragem em milissegundos
const float alarmThreshold = 75.0;  // Limite para acionar o alarme

void setup() {
  Serial.begin(115200);

  // Inicializa a tela ST7789
  tft.init(240, 240, SPI_MODE3);
  tft.setRotation(2);
  tft.fillScreen(ST77XX_BLACK);

  // Configura o pino do decibelímetro como entrada analógica
  analogReadResolution(10);  // Resolução de 10 bits (0-1023)

  // Inicializa o SD_MMC
  SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
  if (!SD_MMC.begin("/sdcard", true, true, SDMMC_FREQ_DEFAULT, 5)) {
    Serial.println("Card Mount Failed");
    return;
  }

  // Verifica se o cartão SD está montado corretamente
  uint8_t cardType = SD_MMC.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("No SD_MMC card attached");
    return;
  }

  // Inicializa a biblioteca de áudio
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12);  // Volume: 0...21

  // A música será tocada apenas quando o alarme for ativado
}

void loop() {
  unsigned long startMillis = millis();
  unsigned int signalMax = 0;
  unsigned int signalMin = 1023;

  // Coleta de dados dentro da janela de amostragem
  while (millis() - startMillis < sampleWindow) {
    int sample = analogRead(DECIBEL_PIN);
    if (sample < 1024) {
      if (sample > signalMax) {
        signalMax = sample;
      }
      if (sample < signalMin) {
        signalMin = sample;
      }
    }
  }

  // Calcula a amplitude pico a pico
  float peakToPeak = signalMax - signalMin;

  // Mapeia a amplitude pico a pico para decibéis
  float db = map(peakToPeak, 20, 900, 30, 90);

  // Verifica se o valor de dB ultrapassa o limite definido
  if (db > alarmThreshold) {
    triggerAlarm();
  } else {
    tft.fillScreen(ST77XX_BLACK);  // Mantenha a tela em preto se o nível de som estiver abaixo do limite
  }

  audio.loop();  // Mantém o áudio em loop
}

void triggerAlarm() {
  // Exibe a mensagem "MUITO BARULHO" e alterna entre vermelho e azul
  for (int i = 0; i < 10; i++) {
    tft.fillRect(0, 0, 240, 40, ST77XX_BLUE);
    tft.fillRect(0, 200, 240, 40, ST77XX_BLUE);
    tft.setCursor(20, 80);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.setTextWrap(false);
    tft.print("MUITO");
    tft.setCursor(20, 130);
    tft.print("BARULHO");

    delay(100);

    tft.fillRect(0, 0, 240, 40, ST77XX_RED);
    tft.fillRect(0, 200, 240, 40, ST77XX_RED);
    tft.setCursor(20, 80);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.setTextWrap(false);
    tft.print("MUITO");
    tft.setCursor(20, 130);
    tft.print("BARULHO");

    delay(100);
  }

  // Inicia a reprodução da música
  audio.connecttoFS(SD_MMC, "/music/Rick Astley - Never Gonna Give You Up (Official Music Video).mp3");
}
