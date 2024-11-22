#include <SPI.h>
#include <MFRC522.h>
#include <Servo.h>

#define RST_PIN 9  // Pin 9 para el reset del RC522
#define SS_PIN 10  // Pin 10 para el SS (SDA) del RC522
#define BUZZER_PIN 8 // Pin para el buzzer
#define SERVO_PIN 6  // Nuevo pin para el servo

MFRC522 mfrc522(SS_PIN, RST_PIN); // Creamos el objeto para el RC522

// Variable para el estado del servo
bool servoPosition = false; // false = posición original, true = posición asignada

// Crear un objeto Servo
Servo lockServo;

// Buffer para almacenar el ID de la tarjeta
String cardID = "";
String accessTag = ""; // Variable para almacenar el primer tag registrado

void setup() {
    Serial.begin(9600); // Iniciar la comunicación serial
    SPI.begin();        // Iniciar el Bus SPI
    mfrc522.PCD_Init(); // Iniciar el MFRC522

    pinMode(BUZZER_PIN, OUTPUT); // Configurar el pin del buzzer como salida

    // Ajustar la ganancia de la antena al máximo
    mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

    // Conectar el servo al nuevo pin
    lockServo.attach(SERVO_PIN);
    
    // Colocar el servo en la posición original (0 grados)
    lockServo.write(0);
    
    // Mensaje inicial
    Serial.println("Setup completo. Esperando tarjetas RFID...");
}

void loop() {
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
                if (servoPosition) {
                    lockServo.write(0); // Mover el servo a la posición original
                    servoPosition = false;
                    Serial.println("Servo en posición original");
                } else {
                    lockServo.write(180); // Mover el servo a la posición asignada
                    servoPosition = true;
                    Serial.println("Servo en posición asignada");
                }
            } else {
                Serial.println("Tarjeta desconocida");
            }
            
            // Terminamos la lectura de la tarjeta actual
            mfrc522.PICC_HaltA();
        }
    }
}
