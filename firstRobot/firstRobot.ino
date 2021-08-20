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
#include "C:/Users/Don/Desktop/TekMow/tekmow.h"

// Defines for the motor controller. Only needed/used for the small robots.
#define L_EN        5
#define L_DIR       4
#define R_EN        7
#define R_DIR       6

#define ROBOT_ADDRESS "1Node"
#define XMTR_ADDRESS "2Node"

uint8_t addresses[][6] = {ROBOT_ADDRESS, XMTR_ADDRESS};

#define MOVE_TIME     1000 // The default time beofee a motion watchdog time out.
#define UPDATE_TIME   10000 // Time between sending GPS location back to controller
#define GPSBAUD       9600

class Safety
{
  public:
    Safety(int x_inputPin, int y_inputPin, int z_inputPin );
    int isLevel();
    int isStable();
    void print_values(int x, int y, int z);
    void plot_all(int x, int y, int z, int xa, int ya, int za);
    void plot_averages();
    void plot_values(int x, int y, int z);
    void calculate_average();

  private:
    int x, y, z;
    int x_pin, y_pin, z_pin;
    const int numReadings = 10;
    int x_readings[10];      // the readings from the A0 input
    int readIndex = 0;              // the index of the current reading
    int xt = 0;                  // the running total for x
    int xa = 0;                // the average for x
    int xd = 0;           // difference between the average and the last x value

    int y_readings[10];      // the readings from the A1 input
    int yt = 0;                  // the running total for y
    int ya = 0;                // the average for y

    int z_readings[10];      // the readings from the A2 input
    int zt = 0;                  // the running total for z
    int za = 0;                // the average for z

};

/* Hardware configuration:
    Set up nRF24L01 radio on SPI bus plus pins 9 & 10
    Set Up GPS on RX1/TX1
*/
RF24 radio(9, 10);

TinyGPSPlus gps;
Safety TekMow_safety(A0, A1, A2); // Creates the accelerometer object

//used to encode/ decode floats to byte array
union {
  byte array[16];
  float Num[4];
} floatUnion;

Coord mycoord;
Coord coord1 = {44.567480, -123.278915};
Coord coord2 = {44.567232, -123.279307};

byte payload[30];
float box[4];
uint8_t command, size;

//State and timer variables
unsigned long driveTime = 0;
unsigned long locationUpdateTime = 0;
unsigned long lastGps = 0;
unsigned long stableTime = 0;
unsigned long motionTime = 0;
unsigned long radioLinkTime = 0;

volatile uint8_t drivePS = STOP, driveNS = STOP;
volatile gpsStates gpsPS = GPS_OUTBOUNDS, gpsNS = GPS_OUTBOUNDS;
volatile motionStates motionPS = RECENTMOTION, motionNS = RECENTMOTION;
volatile robotStates robotPS = DISABLE, robotNS = DISABLE;
String failReason = "No Failure";



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

int GetCommand() {
  uint8_t command, size;
  radio.read( &command, 1 );           // Get the command
  radio.read( &size, 1 );              // Get the size
  if (size > 0) {
    radio.read( &payload, size );
  }

  switch (command) {
    case FORWARD:
    case BACKWARD:
    case LEFT:
    case RIGHT:
    case STOP:
      driveNS = command;
      driveTime = millis();

      return 1;

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
      return 1;
    case HEART_BEAT:
      return 1;
    default:
      Serial.println("invalid command");
      return 0;
  }
  return 0;
}

void nrfDebugText(String text){

  
}

int inBox(Coord one, Coord two) {
  if (gps.location.isValid()) {
    if (gps.location.lat() < one.latitude
        && gps.location.lat() > two.latitude
        && gps.location.lng() < one.longitude
        && gps.location.lng() > two.longitude)
      return 1;//return 1 if it is
    else
      return 2;//return 2 if it isnt
  } else
    return 0;

}

boolean sendLocation(float latitude, float longitude) {
  return 1;
}



/*
   Definiton: Uses union to decode byte array into float
*/
void DecodePayload(int size, byte *input) {
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
      Backwards();
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

void Forword() {
  digitalWrite(L_DIR, HIGH);
  digitalWrite(R_DIR, HIGH);
  digitalWrite(L_EN, LOW);
  digitalWrite(R_EN, LOW);
}

void Backwards() {
  digitalWrite(L_DIR, LOW);
  digitalWrite(R_DIR, LOW);
  digitalWrite(L_EN, LOW);
  digitalWrite(R_EN, LOW);
}

void Left() {
  digitalWrite(L_DIR, HIGH);
  digitalWrite(R_DIR, HIGH);
  digitalWrite(L_EN, HIGH);
  digitalWrite(R_EN, LOW);
}

void Right() {
  digitalWrite(L_DIR, HIGH);
  digitalWrite(R_DIR, HIGH);
  digitalWrite(L_EN, LOW);
  digitalWrite(R_EN, HIGH);
}

void Stop() {
  digitalWrite(L_DIR, HIGH);
  digitalWrite(R_DIR, HIGH);
  digitalWrite(L_EN, HIGH);
  digitalWrite(R_EN, HIGH);
}


Safety::Safety(int x_inputPin, int y_inputPin, int z_inputPin )
{
  x_pin = x_inputPin;
  y_pin = y_inputPin;
  z_pin = z_inputPin;
}

Safety::isLevel()
{
  x = analogRead(x_pin);
  y = analogRead(y_pin);
  z = analogRead(z_pin);

  if (x >= 310 && x <= 350 && y >= 310 && y <= 350 && z > 350)
  {
    return 1;
  } else {
    return 0;
  }
}


Safety::isStable()
{
  x = analogRead(x_pin);
  y = analogRead(y_pin);
  z = analogRead(z_pin);

  if (x >= (xa + 20) || x <= (xa - 20) ||
      y >= (ya + 20) || y <= (ya - 20) ||
      z >= (za + 20) || z <= (za - 20)) //LED lights up if the last reading is not the same as the average
  {
    return 0;
  } else {
    return 1;
  }
}

void Safety:: print_values(int x, int y, int z)
{
  // send it to the computer as ASCII digits
  Serial.print("X reading ");
  Serial.println(x_readings[readIndex]);

  Serial.print("Y reading ");
  Serial.print(y_readings[readIndex]);

  Serial.print("Z reading ");
  Serial.println(z_readings[readIndex]);
}
void Safety::plot_all (int x, int y, int z, int xa, int ya, int za)
{
  Serial.print(x);
  Serial.print(",");
  Serial.print(xa);
  Serial.print(",");

  Serial.print(y);
  Serial.print(",");
  Serial.print(ya);
  Serial.print(",");

  Serial.print(z);
  Serial.print(",");
  Serial.println(za);
}
void Safety::plot_values (int x, int y, int z)
{
  Serial.print(x);
  Serial.print(",");

  Serial.print(y);
  Serial.print(",");

  Serial.println(z);


}
void Safety::plot_averages()
{

  Serial.print(xa);
  Serial.print(",");
  Serial.print(ya);
  Serial.print(",");
  Serial.println(za);

}


void Safety::calculate_average() {

  x = analogRead(x_pin);
  y = analogRead(y_pin);
  z = analogRead(z_pin);

  xt = xt - x_readings[readIndex];
  yt = yt - y_readings[readIndex];
  zt = zt - z_readings[readIndex];

  x_readings[readIndex] = x;
  y_readings[readIndex] = y;
  z_readings[readIndex] = z;

  xt = xt + x_readings[readIndex];
  yt = yt + y_readings[readIndex];
  zt = zt + z_readings[readIndex];

  readIndex = readIndex + 1;

  if (readIndex >= numReadings)
    readIndex = 0;

  xa = xt / numReadings;
  ya = yt / numReadings;
  za = zt / numReadings;
}

void setup() {
  Serial.begin(115200);
  Serial1.begin(GPSBAUD);


  Serial.println(F("TekMow Robot Starting"));
  if (!radio.begin()) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1); // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW);

  radio.openWritingPipe(addresses[0]);
  radio.openReadingPipe(1, addresses[1]);

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
   Notes: this section of code will only be used to manage
   state logic and watchdog timers
*/
void loop() {

  if (millis() > (radioLinkTime + 10000)) { // This is the radio link timer
    Stop();
    driveNS = STOP;
    robotNS = ROBOT_ERROR;
    Serial.println("Radio Link Failed");
    failReason = "Radio not transmitting.";

  }
  
  if (robotPS != ROBOT_ERROR && robotNS != ROBOT_ERROR){ // If we are in this error mode, the only thing we can do is reset robot.
    /**********************|| Receving Command ||**********************/
    if (radio.available()) {
      if (GetCommand() == 1)
        radioLinkTime = millis();

    /**********************|| Control Cycle ||**********************/
    if (driveNS != drivePS) {
      Drive(driveNS);
      drivePS = driveNS;
    }
  
    if (gpsNS != gpsPS) {
      gpsPS = gpsNS;
    }
  
    if (motionNS != motionPS) {
      motionPS = motionNS;
    }
  
    if (robotNS != robotPS) {
      if (robotNS == ARMED) {
        if (inBox(coord1, coord2) != 1) {
          Serial.println("Not in GPS Region");
          robotNS = robotPS;
        }
        if (TekMow_safety.isStable() != 1) {
          Serial.println("Not Level");
          robotNS = robotPS;
        }
        if (millis() < stableTime + 10000) {
          Serial.println("Not Stable");
          robotNS = robotPS;
        }
      }
      robotPS = robotNS;
    }
  
    if (millis() > (driveTime + MOVE_TIME)) {
      driveTime = millis();
      Stop();
      driveNS = STOP;
    }
  
    if (robotPS != ARMED)
      if (millis() > (motionTime + 5)) {
        motionTime = millis();
        // Take new measurements and average
        TekMow_safety.calculate_average();
        if (TekMow_safety.isStable() != 1)
          stableTime = millis();
      }
  
    /**********************|| Read Sensors ||**********************/
  
    if (Serial1.available())
      gps.encode(Serial1.read());
  
    if (lastGps + 5000 < millis()) { //Checks if in Box every 5 seconds.
      lastGps = millis();
      int box = inBox(coord1, coord2);
      if (box == 1) { //not in the box or no GPS
        //Serial.println("In the Box");
        gpsNS = GPS_INBOUNDS;
      } else if (box == 2) {
        //Serial.println("Out of the Box");
        Stop();
        driveNS = STOP;
        gpsNS = GPS_OUTBOUNDS;
      } else {
        //Serial.println("Invalid GPS");
        Stop();
        driveNS = STOP;
        gpsNS = GPS_ERROR;
      }
    }
    /**********************|| Send Data ||**********************/
  
    if (millis() > (locationUpdateTime + UPDATE_TIME)) {
      sendLocation(mycoord.latitude, mycoord.longitude);
      locationUpdateTime = millis();
    }
    
  } else { // This is what shold happen if the robot is in error mode. Should log info repeatedly.
    Serial.println("Robot in ROBOT_ERROR state. Hard Reset to continue.");
    Serial.print("Cause: ");
    Serial.println(failReason);
    delay(10000);
  } // End if ROBOT_ERROR
} // End loop()
