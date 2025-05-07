#include "stepper.h"
#include <bcm2835.h>
#include <iostream>
using namespace std;

Stepper::Stepper( int pin_dir, int pin_step, int pin_nsleep, int pin_nreset, int pin_nenable, int pin_start_endstop, int pin_end_endstop) {
    this -> pin_dir = pin_dir;
    this -> pin_step = pin_step;
    this -> pin_nsleep = pin_nsleep;
    this -> pin_nreset = pin_nreset;
    this -> pin_nenable = pin_nenable;
    this -> pin_start_endstop = pin_start_endstop;
    this -> pin_end_endstop = pin_end_endstop;
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

int Stepper::getToggleIntervalMillis( float steps_per_second ) {
    int toggle_interval_millis = (int)(1000.0 / (2.0*steps_per_second) );
    cout << "[stepper] calculated toggle interval " << toggle_interval_millis << " ms" << endl;
    return toggle_interval_millis;
}

float Stepper::getStepsPerSecond( int toggle_interval_millis) {
    float steps_per_second = 1000.0/(((float)(2.0*toggle_interval_millis)));
    return steps_per_second;    
}

void Stepper::hardstep(int direction, int steps, int toggle_interval_millis) {
    if (direction) {
        cout << "[stepper] setting direction clockwise (end to start)" << endl;
        bcm2835_gpio_write(this->pin_dir, HIGH);
    } else {
        cout << "[stepper] setting direction counterclockwise (start to end)" << endl;
        bcm2835_gpio_write(this->pin_dir, LOW);
    }

    cout << "[stepper] hard-stepping " << steps << " steps with " << Stepper::getStepsPerSecond(toggle_interval_millis) << " steps per second" << endl;
    cout << "[stepper] endstop switches are ignored!" << endl;

    for (int stepno=0; stepno < steps; stepno++) {
        bcm2835_gpio_write(this->pin_step, LOW);
        bcm2835_delay(toggle_interval_millis); //dleay in ms
        bcm2835_gpio_write(this->pin_step, HIGH);
        bcm2835_delay(toggle_interval_millis); //dleay in ms
    }

    bcm2835_gpio_write(this->pin_step, LOW);

    cout << "[stepper] stepping done!" << endl;
}

int Stepper::step(int direction, int steps, int toggle_interval_millis) {
    int pin_endstop;

    if (direction) {
        cout << "[stepper] setting direction clockwise (end to start)" << endl;
        bcm2835_gpio_write(this->pin_dir, HIGH);
        pin_endstop = this->pin_start_endstop;
    } else {
        cout << "[stepper] setting direction counterclockwise (start to end)" << endl;
        bcm2835_gpio_write(this->pin_dir, LOW);
        pin_endstop = this->pin_end_endstop;
    }

    cout << "[stepper] stepping " << steps << " steps with " << Stepper::getStepsPerSecond(toggle_interval_millis) << " steps per second" << endl;
    cout << "[stepper] endstop switches are obeyed!" << endl;

    int stepno;
    for (stepno=0; stepno < steps; stepno++) {
        if (bcm2835_gpio_lev(pin_endstop) == LOW) { //if the enddstop in the direction we want to rotate is pushed, stop stepping
            break;
        }
        bcm2835_gpio_write(this->pin_step, LOW);
        bcm2835_delay(toggle_interval_millis); //dleay in ms
        bcm2835_gpio_write(this->pin_step, HIGH);
        bcm2835_delay(toggle_interval_millis); //dleay in ms
    }

    bcm2835_gpio_write(this->pin_step, LOW);

    if (stepno == steps) {
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