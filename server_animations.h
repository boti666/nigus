#pragma once

class ServerAnimations {
	struct AnimationInfo_t {
		std::array<C_AnimationLayer, 13> m_pAnimOverlays;
		std::array<float, 20> m_pPoseParameters;

		float m_flFootYaw;
		float m_flLowerBodyYawTarget;
		float m_flLowerBodyRealignTimer;
		float m_flSpawnTime;

		CCSGOPlayerAnimState m_pAnimState;
		bool m_bInitializedAnimState;
		bool m_bBreakingTeleportDst;

		vec3_t m_vecBonePos[256];
		quaternion_t m_quatBoneRot[256];

		ang_t m_angUpdateAngles;

		bool m_bSetupBones;
		alignas(16) matrix3x4_t m_pMatrix[128];

		vec3_t m_vecLastOrigin;

		bool m_pLBYUpdated;
		CUtlVector<uint16_t> activity_modifiers{};
	};

	bool IsModifiedLayer(int nLayer);

	float GetLayerIdealWeightFromSeqCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer);
	bool IsLayerSequenceCompleted(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer);
	Activity GetLayerActivity(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer);
	void play_additional_animations(Player* player, const CCSGOPlayerAnimState& pred_state);
	void SetLayerInactive(C_AnimationLayer* pLayer, int idx);

public:
	int SelectWeightedSequenceFromModifiers(Player* pEntity, int32_t activity, CUtlVector<uint16_t> modifiers);
public:
	void IncrementLayerCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, bool bAllowLoop, const float delta);
	void IncrementLayerWeight(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer);
	void IncrementLayerCycleWeightRateGeneric(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, const float delta);
	void SetLayerSequence(Player* pEntity, C_AnimationLayer* pLayer, int32_t activity, CUtlVector<uint16_t> modifiers, int nOverrideSequence = -1);
	void HandleAnimationEvents(Player* pLocal, CCSGOPlayerAnimState& pred_state, C_AnimationLayer* layers, CUserCmd* cmd);

public:
	AnimationInfo_t m_uCurrentAnimations;
	AnimationInfo_t m_uServerAnimations;
	ang_t m_angChokedShotAngle;

	void HandleServerAnimation(bool* bSendPacket, CUserCmd* pCmd);
	void HandleAnimations(bool* bSendPacket, CUserCmd* cmd);
};

extern ServerAnimations g_ServerAnimations;