#define NO_STEPPERS             1

// GPIO connections from the schematic
#define STEPPER1_DIR        25
#define STEPPER1_STEP       24
#define STEPPER1_nSLEEP     23
#define STEPPER1_nRESET     22
#define STEPPER1_nENABLE    27
#define STEPPER1_START_ES   17
#define STEPPER1_END_ES     18

#define STEPPER2_DIR        26
#define STEPPER2_STEP       13
#define STEPPER2_nSLEEP     12
#define STEPPER2_nRESET     6
#define STEPPER2_nENABLE    5
#define STEPPER2_START_ES   0
#define STEPPER2_END_ES     1

//define GPIO connections which are used in the program
#define A4988_DIR       STEPPER1_DIR
#define A4988_STEP      STEPPER1_STEP
#define A4988_nSLEEP    STEPPER1_nSLEEP
#define A4988_nRESET    STEPPER1_nRESET
//#define SET_MICROSTEPS_PINS
#ifdef SET_MICROSTEPS_PINS
 #define A4988_MS3       11
 #define A4988_MS2       0
 #define A4988_MS1       5
#endif
#define A4988_nENABLE   STEPPER1_nENABLE
#define ENDSTOP_START   STEPPER1_START_ES
#define ENDSTOP_STOP    STEPPER1_END_ES


//set GPIO connections for LEDs
#define LED1_PIN    2
#define LED2_PIN    3


//set GPIO connections for rotary encoder
#define LIGHTBARRIER_A      STEPPER2_START_ES
#define LIGHTBARRIER_B      STEPPER2_END_ES


//set GPIO connections for manual mode panel
#define MANUAL_ENABLE   STEPPER2_STEP
#define MANUAL_LEFT     STEPPER2_nSLEEP
#define MANUAL_RIGHT    STEPPER2_nRESET
#define MANUAL_LED      STEPPER2_DIR