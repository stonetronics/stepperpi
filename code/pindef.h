#define NO_STEPPERS             1
//define GPIO connections for first stepper
#define A4988_DIR       25
#define A4988_STEP      24
#define A4988_nSLEEP    23
#define A4988_nRESET    22
//#define SET_MICROSTEPS_PINS
#ifdef SET_MICROSTEPS_PINS
 #define A4988_MS3       11
 #define A4988_MS2       0
 #define A4988_MS1       5
#endif
#define A4988_nENABLE   27
#define ENDSTOP_START   17
#define ENDSTOP_STOP    18


//set GPIO connections for LEDs
#define LED1_PIN    2
#define LED2_PIN    3


//set GPIO connections for rotary encoder
#define LIGHTBARRIER_A      0
#define LIGHTBARRIER_B      1