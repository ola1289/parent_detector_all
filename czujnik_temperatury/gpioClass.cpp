#define GPIO_PATH "/sys/class/gpio/"
#include<string>

namespace exploringRPi {

enum GPIO_DIRECTION{ INPUT, OUTPUT }; // enumerations limit options
enum GPIO_VALUE{ LOW=0, HIGH=1 };
enum GPIO_EDGE{ NONE, RISING, FALLING, BOTH };

class GPIO {
private:
	int number, debounceTime;
	string name, path;

public:
	GPIO(int number); // the constructor exports pin
	virtual int getNumber() { return number; }

	// General Input and Output Settings
	virtual int setDirection(GPIO_DIRECTION);
	virtual GPIO_DIRECTION getDirection();
	virtual int setValue(GPIO_VALUE);
	virtual int toggleOutput();
	virtual GPIO_VALUE getValue();
	virtual int setActiveLow(bool isLow=true); // low=1, high=0
	virtual int setActiveHigh(); // default state
	virtual void setDebounceTime(int time) { this->debounceTime = time; }

	// Advanced output: faster by keeping the stream open (~20x)
	virtual int streamOpen();
	virtual int streamWrite(GPIO_VALUE);
	virtual int streamClose();
	virtual int toggleOutput(int time); // thread invert output every X ms
	virtual int toggleOutput(int numberOfTimes, int time);
	virtual void changeToggleTime(int time) { this->togglePeriod = time; }
	virtual void toggleCancel() { this->threadRunning = false; }

	// Advanced input: presented later in this chapter
	virtual int setEdgeType(GPIO_EDGE);
	virtual GPIO_EDGE getEdgeType();
	virtual int waitForEdge(); // waits until button is pressed
	virtual int waitForEdge(CallbackType callback); // threaded callback
	virtual void waitForEdgeCancel() { this->threadRunning = false; }
	virtual ~GPIO(); // destructor unexports the pin

private:
	int write(string path, string filename, string value);
	int write(string path, string filename, int value);
	string read(string path, string filename);
	int exportGPIO();
	int unexportGPIO();
	ofstream stream;
	pthread_t thread;
	CallbackType callbackFunction;
	bool threadRunning;
	int togglePeriod; // default 100ms
	int toggleNumber; // default -1 (infinite)
	friend void* threadedPoll(void *value);
	friend void* threadedToggle(void *value);
};
	void* threadedPoll(void *value); // callback functions for threads
	void* threadedToggle(void *value); // callback functions for threads
} /* namespace exploringRPi */
