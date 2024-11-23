#include <WiFi.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>

// Configuración WiFi
const char* ssid = "Ricardo2";
const char* password = "ricki1903#$";

// Configuración MQTT
// Casa
//const char* mqttServer = "192.168.1.102";
// TEC
const char* mqttServer = "192.168.209.2";
const int mqttPort = 1883;
const char* topicAlert = "esp32/alert";
const char* topicStatus = "esp32/status";
const char* topicEjeX = "esp32/ejex";
const char* topicEjeY = "esp32/ejey";

// Configuración Serial
#define RX_PIN 19 // RX del ESP32 conectado al TX del Arduino
#define TX_PIN 21 // TX del ESP32 conectado al RX del Arduino
HardwareSerial MySerial(1);

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Variables para datos del acelerómetro
float latestX = 0, latestY = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String message = "";
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.print("Received message on topic [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println(message);

  // Manejar alertas de MQTT
  if (String(topic) == "esp32/alert") {
    if (message == "BLOCK") {
      Serial.println("Recibido BLOCK desde MQTT. Enviando al Arduino...");
      MySerial.println("BLOCK");
      Serial.println("Comando 'BLOCK' enviado al Arduino.");
    }
    // Confirmación de mensaje procesado
    mqttClient.publish(topicStatus, "Mensaje procesado");
  }
}

void setup() {
  // Configuración inicial de Serial
  Serial.begin(9600);
  MySerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);

  // Conexión WiFi
  Serial.println("Conectando al WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");

  // Configurar MQTT
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);

  while (!mqttClient.connected()) {
    Serial.println("Conectando al broker MQTT...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Conectado al broker MQTT!");
      mqttClient.subscribe("esp32/alert");
    } else {
      Serial.print("Error de conexión MQTT: ");
      Serial.println(mqttClient.state());
      delay(2000);
    }
  }
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop(); // Mantener conexión MQTT

  // Leer y procesar datos del Arduino
  if (MySerial.available()) {
    String datosAcelerometro = MySerial.readStringUntil('\n');
    procesarDatosAcelerometro(datosAcelerometro);
  }
}

void procesarDatosAcelerometro(String datosAcelerometro) {
  // Asumimos formato "X: <valor>\tY: <valor>"
  int idxX = datosAcelerometro.indexOf("X: ");
  int idxY = datosAcelerometro.indexOf("\tY: ");

  if (idxX != -1 && idxY != -1) {
    // Extraer y convertir valores
    String inclinacionXStr = datosAcelerometro.substring(idxX + 3, idxY);
    inclinacionXStr.trim();
    String inclinacionYStr = datosAcelerometro.substring(idxY + 3);
    inclinacionYStr.trim();

    float inclinacionX = inclinacionXStr.toFloat();
    float inclinacionY = inclinacionYStr.toFloat();

    if (!isnan(inclinacionX) && !isnan(inclinacionY)) {
      latestX = inclinacionX;
      latestY = inclinacionY;

      // Publicar datos en MQTT
      mqttClient.publish(topicEjeX, String(latestX).c_str());
      mqttClient.publish(topicEjeY, String(latestY).c_str());
    }
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    if (mqttClient.connect("ESP32Client")) {
      mqttClient.subscribe(topicAlert);
    } else {
      delay(5000);
    }
  }
}
