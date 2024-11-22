#include "ThingSpeak.h"
#include "WiFi.h"
#include <HardwareSerial.h>

#define RX_PIN 16  // Pin RX del ESP32 (conectar al TX del Arduino Uno)
#define TX_PIN 17  // Pin TX del ESP32 (conectar al RX del Arduino Uno)

const char* ssid = "Papuseñal";                        // SSID de vuestro router.
const char* password = "82341950";                // Contraseña de vuestro router.

unsigned long channelID = 2746417;                // ID de vuestro canal.
const char* WriteAPIKey = "1PJBDYNGF74R7QSC";     // Write API Key de vuestro canal.

WiFiClient cliente;
HardwareSerial MySerial(1);  // Usamos el puerto serial 1 del ESP32

void setup() {
  Serial.begin(9600); // Iniciar la comunicación serial
  MySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);  // Inicializamos el puerto serial 1 del ESP32

  Serial.println("Conectando al WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  ThingSpeak.begin(cliente);
}

void loop() {
  recibirYEnviarDatosAcelerometro();
  delay(2000);  // Espera de 2 segundos entre cada envío de datos
}

void recibirYEnviarDatosAcelerometro() {
  if (MySerial.available()) {
    String datosAcelerometro = MySerial.readStringUntil('\n');  // Leer datos hasta el fin de línea
    Serial.print("Datos del acelerómetro recibidos: ");
    Serial.println(datosAcelerometro);

    // Asumimos que los datos vienen en el formato "Inclinación en X: <valor>\tInclinación en Y: <valor>"
    int idxX = datosAcelerometro.indexOf("Inclinación en X: ");
    int idxY = datosAcelerometro.indexOf("\tInclinación en Y: ");

    if (idxX != -1 && idxY != -1) {
      // Extraer los valores y limpiar caracteres adicionales
      String inclinacionXStr = datosAcelerometro.substring(idxX + 18, idxY);
      inclinacionXStr.trim();
      String inclinacionYStr = datosAcelerometro.substring(idxY + 18);
      inclinacionYStr.trim();

      // Verificar que inclinacionYStr solo tenga caracteres numéricos o de punto decimal
      inclinacionYStr.replace(":", ""); // Remover ":" en caso de que esté al inicio

      float inclinacionX = inclinacionXStr.toFloat();
      float inclinacionY = inclinacionYStr.toFloat();

      Serial.print("Inclinación en X: ");
      Serial.println(inclinacionX);
      Serial.print("Inclinación en Y: ");
      Serial.println(inclinacionY);

      // Solo enviar datos si ambos son válidos
      if (!isnan(inclinacionX) && !isnan(inclinacionY)) {
        ThingSpeak.setField(1, inclinacionX);
        ThingSpeak.setField(2, inclinacionY);
        if (ThingSpeak.writeFields(channelID, WriteAPIKey) == 200) {
          Serial.println("Datos del acelerómetro enviados a ThingSpeak!");
        } else {
          Serial.println("Error: No se pudieron enviar los datos a ThingSpeak.");
        }
      } else {
        Serial.println("Error: Datos no válidos recibidos.");
      }
    } else {
      Serial.println("Error: Formato de datos del acelerómetro incorrecto.");
    }
  }
}
