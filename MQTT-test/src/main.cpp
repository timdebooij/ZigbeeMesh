#include <Arduino.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
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
int state = 3;
void callback(char*, byte*, unsigned int);
void action(std::string);
void setStopLight(int);

// Timer
int endtime;
void startTimer(int);
void checkTimerSL();
bool timerOn = false;

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
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(ORANGE_PIN, LOW);
      digitalWrite(GREEN_PIN, LOW);
      break;
    case 2:
      digitalWrite(RED_PIN, LOW);
      digitalWrite(ORANGE_PIN, HIGH);
      digitalWrite(GREEN_PIN, LOW);
      timerOn = true;
      state = state - 1;
      startTimer(2000);
      
      break;
    case 3:
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




