#include <NStreamComParser.h>

template <typename T>
bool arrayCompare(T* a, T* b, size_t size)
{
	for (size_t i = 0; i < size; i++) if (a[i] != b[i]) return false;
	return true;
}

typedef void (*TestFunctionPointer)(bool*, size_t*);

#pragma pack(1)
struct DataContainer
{
	bool bistate;
	double x, y;

	DataContainer(bool b, double x, double y)
		: bistate(b), x(x), y(y)
	{
	}

	bool operator==(DataContainer& Other)
	{
		return bistate == Other.bistate && x == Other.x && y == Other.y;
	}

	bool operator!=(DataContainer& Other)
	{
		return !operator==(Other);
	}
};

void SerializeDeserializeTest(bool* Pass, size_t* Name)

{
	const char TestName[] = "Serialize Deserialize";
	*Name = (size_t)TestName;
	DataContainer data = { false, 2, 4 };
	Message M = Message(1, sizeof(data), &data);

	int PacketSize = 8;
	int PacketCount = GetRequiredPacketCount(M.MessageSize, PacketSize);

	Packet* Packets = new Packet[PacketCount];
	M.GetPackets(Packets, PacketSize);

	uint8_t** PacketsBytes = new uint8_t * [PacketCount];
	uint16_t* Sizes = new uint16_t[PacketCount];
	GetMessagePacketsStreamBytes(Packets, PacketCount, PacketsBytes, Sizes);

	Packet* PacketsR = new Packet[PacketCount];
	GetPacketsFromMessageStreamBytes(PacketsR, PacketCount, PacketsBytes);
	Message MessageR = Message(PacketsR, PacketCount);
	DataContainer dataR = *(DataContainer*)MessageR.Data;

	deleteMessagePacketsStreamBytes(PacketsBytes, PacketCount);
	delete[] Sizes;
	delete[] Packets;
	delete[] PacketsR;

	if (data == dataR) *Pass = true;
}

void VerificationTest(bool* Pass, size_t* Name)
{
	const char TestName[] = "Verification Test";
	*Name = (size_t)TestName;
	uint16_t ID = 400;
	uint16_t msize = 512;
	uint16_t psize = 256;

	uint8_t packet1[256 + 6];
	uint8_t packet2[256 + 6];

	for (size_t i = 0; i < 256; i++)
	{
		packet1[6 + i] = i;		
		packet2[6 + i] = i;
	}

	(*(uint16_t*)&packet1[0]) = ID;
	(*(uint16_t*)&packet2[0]) = ID;

	(*(uint16_t*)&packet1[2]) = msize;
	(*(uint16_t*)&packet2[2]) = msize;

	(*(uint16_t*)&packet1[4]) = psize;
	(*(uint16_t*)&packet2[4]) = psize;

	try
	{
		VerifyBytes(packet1, 256 + 6);
		VerifyBytes(packet1, 256 + 6);
	}
	catch (const std::exception&)
	{

	}

	*Pass = true;
}

Message collectedMessage = Message(0, 0, nullptr);

void packetsReady(Packet* packets, uint32_t count)
{
	collectedMessage = Message(packets, count);
}

void PacketCollectorTest(bool* Pass, size_t* Name)
{
	const char TestName[] = "Packet Collector Test";
	*Name = (size_t)TestName;

	PacketCollector collector = PacketCollector();
	collector.onPacketsReady(packetsReady);

	int packetSize = 8;

	DataContainer dc = DataContainer(false, 2, 4);
	Message M = Message(1, sizeof(DataContainer), &dc);
	int packetCount = GetRequiredPacketCount(M.MessageSize, packetSize);

	Packet* packets = new Packet[packetCount];
	M.GetPackets(packets, packetSize);

	uint8_t** packetsBytes = new uint8_t*[packetCount];
	uint16_t* sizes = new uint16_t[packetCount];
	GetMessagePacketsStreamBytes(packets, packetCount, packetsBytes, sizes);

	for (uint16_t i = 0; i < packetCount; i++)
	{
		try
		{
			if(!collector.collect(packetsBytes[i], sizes[i])) goto cleanup;
		}
		catch (std::exception)
		{
			goto cleanup;
		}
	}

	if (M.MessageID != collectedMessage.MessageID)
		return;

	if (M.MessageSize != collectedMessage.MessageSize)
		return;

	for (size_t i = 0; i < M.MessageSize; i++)
		if (M.Data[i] != collectedMessage.Data[i])
			return;

	*Pass = true;
cleanup:
	delete[] packetsBytes;
	delete[] sizes;
}

int main()
{
	TestFunctionPointer Tests[] =
	{
		SerializeDeserializeTest,
		VerificationTest,
		PacketCollectorTest
	};

	const int TestCount = sizeof(Tests) / 8;
	bool TestResults[TestCount] = { false };
	char* TestNames[TestCount] = { nullptr };

	for (uint8_t Test = 0; Test < TestCount; Test++) Tests[Test](&TestResults[Test], (size_t*)& TestNames[Test]);
}