#pragma once

class AdaptiveAngle {
public:
	float m_yaw;
	float m_dist;
	int idx;

public:
	// ctor.
	__forceinline AdaptiveAngle(float yaw, float penalty = 0.f, int index = 0) {
		// set yaw.
		m_yaw = math::NormalizedAngle(yaw);

		// init distance.
		m_dist = 0.f;

		// remove penalty.
		m_dist -= penalty;

		// set index.
		idx = index;
	}
};

enum AntiAimMode : size_t {
	STAND = 0,
	WALK,
	AIR,
};

enum freestanding_side : size_t {
	SIDE_NONE = 0,
	SIDE_LEFT = 1,
	SIDE_RIGHT = 2,
	SIDE_BACK = 3,
};


class HVH {
public:
	size_t m_mode;
	int    m_pitch;
	int    m_yaw;
	float  m_jitter_range;
	float  m_rot_range;
	float  m_rot_speed;
	float  m_rand_update;
	int    m_dir;
	float  m_dir_custom;
	size_t m_base_angle;
	float  m_auto_time;
	bool m_allow_manuals;
	bool   m_step_switch;
	int    m_random_lag;
	float  m_next_random_update;
	float  m_random_angle;
	float  m_direction;
	float  m_auto;
	float m_last_real, m_last_fake;
	bool m_swap;
	float  m_auto_dist;
	float  m_auto_last;
	float  m_view;
	float m_last_lby_break{ 0.0f }, m_last_lby_twist{ 0.0f };
	bool m_pre_flick;
	int m_lby_switch_side;
	bool m_manual_on;
	bool   m_left, m_right, m_back, m_forward;
	bool m_in_use;
	bool m_anti_last_move;
	int m_freestanding_side{ 0 };

	int   m_step;
	int   m_step_ticks;

public:
	//void IdealPitch( );
	void AntiAimPitch();
	void AutoDirection();
	void GetAntiAimDirection();
	bool DoEdgeAntiAim(Player* player, ang_t& out);
	void DoRealAntiAim();
	void DoFakeAntiAim();
	void AntiAim();
	//bool peek_fakelag();
	//void simulate_movement(Player* player1, Player* player2);
	//void PlayerExtrapolation(Player* player, vec3_t& origin, vec3_t& velocity, int& flags, bool on_ground, int ticks, Player* target, bool simulate, bool choke, Color color);
	//void OnPeek(Player* player);
	//bool Peeking();
	void SendPacket();
	//bool OnPeek();
	bool IsDefusing();

};

extern HVH g_hvh;
