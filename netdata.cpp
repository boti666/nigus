#include "includes.h"
#include "netdata.h"

PredictionNetvarManager g_predmanager;

void SharedNetvar::PostUpdate(Player* player) {
	if (!player)
		return;

	m_value = *reinterpret_cast<float*>(player + m_offset);
	*reinterpret_cast<float*>(player + m_offset) = get_new(m_old_value, m_value, m_tolerance, g_inputpred.m_had_pred_error);
}

void SharedNetvar::PreUpdate(Player* player) {
	if (!player)
		return;

	m_old_value = *reinterpret_cast<float*>(player + m_offset);
}

void ManagedVec::PreUpdate(Player* player) {
	if (!player)
		return;

	m_old_value = *reinterpret_cast<vec3_t*>(player + m_offset);
}

void ManagedVec::PostUpdate(Player* player) {
	if (!player)
		return;

	m_value = *reinterpret_cast<vec3_t*>(player + m_offset);

	*reinterpret_cast<float*>(player + m_offset) = get_new(m_old_value.x, m_value.x, m_tolerance, g_inputpred.m_had_pred_error);
	*reinterpret_cast<float*>(player + m_offset + 0x4) = get_new(m_old_value.y, m_value.y, m_tolerance, g_inputpred.m_had_pred_error);
	*reinterpret_cast<float*>(player + m_offset + 0x8) = get_new(m_old_value.z, m_value.z, m_tolerance, g_inputpred.m_had_pred_error);
}

void PredictionNetvarManager::PreUpdate(Player* player) {
	if (!player)
		return;

	called_once = true;

	if (player->IsPlayer())
		for (auto& var : m_vars)
			var->PreUpdate(player);
}

void PredictionNetvarManager::PostUpdate(Player* player) {
	if (!player)
		return;

	if (!called_once)
		return;

	if (player->IsPlayer())
		for (auto& var : m_vars)
			var->PostUpdate(player);
}

void PredictionNetvarManager::init(datamap_t* map) {
	if (m_initalized)
		return;

	m_initalized = true;
	ADDVAR(SharedNetvar, g_entoffsets.m_flFallVelocity, (1.f / AssignRangeMultiplier(17, 4096.f - (-4096.f))), "m_flFallVelocity");
	ADDVAR(SharedNetvar, g_entoffsets.m_flStepSize, (1.f / AssignRangeMultiplier(16, 128.f)), "m_flStepSize");
	ADDDATAMAPVAR(SharedNetvar, g_entoffsets.m_flMaxspeed, (1.f / AssignRangeMultiplier(12, 2048.0f)), "m_flMaxspeed");
	ADDVAR(SharedNetvar, g_entoffsets.m_flStamina, (1.f / AssignRangeMultiplier(14, 100.f)), "m_flStamina");
	ADDVAR(SharedNetvar, g_entoffsets.m_flVelocityModifier, (1.f / AssignRangeMultiplier(8, 1.f)), "m_flVelocityModifier");

	// not using ManagedVec here when viewoffset is a vector is because the z value of viewoffset has a different tolerance than the x and y values
	ADDVAR(SharedNetvar, g_entoffsets.m_vecViewOffset, (1.f / AssignRangeMultiplier(8, 32.0f - (-32.0f))), "m_vecViewOffset[0]");
	ADDVAR(SharedNetvar, g_entoffsets.m_vecViewOffset + 0x4, (1.f / AssignRangeMultiplier(8, 32.0f - (-32.0f))), "m_vecViewOffset[1]");
	ADDVAR(SharedNetvar, g_entoffsets.m_vecViewOffset + 0x8, (1.f / AssignRangeMultiplier(10, 128.f)), "m_vecViewOffset[2]");

	//ADDVAR(ManagedVec, "DT_BasePlayer::m_vecBaseVelocity", (1.f / AssignRangeMultiplier(20, 2000.)), "m_vecBaseVelocity", true); // not doing the velocities because i believe
	ADDVARCOORD(ManagedVec, g_entoffsets.m_vecVelocity, 0.031250f, "m_vecVelocity", true);								   // they are networked to us uncompressed
	ADDVARCOORD(ManagedVec, g_entoffsets.m_viewPunchAngle, 0.031250f, "m_viewPunchAngle", true);
	ADDVARCOORD(ManagedVec, g_entoffsets.m_aimPunchAngle, 0.031250f, "m_aimPunchAngle", true);
	ADDVARCOORD(ManagedVec, g_entoffsets.m_aimPunchAngleVel, 0.031250f, "m_aimPunchAngleVel", true);
	//ADDVARCOORD(ManagedVec, g_entoffsets.m_vecNetworkOrigin, 0.031250f, "m_vecNetworkOrigin", true);
}

PredictionNetvarManager::~PredictionNetvarManager() {
	for (auto& i : m_vars)
		i.release();

	m_vars.clear();
}