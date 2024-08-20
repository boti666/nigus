#include "includes.h"


#define IS_IN_RANGE( value, max, min ) ( value >= max && value <= min )
#define GET_BITS( value ) ( IS_IN_RANGE( value, '0', '9' ) ? ( value - '0' ) : ( ( value & ( ~0x20 ) ) - 'A' + 0xA ) )
#define GET_BYTE( value ) ( GET_BITS( value[0] ) << 4 | GET_BITS( value[1] ) )

std::uintptr_t Scan2(const std::uintptr_t image, const std::string& signature, bool LOL) {
	if (!image) {
		return 0u;
	}

	auto image_base = (std::uintptr_t)(image);
	auto image_dos_hdr = (IMAGE_DOS_HEADER*)(image_base);

	if (image_dos_hdr->e_magic != IMAGE_DOS_SIGNATURE) {
		return 0u;
	}

	auto image_nt_hdrs = (IMAGE_NT_HEADERS*)(image_base + image_dos_hdr->e_lfanew);

	if (image_nt_hdrs->Signature != IMAGE_NT_SIGNATURE) {
		return 0u;
	}

	auto scan_begin = (std::uint8_t*)(image_base);
	auto scan_end = (std::uint8_t*)(image_base + image_nt_hdrs->OptionalHeader.SizeOfImage);

	std::uint8_t* scan_result = nullptr;
	std::uint8_t* scan_data = (std::uint8_t*)(signature.c_str());

	for (auto current = scan_begin; current < scan_end; current++) {
		if (*(std::uint8_t*)scan_data == '\?' || *current == GET_BYTE(scan_data)) {
			if (!scan_result)
				scan_result = current;

			if (!scan_data[2])
				return (std::uintptr_t)(scan_result);

			scan_data += (*(std::uint16_t*)scan_data == '\?\?' || *(std::uint8_t*)scan_data != '\?') ? 3 : 2;

			if (!*scan_data)
				return (std::uintptr_t)(scan_result);
		}
		else if (scan_result) {
			current = scan_result;
			scan_data = (std::uint8_t*)(signature.c_str());
			scan_result = nullptr;
		}
	}

	return 0u;
}
std::uintptr_t Scan(const std::string& image_name, const std::string& signature, bool LOL) {
	auto image = GetModuleHandleA(image_name.c_str());
	return Scan2((std::uintptr_t)image, signature, LOL);
}


void C_VoiceCommunication::SendDataMsg(VoiceDataCustom* pData)
{
	// Creating message
	CCLCMsg_VoiceData_Legacy msg;
	memset(&msg, 0, sizeof(msg));

	static DWORD m_construct_voice_message = (DWORD)Scan("engine.dll", "56 57 8B F9 8D 4F 08 C7 07 ? ? ? ? E8 ? ? ? ? C7", true);

	auto func = (uint32_t(__fastcall*)(void*, void*))m_construct_voice_message;
	func((void*)&msg, nullptr);

	// Setup custom data
	msg.set_data(pData);

	// :D
	lame_string_t CommunicationString{ };

	// Setup voice message
	msg.data = &CommunicationString; // Its mad code
	msg.format = 0; // VoiceFormat_Steam
	msg.flags = 63; // All flags
	//g_cl.print("sending");
	g_csgo.m_engine->GetNetChannelInfo()->SendNetMsg((INetMessage*)&msg, false, true);
}

void C_VoiceCommunication::SendSharedESPData(VoiceTransferPacketFlags nType, C_SharedESPData pData)
{
	if (g_csgo.m_engine->GetNetChannelInfo())
	{
		// Filling packet
		VoiceDataCustom data;
		memcpy(data.get_raw_data(), &pData, sizeof(pData));
		SendDataMsg(&data);
	}
}

int C_VoiceCommunication::SendVoiceTransferData(VoiceTransferPacketFlags nType, void* pData, size_t nDataBytes)
{
	// Check if not many bytes
	if (nDataBytes > MAX_VOICETRANSFER_PACKET_DATA)
		return -1;

	// Filling packet data
	VoiceTransferDataTransmit_t Packet;
	memset(&Packet, 0, sizeof(VoiceTransferDataTransmit_t));
	strcpy(Packet.m_szID, ("GAA")); // 
	memcpy(Packet.m_szDataPacket, pData, nDataBytes);
	Packet.m_nDataType |= nType;

	// filling raw
	VoiceDataCustom data;
	memcpy(data.get_raw_data(), &Packet, sizeof(Packet));
	// send voice message
	SendDataMsg(&data);
	return 0;
}

#ifdef OVERSEE_DEV
std::string hexDump2(const char* desc, void* addr, int len)
{
	if (len <= 0)
		return std::string("NOT VALID SIZE");

	char* buffer = (char*)malloc(((len * 20) * sizeof(char)));
	// DUMB ASS WAY TO DO THIS!!!

	if (!buffer)
		return std::string("BUFFER FAILED TO ALLOC");

	int i;
	unsigned char buff[17];
	unsigned char* pc = (unsigned char*)addr;

	char tempBuf[4096];

	// Output description if given.
	if (desc != NULL) {
		snprintf(tempBuf, 4096, "%s:\n", desc);
		strcat(buffer, tempBuf);
	}

	// Process every byte in the data.
	for (i = 0; i < len; i++) {
		// Multiple of 16 means new line (with line offset).

		if ((i % 16) == 0) {
			// Just don't print ASCII for the zeroth line.
			if (i != 0) {
				snprintf(tempBuf, 4096, "  %s\n", buff);
				strcat(buffer, tempBuf);
			}

			// Output the offset.
			snprintf(tempBuf, 4096, "  %04x ", i);
			strcat(buffer, tempBuf);
		}

		// Now the hex code for the specific character.
		snprintf(tempBuf, 4096, " %02x", pc[i]);
		strcat(buffer, tempBuf);

		// And store a printable ASCII character for later.
		if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
			buff[i % 16] = '.';
		}
		else {
			buff[i % 16] = pc[i];
		}

		buff[(i % 16) + 1] = '\0';
	}

	// Pad out last line if not exactly 16 characters.
	while ((i % 16) != 0) {
		snprintf(tempBuf, 4096, "   ");
		strcat(buffer, tempBuf);
		i++;
	}

	// And print the final ASCII bit.
	snprintf(tempBuf, 4096, "  %s\n", buff);
	strcat(buffer, tempBuf);

	std::string retStr{ buffer };
	free(buffer);
	return retStr;
}
#endif
int lastsent = 0;
void C_VoiceCommunication::RunSharedESP()
{
	//	if (!g_SettingsManager->B[_S("misc.shared_esp")])
			//return;

		/* skip local servers */
	INetChannel* m_NetChannel = g_csgo.m_engine->GetNetChannelInfo();
	if (!m_NetChannel)
		return;

	/* build message */
	C_SharedESPData Data;
	strcpy(Data.m_szID, ("GAA"));

	constexpr int EXPIRE_DURATION = 0.5; // miliseconds-ish?
	bool should_send = g_csgo.m_globals->m_realtime - lastsent > EXPIRE_DURATION;

	if (!should_send)
		return;


	/* iterate all players */
	for (int nPlayer = 1; nPlayer <= g_csgo.m_globals->m_max_clients; nPlayer++)
	{
		/* get player */
		Player* pPlayer = g_csgo.m_entlist->GetClientEntity< Player* >(nPlayer);
		if (!pPlayer || !pPlayer->IsPlayer() || !pPlayer->alive() || pPlayer->dormant() || pPlayer == g_cl.m_local)
			continue;

		/* build data */
		Data.m_nUserID = nPlayer;
		Data.m_nOriginX = pPlayer->m_vecOrigin().x;
		Data.m_nOriginY = pPlayer->m_vecOrigin().y;
		Data.m_nOriginZ = pPlayer->m_vecOrigin().z;
		Data.m_nHealth = pPlayer->m_iHealth();

		/* send data */
		g_VoiceCommunication->SendSharedESPData(ReliableSend, Data);
	}

	lastsent = g_csgo.m_globals->m_realtime;
}
void C_VoiceCommunication::OnVoiceDataReceived(CSVCMsg_VoiceData_Legacy* pMsg)
{
	/* check shared esp */
//	if (!g_SettingsManager->B[_S("misc.shared_esp")] && !g_SettingsManager->B[_S("misc.shared_chams")])
		//return;
	//g_cl.print("recieved");
	/* check format */
	VoiceDataCustom VoiceData = pMsg->get_data();
	if (pMsg->format != 0)
		return;
	//	g_cl.print("recieve2d");
		/* check localplayer, do not process OUR own packet */
	if (!g_cl.m_local || g_cl.m_local->index() == pMsg->client + 1) {
		//	return;
		return;
	}

	//if (g_hooks.i[XOR("yaw_fakebody")] == pMsg->client + 1) {
		//	return;
	//		g_cl.print("recieved` pandora user");
	//	return;
	//}

	/* packet must not be empty */
//	g_cl.print(std::to_string(VoiceData.section_number));
	if (!VoiceData.section_number && !VoiceData.sequence_bytes && !VoiceData.uncompressed_sample_offset)
		return;
	//	g_cl.print("not empty");
		/* get ESP Data */
	C_SharedESPData Data = *(C_SharedESPData*)(VoiceData.get_raw_data());
	//C_SharedChamsData pChamsData = *(C_SharedChamsData*)(VoiceData.GetRawData());

	/* is it ESP from our cheat? */
	if (strstr(Data.m_szID, ("GAA")))
		g_DormantSystem->OnSharedESPReceived(Data);
	//else if (strstr(pChamsData.m_szID, _S("OFC")))
	//	g_ChamsSystem->OnSharedChamsDataReceived(pChamsData, pMsg->m_iClient + 1);
}
