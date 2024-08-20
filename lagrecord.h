#pragma once
// pre-declare.
class LagRecord;

class BackupRecord {
public:
	matrix3x4_t  m_bones[128];
	int        m_bone_count;
	vec3_t     m_origin, m_abs_origin;
	vec3_t     m_mins;
	vec3_t     m_maxs;
	ang_t      m_abs_ang;

public:
	__forceinline void store(Player* player) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache();

		// store bone data.
		std::memcpy(m_bones, cache->m_pCachedBones, sizeof(matrix3x4_t) * 128);

		m_bone_count = cache->m_CachedBoneCount;
		m_origin = player->m_vecOrigin();
		m_mins = player->m_vecMins();
		m_maxs = player->m_vecMaxs();
		m_abs_origin = player->GetAbsOrigin();
		m_abs_ang = player->GetAbsAngles();
	}

	__forceinline void restore(Player* player) {
		// get bone cache ptr.
		CBoneCache* cache = &player->m_BoneCache();

		std::memcpy(cache->m_pCachedBones, m_bones, sizeof(matrix3x4_t) * m_bone_count);
		cache->m_CachedBoneCount = m_bone_count;

		player->m_vecOrigin() = m_origin;
		player->m_vecMins() = m_mins;
		player->m_vecMaxs() = m_maxs;
		//player->SetCollisionBounds(m_mins, m_maxs);
		player->SetAbsAngles(m_abs_ang);
		player->SetAbsOrigin(m_origin);
	}
};

struct anim_side_t {
	std::size_t				m_bones_count{};

	float					m_foot_yaw{}, m_prev_foot_yaw{},
		m_move_yaw_ideal{}, m_move_yaw_cur_to_ideal{},
		m_move_yaw{};

	C_AnimationLayer	m_layers[13]{};
};

enum shot_type_t {
	SHOT_NONE = 0,
	SHOT_REAL,
	SHOT_FAKE
};


enum shift_type_t {
	SHIFT_NONE = 0,
	SHIFT_DEFAULT,
	SHIFT_BREAK_LC
};

enum vel_type_t {
	VEL_TYPE_INVALID = -1,
	VEL_TYPE_NONE,
	VEL_TYPE_LAYER6,
	VEL_TYPE_LAYER6_2,
	VEL_TYPE_LAYER11,
};


enum priority_t {
	PRIORITY_VERY_LOW = 0,
	PRIORITY_LOW,
	PRIORITY_MEDIUM,
	PRIORITY_HIGH,
	PRIORITY_RESOLVED,
	FLICK,
};

enum VelocityDetail_t {
	DETAIL_NONE,
	DETAIL_ZERO,
	DETAIL_RUNNING,
	DETAIL_PERFECT,
	DETAIL_CONSTANT,
	DETAIL_ACCELERATING
};

class LagRecord {
public:
	// data.
	Player* m_player;
	float   m_immune;
	int     m_tick;
	int     m_lag;
	int     m_cl_lag;
	bool    m_dormant;
	bool    m_interpolated;
	float   m_time;
	float   max_speed;
	VelocityDetail_t m_velocity_detail;

	// netvars.
	float  m_sim_time;
	int m_tick_base;
	float  m_old_sim_time;
	int    m_flags;
	vec3_t m_origin;
	vec3_t m_old_origin;
	vec3_t m_velocity, m_anim_velocity;
	vec3_t m_og_velocity;
	vec3_t m_mins;
	vec3_t m_maxs;
	ang_t  m_eye_angles;
	ang_t  m_abs_ang;
	float  m_body;
	float m_anim_speed;
	float  m_duck;

	// anim stuff.
	C_AnimationLayer m_layers[13]{};
	float m_poses[24]{};
	float            m_max_speed;
	float            m_scale_vel;
	int              m_anim_vel_type;
	float			 m_foot_yaw{ 0.f };
	float			 m_last_foot_yaw{ 0.f };
	float			 m_move_yaw{ 0.f };
	float			 m_move_yaw_cur_to_ideal{ 0.f };
	float			 m_move_yaw_ideal{ 0.f };
	float	         m_move_weight_smoothed{ 0.f };
	int              m_tick_count_delta;
	int              m_server_delta;

	// bone stuff.
	bool       m_setup;
	matrix3x4_t m_bones[128];
	matrix3x4_t m_visual_bones[128];


	// lagfix stuff.
	bool   m_broke_lc;
	vec3_t m_pred_origin;
	vec3_t m_pred_velocity;
	float  m_pred_time;
	int    m_pred_flags;
	bool   m_extrapolated;

	// resolver stuff.
	int m_method;
	size_t		m_mode;
	size_t		m_last_mode;
	bool m_invalid;
	int		m_flag;
	int         m_priority;
	std::string m_resolver_mode;
	std::string m_resolver_chance;
	bool		m_fake_walk;
	bool		m_fake_flick;
	bool		m_unsafe_vel;
	int         m_shot_type;
	float		m_anim_time;
	bool		m_has_seen_delta_35;
	bool        m_is_invalid;
	bool		m_ground_for_two_ticks;
	float       m_last_body_delta;
	float m_lag_time;
	bool   m_shot, m_fixing_pitch, m_sim_tick;
	bool        m_has_been_missed;
	float       m_back;
	float       m_air_back;
	std::array< anim_side_t, 4u >	m_anim_sides{};
	std::optional< bool >			m_jumped{};
	float                           m_on_ground_time{};


	// other stuff.
	float  m_interp_time;
	Weapon* m_weapon;
public:

	__forceinline int shift_type() {
		int ret = shift_type_t::SHIFT_NONE;

		// most people dont shift with fakelag so it's useless
		if (m_lag > 3)
			return ret;

		// if delta too high or delta low but he's teleporting
		if (m_server_delta <= -10 || this->m_sim_time < this->m_old_sim_time)
			ret = shift_type_t::SHIFT_BREAK_LC;
		// delta but not breaking lc ( normal shift )
		else if (m_server_delta < -1)
			ret = shift_type_t::SHIFT_DEFAULT;

		return ret;
	}

	// default ctor.
	__forceinline LagRecord() :
		m_setup{ false },
		m_broke_lc{ false },
		m_fake_walk{ false },
		m_invalid{ false },
		m_has_seen_delta_35{ false },
		m_is_invalid{ false },
		m_extrapolated{ false },
		m_shot{ false },
		m_has_been_missed{ false },
		m_interpolated{ false },
		m_shot_type{ 0 },
		m_back{ 0 },
		m_air_back{ 0 },
		m_priority{ 0 },
		m_on_ground_time{ 0.f },
		m_max_speed{ 250.f },
		m_scale_vel{ -1.f },
		m_resolver_chance{ "NONE" },
		m_resolver_mode{ "NONE" },
		m_lag{ 1 },
		m_mode{ 0 },
		m_flag{ 0 },
		m_cl_lag{ 1 },
		m_anim_vel_type{ 0 },
		m_tick_count_delta{ 0 },
		m_server_delta{ 0 },
		m_lag_time{ 0.f },
		m_method{ 0 },
		m_last_body_delta{ 0 } {}

	// ctor.
	__forceinline LagRecord(Player* player) :
		m_setup{ false },
		m_broke_lc{ false },
		m_fake_walk{ false },
		m_has_seen_delta_35{ false },
		m_is_invalid{ false },
		m_extrapolated{ false },
		m_shot{ false },
		m_interpolated{ false },
		m_resolver_chance{ "NONE" },
		m_invalid{ false },
		m_resolver_mode{ "NONE" },
		m_shot_type{ 0 },
		m_priority{ 0 },
		m_back{ 0 },
		m_air_back{ 0 },

		m_mode{ 0 },
		m_flag{ 0 },
		m_scale_vel{ -1.f },
		m_on_ground_time{ 0.f },
		m_tick_count_delta{ 0 },
		m_server_delta{ 0 },
		m_max_speed{ 250.f },
		m_lag{ 1 },
		m_cl_lag{ 1 },
		m_anim_vel_type{ 0 },
		m_lag_time{ 0.f },
		m_method{ 0 },
		m_last_body_delta{ 0 } {

		store(player);
	}

	__forceinline void invalidate() {

		// mark as not setup.
		m_setup = false;
	}

	// function: allocates memory for SetupBones and stores relevant data.
	void store(Player* player) {

		// player data.
		m_player = player;
		m_immune = player->m_fImmuneToGunGameDamageTime();
		m_tick = g_csgo.m_cl->m_server_tick;
		m_time = g_csgo.m_globals->m_realtime;
		m_tick_base = player->m_nTickBase();

		// netvars.
		m_pred_time = m_sim_time = player->m_flSimulationTime();
		m_old_sim_time = player->m_flOldSimulationTime();
		m_lag_time = m_sim_time - m_old_sim_time; // should be lagcomp time ?
		m_pred_flags = m_flags = player->m_fFlags();
		m_pred_origin = m_origin = player->m_vecOrigin();
		m_old_origin = player->m_vecOldOrigin();
		m_eye_angles = player->m_angEyeAngles();
		m_abs_ang = player->GetAbsAngles();
		m_body = player->m_flLowerBodyYawTarget();
		m_mins = player->m_vecMins();
		m_maxs = player->m_vecMaxs();
		m_duck = player->m_flDuckAmount();
		m_og_velocity = m_pred_velocity = m_velocity = player->m_vecVelocity();
		m_weapon = player->GetActiveWeapon();
		m_scale_vel = m_velocity.length_2d();

		m_player->GetAnimLayers(m_layers);

		// normalize eye angles.
		m_eye_angles.z = 0.0f;
		m_eye_angles.normalize();
		math::clamp(m_eye_angles.x, -90.f, 90.f);

		// get lag.
		m_lag = game::TIME_TO_TICKS(m_sim_time - m_old_sim_time);

		// compute animtime.
		m_anim_time = m_old_sim_time + g_csgo.m_globals->m_interval;
		m_server_delta = game::TIME_TO_TICKS(m_sim_time) - g_csgo.m_cl->m_server_tick;
		m_tick_count_delta = game::TIME_TO_TICKS(m_sim_time) - g_csgo.m_globals->m_tick_count;
	}

	// function: restores 'predicted' variables to their original.
	__forceinline void predict() {
		m_method = 0;
		m_pred_origin = m_origin;
		m_pred_velocity = m_velocity;
		m_pred_time = m_sim_time;
		m_pred_flags = m_flags;
	}

	// function: writes current record to bone cache.
	__forceinline void cache() {
		// get bone cache ptr.
		CBoneCache* cache = &m_player->m_BoneCache();

		std::memcpy(cache->m_pCachedBones, m_bones, sizeof(matrix3x4_t) * cache->m_CachedBoneCount);

		m_player->m_vecOrigin() = m_pred_origin;
		m_player->m_vecMins() = m_mins;
		m_player->m_vecMaxs() = m_maxs;

		m_player->SetAbsAngles(m_abs_ang);
		m_player->SetAbsOrigin(m_pred_origin);
	}


	__forceinline bool dormant() {
		return m_dormant;
	}

	__forceinline bool immune() {
		return m_immune > 0.f;
	}

	__forceinline bool lagcomp() {
		return (m_origin - m_old_origin).length_sqr() > 4096.f;
	}

	// function: checks if LagRecord obj is hittable if we were to fire at it now.
	__forceinline bool valid()
	{
		const auto nci = g_csgo.m_engine->GetNetChannelInfo();
		if (!nci)
			return false;

		float correct = 0.0f;

		correct += nci->GetLatency(INetChannel::FLOW_OUTGOING) + nci->GetLatency(INetChannel::FLOW_INCOMING);
		correct += g_cl.m_lerp;
		correct = std::clamp(correct, 0.0f, g_csgo.sv_maxunlag->GetFloat());

		float delta_time = correct - (g_csgo.m_globals->m_curtime - this->m_sim_time);

		if (std::fabs(delta_time) > 0.2f)
			return false;
		
		return true;

	/*	const auto last_server_tick = game::TIME_TO_TICKS(g_csgo.m_engine->GetLastTimestamp());

		const auto rtt = nci->GetLatency(INetChannel::FLOW_OUTGOING) + nci->GetLatency(INetChannel::FLOW_INCOMING);
		const auto possible_future_tick = last_server_tick + game::TIME_TO_TICKS(rtt) + 8;

		float correct = 0.f;
		correct += rtt;
		correct += g_cl.m_lerp;

		const auto deadtime = static_cast<int>(game::TICKS_TO_TIME(last_server_tick) + rtt - g_csgo.sv_maxunlag->GetFloat());
		if (this->m_sim_time <= static_cast<float>(deadtime) || game::TIME_TO_TICKS(this->m_sim_time + g_cl.m_lerp) > possible_future_tick)
			return false;

		correct = std::clamp(correct, 0.f, g_csgo.sv_maxunlag->GetFloat());
		const auto delta_time = correct - (time - this->m_sim_time);

		return std::fabsf(delta_time) < 0.2f;*/
	}
};