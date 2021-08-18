int x,y,z;
int LED = 13;

const int numReadings = 10;

int x_readings[numReadings];      // the readings from the A0 input
int readIndex = 0;              // the index of the current reading
int x_total = 0;                  // the running total for x
int x_average = 0;                // the average for x
int x_difference = 0;           // difference between the average and the last x value

int y_readings[numReadings];      // the readings from the A1 input
int y_total = 0;                  // the running total for y
int y_average = 0;                // the average for y

int z_readings[numReadings];      // the readings from the A2 input
int z_total = 0;                  // the running total for z
int z_average = 0;                // the average for z

int x_inputPin = A0;
int y_inputPin = A1;
int z_inputPin = A2;


unsigned long errorMillis = millis();
const unsigned long period = 10000;  //the value is a number of milliseconds(10 sec)

class Safety
{
  public:
    Safety(int x_inputPin,int y_inputPin,int z_inputPin );
    int isLevel();
  private:
    int x,y,z;
    int x_pin, y_pin, z_pin;
};  
  Safety::Safety(int x_inputPin,int y_inputPin,int z_inputPin )
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

Safety TekMow_safety(A0, A1, A2);

void setup() {

   // initialize serial communication with computer
  Serial.begin(9600);

}

void loop() {
  if (TekMow_safety.isLevel() == 1){
    digitalWrite(LED, HIGH);
  } else
  {
    digitalWrite(LED, LOW);
  }
  
 /*
  //getting data and lighting LED
  x = analogRead(A0);
  y = analogRead(A1);
  z = analogRead(A2);

//running average calculations
  // subtract the last reading:
  x_total = x_total - x_readings[readIndex];
  y_total = y_total - y_readings[readIndex];
  z_total = z_total - z_readings[readIndex];
  // read from the sensor:
  x_readings[readIndex] = x;
  y_readings[readIndex] = y;
  z_readings[readIndex] = z;
  // add the reading to the total:
  x_total = x_total + x_readings[readIndex];
  y_total = y_total + y_readings[readIndex];
  z_total = z_total + z_readings[readIndex];
  // advance to the next position in the array:
  readIndex = readIndex + 1;

  // if we're at the end of the array...
  if (readIndex >= numReadings) {
    // ...wrap around to the beginning:
    readIndex = 0;
  }

  calculate_average(x_average, x_total, y_average, y_total,z_average, z_total);
   
  plot_values (x,y,z);
  //plot_averages(x_average,y_average,z_average);
  //light_LED(x, x_average);
  //timer(x, x_average);

  
  if (out_of_range(x, x_average) == 1)
  {
    errorMillis = millis();  //equates error time to current time
  }

  if (millis() - errorMillis >= period)
  {
    digitalWrite(LED, HIGH);
  } else
  {
    digitalWrite(LED, LOW);
  }
  
*/

}
void print_values(int x, int y, int z)
{
    // send it to the computer as ASCII digits
  Serial.print("X reading ");
  Serial.println(x_readings[readIndex]);

  Serial.print("Y reading ");
  Serial.print(y_readings[readIndex]);

  Serial.print("Z reading ");
  Serial.println(z_readings[readIndex]);
}

void plot_all (int x, int y, int z, int xa, int ya, int za)
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
void plot_values (int x, int y, int z)
{
  Serial.print(x);
  Serial.print(",");
  
  Serial.print(y);
  Serial.print(",");

  Serial.println(z);
  
  
}
void plot_averages (int xa, int ya, int za)
{
 
  Serial.print(xa);
  Serial.print(",");
  
  Serial.print(ya);
  Serial.print(",");
  
  Serial.println(za);

}

int out_of_range(int now, int avg)
{
  if (now >= (avg+20) || now <=(avg-20))            //LED lights up if the last reading is not the same as the average
  { 
    return 1;
  } else {
    return 0;
  }
  
}

void calculate_average(int xa, int xt, int ya, int yt, int za, int zt)
{
  x_average = x_total / numReadings;
  y_average = y_total / numReadings;
  z_average = z_total / numReadings;
}
//void check_level(
