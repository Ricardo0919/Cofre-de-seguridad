#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include "Wire.h"
#include <SoftwareSerial.h>

#define RST_PIN 9   // Pin 9 para el reset del RC522
#define SS_PIN 10   // Pin 10 para el SS (SDA) del RC522
#define BUZZER_PIN 8 // Pin para el buzzer
#define SERVO_PIN 7  // Pin para el servo

MFRC522 mfrc522(SS_PIN, RST_PIN); // Creamos el objeto para el RC522

// Crear un objeto para el sensor
MPU6050 sensor;

// Variables para almacenar las lecturas del acelerómetro
int16_t ax, ay, az;

// Variables para manejar el tiempo de lectura del sensor
unsigned long previousMillis = 0;
const long interval = 1000;  // Intervalo de 1 segundo para leer el sensor

// Crear un objeto Servo
Servo lockServo;

// Buffer para almacenar el ID de la tarjeta
String cardID = "";
String accessTag = ""; // Variable para almacenar el primer tag registrado

// Definir pines para SoftwareSerial
SoftwareSerial espSerial(2, 3); // RX, TX

// Variables para el estado del servo
bool servoPosition = false; // false = posición original (0 grados), true = posición asignada (90 grados)
bool shouldMoveServo = false; // Variable para indicar si el servo debería moverse

// Variables para el bloqueo por alerta
bool blockedByAlert = false;          // Indica si el servo está bloqueado por alerta
unsigned long blockStartTime = 0;     // Tiempo en el que se inició el bloqueo
int previousServoPosition = 0;        // Posición previa del servo antes del bloqueo

void setup() {
    Serial.begin(9600); // Iniciar la comunicación serial
    espSerial.begin(9600); // Iniciar la comunicación serial para el ESP32
    Wire.begin();                    // Iniciar comunicación I²C
    sensor.initialize();             // Iniciar el sensor MPU6050

    // Verificar conexión con el sensor
    if (sensor.testConnection()) Serial.println("Sensor MPU6050 iniciado correctamente");
    else Serial.println("Error al iniciar el sensor MPU6050");

    // Iniciar el bus SPI y el lector RFID
    SPI.begin();
    mfrc522.PCD_Init();

    pinMode(BUZZER_PIN, OUTPUT); // Configurar el pin del buzzer como salida

    // Ajustar la ganancia de la antena al máximo
    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

    // Conectar el servo al pin
    lockServo.attach(SERVO_PIN);
    
    // Colocar el servo en la posición original (0 grados)
    lockServo.write(0);
    delay(500);
    lockServo.detach(); // Desconectar el servo

    // Mensaje inicial
    Serial.println("Setup completo. Esperando tarjetas RFID y datos del acelerómetro...");
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
        // Serial.print("X: ");
        // Serial.print(accel_ang_x); 
        // Serial.print("\tY: ");
        // Serial.println(accel_ang_y);

        // Enviar los datos del acelerómetro al ESP32
        espSerial.print("X: ");
        espSerial.print(accel_ang_x);
        espSerial.print("\tY: ");
        espSerial.println(accel_ang_y);
    }

    // Revisamos si hay nuevas tarjetas presentes
    if (mfrc522.PICC_IsNewCardPresent()) {
        // Seleccionamos una tarjeta
        if (mfrc522.PICC_ReadCardSerial()) {
            // Enviamos serialmente su UID en formato decimal
            Serial.print("Card UID (Decimal):");
            cardID = ""; // Reiniciar el buffer para el nuevo UID
            for (byte i = 0; i < mfrc522.uid.size; i++) {
                Serial.print(" ");
                Serial.print(mfrc522.uid.uidByte[i], DEC);
                cardID += String(mfrc522.uid.uidByte[i], DEC);
            }
            Serial.println();

            // Hacer sonar el buzzer
            digitalWrite(BUZZER_PIN, HIGH); // Encender el buzzer
            delay(100); // Mantener el buzzer encendido por 100 milisegundos
            digitalWrite(BUZZER_PIN, LOW); // Apagar el buzzer

            // Si no hay una tarjeta de acceso registrada, registrar la actual
            if (accessTag == "") {
                accessTag = cardID;
                Serial.print("Tarjeta de acceso registrada: ");
                Serial.println(accessTag);
            }

            // Verificar si el ID leído coincide con el ID de acceso registrado
            bool tagFound = (cardID == accessTag);

            // Mostrar el resultado y mover el servo si se encuentra un ID conocido
            if (tagFound) {
                Serial.println("Tarjeta reconocida");
                if (!blockedByAlert) { // Solo mover el servo si no está bloqueado por alerta
                    shouldMoveServo = true;
                } else {
                    Serial.println("El servo está bloqueado por alerta. Espera a que se desbloquee.");
                }
            } else {
                Serial.println("Tarjeta desconocida");
            }
            
            // Terminamos la lectura de la tarjeta actual
            mfrc522.PICC_HaltA();
        }
    }

    // Revisar comandos desde ESP32
    if (espSerial.available()) {
        String comando = espSerial.readStringUntil('\n');
        comando.trim();

        Serial.print("Comando recibido desde ESP32: ");
        Serial.println(comando);

        if (comando == "BLOCK") {
            Serial.println("Bloqueo activado por alerta.");
            
            // Mover el servo a la posición bloqueada
            lockServo.attach(SERVO_PIN);
            lockServo.write(0); // Posición bloqueada (0 grados)
            delay(500);
            lockServo.detach();
            
            // Hacer sonar el buzzer
            digitalWrite(BUZZER_PIN, HIGH);
            delay(1000);
            digitalWrite(BUZZER_PIN, LOW);
            delay(1000);
            digitalWrite(BUZZER_PIN, HIGH);
            delay(1000);
            digitalWrite(BUZZER_PIN, LOW);

            // Iniciar el bloqueo
            blockedByAlert = true;
            blockStartTime = millis();
        }
    }

    // Comprobar si el servo está bloqueado por alerta y si han pasado 3 segundos
    if (blockedByAlert) {
        if (millis() - blockStartTime >= 3000) {
            // 3 segundos han pasado, regresar el servo a la posición anterior
            lockServo.attach(SERVO_PIN);
            lockServo.write(previousServoPosition); // Regresar a la posición previa
            delay(500);
            lockServo.detach();

            blockedByAlert = false;
            Serial.println("Bloqueo desactivado, servo regresó a posición anterior.");
        }
    }

    // Mover el servo si debería moverse y no está bloqueado por alerta
    if (shouldMoveServo && !blockedByAlert) {
        lockServo.attach(SERVO_PIN); // Conectar el servo
        if (servoPosition) {
            lockServo.write(0); // Mover el servo a la posición original (0 grados)
            servoPosition = false;
            Serial.println("Servo en posición original");
        } else {
            lockServo.write(20); // Mover el servo a la posición asignada (90 grados)
            servoPosition = true;
            Serial.println("Servo en posición asignada");
        }
        delay(500); // Esperar para evitar impulsos
        lockServo.detach(); // Desconectar el servo
        shouldMoveServo = false; // Resetear el indicador
    }
}
