build: clean bin
	g++ main.cpp parser.cpp stepper.cpp -o bin/stepperpi -lpthread -lbcm2835

bin:
	mkdir bin

clean: bin
	rm -f bin/*