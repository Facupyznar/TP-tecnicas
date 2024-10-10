#include "Stepper.h"

// Definir el número de pasos por revolución:
const int pasosPorRevolucion = 200;

// Inicializar la librería Stepper para los dos motores
Stepper motor2 = Stepper(pasosPorRevolucion, 25, 17, 18, 26);  // Motor 1
Stepper motor1 = Stepper(pasosPorRevolucion, 34, 35, 19, 5);  // Motor 2

// Variable de control para saber qué motor está activo
bool motor1Activo = true;

// Número de pasos necesarios para mover 2 cm (según el paso de la varilla)
int pasosPorMovimiento = 1000;  // Cambia este valor según el avance de tu varilla

void setup() {
  // Establecer la velocidad de ambos motores (en RPM):
  motor1.setSpeed(100);
  motor2.setSpeed(100);
}

void loop() {
  // Si el motor 1 está activo, mueve 2 cm y apaga el motor 2
  if (motor1Activo) {
    motor1.step(pasosPorMovimiento);  // Mueve el motor 1 la cantidad de pasos para 2 cm
    motor2.step(0);    // Motor 2 apagado
    delay(1000);       // Pausa para el motor 1
    motor1Activo = false;  // Cambiar el control para permitir que motor 2 gire
  }
  // Si el motor 2 está activo, mueve el motor 2 y apaga el motor 1
  else {
    motor2.step(pasosPorMovimiento);  // Mueve el motor 2
    motor1.step(0);    // Motor 1 apagado
    delay(1000);       // Pausa para el motor 2
    motor1Activo = true;  // Cambiar el control para que el motor 1 pueda moverse nuevamente
  }

  // Espera antes de la próxima secuencia
  delay(1000);
}
