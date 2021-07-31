
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

#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"
#define  BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define TFT_DC 7
#define TFT_CS 6
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
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while (!Serial);
  tft.begin();
    // read diagnostics (optional but can help debug problems)
  uint8_t x = tft.readcommand8(ILI9341_RDMODE);
  Serial.print("Display Power Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDMADCTL);
  Serial.print("MADCTL Mode: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDPIXFMT);
  Serial.print("Pixel Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDIMGFMT);
  Serial.print("Image Format: 0x"); Serial.println(x, HEX);
  x = tft.readcommand8(ILI9341_RDSELFDIAG);
  Serial.print("Self Diagnostic: 0x"); Serial.println(x, HEX); 
  
  if (!ECCX08.begin()) {
    Serial.println("no esta presente el serial EXCC08");
  }
  setTFT();
  if (WiFi.status() == WL_NO_SHIELD) {
    tft.setTextSize(1);
    tft.setTextColor(ILI9341_RED);
    tft.println("WiFi no conectado!");
    Serial.println("WiFi no conectado");
    // digitalWrite(redLED,HIGH);
    // don't continue:
    //display.clearDisplay();
    // display.setCursor(0,0);             // Start at top-left corner
    //display.println(F("WiFi no conectado"));
    // display.display();


    while (true);
    // digitalWrite(redLED,LOW);
  }
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi no conectado");
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(1);
    tft.println("WiFi no presente!");
    while (true);
    //digitalWrite(redLED, LOW);
  }
  while ( status != WL_CONNECTED) {
    //digitalWrite(redLED,HIGH);
    tft.setTextColor(ILI9341_RED);
    Serial.print("Intentando conectarse a la red wi-fi ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);
    // esperar 1- segundos para la conexion

    tft.println("Intentando conectarse a la red wi-fi");
    tft.println(ssid);
    delay(10000);
    // digitalWrite(redLED,LOW);
  }
//  Serial.print("Inicializar  Tarjeta SD...");
//  if (!SD.begin(chipSelect)) {
//    Serial.println("Tarjeta SD no Presente");
//    // don't do anything more:
//    while (1);
//  }
  tft.setTextColor(ILI9341_GREEN);
  tft.setTextSize(5);
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
    // Cliente MQTT desconectado, para conectarse
    connectMQTT();

  }
  mqttClient.poll();
  if (millis() - lastMilis > 5000) {
    lastMilis = millis();

    publishMessage();
  }

}
unsigned long getTime() {
  // get the current time from the WiFi module
  return WiFi.getTime();
}
void connectMQTT() {
  tft.setTextSize(1);
  tft.println("Intentando conectarse al canal MQTT");
  Serial.print("Attempting to MQTT broker: ");
  Serial.print(broker);
  Serial.println(" ");

  while (!mqttClient.connect(broker, 8883)) {
    // digitalWrite(redLED,HIGH);
    // digitalWrite(redGreen,LOW);
    // digitalWrite(redLED,HIGH);
    // failed, retry
    Serial.print(".");
    tft.println("INTENTO FALLIDO");
    tft.setTextColor(ILI9341_RED);
    delay(5000);
    // digitalWrite(redLED,LOW);
  }
  Serial.println();
  tft.setTextColor(ILI9341_GREEN);
  Serial.println("You're connected to the MQTT broker");
  tft.println("Conectado al canal MQTT");
  Serial.println();
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
  // digitalWrite(redYellow,LOW);
  float amps = getCurrent();
  float watts = 110.0 * amps;
  float CT1_Voltage = getVoltageCT1();
  float CT1_VRMS = (CT1_Voltage / 2.0) * 0.707;
  float CT1_APMSRMS = (CT1_VRMS * 1000) / mVperAmp;

  // float CT1_watts = 110.0 * CT1_APMSRMS;
  float CT1_watts = 110.0 * CT1_Voltage;
  String CT1_STATUS = "Conectado";

  // watts = amps * homeVoltaje;
  Serial.println("Publicando mensaje a AWS");
   tft.println("Publicando mensaje a AWS");
  long rssi = WiFi.RSSI();
  // emon.calcVI(20,2000);
  // float realPower = emon.realPower;
  // float apparentPower   = emon.apparentPower;
  unsigned long time_ = getTime();

  StaticJsonDocument<300> jsonDoc;
  JsonObject stateObj = jsonDoc.createNestedObject("readings");
  // amps = emon.calcIrms(1480);
  // watts = amps * homeVoltaje;
  stateObj["device_name"] = deviceName;
  stateObj["device_UserName"] = userName;
  stateObj["wifi_IP"] = WiFi.gatewayIP();
  stateObj["wifi_name"] = WiFi.SSID();
  stateObj["wifi_strength"] = WiFi.RSSI();
  stateObj["device_amps"] = amps;
  stateObj["device_watts"] = watts;
  JsonArray relays = jsonDoc.createNestedArray("Relays");
  JsonObject stateObj1 = relays.createNestedObject();

  stateObj1["Name"] = "Conexion 1";
  stateObj1["CT1_Amps"] = CT1_Voltage;
  stateObj1["CT1_Watts"] = CT1_watts;
  stateObj1["CT1_Status"] = CT1_STATUS;
  //    JsonObject stateObj3 = jsonDoc.createNestedObject("CT2");
  //    stateObj3["CT2_status"] = CT1_STATUS;
  char jsonBuffer[300];
  unsigned long payloadSize = measureJson(jsonDoc); //serializeJson(jsonDoc, jsonBuffer);

  // Serial.println("error al registrar los logs");
  // Serial.println(jsonBuffer);



  // sprintf(msg,"{'lecturas':['senal_wiFi':%f, 'dispositivoAmperios':%fA,'dispositivoWatts:%fW']}",rssi,amps,watts);

  mqttClient.beginMessage(deviceReadings, payloadSize);
  serializeJson(jsonDoc, mqttClient);
  //  mqttClient.print(serializeJson(jsonDoc, jsonBuffer));

  mqttClient.endMessage();
//  File dataFile = SD.open("datalog.txt", FILE_WRITE);
//  if (dataFile) {
//    Serial.println("LOGGEANDO LECTURAS DEL DISPOSITIVO");
//    serializeJson(jsonDoc, dataFile);
//    // dataFile.println(message);
//    dataFile.close();
//    // print to the serial port too:
//    // Serial.println(message);
//  }
//  //LogDeviceReadings(jsonDoc);
//
//  // Serial.println(jsonBuffer);

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
  Serial.print(current);
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