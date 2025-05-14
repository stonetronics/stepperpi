#ifndef STEPPER_H
#define STEPPER_H

#include <stdbool.h>
#include <pthread.h>


#define STEPPER_CW      1  //end to start
#define STEPPER_CCW     0  //start to end

#define STEPPER_STANDARD_ACCELERATION  100.0  // steps/s/s

class Stepper {
    public:
        Stepper( int pin_dir, int pin_step, int pin_nsleep, int pin_nreset, int pin_nenable, int pin_start_endstop, int pin_end_endstop);
        Stepper( int pin_dir, int pin_step, int pin_nsleep, int pin_nreset, int pin_nenable, int pin_start_endstop, int pin_end_endstop, float standard_acceleration);

        void init( void );
        void enable( bool enabled );
        bool isEnabled( void );
        int step( int direction, int steps, float steprate);   // step only when the endstop in the desired direction (0 = start to end, 1 = end to start) is not triggered; returns 0 if not run into endstop, 1 otherwise
        int step( int direction, int steps, float steprate, float acceleration);   // step only when the endstop in the desired direction (0 = start to end, 1 = end to start) is not triggered; returns 0 if not run into endstop, 1 otherwise
        int step( int direction, int steps, float steprate, float acceleration, bool obey_endstops);   // step only when the endstop in the desired direction (0 = start to end, 1 = end to start) is not triggered; returns 0 if not run into endstop, 1 otherwise
        void reset( void );
        void sleep( bool sleep );
        bool isSleeping( void );
        int debug( void );  // write debug message      
        void pulse_step(float microseconds); //pulse the step pin for a number of microseconds
        
    private:
        int pin_dir;
        int pin_step;
        int pin_nsleep;
        int pin_nreset;
        int pin_nenable;
        int pin_start_endstop;
        int pin_end_endstop;
        float standard_acceleration;
        
        void pulse_step_unsafe(float microseconds); //pulse the step pin for a number of microseconds

        pthread_mutex_t lock;
};

#endif