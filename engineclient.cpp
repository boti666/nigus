#include "includes.h"

bool& CClientState::m_bIsHLTV()
{
	static DWORD32 m_bIsHLTV = *(int*)(g_csgo.CL_Move + 0x4);

	return *(bool*)((uintptr_t)this + m_bIsHLTV);
}
