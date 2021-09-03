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

#include "G:/My Drive/PC Transfer/Desktop/Tekbots/TekMow/TekMow/tekmow.h"
#include "G:/My Drive/PC Transfer/Desktop/Tekbots/TekMow/TekMow/Joystick.c"
#include "G:/My Drive/PC Transfer/Desktop/Tekbots/TekMow/TekMow/Comm.c"

#define WD 4000

/* Hardware configuration: Set up nRF24L01 radio on SPI bus plus pins 9 & 10 */
Comm TekMow_Comm; // Creates communication object
Joystick Joy(A1, A0, 10, 1500);

unsigned long heartBeatTimer;
float currentCoord[4];

uint8_t processSerialCommand(uint8_t incoming){
  switch (incoming){
    case 'w':
      return FORWARD;
    case 'a':
      return LEFT;
   case 'd':
      return RIGHT;
   case 's':
      return BACKWARD;
   case ' ':
      return STOP;
   case 'e':
      return ECHO;
   default:
      return incoming; 
  }
}

bool SendCommand(){
  //Handle Serial input
  uint8_t command = processSerialCommand(Serial.read());

  //Fill payload
  switch (command){
    case FORWARD:
    case BACKWARD:
    case LEFT:
    case RIGHT:
    case STOP:
      TekMow_Comm.setSize(0);
      break;
    case SET_COORD:
      break;
    case HEART_BEAT:
      TekMow_Comm.setSize(0);
      break;
    case ECHO:
      TekMow_Comm.setSize(0);
      break;
    default:
      Serial.println("invalid command");
      return 0;
  }

  //Set Command
  TekMow_Comm.setCommand(command);
    
  //Send Payload
  TekMow_Comm.sendPayload();

  return 1;
}

void getCommand(){
  //Grab Info and Payload packets
  TekMow_Comm.pullPayload();

  //Prosses based on Command
  switch (TekMow_Comm.getCommand()){
    case GPS_RESPONSE:
      break;
    case ECHO_RESPONSE:
      Payload Temp = TekMow_Comm.getPayload();
      for(int i = 0; i < TekMow_Comm.getSize(); i++){
        Serial.print(Temp.Str[i]);
      }
      Serial.println(' ');
      break;
    default:
      //Serial.println("Invalid Responce");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println(F("TekMow Transmitter Starting"));
  Serial.println(F("Use WASD to move, SPACE to stop."));
  
  TekMow_Comm.initRadio(9,10,1);
  Joy.calibration();
}

void loop() {
  /**********************|| Sending Command ||**********************/
  if(Serial.available()){
    if(SendCommand()){
      heartBeatTimer = millis();
    }
  }

  /**********************|| Receving Radio ||**********************/
  
  if(TekMow_Comm.available()){
    getCommand();
  }

  /**********************|| Joystick Input ||**********************/

  if(Joy.alive() && true){
    Joy.disp();
    TekMow_Comm.sendJoystick(Joy.getX(), Joy.getY());
    TekMow_Comm.sendPayload();
  }

  /**********************|| Sending Heart Beat ||**********************/
  if(millis() > (heartBeatTimer + WD)){
    TekMow_Comm.setCommand(HEART_BEAT);
    TekMow_Comm.setSize(0);
    if(TekMow_Comm.sendPayload()){
      heartBeatTimer = millis();
    }
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
