#include <Arduino.h>

int buttonState = 0; 
int buttonState2 = 0;
bool send1 = false;
bool send2 = false;

void setup() {
  // put your setup code here, to run once:
   Serial.begin(9600);
  Serial.println("Started");
  pinMode(2, INPUT);
  pinMode(3, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(2);
  buttonState2 = digitalRead(3);
  if(buttonState == HIGH){
    if(!send1){
      Serial.print(6);
      send1 = true;
    }
  }
  if(buttonState == LOW){
    send1 = false;
  }
  
}