run:
	./tester

compile: tester.eh
	ehc tester.eh -o tester

install: compile
	sudo cp tester /opt/local/bin
	sudo cp replacetest.sh /opt/local/bin
	sudo cp addtestfile.sh /opt/local/bin
