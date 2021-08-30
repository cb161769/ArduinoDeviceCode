#include <SPI.h>
#include <Wire.h> // libreria para interconectarse con los componentes
#include <WiFi101.h> // libreria del wifi
#include <ArduinoBearSSL.h> // libreria Cliente SSL
#include <ArduinoECCX08.h> // libreria externa para el proyecto SSl
#include <ArduinoMqttClient.h> // cliente conexixon MQTT 
#include "variablesGlobales.h"
#include <Adafruit_ADS1X15.h>// libreria para utilizar el microcontrolador ADS1115
#include <ArduinoJson.h> // libreria para enviar JSON'S
#include <SD.h>
Adafruit_ADS1115 ads;
#include <Filters.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
RunningStatistics inputStats;
char ssid[] = netWork_SSD; // ssid de la red wifi
char pass[] = netWork_SSD_password; // contraseña del password de la red wi-fi
int status = WL_IDLE_STATUS; // status de la red wi-fi
const char* certificate = awsCertificate;  // certificado AWS IoT
const char broker[]  = brokerSececreto; // broker
const char* deviceReadings = DEVICE_READING;
char msg[512]; // cantidad del arreglo de mensajeria
WiFiClient wifiClient; // cliente Wifi Para las conexiones TCP.
BearSSLClient sslClient(wifiClient); // utilizado para SSL/TLS conexion, integrdo con ECC508
MqttClient    mqttClient(sslClient); // Cliente Mqtt
const float FACTOR = 50; //30A/1V
const float multiplier = 0.0625F;
unsigned long lastMilis = 0;
char deviceName[] = device_name; // nombre del dispositivo
char userName[] = user_Name;
const int chipSelect = CHIP_SELECT; // chip select from SD
const int sensorRelay1 = A3; // sensor CT1 conectado al rele #1
const int relayPin = RELAY_PIN1; //  pin del rele #1
double voltageCT1 = 0; // voltaje del ct 1 , conectado al rele#1
double vRMSCT1 = 0; // voltaje del ct 1 , conectado al rele#1
double ampsRMSCT1 = 0; // voltaje del ct 1 , conectado al rele#1
int mVperAmp = mvVerAmp;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
const char* LibName = libName;
float testFrequency = 60;                     // test signal frequency (Hz)
float windowLength = 20.0 / testFrequency;   // how long to average the signal, for statistist
int sensorValue = 0;
float intercept = 0; // PAra ser ajustado por motivo de calibracion
float slope = 0.0752; // to be adjusted based on calibration testing
float current_amps;
unsigned long previousMillis = 0;
unsigned long printPeriod = 1000;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  while (!Serial);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 46);
  tft.println(LibName);
  delay(2500);
  tft.fillScreen(ILI9341_WHITE);

  if (!ECCX08.begin()) {
    tft.println("no esta presente el serial EXCC08");
  }
  if (WiFi.status() == WL_NO_SHIELD) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi no conectado!");



    while (true);

  }
  if (WiFi.status() == WL_NO_SHIELD) {
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(1);
    tft.println("WiFi no presente!");
    while (true);
    //digitalWrite(redLED, LOW);
  }
  while ( status != WL_CONNECTED) {
    //digitalWrite(redLED,HIGH);
    tft.setTextColor(ILI9341_RED);
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    tft.println("Intentando conectarse a la red wi-fi");
    tft.print(ssid);
    delay(10000);
    // digitalWrite(redLED,LOW);
  }

  tft.setCursor(20, 46);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  Serial.println("Tarjeta SD inicializada");
  ads.setGain(GAIN_TWO);        // ±2.048V  1 bit = 0.0625mV
  ads.begin();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  pinMode(RELAY_PIN2, OUTPUT);
  digitalWrite(RELAY_PIN2, HIGH);
  pinMode(RELAY_PIN3, OUTPUT);
  digitalWrite(RELAY_PIN3, HIGH);
  Serial.println(F("Conectado a la red"));
  tft.println("Conectado a la red");
  long rssi = WiFi.RSSI();
  Serial.println("Señal" + rssi);
  tft.println("Señal" + rssi);
  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);

}

void loop() {

  // put your main code here, to run repeatedly:
  delay(10000);
  if (!mqttClient.connected()) {
    connectMQTT();

  }
  mqttClient.poll();
  if (millis() - lastMilis > 5000) {
    lastMilis = millis();
    publishMessage();
  }


}
unsigned long getTime() {
  return WiFi.getTime();
}
void connectMQTT() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(20, 10);
  tft.setTextSize(1);
  tft.println("Intentando conectarse al canal MQTT");

  tft.print(broker);


  while (!mqttClient.connect(broker, 8883)) {
    tft.setTextColor(ILI9341_RED);
    tft.println("INTENTO FALLIDO");

    delay(5000);
  }
  tft.setTextColor(ILI9341_GREEN);
  tft.println("Conectado al canal MQTT");

  // subscribe to a topic
  mqttClient.subscribe(generalTopic);
  mqttClient.subscribe(IOT_RULE_1);
  mqttClient.subscribe(IOT_RULE_1_OFF);
  mqttClient.subscribe(IOT_RULE_2);
  mqttClient.subscribe(IOT_RULE_2_OFF);
  mqttClient.subscribe(IOT_RULE_3);
  mqttClient.subscribe(IOT_RULE_3_OFF);
  mqttClient.subscribe(IOT_RULE_4);
  mqttClient.subscribe(IOT_RULE_4_OFF);
  mqttClient.subscribe(IOT_RULE_A);
  mqttClient.subscribe(IOT_RULE_A_OFF);
}
void publishMessage() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(20, 10);

  // digitalWrite(redYellow,LOW);
  float amps = getCurrent();
  float watts = 110.0 * amps;
  tft.print(watts, 3);
  tft.setCursor(204, 24);
  tft.println("w");
  tft.setTextSize(5);
  tft.setCursor(20, 60);
  tft.print(" ");
  tft.print(amps, 3);
  tft.setTextSize(3);
  tft.setCursor(204, 74);
  tft.println("A");
  tft.setTextSize(5);
  float CT1_Voltage = getVoltageCT1();
  float test = getVoltageTest();
  // CT
  float CT2_Voltage = getVoltageCT2();
  float CT3_Voltage = getVoltageCT3();
  // CT1
  float CT1_VRMS = (CT1_Voltage / 2.0) * 0.707;
  float CT1_APMSRMS = (CT1_VRMS * 1000) / mVperAmp;
  float CT1_watts = 110.0 * CT1_Voltage;
  String CT1_STATUS = "Conectado";
  //ct2
  float CT2_VRMS = (CT2_Voltage / 2.0) * 0.707;
  float CT2_APMSRMS = (CT2_VRMS * 1000) / mVperAmp;
  float CT2_watts = 110.0 * CT2_Voltage;
  //CT3
  float CT3_VRMS = (CT3_Voltage / 2.0) * 0.707;
  float CT3_APMSRMS = (CT3_VRMS * 1000) / mVperAmp;
  float CT3_watts = 110.0 * CT3_Voltage;

  tft.setTextSize(5);
  tft.setCursor(20, 110);
  tft.print(test, 3);
  tft.setTextSize(3);
  tft.setCursor(204, 124);
  tft.println("Test");
  tft.setTextSize(1);
  tft.println("Publicando mensaje a AWS");
  long rssi = WiFi.RSSI();
  unsigned long time_ = getTime();
  tft.setTextSize(1);
  tft.setCursor(20, 158);
  tft.print("Conexiones   ");
  tft.print(CT1_watts, 3);
  tft.println(" W");
  tft.setCursor(20, 178);
  tft.print("       ");
  tft.print(CT1_Voltage, 3);
  tft.println(" A");
  tft.setTextSize(1);
  tft.setCursor(20, 198);
  tft.print("WIFI ");

  tft.println("C");
  tft.println(" Nombre");
  tft.setCursor(20, 218);
  tft.print("       ");
  tft.print(rssi, 3);
  tft.println(" Señal");
  StaticJsonDocument<300> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("readings");
  stateObj["device_name"] = deviceName;
  stateObj["device_UserName"] = userName;
  stateObj["wifi_IP"] = WiFi.gatewayIP();
  stateObj["wifi_name"] = WiFi.SSID();
  stateObj["wifi_strength"] = WiFi.RSSI();
  stateObj["device_amps"] = amps;
  stateObj["device_watts"] = watts;
  // creacion de objectos en el arreglo de Conexiones
  JsonArray relays = jsonDoc.createNestedArray("Relays");
  JsonObject stateObj1 = relays.createNestedObject();
  stateObj1["Name"] = "Conexion 1";
  stateObj1["CT1_Amps"] = CT1_Voltage;
  stateObj1["CT1_Watts"] = CT1_watts;
  stateObj1["CT1_Status"] = CT1_STATUS;
  JsonObject stateObj2 = relays.createNestedObject();
  // poner CT2 Conexion
  stateObj2["Name"] = "Conexion 2";
  stateObj2["CT1_Amps"] = CT2_Voltage;
  stateObj2["CT1_Watts"] = CT2_watts;
  stateObj2["CT1_Status"] = CT1_STATUS;
  JsonObject stateObj3 = relays.createNestedObject();
  // poner CT3 Conexion
  stateObj3["Name"] = "Conexion 3";
  stateObj3["CT1_Amps"] = CT3_Voltage;
  stateObj3["CT1_Watts"] = CT3_watts;
  stateObj3["CT1_Status"] = CT1_STATUS;
  char jsonBuffer[300];
  unsigned long payloadSize = measureJson(jsonDoc); //serializeJson(jsonDoc, jsonBuffer);
  mqttClient.beginMessage(deviceReadings, payloadSize);
  serializeJson(jsonDoc, mqttClient);
  mqttClient.endMessage();

}
float getCurrent() {
  float Volt_diferencial;
  float current;
  float sum = 0;
  long times = millis();
  int counter = 0;
  while (millis() - times < 1000)
  {
    Volt_diferencial = ads.readADC_Differential_0_1() * multiplier;
    current = Volt_diferencial * FACTOR;
    current /= 1000.0;

    sum += sq(current);
    counter = counter + 1;
  }
  current = sqrt(sum / counter);
  return (current);


}
void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  if (mqttClient.messageTopic() == IOT_RULE_1) {
    //pinMode(relayPin, OUTPUT);
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("encendiendo la Conexion 1");
    digitalWrite(relayPin, HIGH);


  }
  if (mqttClient.messageTopic() == IOT_RULE_1_OFF) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("apagando la Conexion 1");
    // pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
  }
  if (mqttClient.messageTopic() == IOT_RULE_2) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("encendiendo la Conexion 2");
    digitalWrite(RELAY_PIN2, HIGH);
  }
  if (mqttClient.messageTopic() == IOT_RULE_2_OFF) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("apagando la Conexion 2");
    digitalWrite(RELAY_PIN2, LOW);
  }
  if (mqttClient.messageTopic() == IOT_RULE_3) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("encendiendo la Conexion 3");
    digitalWrite(RELAY_PIN3, HIGH);
  }
  if (mqttClient.messageTopic() == IOT_RULE_3_OFF) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("apagando la Conexion 3");
    digitalWrite(RELAY_PIN3, LOW);
  }
  if (mqttClient.messageTopic() == IOT_RULE_4) {

  }
  if (mqttClient.messageTopic() == IOT_RULE_4_OFF) {

  }
  if (mqttClient.messageTopic() == IOT_RULE_A) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("encendiendo todas las conexiones");
    digitalWrite(RELAY_PIN3, HIGH);
    digitalWrite(RELAY_PIN2, HIGH);
    digitalWrite(relayPin, HIGH);

  }
  if (mqttClient.messageTopic() == IOT_RULE_A_OFF) {
    tft.fillScreen(ILI9341_WHITE);
    tft.setTextSize(2);
    tft.println("apagando todas las conexiones");
    digitalWrite(RELAY_PIN3, LOW);
    digitalWrite(RELAY_PIN2, LOW);
    digitalWrite(relayPin, LOW);

  }

  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }

}
void setTFT() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
}
//TODO ; modify
float getVoltageCT1() {
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  uint32_t start_time = millis();
  while ((millis() - start_time) <= 100) {
    readValue = analogRead(sensorRelay1);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the minimum sensor value*/
      minValue = readValue;
    }

  }
  result = ((maxValue - minValue) * 5.0) / 1024.0;
  return result;
}
float getVoltageTest() {
  // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );

  while ( true ) {
    sensorValue = analogRead(SENSOR_1);  // read the analog in value:
    inputStats.input(sensorValue);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time

      // display current values to the screen

      // output sigma or variation values associated with the inputValue itsel

      // convert signal sigma value to current in amps
      current_amps = intercept + slope * inputStats.sigma();
      return current_amps;
    }
  }

}

//LECTURA DE VOLTAJE del ct2
float getVoltageCT2() {
  // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );

  while ( true ) {
    sensorValue = analogRead(SENSOR_2);  // read the analog in value:
    inputStats.input(sensorValue);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time

      // display current values to the screen

      // output sigma or variation values associated with the inputValue itsel

      // convert signal sigma value to current in amps
      current_amps = intercept + slope * inputStats.sigma();
      return current_amps;
    }
  }
}
//LECTURA DE VOLTAJE del ct3
float getVoltageCT3() {
  // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );

  while ( true ) {
    sensorValue = analogRead(SENSOR_3);  // read the analog in value:
    inputStats.input(sensorValue);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time

      // display current values to the screen

      // output sigma or variation values associated with the inputValue itsel

      // convert signal sigma value to current in amps
      current_amps = intercept + slope * inputStats.sigma();
      return current_amps;
    }
  }
}
#include <SPI.h>
#include <Wire.h> // libreria para interconectarse con los componentes
#include <WiFi101.h> // libreria del wifi
#include <ArduinoBearSSL.h> // libreria Cliente SSL
#include <ArduinoECCX08.h> // libreria externa para el proyecto SSl
#include <ArduinoMqttClient.h> // cliente conexixon MQTT 
#include "variablesGlobales.h"
#include <Adafruit_ADS1X15.h>// libreria para utilizar el microcontrolador ADS1115
#include <ArduinoJson.h> // libreria para enviar JSON'S
#include <SD.h>
Adafruit_ADS1115 ads;
RunningStatistics inputStats;
#include <Filters.h>
#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
char ssid[] = netWork_SSD; // ssid de la red wifi
char pass[] = netWork_SSD_password; // contraseña del password de la red wi-fi
int status = WL_IDLE_STATUS; // status de la red wi-fi
const char* certificate = awsCertificate;  // certificado AWS IoT
const char broker[]  = brokerSececreto; // broker
const char* deviceReadings = DEVICE_READING;
char msg[512]; // cantidad del arreglo de mensajeria
WiFiClient wifiClient; // cliente Wifi Para las conexiones TCP.
BearSSLClient sslClient(wifiClient); // utilizado para SSL/TLS conexion, integrdo con ECC508
MqttClient    mqttClient(sslClient); // Cliente Mqtt
const float FACTOR = 50; //30A/1V
const float multiplier = 0.0625F;
unsigned long lastMilis = 0;
char deviceName[] = device_name; // nombre del dispositivo
char userName[] = user_Name;
const int chipSelect = CHIP_SELECT; // chip select from SD
const int sensorRelay1 = A3; // sensor CT1 conectado al rele #1
const int relayPin = RELAY_PIN1; //  pin del rele #1
double voltageCT1 = 0; // voltaje del ct 1 , conectado al rele#1
double vRMSCT1 = 0; // voltaje del ct 1 , conectado al rele#1
double ampsRMSCT1 = 0; // voltaje del ct 1 , conectado al rele#1
int mVperAmp = mvVerAmp;
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
const char* LibName = libName;
float testFrequency = 60;                     // test signal frequency (Hz)
float windowLength = 20.0 / testFrequency;   // how long to average the signal, for statistist
int sensorValue = 0;
float intercept = -0.1129; // to be adjusted based on calibration testing
float slope = 0.0621; // to be adjusted based on calibration testing
float current_amps;
unsigned long previousMillis = 0;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
  while (!Serial);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(ILI9342_WHITE);
  tft.setTextColor(ILI9341_BLACK, ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 46);
  tft.println(LibName);
  delay(2500);
  tft.fillScreen(ILI9341_WHITE);

  if (!ECCX08.begin()) {
    tft.println("no esta presente el serial EXCC08");
  }
  if (WiFi.status() == WL_NO_SHIELD) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi no conectado!");



    while (true);

  }
  if (WiFi.status() == WL_NO_SHIELD) {
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(1);
    tft.println("WiFi no presente!");
    while (true);
    //digitalWrite(redLED, LOW);
  }
  while ( status != WL_CONNECTED) {
    //digitalWrite(redLED,HIGH);
    tft.setTextColor(ILI9341_RED);
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    tft.println("Intentando conectarse a la red wi-fi");
    tft.print(ssid);
    delay(10000);
    // digitalWrite(redLED,LOW);
  }

  tft.setCursor(20, 46);
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(2);
  Serial.println("Tarjeta SD inicializada");
  ads.setGain(GAIN_TWO);        // ±2.048V  1 bit = 0.0625mV
  ads.begin();
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, HIGH);
  Serial.println(F("Conectado a la red"));
  tft.println("Conectado a la red");
  long rssi = WiFi.RSSI();
  Serial.println("Señal" + rssi);
  tft.println("Señal" + rssi);
  ArduinoBearSSL.onGetTime(getTime);
  sslClient.setEccSlot(0, certificate);
  mqttClient.onMessage(onMessageReceived);

}

void loop() {

  // put your main code here, to run repeatedly:
  delay(10000);
  if (!mqttClient.connected()) {
    connectMQTT();

  }
  mqttClient.poll();
  if (millis() - lastMilis > 5000) {
    lastMilis = millis();
    publishMessage();
  }


}
unsigned long getTime() {
  return WiFi.getTime();
}
void connectMQTT() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setCursor(20, 10);
  tft.setTextSize(1);
  tft.println("Intentando conectarse al canal MQTT");

  tft.print(broker);


  while (!mqttClient.connect(broker, 8883)) {
    tft.setTextColor(ILI9341_RED);
    tft.println("INTENTO FALLIDO");

    delay(5000);
  }
  //  Serial.println();
  tft.setTextColor(ILI9341_GREEN);
  //  Serial.println("You're connected to the MQTT broker");
  tft.println("Conectado al canal MQTT");
  //  Serial.println();
  // digitalWrite(redGreen,HIGH);

  // subscribe to a topic
  mqttClient.subscribe("arn:aws:iot:us-west-2:170095147869:topic/$aws/rules/awsdynamodbcognitoapideviotRule174L17O1T8HZT");
  mqttClient.subscribe("/turnOnDeviceOne");
  mqttClient.subscribe("/turnOffDeviceOne");
  mqttClient.subscribe("/turnOnDeviceTwo");
  mqttClient.subscribe("/turnOffDeviceTwo");
  mqttClient.subscribe("/turnOnDeviceThree");
  mqttClient.subscribe("/turnOffDeviceThree");
  mqttClient.subscribe("/turnOnDeviceFour");
  mqttClient.subscribe("/turnOffDeviceFour");
  mqttClient.subscribe("/turnOnAllDevices");
  mqttClient.subscribe("/turnOffAllDevices");
}
void publishMessage() {
  tft.fillScreen(ILI9341_WHITE);
  tft.setTextSize(5);
  tft.setCursor(20, 10);

  // digitalWrite(redYellow,LOW);
  float amps = getCurrent();
  float watts = 110.0 * amps;
  tft.print(watts, 3);
  tft.setCursor(204, 24);
  tft.println("w");
  tft.setTextSize(5);
  tft.setCursor(20, 60);
  tft.print(" ");
  tft.print(amps, 3);
  tft.setTextSize(3);
  tft.setCursor(204, 74);
  tft.println("A");
  tft.setTextSize(5);
  float CT1_Voltage = getVoltageCT1();
  float test = getVoltageTest();
  float CT1_VRMS = (CT1_Voltage / 2.0) * 0.707;
  float CT1_APMSRMS = (CT1_VRMS * 1000) / mVperAmp;
  float CT1_watts = 110.0 * CT1_Voltage;
  String CT1_STATUS = "Conectado";
  tft.setTextSize(5);
  tft.setCursor(20, 110);
  tft.print(test, 3);
  tft.setTextSize(3);
  tft.setCursor(204, 124);
  tft.println("Test")
  tft.setTextSize(1);
  tft.println("Publicando mensaje a AWS");
  long rssi = WiFi.RSSI();
  unsigned long time_ = getTime();
  tft.setTextSize(1);
  tft.setCursor(20, 158);
  tft.print("Conexiones   ");
  tft.print(CT1_watts, 3);
  tft.println(" W");
  tft.setCursor(20, 178);
  tft.print("       ");
  tft.print(CT1_Voltage, 3);
  tft.println(" A");
  tft.setTextSize(1);
  tft.setCursor(20, 198);
  tft.print("WIFI ");

  tft.println("C");
  tft.println(" Nombre");
  tft.setCursor(20, 218);
  tft.print("       ");
  tft.print(rssi, 3);
  tft.println(" Señal");
  StaticJsonDocument<300> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("readings");
  stateObj["device_name"] = deviceName;
  stateObj["device_UserName"] = userName;
  stateObj["wifi_IP"] = WiFi.gatewayIP();
  stateObj["wifi_name"] = WiFi.SSID();
  stateObj["wifi_strength"] = WiFi.RSSI();
  stateObj["device_amps"] = amps;
  stateObj["device_watts"] = watts;
  // creacion de objectos en el arreglo de Conexiones
  JsonArray relays = jsonDoc.createNestedArray("Relays");
  JsonObject stateObj1 = relays.createNestedObject();
  stateObj1["Name"] = "Conexion 1";
  stateObj1["CT1_Amps"] = CT1_Voltage;
  stateObj1["CT1_Watts"] = CT1_watts;
  stateObj1["CT1_Status"] = CT1_STATUS;
  JsonObject stateObj2 = relays.createNestedObject();
  // poner CT2 Conexion
  stateObj2["Name"] = "Conexion 2";
  stateObj2["CT1_Amps"] = CT1_Voltage;
  stateObj2["CT1_Watts"] = CT1_watts;
  stateObj2["CT1_Status"] = CT1_STATUS;
  JsonObject stateObj3 = relays.createNestedObject();
  // poner CT3 Conexion
  stateObj3["Name"] = "Conexion 3";
  stateObj3["CT1_Amps"] = CT1_Voltage;
  stateObj3["CT1_Watts"] = CT1_watts;
  stateObj3["CT1_Status"] = CT1_STATUS;
  char jsonBuffer[300];
  unsigned long payloadSize = measureJson(jsonDoc); //serializeJson(jsonDoc, jsonBuffer);
  mqttClient.beginMessage(deviceReadings, payloadSize);
  serializeJson(jsonDoc, mqttClient);
  mqttClient.endMessage();

}
float getCurrent() {
  float Volt_diferencial;
  float current;
  float sum = 0;
  long times = millis();
  int counter = 0;
  while (millis() - times < 1000)
  {
    Volt_diferencial = ads.readADC_Differential_0_1() * multiplier;
    current = Volt_diferencial * FACTOR;
    current /= 1000.0;

    sum += sq(current);
    counter = counter + 1;
  }
  current = sqrt(sum / counter);
  return (current);


}
void onMessageReceived(int messageSize) {
  // we received a message, print out the topic and contents
  Serial.print("Received a message with topic '");
  Serial.print(mqttClient.messageTopic());
  if (mqttClient.messageTopic() == "/turnOnDeviceOne") {
    //pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH);


  }
  if (mqttClient.messageTopic() == "/turnOffDeviceOne") {

    // pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, LOW);
  }
  if (mqttClient.messageTopic() == "/turnOnDeviceTwo") {

  }
  if (mqttClient.messageTopic() == "/turnOffDeviceTwo") {

  }
  if (mqttClient.messageTopic() == "/turnOnDeviceThree") {

  }
  if (mqttClient.messageTopic() == "/turnOffDeviceThree") {

  }
  if (mqttClient.messageTopic() == "/turnOnDeviceFour") {

  }
  if (mqttClient.messageTopic() == "/turnOffDeviceFour") {

  }
  if (mqttClient.messageTopic() == "/turnOnAllDevices") {

  }
  if (mqttClient.messageTopic() == "/turnOffAllDevices") {

  }
  Serial.print("', length ");
  Serial.print(messageSize);
  Serial.println(" bytes:");

  // use the Stream interface to print the contents
  while (mqttClient.available()) {
    Serial.print((char)mqttClient.read());
  }

}
void setTFT() {
  tft.fillScreen(ILI9341_BLACK);
  unsigned long start = micros();
  tft.setCursor(0, 0);
  tft.setTextColor(ILI9341_WHITE);
}
//TODO ; modify
float getVoltageCT1() {
  float result;
  int readValue;             //value read from the sensor
  int maxValue = 0;          // store max value here
  int minValue = 1024;          // store min value here
  uint32_t start_time = millis();
  while ((millis() - start_time) <= 100) {
    readValue = analogRead(sensorRelay1);
    // see if you have a new maxValue
    if (readValue > maxValue)
    {
      /*record the maximum sensor value*/
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      /*record the minimum sensor value*/
      minValue = readValue;
    }

  }
  result = ((maxValue - minValue) * 5.0) / 1024.0;
  return result;
}
float getVoltageTest() {
  // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );

  while ( true ) {
    sensorValue = analogRead(SENSOR_1);  // read the analog in value:
    inputStats.input(sensorValue);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time

      // display current values to the screen

      // output sigma or variation values associated with the inputValue itsel

      // convert signal sigma value to current in amps
      current_amps = intercept + slope * inputStats.sigma();
      return current_amps;
    }
  }

}

//LECTURA DE VOLTAJE del ct2
float getVoltageCT2() {
    // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );

  while ( true ) {
    sensorValue = analogRead(SENSOR_2);  // read the analog in value:
    inputStats.input(sensorValue);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time

      // display current values to the screen

      // output sigma or variation values associated with the inputValue itsel

      // convert signal sigma value to current in amps
      current_amps = intercept + slope * inputStats.sigma();
      return current_amps;
    }
  }
  }
//LECTURA DE VOLTAJE del ct3
float getVoltageCT3() {
    // create statistics to look at the raw test signal
  inputStats.setWindowSecs( windowLength );

  while ( true ) {
    sensorValue = analogRead(SENSOR_3);  // read the analog in value:
    inputStats.input(sensorValue);  // log to Stats function

    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
      previousMillis = millis();   // update time

      // display current values to the screen

      // output sigma or variation values associated with the inputValue itsel

      // convert signal sigma value to current in amps
      current_amps = intercept + slope * inputStats.sigma();
      return current_amps;
    }
  }
  }
