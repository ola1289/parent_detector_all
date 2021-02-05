CXXFLAGS =	-O2 -g -Wall -fmessage-length=0

OBJS =		gpio01.o

LIBS =		-L/usr/local/lib -lpigpio

TARGET =	gpio01

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS)

all:	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
