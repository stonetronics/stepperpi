build: clean bin
	g++ main.cpp parser.cpp stepper.cpp -o bin/stepperpi -lpthread -lbcm2835

bin:
	mkdir bin

clean:
	rm -f bin/*
	rmdir bin