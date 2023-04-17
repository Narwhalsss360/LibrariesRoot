#include "NStreamComParser.h"

bool VerifyBytes(uint8_t* buffer, uint32_t count)
{
#ifdef PACKET_EXCEPT
    if (*(uint16_t*)&buffer[5] != count - 6) throw PacketException(PacketException::PacketDataSizeBuffer_Mismatch);
#else
    return *(uint16_t*)&buffer[5] == count - 6;
#endif // PACKET_EXCEPT
}

bool VerifyPackets(Packet Packets[], int PacketCount)
{
    uint16_t ID = Packets[0].MessageID;
    uint16_t MessageSize = Packets[0].MessageSize;
    uint16_t PacketSizesTotal = 0;

    for (uint16_t iPacket = 0; iPacket < PacketCount; iPacket++)
    {
        if (Packets[iPacket].MessageID != ID)
        {
#ifdef PACKET_EXCEPT
            throw PacketException(PacketException::ID_Mismatch);
#else
            return false;
#endif // PACKET_EXCEPT

        }

        if (Packets[iPacket].MessageSize != MessageSize)
        {
#ifdef PACKET_EXCEPT
            throw PacketException(PacketException::MessageSize_Mismatch);
#else
            return false;
#endif // PACKET_EXCEPT
        }

        PacketSizesTotal += Packets[iPacket].PacketDataSize;
    }

    if (PacketSizesTotal != MessageSize)
    {
#ifdef PACKET_EXCEPT
        throw PacketException(PacketException::PacketDataSizeTotal_Mismatch);
#else
        return false;
#endif // PACKET_EXCEPT
    }

    return true;
}

int GetRequiredPacketCount(uint16_t MessageSize, uint16_t PacketSize)
{
    return ceil((double)MessageSize / PacketSize);
}

void GetDataFromPackets(uint8_t Data[], Packet Packets[], int PacketCount)
{
    uint16_t DstIdx = 0;
    for (uint16_t iPacket = 0; iPacket < PacketCount; iPacket++)
    {
        memcpy(&Data[DstIdx], Packets[iPacket].Data, Packets[iPacket].PacketDataSize);
        DstIdx += Packets[iPacket].PacketDataSize;
    }
}

void GetMessagePacketsStreamBytes(Packet Packets[], int PacketCount, uint8_t* PointerArray[], uint16_t Sizes[])
{
    for (uint16_t iPacket = 0; iPacket < PacketCount; iPacket++)
    {
        Sizes[iPacket] = Packets[iPacket].StreamBytesArraySize();
        PointerArray[iPacket] = new uint8_t[Sizes[iPacket]];
        Packets[iPacket].GetStreamBytes(PointerArray[iPacket]);
    }
}

void deleteMessagePacketsStreamBytes(uint8_t* PointerArray[], int PacketCount)
{
    for (uint16_t i = 0; i < PacketCount; i++)
        delete[] PointerArray[i];
    delete[] PointerArray;
}

void GetPacketsFromMessageStreamBytes(Packet Packets[], int PacketCount, uint8_t* PointerArray[])
{
    for (uint16_t iPacket = 0; iPacket < PacketCount; iPacket++) Packets[iPacket] = Packet(PointerArray[iPacket]);
}

#pragma region PacketException
#ifdef PACKET_EXCEPT
PacketException::PacketException(ErrorCode Code)
    : Code(Code)
{
}

PacketException::~PacketException()
{
}
#endif // PACKET_EXCEPT
#pragma endregion

#pragma region PacketCollector
#ifdef PACKET_EXCEPT
PacketCollector::PacketCollector(bool throwOnExcept)
    : packets(nullptr), packetCount(0), packetsReady(nullptr), throwOnExcept(throwOnExcept)
{

}
#else
PacketCollector::PacketCollecter()
    : packets(nullptr), packetCount(0), packetsReady(nullptr)
{
}
#endif

bool PacketCollector::Collect(uint8_t* buffer, uint32_t size)
{
#ifdef PACKET_EXCEPT
    try
    {
        VerifyBytes(buffer, size);
    }
    catch (const PacketException& e)
    {
        if (throwOnExcept) 
    }
#else
    if (!VerifyBytes(buffer, size)) return false;
#endif
}

PacketCollector::~PacketCollector()
{
    if (packets != nullptr)
    {
        delete[] packets;
        packetCount = 0;
    }
}
#pragma endregion

#pragma region Packet
Packet::Packet()
    : MessageID(0), MessageSize(0), PacketDataSize(0), Data(nullptr)
{
}

Packet::Packet(const Packet& Other)
    : MessageID(Other.MessageID), MessageSize(Other.MessageSize), PacketDataSize(Other.PacketDataSize), Data(nullptr)
{
    Data = new uint8_t[PacketDataSize];
    memcpy(this->Data, Other.Data, PacketDataSize);
}

Packet::Packet(uint16_t MessageID, uint16_t MessageSize, uint16_t PacketDataSize, void* Data)
    : MessageID(MessageID), MessageSize(MessageSize), PacketDataSize(PacketDataSize), Data(nullptr)
{
    this->Data = new uint8_t[PacketDataSize];
    memcpy(this->Data, Data, PacketDataSize);
}

Packet::Packet(uint8_t Buffer[])
{
    MessageID = *(uint16_t*)&Buffer[0];
    MessageSize = *(uint16_t*)&Buffer[2];
    PacketDataSize = *(uint16_t*)&Buffer[4];
    Data = new uint8_t[PacketDataSize];
    memcpy(Data, &Buffer[6], PacketDataSize);
}

Packet::Packet(Message Message)
    : Packet(Message.MessageID, Message.MessageSize, Message.MessageSize, Message.Data)
{
}

Packet& Packet::operator=(const Packet& Other)
{
    if (Data != nullptr)
        delete[] Data;
    MessageID = Other.MessageID;
    MessageSize = Other.MessageSize;
    PacketDataSize = Other.PacketDataSize;
    Data = new uint8_t[PacketDataSize];
    memcpy(Data, Other.Data, PacketDataSize);
    return *this;
}

void Packet::GetStreamBytes(uint8_t Buffer[])
{
    memcpy(&Buffer[0], &MessageID, sizeof(uint16_t));
    memcpy(&Buffer[2], &MessageSize, sizeof(uint16_t));
    memcpy(&Buffer[4], &PacketDataSize, sizeof(uint16_t));
    memcpy(&Buffer[6], Data, PacketDataSize);
}

int Packet::StreamBytesArraySize()
{
    return PacketDataSize + StreamProtocolSize;
}

Packet::~Packet()
{
    if (Data != nullptr)
        delete[] Data;
}
#pragma endregion

#pragma region Message
Message::Message(uint16_t MessageID, uint16_t MessageSize, void* Data)
    : MessageID(MessageID), MessageSize(MessageSize), Data(nullptr)
{
    this->Data = new uint8_t[MessageSize];
    memcpy(this->Data, Data, MessageSize);
}

Message::Message(Packet Packets[], int PacketCount)
{
    VerifyPackets(Packets, PacketCount);
    MessageID = Packets[0].MessageID;
    MessageSize = Packets[0].MessageSize;
    Data = new uint8_t[MessageSize];
    GetDataFromPackets(Data, Packets, PacketCount);
}

void Message::GetPackets(Packet Packets[], uint16_t PacketSize)
{
    if (PacketSize >= MessageSize)
    {
        Packets[0] = Packet(MessageID, MessageSize, PacketSize, Data);
        return;
    }

    int PacketCount = GetRequiredPacketCount(MessageSize, PacketSize);

    for (int iPacket = 0; iPacket < PacketCount; iPacket++)
    {
        PacketSize = ((iPacket == PacketCount - 1) ? MessageSize - (iPacket * PacketSize) : PacketSize);
        Packets[iPacket] = Packet(MessageID, MessageSize, PacketSize, &Data[iPacket * PacketSize]);
    }
}

Message::~Message()
{
    if (Data != nullptr) delete[] Data;
}
#pragma endregion