#include "includes.h"

void Client::update_shot_cmd()
{
	if (g_cl.m_shot)
	{
		g_cl.shot_cmd = g_cl.m_cmd->m_command_number;
	}
}

void Client::update_viewangles()
{
	if (g_csgo.m_cl->m_choked_commands < 1)
	{
		m_real_angle = g_cl.m_cmd->m_view_angles;
		return;
	}

	if (g_cl.m_cmd->m_command_number >= g_cl.shot_cmd && g_cl.shot_cmd >= g_cl.m_cmd->m_command_number - g_csgo.m_cl->m_choked_commands)
	{
		auto shot_cmd = g_csgo.m_input->get_user_cmd(g_cl.shot_cmd);
		if (shot_cmd)
		{
			if (!*g_cl.m_packet)
				m_real_angle = shot_cmd->m_view_angles;
		}
	}
	else
	{
		if (!*g_cl.m_packet)
			m_real_angle = g_cl.m_cmd->m_view_angles;
	}
}

void Client::HandleBodyYaw() {

	CCSGOPlayerAnimState* state = m_local->m_PlayerAnimState();
	if (!state)
		return;

	// update time.
	m_anim_frame = g_csgo.m_globals->m_curtime - m_anim_time;
	m_anim_time = g_csgo.m_globals->m_curtime;

	// we simulate client body yaw ( server body yaw is delayed so let's "predict" it )
	if (state->m_flVelocityLengthXY > .1f || state->m_flVelocityLengthZ > 100.f)
	{
		m_flick_since_moving = 0;
		m_body = m_real_angle.y;
		m_body_pred = g_csgo.m_globals->m_curtime + .22f;

	}
	else if (g_csgo.m_globals->m_curtime >= m_body_pred && fabsf(math::AngleDiff(state->m_flFootYaw, state->m_flEyeYaw)) > 35.f)
	{
		m_body = m_real_angle.y;
		m_body_pred = g_csgo.m_globals->m_curtime + 1.1f;
		++m_flick_since_moving;
	}
}

void Client::UpdateViewmodel()
{
	if (!g_cl.m_processing) return;

	using update_all_viewmodel_addons_fn = int(__fastcall*)(void*);
	static auto update_all_viewmodel_addons = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 ? 83 EC ? 53 8B D9 56 57 8B 03 FF 90 ? ? ? ? 8B F8 89 7C 24 ? 85 FF 0F 84 ? ? ? ? 8B 17 8B CF")).as< update_all_viewmodel_addons_fn>();

	auto viewmodel = g_cl.m_local->get_view_model();
	if (viewmodel)
		update_all_viewmodel_addons(viewmodel);
}

void Client::HandleAnimations() {
	if (!m_processing)
		return;

	CCSGOPlayerAnimState* anim_state = m_local->m_PlayerAnimState();
	if (!anim_state)
		return;

	auto ieflags_backup = m_local->m_iEFlags();

	// we simulate client body yaw ( server body yaw is delayed so let's "predict" it )
	//m_local->m_flLowerBodyYawTarget() = m_body;
	m_real_angle = m_cmd->m_view_angles;

	bPreviousGroundState = m_bOnGround;
	m_bOnGround = m_local->m_fFlags() & FL_ONGROUND;
	
	// update last simulation time.
	m_last_sim_time = m_local->m_flSimulationTime();

	// deez nuts
	m_flLastUpdateIncrement = std::max(0.0f, g_csgo.m_globals->m_curtime - m_flLastUpdateTime);

	// backup prediction angle
	const ang_t backup_angle = m_local->pl().v_angle;

	// this might work
	m_old_eye_yaw = anim_state->m_flEyeYaw;

	// current angle will be animated.
	if (!m_flicking)
		m_local->pl().v_angle = m_real_angle;
	else
		m_local->pl().v_angle = m_real_angle_old;

	// bunch of shit game does idk
	m_bLandedOnGroundThisFrame = (bPreviousGroundState != m_bOnGround && m_bOnGround);
	m_bLeftTheGroundThisFrame = (bPreviousGroundState != m_bOnGround && !m_bOnGround);

	bPreviouslyOnLadder = m_bOnLadder;
	m_bOnLadder = !m_bOnGround && m_local->m_MoveType() == MOVETYPE_LADDER;
	bStartedLadderingThisFrame = (!bPreviouslyOnLadder && m_bOnLadder);
	bStoppedLadderingThisFrame = (bPreviouslyOnLadder && !m_bOnLadder);

	// allow animations to be animated in the same frame
	if (anim_state->m_nLastUpdateFrame == g_csgo.m_globals->m_frame)
		anim_state->m_nLastUpdateFrame = g_csgo.m_globals->m_frame - 1;

	if (anim_state->m_flLastUpdateTime == g_csgo.m_globals->m_curtime)
		anim_state->m_flLastUpdateTime = g_csgo.m_globals->m_curtime - g_csgo.m_globals->m_interval;

	// handle animation events on client
	g_ServerAnimations.HandleAnimationEvents( m_local, predict_animation_state(m_local).first, m_local->m_AnimOverlay(), m_cmd);

	// update animations.
	auto backup = m_local->m_bClientSideAnimation();
	g_hooks.m_bUpdatingCSA[m_local->index()] = true;
	m_local->m_bClientSideAnimation() = true;
	m_local->UpdateClientSideAnimation();
	m_local->m_bClientSideAnimation() = backup;
	g_hooks.m_bUpdatingCSA[m_local->index()] = false;

	// get last networked poses.
	m_local->GetPoseParameters(m_poses);

	// store updated abs yaw.
	m_abs_yaw = anim_state->m_flFootYaw;

	// store networked layers.
	m_local->GetAnimLayers(m_layers);

	// save updated data.
	m_ground = anim_state->m_bOnGround;

	// save our speed on anim update
	m_speed = anim_state->m_flVelocityLengthXY;

	// do this shit ig
	m_flLastUpdateTime = g_csgo.m_globals->m_curtime;
}

void Client::RestoreData()
{
	if (!g_cl.m_processing)
		return;

	// set these every frame
	m_local->SetPoseParameters(g_cl.m_poses);
	//m_local->SetAnimLayers(g_cl.m_layers);
	m_local->SetAbsAngles(ang_t{ 0.f, m_abs_yaw, 0.f });

	// update the matrix every frame
	std::memcpy(m_local->m_BoneCache().m_pCachedBones, g_cl.m_matrix, sizeof(matrix3x4_t) * m_local->m_BoneCache().m_CachedBoneCount);
	std::memcpy(m_local->m_BoneAccessor().m_pBones, g_cl.m_matrix, sizeof(matrix3x4_t) * m_local->m_BoneCache().m_CachedBoneCount);
}

CUtlVector<uint16_t> Client::build_activity_modifiers(Player* player)
{
	activity_modifiers_wrapper modifier_wrapper{};

	const auto state = player->m_PlayerAnimState();

	modifier_wrapper.add_modifier(state->GetWeaponPrefix());
	
	if (state->m_flSpeedAsPortionOfRunTopSpeed > 0.25f)
		modifier_wrapper.add_modifier("moving");

	if (state->m_flAnimDuckAmount > 0.55000001f)
		modifier_wrapper.add_modifier("crouch");

	return modifier_wrapper.get();
}

std::pair<CCSGOPlayerAnimState, C_AnimationLayer*> Client::predict_animation_state(Player* player)
{
	const auto backup_state = *player->m_PlayerAnimState();
	const auto backup_layers = player->m_AnimOverlay();
	const auto backup_poses = player->m_flPoseParameter();

	if (player->m_PlayerAnimState()->m_nLastUpdateFrame >= g_csgo.m_globals->m_frame)
		player->m_PlayerAnimState()->m_nLastUpdateFrame = g_csgo.m_globals->m_frame - 1;

	g_hooks.m_bUpdatingCSA[m_local->index()] = true;
	player->m_PlayerAnimState()->update(g_cl.m_real_angle);
	g_hooks.m_bUpdatingCSA[m_local->index()] = false;
	const auto pred = *player->m_PlayerAnimState();
	const auto layers = player->m_AnimOverlay();

	*player->m_PlayerAnimState() = backup_state;
	player->SetAnimLayers(backup_layers);
	player->SetPoseParameters(backup_poses);

	return { pred, layers };
}

void Client::try_initiate_animation(Player* player, size_t layer, int32_t activity, CUtlVector<uint16_t> modifiers)
{
	typedef void* (__thiscall* find_mapping_t)(void*);
	static const auto find_mapping = pattern::find(g_csgo.m_server_dll, "55 8B EC 83 E4 ? 81 EC ? ? ? ? 53 56 57 8B F9 8B 17").as<find_mapping_t>();

	typedef int32_t(__thiscall* select_weighted_sequence_from_modifiers_t)(void*, void*, int32_t, const void*, int32_t);
	static const auto select_weighted_sequence_from_modifiers = pattern::find(g_csgo.m_server_dll, "55 8B EC 83 EC 2C 53 56 8B 75 08 8B D9").as< select_weighted_sequence_from_modifiers_t>();

	const auto mapping = find_mapping(player->m_studioHdr());
	const auto sequence = select_weighted_sequence_from_modifiers(mapping, player->m_studioHdr(), activity, &modifiers[0], modifiers.Count());

	if (sequence < 2)
		return;

	auto& l = player->m_AnimOverlay()[layer];
	l.playback_rate = player->GetLayerSequenceCycleRate(&l, sequence);
	l.sequence = sequence;
	l.cycle = l.weight = 0.f;
}

void Client::UpdateLocalMatrix()
{
	Player* player = m_local;
	if (!player)
		return;

	if (!player->alive() || !m_matrix)
		return;

	// adjust render matrix pos
	vec3_t render_origin = player->GetRenderOrigin();
	math::change_matrix_position(m_matrix, 128, vec3_t(), render_origin);

	// force matrix pos, fix jitter attachments, etc
	player->interpolate_moveparent_pos();
	player->set_bone_cache(m_matrix);
	g_bones.m_AttachmentHelper(player, player->GetModelPtr());

	// restore matrix pos
	math::change_matrix_position(m_matrix, 128, render_origin, vec3_t());
}

void Client::HandleUpdates()
{
	if (!m_local || !m_local->alive()) return;

	auto state = m_local->m_PlayerAnimState();
	if (!state) return;

	m_local->SetAnimLayers(m_layers);
}

void Client::UpdateAnimations()
{
	if (!g_cl.m_local || !g_cl.m_processing)
		return;

	CCSGOPlayerAnimState* state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	// update layers, poses, and rotation
	m_local->SetAbsAngles(ang_t{ 0.f, m_abs_yaw, 0.f });
}