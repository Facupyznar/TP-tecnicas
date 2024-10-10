#include <Wire.h>
#include <LiquidCrystal_I2C.h>

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

void setup(){
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
}

void loop(){
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
      requestConfirmation();
      delay(200);  // Esperar a que el botón se suelte (anti rebote)
    } 
    else if (button2State == LOW) { 
      selectedOption = 2;
      thickness = 2.0;
      requestConfirmation();
      delay(200);  // Esperar a que el botón se suelte
    } 
    else if (button3State == LOW) { 
      selectedOption = 3;
      thickness = 3.0;
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

  if (endOfRaceState == LOW){
    resetDisplay();
    delay(200);
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
}