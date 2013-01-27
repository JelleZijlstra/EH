EHCFLAGS = -v
INTERPRETED_COMPILER = ehi ehc.eh
SOURCE = ehc.eh
TARGET = ehc

all: ehc

install: ehc
	sudo cp ehc /usr/bin/ehc

ehc: ehc.eh
	$(INTERPRETED_COMPILER) $(EHCFLAGS) $(SOURCE) -o $(TARGET)

ehc.eh: compiler.eh preprocessor.eh
