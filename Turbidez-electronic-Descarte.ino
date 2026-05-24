/**
    Arduino Electronic-Dreno (I2C) - v1.6 (Corrigido)
    - Corrigido parêntese do digitalRead(RELAY_NC_PIN)
    - Adicionado pinMode para o pino 11 (LED de Incidência)
    - Correção do temporizador do display (WakeUp ao mudar leitura)
*/

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <LED.h>
#include <Relay.h>
#include <GBALib_Potentiometer.h> 

// Instância do potenciômetro: Pino A1, mapeado de 0 a 255 para o PWM do LED
Pot myPot(A1, 0, 255);

LED led(3);
Morse morse; 
// Instância do Relé: Pino 9, false = Normalmente Fechado (NF)
Relay light(9, false); 

#define BUZZER_PIN 8
#define LDR_PIN 4  
#define TURBIDITY_PIN A0
const int pinoLED = 3; 
const int LED_INCIDENCIA_PIN = 11; // Definido pino por constante
const int RELAY_NC_PIN = 7; // Pino físico de feedback do contato do relé

#define DRAIN_DURATION 10000
// Exemplo: Descarga a cada 1 Minuto
unsigned long DRAIN_INTERVAL = 1 * 10 * 1000UL;
// Tempo de espera (cooldown) após o dreno fechar antes de permitir uma nova leitura (5 segundos)
const unsigned long COOLDOWN_INTERVAL = 5000UL; 

float TURBIDITY_THRESHOLD = 2.5;

unsigned long lastDrainTime = 0;
unsigned long lastInteractionTime = 0;
const unsigned long DISPLAY_TIMEOUT = 30000; 
bool displayIsOn = true;
float lastTurbidityValue = -1.0; 

// CONTADORES E LIMITADORES
int drainCount = 0;          // Variável que conta as descargas executadas
const int MAX_DRAINS = 10;   // Limite máximo permitido

LiquidCrystal_I2C lcd(0x27, 16, 2); 

void wakeUpDisplay() {
  lastInteractionTime = millis();
  if (!displayIsOn) {
    lcd.backlight();
    displayIsOn = true;
    lastTurbidityValue = -1.0; 
  }
}

void playTone(int freq, int duration) {
  tone(BUZZER_PIN, freq, duration);
  delay(duration);
}

// Alarme caso o contato físico do relé cole ou falhe ao abrir
void dispararAlarmeFALHA() {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ERR: FALHA RELE!");
  lcd.setCursor(0, 1);
  lcd.print("SISTEMA TRAVADO");

  while (true) {
    tone(BUZZER_PIN, 2000, 300);
    led.sendMorseCode(0, 3500, morse); 
    delay(300);
    tone(BUZZER_PIN, 1500, 300);
    delay(300);
  }
}

// Alarme caso o sistema atinja o teto de 10 descargas consecutivas
void dispararAlarmeLIMITE() {
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ERR: LIMITE MAX");
  lcd.setCursor(0, 1);
  lcd.print("10 DRENOS EXEC.");

  while (true) {
    tone(BUZZER_PIN, 1800, 500);
    digitalWrite(pinoLED, HIGH);
    delay(500);
    tone(BUZZER_PIN, 900, 500);
    digitalWrite(pinoLED, LOW);
    delay(500);
  }
}

float readTurbidity() {
  int sensorValue = analogRead(TURBIDITY_PIN);
  return (float)sensorValue / 1024.0 * 5.0;
}

void executeDrain(String motivo) {
  drainCount++;
  
  wakeUpDisplay();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DRENO ");
  lcd.print(drainCount);
  lcd.print("/");
  lcd.print(MAX_DRAINS);
  
  lcd.setCursor(0, 1);
  lcd.print(motivo.substring(0, 16)); 
  
  playTone(1500, 200);
  playTone(1200, 200);

  light.turnOn(); // Ativa o relé

  // Barra de progresso visual no LCD
  for (int i = 0; i < 10; i++) {
    delay(DRAIN_DURATION / 10);
    lcd.setCursor(i + 4, 1);
    lcd.print(">");
    lcd.print("*");
  }
  
  light.turnOff(); // Desliga o relé
  delay(200); // Pequena pausa para estabilização mecânica dos contatos
  
  // CHECAGEM DE SEGURANÇA MECÂNICA (CORRIGIDO):
  if (digitalRead(RELAY_NC_PIN) == HIGH) {
      dispararAlarmeFALHA();
  }
  
  playTone(1000, 500);
  
  lastDrainTime = millis(); 
  lastTurbidityValue = -1.0; 

  if (drainCount >= MAX_DRAINS) {
    dispararAlarmeLIMITE();
  }
}
 
void setup() {
  Serial.begin(9600);
  morse.translate("SOS");
  led.begin(5000);
  light.begin(); 
  
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(TURBIDITY_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(pinoLED, OUTPUT); 
  pinMode(LED_INCIDENCIA_PIN, OUTPUT); // ADICIONADO: Configuração do pino 11
  pinMode(RELAY_NC_PIN, INPUT_PULLUP); 
  
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("ELETRONIC-DRENO");
  delay(2000);
  lcd.clear();
  lastInteractionTime = millis(); // Inicializa tempo do display
}

void loop() {
  unsigned long currentTime = millis();
  float currentTurbidity = readTurbidity();
  
  Serial.print("Tensao Sensor: ");
  Serial.print(currentTurbidity);
  Serial.print(" V | Drenos executados: ");
  Serial.println(drainCount);

  // 1. Controle de brilho do LED indicador via Potenciômetro
  int mval = myPot.value(); 
  analogWrite(pinoLED, mval);

  // 2. Cálculo matemático de NTU baseado na tensão
  float NTU = -1120.4 * (currentTurbidity * currentTurbidity) + 5742.3 * currentTurbidity - 4353.8;
  if(NTU < 0) NTU = 0; 

  // 3. Atualização Inteligente do Display LCD
  if (abs(currentTurbidity - lastTurbidityValue) > 0.05) {
    wakeUpDisplay(); // ADICIONADO: Mantém display ativo ao detectar mudanças físicas
    
    if (displayIsOn) {
      lcd.setCursor(0, 0);
      lcd.print("Turbidez:");
      lcd.print((int)NTU); 
      lcd.print(" NTU    "); 
      
      lcd.setCursor(0, 1);
      if (currentTurbidity >= TURBIDITY_THRESHOLD) {
        lcd.print("Status: SUJA    ");
      } else {
        lcd.print("Status: MONITOR ");
      }
    }
    lastTurbidityValue = currentTurbidity;
  }

  // 4. Controle Automático do LED de Incidência (Pino 11)
  int brilhoLedIncidencia = map((int)NTU, 0, 1000, 0, 255);
  brilhoLedIncidencia = constrain(brilhoLedIncidencia, 0, 255);
  analogWrite(LED_INCIDENCIA_PIN, brilhoLedIncidencia); 

  // 5. Lógica de Acionamento do Dreno com Filtro de Cooldown
  if (currentTime - lastDrainTime >= COOLDOWN_INTERVAL) {
    if (currentTurbidity > TURBIDITY_THRESHOLD) {
      executeDrain("POR TURBIDEZ   ");
    } 
    else if (currentTime - lastDrainTime >= DRAIN_INTERVAL) {
      executeDrain("POR TEMPO      ");
      drainCount--; // Desconta para que o dreno preventivo por tempo não penalize o limite máximo de segurança
      drainCount = 0;
    }
  }

  // 6. Gerenciamento de Standby do Display (Timeout)
  if (displayIsOn && (currentTime - lastInteractionTime > DISPLAY_TIMEOUT)) {
    lcd.clear();
    lcd.noBacklight();
    displayIsOn = false;
  }
  
  delay(200); 
}
