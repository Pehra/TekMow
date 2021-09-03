#define JOY_X       A1     
#define JOY_Y       A0  
#define Button 		8
#define DEAD        10 

class Joystick{
	public:
		Joystick(int x_pin, int y_pin, int dead);
		
		void calibration();
		void getValue();
		void Disp(); //do not capitalize functions
		bool Alive();
		
	private:
		int cal_X, cal_Y, x_pin, y_pin, dead;
		
		const int numReadings = 10;
		int readIndex = 0;          // the index of the current reading
		
		int x_readings[10];      	// the readings from the A0 input
		int xt = 0;               	// the running total for x
		int xa = 0;                	// the average for x
		int xd = 0;           		// difference between the average and the last x value

		int y_readings[10];      	// the readings from the A1 input
		int yt = 0;                 // the running total for y
		int ya = 0;                	// the average for y

};

//Constructor for class 
Joystick::Joystick(int x_pin, int y_pin){
	x_pin = x_pin;
	y_pin = y_pin;
}

void Joystick::calibration(){
	Serial.println("Keep your joystick in the center position...");//keep joysticked centered
	delay(2500);//wait 2.5 seconds
	
	cal_X = analogRead(X)-512;//read center x
	cal_Y = analogRead(Y)-512;//read center y
}

void Joystick::getValue(){
	//Read potentiamiters
	int Xvalue = analogRead(X)- cal_X;
	int Yvalue = analogRead(Y)- cal_Y;
	
	//Deadzone on X axis
	if(Xvalue < -DEAD ){
		Xvalue = map(Xvalue, -512, -DEAD, -100, -1);
	}else if(Xvalue > DEAD){
		Xvalue = map(Xvalue, DEAD, 512, 1, 100);
	}else{
		Xvalue = 0;
	}
	
	//Deadzone on Y axis
	if(Xvalue < -DEAD ){
		Xvalue = map(Xvalue, -512, -DEAD, -100, -1);
	}else if(Xvalue > DEAD){
		Xvalue = map(Xvalue, DEAD, 512, 1, 100);
	}else{
		Xvalue = 0;
	}
	
	X = Xvalue;
	Y = Yvalue;
}

void Joystick::Disp(){
	Serial.print("JOY_X:"); Serial.println(X);
    Serial.print("JOY_Y:"); Serial.println(Y);
}

bool Joystick::Alive(){
	getValue();

	xt = xt - x_readings[readIndex];
	yt = yt - y_readings[readIndex];

	x_readings[readIndex] = X;
	y_readings[readIndex] = Y;

	xt = xt + x_readings[readIndex];
	yt = yt + y_readings[readIndex];

	readIndex = readIndex + 1;

	if (readIndex >= numReadings)
	readIndex = 0;

	xa = xt / numReadings;
	ya = yt / numReadings;
	
	if(xa == 0 && ya == 0){
		return false;
	}else{
		return true;
	}
}