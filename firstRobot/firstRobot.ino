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
#include "C:/Users/pooki/Desktop/Tekbots/TekMow/TekMow/tekmow.h"
#include "C:/Users/pooki/Desktop/Tekbots/TekMow/TekMow/tb_mc.c"
#include "C:/Users/pooki/Desktop/Tekbots/TekMow/TekMow/Comm.c"

#define MOVE_TIME     1000 // The default time beofee a motion watchdog time out.
#define UPDATE_TIME   10000 // Time between sending GPS location back to controller
#define GPSBAUD       9600

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
    char error[];
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

Coord mycoord;
Coord coord1 = {44.567480, -123.278915};
Coord coord2 = {44.567232, -123.279307};
float box[4];

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
volatile commStates commState = READ;
String failReason = "No Failure";

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

int getCommand(){
  //Grab Info and Payload packets
  TekMow_Comm.pullPayload();

  //Prosses based on Command
  uint8_t newCommand = TekMow_Comm.getCommand();
  switch (newCommand){
    case FORWARD:
    case BACKWARD:
    case LEFT:
    case RIGHT:
    case STOP:
      driveNS = newCommand;
      driveTime = millis();
      break;
    case SET_COORD:
      break;
    case HEART_BEAT:
      break;
    case ECHO:
      commState = COMM_ECHO;
      break;
    default:
      Serial.println("invalid command");
      return 0;
  }
  return 1;
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
  if (!TekMow_Comm.initRadio(9,10,0)) {
    Serial.println(F("radio hardware is not responding!!"));
    while (1); // hold in infinite loop
  }
  
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
    if (TekMow_Comm.available()) 
      if (getCommand()){
        radioLinkTime = millis();
      }

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

    //update location
    if (millis() > (locationUpdateTime + UPDATE_TIME)) {
      TekMow_Comm.sendLocation(mycoord.latitude, mycoord.longitude);
      TekMow_Comm.sendPayload();
      locationUpdateTime = millis();
    }

    //Handle command reply's
    if(commState != READ){
      //Package data to send to controller
      switch (commState){
        case COMM_ECHO:
          TekMow_Comm.nrfDebugText(ECHO_RESPONSE, "Echo Back");
          break;
        default:
          //Serial.println("Invalid Responce");
          break;
      }

      //send data
      TekMow_Comm.sendPayload();

      //reset commState
      commState = READ;
    }
        
  } else { // This is what shold happen if the robot is in error mode. Should log info repeatedly.
    Serial.println("Robot in ROBOT_ERROR state. Hard Reset to continue.");
    Serial.print("Cause: ");
    Serial.println(failReason);
    delay(10000);
  } // End if ROBOT_ERROR
} // End loop()
