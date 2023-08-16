#include "NStreamComParser.h"

bool VerifyBytes(uint8_t* buffer, uint32_t count)
{
#ifdef PACKET_EXCEPT
    if (*(uint16_t*)&buffer[4] != count - 6) throw PacketException(PacketException::PacketDataSizeBuffer_Mismatch);
#else
    return *(uint16_t*)&buffer[4] == count - 6;
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
PacketCollector::PacketCollector(bool clearOnError, bool throwOnExcept)
    : packets(DynamicArray<Packet>()), dataBytesCollected(0), packetsReady(nullptr), clearOnError(clearOnError), throwOnExcept(throwOnExcept)
{

}
#else
PacketCollector::PacketCollector(bool clearOnError)
    : packets(DynamicArray<Packet>()), dataBytesCollected(0), packetsReady(nullptr), clearOnError(clearOnError)
{
}
#endif

PacketCollector::PacketCollector(const PacketCollector& rvalue)
    : packets(DynamicArray<Packet>()), dataBytesCollected(rvalue.dataBytesCollected), packetsReady(rvalue.packetsReady), clearOnError(rvalue.clearOnError)
#ifdef PACKET_EXCEPT
    , throwOnExcept(rvalue.throwOnExcept)
#endif
{
    rvalue.packets.CopyTo(packets);
}

PacketCollector::PacketCollector(const PacketCollector&& lvalue)
    : packets(DynamicArray<Packet>()), dataBytesCollected(lvalue.dataBytesCollected), packetsReady(lvalue.packetsReady), clearOnError(lvalue.clearOnError)
#ifdef PACKET_EXCEPT
    , throwOnExcept(lvalue.throwOnExcept)
#endif
{
    lvalue.packets.CopyTo(packets);
}

bool PacketCollector::collect(uint8_t* buffer, uint32_t size)
{
#ifdef PACKET_EXCEPT
    try
    {
        VerifyBytes(buffer, size);
    }
    catch (const PacketException& e)
    {
        if (clearOnError) clear();
        if (throwOnExcept) throw;
        return false;
    }
#else
    if (!VerifyBytes(buffer, size))
    {
        if (clearOnError) clear();
        return false;
    }
#endif
    Packet collectedPacket = Packet(buffer);

    if (packets.Length() == 0)
        goto addPacket;

    if (collectedPacket.MessageID != packets[packets.Length() - 1].MessageID)
    {
        if (collectedPacket.MessageSize == dataBytesCollected) goto l_packetsReady;
        if (clearOnError) clear();
        collect(buffer, size);
#ifdef PACKET_EXCEPT
        throw PacketException(PacketException::DidNotReceiveAllPackets);
#else
        return false;
#endif
    }

addPacket:
    {
        packets.Push(collectedPacket);
        dataBytesCollected += packets[(packets.Length() > 0 ? packets.Length() - 1 : 0)].PacketDataSize;
    }

    if (dataBytesCollected != collectedPacket.MessageSize)
        return true;

l_packetsReady:
    if (packetsReady != nullptr)
        packetsReady(&packets[0], packets.Length());
    clear();
    return true;
}

void PacketCollector::onPacketsReady(void (*packetsReady)(Packet*, uint32_t))
{
    this->packetsReady = packetsReady;
}

bool PacketCollector::getPacketsReady()
{
    return packetsReady;
}

Packet* PacketCollector::getArray()
{
    return &packets[0];
}

uint32_t PacketCollector::getCount()
{
    return packets.Length();
}

void PacketCollector::clear()
{
    dataBytesCollected = 0;
    packets.SetCapacity(0);
}

PacketCollector::~PacketCollector()
{
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

bool Packet::operator==(const Packet& other) const
{
    if (other.MessageID != MessageID)
        return false;

    if (other.MessageSize != MessageSize)
        return false;

    if (other.PacketDataSize != PacketDataSize)
        return false;

    for (uint16_t i = 0; i < PacketDataSize; i++)
        if (Data[i] != other.Data[i])
            return false;

    return true;
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
        Packets[iPacket] = Packet(MessageID, MessageSize, PacketSize, &Data[(iPacket == PacketCount - 1) ? MessageSize - PacketSize : iPacket * PacketSize]);
    }
}

Message::~Message()
{
    if (Data == nullptr)
        return;
    delete[] Data;
    Data = nullptr;
}
#pragma endregion

#pragma region FastParser
FastParser::FastParser(uint16_t MessageID, uint16_t MessageSize, void* Data, uint16_t packetSize)
    : packets(nullptr), messagePacketsBytes(nullptr), packetsBytesSizes(nullptr), packetCount(GetRequiredPacketCount(MessageSize, packetSize))
{
    packets = new Packet[packetCount];
    packetsBytesSizes = new uint16_t[packetCount];
    messagePacketsBytes = new uint8_t*[packetCount];
    Message fastMessage = Message(MessageID, MessageSize, Data);
    fastMessage.GetPackets(packets, packetSize);
    GetMessagePacketsStreamBytes(packets, packetCount, messagePacketsBytes, packetsBytesSizes);
}

#if defined(ARDUINO) && ARDUINO >= 100
void FastParser::WriteTo(Print& print)
{
    for (int i = 0; i < packetCount; i++)
        print.write(messagePacketsBytes[i], packetsBytesSizes[i]);
}
#endif

FastParser::~FastParser()
{
    delete[] packets;
    delete[] packetsBytesSizes;
    deleteMessagePacketsStreamBytes(messagePacketsBytes, packetCount);
}
#pragma endregion

#pragma region DataSplitter
DataSplitter::DataSplitter()
    : splitCount(0), splittingSizes(nullptr), splittings(nullptr)
{
}

DataSplitter::DataSplitter(void* data, size_t dataSize, uint16_t dataSplitSize)
    : splitCount(-1 * ((-1* dataSize) / dataSplitSize)), splittingSizes(new uint16_t[splitCount]), splittings(new uint8_t[splitCount * (sizeof_index + dataSplitSize)])
{
    for (uint16_t iSplit = 0, iSplittingOffset = 0, iDataOffset = 0;
        iSplit < splitCount;
        iSplit++, iSplittingOffset += sizeof_index + dataSplitSize, iDataOffset += dataSplitSize)
    {
        splittingSizes[iSplit] = iSplit == splitCount - 1 ? (dataSize - dataSplitSize * iSplit) + sizeof_index : sizeof_index + dataSplitSize;
        memcpy(splittings + iSplittingOffset, &iSplit, sizeof_index);
        memcpy(splittings + iSplittingOffset + sizeof_index, &((uint8_t*)data)[iDataOffset], splittingSizes[iSplit] - sizeof_index);
    }
}

DataSplitter::DataSplitter(uint8_t* splttings, uint16_t* splittingsSizes, uint16_t splitCount)
    : splitCount(splitCount), splittingSizes(new uint16_t[splitCount]), splittings(nullptr)
{
    memcpy(this->splittingSizes, splittingSizes, sizeof(uint16_t) * splitCount);
    size_t totalSplittingsSize = 0;
    for (size_t i = 0; i < splitCount; i++)
        totalSplittingsSize += splittingSizes[i];

    splittings = new uint8_t[totalSplittingsSize];
    memcpy(this->splittings, splittings, totalSplittingsSize);
}

void* DataSplitter::Construct()
{
    size_t totalSize = 0;
    for (size_t i = 0; i < splitCount; i++)
        totalSize += splittingSizes[i];

    uint8_t* reconstruction = new uint8_t[totalSize];

    for (size_t iSplit = 0, iSplittingStart = 0, iSourceDataStart = sizeof_index;
        iSplit < splitCount;
        iSplittingStart += splittingSizes[iSplit],
        iSourceDataStart = iSplittingStart + sizeof_index,
        iSplit++)
    {
        uint16_t index = *(uint16_t*)&splittings[iSplittingStart];
        memcpy(reconstruction + ((splittingSizes[0] - sizeof_index) * index), &splittings[iSourceDataStart], splittingSizes[iSplit] - sizeof_index);
    }
    return reconstruction;
}

DataSplitter::~DataSplitter()
{
    if (splittingSizes)
        delete[] splittingSizes;
    if (splittings)
        delete[] splittings;
}
#pragma endregion

#pragma region RequestModel
#pragma region Request
RequestModel::Request::Request(uint16_t IDRequested)
    :   IDRequested(IDRequested)
{
}
#pragma endregion
#pragma endregion