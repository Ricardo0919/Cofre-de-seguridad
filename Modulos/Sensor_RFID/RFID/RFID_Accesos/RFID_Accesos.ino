#include <SoftwareSerial.h>

// Configura el puerto serial en los pines RX y TX
SoftwareSerial rfidSerial(10, 11); // RX, TX

// Define los IDs permitidos en un arreglo
String allowedIds[] = {"50008FAF3949"};
const int numAllowedIds = sizeof(allowedIds) / sizeof(allowedIds[0]);

// Variable para acumular los datos de la tarjeta
String rfidData = "";

void setup() {
  // Inicia la comunicación serial con la computadora
  Serial.begin(9600); 

  // Inicia la comunicación con el módulo RFID
  rfidSerial.begin(9600);

  Serial.println("Esperando tarjeta RFID...");
}

void loop() {
  // Verifica si hay datos disponibles en el lector RFID
  while (rfidSerial.available() > 0) {
    char readChar = (char)rfidSerial.read();

    // Filtrar caracteres no válidos
    if (isHexadecimalDigit(readChar)) { // Solo aceptamos caracteres hexadecimales
      rfidData += readChar;
    } else if (readChar == '\n' || readChar == '\r') {  // Fin del ID detectado
      if (rfidData.length() > 0) {
        Serial.println("Tarjeta detectada:");
        Serial.println(rfidData);

        // Verifica si el ID de la tarjeta está en el arreglo de IDs permitidos
        bool accessGranted = false;
        for (int i = 0; i < numAllowedIds; i++) {
          if (rfidData == allowedIds[i]) {
            accessGranted = true;
            break;
          }
        }

        // Muestra el mensaje correspondiente
        if (accessGranted) {
          Serial.println("Acceso concedido");

        } 
        else {
          Serial.println("Acceso denegado");
        }

        // Limpia rfidData para la próxima lectura
        rfidData = "";
      }
    }
  }

  

}

// // Verificar si el botón ha cambiado de estado
  // if (currentButtonState == LOW && lastButtonState == HIGH) {
  //   // Alternar el estado de la salida
  //   outputState = !outputState;

  //   // Mover el servo según el estado de salida
  //   if (outputState) {
  //     myServo.write(90);      // Mover el servo a 90 grados
  //   } else {
  //     myServo.write(0);       // Regresar el servo a 0 grados
  //   }

  //   Serial.print("Estado del Servo (0: 0 grados, 1: 90 grados): ");
  //   Serial.println(outputState);

  //   delay(50);  // Evitar rebotes del botón
  // }

  // // Actualizar el estado anterior del botón
  // lastButtonState = currentButtonState;
