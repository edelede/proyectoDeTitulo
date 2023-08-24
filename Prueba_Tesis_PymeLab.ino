//Bibliotecas
#include <ArduinoJson.h>
#include <ArduinoJson.hpp>
#include <max6675.h>
#include <Wire.h>
#include <RTClib.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "FS.h"
#include "SD.h"
#include <SPI.h>
////////////

//Definiciones
#define TIEMPO_ENTRE_LECTURAS    10000 //ms
#define pin_SO  14 //D7
#define pin_CLK 27 //D6

float pin_Temp[6];

MAX6675 termopar(pin_CLK, pin_Temp[6], pin_SO);
RTC_DS3231 rtc;

String dataMessage;

const char* ssid = "Cruz 74";                 // Your personal network SSID
const char* wifi_password = "s9m11d9$"; // Your personal network password
const char* mqtt_server = "10.3.0.185";

//const char* ssid = "jaja";                 // Your personal network SSID
//const char* wifi_password = "contra123123"; // Your personal network password
//const char* mqtt_server = "192.168.77.195";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
/////////////

void setup_wifi() {

  delay(100);
  // Comenzamos conectandonos a Wifi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {

}

void reconnect() {
  // en Bucle hasta reconectarnos
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Crea una ID de cliente aleatoria
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Se intenta conectar
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Una vez conectado, se anuncia
      client.publish("outTopic", "hello world");
      // y se resuscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
        
      delay(5000);
    }
  }
}


// Inicialización de SD
void initSDCard(){
   if (!SD.begin()) {
    Serial.println("Fallo al montar SD");
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE){
    Serial.println("No hay SD conectada");
    return;
  }
  Serial.print("Tipo de tarjeta SD: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("Desconocido");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Tamaño de la SD: %lluMB\n", cardSize);
}

// Escribe en la tarjeta SD
void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Escribiendo archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file) {
    Serial.println("Fallo al abrir el archivo para escribir");
    return;
  }
  if(file.print(message)) {
    Serial.println("Fallo al escribir");
  } else {
    Serial.println("Escritura fallida");
  }
  file.close();
}

// Anexa los datos en la tarjeta SD
void appendFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Anexando al archivo: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file) {
    Serial.println("Fallo al abrir el archivo para anexar");
    return;
  }
  if(file.print(message)) {
    Serial.println("Mensaje anexado");
  } else {
    Serial.println("Anexo fallido");
  }
  file.close();
}


void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  initSDCard();
  File file = SD.open("/InauguraciónPymelab.txt");
  if(!file) {
    Serial.println("No existe el archivo");
    Serial.println("Creando archivo...");
    writeFile(SD, "/Piso1-Sur-Oriente.txt", "Cim-Pymelab \r\n");
  }
  else {
    Serial.println("Archivo existente");  
  }
  file.close();

  if (! rtc.begin()) {
 Serial.println("No hay un módulo RTC");
 while (1);
 }

  pinMode(26, OUTPUT); //A0/D1
  pinMode(25, OUTPUT); //A1/D2
  pinMode(17, OUTPUT); //A2/D3
  pinMode(16, OUTPUT); //A3/D4
  pinMode(12, INPUT); 
  
}

void loop() {
  
  DateTime now = rtc.now();
       
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
  StaticJsonDocument<32> doc;
  char output[55];

  long actual = millis();
  if (actual - lastMsg > 5000) {
    lastMsg = actual;
    
    mux (0,0,0,0);
    delay(500);
    pin_Temp[0] = termopar.readCelsius();
    doc["t"] = pin_Temp[0];
    doc["id"] = "-60";
    Serial.println("Leer");

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/pymelab/piso1/surponiente/temperatura", output);
    Serial.println("Enviado");

    mux (1,0,0,0);
    delay(500);
    pin_Temp[1] = termopar.readCelsius();
    doc["t"] = pin_Temp[1];
    doc["id"] = "-30";
    Serial.println("Leer");

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/pymelab/piso1/surponiente/temperatura", output);
    Serial.println("Enviado");

    mux (0,1,0,0);
    delay(500);
    pin_Temp[2] = termopar.readCelsius();
    doc["t"] = pin_Temp[2];
    doc["id"] = "0";
    Serial.println("Leer");

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/pymelab/piso1/surponiente/temperatura", output);
    Serial.println("Enviado");

    mux (1,1,0,0);
    delay(500);
    pin_Temp[3] = termopar.readCelsius();
    doc["t"] = pin_Temp[3];
    doc["id"] = "30";
    Serial.println("Leer");

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/pymelab/piso1/surponiente/temperatura", output);
    Serial.println("Enviado");

    mux (0,0,1,0);
    delay(500);
    pin_Temp[4] = termopar.readCelsius();
    doc["t"] = pin_Temp[4];
    doc["id"] = "60";
    Serial.println("Leer");

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/pymelab/piso1/surponiente/temperatura", output);
    Serial.println("Enviado");

    mux (1,0,1,0);
    delay(500);
    pin_Temp[5] = termopar.readCelsius();
    doc["t"] = pin_Temp[5];
    doc["id"] = "125";
    Serial.println("Leer");

    serializeJson(doc, output);
    Serial.println(output);
    client.publish("/pymelab/piso1/surponiente/temperatura", output);
    Serial.println("Enviado");

  }

  dataMessage = "Piso 1 Sur-Oriente \nFecha: " + String(now.day()) + "/" + String(now.month()) + "/" + String(now.year()) + "  " + "Hora: " + String(now.hour()) + ":" + String(now.minute()) + ":" + String(now.second()) + "\n" + 
   "Termocupla -60: " + String(pin_Temp[0]) + "°C" + "\n" + "Termocupla -30: " + String(pin_Temp[1]) + "°C" + "\n" + "Termocupla 0: " + String(pin_Temp[2]) + "°C" + "\n" +
   "Termocupla -30: " + String(pin_Temp[3]) + "°C" + "\n" + "Termocupla 60: " + String(pin_Temp[4]) + "°C" + "\n" + "Termocupla 125: " + String(pin_Temp[5]) + "°C" + "\n" + "\r\n";
    Serial.print("Guardando datos: ");
    Serial.println(dataMessage);

    appendFile(SD, "/Piso1-Sur-Oriente.txt", dataMessage.c_str());

    delay(TIEMPO_ENTRE_LECTURAS);

}

void mux (int state1,int state2,int state3,int state4) {

    digitalWrite(26, state1); //D6
    digitalWrite(25, state2); //D5
    digitalWrite(17, state3); //D4
    digitalWrite(16, state4); //D3

digitalRead(12);

}
