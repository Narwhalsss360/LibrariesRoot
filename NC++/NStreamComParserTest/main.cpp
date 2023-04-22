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

	bool operator==(DataContainer& Other)
	{
		return bistate == Other.bistate && x == Other.x && y == Other.y;
	}
};

void SerializeDeserializeTest(bool* Pass, size_t* Name)

{
	const char TestName[] = "Serialize Deserialize";
	*Name = (size_t)TestName;
	DataContainer data = { 2, 4 };
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
	*Pass = true;
}

Message collectedMessage;

void PacketCollectorTest(bool* Pass, size_t* Name)
{
	const char TestName[] = "Packet Collector Test";
	*Name = (size_t)TestName;

	PacketCollecter collector = PacketCollecter();
	collector.onPacketsReady(packetsReady);

	int packetSize = 8;

	DataContainer dc = { 2, 4 };
	Message M = Message(1, sizeof(DataContainer), &dc);
	int packetCount = GetRequiredPacketCount(M.MessageSize, packetSize);

	Packet* packets new Packet[packetCount];
	M.GetPackets(packets, packetSize);

	uint8_t* packetsBytes = new uint8_t*[packetCount];
	uint16_t* sizes = new uint16_t[packetCount];
	GetMessagePacketsStreamBytes(packets, packetCount, packetsBytes, sizes);

	for (uint16_t i = 0; i < packetCount; i++;)
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

	if (collectedMessage != M) return;

	*Pass = true;
cleanup:
	delete[] packetsBytes;
	delete[] sizes;
}

void packetsReady(Packet* packets, uint32_t count)
{
	collectedMessage = Message(packets, count);
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