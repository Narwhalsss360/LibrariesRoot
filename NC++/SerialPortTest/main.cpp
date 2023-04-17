#include "SerialPort.h"
#include <iostream>
#include <thread>

#define SLEEP_TIME 8

SerialPort port = SerialPort("COM4", 1000000);
int recvCount = 0;

int main()
{
	if (!port.open("COM4"))
		return -1;

	DWORD size = port.getInQueue();
	unsigned char* buffer = (unsigned char*)malloc(30);
	bool success = port.readBytes(buffer, 30);

	std::cout << buffer;

	return 0;
}