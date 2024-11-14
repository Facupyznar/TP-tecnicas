#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Stepper.h"

// Seteamos columnas y filas del display
int lcdColumns = 16;
int lcdRows = 2;

// Seteo de dirección, columnas, filas del LCD
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Pines para el motor del pan
const int dirPinPan = 5;
const int stepPinPan = 19;
const int stepsPerRevolution = 200;
const int velocity = 750;

// Motor de la cuchilla controlado con puente H L298N
Stepper motorCuchilla(stepsPerRevolution, 25, 17, 18, 26);
const int velocidadCuchilla = 100; // Velocidad en RPM para la cuchilla

// Definimos los pines de los pulsadores y los fines de carrera
const int button1Pin = 32;
const int button2Pin = 27;
const int button3Pin = 33;
const int endOfRacePan = 16;
const int endOfRaceCuchilla = 3;

// Variables para almacenar estados
int button1State = 0;
int button2State = 0;
int button3State = 0;
volatile bool finDeCarreraPanActivado = false;
volatile bool finDeCarreraCuchillaActivado = false;
int selectedOption = 0;
float thickness = 0.0;
int cortando = 0;

// Variables de control de secuencia de motores
bool motorPanActivo = true;
bool motorCuchillaActivo = false;
int pasosCuchilla = 4200;
int pasosPan;
int movimientosPan = 0;
unsigned long ultimoMovimientoCuchilla = 0;
const unsigned long TIEMPO_ESPERA_ENTRE_MOTORES = 1000; // 1 segundo entre movimientos

void IRAM_ATTR endOfRacePanInterrupt() {
  finDeCarreraPanActivado = true;
}

void IRAM_ATTR endOfRaceCuchillaInterrupt() {
  finDeCarreraCuchillaActivado = true;
}

void setup() {
  Serial.begin(115200); // Para debugging
  
  // Configuración de pines
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(endOfRacePan, INPUT_PULLUP);
  pinMode(endOfRaceCuchilla, INPUT_PULLUP);
  pinMode(stepPinPan, OUTPUT);
  pinMode(dirPinPan, OUTPUT);

  // Configurar interrupciones
  attachInterrupt(digitalPinToInterrupt(endOfRacePan), endOfRacePanInterrupt, FALLING);
  attachInterrupt(digitalPinToInterrupt(endOfRaceCuchilla), endOfRaceCuchillaInterrupt, FALLING);

  // Inicializar LCD
  lcd.init();
  lcd.backlight();
  
  // Configurar velocidad inicial del motor de la cuchilla
  motorCuchilla.setSpeed(velocidadCuchilla);

  // Mostrar menú inicial
  mostrarMenuInicial();
}

void loop() {
  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);
  button3State = digitalRead(button3Pin);

  if (finDeCarreraPanActivado) {
    moverMotorPan(false, movimientosPan * pasosPan);
    movimientosPan = 0;
    finDeCarreraPanActivado = false;
    detenerMotores();
    return;
  }

  // Manejo de selección de grosor
  manejarSeleccionGrosor();

  // Proceso de corte
  if (cortando == 1) {
    unsigned long tiempoActual = millis();
    
    if (motorPanActivo) {
      Serial.println("Moviendo motor pan..."); // Debug
      moverMotorPan(true, pasosPan);
      movimientosPan++;
      motorPanActivo = false;
      motorCuchillaActivo = true;
      ultimoMovimientoCuchilla = tiempoActual;
      
    } else if (motorCuchillaActivo && 
              (tiempoActual - ultimoMovimientoCuchilla >= TIEMPO_ESPERA_ENTRE_MOTORES)) {
      Serial.println("Moviendo motor cuchilla..."); // Debug
      
      // Mover la cuchilla en una dirección
      motorCuchilla.step(-pasosCuchilla);
      delay(2000); // Pequeña pausa
      motorCuchilla.step(pasosCuchilla);
      
      // Si el fin de carrera de la cuchilla no está activado, regresar
      //if (!finDeCarreraCuchillaActivado) {
        //motorCuchilla.step(pasosCuchilla); // Regresar la cuchilla
      //}
      
      //finDeCarreraCuchillaActivado = false;
      motorCuchillaActivo = false;
      motorPanActivo = true;
      
      delay(500); // Pausa antes de continuar con el siguiente ciclo
    }
  }
}

void moverMotorPan(bool sentido, int pasos) {
  digitalWrite(dirPinPan, sentido ? LOW : HIGH);
  
  for (int x = 0; x < pasos; x++) {
    digitalWrite(stepPinPan, HIGH);
    delayMicroseconds(velocity);
    digitalWrite(stepPinPan, LOW);
    delayMicroseconds(velocity);
  }
}

void manejarSeleccionGrosor() {
  if (selectedOption == 0) {
    if (button1State == LOW) { 
      selectedOption = 1;
      thickness = 1.25;
      pasosPan = 1000;
      requestConfirmation();
      delay(200);
    } 
    else if (button2State == LOW) { 
      selectedOption = 2;
      thickness = 2.0;
      pasosPan = 1600;
      requestConfirmation();
      delay(200);
    } 
    else if (button3State == LOW) { 
      selectedOption = 3;
      thickness = 3.0;
      pasosPan = 2400;
      requestConfirmation();
      delay(200);
    }
  }

  if (selectedOption != 0) {
    if (button1State == LOW) { 
      confirmSelection();
      delay(200);
      selectedOption = 0;
    } else if (button3State == LOW) {
      resetDisplay();
      delay(200);
      selectedOption = 0;
    }
  }
}

void mostrarMenuInicial() {
  lcd.setCursor(3, 0); 
  lcd.print("Grosor(cm)");
  lcd.setCursor(2, 1);
  lcd.print("1.25");
  lcd.setCursor(9, 1);
  lcd.print("2");
  lcd.setCursor(13, 1);
  lcd.print("3");
}

void confirmSelection() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirmado!");
  lcd.setCursor(0, 1);
  lcd.print("Grosor: ");
  lcd.print(thickness);
  lcd.print("cm");
  delay(2000);
  cuttingMessage();
}

void resetDisplay() {
  selectedOption = 0;
  lcd.clear();
  mostrarMenuInicial();
  delay(200);
}

void requestConfirmation() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Confirmar ");
  lcd.print(thickness);
  lcd.print("?");
  lcd.setCursor(0, 1);
  lcd.print("No");
  lcd.setCursor(14, 1);
  lcd.print("Si");
}

void cuttingMessage() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cortando");
  lcd.setCursor(8, 0);
  lcd.print("...");
  delay(1000);
  cortando = 1;
}

void detenerMotores() {
  digitalWrite(stepPinPan, LOW);
  motorCuchilla.step(0);
  selectedOption = 0;
  cortando = 0; 
  resetDisplay();
}