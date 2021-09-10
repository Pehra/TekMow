struct Position{
	short X;
	short Y;
};

class Joystick{
	public:
		Joystick(uint8_t x_pin, uint8_t y_pin, uint8_t dead, short stopSend);
		
		void calibration();
		short getX();
		short getY();
		void readPos();
		void disp(); //do not capitalize functions
		bool alive();
		
	private:
		uint8_t x_pin, y_pin, dead;
		int8_t x_mod = 1, y_mod = 1;
		Position cal, pos, lower, upper;
		unsigned long lastMove;
		short stopSend;
		

};

//Constructor for class 
Joystick::Joystick(uint8_t x_pin, uint8_t y_pin, uint8_t dead, short stopSend){
	this->x_pin = x_pin;
	this->y_pin = y_pin;
	this->dead = dead;
	this->stopSend = stopSend;
	lastMove = millis();
}

void Joystick::calibration(){	
	Serial.println("Keep your joystick in the center position...\nwait...");
	delay(700);
	Serial.print("1...");
	delay(700);
	Serial.print("2...");
	delay(700);
	Serial.println("3!");
	
	
	
	lower.X = -cal.X;
	lower.Y = -cal.Y;
	upper.X = 1023 - cal.X;
	upper.Y = 1023 - cal.Y;
	
	Serial.println("move your joystick to the forword position...\nwait...");
	
	do{
		readPos();
	}while(!((pos.X == 0 && pos.Y != 0) || (pos.X != 0 && pos.Y == 0)));
	
	if ( pos.X == 0){
		if ( pos.Y < 0){
			y_mod = -1;
		}else if( pos.Y > 0){
			y_mod = 1;
		}
	}else if( pos.Y == 0){
		uint8_t temp = x_pin;
		x_pin = y_pin;
		y_pin = temp;
		
		if ( pos.X < 0){
			y_mod = -1;
		}else if( pos.X > 0){
			y_mod = 1;
		}
	}
	
	Serial.println("move your joystick to the right position...\nwait...");
	
	do{
		readPos();
	}while(!(pos.X != 0 && pos.Y == 0));
	
	if ( pos.X < 0){
		x_mod = -1;
	}else if( pos.X > 0){
		x_mod = 1;
	}
	
	Serial.print("mod_x:"); Serial.println(x_mod);
    Serial.print("mod_y:"); Serial.println(y_mod);
	
}

void Joystick::readPos(){
	//Read potentiamiters
	short Xvalue = analogRead(x_pin)- cal.X;
	short Yvalue = analogRead(y_pin)- cal.Y;
	
	//Deadzone
	if(Xvalue < -dead ){
		Xvalue = map(Xvalue, lower.X, -dead , -100, -1);
	}else if(Xvalue > dead){
		Xvalue = map(Xvalue, dead , upper.X, 1, 100);
	}else{
		Xvalue = 0;
	}	
	
	if(Yvalue < -dead ){
		Yvalue = map(Yvalue, lower.Y, -dead, -100, -1);
	}else if(Yvalue > dead){
		Yvalue = map(Yvalue, dead, upper.Y, 1, 100);
	}else{
		Yvalue = 0;
	}	

	pos.X = Xvalue * x_mod;
	pos.Y = Yvalue * y_mod;
}

short Joystick::getX(){
	return pos.X;
}

short Joystick::getY(){
	return pos.Y;
}

void Joystick::disp(){
	Serial.print("JOY_X:"); Serial.println(pos.X);
    Serial.print("JOY_Y:"); Serial.println(pos.Y);
}

bool Joystick::alive(){
	readPos();
	
	if(pos.X != 0 || pos.Y != 0){
		lastMove = millis();
		return true;
	}else if(millis() > lastMove + stopSend){
		return false;
	}else{
		return true;
	}
}
