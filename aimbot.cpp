#include "includes.h"

Aimbot g_aimbot{ };;

void AimPlayer::AimwareExtrapolation(Player* player, vec3_t& origin, vec3_t& velocity, int& flags, bool on_ground) {
	static const auto sv_gravity = g_csgo.m_cvar->FindVar(HASH("sv_gravity"));
	static const auto sv_jump_impulse = g_csgo.m_cvar->FindVar(HASH("sv_jump_impulse"));

	if (!(flags & FL_ONGROUND))
		velocity.z -= game::TICKS_TO_TIME(sv_gravity->GetFloat());
	else if ((player->m_fFlags() & FL_ONGROUND) && !on_ground)
		velocity.z = sv_jump_impulse->GetFloat();

	const auto src = origin;
	auto end = src + velocity * g_csgo.m_globals->m_interval;

	Ray r(src, end, player->m_vecMins(), player->m_vecMaxs());

	CGameTrace t{ };
	static CTraceFilterSimple_game filter{ };
	filter.SetPassEntity(player);

	g_csgo.m_engine_trace->TraceRay(r, MASK_PLAYERSOLID, (ITraceFilter*)&filter, &t);

	if (t.m_fraction != 1.f) {
		for (auto i = 0; i < 2; i++) {
			velocity -= t.m_plane.m_normal * velocity.dot(t.m_plane.m_normal);

			const auto dot = velocity.dot(t.m_plane.m_normal);
			if (dot < 0.f)
				velocity -= vec3_t(dot * t.m_plane.m_normal.x,
					dot * t.m_plane.m_normal.y, dot * t.m_plane.m_normal.z);

			end = t.m_endpos + velocity * game::TICKS_TO_TIME(1.f - t.m_fraction);

			r = Ray(t.m_endpos, end, player->m_vecMins(), player->m_vecMaxs());
			g_csgo.m_engine_trace->TraceRay(r, MASK_PLAYERSOLID, (ITraceFilter*)&filter, &t);

			if (t.m_fraction == 1.f)
				break;
		}
	}
	origin = end = t.m_endpos;
	end.z -= 2.f;

	r = Ray(origin, end, player->m_vecMins(), player->m_vecMaxs());
	g_csgo.m_engine_trace->TraceRay(r, MASK_PLAYERSOLID, (ITraceFilter*)&filter, &t);

	flags &= FL_ONGROUND;

	if (t.hit() && t.m_plane.m_normal.z > .7f)
		flags |= FL_ONGROUND;
}

void AimPlayer::OnNetUpdate(Player* player) {
	bool clear_records = (!g_menu.main.aimbot.enabled.get() || player->m_lifeState() == e_life_state::dead || !player->enemy(g_cl.m_local));

	// if this happens, delete all the lagrecords.
	if (clear_records) {
		m_records.clear();
		return;
	}

	auto& data = g_correction.data[player->index()];

	// update player ptr if required.
	if (m_player != player) {
		m_last_rate = m_last_cycle = -1.f;
		m_shots = 0;
		m_old_stand_move_idx = 0;
		m_old_stand_no_move_idx = 0;
		m_body_index = 0;
		m_walk_record = LagRecord{};
		m_air_brute_index = 0;
		m_high_lby_idx = 0;
		m_low_lby_idx = 0;
		m_stand_move_idx = m_stand_no_move_idx = 0;
		m_move_data.m_sim_time = -1.f;
		m_update_captured = 0;
		m_delta_1 = 90.f;
		m_overlap_offset = 0.f;
		m_body_timer = FLT_MAX;
		m_has_updated = 0;
		m_high_lby_idx = m_low_lby_idx = 0;
		m_last_body = -FLT_MAX;
		m_update_count = 0;
		m_upd_time = -FLT_MAX;
		m_last_stored_body = -999.f;
		m_moved = false;
		m_change_stored = 0;
		m_last_time = m_last_rate = m_last_cycle = -1.f;
		data.m_has_updated = false;
		data.m_has_flicked = false;
		m_records.clear();
	}

	// update player ptr.
	m_player = player;

	// indicate that this player has been out of pvs.
	// insert dummy record to separate records
	// to fix stuff like animation and prediction.
	if (player->dormant()) {

		bool insert = true;

		// we have any records already?
		if (!m_records.empty()) {

			LagRecord* front = m_records.front().get();

			// we already have a dormancy separator.
			if (front->dormant())
				insert = false;
		}

		if (insert) {
			// add new record.
			m_records.emplace_front(std::make_shared< LagRecord >(player));

			// get reference to newly added record.
			LagRecord* current = m_records.front().get();

			// mark as dormant.
			current->m_dormant = true;
		}

		const int predicted_tick = g_csgo.m_globals->m_tick_count - game::TIME_TO_TICKS(0.03f / g_csgo.m_globals->m_interval);

		if (std::abs(m_last_time - game::TICKS_TO_TIME(g_csgo.m_cl->m_server_tick)) > 1.f) {
			m_last_stored_body = -999.f;
			m_move_data.m_sim_time = -1.f;
			m_change_stored = m_update_count = 0;
		}

		m_last_duration_in_air = 1.f;
		m_last_jump_fall = 1.f;
		m_last_prev_ladder = false;
		m_last_prev_ground = true;
		m_last_rate = m_last_cycle = -1.f;

		return;
	}

	// check if we received a new server update
	if (player->m_flSimulationTime() != player->m_flOldSimulationTime()) {
		// add new record.
		m_records.emplace_front(std::make_shared< LagRecord >(player));

		// get reference to newly added record.
		LagRecord* current = m_records.front().get();

		// mark as non dormant.
		current->m_dormant = false;

		// get previous record
		LagRecord* prev = m_records.size() > 1 ? m_records[1].get() : nullptr;

		// update animations (woo)
		g_anims.Update(this, current, prev);

		// save new ground state.
		m_last_prev_ground = current->m_flags & FL_ONGROUND;
		m_last_prev_ladder = (m_player->m_MoveType() == MOVETYPE_LADDER) && !(current->m_flags & FL_ONGROUND);
	}

	// erase the records every tick (since the server animates it every tick)
	int nTickRate = int(1.0f / g_csgo.m_globals->m_interval);
	while (m_records.size() > nTickRate)
		m_records.pop_back();
}

void AimPlayer::OnRoundStart(Player* player) {
	m_shots = 0;
	m_old_stand_move_idx = m_old_stand_no_move_idx = 0;
	m_body_index = m_body_pred_index = 0;
	m_air_brute_index = 0;
	m_high_lby_idx = m_low_lby_idx = 0;
	m_body_idx = 0;
	m_air_idx = 0;
	m_stand_move_idx = m_stand_no_move_idx = 0;
	m_move_data.m_sim_time = -1.f;
	m_update_captured = 0;
	m_delta_1 = m_delta_2 = 90.f;
	m_overlap_offset = 0.f;
	m_high_lby_idx = m_low_lby_idx = 0;
	m_body_timer = FLT_MAX;
	m_has_updated = 0;
	m_last_body = -FLT_MAX;
	m_update_count = 0;
	m_upd_time = -FLT_MAX;
	m_moved = false;
	m_last_time = m_last_rate = m_last_cycle = -1.f;
	m_change_stored = 0;
	m_last_stored_body = -999.f;
	m_last_prev_ground = true;
	m_last_prev_ladder = false;
	m_missed_invertfs = false;
	m_missed_last = false;
	m_missed_back = false;
	m_missed_air_back = false;
	m_last_duration_in_air = 0.f;
	m_body_index = 0;
	m_air_brute_index = 0;
	m_stand_move_idx = m_stand_no_move_idx = 0;
	m_move_data.m_sim_time = -1.f;
	m_update_captured = 0;
	m_body_timer = FLT_MAX;
	m_has_updated = 0;
	m_last_body = -FLT_MAX;
	m_update_count = 0;
	m_upd_time = -FLT_MAX;
	m_moved = false;
	m_change_stored = 0;
	m_last_time = m_last_rate = m_last_cycle = -1.f;
	m_override = false;
	m_walk_record = LagRecord{ };
	m_records.clear();
	m_hitboxes.clear();
}

void AimPlayer::SetupHitboxes(LagRecord* record) {
	// reset hitboxes.
	m_hitboxes.clear();
	m_prefer_body = false;
	m_prefer_head = false;

	if (g_cl.m_weapon_id == ZEUS) {
		// hitboxes for the zeus.
		m_hitboxes.push_back({ HITBOX_BODY });
		m_hitboxes.push_back({ HITBOX_CHEST });
		return;
	}

	m_prefer_body = true;




	if (m_prefer_head)
		m_hitboxes.push_back({ HITBOX_HEAD, HitscanMode::PREFER });

	if (m_prefer_body) {
		m_hitboxes.push_back({ HITBOX_BODY, HitscanMode::PREFER });
		m_hitboxes.push_back({ HITBOX_CHEST, HitscanMode::PREFER });
	}

	// only, on key.
	MultiDropdown hitbox{ g_menu.main.aimbot.hitboxes };

	// head
	if (hitbox.get(0)
		&& !g_aimbot.m_force_body) {
		m_hitboxes.push_back({ HITBOX_HEAD });
	}

	// stomach and pelvis
	if (hitbox.get(2)) {
		m_hitboxes.push_back({ HITBOX_BODY });
		m_hitboxes.push_back({ HITBOX_PELVIS });
	}

	// chest
	if (hitbox.get(1)) {
		m_hitboxes.push_back({ HITBOX_CHEST });

		if (!g_aimbot.m_force_body)
			m_hitboxes.push_back({ HITBOX_UPPER_CHEST });
	}

	if (hitbox.get(3)) {
		m_hitboxes.push_back({ HITBOX_L_UPPER_ARM });
		m_hitboxes.push_back({ HITBOX_R_UPPER_ARM });
	}

	// legs
	if (hitbox.get(4)) {
		m_hitboxes.push_back({ HITBOX_L_CALF });
		m_hitboxes.push_back({ HITBOX_R_CALF });

		m_hitboxes.push_back({ HITBOX_L_THIGH });
		m_hitboxes.push_back({ HITBOX_R_THIGH });
	}

	// feet
	if (hitbox.get(5)) {
		m_hitboxes.push_back({ HITBOX_L_FOOT });
		m_hitboxes.push_back({ HITBOX_R_FOOT });
	}
}

void Aimbot::init() {
	// clear old targets.
	working = false;
	m_targets.clear();
	m_target = nullptr;
	m_aim = vec3_t{ };
	m_angle = ang_t{ };
	m_stop = false;
	m_damage = 0.f;
	m_total_scanned = 0;
	m_record = nullptr;
	m_hit_chance_wall = 0;
	m_hit_chance = 0;
	m_best_damage = 0.f;
}

void Aimbot::StripAttack() {
	if (g_cl.m_weapon_id == REVOLVER)
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK2;
	else
		g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
}

void Aimbot::StartTargetSelection() {
	if (!g_cl.m_processing)
		return;

	AimPlayer* data{ };

	// setup bones for all valid targets.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!IsValidTarget(player))
			continue;

		data = &m_players[i - 1];

		if (!data
			|| !data->m_player
			|| data->m_player->index() != player->index())
			continue;

		data->m_hit = false;

		if (data->m_ignore)
			continue;

		m_targets.push_back(data);
	}
}

void Aimbot::FinishTargetSelection() {
	if (!g_cl.m_processing)
		return;

	static auto sort_targets = [&](const AimPlayer* a, const AimPlayer* b) {
		// player b and player a are the same
		// do nothing
		if (a == b)
			return false;

		// player a is invalid, if player b is valid, prioritize him
		// else do nothing
		if (!a)
			return b ? true : false;

		// player b is invalid, if player a is valid, prioritize him
		// else do nothing
		if (!b)
			return a ? true : false;

		// this is the same player
		// in that case, do nothing
		if (a->m_player == b->m_player || a->m_player->index() == b->m_player->index())
			return false;

		// get fov of player a
		float fov_a = math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, a->m_player->WorldSpaceCenter());

		// get fov of player b
		float fov_b = math::GetFOV(g_cl.m_view_angles, g_cl.m_shoot_pos, b->m_player->WorldSpaceCenter());

		// if player a fov lower than player b fov prioritize him
		return fov_a < fov_b;
		};

	// if we have only 1 targets or less, no need to sort
	if (m_targets.size() <= 1)
		return;

	// std::execution::par -> parallel sorting (multithreaded)
	// NOTE: not obligated, std::sort doesnt take alot of cpu power but its still better
	std::sort(std::execution::par, m_targets.begin(), m_targets.end(), sort_targets);

	// target limit based on our prioritized targets
	while (this->m_targets.size() > 3)
		this->m_targets.pop_back();
}

void Aimbot::think() {
	m_stop = false;

	// do all startup routines.
	init();

	// sanity.
	if (!g_cl.m_weapon || !g_cl.m_weapon_info)
		return;

	// no grenades or bomb.
	if (g_cl.m_weapon_type == WEAPONTYPE_GRENADE || g_cl.m_weapon_type == WEAPONTYPE_C4)
		return;

	// we have no aimbot enabled.
	if (!g_menu.main.aimbot.enabled.get())
		return;

	if (!g_cl.m_weapon_fire)
		StripAttack();

	// animation silent aim, prevent the ticks with the shot in it to become the tick that gets processed.
	// we can do this by always choking the tick before we are able to shoot.
	bool revolver = g_cl.m_weapon_id == REVOLVER && g_cl.m_revolver_cock != 0;

	// one tick before being able to shoot.
	if (revolver && g_cl.m_revolver_cock > 0 && g_cl.m_revolver_cock == g_cl.m_revolver_query) {
		*g_cl.m_packet = false;
		return;
	}

	// start it.
	StartTargetSelection();

	// run knifebot.
	if (g_cl.m_weapon_type == WEAPONTYPE_KNIFE && g_cl.m_weapon_id != ZEUS)
		return knife();

	// scan available targets... if we even have any.
	find();

	// we have a normal weapon or a non cocking revolver
	// choke if its the processing tick.
	if (g_cl.m_weapon_fire && !g_cl.m_lag && !revolver) {
		*g_cl.m_packet = false;
		StripAttack();
		return;
	}

	if (!g_cl.m_weapon_fire)
		return StripAttack();

	// finally set data when shooting.
	apply();
}

void Aimbot::find() {
	struct BestTarget_t { Player* player{}; vec3_t pos{}; float damage{}; int hitbox{}; int hitgroup{}; LagRecord* record{}; };

	vec3_t       tmp_pos;
	float        tmp_damage;
	int			 tmp_hitbox{}, tmp_hitgroup{};
	BestTarget_t best;
	float best_health = 999.f;
	best.player = nullptr;
	best.damage = -1.f;
	best.pos = vec3_t{ };
	best.record = nullptr;
	best.hitbox = -1;
	best.hitgroup = -1;

	if (m_targets.empty())
		return;

	// iterate all targets.
	for (const auto& t : m_targets) {
		if (t->m_records.empty())
			continue;

		// set em up
		t->SetupHitboxes(nullptr);

		// lol.. what hitbox are you gonna shoot at?
		if (t->m_hitboxes.empty())
			continue;
		
		const int pred = g_lagcomp.StartPrediction(t);

		// this player broke lagcomp.
		// his bones have been resetup by our lagcomp.
		// therfore now only the front record is valid.
		if (pred != RETURN_NO_LC) {
			if (pred == RETURN_DELAY)
				continue;

			LagRecord* front = t->m_records.front().get();

			// rip something went wrong..
			if (t->GetBestAimPosition(tmp_pos, tmp_damage, tmp_hitbox, tmp_hitgroup, front) && SelectTarget(front, tmp_pos, tmp_damage)) {
				// if we made it so far, set shit.
				best.player = t->m_player;
				best.pos = tmp_pos;
				best.damage = tmp_damage;
				best.record = front;
				best.hitbox = tmp_hitbox;
				best.hitgroup = tmp_hitgroup;
			}

			// breaking lc
			continue;
		}

		LagRecord* ideal = g_resolver.FindIdealRecord(t);
		if (!ideal)
			continue;

		bool hit_ideal = t->GetBestAimPosition(tmp_pos, tmp_damage, tmp_hitbox, tmp_hitgroup, ideal) && SelectTarget(ideal, tmp_pos, tmp_damage);

		// try to select best record as target.
		if (hit_ideal) {
			// if we made it so far, set shit.
			best.player = t->m_player;
			best.pos = tmp_pos;
			best.damage = tmp_damage;
			best.record = ideal;
			best.hitbox = tmp_hitbox;
			best.hitgroup = tmp_hitgroup;
		}

		// last record
		LagRecord* last = g_resolver.FindLastRecord(t);
		if (!last
			|| last == ideal)
			continue;

		// rip something went wrong..
		if (t->GetBestAimPosition(tmp_pos, tmp_damage, tmp_hitbox, tmp_hitgroup, last) && SelectTarget(last, tmp_pos, tmp_damage)) {
			// if we made it so far, set shit.
			best.player = t->m_player;
			best.pos = tmp_pos;
			best.damage = tmp_damage;
			best.record = last;
			best.hitbox = tmp_hitbox;
			best.hitgroup = tmp_hitgroup;
		}
	}

	// verify our target and set needed data
	if (best.player
		&& best.record) {
		working = true;

		float pitch;
		float backup_pose;

		// calculate aim angle.
		math::VectorAngles(best.pos - g_cl.m_shoot_pos, m_angle);

		pitch = m_angle.x;
		for (int i = 0; i <= 15; i++)
		{
			backup_pose = g_cl.m_local->m_flPoseParameter()[12];

			g_cl.m_local->m_flPoseParameter()[12] = std::clamp((m_angle.x + 90.f) / 180.f, 0.0f, 1.1f);
			const auto ret = g_bones.SetupBones(g_cl.m_local, g_cl.m_matrix, 128, 0x7FF00, g_cl.m_local->m_flSimulationTime());
			g_cl.m_local->m_flPoseParameter()[12] = backup_pose;

			g_cl.m_shoot_pos = g_cl.m_local->GetShootPosition(g_cl.m_matrix);

			// recalculate aim angle.
			math::VectorAngles(best.pos - g_cl.m_shoot_pos, m_angle);

			// angle is already close enough
			if (std::abs(math::AngleDiff(m_angle.x, pitch)) <= 0.05f)
				break;
		}

		// set member vars.
		m_target = best.player;
		m_aim = best.pos;
		m_damage = best.damage;
		m_record = best.record;
		m_hitbox = best.hitbox;
		m_hitgroup = best.hitgroup;

		// write data, needed for traces / etc.
		m_record->cache();

		// set autostop shit.
		if (g_cl.m_local->m_fFlags() & FL_ONGROUND
			&& !(g_cl.m_buttons & IN_JUMP) && !(g_cl.m_weapon_id == ZEUS)
			&& g_menu.main.aimbot.quick_stop_mode.get() > 0) {
			if (g_cl.m_weapon_fire || (g_menu.main.aimbot.quick_stop_mode.get() == 2 && g_cl.m_player_fire))
				m_stop = true;
		}

		m_shot_hitchance = 0.f;
		bool hit = CheckHitchance(m_target, m_angle, m_hitbox, m_record, &m_shot_hitchance);


		// if we can scope.
		bool can_scope = !g_cl.m_local->m_bIsScoped()
			&& (g_cl.m_weapon_id == AUG || g_cl.m_weapon_id == SG553 || g_cl.m_weapon_type == WEAPONTYPE_SNIPER_RIFLE);

		if (m_stop) {
			if (can_scope) {
				// always.
				if (g_menu.main.aimbot.auto_scope.get()) {
					g_cl.m_cmd->m_buttons |= IN_ATTACK2;
					g_cl.m_cmd->m_buttons &= ~IN_ATTACK;
					return;
				}
			}
		}

		if (hit) {
			// right click attack.
			if (g_cl.m_weapon_id == REVOLVER)
				g_cl.m_cmd->m_buttons |= IN_ATTACK2;

			// left click attack.
			else
				g_cl.m_cmd->m_buttons |= IN_ATTACK;
		}
	}
}

bool Aimbot::SelectTarget(LagRecord* record, const vec3_t& aim, float damage) {

	if (damage > m_best_damage) {
		m_best_damage = damage;
		return true;
	}

	return false;
}

bool can_hit_hitbox(const vec3_t& start, const vec3_t& end, Player* player, int hitbox, LagRecord* record, matrix3x4_t* matrix)
{
	auto current_bones = matrix ? matrix : record->m_bones;
	auto model = player->GetModel();
	if (!model)
		return false;

	auto studio_model = g_csgo.m_model_info->GetStudioModel(model);
	auto set = studio_model->GetHitboxSet(0);

	if (!set)
		return false;

	auto studio_box = set->GetHitbox(hitbox);
	if (!studio_box)
		return false;

	vec3_t min{ }, max{ };

	math::VectorTransform(studio_box->m_mins, current_bones[studio_box->m_bone], min);
	math::VectorTransform(studio_box->m_maxs, current_bones[studio_box->m_bone], max);

	if (studio_box->m_radius <= 0.f)
		return math::SegmentToSegment(start, end, min, max) < studio_box->m_radius;

	math::VectorITransform(start, current_bones[studio_box->m_bone], min);
	math::VectorITransform(end, current_bones[studio_box->m_bone], max);
	return math::IntersectLineWithBB(min, max, studio_box->m_mins, studio_box->m_maxs);
}

bool Aimbot::CheckHitchance(Player* player, const ang_t& angle, int hitbox, LagRecord* record, float* output) {

	int hitchance = g_menu.main.aimbot.hit_chance.get();

	if (hitchance < 2.f)
		return true;

	auto pModel = player->GetModel();
	if (!pModel)
		return false;

	auto pHdr = g_csgo.m_model_info->GetStudioModel(pModel);
	if (!pHdr)
		return false;

	auto pHitboxSet = pHdr->GetHitboxSet(player->m_nHitboxSet());

	if (!pHitboxSet)
		return false;

	auto pHitbox = pHitboxSet->GetHitbox(hitbox);

	if (!pHitbox)
		return false;

	auto state = g_cl.m_local->m_PlayerAnimState();
	if (!state)
		return false;

	auto weap_data = g_cl.m_weapon->GetWpnData();

	vec3_t forward, right, up;
	math::AngleVectors(angle, &forward, &right, &up);

	int TraceHits = 0;
	int cNeededHits = static_cast<int>(255.f * (hitchance * 0.01f));

	g_cl.m_weapon->UpdateAccuracyPenalty();
	float weap_sir = g_cl.m_weapon->GetSpread();
	float weap_inac = g_cl.m_weapon->GetInaccuracy();
	auto recoil_index = g_cl.m_weapon->m_flRecoilIndex();

	if (g_cl.m_weapon_id == ZEUS)
		hitchance = 80.f;

	if (weap_sir <= 0.f)
		return true;

	for (int i = 0; i < 255; i++) {
		float a = g_csgo.RandomFloat(0.f, 1.f);
		float b = g_csgo.RandomFloat(0.f, 6.2831855f);
		float c = g_csgo.RandomFloat(0.f, 1.f);
		float d = g_csgo.RandomFloat(0.f, 6.2831855f);

		float inac = a * weap_inac;
		float sir = c * weap_sir;

		if (g_cl.m_weapon_id == REVOLVER) {
			a = 1.f - a * a;
			a = 1.f - c * c;
		}
		else if (g_cl.m_weapon->m_iItemDefinitionIndex() == NEGEV && recoil_index < 3.0f) {
			for (int i = 3; i > recoil_index; i--) {
				a *= a;
				c *= c;
			}

			a = 1.0f - a;
			c = 1.0f - c;
		}

		// credits: haircuz
		else if (!(g_cl.m_flags & FL_ONGROUND) && g_cl.m_weapon_id == SSG08) {
			if (weap_inac < 0.009f) {
				return true;
			}
		}

		vec3_t sirVec((cos(b) * inac) + (cos(d) * sir), (sin(b) * inac) + (sin(d) * sir), 0), direction;

		direction.x = forward.x + (sirVec.x * right.x) + (sirVec.y * up.x);
		direction.y = forward.y + (sirVec.x * right.y) + (sirVec.y * up.y);
		direction.z = forward.z + (sirVec.x * right.z) + (sirVec.y * up.z);
		direction.normalize();

		ang_t viewAnglesSpread;
		math::VectorAngles(direction, viewAnglesSpread, &up);
		viewAnglesSpread.normalize();

		vec3_t viewForward;
		math::AngleVectors(viewAnglesSpread, &viewForward);
		viewForward.normalize_in_place();

		viewForward = g_cl.m_shoot_pos + (viewForward * weap_data->m_range);

		CGameTrace tr;
		// glass fix xD
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(g_cl.m_shoot_pos, viewForward), MASK_SHOT | CONTENTS_GRATE | CONTENTS_WINDOW, player, &tr);

		//int hit = ClipRayToHitbox(Ray(g_cl.m_shoot_pos, viewForward), pHitbox, record->m_bones[pHitbox->m_bone], tr);
		//
		//if (hit >= 0)
		//++TraceHits;

		// additional checks if we are actually hitting that specific hitbox.
		if (tr.m_entity == player && game::IsValidHitgroup(tr.m_hitgroup) /* && GetHitgroupFromHitbox(hitbox) == tr.m_hitgroup*/) // last check causes us to not shoot in a LOT of cases due to wrong filters i believe
			++TraceHits;

		// adding manual accuracy boost calculation here
		if (static_cast<int>((static_cast<float>(TraceHits) / 255.f) * 100.f) >= hitchance) {
			*output = static_cast<int>((static_cast<float>(TraceHits) / 255.f) * 100.f);
			return true;
		}

		if ((255 - i + TraceHits) < cNeededHits)
			return false;
	}

	return false;
}

bool AimPlayer::SetupHitboxPoints(LagRecord* record, matrix3x4_t* bones, int index, std::vector< vec3_t >& points) {
	// reset points.
	points.clear();

	const model_t* model = m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(m_player->m_nHitboxSet());
	if (!set)
		return false;

	mstudiobbox_t* bbox = set->GetHitbox(index);
	if (!bbox)
		return false;

	// get hitbox scales.
	float head_scale_menu = g_menu.main.aimbot.head_scale.get() / 100.f;
	float body_scale_menu = g_menu.main.aimbot.body_scale.get() / 100.f;
	float limbs_scale_menu = g_menu.main.aimbot.feet_scale.get() / 100.f;

	if (g_cl.m_weapon_id == ZEUS)
		body_scale_menu = head_scale_menu = 0.7f;

	// big inair fix.
	if (!(record->m_pred_flags & FL_ONGROUND))
		body_scale_menu = head_scale_menu = 0.5f;

	// these indexes represent boxes.
	if (bbox->m_radius <= 0.f) {
		// references: 
		//      https://developer.valvesoftware.com/wiki/Rotation_Tutorial
		//      CBaseAnimating::GetHitboxBonePosition
		//      CBaseAnimating::DrawServerHitboxes

		// convert rotation angle to a matrix.
		matrix3x4_t rot_matrix;
		g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

		// apply the rotation to the entity input space (local).
		matrix3x4_t matrix;
		math::ConcatTransforms(bones[bbox->m_bone], rot_matrix, matrix);

		// extract origin from matrix.
		vec3_t origin = matrix.GetOrigin();

		// compute raw center point.
		vec3_t center = (bbox->m_mins + bbox->m_maxs) / 2.f;

		// the feet hiboxes have a side, heel and the toe.
		if (index == HITBOX_R_FOOT || index == HITBOX_L_FOOT) {
			float d1 = (bbox->m_mins.z - center.z) * 0.875f;

			// invert.
			if (index == HITBOX_L_FOOT)
				d1 *= -1.f;

			// side is more optimal then center.
			points.push_back({ center.x, center.y, center.z + d1 });

			if (g_menu.main.aimbot.multipoints.get(4)) {
				// get point offset relative to center point
				// and factor in hitbox scale.
				float d2 = (bbox->m_mins.x - center.x) * limbs_scale_menu;
				float d3 = (bbox->m_maxs.x - center.x) * limbs_scale_menu;

				// heel.
				points.push_back({ center.x + d2, center.y, center.z });

				// toe.
				points.push_back({ center.x + d3, center.y, center.z });
			}
		}

		// nothing to do here we are done.
		if (points.empty())
			return false;

		// rotate our bbox points by their correct angle
		// and convert our points to world space.
		for (auto& p : points) {
			// VectorRotate.
			// rotate point by angle stored in matrix.
			p = { p.dot(matrix[0]), p.dot(matrix[1]), p.dot(matrix[2]) };

			// transform point to world space.
			p += origin;
		}
	}

	// these hitboxes are capsules.
	else {
		// factor in the pointscale.
		float head_scale = bbox->m_radius * head_scale_menu;
		float head_scale_backwards = bbox->m_radius * 0.65f;
		float body_scale = bbox->m_radius * body_scale_menu;

		// compute raw center point.
		vec3_t center = (bbox->m_mins + bbox->m_maxs) / 2.f;

		// head has 5 points.
		if (index == HITBOX_HEAD) {
			// add center.
			points.push_back(center);

			if (g_menu.main.aimbot.multipoints.get(0)) {
				// rotation matrix 45 degrees.
				// https://math.stackexchange.com/questions/383321/rotating-x-y-points-45-degrees
				// std::cos( deg_to_rad( 45.f ) )
				constexpr float rotation = 0.70710678f;

				// top/back 45 deg.
				// this is the best spot to shoot at.
				points.push_back({ bbox->m_maxs.x + (rotation * head_scale_backwards), bbox->m_maxs.y + (-rotation * head_scale_backwards), bbox->m_maxs.z });

				// right.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z + head_scale });

				// left.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y, bbox->m_maxs.z - head_scale });

				// back.
				points.push_back({ bbox->m_maxs.x, bbox->m_maxs.y - head_scale, bbox->m_maxs.z });

				// get animstate ptr.
				CCSGOPlayerAnimState* state = record->m_player->m_PlayerAnimState();

				// add this point only under really specific circumstances.
				// if we are standing still and have the lowest possible pitch pose.
				if (state && record->m_velocity.length() <= 0.1f && record->m_eye_angles.x <= state->m_flAimPitchMin) {

					// bottom point.
					points.push_back({ bbox->m_maxs.x - head_scale, bbox->m_maxs.y, bbox->m_maxs.z });
				}
			}
		}

		// body has 5 points.
		else if (index == HITBOX_BODY) {
			// center.
			points.push_back(center);

			// back.
			if (g_menu.main.aimbot.multipoints.get(2))
				points.push_back({ center.x, bbox->m_maxs.y - body_scale, center.z });
		}

		else if (index == HITBOX_PELVIS || index == HITBOX_UPPER_CHEST) {
			// back.
			points.push_back({ center.x, bbox->m_maxs.y - body_scale, center.z });
		}

		// other stomach/chest hitboxes have 2 points.
		else if (index == HITBOX_THORAX || index == HITBOX_CHEST) {
			// add center.
			points.push_back(center);

			// add extra point on back.
			if (g_menu.main.aimbot.multipoints.get(1))
				points.push_back({ center.x, bbox->m_maxs.y - body_scale, center.z });
		}

		else if (index == HITBOX_R_CALF || index == HITBOX_L_CALF) {
			// add center.
			points.push_back(center);

			// half bottom.
			if (g_menu.main.aimbot.multipoints.get(3))
				points.push_back({ bbox->m_maxs.x - (bbox->m_radius / 2.f), bbox->m_maxs.y, bbox->m_maxs.z });
		}

		else if (index == HITBOX_R_THIGH || index == HITBOX_L_THIGH) {
			// add center.
			points.push_back(center);
		}

		// arms get only one point.
		else if (index == HITBOX_R_UPPER_ARM || index == HITBOX_L_UPPER_ARM) {
			// elbow.
			points.push_back({ bbox->m_maxs.x + bbox->m_radius, center.y, center.z });
		}

		// nothing left to do here.
		if (points.empty())
			return false;

		// transform capsule points.
		for (auto& p : points)
			math::VectorTransform(p, bones[bbox->m_bone], p);
	}

	return true;
}

bool AimPlayer::GetBestAimPosition(vec3_t& aim, float& damage, int& hitbox, int& hitgroup, LagRecord* record) {
	const model_t* model = m_player->GetModel();
	if (!model)
		return false;

	studiohdr_t* hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return false;

	mstudiohitboxset_t* set = hdr->GetHitboxSet(m_player->m_nHitboxSet());
	if (!set)
		return false;

	bool                  pen;
	float                 dmg, pendmg;
	HitscanData_t         best_point, best_body_point;
	std::vector< vec3_t > points;

	best_point.m_damage = best_body_point.m_damage = 0.f;

	// get player hp.
	int hp = std::min(100, m_player->m_iHealth());

	if (g_cl.m_weapon_id == ZEUS) {
		dmg = pendmg = hp + 1;
		pen = false;
	}

	else {
		dmg = g_menu.main.aimbot.min_damage.get();

		if (g_aimbot.m_damage_toggle)
			dmg = g_menu.main.aimbot.damage_override_amt.get();

		if (dmg >= 100)
			dmg = hp + (dmg - 100.f);

		pendmg = dmg;
		pen = true;
	}

	g_aimbot.m_goal_damage = dmg;

	// write all data of this record l0l.
	record->cache();

	// get max body damage
	const int health = m_player->m_iHealth();

	int scanned_hitboxes = 0;
	bool skip_this_hitbox = false;
	bool skip_all_hitboxes = false;

	// iterate hitboxes.
	for (const auto& it : m_hitboxes) {

		// reset this as we've just changed hitbox
		skip_this_hitbox = false;

		// skip all other hitboxes
		if (skip_all_hitboxes)
			break;

		mstudiobbox_t* bbox = set->GetHitbox(it.m_index);
		if (!bbox)
			return false;

		// setup points on hitbox.
		if (!SetupHitboxPoints(record, record->m_bones, it.m_index, points))
			continue;

		// increase the amount of scanned hitboxes
		++scanned_hitboxes;

		// iterate points on hitbox.
		for (const auto& point : points) {

			// skip multipoints if center is hittable
			// (if we're prefering center atleast)
			if (skip_this_hitbox)
				continue;

			// set this for quick access
			const bool is_center = point_t::type_t::TYPE_PREFERED;

			pen_data_t pen_data = g_auto_wall.get()->simulate_fire(g_cl.m_shoot_pos, point, m_player);

			if (pen_data.m_dmg >= dmg / 2.f)
				this->m_hit = true;

			// we can hit p!
			if (pen_data.m_dmg >= hp || pen_data.m_dmg >= dmg) {

				// nope we did not hit head..
				if (it.m_index <= 2 && pen_data.m_hitbox > HITBOX_LOWER_NECK)
					continue;

				// nope we did not hit body..
				if (it.m_index > 2 && pen_data.m_hitbox <= HITBOX_LOWER_NECK)
					continue;

				if (is_center)
					skip_this_hitbox = true;

				// if our point is body
				if (pen_data.m_hit_group == HITGROUP_STOMACH || pen_data.m_hit_group == HITGROUP_CHEST) {
					// set best point as current if our current point is better 
					if (best_body_point.m_damage <= 0.f) {
						best_body_point.m_damage = pen_data.m_dmg;
						best_body_point.m_hitgroup = pen_data.m_hit_group;
						best_body_point.m_pos = point;
						best_body_point.m_hitbox = it.m_index;
						best_body_point.m_center = is_center;

						// if lethal, lets skip other points & hitboxes
						if (pen_data.m_dmg >= health) {
							skip_all_hitboxes = true;

							// break out of the point loop
							break;
						}
					}

					continue;
				}

				// check if its a better point than our current best head point
				const bool better_head_point = pen_data.m_dmg - best_point.m_damage >= (is_center ? 0.f : 5.f);

				// set best point as current if our current point is better 
				if (best_point.m_damage <= 0.f || better_head_point) {
					best_point.m_damage = pen_data.m_dmg;
					best_point.m_hitgroup = pen_data.m_hit_group;
					best_point.m_pos = point;
					best_point.m_hitbox = it.m_index;
					best_point.m_center = is_center;

					// if lethal, lets skip other points
					if (pen_data.m_dmg >= health) {

						// break out of the point loop
						break;
					}
				}
			}
		}
	}

	// we havent found any point to hit, abort!!!
	if (best_point.m_damage <= 0.f && best_body_point.m_damage <= 0.f)
		return false;

	if (best_point.m_damage <= 0.f // if head has no damage
		|| best_body_point.m_damage >= best_point.m_damage // or body has more damage than head
		|| best_body_point.m_damage >= health // or body is lethal
		|| m_prefer_body // or we're prefering baim
		|| (best_body_point.m_damage * 2.f >= health)) {

		if (best_body_point.m_damage > 0.f) {
			// set data to body data
			aim = best_body_point.m_pos;
			damage = best_body_point.m_damage;
			hitbox = best_body_point.m_hitbox;
			hitgroup = best_body_point.m_hitgroup;
			return true;
		}
	}

	// else set data to head data
	aim = best_point.m_pos;
	damage = best_point.m_damage;
	hitbox = best_point.m_hitbox;
	hitgroup = best_point.m_hitgroup;
	return true;
}

void Aimbot::apply() {
	bool attack, attack2;
	static AimPlayer* data;

	// attack states.
	attack = (g_cl.m_cmd->m_buttons & IN_ATTACK);
	attack2 = (g_cl.m_weapon_id == REVOLVER && g_cl.m_cmd->m_buttons & IN_ATTACK2);

	// ensure we're attacking.
	if (attack || attack2) {
		*g_cl.m_packet = false;

		if (m_target) {
			if (m_record)
				g_cl.m_cmd->m_tick = game::TIME_TO_TICKS(m_record->m_sim_time + g_cl.m_lerp);

			// set angles to target.
			g_cl.m_cmd->m_view_angles = m_angle;

			// if not silent aim, apply the viewangles.
			if (!g_menu.main.aimbot.silent_aim.get())
				g_csgo.m_engine->SetViewAngles(m_angle);

			data = &g_aimbot.m_players[m_target->index() - 1];

			// draw aimbot matrix.
			g_visuals.DrawHitboxMatrix(m_record, 3.f);

		}

		if (m_target && m_record)
			g_shots.add( g_cl.m_shoot_pos, m_target,
				-1, g_cl.m_cmd->m_command_number, g_csgo.m_globals->m_realtime,
				g_csgo.m_engine->GetNetChannelInfo()->GetLatency(INetChannel::FLOW_INCOMING) +
				g_csgo.m_engine->GetNetChannelInfo()->GetLatency(INetChannel::FLOW_INCOMING),
				m_record, data, m_hitbox
			);

		// norecoil
		g_cl.m_cmd->m_view_angles -= g_cl.m_local->m_aimPunchAngle() * g_csgo.weapon_recoil_scale->GetFloat();

		g_movement.fired_shot = true;
		g_cl.m_shot = true;
	}
}

