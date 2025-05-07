#pragma once

#include <pthread.h>

#define DEFAULT_BUFFERSIZE  1024

template <typename T>
class CircBuffer {

    public:
        CircBuffer();
        CircBuffer(unsigned int buffersize);
        ~CircBuffer();

        void reset( void );     // reset buffer
        int write(T to_write);  // write an element, returns 0 on pass
        int empty( void );      // check if the buffer is empty, returns 1 if empty
        T read( void );         // read an element

    private:
        T* buffer;
        unsigned int memsize;
        unsigned int writepointer;
        unsigned int readpointer;
        int unsafe_empty (void); //empty function if called from inside a mutexed method
        pthread_mutex_t lock; // mutex for threadsafety
};


template <typename T>
CircBuffer<T>::CircBuffer() : CircBuffer<T>(DEFAULT_BUFFERSIZE) {
    cout << "[CIRCBUFFER] Default Constructor Called!" << endl;
}

template <typename T>
CircBuffer<T>::CircBuffer(unsigned int buffersize) {
    cout << "[CIRCBUFFER] Constructor called with buffersize: " << buffersize << endl;
    this->memsize = buffersize +1;
    this->buffer = (T*) malloc(sizeof(T)*(memsize));
    this->readpointer = 0;
    this->writepointer = 0;
    if (pthread_mutex_init(&(this->lock), NULL)!= 0) 
    {
        cout << "[CIRCBUFFER]  mutex initialization failed! " << endl;
    }
}

template <typename T>
void CircBuffer<T>::reset( void ) {
    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    this->readpointer = 0;
    this->writepointer = 0;
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
}

template <typename T>
int CircBuffer<T>::write(T to_write) {
    int retval;

    pthread_mutex_lock(&(this->lock)); // lock for thread safety

    if ((this->writepointer +1) % this->memsize == this->readpointer) {
        retval = 1; // buffer is full, return 1
    } else {

        this->buffer[this->writepointer] = to_write;
        this->writepointer = (this->writepointer +1) % this->memsize;
        retval = 0;
    }

    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
    return retval;
}

template <typename T>
int CircBuffer<T>::unsafe_empty( void ) {
    return (this->writepointer == this->readpointer);
}

template <typename T>
int CircBuffer<T>::empty( void ) {
    int retval;
    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    retval = this->unsafe_empty();
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
    return retval;
}

template <typename T> 
T CircBuffer<T>::read( void ) {
    pthread_mutex_lock(&(this->lock)); // lock for thread safety
    T retval = this->buffer[this->readpointer];
    if (!this->unsafe_empty()) { // only increment if the buffer isnt empty, otherwise always the last element should be read
        this->readpointer = (this->readpointer + 1) % this->memsize;
    }
    pthread_mutex_unlock(&(this->lock)); // unlock mutex again
    return retval;
}

template <typename T>
CircBuffer<T>::~CircBuffer() {
    cout << "[CIRCBUFFER] Destructor called" << endl;
    free(this->buffer);
}