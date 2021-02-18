CXXFLAGS =	-O2 -g -Wall -fmessage-length=0 -std=c++03

OBJS =		all_sensors.o

LIBS =		-L/usr/local/lib -lwiringPi

TARGET =	all_sensors

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
