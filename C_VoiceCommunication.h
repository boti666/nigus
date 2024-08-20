#pragma once
#include "DormantSystem.h"

class C_VoiceCommunication
{
public:
    virtual void SendDataMsg(VoiceDataCustom* pData);
    virtual void SendSharedESPData(VoiceTransferPacketFlags nType, C_SharedESPData Data);
    virtual int SendVoiceTransferData(VoiceTransferPacketFlags nType, void* pData, size_t nDataBytes);
    //virtual int SendSplitPacketMessage(VoiceTransferPacketFlags nType, void* pData, size_t nDataBytes); // For very big data  ( Shared Resolver / Shared Skins ) @ SapDragon
    virtual void OnVoiceDataReceived(CSVCMsg_VoiceData_Legacy* pMsg);
    virtual void RunSharedESP();
};

inline C_VoiceCommunication* g_VoiceCommunication = new C_VoiceCommunication();