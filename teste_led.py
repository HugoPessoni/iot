from machine import Pin, SPI, ADC
import st7789
import time

# Configuração do ADC para o decibelímetro
decibel_pin = ADC(Pin(34))
decibel_pin.atten(ADC.ATTN_11DB)
decibel_pin.width(ADC.WIDTH_10BIT)

# Configuração do SPI e dos pinos da tela ST7789
spi = SPI(1, baudrate=20000000, polarity=1, phase=1, sck=Pin(22), mosi=Pin(21))
display = st7789.ST7789(
    spi,
    135, 240,  # Tamanho da tela
    reset=Pin(3, Pin.OUT),  # Pino de reset
    dc=Pin(1, Pin.OUT),     # Pino de controle de dados/comandos
    cs=Pin(5, Pin.OUT),     # Pino de chip select
    backlight=Pin(4, Pin.OUT),  # Pino de backlight (opcional)
    rotation=0)  # Rotação da tela (ajuste conforme necessário)

display.init()

# Função para calcular decibéis a partir do sinal do sensor
def read_decibel():
    sample_window = 50  # Janela de amostragem em milissegundos
    start_time = time.ticks_ms()
    signal_max = 0
    signal_min = 1024

    while time.ticks_diff(time.ticks_ms(), start_time) < sample_window:
        sample = decibel_pin.read()
        if sample > signal_max:
            signal_max = sample
        elif sample < signal_min:
            signal_min = sample
    
    peak_to_peak = signal_max - signal_min

    if peak_to_peak < 10:
        peak_to_peak = 10
    elif peak_to_peak > 900:
        peak_to_peak = 900

    db = int((peak_to_peak - 10) * (90 - 30) / (900 - 10) + 30)
    
    return db

# Função para exibir os decibéis na tela
def display_decibels(db_value):
    display.fill(st7789.BLACK)  # Limpa a tela
    display.text("Loudness:", 10, 30, st7789.WHITE)
    display.text(str(db_value) + " dB", 10, 60, st7789.WHITE)

# Loop principal
while True:
    db_value = read_decibel()
    display_decibels(db_value)
    time.sleep(0.5)
