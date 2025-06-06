# stepperpi
control of one stepper motor (NEMA17) and measurement by rotary encoder with a raspberry pi through a ethernet interface  

## pcb
The pcb folder contains a PCB that fits on to a regular raspberry pi (drawn in KiCAD 9.0). It holds two A4988 stepper motor drivers complete with JST connector for a NEMA17 motor and two JST connectors for endswitches, all following common pin layouts as they are used with 3D printers. Options for microstepping are set through a solder jumper.  
Each hardware pin connection of both the stepper driver and endswitches is connected to the raspberry pi through a solder jumper and can be enabled/disabled as needed.  
An additional DC/DC converter is present on the board. It steps down the input voltage to the 5V needed by the raspberry pi, eliminating the need for a seperate USB power supply. The connection to the DC/DC converter is optional and can be enabled through a jumper.  

## code
In the code directory, three subdirectories exist. One holds a jupyter notebook for the analysis and development of the stepping algorithm and the other one holds a c++ project for the control of the stepper motor on the raspberry pi through a socket interface. Another directory holds a client written in python which connects to the raspberry pi's socket and enables the user to send and receive string commands and messages. Please refer to the respective readme for further info.  