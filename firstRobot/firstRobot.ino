/*
 * TekMow Robot Code
 * September 2021
 * 
 * This demo project uses an NRF24L01 RF link to drive a mowing robot. THe code is based on software state machines that implement 
 * the robot functions. A basic set of commands is predefined.
 * 
 * 
 */
 
#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <TinyGPS++.h>

// These files need to have thier locations updated before compile to match where you placed your files.
#include "C:/Users/Don/Desktop/TekMow/tekmow.h"
#include "C:/Users/Don/Desktop/TekMow/Comm.c"

// Only include one of the files below base don the mototr controller type used
#include "C:/Users/Don/Desktop/TekMow/tb_mc.c"
//#include "G:/My Drive/PC Transfer/Desktop/Tekbots/TekMow/TekMow/vesc_mc.c"

#define MOVE_TIME     4000 // The default time (in milliseconds) before a motion watchdog time out.
#define UPDATE_TIME   10000 // Time (in milliseconds) between sending GPS location back to controller
#define GPSBAUD       9600 // Baud rate for the GPS


// This is the class implementation for the accelerometer.
class Safety{
  public:
    Safety(int x_inputPin, int y_inputPin, int z_inputPin );
    int isLevel();
    int isStable();
    void print_values(int x, int y, int z);
    void plot_all(int x, int y, int z, int xa, int ya, int za);
    void plot_averages();
    void plot_values(int x, int y, int z);
    void calculate_average();
    void setError();

  private:
    uint8_t error_size;
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
TinyGPSPlus gps;
Safety TekMow_safety(A0, A1, A2); // Creates the accelerometer object
Comm TekMow_Comm; // Creates communication object

Coord currentCoord;
Coord boxCorner1 = {44.567480, -123.278915};
Coord boxCorner2 = {44.567232, -123.279307};

//State and timer variables
unsigned long driveTime = 0;
unsigned long locationUpdateTime = 0;
unsigned long lastGpsTime = 0;
unsigned long stableTime = 0;
unsigned long motionTime = 0;
unsigned long radioLinkTime = 0;

#define RADIO_TIMEOUT 10000
#define GPS_TIMEOUT 5000

volatile uint8_t drivePS = STOP, driveNS = STOP;
volatile gpsStates gpsPS = GPS_OUTBOUNDS, gpsNS = GPS_OUTBOUNDS;
volatile motionStates motionPS = RECENTMOTION, motionNS = RECENTMOTION;
volatile robotStates robotPS = ROBOT_DISABLE, robotNS = ROBOT_DISABLE;
volatile commStates commState = READ;
String failReason = "No Failure";
bool GPSgood;

/*
 * Definition: exicutes non_blocking controle of the drive motors
 * 
 */
int Drive(uint8_t driveCommand){
//  Serial.println(driveCommand);
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
    case JOY_DRIVE:
      Payload Temp = TekMow_Comm.getPayload();
      anlgDrive(Temp.ints[0], Temp.ints[1]);
      break;
    default:
      Stop();
      Serial.println("drive command out of range!");
      return 0;
      break;
  }
  return 1;
}

int getGPSdata(){
  // feed the GPS object
  while(Serial1.available())
      gps.encode(Serial1.read());
      
//  Serial.println("GPS");
  
  if (gps.location.isValid()){
//    Serial.print(gps.location.lat());
//    Serial.print(" : ");
//    Serial.println(gps.location.lng());
    
    currentCoord.latitude = gps.location.lat();
    currentCoord.longitude = gps.location.lng();
    return true;
  }else{
//    Serial.print(gps.location.lat());
//   Serial.print(" : ");
//    Serial.println(gps.location.lng());
    return false;
  }
}

int inBox(Coord one, Coord two) {
  if (gps.location.isValid()) {
    if (gps.location.lat() < one.latitude
        && gps.location.lat() > two.latitude
        && gps.location.lng() < one.longitude
        && gps.location.lng() > two.longitude)
      return 1;//return 1 if it is in the box
    else
      return 2;//return 2 if it is not in the box
  } else
    return 0; //return 0 if there is a GPS error

}

int getCommand(){
  //Grab Info and Payload packets
  TekMow_Comm.pullPayload();

  //Processes based on Command
  uint8_t newCommand = TekMow_Comm.getCommand();
  switch (newCommand){
    case FORWARD:
    case BACKWARD:
    case LEFT:
    case RIGHT:
    case STOP:
    case JOY_DRIVE:
      if (robotPS == ROBOT_ARMED || robotPS == ROBOT_MOW){
        driveNS = newCommand;
        driveTime = millis();
        if (Drive(driveNS) == 1)
          drivePS = driveNS;
        return 1;
      }
      break;
    case SET_COORD:
      return 1;
    case HEART_BEAT:
      return 1;
    case ECHO:
      commState = COMM_ECHO;
      return 1;
    case COMM_ARM:
      robotNS = ROBOT_ARMED;
      return 1;
    case COMM_MOW:
      robotNS = ROBOT_MOW;
      return 1;
    case COMM_DISABLE:
      robotNS = ROBOT_DISABLE;
      return 1;
    default:
      Serial.println("invalid command");
      return 0;
  }
  
}

/**********************|| Safety ||**********************/

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

  if (x >= 310 && x <= 350 && y >= 310 && y <= 350 && z > 350) // x and y axis are near 0G and the robot is right side up.
  {
    return 1; // Return we are level
  } else {
    return 0; // Return we are not level.
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

void Safety::print_values(int x, int y, int z)
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

void stopRobot(){
  Stop();
  drivePS = driveNS = STOP;
}

void startMow() { //Activate mowing relay
  return;
}

void stopMow() { //Deactivate mowing relay
  return;
}

void setup() {
  Serial.begin(115200); // Sets up debug serial monitor
  Serial1.begin(GPSBAUD); // Connection to GPS

  Serial.println(F("TekMow Robot Starting"));
  if (!TekMow_Comm.initRadio(9,10,1)) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1); // hold in infinite loop
  }

  motor_init();
  drivePS = driveNS = STOP;
  driveTime = millis();
}

/*
   Notes: this section of code will only be used to manage
   state logic and watchdog timers
*/
void loop() {
  /**********************|| Receving Command ||**********************/
  if (TekMow_Comm.available()){
    if (getCommand()){
      radioLinkTime = millis();
    }
  }
     
  if (millis() > (radioLinkTime + RADIO_TIMEOUT)) { 
    stopRobot();
    robotNS = ROBOT_ERROR;
    Serial.println("Radio Link Failed");
    failReason = "Radio not transmitting.";
  }

  /****Idle state: if there isnt an error the robot can do things****/
  if (robotNS != ROBOT_ERROR && robotPS != ROBOT_ERROR){
    /**********************|| Read Sensors ||**********************/
    
    if (robotPS == ROBOT_DISABLE){ //Do things that only need ot happen when not armed or mowing
      //Calculate running average for stabliity testing. Only to be done when not armed or mowing.
      if (millis() > (motionTime + 5)) {
        motionTime = millis();
        // Take new measurements and average
        TekMow_safety.calculate_average();
        if (TekMow_safety.isStable())
          stableTime = millis();
      }
    }
    
    if (lastGpsTime + 50 < millis()){ // Always be reading from the Serial port to avoid missing GPS messages (50mS)
      GPSgood = getGPSdata();
      lastGpsTime = millis();
    }
    
    if (locationUpdateTime + GPS_TIMEOUT < millis()) { //Checks if in Box every 5 seconds.
      locationUpdateTime = millis();
      int box = inBox(boxCorner1, boxCorner2);
        if (box == 1) { 
          gpsNS = GPS_INBOUNDS;
        } else if (box == 2) {
          gpsNS = GPS_OUTBOUNDS;
        } else {
          gpsNS = GPS_ERROR;
        }
    }


    /**********************|| Safty Checks ||********************
    This section is for checks that should always be running regardless of robot state
    **/
    
    if (millis() > (driveTime + MOVE_TIME)) { // Drive Watchdog Timer
      driveTime = millis();
      Stop();
      driveNS = STOP;
      TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Drive Watchdog Elapsed");
    }

    //Check GPS box
    if (gpsNS == GPS_OUTBOUNDS) {
//      Serial.println("Not in GPS Region");
      TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Robot Not in GPS Region");
      robotNS = ROBOT_DISABLE;
      gpsPS = gpsNS;
    }

    /**********************|| Control Cycle ||**********************/
    /****Control state: If it is safe the robot can move and mow****/

    if(robotPS == ROBOT_DISABLE){
      if(robotNS == ROBOT_ARMED){
//Line below is wrong and need to be less than instead once accelerometer is installed.
        if (millis() > stableTime + 10000){ // Not stable so report error and disable state change
          robotNS = ROBOT_DISABLE;
          TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Robot not stable, state change canceled");
          TekMow_Comm.sendPayload();
        }
      }
      if(robotNS == ROBOT_MOW){
        robotNS = ROBOT_DISABLE;
        TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Robot can not move from ROBOT_DISABLE to ROBOT_MOW");
        TekMow_Comm.sendPayload();
      }
    } 
    else if(robotPS == ROBOT_ARMED){
      if(robotNS == ROBOT_DISABLE){
        Drive(STOP);
        TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Moving to ROBOT_DISABLE");
        TekMow_Comm.sendPayload();
      }
      if(robotNS == ROBOT_MOW){
        TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Moving to ROBOT_MOW");
        TekMow_Comm.sendPayload();
        startMow();
      }
    } 
    else if(robotPS == ROBOT_MOW){
      if(robotNS == ROBOT_DISABLE){
        Drive(STOP);
        stopMow();
        TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Moving to ROBOT_DISABLE");
        TekMow_Comm.sendPayload();
      }
      if(robotNS == ROBOT_ARMED){
        stopMow();
        TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Moving to ROBOT_ARMED");
        TekMow_Comm.sendPayload();
      }
    }

    robotPS = robotNS;
     
    if(robotPS == ROBOT_ARMED || robotPS == ROBOT_MOW){ // These states allows for motion
      if(driveNS == JOY_DRIVE ) {  //joystick overides serial motion commands
        Drive(driveNS); 
      } else if (driveNS != drivePS && drivePS != JOY_DRIVE){
        Drive(driveNS);
      }

      drivePS = driveNS;
    }

    /****Error state: If we are in this error mode, the only thing we can do is communicate****/
  } else { 
    Serial.println("Robot in ROBOT_ERROR state!");
    Serial.print("Cause: ");
    Serial.println(failReason);
    delay(10000);
  } // End if ROBOT_ERROR

  /**********************|| Reply ||**********************/
  if(commState != READ){
    //Package data to send to controller
    switch (commState){
      case COMM_ECHO:
        TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Echo Back");
        break;
      case SEND_GPS:
        TekMow_Comm.sendLocation(currentCoord.latitude, currentCoord.longitude);
        break;
      default:
        //Serial.println("Invalid Response");
        break;
    }
  
    //send data
    TekMow_Comm.sendPayload();
  
    //reset commState
    commState = READ;
  }
    
} // End loop()
