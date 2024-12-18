#include <WiFi.h>
#include <PubSubClient.h>
#include <ThingSpeak.h>
#include <HardwareSerial.h>

// Configuración WiFi
const char* ssid = "Ricardo2";
const char* password = "ricki1903#$";

// Configuración MQTT
const char* mqttServer = "192.168.209.2";
const int mqttPort = 1883;
const char* topicAlert = "esp32_alert";
const char* topicStatus = "esp32_status";

// Configuración ThingSpeak
unsigned long channelID = 2746417;       // ID del canal de ThingSpeak
const char* WriteAPIKey = "1PJBDYNGF74R7QSC"; // API Key de escritura
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// Configuración Serial
#define RX_PIN 19 // RX del ESP32 conectado al TX del Arduino
#define TX_PIN 21 // TX del ESP32 conectado al RX del Arduino
HardwareSerial MySerial(1);

// Variables para ThingSpeak
float latestX = 0, latestY = 0;
unsigned long lastWriteTime = 0;
const unsigned long writeInterval = 15000; // Intervalo entre escrituras (15s)

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
    } else if (message == "NONE") {
      Serial.println("Recibido NONE desde MQTT. Enviando al Arduino...");
      MySerial.println("NONE");
    }
    // Confirmación de mensaje procesado
    mqttClient.publish("esp32/status", "Mensaje procesado");
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

  // Configurar ThingSpeak
  ThingSpeak.begin(wifiClient);
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
  //Serial.print("Datos del acelerómetro recibidos: ");
  //Serial.println(datosAcelerometro);

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

      // Enviar datos a ThingSpeak si corresponde
      if (millis() - lastWriteTime >= writeInterval) {
        enviarDatosAThingSpeak();
        lastWriteTime = millis();
      }
    } else {
      Serial.println("Error: Datos no válidos recibidos.");
    }
  } else {
    Serial.println("Error: Formato de datos del acelerómetro incorrecto.");
  }
}

void enviarDatosAThingSpeak() {
  ThingSpeak.setField(1, latestX);
  ThingSpeak.setField(2, latestY);
  if (ThingSpeak.writeFields(channelID, WriteAPIKey) == 200) {
    Serial.println("Datos del acelerómetro enviados a ThingSpeak:");
    Serial.print("X: ");
    Serial.println(latestX);
    Serial.print("Y: ");
    Serial.println(latestY);
  } else {
    Serial.println("Error: No se pudieron enviar los datos a ThingSpeak.");
  }
}

void reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Connected");
      mqttClient.subscribe("esp32/alert");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}
