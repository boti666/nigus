#pragma once

struct C_SharedESPData
{
    char m_szID[4]{ };
    uint8_t m_nUserID;
    int16_t m_nOriginX;
    int16_t m_nOriginY;
    int16_t m_nOriginZ;
    int16_t m_nHealth;
};

struct DormantPlayer_t
{
    float m_DormantAlpha = 0.0f;
    float m_LastDormantTime = 0.0f;
    float m_LastNonDormantTime = 0.0f;

    vec3_t m_DormantOrigin = vec3_t(0, 0, 0);
    vec3_t m_NetworkOrigin = vec3_t(0, 0, 0);

    int m_LastDormantHealth = 0;
    int m_LastEventWeaponID = 0;
    int m_LastEventWeaponType = 0;
};

struct C_SharedESPData;
class C_DormantSystem
{
public:
    virtual void OnItemEquip(IGameEvent* m_EventInfo);
    virtual void OnPostNetworkDataReceived();

    //virtual void OnRadarDataReceived(CCSUsrMsg_ProcessSpottedEntityUpdate_SpottedEntityUpdate* pMsg);
    virtual void OnSharedESPReceived(C_SharedESPData Data);

    virtual void ResetPlayer(Player* Player);
    virtual void ResetData();

    virtual int GetDormantHealth(int nPlayerIndex);
    virtual int GetLastActiveWeapon(int nPlayerIndex);
    virtual int GetLastActiveWeaponType(int nPlayerIndex);
    virtual float GetTimeInDormant(int nPlayerIndex);
    virtual float GetTimeSinceLastData(int nPlayerIndex);
    virtual float GetLastDormantTime(int nPlayerIndex);
    virtual float GetLastActiveTime(int nPlayerIndex);
    virtual bool IsValidSoundData(SndInfo_t m_Sound);

    virtual vec3_t GetLastNetworkOrigin(int nPlayerIndex);
    virtual vec3_t GetLastDormantOrigin(int nPlayerIndex);
private:
    struct
    {
        CUtlVector < SndInfo_t > m_CurrentSoundData;
        CUtlVector < SndInfo_t > m_CachedSoundData;
    } m_SoundData;

    std::array < DormantPlayer_t, 64 > m_DormantPlayers;
};

inline C_DormantSystem* g_DormantSystem = new C_DormantSystem();