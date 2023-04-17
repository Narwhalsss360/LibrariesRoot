#pragma once
#include <Windows.h>
#include <time.h>
#include <stdint.h>

class SerialPort
{
	HANDLE portHandle;

	const char* portName;

	DCB deviceControlBlock;

	COMSTAT comStatus;

	uint32_t errors;

	uint32_t baudrate;

	uint32_t timeout;

	void loadStatus();

	void loadDeviceControlBlock();

public:

	SerialPort(uint32_t);

	SerialPort();

	SerialPort(const char*, uint32_t);

	~SerialPort();

	bool open();

	bool open(const char*);

	bool close();

	void purge();

	DWORD getInQueue();

	HANDLE getPortHandle();

	void setPortName(const char*);

	const char* getPortName();

	DCB* getDCB();

	COMSTAT* getComStatus();

	uint32_t getErrors();

	void clearErrors();

	int setBaudrate(uint32_t);

	uint32_t getBaudrate();

	void setTimeout(uint32_t);

	uint32_t getTimeout();

	bool empty();

	int write(uint8_t);

	int writeBytes(uint8_t*, size_t);

	int read(int* = nullptr);

	int readWait(int* = nullptr);

	int readBytes(uint8_t*, size_t, int(SerialPort::*)(int*) = &readWait);

	int readBytesUntil(uint8_t*, size_t, uint8_t = NULL, int(SerialPort::*)(int*) = &readWait);

	char* readAll(int* = nullptr);
};