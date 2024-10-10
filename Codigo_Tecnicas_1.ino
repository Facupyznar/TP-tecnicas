#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "Stepper.h"


// Seteamos columnas y filas del display
int lcdColumns = 16;
int lcdRows = 2;

// Seteo de dirección, columnas, filas del LCD (ajusta la dirección si es necesario)
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  

// Definimos los pines de los pulsadores
const int button1Pin = 32; // Pin para el botón 1 (1.25 cm)
const int button2Pin = 27;  // Pin para el botón 2 (2 cm)
const int button3Pin = 33; // Pin para el botón 3 (3 cm)
const int endOfRace = 16; // Pin del fin de carrera

// Variables para almacenar el estado de los botones
int button1State = 0;
int button2State = 0;
int button3State = 0;
int endOfRaceState = 0;
int selectedOption = 0; // 0: ninguna opción seleccionada
float thickness = 0.0;  // Variable para almacenar el grosor elegido

// Definir el número de pasos por revolución:
const int pasosPorRevolucion = 200;

// Defino variable para iniciar corte
int cortando = 0;

// Inicializar la librería Stepper para los dos motores
Stepper motorCuchilla = Stepper(pasosPorRevolucion, 25, 17, 18, 26);  // Motor cuchilla
Stepper motorPan = Stepper(pasosPorRevolucion, 34, 35, 19, 5);  // Motor pan


// Variable de control para saber qué motor está activo
bool motorPanActivo = true;

int pasosCuchilla = 1600;  // Pasos para mover la cuchilla 2cm
int pasosPan; // Pasos para mover el pan con el grosor elegido

void setup() {
  // Configuramos los pines de los botones como entradas
  pinMode(button1Pin, INPUT_PULLUP);
  pinMode(button2Pin, INPUT_PULLUP);
  pinMode(button3Pin, INPUT_PULLUP);
  pinMode(endOfRace, INPUT_PULLUP);

  // Inicializo LCD
  lcd.init();
  lcd.backlight();

  // Mostrar el menú inicial
  lcd.setCursor(3, 0); 
  lcd.print("Grosor(cm)");
  lcd.setCursor(2, 1);
  lcd.print("1.25");
  lcd.setCursor(9, 1);
  lcd.print("2");
  lcd.setCursor(13, 1);
  lcd.print("3");
  // Establecer la velocidad de ambos motores (en RPM):
  motorPan.setSpeed(100);
  motorCuchilla.setSpeed(100);
}

void loop() {
  // Leemos el estado de los botones
  button1State = digitalRead(button1Pin);
  button2State = digitalRead(button2Pin);
  button3State = digitalRead(button3Pin);
  endOfRaceState = digitalRead(endOfRace);

  // Si no hay una opción seleccionada, detectamos el botón presionado
  if (selectedOption == 0) {
    if (button1State == LOW) { 
      selectedOption = 1;
      thickness = 1.25;
      pasosPan = 1000; // Pasos para mover el pan 1.25cm
      requestConfirmation();
      delay(200);  // Esperar a que el botón se suelte (anti rebote)
    } 
    else if (button2State == LOW) { 
      selectedOption = 2;
      thickness = 2.0;
      pasosPan = 1600; // Pasos para mover el pan 2cm
      requestConfirmation();
      delay(200);  // Esperar a que el botón se suelte
    } 
    else if (button3State == LOW) { 
      selectedOption = 3;
      thickness = 3.0;
      pasosPan = 2400; // Pasos para mover el pan 3cm
      requestConfirmation();
      delay(200);  // Esperar a que el botón se suelte
    }
  }

  // Si ya hay una opción seleccionada, confirmar o cancelar
  if (selectedOption != 0) {
    button1State = HIGH;
    button2State = HIGH;
    button3State = HIGH;
    button1State = digitalRead(button1Pin);
    button2State = digitalRead(button2Pin);
    button3State = digitalRead(button3Pin);
    if (button1State == LOW) { 
      confirmSelection();
      delay(200);  // Esperar a que el botón se suelte
      selectedOption = 0;
    }
    else if (button3State == LOW) {
      resetDisplay();
      delay(200);  // Esperar a que el botón se suelte
      selectedOption = 0;
    }
  }
  if (cortando == 1){
    // Si el motor pan está activo, mueve 2 cm y apaga el motor cuchilla
    if (motorPanActivo) {
      motorPan.step(pasosPan);  // Mueve el motor pan la cantidad de pasos para el grosor elegido
      motorPanActivo = false;  // Cambiar el control para permitir que motor cuchilla gire
    }
  // Si el motor cuchilla está activo, mueve el motor chuchilla y apaga el motor pan
    else {
      motorCuchilla.step(pasosCuchilla);  // Mueve el motor cuchilla
      delay(1000);       // Pausa para el motor cuchilla
      motorPanActivo = true;  // Cambiar el control para que el motor pan pueda moverse nuevamente
    }
  // Espera antes de la próxima secuencia
    delay(1000);
  } 
  // Verificamos si el fin de carrera ha sido presionado
  if (endOfRaceState == LOW) {
    detenerMotores();
    return; // Salir del loop para que no siga ejecutando corte
  }
}

void confirmSelection() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Confirmado!");
  lcd.setCursor(0, 1);
  lcd.print("Grosor: ");
  lcd.print(thickness);
  lcd.print("cm");
  delay(2000); // Pausa para mostrar el mensaje
  cuttingMessage();
}

void resetDisplay() {
  // Reiniciamos el menú después de confirmar
  selectedOption = 0;
  lcd.clear();
  lcd.setCursor(3, 0); 
  lcd.print("Grosor(cm)");
  lcd.setCursor(2, 1);
  lcd.print("1.25");
  lcd.setCursor(9, 1);
  lcd.print("2");
  lcd.setCursor(13, 1);
  lcd.print("3");
  delay(200);
}

void requestConfirmation(){
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
  delay(1000);  // Esperar un momento antes de reiniciar el menú
  cortando = 1;
}

// Función para detener los motores y reiniciar
void detenerMotores() {
  motorPan.step(0);      // Detener motor del pan
  motorCuchilla.step(0); // Detener motor de la cuchilla
  selectedOption = 0;
  cortando = 0; 
  resetDisplay();

}
