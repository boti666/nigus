#pragma once
#include "CPlayerResource.h"

class Sequence {
public:
	float m_time;
	int   m_state;
	int   m_seq;

public:
	__forceinline Sequence() : m_time{}, m_state{}, m_seq{} {};
	__forceinline Sequence(float time, int state, int seq) : m_time{ time }, m_state{ state }, m_seq{ seq } {};
};

class modifier_table_t
{
private:
	uint16_t* find(uint16_t* symbol, char* lookup)
	{
		static auto find_fn = pattern::find(g_csgo.m_client_dll, "E8 ? ? ? ? 0F B7 F0 66 3B F7 75 12").rel32(1).as<uint16_t * (__thiscall*)(void*, uint16_t*, char*)>();
		return find_fn(this, symbol, lookup);
	}

	uint16_t* add_string(uint16_t* symbol, char* lookup)
	{
		static auto add_fn = pattern::find(g_csgo.m_client_dll, "E8 ? ? ? ? 8B 0F 8B D3").rel32(1).as<uint16_t * (__thiscall*)(void*, uint16_t*, char*)>();
		return add_fn(this, symbol, lookup);
	}

public:
	uint16_t find_or_insert(char* lookup)
	{
		uint16_t symbol = 0xFFFF;
		find(&symbol, lookup);
		if (symbol == 0xFFFF)
			symbol = *add_string(&symbol, lookup);
			
		return symbol;
	}
};

class activity_modifiers_wrapper
{
private:
	void add_activity_modifier(const char* name)
	{
		static auto add_modifier = pattern::find(g_csgo.m_server_dll, "55 8B EC 8B 55 08 83 EC 24 56 8B F1 85 D2 0F 84 ? ? ? ? 8D 45 DC").as<void(__thiscall*)(void*, const char*)>();
		add_modifier(this, name);
	}
	
	PAD(0x148);
	CUtlVector<uint16_t> modifiers{};

public:
	activity_modifiers_wrapper() = default;

	explicit activity_modifiers_wrapper(CUtlVector<uint16_t> current_modifiers)
	{
		modifiers.RemoveAll();
		modifiers.GrowVector(current_modifiers.Count());

		for (auto i = 0; i < current_modifiers.Count(); i++)
			modifiers[i] = current_modifiers[i];
	}

	void add_modifier(const char* name)
	{
		add_activity_modifier(name);
	}

	CUtlVector<uint16_t> get() const
	{
		return modifiers;
	}
};


class NetPos {
public:
	float  m_time;
	vec3_t m_pos;

public:
	__forceinline NetPos() : m_time{}, m_pos{} {};
	__forceinline NetPos(float time, vec3_t pos) : m_time{ time }, m_pos{ pos } {};
};

class Client {
public:
	// hack thread.
	static ulong_t __stdcall init(void* arg);

	void UnlockHiddenConvars();

	void StartMove(CUserCmd* cmd);
	void EndMove(CUserCmd* cmd);
	void BackupPlayers(bool restore);
	void DoMove();
	void DrawHUD();
	void KillFeed();

	void UpdateAnimations();

	void update_shot_cmd();
	void update_viewangles();
	void HandleBodyYaw();
	void UpdateLocalMatrix();
	void HandleUpdates();
	void HandleAnimations();
	void UpdateViewmodel();
	void RestoreData();
	CUtlVector<uint16_t> build_activity_modifiers(Player* player);
	std::pair<CCSGOPlayerAnimState, C_AnimationLayer*> predict_animation_state(Player* player);
	void try_initiate_animation(Player* player, size_t layer, int32_t activity, CUtlVector<uint16_t> modifiers);

	void Cvars();

	void OnPaint();
	void OnMapload();
	void OnTick(CUserCmd* cmd);

	// debugprint function.
	void print(const std::string text, ...);

	void printcustom(Color color, const std::string text, ...);

	// check if we are able to fire this tick.
	void print_clr(Color color, const std::string text, ...);
	bool CanFireWeapon( float curtime );
	void UpdateRevolverCock();
	void UpdateIncomingSequences();

public:
	float adjust_weight;
	// local player variables.
	Player* m_local;
	bool	         m_processing;
	int last_cmd_num;
	std::vector< int32_t > m_cmds;
	bool	         m_setupped, m_updated_values;
	int	             m_flags;
	std::deque< int > m_outgoing_cmd_nums = {};
	int vm;
	int m_old_flags;
	bool             m_hit_floor;
	vec3_t	         m_shoot_pos;
	vec3_t	         m_shoot_pos_xt;
	bool	         m_player_fire;
	bool	         m_shot;
	bool	         m_old_shot;
	float            m_abs_yaw;
	float m_poses[24]{ };
	C_AnimationLayer m_layers[13];
	C_AnimationLayer m_last_queued_layers[13];
	float            m_last_in_air_time;
	quaternion_t	bonerotations[256];

	float LastJumpFall;

	float            m_backup_poses[24];
	float m_old_duck;
	bool m_frame_changed{ true };
	int m_old_frame{ -1 };
	C_AnimationLayer m_backup_layers[13];
	bool m_should_run{ false };
	vec3_t           m_last_sent_origin;
	float            m_upd_time;
	int              m_upd_tick;
	bool			 m_update_sound;
	float            m_spawn_time;
	bool             m_has_updated;
	int              m_upd_time_test;
	float            m_last_sim_time;
	bool             m_real_update;

	// hit saynd
	std::vector<std::string> m_hitsounds;
	std::vector<std::string> m_menu;

	std::vector< int > m_saved_command;

	ang_t            m_real_angle;
	ang_t            m_real_angle_old;
	ang_t            m_flick_angle;
	float m_moving_body;
	bool  m_awall_hit;
	bool  m_awall_pen;
	bool             m_pressing_move;

	// active weapon variables.
	Weapon* m_weapon;
	int         m_weapon_id;
	WeaponInfo* m_weapon_info;
	int         m_weapon_type;
	vec3_t m_unpredicted_vel;
	bool        m_weapon_fire;

	// revolver variables.
	int	 m_revolver_cock;
	int	 m_revolver_query;
	bool m_revolver_fire;

	// general game varaibles.
	bool     m_round_end;
	Stage_t	 m_stage;
	int	     m_max_lag;
	int      m_lag;
	int	     m_old_lag;
	bool     m_should_try_upd;
	bool* m_packet;
	bool* m_final_packet;
	bool	 m_old_packet;
	float	 m_lerp;
	float    m_latency;
	float    m_latency2;
	int      m_latency_ticks;
	int      m_server_tick;
	const char* m_server_ip;
	int      m_arrival_tick;
	int      m_width, m_height;
	bool m_sideways;
	// usercommand variables.
	CUserCmd* m_cmd;
	int shot_cmd{ };
	int	      m_tick;
	int	      m_buttons;
	int       m_old_buttons;
	ang_t     m_view_angles;
	ang_t	  m_strafe_angles;
	bool m_allow_client_to_update;
	vec3_t	  m_forward_dir;
	bool		m_override_active = false;

	std::deque< Sequence > m_sequences;
	std::deque< NetPos >   m_net_pos;
	BoneArray        m_local_bones[128];

	// animation variables.
	ang_t  m_angle;
	ang_t  m_rotation;
	ang_t  m_orig_ang;
	ang_t  m_radar;
	float  m_body;
	float  m_body_pred;
	float  m_speed;
	float  m_anim_time;
	float  m_anim_frame;
	bool   m_has_flicked{ false };
	bool   m_pen;
	float  m_max_lat;
	bool   m_flicking{ false };
	float m_spawn;
	int    m_frame_shit;
	bool   m_ground;
	bool   m_lagcomp;
	bool   m_old_lagcomp;
	bool m_enable_voice;

	// shootpos stuff
	matrix3x4_t m_shoot_matrix[128];

	float m_flLastUpdateTime;
	float m_flLastUpdateIncrement;
	bool  m_bLandedOnGroundThisFrame;
	bool  m_bLeftTheGroundThisFrame;
	bool  m_bOnLadder;
	bool  m_bOnGround;
	bool  bPreviousGroundState;
	bool  bPreviouslyOnLadder;
	bool  bStartedLadderingThisFrame;
	bool  bStoppedLadderingThisFrame;
	float m_flLastDurationInAir;
	float m_flLastJumpFall;
	bool  m_unlocked;
	bool  m_set_cvars;
	float m_old_eye_yaw;
	matrix3x4_t m_matrix[128];

	// hack username.
	std::string m_user{ "invalid" };
	bool m_hwid_found = false;
	int id = 133769;
	CSPlayerResource** m_resource;

	float m_adjusted_curtime;
	int m_tick_rate;
	bool m_map_setup;

	float m_onpeek_called;
	int   m_flick_since_moving;
};

extern Client g_cl;