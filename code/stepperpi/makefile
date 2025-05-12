build: clean bin
	g++ main.cpp parser.cpp stepper.cpp encoder.cpp -o bin/stepperpi -lpthread -lbcm2835

bin:
	mkdir bin

clean: bin
	rm -f bin/*


install: build
	# copy binary and service definition
	sudo cp -f bin/stepperpi /usr/bin/
	sudo cp ./stepperpi.service /etc/systemd/system/

	# register binary stu
	# sed -i 's#ExecStart=/usr/bin/gse4pi#ExecStart=/usr/bin/gse4pi $(IID)#' /etc/systemd/system/gse4pi.service
	# sed -i 's#User=pi#User=$(USER)#' /etc/systemd/system/gse4pi.service


	#kill stepperpi service if it is running
	(! pidof stepperpi) || sudo systemctl kill stepperpi
	sudo systemctl daemon-reload
	sudo systemctl enable stepperpi
	sudo systemctl restart stepperpi