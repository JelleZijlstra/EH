EHCFLAGS = -v
INTERPRETED_COMPILER = ehi ehc.eh
SOURCE = ehc.eh
TARGET = ehc

all: ehc

install: ehc
	sudo cp ehc /usr/bin/ehc

optimize: ehc.eh
	$(TARGET) $(EHCFLAGS) -O $(SOURCE) -o $(TARGET)

ehc: ehc.eh
	$(INTERPRETED_COMPILER) $(EHCFLAGS) $(SOURCE) -o $(TARGET)

ehc.eh: compiler.eh preprocessor.eh ../ehi/libeh.a
