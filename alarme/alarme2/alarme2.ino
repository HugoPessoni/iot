#include "Arduino.h"
#include "Audio.h"
#include "FS.h"
#include "SD_MMC.h"

// Pinos do decibelímetro e SDMMC
#define DECIBEL_PIN 14
#define SD_MMC_CMD 15
#define SD_MMC_CLK 14
#define SD_MMC_D0  2
#define I2S_BCLK   26
#define I2S_DOUT   22
#define I2S_LRC    25

// Parâmetros do decibelímetro
const int sampleWindow = 50;  // Janela de amostragem em milissegundos
const float alarmThreshold = 75.0;  // Limite para acionar o áudio (ajuste conforme necessário)

Audio audio;
bool isPlaying = false;  // Flag para verificar se o áudio está tocando

void setup() {
  Serial.begin(115200);

  // Configura o SDMMC
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

  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  audio.setVolume(12);  // Volume do áudio (0...21)

  analogReadResolution(10);  // Resolução de 10 bits (0-1023)
}

void loop() {
  if (!isPlaying) {
    // Realiza a leitura do decibelímetro
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
      // Inicia a reprodução do áudio
      playAudio();
    }
  } else {
    // Mantém o áudio tocando
    audio.loop();

    // Verifica se o áudio terminou
    if (!audio.isRunning()) {
      isPlaying = false;  // Áudio terminou, volta a medir o som
    }
  }
}

void playAudio() {
  audio.connecttoFS(SD_MMC, "/music/Jingle Bells.mp3");
  isPlaying = true;  // Define a flag como true para indicar que o áudio está tocando
}

// Funções opcionais para depuração do áudio
void audio_info(const char *info) {
  Serial.print("info        ");
  Serial.println(info);
}
void audio_id3data(const char *info) {  //id3 metadata
  Serial.print("id3data     ");
  Serial.println(info);
}
void audio_eof_mp3(const char *info) {  //end of file
  Serial.print("eof_mp3     ");
  Serial.println(info);
}
void audio_showstation(const char *info) {
  Serial.print("station     ");
  Serial.println(info);
}
void audio_showstreamtitle(const char *info) {
  Serial.print("streamtitle ");
  Serial.println(info);
}
void audio_bitrate(const char *info) {
  Serial.print("bitrate     ");
  Serial.println(info);
}
void audio_commercial(const char *info) {  //duration in sec
  Serial.print("commercial  ");
  Serial.println(info);
}
void audio_icyurl(const char *info) {  //homepage
  Serial.print("icyurl      ");
  Serial.println(info);
}
void audio_lasthost(const char *info) {  //stream URL played
  Serial.print("lasthost    ");
  Serial.println(info);
}
