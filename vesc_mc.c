// Defines for the motor controller.
#define VESK_R_OUT  A6
#define VESK_L_OUT  A7

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