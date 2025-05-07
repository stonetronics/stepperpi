#include "stepper.h"
#include <bcm2835.h>
#include <iostream>
#include <cmath>
using namespace std;

Stepper::Stepper( int pin_dir, int pin_step, int pin_nsleep, int pin_nreset, int pin_nenable, int pin_start_endstop, int pin_end_endstop) 
    : Stepper( pin_dir, pin_step, pin_nsleep, pin_nreset, pin_nenable, pin_start_endstop, pin_end_endstop, STEPPER_STANDARD_ACCELERATION){
}

Stepper::Stepper( int pin_dir, int pin_step, int pin_nsleep, int pin_nreset, int pin_nenable, int pin_start_endstop, int pin_end_endstop, float standard_acceleration) {
    this -> pin_dir = pin_dir;
    this -> pin_step = pin_step;
    this -> pin_nsleep = pin_nsleep;
    this -> pin_nreset = pin_nreset;
    this -> pin_nenable = pin_nenable;
    this -> pin_start_endstop = pin_start_endstop;
    this -> pin_end_endstop = pin_end_endstop;
    this -> standard_acceleration = standard_acceleration;
}

void Stepper::init(void ) {
    //init IOs
    bcm2835_gpio_fsel(pin_dir, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_step, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_nsleep, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_nreset, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_nenable, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(pin_start_endstop, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(pin_end_endstop, BCM2835_GPIO_FSEL_INPT); 

    //pull driver out of reset, dont sleep
    this->reset();
    this->sleep(false);
    //stepper_enable(true);


    cout << "[stepper] initialized!" << endl;
}

void Stepper::enable(bool enabled) {
    if (enabled) {
        //enable driver
        cout << "[stepper] enable" << endl;
        bcm2835_gpio_write(this->pin_nenable, LOW);
    } else {
        cout << "[stepper] disable" << endl;
        bcm2835_gpio_write(this->pin_nenable, HIGH);
    }
}

int Stepper::step(int direction, int steps, float steprate) {
    return this->step(direction, steps, steprate, this->standard_acceleration, true);
}

int Stepper::step(int direction, int steps, float steprate, float acceleration) {
    return this->step(direction, steps, steprate, acceleration, true);
}

int Stepper::step(int direction, int steps, float steprate, float acceleration, bool obey_endstops) {
    int pin_endstop;
    int stepno = 0;

    if (direction) {
        cout << "[stepper] setting direction clockwise (end to start)" << endl;
        bcm2835_gpio_write(this->pin_dir, HIGH);
        pin_endstop = this->pin_start_endstop;
    } else {
        cout << "[stepper] setting direction counterclockwise (start to end)" << endl;
        bcm2835_gpio_write(this->pin_dir, LOW);
        pin_endstop = this->pin_end_endstop;
    }

    cout << "[stepper] stepping " << steps << " steps with " << steprate << " steps/s, acceleration at " << acceleration << " steps/s/s" << endl;
    cout << "[stepper] endstop switches are obeyed!" << endl;

    //calculate velocity ramps
    int no_ramp_steps = (int) (steprate*steprate /2.0/acceleration); //calculate how many steps the ramp runs
    if (steps < 2*no_ramp_steps) //handle case when full steprate cannot be reached
        no_ramp_steps = steps/2;

    //calculate the length of constant velocity
    int no_const_steps = steps - 2*no_ramp_steps;

    //   ramp up
    int i = 0;
    float step_duration_microseconds;
    while (i < no_ramp_steps) {
        //calculate step duration of the current step
        step_duration_microseconds = (sqrt(2.0*(i+1)/acceleration) - sqrt(2.0*(i)/acceleration)) * 1000000.0;
        if (obey_endstops && (bcm2835_gpio_lev(pin_endstop) == LOW)) //if the enddstop in the direction we want to rotate is pushed, stop stepping
            break;
        this->pulse_step(step_duration_microseconds);
        i++;
        stepno++;
    }

    //   constant velocity
    step_duration_microseconds = 1000000.0/steprate;
    i=1;
    while (i++ < no_const_steps) {
        if (obey_endstops && (bcm2835_gpio_lev(pin_endstop) == LOW)) //if the enddstop in the direction we want to rotate is pushed, stop stepping
            break;
        this->pulse_step(step_duration_microseconds);
        stepno++;
    }


    //   ramp down
    i = no_ramp_steps;
    while (i >= 0) {
        //calculate step duration of the current step
        step_duration_microseconds = (sqrt(2.0*(i+1)/acceleration) - sqrt(2.0*(i)/acceleration)) * 1000000.0;
        if (obey_endstops && (bcm2835_gpio_lev(pin_endstop) == LOW)) //if the enddstop in the direction we want to rotate is pushed, stop stepping
            break;
        this->pulse_step(step_duration_microseconds);
        i--;
        stepno++;
    }

    printf("[DEBUG] stepno: %d\n", stepno);

    bcm2835_gpio_write(this->pin_step, LOW);

    if (--stepno == steps) {
        cout << "[stepper] stepping finished without running into endstop!" << endl;
        return 0;
    } else {
        cout << "[stepper] stepping interrupted by running into endstop!" << endl;
        cout << "[stepper] stepped " << stepno << " of " << steps << " steps" << endl;  
        return 1;
    }
}

void Stepper::reset(void) {
    //enable driver
    cout << "[stepper] resetting stepper driver" << endl;
    bcm2835_gpio_write(this->pin_nreset, LOW);
    bcm2835_delay(10);
    bcm2835_gpio_write(this->pin_nreset, HIGH);
}

void Stepper::sleep(bool sleep) {
    if (sleep) {
        cout << "[stepper] enter sleep" << endl;
        bcm2835_gpio_write(this->pin_nsleep, LOW);
    } else {
        cout << "[stepper] waking up" << endl;
        bcm2835_gpio_write(this->pin_nsleep, HIGH);
    }
}

int Stepper::debug( void ) {
    int lev_dir = bcm2835_gpio_lev(this->pin_dir);
    int lev_step = bcm2835_gpio_lev(this->pin_step);
    int lev_nsleep = bcm2835_gpio_lev(this->pin_nsleep);
    int lev_nreset = bcm2835_gpio_lev(this->pin_nreset);
    int lev_nenable = bcm2835_gpio_lev(this->pin_nenable);
    int lev_start_endstop = bcm2835_gpio_lev(this->pin_start_endstop);
    int lev_end_endstop = bcm2835_gpio_lev(this->pin_end_endstop);

    cout << "[stepper] DEBUG:        PIN  :level " << endl;
    cout << "[stepper]             pin_dir:" << lev_dir << endl;
    cout << "[stepper]            pin_step:" << lev_step << endl;
    cout << "[stepper]          pin_nsleep:" << lev_nsleep << endl;
    cout << "[stepper]          pin_nreset:" << lev_nreset << endl;
    cout << "[stepper]         pin_nenable:" << lev_nenable << endl;
    cout << "[stepper]   pin_start_endstop:" << lev_start_endstop << endl;
    cout << "[stepper]     pin_end_endstop:" << lev_end_endstop << endl;

    return ( (lev_dir<<6) | (lev_step<<5) | (lev_nsleep<<4) | (lev_nreset<<3) | (lev_nenable<<2) | (lev_start_endstop<<1) | (lev_end_endstop<<0) );
}

void Stepper::pulse_step(float microseconds) {
    //calculate toggle intervals
    int toggle_interval1 = (int)(microseconds/2.0);
    int toggle_interval2 = toggle_interval1;
    //approximate the step duration in a better way
    if (microseconds-(toggle_interval1*2.0) > 1) {
        toggle_interval2++;
    }
    bcm2835_gpio_write(this->pin_step, LOW);
    bcm2835_delayMicroseconds(toggle_interval1); //dleay in ms
    bcm2835_gpio_write(this->pin_step, HIGH);
    bcm2835_delayMicroseconds(toggle_interval2); //dleay in ms
}