#pragma once

class Movement {
public:
	float  m_speed;
	float  m_ideal;
	float  m_ideal2;
	vec3_t m_mins;
	vec3_t m_maxs;
	vec3_t m_origin;
	float  m_switch_value = 1.f;
	int    m_strafe_index;
	float m_max_player_speed;
	float m_max_weapon_speed;
	float  m_old_yaw;
	float  m_circle_yaw;
	bool   m_invert;
	vec3_t start_position = vec3_t(0.0f, 0.0f, 0.0f);
	bool   fired_shot = false;
	ang_t wish_angle;

public:
	void JumpRelated( );
	void Strafe(CUserCmd* cmd, const ang_t& old_angs);
	void DoPrespeed( );
	void DirectionalStrafe(CUserCmd* cmd, const ang_t& old_angs);
	bool GetClosestPlane( vec3_t& plane );
	bool WillCollide( float time, float step );
	void LegMovement(CUserCmd* cmd);
	void FixMove(CUserCmd* user_cmd);
	/*void FixMove(CUserCmd* cmd, ang_t& wanted_move, bool a7);*/
	void FixButtonMoves(CUserCmd* user_cmd);
	void AutoPeek(CUserCmd* cmd, float wish_yaw);
	void NullVelocity(CUserCmd* cmd);
	void AutoStop();
	void accelerate(const CUserCmd& cmd, const vec3_t& wishdir, const float wishspeed, vec3_t& velocity, float acceleration);
	void walk_move(const CUserCmd& cmd, vec3_t& move, vec3_t& fwd, vec3_t& right, vec3_t& velocity);
	void full_walk_move(const CUserCmd cmd, vec3_t move, vec3_t fwd, vec3_t right, vec3_t velocity);
	void modify_move(CUserCmd cmd, vec3_t velocity);
	void predict_move(const CUserCmd cmd, vec3_t velocity);
	void FakeWalk(bool forced = false);
	void FastStop();
	void ClampMovementSpeed(float speed);
};

extern Movement g_movement;