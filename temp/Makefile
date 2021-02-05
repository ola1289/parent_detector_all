CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		temp.o

LIBS =		-L/usr/local/lib -lwiringPi

TARGET =	temp

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
