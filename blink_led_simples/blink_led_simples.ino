#define PIN_LED 2 //define uma constante chamada PIN_LED com o valor 2
// se precisarmos mudar qual o pino do LED, basta alterar esse valor.

// so roda uma vez quando a esp é ligada
void setup() {
 // configura o pino 2 como saída, permitindo controlar um dispositivo externo (no caso, o LED).
 pinMode(PIN_LED, OUTPUT);
}

// loop infinito
void loop() {
 digitalWrite(PIN_LED, HIGH); // envia um sinal de tensão alta ao pino 2 e liga o led
 delay(1000); // espera 1s
 digitalWrite(PIN_LED, LOW); // envia um sinal de tensão baixa ao pino 2 e desliga o led
 delay(1000); // espera 1s
}


