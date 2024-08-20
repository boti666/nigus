#pragma once

class ShotRecord;

class Resolver {
public:

	enum Modes : size_t {
		RESOLVE_NONE = 0,
		RESOLVE_WALK,
		RESOLVE_LBY,
		RESOLVE_STAND,
		RESOLVE_AIR,
		RESOLVE_STOPPED_MOVING,
		RESOLVE_NO_DATA,
		RESOLVE_BODY,
		RESOLVE_DATA,
		RESOLVE_OVERRIDE,
		RESOLVE_LBY_PRED,
		RESOLVE_NO_FLICK,
		RESOLVE_NO_PITCH
	};

	enum Flags : size_t {
		HIGH_LBY = 0,
		INVERT_FS = 1,
	};

	enum override_side : size_t {
		NONE = 0,
		BACKWARD,
		FORWARD,
		LEFT,
		RIGHT
	};

public:
	LagRecord* FindFirstRecord(AimPlayer* data);
	LagRecord* FindIdealRecord(AimPlayer* data);
	LagRecord* FindLastRecord(AimPlayer* data);
	
	void OnBodyUpdate(Player* player, float value);
	float AntiFreestand(Player* player, LagRecord* record, vec3_t start_, vec3_t end, bool include_base, float base_yaw, float delta);
	float GetAwayAngle(LagRecord* record);
	void OnPlayerFire(LagRecord* record, AimPlayer* data);
	bool UnsafeVelocityTransitionDetection(LagRecord* pRecord, AimPlayer* pData);
	bool IsFakeFlick(LagRecord* pRecord, AimPlayer* pData);
	void SetMode(LagRecord* record);
	void ResolveAngles(Player* player, LagRecord* record);
	void ResolveAir(AimPlayer* data, LagRecord* record);
	void ResolveWalk(AimPlayer* data, LagRecord* record);
	void SetupResolveChance(AimPlayer* data, LagRecord* record);
	void UpdateLBYPrediction(AimPlayer* data, LagRecord* record);
	void ResolveStand(AimPlayer* data, LagRecord* record);
	void ResolveOverride(AimPlayer* data, LagRecord* record, Player* player);
	void ResolvePoses(Player* player, LagRecord* record);
	void AirNS(AimPlayer* data, LagRecord* record);
	bool IsSideways(float angle, LagRecord* player);
	int	 GetNearestEntity(Player* player, LagRecord* record);

public:

	std::array< vec3_t, 64 > m_impacts;
	int iPlayers[64];
};

extern Resolver g_resolver;