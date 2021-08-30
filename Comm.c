#define ROBOT_ADDRESS "MELVIN"
#define XMTR_ADDRESS "XMTR00"

//Used to encode and decode floats and strings to byte array
union Payload{
  byte 	Byt[32];
  char 	Str[32];
  float Num[8];
};

/*
	Assumptions:
	- Class data should be cleared and filled with new data befor sending
	since same data space is uesed as buffer for sending and receiving
	
	- Data is stored and sent in the following format...
 
        _______byte Info[2]_______  	 ______Union Payload Buffer______
       |  Command  | Payload Size |     | 			 Payload        	 |
       |--------------------------|		|--------------------------------|
       |  1 Byte   |    1 Byte    |     | 			0-32 Bytes      	 |
       |--------------------------|		|--------------------------------|
    
	In this format the first packet sent is "byte Info[2]". This tells the 
	receiver the command and size of the payload. Next the "Union Payload Buffer"
	is sent. This is a 32 byte array and only "uint8_t Size" bytes are sent.
	
	- The RF24 radio object is stored and managed compleatly by this class.

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
		void setCommand(uint8_t newCommand);
		void setPayload(Payload newPayload);
		void setSize(uint8_t newSize);
		int sendPayload();
		
		bool nrfDebugText(uint8_t newCommand, String text);
		bool sendLocation(float latitude, float longitude);	

	private:
		RF24 radio;
		uint8_t addresses[2][6] = {ROBOT_ADDRESS, XMTR_ADDRESS};
		
		uint8_t Command;
		uint8_t Size;
		byte Info[2];
		Payload Buffer;

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

//check is there is a package avalable
bool Comm::available(){
	return radio.available();
}

// Getters and setters for Command, Size, and Buffer
uint8_t Comm::getCommand(){
	return Command;
}

Payload Comm::getPayload(){
	return Buffer;
}

uint8_t Comm::getSize(){
	return Size;
}

void Comm::setCommand(uint8_t newCommand){
	Command = newCommand;
}

void Comm::setPayload(Payload newPayload){
	Buffer = newPayload;
}

void Comm::setSize(uint8_t newSize){
	Size = newSize;
}

/*
	Assumptions:
	- Command, Size, Buffer and Info can be overriden
	- Buffer will be saved and decoded outside of the class
*/
int Comm::pullPayload(){
	// Get the command and size
	radio.read( &Info, 2 );
	Command = Info[0];
	Size = Info[1];
	
	//Get Payload
	if (Size > 0) {
		while(!radio.available()){}
		radio.read( &Buffer, Size );
	}
	
	//Debug
	if (false){
		Serial.println("Received Payload:");
		Serial.print(Command);
		Serial.print('|');
		Serial.print(Size);
		for(int i = 0; i < Size; i++){
			Serial.print('|');
			Serial.print(Buffer.Byt[i]);
		}
		Serial.println(' ');
	}
	
}

/*
	Assumptions:
	- Command, Size, and Buffer have all been filled
	- Info will be overiden
	- If Size is 0 the Buffer wont be sent
*/
int Comm::sendPayload(){
	radio.stopListening();
	
	//Fill info packet
	Info[0] = Command;
	Info[1] = Size;
	
	//Send info packet
	radio.write( &Info, 2 );
	
	//Send Payload packet
	if(Size > 0){
		radio.write( &Buffer, Size );
	}
  
	radio.startListening();
	
	//Debug
	if (false){
		Serial.println("Sent Payload:");
		Serial.print(Info[0]);
		Serial.print('|');
		Serial.print(Info[1]);
		for(int i = 0; i < Size; i++){
			Serial.print('|');
			Serial.print(Buffer.Byt[i]);
		}
		Serial.println(' ');
	}
	
	//clear buffer
	for(int i = 0; i < Size; i++){
		Buffer.Byt[i] = 0;
	}
	Info[0] = NULL_COMM;
	Info[1] = 0;
	
	return 1;
}

/*
	Assumptions:
	- The string being sent is 32 bytes or less
	- newCommand will overide Command
	- Size and Buffer will be overiden
*/
bool Comm::nrfDebugText(uint8_t newCommand, String text){
	//Set Size and Command
	Size = text.length();
	Command = newCommand;
	
	//Fill payload with string
	for(int i = 0; i < Size; i++){
		Buffer.Str[i] = text.charAt(i);
	}
	
	//Debug 
	if (false){
		Serial.println("Saved Payload:");
		Serial.print(Command);
		Serial.print('|');
		Serial.print(Size);
		for(int i = 0; i < Size; i++){
			Serial.print('|');
			Serial.print(Buffer.Str[i]);
		}
		Serial.println(' ');
	}
	
	return 1;
}

/*
	Assumptions:
	- Command will be set to GPS_RESPONSE
	- Size and Buffer will be overiden
*/
bool Comm::sendLocation(float latitude, float longitude) {
	//Set Size and Command
	Size = 8;
	Command = GPS_RESPONSE;
	
	//Fill payload with two floats
	Buffer.Num[0] = latitude;
	Buffer.Num[1] = longitude;
	
	//Debug 
	if (false){
		Serial.println("Saved Payload:");
		Serial.print(Command);
		Serial.print('|');
		Serial.print(Size);
		for(int i = 0; i < Size; i++){
			Serial.print('|');
			Serial.print(Buffer.Num[i]);
		}
		Serial.println(' ');
	}
	
	return 1;
}