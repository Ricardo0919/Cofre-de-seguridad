#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <SoftwareSerial.h>

// Crear un objeto para el sensor
MPU6050 sensor;

// Variables para almacenar las lecturas del acelerómetro
int ax, ay, az;

// Variables para manejar el tiempo de lectura del sensor
unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de 1 segundo para leer el sensor

// Definir pines para SoftwareSerial (cambiar si es necesario)
SoftwareSerial espSerial(2, 3); // RX, TX

void setup() {
  // Iniciar puertos seriales
  Serial.begin(9600);
  espSerial.begin(9600);
  Wire.begin();                    // Iniciar comunicación I²C
  sensor.initialize();             // Iniciar el sensor MPU6050

  // Verificar conexión con el sensor
  if (sensor.testConnection()) Serial.println("Sensor MPU6050 iniciado correctamente");
  else Serial.println("Error al iniciar el sensor MPU6050");

  // Mensaje inicial
  Serial.println("Setup completo. Esperando datos del acelerómetro...");
}

void loop() {
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

    // Enviar los datos del acelerómetro al ESP
    espSerial.print("Inclinación en X: ");
    espSerial.print(accel_ang_x);
    espSerial.print("\tInclinación en Y: ");
    espSerial.println(accel_ang_y);
  }
}
