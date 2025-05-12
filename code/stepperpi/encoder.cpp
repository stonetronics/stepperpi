#include "encoder.h"
#include <bcm2835.h>
#include <iostream>
using namespace std;

//#define COUT_ON_GETTER

Encoder::Encoder( int pin_a, int pin_b)
    : Encoder(pin_a, pin_b, false) {
}

Encoder::Encoder(int pin_a, int pin_b, bool reverse_direction) {
    this->pin_a = pin_a;
    this->pin_b = pin_b;
    if (reverse_direction)
        this->directionMultiplier = -1;
    else
        this->directionMultiplier = 1;
}

void Encoder::init( void ) {
    //init IOs
    bcm2835_gpio_fsel(this->pin_a, BCM2835_GPIO_FSEL_INPT);
    bcm2835_gpio_fsel(this->pin_b, BCM2835_GPIO_FSEL_INPT); 

    //read IOs for the first time
    this->pin_a_level_before = bcm2835_gpio_lev(this->pin_a);
    this->pin_b_level_before = bcm2835_gpio_lev(this->pin_b);

    //initialize counters
    this->resetCounters();

    if (pthread_mutex_init(&(this->lock), NULL)!= 0) 
    {
        cout << "[encoder]  mutex initialization failed! " << endl;
    }

    cout << "[encoder] initialized!" << endl;
}

void Encoder::process( void ){
    // record current pin levels
    this->pin_a_level = bcm2835_gpio_lev(this->pin_a);
    this->pin_b_level = bcm2835_gpio_lev(this->pin_b);

    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    // check for illegal transition (both pin levels change at the same time)
    if ( (this->pin_a_level != this->pin_a_level_before) && (this->pin_b_level != this->pin_b_level_before) )
        this->illegalCount++;
    else { // no illegal transition ->check which direction is the step and count accordingly
        if (this->pin_a_level != this->pin_a_level_before) { // edge on a
            if (this->pin_a_level) { //positive edge on a
                if (this->pin_b_level) //b high -> lefthand rotation (+)
                    this->rotationCount += this->directionMultiplier;
                else //b low -> righthand rotation (-)
                    this->rotationCount -= this->directionMultiplier;
            } else { //negative edge on a
                if (this->pin_b_level) //b high -> righthand rotation (-)
                    this->rotationCount -= this->directionMultiplier;
                else //b low -> lefthand rotation (+)
                    this->rotationCount += this->directionMultiplier;
            }
        } else if (this->pin_b_level != this->pin_b_level_before) { //edge on b
            if (this->pin_b_level) { //positive edge on b
                if (this->pin_a_level) //a high -> righthand rotation (-)
                    this->rotationCount -= this->directionMultiplier;
                else //a low -> lefthand rotation (+)
                    this->rotationCount += this->directionMultiplier;
            } else { //negative edge on b
                if (this->pin_a_level) //a high -> lefthand rotation (+)
                    this->rotationCount += this->directionMultiplier;
                else //a low -> righthand rotation (-)
                    this->rotationCount -= this->directionMultiplier;
            }
        }
    }
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again


    // save the levels for next execution
    this->pin_a_level_before = this->pin_a_level;
    this->pin_b_level_before = this->pin_b_level;    
}

void Encoder::resetCounters( void ) {
    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    this->rotationCount=0;
    this->illegalCount=0;
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
    cout << "[encoder] reset counters!" << endl;
}

void Encoder::resetRotation( void ) {
    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    this->rotationCount=0;
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
    cout << "[encoder] reset rotation count!" << endl;
}

void Encoder::resetIllegal( void ) {
    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    this->illegalCount=0;
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
    cout << "[encoder] reset illegal count!" << endl;
}

int Encoder::getRotationCount( void ) {
    #ifdef COUT_ON_GETTER
    cout << "[encoder] rotation count " << this->rotationCount << ";" << endl; 
    #endif
    return this->rotationCount;
}

int Encoder::getIllegalCount( void ) {
    #ifdef COUT_ON_GETTER
    cout << "[encoder] illegal count " << this->illegalCount << ";" << endl; 
    #endif
    return this->illegalCount;
}