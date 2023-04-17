#pragma once
#include <stdint.h>

#if defined(ARDUINO) && ARDUINO >= 100
#include "Arduino.h"
#else
#define PACKET_EXCEPT
#include <memory>
#endif // DEBUG

#ifdef PACKET_EXCEPT
#include <exception>
#endif

typedef struct Packet;
typedef struct Message;

bool VerifyBytes(uint8_t*, uint32_t);
bool VerifyPackets(Packet Packets[], int PacketCount);
int GetRequiredPacketCount(uint16_t MessageSize, uint16_t PacketSize);

void GetDataFromPackets(uint8_t Data[], Packet Packets[], int PacketCount);
void GetMessagePacketsStreamBytes(Packet Packets[], int PacketCount, uint8_t* PointerArray[], uint16_t sizes[]);
void deleteMessagePacketsStreamBytes(uint8_t* PointerArray[], int PacketCount);
void GetPacketsFromMessageStreamBytes(Packet Packets[], int PacketCount, uint8_t* PointerArray[]);

#ifdef PACKET_EXCEPT
class PacketException : std::exception
{
public:
    enum ErrorCode
    {
        NA,
        ID_Mismatch,
        MessageSize_Mismatch,
        PacketDataSizeTotal_Mismatch,
        PacketDataSizeBuffer_Mismatch
    } Code;

public:
    PacketException(ErrorCode Code);
    ~PacketException();

private:

};
#endif // PACKET_EXCEPT

class PacketCollector
{
public:
#ifdef PACKET_EXCEPT
    PacketCollector(bool = true);
#else
    PacketCollector();
#endif
    bool Collect(uint8_t*, uint32_t);
    ~PacketCollector();

private:
    Packet* packets;
    uint32_t packetCount;
    void (*packetsReady)(Packet*, uint32_t);
#ifdef PACKET_EXCEPT
    const bool throwOnExcept;
#endif
};

struct Packet
{
    const static uint8_t StreamProtocolSize = 6;

    uint16_t MessageID;
    uint16_t MessageSize;
    uint16_t PacketDataSize;
    uint8_t* Data = nullptr;

    Packet();

    Packet(const Packet& Other);

    Packet(uint16_t MessageID, uint16_t MessageSize, uint16_t PacketDataSize, void* Data);

    Packet(uint8_t Buffer[]);

    Packet(Message Message);

    Packet& operator=(const Packet& Other);

    void GetStreamBytes(uint8_t Buffer[]);

    int StreamBytesArraySize();

    ~Packet();
};

struct Message
{
    uint16_t MessageID;
    uint16_t MessageSize;
    uint8_t* Data = nullptr;

    Message(uint16_t MessageID, uint16_t MessageSize, void* Data);

    Message(Packet Packets[], int PacketCount);

    void GetPackets(Packet Packets[], uint16_t PacketSize);

    ~Message();
};