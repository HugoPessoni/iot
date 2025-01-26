import smbus
import time

# Endereço I2C do MPU6050
MPU6050_ADDR = 0x68

# Registros do MPU6050
ACCEL_XOUT_H = 0x3B
ACCEL_YOUT_H = 0x3D
ACCEL_ZOUT_H = 0x3F
GYRO_XOUT_H = 0x43
GYRO_YOUT_H = 0x45
GYRO_ZOUT_H = 0x47
TEMP_OUT_H = 0x41
PWR_MGMT_1 = 0x6B

# Função para inicializar o MPU6050
def mpu6050_init(bus):
    bus.write_byte_data(MPU6050_ADDR, PWR_MGMT_1, 0)

# Função para ler dados de 2 bytes do MPU6050
def read_raw_data(bus, addr):
    high = bus.read_byte_data(MPU6050_ADDR, addr)
    low = bus.read_byte_data(MPU6050_ADDR, addr+1)
    value = (high << 8) | low
    if value > 32768:
        value = value - 65536
    return value

# Função para ler a temperatura
def read_temperature(bus):
    temp_raw = read_raw_data(bus, TEMP_OUT_H)
    temp_c = temp_raw / 340.0 + 36.53
    return temp_c

# Função para teste isolado do MPU6050
def mpu6050_test():
    try:
        bus = smbus.SMBus(2)  # Inicializa a comunicação I2C no bus 2
        mpu6050_init(bus)  # Inicializa o MPU6050
        
        filename = "/home/orangepi/test_mpu6050.csv"
        with open(filename, mode='w') as file:
            # Inclui a coluna de temperatura no cabeçalho
            file.write('AccelX,AccelY,AccelZ,GyroX,GyroY,GyroZ,TempC\n')

        print("Iniciando coleta de dados... (Pressione CTRL+C para parar)")

        while True:
            # Lê os dados do acelerômetro e giroscópio
            accel_x = read_raw_data(bus, ACCEL_XOUT_H) / 16384.0
            accel_y = read_raw_data(bus, ACCEL_YOUT_H) / 16384.0
            accel_z = read_raw_data(bus, ACCEL_ZOUT_H) / 16384.0

            gyro_x = read_raw_data(bus, GYRO_XOUT_H) / 131.0
            gyro_y = read_raw_data(bus, GYRO_YOUT_H) / 131.0
            gyro_z = read_raw_data(bus, GYRO_ZOUT_H) / 131.0

            # Lê e converte a temperatura
            temp_c = read_temperature(bus)

            # Salva os dados no CSV, incluindo a temperatura
            with open(filename, mode='a') as file:
                file.write(f"{accel_x},{accel_y},{accel_z},{gyro_x},{gyro_y},{gyro_z},{temp_c}\n")
            
            print(f"AccelX: {accel_x:.2f}, AccelY: {accel_y:.2f}, AccelZ: {accel_z:.2f}, "
                  f"GyroX: {gyro_x:.2f}, GyroY: {gyro_y:.2f}, GyroZ: {gyro_z:.2f}, TempC: {temp_c:.2f}")

            time.sleep(0.1)  # Intervalo de leitura (ajuste conforme necessário)
    
    except KeyboardInterrupt:
        print("Coleta interrompida pelo usuário.")
    except Exception as e:
        print(f"Erro na coleta de dados: {str(e)}")

# Executa o teste
if __name__ == "__main__":
    mpu6050_test()
