// criar constantes 
#define PIN_LED 2
#define PIN_BUTTON 13

void setup() {
 pinMode(PIN_LED, OUTPUT); //configura o pino 2 (PIN_LED) como saída, permitindo controlar o LED.
 pinMode(PIN_BUTTON, INPUT); //configura o pino 13 (PIN_BUTTON) como entrada, permitindo ler o estado do botão.
}

// Executa continuamente a leitura do botão e controla o LED com base no estado do botão.
void loop() {
 if (digitalRead(PIN_BUTTON) == LOW) { //Lê o estado lógico do pino 13
 //LOW: Representa um estado baixo de tensão (0V).
 //HIGH: Representa um estado alto de tensão (5V)

 digitalWrite(PIN_LED,LOW); //desliga led
 }else{
 digitalWrite(PIN_LED,HIGH); //liga led
 }
}

