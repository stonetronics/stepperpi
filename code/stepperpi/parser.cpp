#include <iostream>
using namespace std;
#include <string.h>
#include "parser.h"

void print_stepper_command(Stepper_Command thecommand) {
    cout << "Stepper command name: " << thecommand.type << "; direction: " << thecommand.direction << "; steps: " << thecommand.steps << "; speed: " << thecommand.speed << ";";
}

int parse_stepper_command(char* toparse, Stepper_Command* thecommand) {
    const char* delimiter = " \t";
    char* the_token;
    Stepper_Command retval;


    cout << "[parser] parsing stepper command from string [" << toparse << "]" << endl;

    the_token = strtok(toparse, delimiter);

    if (the_token == NULL) { // exit if nothing is found in the string
        return 1;
    }

    // determine type of command sent
    if ( !strcmp(the_token, "RESET") ) {
        thecommand->type = RESET;
    } else if ( !strcmp(the_token, "ENABLE") ) {
        thecommand->type = ENABLE;
    } else if ( !strcmp(the_token, "DISABLE") ) {
        thecommand->type = DISABLE;
    } else if ( !strcmp(the_token, "MOVE") ) {
        thecommand->type = MOVE;
    } else if ( !strcmp(the_token, "HARDMOVE") ) {
        thecommand->type = HARDMOVE;
    } else if ( !strcmp(the_token, "DEBUG") ) {
        thecommand->type = DEBUG;
    } else {  // didnt recognize command! exit
        return 1;
    }

    //parse the rest of the data if needed
    if ( (thecommand->type == MOVE) or (thecommand->type == HARDMOVE) ){
        if ((the_token = strtok(NULL, delimiter)) != NULL)
            thecommand->direction = atoi(the_token); // get direction
        else // could not parse! exit
            return 1;
        if ((the_token = strtok(NULL, delimiter)) != NULL)
            thecommand->steps = atoi(the_token); // get steps
        else // could not parse! exit
            return 1;
        if ((the_token = strtok(NULL, delimiter)) != NULL)
            thecommand->speed = atof(the_token); // get speed
        else // could not parse! exit
            return 1;
        if ((the_token = strtok(NULL, delimiter)) != NULL)
            thecommand->acceleration = atof(the_token); // get acceleration
        else
            thecommand->acceleration = -1; //no acceleration given
    }

    return 0;
}