#ifndef ENCODER_H
#define ENCODER_H

#include <stdbool.h>
#include <stdint.h>

#include <pthread.h>


class Encoder {
    public:
        Encoder( int pin_a, int pin_b);
        Encoder( int pin_a, int pin_b, bool reverse_direction);

        void init( void );
        void process( void ); //process the inputs and count if necessary
        void resetCounters( void ); //reset rotation&illegal counter
        void resetRotation( void );
        void resetIllegal( void );

        int getRotationCount( void ); //get counts of rotation
        int getIllegalCount( void ); //get counts of illegal state transfers (both encoders flipped at the same time)
    private:
        int pin_a;
        uint8_t pin_a_level;
        uint8_t pin_a_level_before;
        int pin_b;
        uint8_t pin_b_level;
        uint8_t pin_b_level_before;

        int rotationCount;
        int illegalCount;
        int directionMultiplier;

        pthread_mutex_t lock; //mutex for threadsafety
};

#endif