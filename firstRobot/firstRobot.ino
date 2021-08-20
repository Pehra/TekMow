/*
 * Notes: currnetly this code can read commands from the controller. 
 * The drive funtion was changed to non blocking and stoped working, 
 * some fixes were made but they are untested. The decoding of NRF 
 * data was moved to the function GetCommand() and a switch function 
 * is now used instead of if statements. these changes dont have 
 * compiler errors but arnt tested.
 * 
 * next steps are to impliment the Comm and Controller class instead 
 * of current layout. I think making a error, state, and config struct 
 * and/or class may also be helpfull.
 * 
 * 
 */
 
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <TinyGPS++.h>

// These files need to have thier locations updated before compile to match where you placed your files.
#include "C:/Users/pooki/Desktop/Tekbots/TekMow/github/TekMow/tekmow.h"

#define L_EN        5
#define L_DIR       4
#define R_EN        7
#define R_DIR       6
#define MOVE_TIME   1000

static const uint32_t GPSBaud = 9600;
byte addresses[][6] = {"1Node","2Node"};

/* Hardware configuration: 
 *  Set up nRF24L01 radio on SPI bus plus pins 9 & 10 
 *  Set Up GPS on RX1/TX1 
 */
RF24 radio(9,10);
TinyGPSPlus gps;

//used to encode/ decode floats to byte array
union {
  byte array[16];
  float Num[4];
} floatUnion;
  
byte payload[30];
float box[4];
uint8_t command, size;

//State and timer variables
unsigned long driveTime;
volatile uint8_t drivePS, driveNS;

void setup() {
  Serial.begin(115200);
  
  Serial1.begin(GPSBaud);
  
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1) {} // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW);
  
  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1,addresses[1]);

  radio.startListening();

  pinMode(L_EN, OUTPUT);
  pinMode(L_DIR, OUTPUT);
  pinMode(R_EN, OUTPUT);
  pinMode(R_DIR, OUTPUT);
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH);          
  digitalWrite(L_EN, HIGH); 
  digitalWrite(R_EN, HIGH); 

  drivePS = driveNS = STOP;
  driveTime = millis();
}

/*
 * Notes: this section of code will only be used to manage 
 * state logic and watchdog timers
 */
void loop() {
  
  /**********************|| Receving Command ||**********************/
  if( radio.available()){
    GetCommand();
  }
  
  /**********************|| Control Cycle ||**********************/
  if (driveNS != drivePS){
    Drive(driveNS);
    drivePS = driveNS;
  }
  
  if(millis() > (driveTime + MOVE_TIME)){
    Stop();
    drivePS = driveNS = STOP;
    Serial.println("Drive WD Timer Hit!");
  }
  /**********************|| Read Sensors ||**********************/

  /**********************|| Send Data ||**********************/
  
}

/*
 * Definition: this function is used to pull and decode the packet sent over NRF
 * 
 * Assumptions: the input buffer for the NRF has data in it 
 * and the data is formatted as shown below
 * 
 *        _________________________________________________
 *       |  Command  | Payload Size |       Payload        |
 *       |-------------------------------------------------|
 *       |  1 Byte   |    1 Byte    |      0-30 Bytes      |
 *       |-------------------------------------------------|
 *       
 * Output: state logic is set acording to the input command, Config data is set
 */
void GetCommand(){
  radio.read( &command, 1 );           // Get the command
  radio.read( &size, 1 );              // Get the size
  if(size > 0){
    radio.read( &payload, size );
  }

  switch (command){
    case FORWARD:
    case BACKWARD:
    case LEFT:
    case RIGHT:
    case STOP:
      driveNS = command;
      driveTime = millis();
      break;
    case SET_COORD:
      DecodePayload(size, payload);
      box[0] = floatUnion.Num[0];
      box[1] = floatUnion.Num[1];
      box[2] = floatUnion.Num[2];
      box[3] = floatUnion.Num[3];
      
      Serial.println(floatUnion.Num[0]);
      Serial.println(floatUnion.Num[1]);
      Serial.println(floatUnion.Num[2]);
      Serial.println(floatUnion.Num[3]);
      break;
    default:
      Serial.println("invalid command");
      break;
  }
}

/*
 * Definiton: Uses union to decode byte array into float
 */
void DecodePayload(int size, byte *input){
  for (int i = 0; i < size; i++) {
      floatUnion.array[i] = input[i];      
  }
}

/*
 * Definition: exicutes non_blocking controle of the drive motors
 * 
 */
void Drive(uint8_t driveCommand){
  switch(driveCommand){
    case FORWARD:
      Forword();
      break;
    case BACKWARD:
      Backwords();
      break;
    case LEFT:
      Left();
      break;
    case RIGHT:
      Right();
      break;
    case STOP:
      Stop();
      break;
    default:
      Stop();
      Serial.println("drive command out of range!");
      break;
  }
}

void Forword(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH); 
  digitalWrite(L_EN, LOW); 
  digitalWrite(R_EN, LOW);            
}

void Backwords(){
  digitalWrite(L_DIR, LOW); 
  digitalWrite(R_DIR, LOW); 
  digitalWrite(L_EN, LOW); 
  digitalWrite(R_EN, LOW); 
}

void Left(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH); 
  digitalWrite(L_EN, HIGH); 
  digitalWrite(R_EN, LOW); 
}

void Right(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH); 
  digitalWrite(L_EN, LOW); 
  digitalWrite(R_EN, HIGH); 
}

void Stop(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH);          
  digitalWrite(L_EN, HIGH); 
  digitalWrite(R_EN, HIGH);
}

bool inBox(float* coord){
  if(gps.encode(Serial1.read())){
    if(gps.location.lat() > coord[0] 
    || gps.location.lat() < coord[2] 
    || gps.location.lng() > coord[1] 
    || gps.location.lng() < coord[3]){
      return false;
    }else{
      return true;
    }  
  }else{
    Serial.println("Error Reading GPS!");  
  }
}
