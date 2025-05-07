#ifndef __PARSER_H__
#define __PARSER_H__

/*
COMMANDS:

Command word followed by parameters, seperated by spaces

RESET                                                               // reset driver
ENABLE                                                              // enable driver
DISABLE                                                             // disable driver
MOVE        <direction>:[0,1]   <steps>[int]     <speed>[float]     // move <steps> into <direction> at <speed>, respecting the endstop in the direction that is requested
HARDMOVE    <direction>:[0,1]   <steps>[int]     <speed>[float]     // as move, but ignore end stops
DEBUG                                                               // give debug information (which endstops are pushed)
*/

typedef enum {
    RESET, ENABLE, DISABLE, MOVE, HARDMOVE, DEBUG
} Stepper_Command_Name;

typedef struct {
    Stepper_Command_Name    type;
    int                     direction;
    int                     steps;
    float                   speed;
} Stepper_Command;

/* print a full stepper command*/
void print_stepper_command(Stepper_Command thecommand);

/* parse a command from a string and save it to "thecommand", returns 0 on success, 1 on fail*/
int parse_stepper_command(char* toparse, Stepper_Command* thecommand);


#endif