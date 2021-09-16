#define ROBOT_ADDRESS "MELVIN"
#define XMTR_ADDRESS "XMTR00"

//Used to encode and decode floats and strings to byte array
union Payload{
  byte 	bytes[32];
  float floats[8];
  int16_t ints[16];
};

/*
	Assumptions:
	- Class data should be cleared and filled with new data before sending
	since same data space is used as buffer for sending and receiving
	
	- Data is stored and sent in the following format...
 
        _______byte info[2]_______  	 ______Union Payload buffer______
       |  command  | Payload Size |     | 			 Payload        	 |
       |--------------------------|		|--------------------------------|
       |  1 Byte   |    1 Byte    |     | 			0-32 Bytes      	 |
       |--------------------------|		|--------------------------------|
    
	In this format the first packet sent is "byte info[2]". This tells the 
	receiver the command and size of the payload. Next the "Union Payload buffer"
	is sent. This is a 32 byte array and only "uint8_t size" bytes are sent.
	
	- The RF24 radio object is stored and managed completely by this class.

*/
class Comm{
	public:
		Comm();
		int initRadio(uint16_t cepin, uint16_t cspin, uint8_t type);
		bool available();
		
		int pullPayload();
		uint8_t getCommand();
		Payload getPayload();
		uint8_t getSize();
		void setCommand(uint8_t newcommand);
		void setPayload(Payload newPayload);
		void setSize(uint8_t newsize);
		int sendPayload();
		void displayPayload();

//Why is this a load data while the others are load and send?		
		bool nrfDebugText(uint8_t newcommand, String text);
		bool sendLocation(float latitude, float longitude, bool good);	
		bool sendSensor(uint16_t x, uint16_t y, uint16_t z);	
		bool sendJoystick(int X, int Y);

	private:
		RF24 radio;
		uint8_t addresses[2][6] = {ROBOT_ADDRESS, XMTR_ADDRESS};
		
		uint8_t command;
		uint8_t size;
		byte info[2];
		Payload buffer;

};

//Constructor for class 
Comm::Comm(){

}

//Initialize the RF24 object 
int Comm::initRadio(uint16_t cepin, uint16_t cspin, uint8_t type){
	// initialize the transceiver on the SPI bus
	if (!radio.begin(cepin, cspin)) {
		Serial.println(F("radio hardware is not responding!!"));
		return 0;
	}

	radio.setPALevel(RF24_PA_LOW);
  
	if(type == 1){
		radio.openWritingPipe(addresses[1]);
		radio.openReadingPipe(1,addresses[0]);
	} else{
		radio.openWritingPipe(addresses[0]);
		radio.openReadingPipe(1,addresses[1]);
	}

	radio.startListening();
	
	return 1;
}

//check is there is a payload available
bool Comm::available(){
	return radio.available();
}

// Getters and setters for command, size, and buffer
uint8_t Comm::getCommand(){
	return command;
}

Payload Comm::getPayload(){ // This 
	return buffer;
}

uint8_t Comm::getSize(){
	return size;
}

void Comm::setCommand(uint8_t newcommand){
	command = newcommand;
}

void Comm::setPayload(Payload newPayload){
	buffer = newPayload;
}

void Comm::setSize(uint8_t newsize){
	size = newsize;
}

void Comm::displayPayload(){
	Serial.print(command);
	Serial.print('|');
	Serial.print(size);
	
	if(size > 0){
		Serial.print(": ");
		
		//print hex
		for(int i = 0; i < size; i++){
			Serial.print(buffer.bytes[i]);
			Serial.print('|');
		}
		Serial.print(" [");
		
		//print char
		for(int i = 0; i < size; i++){
			Serial.print((char)buffer.bytes[i]);
		}
		Serial.print(']');
	}
	Serial.println("");
}

/*
	Assumptions:
	- command, size, buffer and info can be overriden
	- buffer will be saved and decoded outside of the class
*/
int Comm::pullPayload(){ //This loads buffer with up to 32 bytes of info. Is blocking for second payload. requires check of if available.
	// Get the command and size
	radio.read( &info, 2 );
	command = info[0];
	size = info[1];
	
	//Get Payload
	if (size > 0) {
		while(!radio.available());
		radio.read( &buffer, size );
	}
	
	//Debug
	if (false){
		Serial.println("Received Payload:");
		displayPayload();
	}
	
}

/*
	Assumptions:
	- command, size, and buffer have all been filled
	- info will be overwritten
	- If size is 0 the buffer wont be sent
*/
int Comm::sendPayload(){
	//Fill info packet
	info[0] = command;
	info[1] = size;
	
	radio.stopListening();
	//Send info packet
	radio.write( &info, 2 );
	
	//Send Payload packet
	if(size > 0){
		radio.write( &buffer, size );
	}
  
	radio.startListening();
	
	//Debug
	if (false){
		Serial.println("Sent Payload:");
		displayPayload();
	}
	
	//clear buffer
	for(int i = 0; i < size; i++){
		buffer.bytes[i] = 0;
	}
	info[0] = NULL_COMM;
	info[1] = 0;
	
	return 1;
}

/*
	Assumptions:
	- The string being sent is 30 bytes or less. Function will drop extra bytes
	- newcommand will override command
	- size and buffer will be overiden
	- auto sends message
*/
bool Comm::nrfDebugText(uint8_t newcommand, String text){
	//Set size and command
	size = text.length();
	command = newcommand;
	
	if (size > 30) //Double check that there is no buffer overrun
		size = 30;
	
	//Fill payload with string
	for(int i = 0; i < size; i++){
		buffer.bytes[i] = (byte) text.charAt(i);
	}
	
	//Debug 
	if (false){
		Serial.println("Saved Payload:");
		displayPayload();
	}
	
	sendPayload();
	
	return 1;
}

/*
	Assumptions:
	- command will be set to GPS_RESPONSE
	- size and buffer will be overiden
*/
bool Comm::sendLocation(float latitude, float longitude, bool good) {
	//Set size and command
	size = 9;
	command = GPS_RESPONSE;
	
	//Fill payload with two floats
	buffer.floats[0] = latitude;
	buffer.floats[1] = longitude;
	buffer.bytes[8] = good;
	
	//Debug 
	if (false){
		Serial.println("Saved Payload:");
		displayPayload();
	}
	
	return 1;
}

bool Comm::sendSensor(uint16_t x, uint16_t y, uint16_t z) {
	//Set size and command
	size = 6;
	command = SENSOR_RESPONSE;
	
	//Fill payload with two floats
	buffer.ints[0] = x;
	buffer.ints[1] = y;
	buffer.ints[2] = z;
	
	//Debug 
	if (false){
		Serial.println("Saved Payload:");
		displayPayload();
	}
	
	return 1;
}

bool Comm::sendJoystick(int X, int Y){
	//Set size and command
	size = 4;
	command = JOY_DRIVE;
	
	//Fill payload with two short ints
	buffer.ints[0] = X;
	buffer.ints[1] = Y;
}