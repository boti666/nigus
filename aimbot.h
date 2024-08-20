#pragma once

enum HitscanMode : int {
	NORMAL = 0,
	LETHAL = 1,
	LETHAL2 = 3,
	PREFER = 4
};

enum HitchanceGroup_t : int {
	GROUP_INVALID = -1,
	GROUP_HEAD = 0,
	GROUP_BODY = 1,
	GROUP_LEFTARM = 2,
	GROUP_RIGHTARM = 3,
	GROUP_LEFTLEG = 3,
	GROUP_RIGHTLEG = 4
};

enum IntersectionType_t {
	INTERSECT_NONE = 0,
	INTERSECT_LOW = 1,
	INTERSECT_MEDIUM = 2,
	INTERSECT_HIGH = 3,
	INTERSECT_SAFE = 4,
};

struct MoveData_t {
	float m_time;
	float m_body;
	float m_sim_time;
	float m_anim_time;
	int m_server_tick;
	vec3_t m_origin;
};

struct AnimationBackup_t {
	vec3_t           m_origin, m_abs_origin;
	vec3_t           m_velocity, m_abs_velocity;
	ang_t m_abs_angle;
	int              m_flags, m_eflags;
	float            m_duck, m_body;
	C_AnimationLayer m_layers[13];
	float m_poses[24];
};

struct HitscanData_t {
	float  m_damage;
	vec3_t m_pos;
	int m_hitbox, m_hitgroup;
	bool   m_center;
	bool m_lethal;

	__forceinline HitscanData_t() : m_damage{ 0.f }, m_pos{}, m_hitbox{0}, m_lethal{ false }, m_hitgroup{0} {}
};

struct point_t {
	vec3_t m_pos;
	int    m_type;

	enum type_t : int {
		TYPE_DEFAULT = 0,
		TYPE_PREFERED,
		TYPE_SAFE
	};

	__forceinline point_t() : m_pos{}, m_type{ type_t::TYPE_DEFAULT } {}
	__forceinline point_t(vec3_t pos, int type = type_t::TYPE_DEFAULT) : m_pos{ pos }, m_type{ type } {}

	__forceinline int calc_needed_intersects() {

		int needed = 0;

		switch (m_type) {
		case TYPE_DEFAULT:
			needed = 1;
			break;
		case TYPE_PREFERED:
			needed = 1;
			break;
		case TYPE_SAFE:
			needed = 3;
			break;
		}

		return needed;
	}
};

struct HitscanBox_t {
	int         m_index;
	HitscanMode m_mode;

	__forceinline bool operator==(const HitscanBox_t& c) const {
		return m_index == c.m_index && m_mode == c.m_mode;
	}
};

struct interpolated_t {
	__forceinline constexpr interpolated_t() = default;

	__forceinline constexpr interpolated_t(
		const float sim_time, const float duck_amount,
		const int flags, const vec3_t& velocity
	) : m_sim_time{ sim_time }, m_duck_amount{ duck_amount },
		m_flags{ flags }, m_velocity{ velocity } {}

	float				m_sim_time{},
		m_duck_amount{};

	int			    	m_flags{};
	vec3_t				m_velocity{};

};

class AimPlayer {
public:
	using records_t = std::deque< std::shared_ptr< LagRecord > >;
	using hitboxcan_t = stdpp::unique_vector< HitscanBox_t >;

public:

	records_t m_records;
	bool m_delayed;
	BoneArray						m_visual_bones[CS_MAX_BONES];
	std::vector< interpolated_t >	m_interpolated{};

	Player* m_player;
	float	  m_spawn;

	// aimbot data.
	hitboxcan_t m_hitboxes;

	float m_anti_fs_angle;
	bool freestand_data;

	// resolve data.
	int       m_shots;
	bool m_should_target{ false };
	float m_last_rate, m_last_cycle;
	float m_last_time;
	float m_last_duration_in_air;
	float m_last_jump_fall;
	bool  m_last_prev_ground;
	bool  m_last_prev_ladder;
	bool      m_body_updated;
	bool      m_moved;
	bool m_triggered_late;
	bool m_triggered_979;
	int       m_body_index;
	int		  m_air_index;
	int		  m_stand_index;
	int m_body_pred_index;
	int m_high_lby_idx;
	int m_low_lby_idx;
	int m_air_brute_index;
	int m_change_stored;
	float m_last_stored_body;
	bool m_has_ever_updated;
	bool m_missed_invertfs;
	bool m_missed_back;
	bool m_missed_last;
	bool m_missed_air_back;
	bool m_prefer_body;
	bool m_prefer_head;
	bool m_hit{ false };
	float     m_last_normal_pitch;
	LagRecord m_walk_record;
	bool m_shot_time;

	// data about the LBY proxy.
	float m_body;
	float m_old_body;
	float m_body_timer;
	float m_delta_1;
	int m_last_lby_activity;
	int m_detected_body_upd;
	bool m_updated;
	float m_delta_2;
	float m_overlap_offset = 0.f;
	bool m_update_captured;
	bool m_has_updated;
	int       m_update_count;
	float m_last_body;
	int m_old_stand_move_idx;
	int m_old_stand_no_move_idx;
	int m_stand_no_move_idx;
	int m_stand_move_idx;
	float m_time_since_something;
	int m_body_updated_idk;
	float m_upd_time;
	bool m_override{ false };
	float m_override_angle;
	int m_body_idx;
	int m_air_idx;

	// whitelist xd
	bool m_ignore;
		
	// we store our move data here.
	MoveData_t m_move_data;
public:
	void AimwareExtrapolation(Player* player, vec3_t& origin, vec3_t& velocity, int& flags, bool on_ground);
	void OnNetUpdate(Player* player);
	bool FixVelocity(vec3_t& vel, C_AnimationLayer* animlayers, const vec3_t& origin);
	void OnRoundStart(Player* player);
	void SetupHitboxes(LagRecord* record);
	bool SetupHitboxPoints(LagRecord* record, matrix3x4_t* bones, int index, std::vector< vec3_t >& points);
	bool GetBestAimPosition(vec3_t& aim, float& damage, int& hitbox, int& hitgroup, LagRecord* record);
public:

	void reset() {
		m_player = nullptr;
		m_spawn = 0.f;
		m_shots = 0;
		m_last_stored_body = -999.f;
		m_high_lby_idx = 0;
		m_low_lby_idx = 0;
		m_body_idx = 0;
		m_air_idx = 0;
		m_shots        = 0;
		m_old_stand_move_idx = 0;
		m_old_stand_no_move_idx = 0;
		m_body_index = 0;
		m_air_brute_index = 0;
		m_stand_move_idx = m_stand_no_move_idx = 0;
		m_move_data.m_sim_time = -1.f;
		m_update_captured = 0;
		m_delta_1 = 90.f;
		m_overlap_offset = 0.f;
		m_body_timer = FLT_MAX;
		m_has_updated = 0;
		m_last_body = -FLT_MAX;
		m_update_count = 0;
		m_upd_time = -FLT_MAX;
		m_moved = false;
		m_change_stored = 0;
		m_last_time = m_last_rate = m_last_cycle = -1.f;
		m_last_stored_body = -999.f;

		m_missed_last = false; 
		m_missed_invertfs = false;
		m_missed_back = false;
		m_missed_air_back = false;

		m_last_prev_ladder = false;
		m_last_prev_ground = true;
		m_last_duration_in_air = 0.f;


		m_records.clear();
		m_hitboxes.clear();
	}
	void SetOverwriteTick(Player* player, ang_t angles, float_t correct_time, LagRecord* record);
};

class Aimbot {
private:
	struct target_t {
		Player* m_player;
		AimPlayer* m_data;
	};

	struct knife_target_t {
		target_t  m_target;
		LagRecord m_record;
	};

	struct table_t {
		uint8_t swing[2][2][2]; // [ first ][ armor ][ back ]
		uint8_t stab[2][2];		  // [ armor ][ back ]
	};

	const table_t m_knife_dmg{ { { { 25, 90 }, { 21, 76 } }, { { 40, 90 }, { 34, 76 } } }, { { 65, 180 }, { 55, 153 } } };

public:
	std::array< AimPlayer, 64 > m_players;
	std::vector< AimPlayer* >   m_targets;

	BackupRecord m_backup[64];

	// found target stuff.
	float m_best_damage;
	float m_best_dist;
	Player* m_target;
	AimPlayer* m_override_target;
	ang_t      m_angle;
	vec3_t     m_aim;
	int        m_hitbox, m_hitgroup;
	float      m_damage;
	LagRecord* m_record;
	float      m_hit_chance, m_hit_chance_wall;
	float      m_shot_hitchance;

	// rage stuff.
	bool       m_fake_latency, m_fake_latency2;
	bool	   m_override{ false };
	bool m_just_headshotted{ false };
	int        m_override_side{ 0 };
	bool	   m_damage_toggle;
	bool	   m_force_body;
	int m_goal_damage;
	bool m_double_tap;

	bool m_stop{ false };
	bool m_found_record;
	bool m_stop_air{ false };
	int  m_total_scanned;
	bool working = false;
	float latency_amount;
public:

	void handle_fakelatency(int tick_count, bool secondary, float secondary_val, float primary_val) {
		if (tick_count % 2 != 0)
			return;

		if (secondary) {
			if (latency_amount > secondary_val) {
				latency_amount = secondary_val;
				return;
			}

			if (latency_amount < secondary_val)
				latency_amount += 10.f;
		}
		else
			latency_amount = primary_val;
	}

	__forceinline void reset() {
		// reset aimbot data.
		init();

		// reset all players data.
		for (auto& p : m_players)
			p.reset();
	}

	__forceinline bool IsValidTarget(Player* player) {
		if (!player)
			return false;

		if (!player->IsPlayer())
			return false;

		if (player->index() > 64 || player->index() <= 0)
			return false;

		if (!player->GetClientClass())
			return false;

		if (!player->alive())
			return false;

		if (player->m_bIsLocalPlayer())
			return false;

		if (!player->enemy(g_cl.m_local))
			return false;

		if (player->dormant())
			return false;

		if (player->m_fImmuneToGunGameDamageTime())
			return false;

		return true;
	}

public:
	
	bool CheckHitchance(Player* player, const ang_t& angle, int hitbox, LagRecord* record, float* output);
	// aimbot.
	void init();
	void StripAttack();
	void think();
	void find();
	void apply();
	void StartTargetSelection();
	void FinishTargetSelection();
	bool SelectTarget(LagRecord* record, const vec3_t& aim, float damage);

	int CalcHitchance(Player* target, const ang_t& angle);

	__forceinline int hitbox_to_group(int hitbox) {

		int ret = HitchanceGroup_t::GROUP_INVALID;

		switch (hitbox) {
		case HITBOX_HEAD:
		case HITBOX_NECK:
			ret = HitchanceGroup_t::GROUP_HEAD;
			break;
		case HITBOX_LOWER_NECK:
		case HITBOX_THORAX:
		case HITBOX_UPPER_CHEST:
		case HITBOX_CHEST:
		case HITBOX_BODY:
		case HITBOX_PELVIS:
			ret = HitchanceGroup_t::GROUP_BODY;
			break;
		case HITBOX_L_UPPER_ARM:
		case HITBOX_L_FOREARM:
		case HITBOX_L_HAND:
			ret = HitchanceGroup_t::GROUP_LEFTARM;
			break;
		case HITBOX_R_UPPER_ARM:
		case HITBOX_R_FOREARM:
		case HITBOX_R_HAND:
			ret = HitchanceGroup_t::GROUP_RIGHTARM;
			break;
		case HITBOX_L_CALF:
		case HITBOX_L_THIGH:
		case HITBOX_L_FOOT:
			ret = HitchanceGroup_t::GROUP_LEFTLEG;
			break;
		case HITBOX_R_CALF:
		case HITBOX_R_THIGH:
		case HITBOX_R_FOOT:
			ret = HitchanceGroup_t::GROUP_RIGHTLEG;
			break;
		default:
			break;
		}

		return ret;
	}

	__forceinline int ToHitGroup(int hitbox) {

		int ret = HITGROUP_GENERIC;

		switch (hitbox) {
		case HITBOX_HEAD:
		case HITBOX_NECK:
		case HITBOX_LOWER_NECK:
			ret = HITGROUP_HEAD;
			break;
		case HITBOX_L_FOREARM:
		case HITBOX_L_UPPER_ARM:
		case HITBOX_L_HAND:
			ret = HITGROUP_LEFTARM;
			break;
		case HITBOX_R_FOREARM:
		case HITBOX_R_UPPER_ARM:
		case HITBOX_R_HAND:
			ret = HITGROUP_RIGHTARM;
			break;
		case HITBOX_CHEST:
		case HITBOX_THORAX:
		case HITBOX_UPPER_CHEST:
			ret = HITGROUP_CHEST;
			break;
		case HITBOX_BODY:
		case HITBOX_PELVIS:
			ret = HITGROUP_STOMACH;
			break;
		case HITBOX_L_THIGH:
		case HITBOX_R_THIGH:
		case HITBOX_L_CALF:
		case HITBOX_R_CALF:
		case HITBOX_L_FOOT:
		case HITBOX_R_FOOT:
			ret = HITGROUP_LEGS;
			break;
		}

		return ret;
	}

	// knifebot.
	void knife();
	bool CanKnife(LagRecord* record, ang_t angle, bool& stab);
	bool KnifeTrace(vec3_t dir, bool stab, CGameTrace* trace);
	bool KnifeIsBehind(LagRecord* record);
};

extern Aimbot g_aimbot;
