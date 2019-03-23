CC = gcc
INC = 20171697.h
CFLAGS = -g -c
TARGET = 20171697.out
OBJECTS = 20171697.o
SRCS = 20171697.c

$(TARGET) : $(OBJECTS)
			  $(CC) -o $(TARGET) $(OBJECTS)
$(OBJECTS) : $(INC) $(SRCS)
			  $(CC) $(CFLAGS) -o $(OBJECTS) $(SRCS)
clean:
	-rm -f $(OBJECTS) $(TARGET)
