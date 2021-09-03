// Defines for the motor controller. Only needed/used for the small robots.
#define L_EN        5
#define L_DIR       4
#define R_EN        7
#define R_DIR       6

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
void motor_init() {
	pinMode(L_EN, OUTPUT);
	pinMode(L_DIR, OUTPUT);
	pinMode(R_EN, OUTPUT);
	pinMode(R_DIR, OUTPUT);
	Stop();
}

//assumption: values are -100 -> 100
void anlgDrive(int X, int Y){
	//getting throttle values
	int RSpeed = map(Y, -100 , 100, -255, 255);
    int LSpeed = map(Y, -100 , 100, -255, 255);
	
	if(X < 0){
		RSpeed = RSpeed + map(X, -100 , 0, 50, 0);
		LSpeed = LSpeed - map(X, -100 , 0, 50, 0);
	}else if(X > 0){
		RSpeed = RSpeed - map(X, 0 , 100, 0, 50);
		LSpeed = LSpeed + map(X, 0 , 100, 0, 50);
	}
	
	//writing values
	analogWrite(R_EN, 255 - abs(RSpeed));
	analogWrite(L_EN, 255 - abs(LSpeed));
	
	if(RSpeed < 0)
		digitalWrite(R_DIR, LOW);
	else
		digitalWrite(R_DIR, HIGH);
	
	if(LSpeed < 0)
		digitalWrite(L_DIR, LOW);
	else
		digitalWrite(L_DIR, HIGH);
}