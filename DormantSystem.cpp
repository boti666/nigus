#include "includes.h"

void C_DormantSystem::OnItemEquip(IGameEvent* m_GameEvent)
{
	int nPlayerID = g_csgo.m_engine->GetPlayerForUserID(m_GameEvent->GetInt(("userid")));

	m_DormantPlayers[nPlayerID].m_LastEventWeaponID = m_GameEvent->GetInt(("defindex"));
	m_DormantPlayers[nPlayerID].m_LastEventWeaponType = m_GameEvent->GetInt(("weptype"));
}
void C_DormantSystem::OnSharedESPReceived(C_SharedESPData Data)
{
	m_DormantPlayers[Data.m_nUserID].m_DormantOrigin = vec3_t(Data.m_nOriginX, Data.m_nOriginY, Data.m_nOriginZ);
	m_DormantPlayers[Data.m_nUserID].m_LastDormantTime = g_csgo.m_globals->m_realtime;
	m_DormantPlayers[Data.m_nUserID].m_LastDormantHealth = Data.m_nHealth;
}
void C_DormantSystem::OnPostNetworkDataReceived()
{
	// clear sounds
	m_SoundData.m_CurrentSoundData.RemoveAll();

	// parse sounds
	g_csgo.m_sound->GetActiveSounds(m_SoundData.m_CurrentSoundData);
	if (!m_SoundData.m_CurrentSoundData.Count())
		return;

	CGameTrace Trace;
	for (int nSoundIndex = 0; nSoundIndex < m_SoundData.m_CurrentSoundData.Count(); nSoundIndex++)
	{
		auto& m_Sound = m_SoundData.m_CurrentSoundData[nSoundIndex];
		if (m_Sound.m_nSoundSource < 1 || m_Sound.m_nSoundSource > g_csgo.m_globals->m_max_clients)
			continue;

		if (m_Sound.m_pOrigin->IsZero() || !this->IsValidSoundData(m_Sound))
			continue;

		Player* pPlayer = g_csgo.m_entlist->GetClientEntity< Player* >(m_Sound.m_nSoundSource);
		if (!pPlayer || !pPlayer->alive() || !pPlayer->dormant())
			continue;

		vec3_t vecStart = *m_Sound.m_pOrigin + vec3_t(0.0f, 0.0f, 1.0f);
		vec3_t vecEnd = vecStart - vec3_t(0.0f, 0.0f, 100.0f);

		g_csgo.m_engine_trace->TraceLine(vecStart, vecEnd, MASK_PLAYERSOLID, pPlayer, COLLISION_GROUP_NONE, &Trace);
		if (Trace.m_allsolid)
			continue;

		// set dormant data
		m_DormantPlayers[m_Sound.m_nSoundSource].m_DormantOrigin = Trace.m_fraction <= 0.97f ? Trace.m_endpos : *m_Sound.m_pOrigin;
		m_DormantPlayers[m_Sound.m_nSoundSource].m_LastDormantTime = g_csgo.m_globals->m_realtime;

		pPlayer->m_bSpotted() = true;
		// spot player
		//pPlayer->MakeEntitySpotted();
	}

	m_SoundData.m_CachedSoundData = m_SoundData.m_CurrentSoundData;
}
void C_DormantSystem::ResetPlayer(Player* Player)
{
	m_DormantPlayers[Player->index()].m_DormantOrigin = Player->GetAbsOrigin();
	m_DormantPlayers[Player->index()].m_NetworkOrigin = Player->GetAbsOrigin();
	m_DormantPlayers[Player->index()].m_LastNonDormantTime = g_csgo.m_globals->m_realtime;
	m_DormantPlayers[Player->index()].m_LastDormantTime = g_csgo.m_globals->m_realtime;
	m_DormantPlayers[Player->index()].m_LastEventWeaponID = 0;
	m_DormantPlayers[Player->index()].m_LastEventWeaponType = -1;
	m_DormantPlayers[Player->index()].m_LastDormantHealth = Player->m_iHealth();
}
/*
void C_DormantSystem::OnRadarDataReceived(CCSUsrMsg_ProcessSpottedEntityUpdate_SpottedEntityUpdate* Message)
{
	if (!g_Globals->m_LocalPlayer)
		return;
	int nEntityUpdatesSize = *(int*)((DWORD)(Message)+0xC);
	if (nEntityUpdatesSize <= 0)
		return;
	for (int i = 0; i < nEntityUpdatesSize; i++)
	{
		DWORD dwEntity = *(DWORD*)(*(DWORD*)((DWORD)Message + 0x8) + 4 * i);
		if (!dwEntity)
			continue;
		int nEntityID = *(int*)(dwEntity + 0x8);
		if (nEntityID <= 0 || nEntityID > 64)
			continue;
		int nClassID = *(int*)(dwEntity + 0xC);
		if (nClassID != ClassId_CCSPlayer)
			continue;
		Vector vecDormantOrigin = *(Vector*)(dwEntity + 0x10) * 4;
		if (vecDormantOrigin.IsZero())
			continue;
		/* set dormant data
		m_DormantPlayers[nEntityID].m_DormantOrigin = vecDormantOrigin;
		m_DormantPlayers[nEntityID].m_LastDormantTime = SDK::Interfaces::GlobalVars->m_flRealTime;
		/* get player
		C_BasePlayer* m_Player = C_BasePlayer::GetPlayerByIndex(nEntityID);
		if (m_Player)
			m_Player->MakeEntitySpotted();
	}
}*/
float C_DormantSystem::GetLastActiveTime(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_LastNonDormantTime;
}
float C_DormantSystem::GetLastDormantTime(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_LastDormantTime;
}
float C_DormantSystem::GetTimeInDormant(int nPlayerIndex)
{
	return fmax(0.0f, g_csgo.m_globals->m_realtime - m_DormantPlayers[nPlayerIndex].m_LastNonDormantTime);
}
float C_DormantSystem::GetTimeSinceLastData(int nPlayerIndex)
{
	return fmax(0.0f, g_csgo.m_globals->m_realtime - m_DormantPlayers[nPlayerIndex].m_LastDormantTime);
}
vec3_t C_DormantSystem::GetLastNetworkOrigin(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_NetworkOrigin;
}
vec3_t C_DormantSystem::GetLastDormantOrigin(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_DormantOrigin;
}
int C_DormantSystem::GetDormantHealth(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_LastDormantHealth;
}
int C_DormantSystem::GetLastActiveWeapon(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_LastEventWeaponID;
}
int C_DormantSystem::GetLastActiveWeaponType(int nPlayerIndex)
{
	return m_DormantPlayers[nPlayerIndex].m_LastEventWeaponType;
}



void C_DormantSystem::ResetData()
{
	m_DormantPlayers.fill(DormantPlayer_t());
	m_SoundData.m_CachedSoundData.FillWithValue(SndInfo_t());
	m_SoundData.m_CurrentSoundData.FillWithValue(SndInfo_t());
}
bool C_DormantSystem::IsValidSoundData(SndInfo_t m_Sound)
{
	for (int nIndex = 0; nIndex < m_SoundData.m_CachedSoundData.Count(); nIndex++)
		if (m_Sound.m_nGuid == m_SoundData.m_CachedSoundData[nIndex].m_nGuid)
			return false;

	return true;
}