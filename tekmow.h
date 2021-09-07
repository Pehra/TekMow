//Command List
typedef enum {
  NULL_COMM,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  STOP,
  SET_COORD,
  HEART_BEAT,
  ECHO,
  ECHO_RESPONSE,
  GPS_RESPONSE,
  JOY_DRIVE,
  
  DUMP_VARS,
  READ_DATA,
  BLADE_ON,
  BLADE_OFF,
  IDLE_MODE,
  TRANSPORT_MODE,
  OPERATION_MODE,
  SHUT_DOWN,
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
	MOW,
	ROBOT_ERROR
};

typedef enum commStates{
	READ,
	SEND_GPS,
	COMM_ECHO,
	SEND_ERROR
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

