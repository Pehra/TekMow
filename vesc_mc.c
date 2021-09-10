// Defines for the motor controller.
#define VESK_R_OUT  6
#define VESK_L_OUT  5

#include <Servo.h>

Servo esc_L;
Servo esc_R;

void Forword() {
	esc_R.writeMicroseconds(2000);
	esc_L.writeMicroseconds(2000);
}

void Backwards() {
	esc_R.writeMicroseconds(1000);
	esc_L.writeMicroseconds(1000);
}

void Left() {
	esc_R.writeMicroseconds(2000);
	esc_L.writeMicroseconds(1500);
}

void Right() {
	esc_R.writeMicroseconds(1500);
	esc_L.writeMicroseconds(2000);
}

void Stop() {
	esc_R.writeMicroseconds(1500);
	esc_L.writeMicroseconds(1500);
}

void motor_init() {
	esc_L.attach(VESK_L_OUT);
	esc_R.attach(VESK_R_OUT);
	Stop();
}

//assumption: values are -100 -> 100
void anlgDrive(int X, int Y){
	int RSpeed = map(Y, -100 , 100, -350, 350);
    int LSpeed = map(Y, -100 , 100, -350, 350);
	
	int turnAmount = map(X, -100 , 100, -150, 150);
	
	if(Y > 0){
		RSpeed = RSpeed - turnAmount;
		LSpeed = LSpeed + turnAmount;
	}else if (Y < 0){
		RSpeed = RSpeed + turnAmount;
		LSpeed = LSpeed - turnAmount;
	}else if (Y == 0){
		RSpeed =  turnAmount * -2;
		LSpeed =  turnAmount *  2;
	}
	
	RSpeed += 1500;
	LSpeed += 1500;
	
	/*Serial.print("2000 1000 ");
	Serial.print(LSpeed);
	Serial.print(" ");
	Serial.println(RSpeed);
	*/
	
	esc_R.writeMicroseconds(RSpeed);
	esc_L.writeMicroseconds(LSpeed);
}