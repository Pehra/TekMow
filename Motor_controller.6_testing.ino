#define ROBOT 1

#if ROBOT == 2

#define L_1        5
#define L_2       4
#define R_1        7
#define R_2       6

void Forword(){
  digitalWrite(L_1, LOW); 
  digitalWrite(L_2, HIGH); 
  digitalWrite(R_1, LOW); 
  digitalWrite(R_2, HIGH);            
}

void Backwords(){
  digitalWrite(L_1, HIGH); 
  digitalWrite(L_2, LOW); 
  digitalWrite(R_1, HIGH); 
  digitalWrite(R_2, LOW); 
}

void Left(){
  digitalWrite(L_1, HIGH); 
  digitalWrite(L_2, HIGH); 
  digitalWrite(R_1, LOW); 
  digitalWrite(R_2, HIGH); 
}

void Right(){
  digitalWrite(L_1, LOW); 
  digitalWrite(L_2, HIGH); 
  digitalWrite(R_1, HIGH); 
  digitalWrite(R_2, HIGH); 
}

void Stop(){
  digitalWrite(L_1, HIGH); 
  digitalWrite(L_2, HIGH);          
  digitalWrite(R_1, HIGH); 
  digitalWrite(R_2, HIGH);
}

  
 
#else
#define L_EN        5
#define L_DIR       4
#define R_EN        7
#define R_DIR       6

void Forword(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH); 
  digitalWrite(L_EN, LOW); 
  digitalWrite(R_EN, LOW);            
}

void Backwords(){
  digitalWrite(L_DIR, LOW); 
  digitalWrite(R_DIR, LOW); 
  digitalWrite(L_EN, LOW); 
  digitalWrite(R_EN, LOW); 
}

void Left(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH); 
  digitalWrite(L_EN, HIGH); 
  digitalWrite(R_EN, LOW); 
}

void Right(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH); 
  digitalWrite(L_EN, LOW); 
  digitalWrite(R_EN, HIGH); 
}

void Stop(){
  digitalWrite(L_DIR, HIGH); 
  digitalWrite(R_DIR, HIGH);          
  digitalWrite(L_EN, HIGH); 
  digitalWrite(R_EN, HIGH);
}

#endif

void setup() {
  // put your setup code here, to run once:
#if ROBOT == 2 
    pinMode(L_1, OUTPUT);
    pinMode(L_2, OUTPUT);
    pinMode(R_1, OUTPUT);
    pinMode(R_2, OUTPUT);
    digitalWrite(L_2, HIGH); 
    digitalWrite(R_2, HIGH);          
    digitalWrite(L_1, HIGH); 
    digitalWrite(R_1, HIGH);
#else
    pinMode(L_EN, OUTPUT);
    pinMode(L_DIR, OUTPUT);
    pinMode(R_EN, OUTPUT);
    pinMode(R_DIR, OUTPUT);
    digitalWrite(L_DIR, HIGH); 
    digitalWrite(R_DIR, HIGH);          
    digitalWrite(L_EN, HIGH); 
    digitalWrite(R_EN, HIGH); 
#endif
}

void loop() {
  // put your main code here, to run repeatedly:
 Forword();
 delay(1000);
 Stop();
 delay(1000);
 Backwords();
 delay(1000);
 Stop();
 
}
