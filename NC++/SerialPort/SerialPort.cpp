#include "SerialPort.h"

#define DEFAULT_TIMEOUT 1000
#define setNullablePointer(ptr, val) ((ptr == nullptr) ? (val) : (*ptr = val))
#define clockCyclesToMilliseconds(cycles) (cycles / (CLOCKS_PER_SEC / 1000))
#define PURGE_FLAGS PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR

SerialPort::SerialPort(uint32_t baudrate)
	: portHandle(nullptr), portName(nullptr), errors(0), baudrate(0), timeout(DEFAULT_TIMEOUT), comStatus(COMSTAT()), deviceControlBlock(DCB())
{
	loadDeviceControlBlock();
	setBaudrate(baudrate);
}

SerialPort::SerialPort()
	: SerialPort(115200)
{
	loadDeviceControlBlock();
}

SerialPort::SerialPort(const char* portName, uint32_t baudrate)
	: portHandle(nullptr), portName(portName), errors(0), baudrate(0), timeout(DEFAULT_TIMEOUT), comStatus(COMSTAT()), deviceControlBlock(DCB())
{
	loadDeviceControlBlock();
	setBaudrate(baudrate);
}

SerialPort::~SerialPort()
{
}

void SerialPort::loadStatus()
{
	ClearCommError(portHandle, (LPDWORD)&errors, &comStatus);
}

void SerialPort::loadDeviceControlBlock()
{
	GetCommState(portHandle, &deviceControlBlock);
	deviceControlBlock.BaudRate = baudrate;
	deviceControlBlock.ByteSize = 8;
	deviceControlBlock.DCBlength = sizeof(DCB);
	deviceControlBlock.fParity = PARITY_NONE;
	deviceControlBlock.StopBits = ONESTOPBIT;
	SetCommState(portHandle, &deviceControlBlock);
}

bool SerialPort::open()
{
	if (portName == nullptr) return false;
	purge();
	portHandle = CreateFileA(portName, GENERIC_READ | GENERIC_WRITE, NULL, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	purge();
	return !(INVALID_HANDLE_VALUE == portName);
}

bool SerialPort::open(const char* portName)
{
	this->portName = portName;
	return open();
}

bool SerialPort::close()
{
	if (portHandle == nullptr) return false;
	purge();
	bool result = CloseHandle(portHandle);
	portHandle = nullptr;
	return result;
}

void SerialPort::purge()
{
	PurgeComm(portHandle, PURGE_FLAGS);
}

DWORD SerialPort::getInQueue()
{
	loadStatus();
	return comStatus.cbInQue;
}

HANDLE SerialPort::getPortHandle()
{
	return portHandle;
}

void SerialPort::setPortName(const char* portName)
{
	this->portName = portName;
}

const char* SerialPort::getPortName()
{
	return portName;
}

DCB* SerialPort::getDCB()
{
	return &deviceControlBlock;
}

COMSTAT* SerialPort::getComStatus()
{
	return &comStatus;
}

uint32_t SerialPort::getErrors()
{
	return errors;
}

void SerialPort::clearErrors()
{
	errors = 0;
}

int SerialPort::setBaudrate(uint32_t baudrate)
{
	this->baudrate = baudrate;

	int getter = GetCommState(portHandle, &deviceControlBlock);

	deviceControlBlock.DCBlength = sizeof(DCB);
	deviceControlBlock.BaudRate = this->baudrate;
	deviceControlBlock.Parity = NOPARITY;
	deviceControlBlock.StopBits = ONESTOPBIT;
	deviceControlBlock.ByteSize = 8;

	int setter = SetCommState(portHandle, &deviceControlBlock);

	return getter & setter;
}

uint32_t SerialPort::getBaudrate()
{
	return baudrate;
}

void SerialPort::setTimeout(uint32_t timeout)
{
	this->timeout = timeout;
}

uint32_t SerialPort::getTimeout()
{
	return timeout;
}

bool SerialPort::empty()
{
	loadStatus();
	return (comStatus.cbInQue == 0);
}

int SerialPort::write(uint8_t byte)
{
	return WriteFile(portHandle, &byte, 1, nullptr, nullptr);
}

int SerialPort::writeBytes(uint8_t* buffer, size_t bufferSize)
{
	return WriteFile(portHandle, buffer, (DWORD)bufferSize, nullptr, nullptr);
}

int SerialPort::read(int* success)
{
	success = 0;
	if (empty()) return -1;
	int readResult = 0;
	setNullablePointer(success, ReadFile(portHandle, &readResult, 1, nullptr, nullptr));
	return readResult;
}

int SerialPort::readWait(int* success)
{
	int readResult = 0;
	setNullablePointer(success, ReadFile(portHandle, &readResult, 1, nullptr, nullptr));
	return readResult;
}

int SerialPort::readBytes(uint8_t* buffer, size_t bufferSize, int (SerialPort::*reader) (int*))
{
	int finalResult = 1;
	clock_t start = clock();

	for (size_t i = 0; i < bufferSize; i++)
	{
		int result;
		buffer[i] = (this->*reader)(&result);
		finalResult &= result;
		if (clockCyclesToMilliseconds(clock() - start) > timeout)
			finalResult = 0;
	}

	return finalResult;
}

int SerialPort::readBytesUntil(uint8_t* buffer, size_t bufferSize, uint8_t terminator, int (SerialPort::* reader) (int*))
{
	int finalResult = 1;
	clock_t start = clock();

	for (size_t i = 0; i < bufferSize; i++)
	{
		int result;
		buffer[i] = (this->*reader)(&result);
		finalResult &= result;
		if (clockCyclesToMilliseconds(clock() - start) > timeout)
			finalResult = 0;
		if (buffer[i] == terminator)
			return finalResult;
	}

	return finalResult;
}

char* SerialPort::readAll(int* success)
{
	loadStatus();
	size_t bytesToRead = comStatus.cbInQue;
	if (bytesToRead == 0)
		return nullptr;

	//char* buffer = new char[bytesToRead];
	setNullablePointer(success, ReadFile(portHandle, nullptr, bytesToRead, nullptr, nullptr));
	return 0;
}