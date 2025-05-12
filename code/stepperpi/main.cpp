/*

Multi-threaded application that receives commands for a stepper motor on a socket and executes the desired stepper steps rotation woohoo

build command: g++ *.cpp -o main -lpthread -lbcm2835


*/


#include <cstring>
#include <iostream>
using namespace std;
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include "CircBuffer.h"

/*
uses the bcm2835 library as found on https://www.airspayce.com/mikem/bcm2835/index.html, version 1.75 at the time of coding this
download and install beforehand
*/
#include <bcm2835.h>
#include "stepper.h"
#include "encoder.h"
#include "parser.h"

#include "pindef.h"


#define COMMAND_BUFFER_SIZE     128 //number of commmands that can be stored at once

#define TRANSMIT_MSG_SIZE       256 //transmit message maximum string length
#define TRANSMIT_BUFFER_SIZE    128 // number of transmit messages that can be stored at once

#define SERVER_PORT     8000

/*
GENERAL APPLICATION RUN & EXIT HANDLING
*/
volatile int running;

void my_handler(int s){
    cout << "Caught signal " << s << endl;
    cout << "stopping program..." << endl;
    // signal the stop to all threads
    running = 0;
}

// type that stores transmission strings
typedef struct {
    char msg[TRANSMIT_MSG_SIZE];
} Transmit_Msg;

/* all server info */
typedef struct {
    pthread_t   receive_thread_id;                  // receiver thread id
    pthread_t   transmit_thread_id;                 // transmission thread id
    pthread_t   stepper_thread_id;                  // stepper handling thread id
    pthread_t   encoder_thread_id;                  // encoder thread id
    int server_port;                                // port to listen on
    int server_socket;                              // file descriptor of server socket
    int client_socket;                              // file descriptor of the client socket
    CircBuffer<Transmit_Msg>* transmit_buffer;      // buffer where transmissions are saved
    CircBuffer<Stepper_Command>* command_buffer;    // stepper command buffer
    Stepper* stepper;                               // the stepper
    Encoder* encoder;                               // the encoder
    pthread_mutex_t client_lock;                    // lock for the client server
} Server_Info;


/*socket open indicator*/
volatile int socket_open;

/*void* debug_transmitter(void* param) {
    Server_Info* si = (Server_Info*) param;
    Transmit_Msg msg;
    int counter = 0;

    while (running && socket_open) {
        sleep(3); //send something every three minutes
        sprintf(msg.msg, "Debug message number %d", counter++);
        si->transmit_buffer->write(msg);
    }

    return (void*) "Happy Debugging";
}*/

/*transmit handling*/
void* transmit_thread(void* param) {
    Server_Info* si = (Server_Info*) param;

    Transmit_Msg msg;

    cout << "[transmit] transmission thread started!" << endl;

    while (socket_open && running) {
        if (!(si->transmit_buffer->empty()) ){
            msg = si->transmit_buffer->read();
            cout << "[transmit] message in buffer: " << msg.msg << endl;
            cout << "[transmit] sending ... " << endl;
            //pthread_mutex_lock(&(si->client_lock));
            send(si->client_socket, msg.msg, strlen(msg.msg), 0);
            //pthread_mutex_unlock(&(si->client_lock));
        }
    }

    cout << "[transmit] ending transmission thread!" << endl;

    return (void* ) "no error";

}


/*Receive handling*/
void* receive_thread(void* param) {
    Server_Info* si = (Server_Info*) param; // get server infos

    char buffer[1024]; // receive buffer
    int msglen;        // length of received message

    cout << "[receive] started receive thread" << endl;

    while (socket_open && running) {
        cout << "[receive] try to receive" << endl;
        //pthread_mutex_lock(&(si->client_lock));
        msglen = recv(si->client_socket, buffer, sizeof(buffer), 0);
        //pthread_mutex_unlock(&(si->client_lock));
        if (msglen>0) {
            cout << "[receive] Message from client: ";
            buffer[msglen] = '\0';
            for(int i =0; i < msglen; i++) {
                cout << buffer[i];
            }
            cout << endl;
            /*do something with the message*/
            Stepper_Command tmp_command;
            if ( !parse_stepper_command(buffer, &tmp_command) ) {
                cout << "[receive] found command: ";
                print_stepper_command(tmp_command);
                cout << endl;
                cout << "[receive] feeding queue: ";
                if ( si->command_buffer->write(tmp_command) ) {
                    cout << "[receive] OK" << endl;
                } else {
                    cout << "[receive] buffer not empty!" << endl;
                }
            } else {
                cout << "[receive] could not process command!" << endl;
            }

            /*
            Transmit_Msg msg;
            strcpy(msg.msg, buffer);
            si->transmit_buffer->write(msg);*/
        } else {
            cout << "[receive] msglen read: " << msglen << endl;
            if (msglen == 0) {
                cout << "[receive] Client disconnected! stopping thread" << endl;
                socket_open = 0;
            }
        }
    }

    cout << "[receive] end of receive thread" << endl;

    return (void*) "no error";
}

/*seperate thread for encoder*/
void* encoder_thread(void* param) {
    Server_Info* si = (Server_Info*) param; // get server infos
    cout << "[encoderthread] encoder thread started!" << endl;
    while (running) {
        si->encoder->process();
    }

    return NULL;
}

/*Handle single stepper commands*/
void* stepper_thread(void* param) {
    Server_Info* si = (Server_Info*) param; // get server infos

    cout << "[stepperthread] stepper thread started!" << endl;

    int toggle_interval_millis;

    int rotation_count_before;
    int illegal_count_before;
    int rotation_count_after;   //to store encoder counts before and after movement - to check wether the motor moves for real
    int illegal_count_after;   

    int stepper_retval;
    Transmit_Msg msg;  //message to return

    while (running) {
        if (!si->command_buffer->empty()) {
            cout << "[stepperthread] new command in queue. processing ..." << endl;
            Stepper_Command tmp_command = si->command_buffer->read();
            switch (tmp_command.type) {
                case RESET:
                    cout << "[stepperthread] resetting hardware" << endl;
                    strcpy(msg.msg, "resetting hardware");
                    si->transmit_buffer->write(msg);
                    si->stepper->reset();
                    break;

                case ENABLE:
                    cout << "[stepperthread] enabling hardware" << endl;
                    strcpy(msg.msg, "enabling hardware");
                    si->transmit_buffer->write(msg);
                    si->stepper->enable(true);
                    si->stepper->sleep(false);
                    break;
                    
                case DISABLE:
                    cout << "[stepperthread] disabling hardware" << endl;
                    strcpy(msg.msg, "disabling hardware");
                    si->transmit_buffer->write(msg);
                    si->stepper->enable(false);
                    si->stepper->sleep(true);
                    break;

                case MOVE:
                    cout << "[stepperthread] stepping " << tmp_command.steps << " steps in direction" << tmp_command.direction << endl;
                    cout << "[stepperthread] endswitches are obeyed!" << endl;

                    // get encoder values before
                    rotation_count_before = si->encoder->getRotationCount();
                    illegal_count_before = si->encoder->getIllegalCount();

                    //do the stepping
                    if (tmp_command.acceleration <= 0) {
                        sprintf(msg.msg, "stepping %d steps in direction %d, requested speed: %.2f steps/s; default acceleration; endswitches are obeyed!", tmp_command.steps, tmp_command.direction, tmp_command.speed);
                        si->transmit_buffer->write(msg);
                        stepper_retval = si->stepper->step(tmp_command.direction, tmp_command.steps, tmp_command.speed);
                    } else {
                        sprintf(msg.msg, "stepping %d steps in direction %d, requested speed: %.2f steps/s; acceleration: %.2f steps/s/s; endswitches are obeyed!", tmp_command.steps, tmp_command.direction, tmp_command.speed, tmp_command.acceleration);
                        si->transmit_buffer->write(msg);
                        stepper_retval = si->stepper->step(tmp_command.direction, tmp_command.steps, tmp_command.speed, tmp_command.acceleration);
                    }
                    //report

                    // get encoder values after
                    rotation_count_after = si->encoder->getRotationCount();
                    illegal_count_after = si->encoder->getIllegalCount();

                    cout << "[stepperthread] rotation count: " << rotation_count_after << "; illegal count: " << illegal_count_after << ";" << endl;
                    if ( (rotation_count_after == rotation_count_before) && (illegal_count_after == illegal_count_before) ) {
                        cout << "[stepperthread] encoder did not recognize any movement! check motor/encoder - maybe hardware is not enabled?" << endl;
                        sprintf(msg.msg, "Warning: encoder did not recognize any movement! rotation count: %d; illegal count: %d", rotation_count_after, illegal_count_after);
                        si->transmit_buffer->write(msg);
                    } else {
                        sprintf(msg.msg, "Encoder recognized movement! rotation count: %d; illegal count: %d", rotation_count_after, illegal_count_after);
                        si->transmit_buffer->write(msg);
                    }


                    if (!stepper_retval) {
                        cout << "[stepperthread] stepping finished without endstops hitting!" << endl;
                        strcpy(msg.msg, "stepping finished without endstops hitting!");
                        si->transmit_buffer->write(msg);
                    } else {
                        cout << "[stepperthread] stepped into one of the endstops!" << endl;
                        strcpy(msg.msg, "stepped into one of the endstops!");
                        si->transmit_buffer->write(msg);
                        
                    }
                    break;

                case HARDMOVE:
                    if (tmp_command.acceleration <= 0) {
                        cout << "[stepperthread] cannot hardstep, missing acceleration!" << endl;
                        strcpy(msg.msg, "cannot hardstep, missing acceleration!");
                        si->transmit_buffer->write(msg);
                    } else {
                        cout << "[stepperthread] stepping " << tmp_command.steps << " steps in direction" << tmp_command.direction << endl;
                        cout << "[stepperthread] endswitches are !!!NOT!!! obeyed!" << endl;
                        sprintf(msg.msg, "stepping %d steps in direction %d, requested speed: %.2f steps/s; acceleration: %.2f steps/s/s; endswitches are !!!NOT!!! obeyed!", tmp_command.steps, tmp_command.direction, tmp_command.speed, tmp_command.acceleration);
                        si->transmit_buffer->write(msg);
                        si->stepper->step(tmp_command.direction, tmp_command.steps, tmp_command.speed, tmp_command.acceleration, false);
                        cout << "[stepperthread] finished stepping!" << endl;
                        strcpy(msg.msg, "finished stepping!");
                        si->transmit_buffer->write(msg);
                    }
                    break;

                case ENCODER_RESET:
                    si->encoder->resetCounters();
                    cout << "[stepperthread] reset encoder values!" << endl;
                    strcpy(msg.msg,  "reset encoder values!");
                    si->transmit_buffer->write(msg);
                    break;
                    
                case ENCODER_GET:
                    rotation_count_before = si->encoder->getRotationCount(); //reuse variables, the "_before" doesnt mean anything here
                    illegal_count_before = si->encoder->getIllegalCount();
                    cout << "[stepperthread] got rotation count: " << rotation_count_before << "; and illegal count: " << illegal_count_before << ";" << endl;

                    sprintf(msg.msg, "rotation count:%d; illegal count:%d;", rotation_count_before, illegal_count_before);
                    si->transmit_buffer->write(msg);
                    break;

                case DEBUG:
                    stepper_retval = si->stepper->debug();

                    sprintf(msg.msg, "DEBUG; DIR:%d, STEP:%d, nSLEEP:%d, nRESET:%d, nENABLE:%d, START_ENDSTOP:%d, END_ENDSTOP:%d, rotation count:%d, illegal count:%d", (stepper_retval&(1<<6))>>6, (stepper_retval&(1<<5))>>5, (stepper_retval&(1<<4))>>4, (stepper_retval&(1<<3))>>3, (stepper_retval&(1<<2))>>2,(stepper_retval&(1<<1))>>1, (stepper_retval&(1<<0))>>0, si->encoder->getRotationCount(), si->encoder->getIllegalCount());
                    si->transmit_buffer->write(msg);
                    break;

                default:
                    cout << "[stepperthread] can not process command! going to next command" << endl;
                    break;
            }
            if (si->command_buffer->empty() ) {
                cout << "[stepperthread] no commands left!" << endl;
            }
        }
    }

    cout << "[stepperthread] stepper thread ended!" << endl;

    return NULL;
}

/*set up the server socket*/
void setup_socket(Server_Info* ti) {

    // socket setup
    cout << "[SETUP_SOCKET] creating socket" << endl;
    // creating socket
    ti->server_socket = socket(AF_INET, SOCK_STREAM, 0);
    cout << "[SETUP_SOCKET] created socked with fd: " << ti->server_socket << endl;

    // specifying the address
    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(ti->server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // binding socket.
    if (bind(ti->server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        cout << "[SETUP_SOCKET] bind error" << endl;
        exit(1);
    }

    // listening to the assigned socket
    if (listen(ti->server_socket, 1) == -1){
        cout << "[SETUP_SOCKET] listen error" << endl;
        exit(1);
    }

    cout << "[SETUP_SOCKET] socket is set up!" << endl;

    return;
}

int main()
{
    running = 1;

    // strg+c handling
    struct sigaction sigIntHandler;
    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;
    sigaction(SIGINT, &sigIntHandler, NULL);

    // set up socket
    Server_Info si;
    si.server_port = SERVER_PORT;
    setup_socket(&si);

    // setup transmission buffer
    CircBuffer<Transmit_Msg> transmit_buffer(TRANSMIT_BUFFER_SIZE);
    si.transmit_buffer = &transmit_buffer;
    
    // setup stepper command buffer
    CircBuffer<Stepper_Command> command_buffer(COMMAND_BUFFER_SIZE);
    si.command_buffer = &command_buffer;

    // setup stepper module
    Stepper stepper(A4988_DIR, A4988_STEP, A4988_nSLEEP, A4988_nRESET, A4988_nENABLE, ENDSTOP_START, ENDSTOP_STOP);

    // setup encoder module
    Encoder encoder(LIGHTBARRIER_A, LIGHTBARRIER_B);

    //init gpio lib
    if (!bcm2835_init()) {
        cout << "Unable to init GPIO." << endl;
        return 1;
    }

    //set configuration - to be hardwired
    //no microstepping
    #ifdef SET_MICROSTEPS_PINS
        bcm2835_gpio_fsel(A4988_MS1, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(A4988_MS2, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_fsel(A4988_MS3, BCM2835_GPIO_FSEL_OUTP);
        bcm2835_gpio_write(A4988_MS1, LOW);
        bcm2835_gpio_write(A4988_MS2, LOW);
        bcm2835_gpio_write(A4988_MS3, LOW);
    #endif

    // init led pins, keep them off at first
    bcm2835_gpio_fsel(LED1_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_fsel(LED2_PIN, BCM2835_GPIO_FSEL_OUTP);
    bcm2835_gpio_write(LED1_PIN, LOW);
    bcm2835_gpio_write(LED2_PIN, LOW);


    //init stepper
    stepper.init();
    si.stepper = &stepper; //save to server info

    //init encoder
    encoder.init();
    si.encoder = &encoder; //save to server info

    // set up mutex for threadsafety of the client server
    if (pthread_mutex_init(&(si.client_lock), NULL)!= 0) {
        cout << "[main] client server mutex initialization failed! " << endl;
    }

    // set up stepper thread
    pthread_create(&(si.stepper_thread_id), NULL, &stepper_thread, &si);

    // set up encoder thread
    pthread_create(&(si.encoder_thread_id), NULL, &encoder_thread, &si);

    cout << "[main] DEBUG: " << si.server_port << "; " << si.server_socket << endl;

    struct sockaddr_in client; // for recording the client infos
    unsigned int namelen;

    // return values
    void* receive_thread_return;
    void* transmit_thread_return;
    void* stepper_thread_return;
    void* encoder_thread_return;

    while (running) {
        cout << "[main] Accepting connection on port " << si.server_port << "  with FD " << si.server_socket << endl;
        //signal that the server accepts connections on LED1
        bcm2835_gpio_write(LED1_PIN, HIGH);

        // get the lenght of the client info
        namelen = sizeof(client);
        
        // accept connection (sleep until a connection is made)
        if ( (si.client_socket = accept(si.server_socket, (struct sockaddr*) &client, &namelen )) == -1 ){
            cout << "[main] error in accepting client connection: " << si.client_socket << endl;
            cout << "[main] exiting main loop!" << endl;
            break;
        } else {
            cout << "[main] accepted client connection" << endl;
            // signal that a connection was accepted on LED2
            bcm2835_gpio_write(LED2_PIN, HIGH);
        }

        socket_open = 1;
        //reset the transmit buffer before starting threads
        si.transmit_buffer->reset();
        //with the accepted connection, start the receiving and transmitting thread
        pthread_create(&(si.receive_thread_id), NULL, &receive_thread, &si);
        pthread_create(&(si.transmit_thread_id), NULL, &transmit_thread, &si);


        /* for debug
        pthread_t debug_thread_id;
        pthread_create(&debug_thread_id, NULL, &debug_transmitter, &si);
        void* debug_thread_return;
        pthread_join(debug_thread_id, &debug_thread_return);
        cout << "[main] debg thread stopped, exited with return: " << (char*) debug_thread_return << endl;*/

        // wait for transmission and reception thread to finish
        pthread_join(si.receive_thread_id, &receive_thread_return);
        cout << "[main] receiver thread stopped, exited with return: " << (char*) receive_thread_return << endl;
        pthread_join(si.transmit_thread_id, &transmit_thread_return);
        cout << "[main] transmission thread stopped, exited with return: " << (char*) transmit_thread_return << endl;
        
        //signal that no accepting is done right now and no connection is given on the socket
        bcm2835_gpio_write(LED1_PIN, LOW);
        bcm2835_gpio_write(LED2_PIN, LOW);


        sleep(1); // sleep 1 second before accepting connections again
    }
    pthread_join(si.stepper_thread_id, &stepper_thread_return);
    pthread_join(si.encoder_thread_id, &encoder_thread_return);

    close(si.client_socket);
    // shutdown socket
    shutdown(si.server_socket, SHUT_RDWR);
    close(si.server_socket);

    // release stepper control
    stepper.sleep(true);
    stepper.enable(false);

    //make sure no false indication is on the LEDs
    bcm2835_gpio_write(LED1_PIN, LOW);
    bcm2835_gpio_write(LED2_PIN, LOW);

    return 0;
}

