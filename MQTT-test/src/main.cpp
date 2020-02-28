#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <chrono>
#include <iostream>
#include <ctime>
#include <string>
#include <sstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>

#include "fsm.h"
#include "pubsubclient.h"



enum fsm_states {
	S_RED,
	S_GREEN,
	S_ORANGE,
	S_ERROR,
	NUM_STATES
};

enum alphabet {
	RED,
	GREEN,
	ORANGE,
	ERROR,
	TIMER,
	NUM_SYMBOLS
};

static constexpr int RED_PIN    = D4;
static constexpr int ORANGE_PIN = D2;
static constexpr int GREEN_PIN  = D3;

// WiFi
const char* ssid = "dlink";
const char* password =  "";

WiFiClient espClient;
smartlight::PubSubClient client(espClient);

// MQTT
const char* mqttServer = "hairdresser.cloudmqtt.com";
const int mqttPort = 17636;
const char* mqttUser = "tcmoqsbg";
const char* mqttPassword = "bCixl5gQUDiJ";
void callback(char*, byte*, unsigned int);
void action(std::string);
int state = 3;
void checkTimerSL();
void setStopLight(int);
bool timerOn = false;

// DHP Sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define DHTPIN 5     // Digital pin connected to the DHT sensor
DHT dht(DHTPIN, DHTTYPE);
void checkTimer2();

template <typename T>
  std::string to_string(T value)
{
	std::ostringstream os ;
	os << value ;
	return os.str() ;
}

// Timer
int endtime;
void startTimer(int);
void checkTimer();


static smartlight::FSM<fsm_states, alphabet, NUM_STATES, NUM_SYMBOLS> fsm;


void setup() {
 
  Serial.begin(9600);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
 
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
 
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
 
      Serial.println("connected");  
      //client.publish("esp/test", "Hello from ESP8266");
      Serial.println("send data");
    } else {
 
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
 
    }
  }
 
  
  client.subscribe("esp/test");
 pinMode(RED_PIN, OUTPUT);
	pinMode(ORANGE_PIN, OUTPUT);
	pinMode(GREEN_PIN, OUTPUT);

	digitalWrite(RED_PIN, LOW);
	digitalWrite(GREEN_PIN, LOW);
	digitalWrite(ORANGE_PIN, LOW);
  Serial.println("configuration complete");
  dht.begin();
  startTimer(2000);
  Serial.println("timer started");
}
 
void callback(char* topic, byte* payload, unsigned int length) {
  std::string message;
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
 
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    message.push_back((char)payload[i]);
  }
  action(message);
 
 
  Serial.println();
  Serial.println("-----------------------");
}
 
void loop() {
  client.loop();
  if(timerOn){
    checkTimerSL();
  }


}

void action(std::string message){
 if(message == "RED-ON"){
    digitalWrite(RED_PIN, HIGH);
  }else if(message == "RED-OFF"){
digitalWrite(RED_PIN, LOW);
  }else if(message == "ORANGE-ON"){
    digitalWrite(ORANGE_PIN, HIGH);
  }else if(message == "ORANGE-OFF"){
    digitalWrite(ORANGE_PIN, LOW);
  }else if(message == "GREEN-ON"){
    digitalWrite(GREEN_PIN, HIGH);
  }else if(message == "GREEN-OFF"){
    digitalWrite(GREEN_PIN, LOW);
  }
  if(message == "6"){
    setStopLight(3);
  }
}

void startTimer(int time){
  timerOn = true;
  Serial.println("timer started");
  endtime = millis() + time;
  
}

void setStopLight(int stateNew){
  state = stateNew;
  Serial.println("new state");
  switch(stateNew){
    case 1:
    //Serial.println("reached case 1");
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(ORANGE_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      break;
    case 2:
    //Serial.println("reached case 2");
      digitalWrite(RED_PIN, LOW);
      digitalWrite(ORANGE_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      timerOn = true;
      state = state - 1;
      startTimer(2000);
      
      break;
    case 3:
    //Serial.println("reached case 3");
      digitalWrite(RED_PIN, LOW);
      digitalWrite(ORANGE_PIN, LOW);
      digitalWrite(GREEN_PIN, HIGH);
      timerOn = true;
      state = state - 1;
      startTimer(2000);
      
      
      break;
  }
}

void checkTimerSL(){
  if(millis()>=endtime){
    timerOn = false;
    setStopLight(state);
    
  }
}

void checkTimer() {
  if(millis()>=endtime){
    int temp = round(rand()%30);
    int hum = round(rand()%100);
    std::string json = "{\"location\":\"ESP-Haagdijk-Room-Tim\",\"temp\":" + to_string(temp) + ",\"humidity\":" + to_string(hum) + ",\"energy\":35489,\"version\":1}";//'{"location": "ESP-Haagdijk-Room-Tim"},{"time", "08-02-2020"},{"temp",' + to_string(temp) + '},{"humidity", ' + to_string(hum) + '},{"energy", 26991},{"version", 1}}';
    
   client.publish("esp/test", json.c_str());
   startTimer(10000);
  }
}

void checkTimer2() {
  if(millis()>=endtime){
// Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    //client.publish("esp/test", "failed to read data");
  }
  //else{
    float hic = dht.computeHeatIndex(t, h, false);
    std::string json = "Temperature: " + to_string(hic);
    
//Serial.print(F("Humidity: "));
  //Serial.print(h);
  //Serial.print(F("%  Temperature: "));
  //Serial.print(t);
  //Serial.print(F("°C "));

    //client.publish("esp/test", json.c_str());
  //}
    startTimer(2000);
  }
 
}

