#pragma once

struct pen_data_t {
	__forceinline pen_data_t() = default;
	__forceinline pen_data_t(int dmg, int hitbox, int group, int pen) : m_dmg{ dmg }, m_hitbox{ hitbox }, m_hit_group{ group }, m_remaining_pen{ pen } {}

	int m_dmg{}, m_hitbox{}, m_hit_group{}, m_remaining_pen{};
};

class c_auto_wall {
public:
	pen_data_t simulate_fire(const vec3_t& shoot_pos, const vec3_t& point, Player* enemy, bool strict = false, int hitgroup = -1);
	void scale_dmg(Player* player, CGameTrace& trace, WeaponInfo* wpn_info, float& cur_dmg, const int hit_group);

	bool handle_bullet_penetration(
		WeaponInfo* wpn_data, CGameTrace& enter_trace, vec3_t& eye_pos, const vec3_t& direction, int& possible_hits_remain,
		float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration
	);

	void ClipTraceToPlayer(const vec3_t& src,
		const vec3_t& dst,
		CGameTrace& trace,
		Player* const player,
		ShouldHitFunc_t should_hit_fn);

	bool fire_bullet(Weapon* wpn, const vec3_t& direction, bool& visible, float& cur_dmg, int& remaining_pen, int& hit_group,
		int& hitbox, Player* entity, float length, const vec3_t& pos,
		bool strict = false,
		int hitgroup = -1);

	bool trace_to_exit_short(vec3_t& point, vec3_t& dir, const float step_size, float max_distance);
	float get_thickness(vec3_t& start, vec3_t& end, float distance);
};

inline const auto g_auto_wall = std::make_unique < c_auto_wall >();