#pragma once

class ShotRecord
{
public:
	__forceinline ShotRecord(
		const vec3_t& src, Player* target, const int next_shift_amount, const std::ptrdiff_t cmd_num, const float sent_time, const float latency,
		LagRecord* m_record, AimPlayer* data, int hitbox
	) : m_src{ src }, m_target{ target }, m_shot_time{ g_csgo.m_globals->m_realtime },
		m_target_index{ target && target ? target->index() : -1 }, m_next_shift_amount{ next_shift_amount },
		m_sent_time{ cmd_num != -1 ? sent_time : 0.f }, m_cmd_number{ cmd_num != -1 ? cmd_num : -1 }, m_latency{ cmd_num != -1 ? latency : 0.f },
		m_record{ m_record ? m_record : nullptr }, m_data{ data }, m_hitbox{
		hitbox
		} {}

public:
	vec3_t							m_src{};

	Player* m_target{};
	LagRecord* m_record;
	AimPlayer* m_data{};
	std::string                 m_str{};

	int m_hitbox{};
	float							m_shot_time{},
		m_sent_time{}, m_latency{};
	bool							m_processed{};
	int								m_dmg{},
		m_target_index{},
		m_next_shift_amount{},
		m_cmd_number{ -1 }, m_process_tick{};

	struct {
		vec3_t	m_impact_pos{};
		int		m_fire_tick{}, m_hurt_tick{}, m_hitgroup{}, m_dmg{};
	} m_server_info{};
};

class VisualImpactData_t
{
public:
	vec3_t m_impact_pos, m_shoot_pos;
	int    m_tickbase;
	bool   m_ignore, m_hit_player;

public:
	__forceinline VisualImpactData_t(const vec3_t& impact_pos, const vec3_t& shoot_pos, int tickbase) :
		m_impact_pos{ impact_pos }, m_shoot_pos{ shoot_pos }, m_tickbase{ tickbase }, m_ignore{ false }, m_hit_player{ false } {}
};


class HitmarkerWorld_t {
public:
	ImpactInfo m_pos;
	float  m_time;
	float  m_alpha;
};


class Shots
{
public:
	__forceinline ShotRecord* last_unprocessed() {
		if (m_shots.empty())
			return nullptr;

		const auto shot = std::find_if(
			m_shots.begin(), m_shots.end(),
			[](const ShotRecord& shot)
			{
				return !shot.m_processed
					&& shot.m_server_info.m_fire_tick
					&& shot.m_server_info.m_fire_tick == g_csgo.m_cl->m_server_tick;
			}
		);

		return shot == m_shots.end() ? nullptr : &*shot;
	}

	__forceinline void add(
		const vec3_t& src, Player* target, const int next_shift_amount, const std::ptrdiff_t cmd_num, const float sent_time, const float latency, LagRecord* m_record, AimPlayer* data, int hitbox
	) {
		m_shots.emplace_back(src, target, next_shift_amount, cmd_num, sent_time, latency, m_record, data, hitbox);

		if (m_shots.size() < 128)
			return;

		m_shots.pop_front();
	}

public:
	void OnImpact(IGameEvent* evt);
	void OnHurt(IGameEvent* evt);
	void OnWeaponFire(IGameEvent* evt);
	void Think();

public:
	std::array< std::string, 8 > m_groups = {
		XOR("body"),
		XOR("head"),
		XOR("chest"),
		XOR("stomach"),
		XOR("arms"),
		XOR("arms"),
		XOR("legs"),
		XOR("legs")
	};

	__forceinline std::string get_group(int group) {

		std::string ret = "?";


		switch (group) {
		case HITGROUP_HEAD:
			ret = "head";
			break;
		case HITGROUP_NECK:
			ret = "neck";
			break;
		case HITGROUP_CHEST:
			ret = "chest";
			break;
		case HITGROUP_STOMACH:
			ret = "stomach";
			break;
		case HITGROUP_LEFTARM:
			ret = "arms";
			break;
		case HITGROUP_RIGHTARM:
			ret = "arms";
			break;
		case HITGROUP_LEFTLEG:
			ret = "legs";
			break;
		case HITGROUP_RIGHTLEG:
			ret = "legs";
			break;
		default:
			break;
		}

		return ret;
	}

	int m_headtaps{ 0 };
	std::deque< ShotRecord >          m_shots;
	std::vector< VisualImpactData_t > m_vis_impacts;
	std::vector< HitmarkerWorld_t > m_hitmarkers;
};

extern Shots g_shots;
