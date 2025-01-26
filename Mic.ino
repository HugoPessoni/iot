#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789

// ST7789 TFT module connections
#define TFT_DC      4    // Data/Command pin connected to GPIO4     
#define TFT_RST     15   // Reset pin connected to GPIO15    
#define TFT_CS     -1    // CS not used

// Initialize ST7789 TFT library with hardware SPI module
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);

// Pino do decibelímetro
#define DECIBEL_PIN 14

const int sampleWindow = 50;  // Janela de amostragem em milissegundos
const float alarmThreshold = 75.0;  // Limite para acionar o alarme (ajuste conforme necessário)

void setup(void) {
  Serial.begin(9600);

  // Init ST7789 display 240x240 pixel, using SPI_MODE3
  tft.init(240, 240, SPI_MODE3);    

  // Set rotation to 2
  tft.setRotation(2);

  // Configura a tela inicialmente
  tft.fillScreen(ST77XX_BLACK);

  // Configura o pino do decibelímetro como entrada analógica
  analogReadResolution(10);  // Resolução de 10 bits (0-1023)
}

void loop() {
  unsigned long startMillis = millis();  // Início da janela de amostragem
  unsigned int signalMax = 0;            // Valor máximo inicial
  unsigned int signalMin = 1023;         // Valor mínimo inicial

  // Coleta de dados dentro da janela de amostragem
  while (millis() - startMillis < sampleWindow) {
    int sample = analogRead(DECIBEL_PIN);
    if (sample < 1024) {  // Ignora leituras espúrias
      if (sample > signalMax) {
        signalMax = sample;  // Salva o valor máximo
      }
      if (sample < signalMin) {
        signalMin = sample;  // Salva o valor mínimo
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
    // Se o som estiver abaixo do limite, mantenha a tela em preto
    tft.fillScreen(ST77XX_BLACK);
  }

  delay(100);  // Pequena pausa antes da próxima leitura
}

void triggerAlarm() {
  // Pisca alternadamente vermelho e azul acima da mensagem
  for (int i = 0; i < 10; i++) {  // Repete o efeito de piscar 10 vezes
    tft.fillRect(0, 0, 240, 60, ST77XX_BLUE);  // Área piscando em azul acima da mensagem
    tft.fillRect(0, 180, 240, 60, ST77XX_BLUE);  // Área piscando em azul abaixo da mensagem
    tft.setCursor(20, 80);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.setTextWrap(false);
    tft.print("  MUITO");
    tft.setCursor(20, 120);
    tft.print("  BARULHO");

    delay(100);
    
    tft.fillRect(0, 0, 240, 60, ST77XX_RED);  // Área piscando em vermelho acima da mensagem
    tft.fillRect(0, 180, 240, 60, ST77XX_RED);  // Área piscando em vermelho abaixo da mensagem
    tft.setCursor(20, 80);
    tft.setTextColor(ST77XX_WHITE);
    tft.setTextSize(3);
    tft.setTextWrap(false);
    tft.print("  MUITO");
    tft.setCursor(20, 120);
    tft.print("  BARULHO");

    delay(100);
  }
}
