/*
 * Notes: This code is able to send commands to the robot over NRF
 * the commands are enterd as there numarical value into the arduino 
 * serial terminal. 
 * 
 */

#include <SPI.h>
#include "printf.h"
#include "RF24.h"

// These files need to have thier locations updated before compile to match where you placed your files.
#include "C:/Users/pooki/Desktop/Tekbots/TekMow/github/TekMow/tekmow.h"

#define WD 5000

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
RF24 radio(9,10);

union {
   byte array[16];
   float Num[4];
} floatUnion;

unsigned long heartBeat;
float currentCoord[4];
byte payload[30];
byte Buffer[32];
byte addresses[][6] = {"1Node","2Node"};

void setup() {
  Serial.begin(115200);
  
  // initialize the transceiver on the SPI bus
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW);
  
  radio.openWritingPipe(addresses[1]);
  radio.openReadingPipe(1,addresses[0]);

  radio.startListening();
}

void loop() {
  /**********************|| Sending Command ||**********************/
  if( Serial.available()){
    uint8_t command = Serial.parseInt();
    Serial.println(command);

    Buffer[0] = command;

    switch (command){
      case FORWARD:
      case BACKWARD:
      case LEFT:
      case RIGHT:
      case STOP:
        Buffer[1] = 0;
        break;
      case SET_COORD:
        Buffer[1] = 16;
        getCoord(floatUnion.Num);
        fillBuff(16,2,floatUnion.array);
        break;
      default:
        Serial.println("invalid command");
        break;
    }

    sendBuffer();
    
  }/**********************|| Receving Radio ||**********************/
  
   //this section of code is untested
   
  if(radio.available()){
    uint8_t command,size;
    radio.read( &command, 1 );           // Get the command
    radio.read( &size, 1 );              // Get the size
    radio.read( &payload, size );

    switch (command){
      case GPS_RESPONCE:
        DecodePayload(size, payload);
        currentCoord[0] = floatUnion.Num[0];
        currentCoord[1] = floatUnion.Num[1];
        currentCoord[2] = floatUnion.Num[2];
        currentCoord[3] = floatUnion.Num[3];
        break;
      default:
        Serial.println("Invalid Responce");
        break;
    }
    
    if(command == GPS_RESPONCE){
      
    }else{
      
    }
  }
  /**********************|| Sending Heart Beat ||**********************/
  if(millis() > (heartBeat + WD)){
    Buffer[0] = HEART_BEAT;
    Buffer[1] = 1;

    Serial.println("Sending Heart Beat");
    sendBuffer();
    heartBeat = millis();
  }
}

bool sendBuffer(){
  radio.stopListening();
  
  if(radio.write( &Buffer, Buffer[1]+2) ){
    Serial.println("Com sent");
    heartBeat = millis();
  }else{
    Serial.println("no com ACK");
  }
  
  radio.startListening();
}

void fillBuff(int size, int index, byte *input){
  for (int i = 0; i < size; i++) {
      Buffer[index+i] = input[i];      
  }
}

void DecodePayload(int size, byte *input){
  for (int i = 0; i < size; i++) {
      floatUnion.array[i] = input[i];      
  }
}

void getCoord(float* coord){
  Serial.println("Type your max lat");
  coord[0] = get_float();
  Serial.println("Type your max lon");
  coord[1] = get_float();
  Serial.println("Type your min lat");
  coord[2] = get_float();
  Serial.println("Type your min lon");
  coord[3] = get_float();
}

float get_float(){
  bool good = false;
  String s = "";
  float f = 0;
  do{//go through this loop while the input is not made of digits exclusively
    good = true;
    
    while(Serial.available() == 0){}//wait for user input
    s = Serial.readString();
    Serial.println(s);
    Serial.println("printed string");
    //Serial.println(s.length());
    // Serial.println("Printed string length");
    for(int i=0; i<s.length()-1;i++){//loop through string
      char c = s[i];//check if each character is an integer
      if((c < 48 || c > 57) && c!=46 && c!=45){
        good = false;//set condition to do loop again
        Serial.println("Invalid input");
      }
    } 
  }while(good == false);
   
   char carray[s.length() + 1]; //determine size of the array
   s.toCharArray(carray, sizeof(carray)); //put readStringinto an array
   f = atof(carray); //convert the array into a float

  Serial.println(f);
  return f;//return valid integer value
}
