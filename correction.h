#pragma once

enum RRModes
{
	R_NONE,
	R_STAND,
	R_MOVE,
	R_AIR
};

enum RModes
{
	STAND_UNMOVED,
	STAND_LBY_UPDATE,
	STAND_LBY_PRE_UPDATE,
	STAND_LBY,
	STAND_DELTA,
	STAND_FREESTAND,
	STAND_LASTMOVE,
	STAND_BACK,
	STAND_1,
	STAND_2,
	AIR_,
	AIR_LOGIC,
	AIR_LBY,
};

struct Resolver_Data
{
	int air_misses{};
	LagRecord move_data{};
	int unknown_misses{};
	bool m_has_moved{};
	bool m_moved{};
	float m_overlap_offset{};
	bool m_has_flicked{ false };
	bool m_has_updated{ false };
	bool m_trigger_timer{};
	float m_old_body{};
	float m_body_yaw{};
	float m_old_body_yaw{};
	float m_body{};
	float m_pre_update_body{};
	float m_body_timer{};
	float m_updated_body{};
	int m_unmoved_misses{};
	float m_anti_fs_angle{};
	bool m_freestand_data{};
	RModes m_mode{};
	int m_stand_index2{};
	int m_stand_index1{};
	int m_reversefs_index{};
	int m_lastmove_index{};
	int m_back_index{};
	int m_delta_misses{};
	float m_body_before_leaving_ground{};
};

class Correction
{
public:
	Resolver_Data data[65];

	void FindBestAngle(Player* player, LagRecord* record);

	float AntiFreestand(Player* player, LagRecord* record, vec3_t start_, vec3_t end, bool include_base, float base_yaw, float delta);
	float GetAwayAngle(LagRecord* record);
	void CorrectPitch(Player* player, LagRecord* record);
	void CorrectStand(Player* player, LagRecord* record);
	void CorrectMove(Player* player, LagRecord* record);
	void CorrectAir(Player* player, LagRecord* record);
	void PredictUpdates(Player* player, LagRecord* record);
	void Run(Player* player, LagRecord* record);
	bool IsYawSideways(Player* entity, float yaw);
};

extern Correction g_correction;