CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		pir.o

LIBS =		-L/usr/local/lib -lwiringPi

TARGET =	pir

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
