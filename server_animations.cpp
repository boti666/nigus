#include "includes.h"

ServerAnimations g_ServerAnimations;

void ServerAnimations::IncrementLayerCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, bool bAllowLoop, const float delta) {
	if (!pLayer || !m_pAnimstate)
		return;

	if (!m_pAnimstate->m_pPlayer)
		return;

	if (fabs(pLayer->playback_rate) <= 0.f)
		return;

	auto cur_cycle = pLayer->cycle;
	cur_cycle += delta * pLayer->playback_rate;

	if (!bAllowLoop && cur_cycle >= 1.f)
		cur_cycle = .999f;

	cur_cycle -= int(cur_cycle);

	if (cur_cycle < 0.f)
		cur_cycle += 1.f;
	else if (cur_cycle > 1.f)
		cur_cycle -= 1.f;

	pLayer->cycle = cur_cycle;
}

void ServerAnimations::IncrementLayerWeight(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer) {
	if (!pLayer)
		return;

	if (abs(pLayer->weight_delta_rate) <= 0.f)
		return;

	float flCurrentWeight = pLayer->weight;
	flCurrentWeight += m_pAnimstate->m_flLastUpdateIncrement * pLayer->weight_delta_rate;
	flCurrentWeight = std::clamp(flCurrentWeight, 0.f, 1.f);

	if (pLayer->weight != flCurrentWeight) {
		pLayer->weight = flCurrentWeight;
	}
}

float ServerAnimations::GetLayerIdealWeightFromSeqCycle(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer) {
	if (!pLayer)
		return 0.f;


	float flCycle = pLayer->cycle;
	if (flCycle >= 0.999f)
		flCycle = 1;

	float flEaseIn = pLayer->blend_in; // seqdesc.fadeintime;
	float flEaseOut = pLayer->blend_in; // seqdesc.fadeouttime;
	float flIdealWeight = 1;

	if (flEaseIn > 0 && flCycle < flEaseIn)
	{
		flIdealWeight = math::SmoothStepBounds(0, flEaseIn, flCycle);
	}
	else if (flEaseOut < 1 && flCycle > flEaseOut)
	{
		flIdealWeight = math::SmoothStepBounds(1.0f, flEaseOut, flCycle);
	}

	if (flIdealWeight < 0.0015f)
		return 0.f;

	return (std::clamp(flIdealWeight, 0.f, 1.f));
}

bool ServerAnimations::IsLayerSequenceCompleted(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer) {
	if (pLayer) {
		return ((pLayer->cycle + (m_pAnimstate->m_flLastUpdateIncrement * pLayer->playback_rate)) >= 1);
	}

	return false;
}

Activity ServerAnimations::GetLayerActivity(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer) {
	if (!m_pAnimstate || !m_pAnimstate->m_pPlayer)
		return ACT_INVALID;

	if (pLayer) {
		return (Activity)m_pAnimstate->m_pPlayer->GetSequenceActivity(pLayer->sequence);
	}

	return ACT_INVALID;
}

void ServerAnimations::play_additional_animations(Player* player, const CCSGOPlayerAnimState& pred_state)
{

}

void ServerAnimations::IncrementLayerCycleWeightRateGeneric(CCSGOPlayerAnimState* m_pAnimstate, C_AnimationLayer* pLayer, const float delta) {
	float flWeightPrevious = pLayer->weight;
	IncrementLayerCycle(m_pAnimstate, pLayer, false, delta);
	pLayer->weight = GetLayerIdealWeightFromSeqCycle(m_pAnimstate, pLayer);
	pLayer->weight_delta_rate = flWeightPrevious;

}

int ServerAnimations::SelectWeightedSequenceFromModifiers(Player* pEntity, int32_t activity, CUtlVector<uint16_t> modifiers) {
	typedef CStudioHdr::CActivityToSequenceMapping* (__thiscall* fnFindMapping)(void*);
	typedef int32_t(__thiscall* fnSelectWeightedSequenceFromModifiers)(void*, void*, int32_t, const void*, int32_t);

	static const auto FindMappingAdr = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 57 8B F9 8B 17")).as< fnFindMapping>();
	static const auto SelectWeightedSequenceFromModifiersAdr = pattern::find(g_csgo.m_server_dll, XOR("55 8B EC 83 EC 2C 53 56 8B 75 08 8B D9")).as< fnSelectWeightedSequenceFromModifiers>();

	auto pHdr = pEntity->m_studioHdr();
	if (!pHdr) {
		return -1;
	}

	const auto pMapping = FindMappingAdr(pHdr);
	if (!pHdr->m_pActivityToSequence) {
		pHdr->m_pActivityToSequence = pMapping;
	}

	return SelectWeightedSequenceFromModifiersAdr(pMapping, pHdr, activity, modifiers.Base(), modifiers.Count());
}

void ServerAnimations::SetLayerSequence(Player* pEntity, C_AnimationLayer* pLayer, int32_t activity, CUtlVector<uint16_t> modifiers, int nOverrideSequence) {

	int nSequence = SelectWeightedSequenceFromModifiers(pEntity, activity, modifiers);

	if (nOverrideSequence != -1)
		nSequence = nOverrideSequence;

	if (nSequence >= 2) {

		if (pLayer) {
			pLayer->sequence = nSequence;
			pLayer->playback_rate = pEntity->GetLayerSequenceCycleRate(pLayer, nSequence);
			pLayer->cycle = 0.0f;
			pLayer->weight = 0.0f;

			// todo: maybe some other day, i don't think it's needed
			// UpdateLayerOrderPreset( 0.0f, layer, sequence ); 
		}
	}
}

bool ServerAnimations::IsModifiedLayer(int nLayer) {
	// note - michal;
	// in the future, we should look into rebuilding each layer (or atleast the most curcial ones,
	// such as weapon_action, movement_move, whole_body, etc). i only rebuilt these for the time being
	// as they're the only layers we really need to rebuild, they're responsible for eyepos when landing
	// (also known as "landing comp" kek) etc. plus once they're fixed the animations are eye candy :-)
	return (nLayer == ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB || nLayer == ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL);
}

bool has_sequence_completed(const float delta, float cycle, float playback)
{
	return cycle + playback * delta >= 1.f;
}

void set_layer_weight_rate(const float delta, const float previous, float weightdelta, float weight)
{
	if (delta == 0.f)
		return;

	weightdelta = (weight - previous) / delta;
}

mstudioseqdesc_t* get_seq_desc(CStudioHdr* hdr, const int index)
{
	if (!hdr->m_pVModel)
		return hdr->m_pStudioHdr->get_local_seqdesc(index);

	static auto get_seq = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 79 04 00 75 25 8B 45 08 8B 09 85 C0 78 08 3B 81 ? ? ? ? 7C 02 33 C0 69 C0 ? ? ? ? 03").as<mstudioseqdesc_t * (__thiscall*)(void*, int32_t)>();

	return get_seq(hdr, index);
}


void ServerAnimations::HandleAnimationEvents(Player* pLocal, CCSGOPlayerAnimState& pred_state, C_AnimationLayer* layers, CUserCmd* cmd) {
	if (!pLocal || !cmd)
		return;

	if (!pLocal->alive())
		return;

	if (!pLocal->m_PlayerAnimState() || !g_csgo.m_engine->IsInGame() || !g_csgo.m_engine->IsConnected())
		return;

	auto pWeapon = pLocal->GetActiveWeapon();
	if (!pWeapon)
		return;

	const auto p = &pred_state;

	// build activity modifiers
	CUtlVector<uint16_t> uModifiers = g_cl.build_activity_modifiers(pLocal);
	auto wpn = pLocal->GetActiveWeapon();
	const auto s = pLocal->m_PlayerAnimState();

	bool m_bJumping = false;
	bool in_idle = false;
	float adjust_weight = 0.0f;
	bool in_deploy_rate_limit = false;
	bool swing_left = false;
	bool m_bOnGround = false;

	// CCSGOPlayerAnimState::DoAnimationEvent
	if (wpn && wpn->m_iItemDefinitionIndex() == WEAPONTYPE_KNIFE && wpn->m_flNextPrimaryAttack() + .4f < g_csgo.m_globals->m_curtime)
		swing_left = true;

	// CCSGOPlayerAnimState::DoAnimationEvent
	if (cmd->m_buttons & IN_JUMP && !(pLocal->m_fFlags() & FL_ONGROUND) && !pLocal->GetGroundEntity())
	{
		m_bJumping = true;
		g_cl.try_initiate_animation(pLocal, 4, ACT_CSGO_JUMP, uModifiers);
	}

	// CCSGOPlayerAnimState::SetupVelocity
	auto& layer3 = pLocal->m_AnimOverlay()[3];
	const auto update_time = fmaxf(0.f, p->m_flLastUpdateTime - s->m_flLastUpdateTime);
	const auto layer3_act = pLocal->GetSequenceActivity(layer3.sequence);

	if (layer3_act == 980 || layer3_act == 979)
	{
		if (in_idle && p->m_flSpeedAsPortionOfCrouchTopSpeed <= .25f)
		{
			IncrementLayerCycleWeightRateGeneric(s, &layer3, update_time);
			in_idle = !has_sequence_completed(update_time, layer3.cycle, layer3.playback_rate);
		}
		else
		{
			const auto weight = layer3.weight;
			layer3.weight = math::Approach(0.f, weight, update_time * 5.f);
			set_layer_weight_rate(update_time, weight, layer3.weight_delta_rate, layer3.weight);
			in_idle = false;
		}
	}

	if (p->m_flVelocityLengthXY <= 1.f && s->m_bOnGround && !s->m_bOnLadder && !s->m_bLanding && s->m_flLastUpdateIncrement > 0
		&& std::abs(math::AngleDiff(s->m_flFootYaw, p->m_flFootYaw) / update_time > 120.f))
	{
		g_cl.try_initiate_animation(pLocal, 3, ACT_CSGO_IDLE_TURN_BALANCEADJUST, uModifiers);
		in_idle = true;
	}

	vec3_t forward{}, right{}, up{};
	math::AngleVectors(ang_t(0.f, s->m_flFootYaw, 0.f), &forward, &right, &up);
	right.normalize_in_place();
	const auto to_forward_dot = s->m_vecVelocityNormalizedNonZero.Dot(forward);
	const auto to_right_dot = s->m_vecVelocityNormalizedNonZero.Dot(right);

	const auto move_right = (cmd->m_buttons & IN_MOVERIGHT) != 0;
	const auto move_left = (cmd->m_buttons & IN_MOVELEFT) != 0;
	const auto move_forward = (cmd->m_buttons & IN_FORWARD) != 0;
	const auto move_backwards = (cmd->m_buttons & IN_BACK) != 0;
	const auto strafe_forward = s->m_flSpeedAsPortionOfRunTopSpeed >= .65f && move_forward && !move_backwards && to_forward_dot < -.55f;
	const auto strafe_backwards = s->m_flSpeedAsPortionOfRunTopSpeed >= .65f && move_backwards && !move_forward && to_forward_dot > .55f;
	const auto strafe_right = s->m_flSpeedAsPortionOfRunTopSpeed >= .73f && move_right && !move_left && to_right_dot < -.63f;
	const auto strafe_left = s->m_flSpeedAsPortionOfRunTopSpeed >= .73f && move_left && !move_right && to_right_dot > .63f;

	pLocal->m_bStrafing() = strafe_forward || strafe_backwards || strafe_right || strafe_left;

	const auto swapped_ground = s->m_bOnGround != p->m_bOnGround || s->m_bOnLadder != p->m_bOnLadder;

	if (p->m_bOnGround)
	{
		auto& layer5 = pLocal->m_AnimOverlay()[5];

		if (!s->m_bLanding && swapped_ground)
			g_cl.try_initiate_animation(pLocal, 5, s->m_flDurationInAir > 1.f ? 989 : 988, uModifiers);

		if (p->m_bLanding && pLocal->GetSequenceActivity(layer5.sequence) != 987)
			m_bJumping = false;

		if (!p->m_bLanding && !m_bJumping && p->m_flLadderSpeed <= 0.f)
			layer5.weight = 0.f;
	}
	else if (swapped_ground && !m_bJumping)
		g_cl.try_initiate_animation(pLocal, 4, 986, uModifiers);

	// CCSGOPlayerAnimState::SetupAliveLoop
	auto& alive = pLocal->m_AnimOverlay()[11];
	if (pLocal->GetSequenceActivity(alive.sequence) == 981)
	{
		if (p->m_pWeapon && p->m_pWeapon != p->m_pWeaponLast)
		{
			const auto cycle = alive.cycle;
			g_cl.try_initiate_animation(pLocal, 11, 981, uModifiers);
			alive.cycle = cycle;
		}
		else if (!has_sequence_completed(update_time, alive.cycle, alive.playback_rate))
			alive.weight = 1.f - std::clamp((p->m_flSpeedAsPortionOfWalkTopSpeed - .55f) / .35f, 0.f, 1.f);
	}

	const auto world_model = wpn ? reinterpret_cast<Weapon*>(g_csgo.m_entlist->GetClientEntityFromHandle(wpn->m_hWeaponWorldModel())) : nullptr;

	// CCSGOPlayerAnimState::SetUpWeaponAction
	auto increment = true;
	auto& action = pLocal->m_AnimOverlay()[1];
	if (wpn && in_deploy_rate_limit && pLocal->GetSequenceActivity(action.sequence) == 972)
	{
		if (world_model)
			world_model->m_fEffects() |= EF_NODRAW;

		if (action.cycle >= .15f)
		{
			in_deploy_rate_limit = false;
			g_cl.try_initiate_animation(pLocal, 1, 972, uModifiers);
			increment = false;
		}
	}

	auto& recrouch = pLocal->m_AnimOverlay()[2];
	auto recrouch_weight = 0.f;

	if (action.weight > 0.f)
	{
		if (recrouch.sequence <= 0)
		{
			_(r, "recrouch_generic");
			const auto seq = pLocal->LookupSequence(r.c_str());
			recrouch.playback_rate = pLocal->GetLayerSequenceCycleRate(&recrouch, seq);
			recrouch.sequence = seq;
			recrouch.cycle = recrouch.weight = 0.f;
		}

		auto has_modifier = false;
		_(c, "crouch");
		const auto& seqdesc = get_seq_desc(pLocal->GetModelPtr(), action.sequence);
		for (auto i = 0; i < seqdesc->num_activity_modifiers; i++)
			if (!strcmp(seqdesc->pActivityModifier(i)->pszName(), c.c_str()))
			{
				has_modifier = true;
				break;
			}

		if (has_modifier)
		{
			if (p->m_flAnimDuckAmount < 1.f)
				recrouch_weight = action.weight * (1.f - p->m_flAnimDuckAmount);
		}
		else if (p->m_flAnimDuckAmount > 0.f)
			recrouch_weight = action.weight * p->m_flAnimDuckAmount;
	}
	else if (recrouch.weight > 0.f)
		recrouch_weight = math::Approach(0.f, recrouch.weight, 4.f * update_time);

	recrouch.weight = std::clamp(recrouch_weight, 0.f, 1.f);

	if (increment)
	{
		IncrementLayerCycle(s, &action, false, update_time);
		GetLayerIdealWeightFromSeqCycle(s, &action);
		set_layer_weight_rate(p->m_flLastUpdateIncrement, action.weight, action.weight_delta_rate, action.weight);

		action.cycle = std::clamp(action.cycle - p->m_flLastUpdateIncrement * action.playback_rate, 0.f, 1.f);
		action.weight = std::clamp(action.weight - p->m_flLastUpdateIncrement * action.weight_delta_rate, .0000001f, 1.f);
	}

	g_cl.vm = 0;
}

void ServerAnimations::SetLayerInactive(C_AnimationLayer* layers, int idx) {
	if (!layers)
		return;

	layers[idx].cycle = 0.f;
	layers[idx].sequence = 0.f;
	layers[idx].weight = 0.f;
	layers[idx].playback_rate = 0.f;
}

void ServerAnimations::HandleServerAnimation(bool* bSendPacket, CUserCmd* pCmd)
{
	auto pLocal = g_cl.m_local; // lol

	// perform basic sanity checks
	if (!pLocal->alive())
		return;

	auto pState = pLocal->m_PlayerAnimState();
	if (!pState)
		return;

	if (g_csgo.m_cl->m_choked_commands)
		return;

	auto ieflags_backup = pLocal->m_iEFlags();

	g_cl.m_real_angle = pCmd->m_view_angles;

	// set thirdperson angles
	pLocal->pl().v_angle = g_cl.m_real_angle;

	// allow re-animating in the same frame
	if (pState->m_nLastUpdateFrame == g_csgo.m_globals->m_frame)
		pState->m_nLastUpdateFrame = g_csgo.m_globals->m_frame - 1;

	// prevent C_BaseEntity::CalcAbsoluteVelocity being called
	pLocal->m_iEFlags() &= ~EFL_DIRTY_ABSVELOCITY;

	// snap to footyaw, instead of approaching
	pState->m_flMoveWeight = 0.f;

	// update animations
	pLocal->UpdateClientSideAnimationEx();

	// build activity modifiers
	auto modifier = CUtlVector<uint16_t>();

	// handle animation events on client
	//HandleAnimationEvents(pLocal, pState, pLocal->m_AnimOverlay(), modifier, pCmd);

	// note - michal;
	// might want to make some storage for constant anim variables
	const float CSGO_ANIM_LOWER_REALIGN_DELAY{ 1.1f };

	if (pState->m_bOnGround)
	{
		// rebuild server CCSGOPlayerAnimState::SetUpVelocity
		// predict m_flLowerBodyYawTarget
		if (pState->m_flVelocityLengthXY > 0.1f) {
			m_uServerAnimations.m_pLBYUpdated = true;
			m_uServerAnimations.m_flLowerBodyRealignTimer = g_csgo.m_globals->m_curtime + (CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f);
			m_uServerAnimations.m_flLowerBodyYawTarget = pState->m_flEyeYaw;
		}
		// note - michal;
		// hello ledinchik men so if you've noticed our fakewalk breaks/stops for a while if we don't use "random" 
		// fake yaw option, coz random swaps flick side making this footyaw and eyeyaw delta pretty much always > 35.f
		// and the other options only flick to one side and due to something something footyaw being weird when flicking 
		// the delta jumps below 35, which causes the fakewalk to freak out and stop
		// so if we remove the delta check the fakewalk will work perfectly on every lby breaker option (but sometimes fail cos obv it failed on server)
		// only way i can think of fixing this without removing the delta check (coz ur comment below is right) is to force flick further or smth or 
		// somehow make sure that the footyaw is always at a bigger than 35deg delta from eyeyaw, whether that'd be by recalculating it ourselves
		// or maybe doing some other fuckery shit IDK!!
		// TLDR: fakewalk stops for 2 hours coz of the delta check (vague asf)
		else if (g_csgo.m_globals->m_curtime > m_uServerAnimations.m_flLowerBodyRealignTimer && abs(math::AngleDiff(pState->m_flFootYaw, pState->m_flEyeYaw)) > 35.0f) {
			//
			// actually angle diff check is needed cause else lby breaker will see update when it didn't happen on server which is going to desync the timer
			// -- L3D415R7
			//

			m_uServerAnimations.m_pLBYUpdated = true;
			m_uServerAnimations.m_flLowerBodyRealignTimer = g_csgo.m_globals->m_curtime + CSGO_ANIM_LOWER_REALIGN_DELAY;
			m_uServerAnimations.m_flLowerBodyYawTarget = pState->m_flEyeYaw;
		}
	}

	// fix legs failing idk
	if (!pState->m_bOnGround) {
		pLocal->m_AnimOverlay()[animstate_layer_t::ANIMATION_LAYER_MOVEMENT_MOVE].weight = 0.f;
		pLocal->m_AnimOverlay()[animstate_layer_t::ANIMATION_LAYER_MOVEMENT_MOVE].cycle = 0.f;
	}

	// remove model sway
	pLocal->m_AnimOverlay()[animstate_layer_t::ANIMATION_LAYER_LEAN].weight = 0.f;

	// build bones, save bone rotation etc. on network update
	g_bones.SetupBones(pLocal, g_cl.m_matrix, 128, 0x7FF00, pLocal->m_flSimulationTime());

	// save real rotation
	m_uServerAnimations.m_flFootYaw = pState->m_flFootYaw;

	// restore it
	pLocal->m_iEFlags() = ieflags_backup;
}

void ServerAnimations::HandleAnimations(bool* bSendPacket, CUserCmd* cmd) {
	if (!g_cl.m_local)
		return;

	if (!g_cl.m_local->alive())
		return;

	// handle server animations
	HandleServerAnimation(bSendPacket, cmd);
}