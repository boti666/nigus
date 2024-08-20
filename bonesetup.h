#pragma once

enum BoneSetupFlags {
	None = 0,
	UseInterpolatedOrigin = (1 << 0),
	UseCustomOutput = (1 << 1),
	ForceInvalidateBoneCache = (1 << 2),
	AttachmentHelper = (1 << 3),
};

class Bones {

public:
	bool m_running = true;

public:
	void m_AttachmentHelper(Entity* entity, CStudioHdr* hdr);
	bool PerformBoneSetup(LagRecord* record, matrix3x4_t* mat, int mask, ang_t rotation, vec3_t origin, float time, std::array<float, 24>& poses);
	bool PerformBoneSetupLocal(Player* player, matrix3x4_t* mat, int mask, ang_t rotation, vec3_t origin, float time, float* poses);
	bool BuildLocalBones(Player* player, matrix3x4_t* out, float time, LagRecord* record = nullptr);
	bool SetupBones(Player* player, matrix3x4_t* world, int max, int mask, float time, LagRecord* record = nullptr);
	bool Build(Player* player, matrix3x4_t* out, float curtime);
	bool setup(Player* player, matrix3x4_t* out, LagRecord* record);
	bool BuildBones(Player* target, int mask, matrix3x4_t* out, LagRecord* record);
};

extern Bones g_bones;