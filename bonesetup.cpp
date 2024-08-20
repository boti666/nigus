#include "includes.h"

Bones g_bones{};;

void Bones::m_AttachmentHelper(Entity* entity, CStudioHdr* hdr) {

	using AttachmentHelperFn = void(__thiscall*)(Entity*, CStudioHdr*);
	g_csgo.m_AttachmentHelper.as< AttachmentHelperFn  >()(entity, hdr);
}

bool Bones::setup(Player* player, matrix3x4_t* out, LagRecord* record) {
	// if the record isnt setup yet.
	if (!record->m_setup) {
		// run setupbones rebuilt.
		if (!BuildBones(player, 0x7FF00, record->m_bones, record))
			return false;

		// we have setup this record bones.
		record->m_setup = true;
	}

	// record is setup.
	if (out && record->m_setup)
		std::memcpy(out, record->m_bones, sizeof(matrix3x4_t) * 128);

	return true;
}

bool Bones::BuildBones(Player* target, int mask, matrix3x4_t* out, LagRecord* record) {
	vec3_t		     pos[128];
	quaternion_t     q[128];
	vec3_t           backup_origin;
	ang_t            backup_angles;
	float            backup_poses[24];
	C_AnimationLayer backup_layers[13];

	// get hdr.
	CStudioHdr* hdr = target->GetModelPtr();
	if (!hdr)
		return false;

	// get ptr to bone accessor.
	CBoneAccessor* accessor = &target->m_BoneAccessor();
	if (!accessor)
		return false;

	// store origial output matrix.
	// likely cachedbonedata.
	matrix3x4_t* backup_matrix = accessor->m_pBones;
	if (!backup_matrix)
		return false;

	// prevent the game from calling ShouldSkipAnimationFrame.
	auto bSkipAnimationFrame = *reinterpret_cast<int*>(uintptr_t(target) + 0x260);
	*reinterpret_cast<int*>(uintptr_t(target) + 0x260) = NULL;

	// backup original.
	backup_origin = target->GetAbsOrigin();
	backup_angles = target->GetAbsAngles();
	target->GetPoseParameters(backup_poses);
	target->GetAnimLayers(backup_layers);

	// compute transform from raw data.
	matrix3x4_t transform;
	math::AngleMatrix(record->m_abs_ang, record->m_pred_origin, transform);

	// set non interpolated data.
	target->AddEffect(EF_NOINTERP);
	target->SetAbsOrigin(record->m_pred_origin);
	target->SetAbsAngles(record->m_abs_ang);
	target->SetPoseParameters(record->m_poses);
	target->SetAnimLayers(record->m_layers);

	// force game to call AccumulateLayers - pvs fix.
	m_running = true;

	// set bone array for write.
	accessor->m_pBones = out;

	// compute and build bones.
	target->StandardBlendingRules(hdr, pos, q, record->m_pred_time, mask);

	uint8_t computed[0x100];
	std::memset(computed, 0, 0x100);
	target->BuildTransformations(hdr, pos, q, transform, mask, computed);

	// restore old matrix.
	accessor->m_pBones = backup_matrix;

	// restore original interpolated entity data.
	target->SetAbsOrigin(backup_origin);
	target->SetAbsAngles(backup_angles);
	target->SetPoseParameters(backup_poses);
	target->SetAnimLayers(backup_layers);

	// revert to old game behavior.
	m_running = false;

	// allow the game to call ShouldSkipAnimationFrame.
	*reinterpret_cast<int*>(uintptr_t(target) + 0x260) = bSkipAnimationFrame;

	return true;
}

bool Bones::PerformBoneSetup(LagRecord* record, matrix3x4_t* mat, int mask, ang_t rotation, vec3_t origin, float time, std::array<float, 24>& poses)
{
	const auto player = record->m_player;
	if (!player)
		return false;

	const auto hdr = player->GetModelPtr();
	if (!hdr)
		return false;

	vec3_t           backup_origin;
	ang_t            backup_angles;
	C_AnimationLayer backup_layers[13];
	float backup_poses[24];

	backup_origin = player->GetAbsOrigin();
	backup_angles = player->GetAbsAngles();
	player->GetPoseParameters(backup_poses);
	player->GetAnimLayers(backup_layers);

	if (!player || !player->alive())
		return false;

	auto cstudio = player->GetModelPtr();

	if (!cstudio)
		return false;

	// get ptr to bone accessor.
	CBoneAccessor* accessor = &player->m_BoneAccessor();
	if (!accessor)
		return false;

	// store origial output matrix.
	// likely cachedbonedata.
	matrix3x4_t* backup_matrix = accessor->m_pBones;
	if (!backup_matrix)
		return false;

	// build matrix using un-interpolated server data.
	matrix3x4_t parentTransform;
	math::AngleMatrix(rotation, origin, parentTransform);

	//  we need an aligned matrix in the bone accessor, so do this :) bad performance cause memcpy but that's ok
	matrix3x4a_t used[128];

	player->SetAnimLayers(record->m_layers);
	player->SetAbsAngles(rotation);

	uint8_t computed[0x100] = { 0 };

	alignas(16) vec3_t pos[128];
	alignas(16) quaternion_t q[128];

	// Allocate our own
	CIKContext* ik = player->m_pIK();

	player->m_fEffects() |= 8;

	if (ik)
	{
		ik->ClearTargets();
		ik->Init(hdr, backup_angles, backup_origin, time, g_csgo.m_globals->m_frame, mask);
	}

	// set bone array for write.
	accessor->m_pBones = used;

	//	build some shit
	player->StandardBlendingRules(hdr, pos, q, time, mask);

	// set iks
	if (ik)
	{
		vfunc< void(__thiscall*)(Player*, float) >(player, 186) (player, time); // update_ik_locks
		ik->UpdateTargets(pos, q, accessor->m_pBones, computed);
		vfunc< void(__thiscall*)(Player*, float) >(player, 187) (player, time); // CalculateIKLocks
		ik->SolveDependencies(pos, q, accessor->m_pBones, computed);
	}

	//	build the matrix
	player->BuildTransformations(hdr, pos, q, parentTransform, mask, computed);

	//	restore flags and bones
	player->SetAnimLayers(backup_layers);
	player->SetAbsAngles(backup_angles);
	player->m_fEffects() &= ~8;

	accessor->m_pBones = backup_matrix;

	//  and pop out our new matrix
	memcpy(mat, used, sizeof(matrix3x4_t) * 128);

	return true;
}

bool Bones::PerformBoneSetupLocal(Player* player, matrix3x4_t* mat, int mask, ang_t rotation, vec3_t origin, float time, float* poses)
{
	const auto hdr = player->GetModelPtr();
	if (!hdr)
		return false;

	vec3_t           backup_origin;
	ang_t            backup_angles;
	C_AnimationLayer backup_layers[13];
	float backup_poses[24];

	backup_origin = player->GetAbsOrigin();
	backup_angles = player->GetAbsAngles();
	player->GetPoseParameters(backup_poses);
	player->GetAnimLayers(backup_layers);

	if (!player || !player->alive())
		return false;

	auto cstudio = player->GetModelPtr();

	if (!cstudio)
		return false;

	// get ptr to bone accessor.
	CBoneAccessor* accessor = &player->m_BoneAccessor();
	if (!accessor)
		return false;

	// store origial output matrix.
	// likely cachedbonedata.
	matrix3x4_t* backup_matrix = accessor->m_pBones;
	if (!backup_matrix)
		return false;

	m_running = false;

	// build matrix using un-interpolated server data.
	matrix3x4_t parentTransform;
	math::AngleMatrix(rotation, origin, parentTransform);

	// set un-interpolated server data.
	//player->SetPoseParameters(poses);

	uint8_t computed[0x100] = { 0 };

	alignas(16) vec3_t pos[128];
	alignas(16) quaternion_t q[128];

	// Allocate our own
	CIKContext* ik = player->m_pIK();

	player->m_fEffects() |= 8;

	if (ik)
	{
		ik->ClearTargets();
		ik->Init(hdr, backup_angles, backup_origin, time, g_csgo.m_globals->m_frame, mask);
	}

	// set bone array for write.
	accessor->m_pBones = mat;

	//	build some shit
	player->StandardBlendingRules(hdr, pos, q, time, mask);

	// set iks
	if (ik)
	{
		vfunc< void(__thiscall*)(Player*, float) >(player, 186) (player, time); // update_ik_locks
		ik->UpdateTargets(pos, q, accessor->m_pBones, computed);
		vfunc< void(__thiscall*)(Player*, float) >(player, 187) (player, time); // CalculateIKLocks
		ik->SolveDependencies(pos, q, accessor->m_pBones, computed);
	}

	//	build the matrix
	player->BuildTransformations(hdr, pos, q, parentTransform, mask, computed);

	//	restore flags and bones
	player->m_fEffects() &= ~8;

	accessor->m_pBones = backup_matrix;

	// restore poses
	//player->SetPoseParameters(backup_poses);
	//m_running = false;

	m_running = true;

	return true;
}

bool Bones::BuildLocalBones(Player* player, matrix3x4_t* out, float time, LagRecord* record)
{
	vec3_t           backup_origin;
	ang_t            backup_angles;
	float            backup_poses[24];
	C_AnimationLayer backup_layers[13];

	// backup original.
	backup_origin = player->GetAbsOrigin();
	backup_angles = player->GetAbsAngles();
	player->GetPoseParameters(backup_poses);
	player->GetAnimLayers(backup_layers);

	if (player != g_cl.m_local && record)
	{
		player->AddEffect(EF_NOINTERP);
		player->SetAnimLayers(record->m_layers);
		player->SetPoseParameters(record->m_poses);
		player->SetAbsAngles(record->m_abs_ang);
	}

	player->InvalidateBoneCache();

	auto a = player->ent_client_flags();
	auto b = player->ik_context();
	auto c = player->m_fEffects();

	// pass checks for existing ik ctx and skip calcs then
	player->ent_client_flags() |= 2;
	player->ik_context() = 0;

	// skip bone lerp
	player->m_fEffects() |= 8;

	int frame_count = g_csgo.m_globals->m_frame;

	g_csgo.m_globals->m_frame = -999;

	g_hooks.m_updating_bones[player->index()] = true;
	const bool ret = player->GameSetupBones(out, 128, 0x7FF00, time);
	g_hooks.m_updating_bones[player->index()] = false;

	g_csgo.m_globals->m_frame = frame_count;

	player->ent_client_flags() = a;
	player->ik_context() = b;

	player->m_fEffects() = c;

	if (player != g_cl.m_local && record)
	{
		// restore original interpolated entity data.
		player->SetAbsOrigin(backup_origin);
		player->SetAbsAngles(backup_angles);
		player->SetPoseParameters(backup_poses);
		player->SetAnimLayers(backup_layers);
	}

	return ret;
}

bool Bones::SetupBones(Player* player, matrix3x4_t* world, int max, int mask, float curtime, LagRecord* record)
{
	m_running = true;
	C_AnimationLayer backup[13];
	player->GetAnimLayers(backup);

	player->InvalidateBoneCache();
	vec3_t origin_backup = player->GetAbsOrigin();

	const auto m_pIk = player->m_pIK();
	const auto client_ent_flags = player->m_ClientEntEffects();
	const auto effects = player->m_fEffects();
	const auto animlod = player->m_nAnimLODflags();
	const auto jigglebones = player->m_bIsJiggleBonesEnabled();

	const auto curtime_backup = g_csgo.m_globals->m_curtime;
	const auto frametime = g_csgo.m_globals->m_frametime;

	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	if (record) {
		player->SetAnimLayers(record->m_layers);
		player->SetAbsOrigin(record->m_origin);
	}

	player->m_pIK() = nullptr;
	player->InvalidateBoneCache();
	player->m_nAnimLODflags() &= ~2u; //flag: ANIMLODFLAG_OUTSIDEVIEWFRUSTUM
	player->m_nCustomBlendingRuleMask() = -1;
	player->m_ClientEntEffects() |= 2u; //flag: NO_IK
	player->m_fEffects() |= EF_NOINTERP;
	player->m_bIsJiggleBonesEnabled() = false;

	auto& boneSnapshot1 = *(float*)(uintptr_t(player) + 0x39F0 + 4);
	auto& boneSnapshot2 = *(float*)(uintptr_t(player) + 0x6E40 + 4);

	auto bk_snapshot1 = &boneSnapshot1;
	auto bk_snapshot2 = &boneSnapshot2;

	boneSnapshot1 = 0.0f;
	boneSnapshot2 = 0.0f;

	g_hooks.m_updating_bones[player->index()] = true;
	auto bone_result = player->SetupBones(world, max, mask, curtime);
	g_hooks.m_updating_bones[player->index()] = false;

	boneSnapshot1 = *bk_snapshot1;
	boneSnapshot2 = *bk_snapshot2;

	player->m_pIK() = m_pIk;
	player->m_fEffects() = effects;
	player->m_ClientEntEffects() = client_ent_flags;
	player->m_nAnimLODflags() = animlod;

	if (record) {
		player->SetAnimLayers(backup);
		player->SetAbsOrigin(origin_backup);
	}

	g_csgo.m_globals->m_curtime = curtime_backup;
	g_csgo.m_globals->m_frametime = frametime;
	m_running = false;

	return bone_result;
}

bool Bones::Build(Player* player, matrix3x4_t* out, float curtime)
{
	const auto cur_time = g_csgo.m_globals->m_curtime;
	const auto frame_time = g_csgo.m_globals->m_frametime;
	const auto abs_frame_time = g_csgo.m_globals->m_abs_frametime;
	const auto interval_per_tick = curtime / (g_csgo.m_globals->m_interval + 0.5f);
	const auto frame_count = g_csgo.m_globals->m_frame;
	const auto tick_count = g_csgo.m_globals->m_tick_count;

	g_csgo.m_globals->m_curtime = curtime;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_abs_frametime = g_csgo.m_globals->m_interval;
	g_csgo.m_globals->m_frame = interval_per_tick;
	g_csgo.m_globals->m_tick_count = interval_per_tick;

	const auto backup_effects = player->m_fEffects();
	const auto backup_setup_time = player->m_flLastBoneSetupTime();
	const auto backup_maintain_seq_transitions_value = *(bool*)(std::uintptr_t(player) + 0x9F0);

	*(bool*)(std::uintptr_t(player) + 0x9F0) = false; // skip call to MaintainSequenceTransitions

	player->m_fEffects() |= EF_NOINTERP;
	player->InvalidateBoneCache();
	player->m_flLastBoneSetupTime() = 0;

	g_hooks.m_updating_bones[player->index()] = true;
	bool setup = player->SetupBones(out, 128, 0x7FF00, curtime);
	g_hooks.m_updating_bones[player->index()] = false;

	player->m_fEffects() = backup_effects;
	player->m_flLastBoneSetupTime() = backup_setup_time;
	*(bool*)(std::uintptr_t(player) + 0x9F0) = backup_maintain_seq_transitions_value;

	g_csgo.m_globals->m_curtime = cur_time;
	g_csgo.m_globals->m_frametime = frame_time;
	g_csgo.m_globals->m_abs_frametime = abs_frame_time;
	g_csgo.m_globals->m_frame = frame_count;
	g_csgo.m_globals->m_tick_count = tick_count;

	return setup;
}
