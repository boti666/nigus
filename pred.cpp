#include "includes.h"
#include "pred.h"

#define VEC_VIEW			vec3_t( 0, 0, 64 )
#define VEC_HULL_MIN		vec3_t( -16, -16, 0 )
#define VEC_HULL_MAX		vec3_t( 16, 16, 72 )
#define VEC_DUCK_HULL_MIN	vec3_t( -16, -16, 0 )
#define VEC_DUCK_HULL_MAX	vec3_t( 16, 16, 36 )
#define VEC_DUCK_VIEW		vec3_t( 0, 0, 46 )

InputPrediction g_inputpred{};;

void InputPrediction::UpdatePitch(float& pitch)
{
	auto state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return;

	const auto old_abs_angles = g_cl.m_local->GetAbsAngles();
	const auto old_poses = g_cl.m_local->m_flPoseParameter()[12];

	g_cl.m_local->SetAbsAngles({ 0.f, state->m_flFootYaw, 0.f });

	auto eye_pitch = math::normalize(pitch);

	if (eye_pitch > 180.f)
		eye_pitch = eye_pitch - 360.f;

	eye_pitch = std::clamp(eye_pitch, -90.f, 90.f);
	g_cl.m_local->m_flPoseParameter()[12] = std::clamp((eye_pitch + 90.f) / 180.f, 0.0f, 1.0f);

	g_cl.m_local->InvalidateBoneCache();

	const auto old_abs_origin = g_cl.m_local->GetAbsOrigin();
	g_cl.m_local->SetAbsOrigin(g_cl.m_local->m_vecOrigin());

	g_bones.SetupBones(g_cl.m_local, g_cl.m_shoot_matrix, 128, 0x100, g_cl.m_local->m_flOldSimulationTime() + g_csgo.m_globals->m_interval);

	g_cl.m_shoot_pos = g_cl.m_local->GetShootPosition(g_cl.m_shoot_matrix);

	g_cl.m_local->SetAbsOrigin(old_abs_origin);

	g_cl.m_local->m_flPoseParameter()[12] = old_poses;
	g_cl.m_local->SetAbsAngles(old_abs_angles);
}

void InputPrediction::update() {
	bool        valid{ g_csgo.m_cl->m_delta_tick > 0 };

	// EDIT; from what ive seen RunCommand is called when u call Prediction::Update
	// so the above code is not fucking needed.

	int start = g_csgo.m_cl->m_last_command_ack;
	int stop = g_csgo.m_cl->m_last_outgoing_command + g_csgo.m_cl->m_choked_commands;

	//vec3_t abs_origin = g_cl.m_local->GetAbsOrigin();

	// call CPrediction::Update.
	if (valid)
		g_csgo.m_prediction->Update(g_csgo.m_cl->m_delta_tick, valid, start, stop);

	//g_cl.m_local->SetAbsOrigin(abs_origin);

	static bool unlocked_fakelag = false;
	if (!unlocked_fakelag) {
		auto cl_move_clamp = pattern::find(g_csgo.m_engine_dll, XOR("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC")) + 1;
		unsigned long protect = 0;

		VirtualProtect((void*)cl_move_clamp, 4, PAGE_EXECUTE_READWRITE, &protect);
		*(std::uint32_t*)cl_move_clamp = 62;
		VirtualProtect((void*)cl_move_clamp, 4, protect, &protect);
		unlocked_fakelag = true;
	}

	//return SaveNetvars();
}

void InputPrediction::SaveNetvars() {
	m_PredictionData.m_nFlags = g_cl.m_local->m_fFlags();
	m_PredictionData.m_angAimPunchAngle = g_cl.m_local->m_aimPunchAngle();
	m_PredictionData.m_angViewPunchAngle = g_cl.m_local->m_viewPunchAngle();
	m_PredictionData.m_vecAimPunchAngleVel = g_cl.m_local->m_aimPunchAngleVel();
	m_PredictionData.m_flDuckAmount = g_cl.m_local->m_flDuckAmount();
	m_PredictionData.m_flDuckSpeed = g_cl.m_local->m_flDuckSpeed();
	m_PredictionData.m_flVelocityModifier = g_cl.m_local->m_flVelocityModifier();
	m_PredictionData.m_flStamina = g_cl.m_local->m_flStamina();
	m_PredictionData.m_vecVelocity = g_cl.m_local->m_vecVelocity();
	m_PredictionData.m_vecAbsVelocity = g_cl.m_local->m_vecAbsVelocity();
	m_PredictionData.m_vecOrigin = g_cl.m_local->m_vecOrigin();
	m_PredictionData.m_vecViewOffset = g_cl.m_local->m_vecViewOffset();

	Weapon* weapon = g_cl.m_local->GetActiveWeapon();
	if (weapon) {
		m_PredictionData.m_flAccuracyPenalty = weapon->m_fAccuracyPenalty();
		m_PredictionData.m_flRecoilIndex = weapon->m_flRecoilIndex();
	}

	m_PredictionData.m_GroundEntity = g_cl.m_local->m_hGroundEntity();
}

void InputPrediction::OnPlayerMove()
{
	if (!g_cl.m_local || !g_cl.m_local->alive() || g_csgo.m_game_movement->m_Player() != g_cl.m_local)
		return;

	if (!(g_cl.m_local->m_fFlags() & FL_DUCKING) && !g_cl.m_local->m_bDucking() && !g_cl.m_local->m_bDucked())
		g_cl.m_local->m_vecViewOffset() = VEC_VIEW;
	else if (g_cl.m_local->m_bDuckUntilOnGround())
	{
		vec3_t hullSizeNormal = VEC_HULL_MAX - VEC_HULL_MIN;
		vec3_t hullSizeCrouch = VEC_DUCK_HULL_MAX - VEC_DUCK_HULL_MIN;
		vec3_t lowerClearance = hullSizeNormal - hullSizeCrouch;
		vec3_t duckEyeHeight = g_csgo.m_game_movement->GetPlayerViewOffset(false) - lowerClearance;

		g_cl.m_local->m_vecViewOffset() = duckEyeHeight;
	}
	else if (g_cl.m_local->m_bDucked() && !g_cl.m_local->m_bDucking())
		g_cl.m_local->m_vecViewOffset() = VEC_DUCK_VIEW;
}

void RunPostThink(Player* ent) {
	g_csgo.m_model_cache->BeginLock();

	static auto post_think_vphysics = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 8B D9 56 57 83 BB")).as< bool(__thiscall*)(Player*) >();
	static auto simulate_player_simulated_entities = pattern::find(g_csgo.m_client_dll, XOR("56 8B F1 57 8B BE ? ? ? ? 83 EF 01 78 72 90 8B 86")).as< void(__thiscall*)(void*) >();

	if (ent->alive()) {
		util::get_method< void(__thiscall*)(void*) >(ent, 329) (ent);

		if (ent->m_fFlags() & FL_ONGROUND)
			ent->m_flFallVelocity() = 0.f;

		if (ent->m_nSequence() == -1)
			ent->SetSequence(0);

		util::get_method< void(__thiscall*)(void*) >(ent, 214) (ent);
		post_think_vphysics(ent);
	}

	simulate_player_simulated_entities(ent);
	g_csgo.m_model_cache->EndLock();
}

void InputPrediction::run() {
	static CMoveData data{};

	g_csgo.m_prediction->m_in_prediction = true;

	// CPrediction::StartCommand
	g_cl.m_local->m_pCurrentCommand() = g_cl.m_cmd;
	g_cl.m_local->m_LastCmd() = *g_cl.m_cmd;
	*g_csgo.m_nPredictionRandomSeed = g_cl.m_cmd ? g_cl.m_cmd->m_random_seed : -1;
	g_csgo.m_pPredictionPlayer = g_cl.m_local;

	// backup globals.
	m_curtime = g_csgo.m_globals->m_curtime;
	m_frametime = g_csgo.m_globals->m_frametime;

	//vec3_t abs_origin = g_cl.m_local->GetAbsOrigin();

	// set globals appropriately.
	g_csgo.m_globals->m_curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
	g_csgo.m_globals->m_frametime = g_csgo.m_prediction->m_engine_paused ? 0.f : g_csgo.m_globals->m_interval;

	g_cl.m_cmd->m_buttons |= g_cl.m_local->m_afButtonForced();
	// g_cl.m_cmd->m_buttons &= ~g_cl.m_local->m_afButtonDisabled(); // server does this line too so Fxck it why not // ok it makes us not be able to do anything but move like no jump no shoot so derp

	// set target player ( host ).
	g_csgo.m_move_helper->SetHost(g_cl.m_local);
	g_csgo.m_game_movement->StartTrackPredictionErrors(g_cl.m_local);

	// Do weapon selection
	if (g_cl.m_cmd->m_weapon_select != 0) {
		Weapon* weapon = g_csgo.m_entlist->GetClientEntity<Weapon*>(g_cl.m_cmd->m_weapon_select);
		if (weapon) {
			WeaponInfo* weapon_info = weapon->GetWpnData();

			if (weapon_info)
				g_cl.m_local->SelectItem(weapon_info->m_weapon_name, g_cl.m_cmd->m_weapon_subtype);
		}
	}

	// latch in impulse.
	if (g_cl.m_cmd->m_impulse)
		g_cl.m_local->m_nImpulse() = g_cl.m_cmd->m_impulse;

	// get button states.
	g_cl.m_local->UpdateButtonState(g_cl.m_cmd->m_buttons);

	g_csgo.m_prediction->CheckMovingGround(g_cl.m_local, g_csgo.m_globals->m_frametime);

	// copy from command to player unless game .dll has set angle using fixangle.
	g_cl.m_local->SetLocalViewAngles(g_cl.m_cmd->m_view_angles);

	// call standard client pre-think.
	g_cl.m_local->RunPreThink();

	// call Think if one is set.
	g_cl.m_local->RunThink(g_csgo.m_globals->m_interval);

	// setup input.
	g_csgo.m_prediction->SetupMove(g_cl.m_local, g_cl.m_cmd, g_csgo.m_move_helper, &data);

	// run movement.
	g_csgo.m_game_movement->ProcessMovement(g_cl.m_local, &data);
	g_csgo.m_prediction->FinishMove(g_cl.m_local, g_cl.m_cmd, &data);

	//g_csgo.m_move_helper->ProcessImpacts();

	RunPostThink(g_cl.m_local);

	g_csgo.m_game_movement->FinishTrackPredictionErrors(g_cl.m_local);
	// reset target player ( host ).
	g_csgo.m_move_helper->SetHost(nullptr);
	//g_cl.m_local->SetAbsOrigin(abs_origin);
}

void InputPrediction::restore() {
	g_csgo.m_prediction->m_in_prediction = false;

	g_cl.m_local->m_pCurrentCommand() = NULL;
	*g_csgo.m_nPredictionRandomSeed = -1;
	g_csgo.m_pPredictionPlayer = nullptr;

	g_csgo.m_game_movement->Reset();

	// restore globals.
	g_csgo.m_globals->m_curtime = m_curtime;
	g_csgo.m_globals->m_frametime = m_frametime;

	if (!g_csgo.m_prediction->m_engine_paused && g_csgo.m_globals->m_frametime > 0.f)
		g_cl.m_local->m_nTickBase()++;

	//g_cl.m_cmd->m_predicted = true;

}

void InputPrediction::SaveViewmodel() {
	auto view_model = g_csgo.m_entlist->GetClientEntityFromHandle(g_cl.m_local->m_hViewModel());

	if (!view_model)
		return;

	m_viewmodel.m_animation_parity = view_model->m_nAnimationParity();
	m_viewmodel.m_view_sequence = view_model->m_nSequenceViewmodel();
	m_viewmodel.m_view_cycle = view_model->m_flCycle();
	m_viewmodel.m_anim_time = view_model->m_flAnimTime();
}

void InputPrediction::FixViewmodel() {
	auto view_model = g_csgo.m_entlist->GetClientEntityFromHandle(g_cl.m_local->m_hViewModel());

	if (!view_model)
		return;

	if (m_viewmodel.m_view_sequence != view_model->m_nSequenceViewmodel() || m_viewmodel.m_animation_parity != view_model->m_nAnimationParity())
		return;

	view_model->m_flCycle() = m_viewmodel.m_view_cycle;
	view_model->m_flAnimTime() = m_viewmodel.m_anim_time;
}

void InputPrediction::PingReducer() {
	if (!g_csgo.m_cl || g_csgo.m_cl->m_next_message_time < 6) {
		m_read_packets = true;
		return;
	}

	if (!g_cl.m_processing) {
		m_read_packets = true;
		return;
	}

	INetChannel* channel = g_csgo.m_engine->GetNetChannelInfo();
	if (!channel || channel->IsLoopback()) {
		m_read_packets = true;
		return;
	}

	std::array< vec3_t, 64 > m_origin;

	for (int i = 1; i <= g_csgo.m_globals->m_max_clients; i++) {
		Player* player = g_csgo.m_entlist->GetClientEntity<Player*>(i);

		if (!player || !player->alive())
			continue;

		m_origin[i - 1] = player->GetAbsOrigin();
	}

	m_read_packets = true;
	g_csgo.CL_ReadPackets(true);
	m_read_packets = false;

	for (int i = 1; i <= g_csgo.m_globals->m_max_clients; i++) {
		Player* player = g_csgo.m_entlist->GetClientEntity<Player*>(i);

		if (!player || !player->alive())
			continue;

		player->SetAbsOrigin(m_origin[i - 1]);
	}
}

CPredictionCopy::CPredictionCopy(int type, byte* dest, bool dest_packed, const byte* src, bool src_packed,
	optype_t opType, FN_FIELD_COMPARE func /*= NULL*/) {
	m_OpType = opType;
	m_nType = type;
	m_pDest = dest;
	m_pSrc = src;
	m_nDestOffsetIndex = dest_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;
	m_nSrcOffsetIndex = src_packed ? TD_OFFSET_PACKED : TD_OFFSET_NORMAL;

	m_nErrorCount = 0;
	m_nEntIndex = -1;

	m_pWatchField = nullptr;
	m_FieldCompareFunc = func;
}

int CPredictionCopy::TransferData(const char* operation, int entindex, datamap_t* dmap) {
	using TransferDataFn = int(__thiscall*)(CPredictionCopy*, const char*, int, datamap_t*);
	static auto pat = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 8B 45 10 53 56 8B F1 57")).as< TransferDataFn >();
	return pat(this, operation, entindex, dmap);
}

bool CPredictionCopy::PrepareDataMap(datamap_t* dmap) {
	dmap->m_packed_size = 0;
	using PrepDataMap = bool(__thiscall*)(datamap_t*);
	static auto pat = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC ? 57 8B F9 89 7D ? 83 7F ? ?")).as< PrepDataMap >();
	return pat(dmap);
}