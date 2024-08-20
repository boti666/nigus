#include "includes.h"
struct mstudioposeparamdesc_t1 {
	int sznameindex;
	inline char* const pszName(void) const { return ((char*)this) + sznameindex; }
	int flags;   // ???? ( volvo, really? )
	float start; // starting value
	float end;   // ending value
	float loop;  // looping range, 0 for no looping, 360 for rotations, etc.
};

__forceinline mstudioposeparamdesc_t1* pPoseParameter(CStudioHdr* hdr, int index) {
	using poseParametorFN = mstudioposeparamdesc_t1 * (__thiscall*)(CStudioHdr*, int);
	static auto p_pose_parameter = pattern::find(g_csgo.m_client_dll, "55 8B EC 8B 45 08 57 8B F9 8B 4F 04 85 C9 75 15 8B").as< poseParametorFN>();
	return p_pose_parameter(hdr, index);
}

void animstate_pose_param_cache_t::SetValue(Player* player, float flValue)
{
	auto hdr = player->m_studioHdr();
	if (hdr) {
		auto pose_param = pPoseParameter(hdr, m_nIndex);
		if (!pose_param)
			return;

		auto PoseParam = *pose_param;

		if (PoseParam.loop) {
			float wrap = (PoseParam.start + PoseParam.end) / 2.0f + PoseParam.loop / 2.0f;
			float shift = PoseParam.loop - wrap;

			flValue = flValue - PoseParam.loop * std::floorf((flValue + shift) / PoseParam.loop);
		}

		auto ctlValue = (flValue - PoseParam.start) / (PoseParam.end - PoseParam.start);
		player->m_flPoseParameter()[m_nIndex] = ctlValue;
	}
}


void Player::UpdateClientSideAnimationEx()
{
	auto backup = this->m_bClientSideAnimation();
	auto ishltv = g_csgo.m_cl->m_bIsHLTV();
	g_hooks.m_bUpdatingCSA[this->index()] = true;
	this->m_bClientSideAnimation() = true;
	g_csgo.m_cl->m_bIsHLTV() = true; // disable velocity and duck amount interpolation
	this->UpdateClientSideAnimation();
	this->m_bClientSideAnimation() = backup;
	g_csgo.m_cl->m_bIsHLTV() = ishltv;
	g_hooks.m_bUpdatingCSA[this->index()] = false;
}
