// Librerías necesarias para controlar el MPU6050, I2C y el Servo
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <Servo.h>

// Definir pines
#define BUTTON_PIN 7
int SERVO_PIN = 9;

// Crear un objeto para el sensor y para el servo
MPU6050 sensor;
Servo myServo;

// Variables para almacenar las lecturas del acelerómetro
int ax, ay, az;

// Variables de estado del botón y salida del servo
bool buttonState = LOW;
bool lastButtonState = HIGH;
bool outputState = LOW;

// Variables para manejar el tiempo de lectura del sensor
unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de 1 segundo para leer el sensor

// Configuración inicial
void setup() {
  Serial.begin(9600);             // Iniciar comunicación serial
  Wire.begin();                    // Iniciar comunicación I²C
  sensor.initialize();             // Iniciar el sensor MPU6050

  // Verificar conexión con el sensor
  if (sensor.testConnection()) Serial.println("Sensor MPU6050 iniciado correctamente");
  else Serial.println("Error al iniciar el sensor MPU6050");

  // Configurar botón y servo
  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Configurar botón con resistencia interna
  myServo.attach(SERVO_PIN);          // Conectar el servo
  myServo.write(0);                   // Posicionar el servo en 0 grados
}

// Bucle principal
void loop() {
  // Leer el estado del botón
  bool currentButtonState = digitalRead(BUTTON_PIN);

  // Verificar si el botón ha cambiado de estado
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    // Alternar el estado de la salida
    outputState = !outputState;

    // Mover el servo según el estado de salida
    if (outputState) {
      myServo.write(90);      // Mover el servo a 90 grados
    } else {
      myServo.write(0);       // Regresar el servo a 0 grados
    }

    Serial.print("Estado del Servo (0: 0 grados, 1: 90 grados): ");
    Serial.println(outputState);

    delay(50);  // Evitar rebotes del botón
  }

  // Actualizar el estado anterior del botón
  lastButtonState = currentButtonState;

  // Verificar si ha pasado el tiempo necesario para leer el sensor
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // Guardar el tiempo de la última lectura
    previousMillis = currentMillis;

    // Leer los valores del acelerómetro
    sensor.getAcceleration(&ax, &ay, &az);

    // Calcular los ángulos de inclinación en los ejes X e Y
    float accel_ang_x = atan(ax / sqrt(pow(ay, 2) + pow(az, 2))) * (180.0 / 3.14);
    float accel_ang_y = atan(ay / sqrt(pow(ax, 2) + pow(az, 2))) * (180.0 / 3.14);

    // Mostrar los ángulos de inclinación en el monitor serial
    Serial.print("Inclinación en X: ");
    Serial.print(accel_ang_x); 
    Serial.print("\tInclinación en Y: ");
    Serial.println(accel_ang_y);
  }
}
