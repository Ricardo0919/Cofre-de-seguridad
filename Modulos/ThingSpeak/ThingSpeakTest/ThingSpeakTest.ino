#include "ThingSpeak.h"
#include "WiFi.h"
#include "DHT.h"

#define pin1 18       //Pin del DHT11.

const char* ssid = "Ricardo";                        //SSID de vuestro router.
const char* password = "ricki1903#$";                //Contraseña de vuestro router.

unsigned long channelID = 2746293;                //ID de vuestro canal.
const char* WriteAPIKey = "PV5SLL82A8FP9MQN";     //Write API Key de vuestro canal.

WiFiClient cliente;

DHT dht1(pin1, DHT11);    //El azul.

void setup() {
  Serial.begin(9600);
  Serial.println("Test de sensores:");

  WiFi.begin(ssid,password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Wifi conectado!");

  ThingSpeak.begin(cliente);

  dht1.begin();
}

void loop() {

  delay(2000);
  leerdht1();

  ThingSpeak.writeFields(channelID,WriteAPIKey);
  Serial.println("Datos enviados a ThingSpeak!");
  delay(14000);
}

void leerdht1() {
  
  float t1 = dht1.readTemperature();
  float h1 = dht1.readHumidity();

  while (isnan(t1) || isnan(h1)){
    Serial.println("Lectura fallida en el sensor DHT11, repitiendo lectura...");
    delay(2000);
    t1 = dht1.readTemperature();
    h1 = dht1.readHumidity();
  }

  Serial.print("Temperatura DHT11: ");
  Serial.print(t1);
  Serial.println(" ºC.");

  Serial.print("Humedad DHT11: ");
  Serial.print(h1);
  Serial.println(" %."); 

  Serial.println("-----------------------");

  ThingSpeak.setField (1,t1);
  ThingSpeak.setField (2,h1);
}
