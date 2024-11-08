// Librerías necesarias para controlar el MPU6050, I2C y el Servo
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <Servo.h>

// Definir el pin del servo
int SERVO_PIN = 9;

// Crear un objeto para el sensor y para el servo
MPU6050 sensor;
Servo myServo;

// Variables para almacenar las lecturas del acelerómetro
int ax, ay, az;

// Variable de estado del servo
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

  // Configurar el servo
  myServo.attach(SERVO_PIN);       // Conectar el servo
  myServo.write(0);                // Posicionar el servo en 0 grados
}

// Bucle principal
void loop() {
  // Verificar si hay datos disponibles en el puerto serial
  if (Serial.available() > 0) {
    // Leer el valor enviado desde la entrada serial
    int input = Serial.parseInt();

    // Controlar el servo basado en el valor recibido (1 para 90 grados, 0 para 0 grados)
    if (input == 1) {
      outputState = HIGH;
      myServo.write(90);  // Mover el servo a 90 grados
    } else if (input == 0) {
      outputState = LOW;
      myServo.write(0);   // Mover el servo a 0 grados
    }

    // Mostrar el estado actual del servo en el monitor serial
    Serial.print("Estado del Servo (0: 0 grados, 1: 90 grados): ");
    Serial.println(outputState);
  }

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
