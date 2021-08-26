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
  digitalWrite(L_DIR, HIGH);
  digitalWrite(R_DIR, HIGH);
  digitalWrite(L_EN, HIGH);
  digitalWrite(R_EN, HIGH);
}