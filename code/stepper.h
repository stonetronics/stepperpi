#ifndef STEPPER_H
#define STEPPER_H

#include <stdbool.h>


#define STEPPER_CW      1  //end to start
#define STEPPER_CCW     0  //start to end

class Stepper {
    public:
        Stepper( int pin_dir, int pin_step, int pin_nsleep, int pin_nreset, int pin_nenable, int pin_start_endstop, int pin_end_endstop);

        void init( void );
        void enable( bool enabled );
        static int getToggleIntervalMillis( float steps_per_second );
        static float getStepsPerSecond( int toggle_interval_millis);
        int step( int direction, int steps, int toggle_interval_millis);   // step only when the endstop in the desired direction (0 = start to end, 1 = end to start) is not triggered; returns 0 if not run into endstop, 1 otherwise
        void hardstep( int direction, int steps, int toggle_interval_millis);  // step anyways, even if the endstops are triggered
        void reset( void );
        void sleep( bool sleep );
        int debug( void );  // write debug message      

    private:
        int pin_dir;
        int pin_step;
        int pin_nsleep;
        int pin_nreset;
        int pin_nenable;
        int pin_start_endstop;
        int pin_end_endstop;
};

#endif