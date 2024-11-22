#include "ThingSpeak.h"
#include "WiFi.h"
#include <HardwareSerial.h>

#define RX_PIN 19  // Pin RX del ESP32 (conectar al TX del Arduino Uno)
#define TX_PIN 21  // Pin TX del ESP32 (conectar al RX del Arduino Uno)

const char* ssid = "Ricardo";                        // SSID de vuestro router
const char* password = "ricki1903#$";                // Contraseña de vuestro router

unsigned long channelID = 2746417;                // ID de vuestro canal
const char* WriteAPIKey = "1PJBDYNGF74R7QSC";     // Write API Key de vuestro canal

WiFiClient cliente;
HardwareSerial MySerial(1);  // Usamos el puerto serial 1 del ESP32

float latestX = 0, latestY = 0;  // Variables para almacenar los últimos datos válidos
unsigned long lastWriteTime = 0;  // Tiempo de la última escritura en ThingSpeak
const unsigned long writeInterval = 15000;  // Intervalo mínimo entre escrituras (15 segundos)

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
  // Escuchar alertas desde MATLAB
  if (cliente.available()) {
    String alerta = cliente.readStringUntil('\n');
    alerta.trim();

    if (alerta == "ALERTA") {
      Serial.println("Alerta recibida. Enviando bloqueo al Arduino...");
      MySerial.println("BLOCK"); // Enviar mensaje al Arduino Uno
    }
  }

  leerDatosAcelerometro(); // Leer y enviar datos del acelerómetro a ThingSpeak
}

void leerDatosAcelerometro() {
  if (MySerial.available()) {
    String datosAcelerometro = MySerial.readStringUntil('\n');  // Leer datos hasta el fin de línea
    Serial.print("Datos del acelerómetro recibidos: ");
    Serial.println(datosAcelerometro);

    // Asumimos que los datos vienen en el formato "X: <valor>\tY: <valor>"
    int idxX = datosAcelerometro.indexOf("X: ");
    int idxY = datosAcelerometro.indexOf("\tY: ");

    if (idxX != -1 && idxY != -1) {
      // Extraer los valores y limpiar caracteres adicionales
      String inclinacionXStr = datosAcelerometro.substring(idxX + 3, idxY);
      inclinacionXStr.trim();
      String inclinacionYStr = datosAcelerometro.substring(idxY + 3);
      inclinacionYStr.trim();

      // Convertir las cadenas a flotantes
      float inclinacionX = inclinacionXStr.toFloat();
      float inclinacionY = inclinacionYStr.toFloat();

      if (!isnan(inclinacionX) && !isnan(inclinacionY)) {
        latestX = inclinacionX;
        latestY = inclinacionY;
        Serial.print("X: ");
        Serial.println(latestX);
        Serial.print("Y: ");
        Serial.println(latestY);

        // Enviar datos si ha pasado el intervalo mínimo
        if (millis() - lastWriteTime >= writeInterval) {
          enviarDatosAThingSpeak();
          lastWriteTime = millis();  // Actualizar el tiempo de la última escritura
        }
      } else {
        Serial.println("Error: Datos no válidos recibidos.");
      }
    } else {
      Serial.println("Error: Formato de datos del acelerómetro incorrecto.");
    }
  }
}

void enviarDatosAThingSpeak() {
  ThingSpeak.setField(1, latestX);
  ThingSpeak.setField(2, latestY);
  if (ThingSpeak.writeFields(channelID, WriteAPIKey) == 200) {
    Serial.println("Datos del acelerómetro enviados a ThingSpeak!");
  } else {
    Serial.println("Error: No se pudieron enviar los datos a ThingSpeak.");
  }
}
