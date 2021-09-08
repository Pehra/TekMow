//Command List
typedef enum {
  NULL_COMM,
  FORWARD,
  BACKWARD,
  LEFT,
  RIGHT,
  STOP,
  JOY_DRIVE,
	
  SET_COORD,
  HEART_BEAT,
  ECHO,

  
  ECHO_RESPONSE,
  GPS_RESPONSE,
  SENSOR_RESPONSE,
  
  COMM_ARM,
  COMM_DISABLE,
  COMM_MOW,
  
  
  DUMP_VARS,
  READ_DATA,
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
	ROBOT_DISABLE,
	ROBOT_ARMED,
	ROBOT_MOW,
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

