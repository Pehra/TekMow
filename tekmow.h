//Command List
typedef enum {
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  STOP,
  SET_COORD,
  GPS_RESPONCE,
  
  DUMP_VARS,
  READ_DATA,
  BLADE_ON,
  BLADE_OFF,
  IDEL_MODE,
  TRASPORT_MODE,
  OPERATION_MODE,
  SHUT_DOWN,
  HEART_BEAT,
  num_commands
} commands;

typedef enum gpsStates{
	GPS_INBOUNDS,
	GPS_OUTBOUNDS,
	GPS_ERROR
};

typedef enum motionStates{
	READY,
	NOTLEVEL,
	RECENTMOTION
};

typedef enum robotStates{
	DISABLE,
	ARMED,
	ROBOT_ERROR
};

typedef enum batteryStates{
	OK,
	NOTOK,
	BATTERY_ERROR
};

struct Coord{
  float latitude;
  float longitude;
};