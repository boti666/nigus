
#pragma once


// #define USE_SAFEPOINTS

enum anim_t : int {
	side_default = 0,
	side_left = 1,
	side_right = 2,
	side_center = 3,
};

class C_AnimationSync {

	using records_t = std::deque< std::shared_ptr< LagRecord > >;
	records_t			m_records;

public:

	std::array<std::array< matrix3x4_t, 128 >, 65> usable_bones{};
	std::array<vec3_t, 65> usable_origin{};

	VelocityDetail_t UpdateVelocity(C_AnimationLayer* animlayers, LagRecord* previous, Player* player);
	void Update(AimPlayer* entry, LagRecord* record, LagRecord* prev);
};

extern C_AnimationSync g_anims;	