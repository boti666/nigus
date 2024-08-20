#pragma once

struct PredictionData_t
{
	int m_nTickBase = 0;
	int m_nFlags = 0;

	vec3_t m_vecVelocity = vec3_t(0, 0, 0);
	vec3_t m_vecBaseVelocity = vec3_t(0, 0, 0);
	vec3_t m_vecAbsVelocity = vec3_t(0, 0, 0);
	vec3_t m_vecViewOffset = vec3_t(0, 0, 0);
	vec3_t m_vecOrigin = vec3_t(0, 0, 0);

	ang_t m_vecAimPunchAngleVel = ang_t(0, 0, 0);
	ang_t m_angAimPunchAngle = ang_t(0, 0, 0);
	ang_t m_angViewPunchAngle = ang_t(0, 0, 0);

	float m_flDuckAmount = 0.0f;
	float m_flDuckSpeed = 0.0f;
	float m_flFallVelocity = 0.0f;
	float m_flAccuracyPenalty = 0.0f;
	float m_flRecoilIndex = 0.0f;
	float m_flVelocityModifier = 0.0f;
	float m_flStamina = 0.0f;

	CHandle < Entity > m_GroundEntity;
};

struct ViewmodelData_t {
	int m_view_model_index, m_animation_parity;
	float m_view_cycle, m_anim_time;
	int m_view_sequence;
};

class InputPrediction {
public:
	float m_curtime;
	float m_frametime;
	bool  m_running;
	bool  m_had_pred_error = false;
	bool  m_read_packets;
	ViewmodelData_t m_viewmodel;

	struct {
		float m_old_velocity_modifier;
		float m_velocity_modifier;
	} stored;

public:
	void update();
	void SaveNetvars();
	void OnPlayerMove();
	void run();
	void UpdatePitch(float& pitch);
	void restore();
	void SaveViewmodel();
	void FixViewmodel();
	void PingReducer();
	PredictionData_t m_PredictionData;
};

extern InputPrediction g_inputpred;