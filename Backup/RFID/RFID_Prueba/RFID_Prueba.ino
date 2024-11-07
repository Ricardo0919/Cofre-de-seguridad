#include <SoftwareSerial.h>

// Configura el puerto serial en los pines RX y TX
SoftwareSerial rfidSerial(10, 11); // RX, TX

void setup() {
  // Inicia la comunicación serial con la computadora
  Serial.begin(9600); 

  // Inicia la comunicación con el módulo RFID
  rfidSerial.begin(9600);

  Serial.println("Esperando tarjeta RFID...");
}

void loop() {
  // Verifica si hay datos disponibles en el lector RFID
  if (rfidSerial.available() > 0) {
    String rfidData = ""; // String para almacenar los datos RFID

    // Lee los datos y los guarda en la variable rfidData
    while (rfidSerial.available() > 0) {
      rfidData += (char)rfidSerial.read();
    }

    // Muestra los datos recibidos en el Monitor Serial
    Serial.println("Tarjeta detectada:");
    Serial.println(rfidData);
  }
}
