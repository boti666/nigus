#include "includes.h"

Resolver g_resolver{};;

void Resolver::OnBodyUpdate(Player* player, float value)
{
	auto& g_data = g_correction.data[player->index()];


	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	g_data.m_old_body_yaw = g_data.m_body_yaw;
	g_data.m_body_yaw = player->m_flPoseParameter()[11];
}

float Resolver::AntiFreestand(Player* player, LagRecord* record, vec3_t start_, vec3_t end, bool include_base, float base_yaw, float delta) {
	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 32.f };

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };

	angles.emplace_back(base_yaw + delta);
	angles.emplace_back(base_yaw - delta);

	if (include_base)
		angles.emplace_back(base_yaw);

	// start the trace at the enemy shoot pos.
	vec3_t start = start_;

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// get the enemies shoot pos.
	vec3_t shoot_pos = end;

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ shoot_pos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			shoot_pos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			shoot_pos.z };

		// draw a line for debugging purposes.
		//g_csgo.m_debug_overlay->AddLineOverlay( start, end, 255, 0, 0, true, 0.1f );

		// compute the direction.
		vec3_t dir = end - start;
		float len = dir.normalize();

		// should never happen.
		if (len <= 0.f)
			continue;

		// step thru the total distance, 4 units per step.
		for (float i{ 0.f }; i < len; i += STEP) {
			// get the current step position.
			vec3_t point = start + (dir * i);

			// get the contents at this point.
			int contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);

			// contains nothing that can stop a bullet.
			if (!(contents & MASK_SHOT_HULL))
				continue;

			float mult = 1.f;

			// over 50% of the total length, prioritize this shit.
			if (i > (len * 0.5f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.75f))
				mult = 1.25f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid)
		return base_yaw;

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	return angles.front().m_yaw;
}

LagRecord* Resolver::FindFirstRecord(AimPlayer* data) {
	if (data->m_records.empty())
		return nullptr;

	return data->m_records.front().get();
}

LagRecord* Resolver::FindIdealRecord(AimPlayer* data) {
	LagRecord* first_valid, * current;

	if (data->m_records.empty())
		return nullptr;

	first_valid = nullptr;

	// iterate records.
	for (const auto& it : data->m_records) {
		if (it->dormant() || it->immune() || !it->valid() || it->m_broke_lc)
			continue;

		// get current record.
		current = it.get();

		// first record that was valid, store it for later.
		if (!first_valid)
			first_valid = current;

		// try to find a record with a shot, lby update, walking or no anti-aim.
		if (it->m_shot || it->m_mode == Modes::RESOLVE_LBY_PRED || it->m_mode == Modes::RESOLVE_LBY || it->m_mode == Modes::RESOLVE_WALK || it->m_mode == Modes::RESOLVE_NONE)
			return current;
	}

	// none found above, return the first valid record if possible.
	return (first_valid) ? first_valid : nullptr;
}

LagRecord* Resolver::FindLastRecord(AimPlayer* data) {
	LagRecord* current;

	if (data->m_records.empty())
		return nullptr;

	// iterate records in reverse.
	for (auto it = data->m_records.crbegin(); it != data->m_records.crend(); ++it) {
		current = it->get();

		// if this record is valid.
		// we are done since we iterated in reverse.
		if (current->valid() && !current->immune() && !current->dormant() && !current->m_broke_lc)
			return current;
	}

	return nullptr;
}

float Resolver::GetAwayAngle(LagRecord* record) {
	float  delta{ std::numeric_limits< float >::max() };
	vec3_t pos;
	ang_t  away;

	math::VectorAngles(g_cl.m_local->m_vecOrigin() - record->m_origin, away);
	return away.y;
}

void Resolver::OnPlayerFire(LagRecord* record, AimPlayer* data) {
	Weapon* wpn = data->m_player->GetActiveWeapon();

	if (!wpn)
		return;

	WeaponInfo* wpn_data = wpn->GetWpnData();

	if (!wpn_data)
		return;

	if ((wpn_data->m_weapon_type != WEAPONTYPE_GRENADE && wpn_data->m_weapon_type > 6) || wpn_data->m_weapon_type <= 0)
		return;

	const auto shot_time = wpn->m_fLastShotTime();
	const auto shot_tick = game::TIME_TO_TICKS(shot_time);
	const auto sim_tick = game::TIME_TO_TICKS(record->m_sim_time);
	const auto anim_tick = game::TIME_TO_TICKS(record->m_anim_time);

	player_info_t info;
	g_csgo.m_engine->GetPlayerInfo(record->m_player->index(), &info);

	// player fired this tick
	if (shot_tick == sim_tick && record->m_lag <= 2)
	{
		// dont need to fix pitch here, they should have onshot.
		record->m_fixing_pitch = false;
		record->m_shot = record->m_sim_tick = true;
		//g_cl.print(tfm::format("player fired | lag: %d | shot_tick: %d | sim tick: %f | anim tick: %f \n", record->m_lag, shot_tick, sim_tick, anim_tick));
	}
	else
	{
		if (shot_tick > anim_tick && sim_tick > shot_tick)
		{
			//g_cl.print(tfm::format("player fired 3 | lag: %d | shot_tick: %d | sim tick: %f | anim tick: %f \n", record->m_lag, shot_tick, sim_tick, anim_tick));
			record->m_shot = record->m_fixing_pitch = true;
		}
		else if (shot_tick == anim_tick)
		{
			//g_cl.print(tfm::format("player fired 2 | lag: %d | shot_tick: %d | sim tick: %f | anim tick: %f \n", record->m_lag, shot_tick, sim_tick, anim_tick));

			// dont need to fix pitch here, they should have onshot.
			if (shot_tick + 1 == sim_tick) {
				record->m_shot = true;
				record->m_fixing_pitch = false;
				return;
			}

			// fix it
			record->m_shot = record->m_fixing_pitch = true;
		}
	}

	if (record->m_fixing_pitch) {
		float valid_pitch = 89.f;

		for (const auto& it : data->m_records) {
			if (it.get() == record || it->m_dormant)
				continue;

			if (!it->m_fixing_pitch && !record->m_sim_tick) {
				valid_pitch = it->m_eye_angles.x;
				break;
			}
		}

		record->m_eye_angles.x = valid_pitch;
	}
}

bool Resolver::UnsafeVelocityTransitionDetection(LagRecord* pRecord, AimPlayer* pData)
{
	if (pData->m_records.size() <= 1 || !pData->m_records[1].get() || pData->m_records[1].get()->dormant())
		return false;

	if (pRecord->m_velocity.length_2d() <= 0.f || pRecord->m_velocity.length() > 35.f || !(pRecord->m_flags & FL_ONGROUND))
		return false;

	if (pRecord->m_layers[6].weight == 0.0f || pRecord->m_layers[6].weight == 1.0f)
		return false;

	return pRecord->m_layers[6].weight != pData->m_records[1].get()->m_layers[6].weight;
}

bool Resolver::IsFakeFlick(LagRecord* pRecord, AimPlayer* pData)
{
	static bool bFakeFlick = false;
	static float flCurtime = 0.f;

	if (pRecord->m_unsafe_vel)
	{
		bFakeFlick = true;
		flCurtime = g_csgo.m_globals->m_curtime + 1.1f;
	}
	else if (flCurtime <= g_csgo.m_globals->m_curtime)
		bFakeFlick = false;

	if (pRecord->m_velocity.length_2d() > 0.f && !pRecord->m_unsafe_vel || !(pRecord->m_flags & FL_ONGROUND)) {
		bFakeFlick = false;
		flCurtime = 0.f;
	}

	return bFakeFlick;
}

void Resolver::SetMode(LagRecord* record) {
	// the resolver has 3 modes to chose from.
	// these modes will vary more under the hood depending on what data we have about the player
	// and what kind of hack vs. hack we are playing (mm/nospread).

	float speed = record->m_anim_velocity.length();

	if (g_aimbot.m_override && speed <= 20.f && record->m_flags & FL_ONGROUND)
		record->m_mode = Modes::RESOLVE_OVERRIDE;
	else
	{
		// if on ground, moving, and not fakewalking.
		if ((record->m_flags & FL_ONGROUND) && speed > 0.1f && !record->m_fake_walk)
			record->m_mode = Modes::RESOLVE_WALK;

		// if on ground, not moving or fakewalking.
		if ((record->m_flags & FL_ONGROUND) && (speed < 0.1f || record->m_fake_walk))
			record->m_mode = Modes::RESOLVE_STAND;

		// if not on ground.
		else if (!(record->m_flags & FL_ONGROUND))
			record->m_mode = Modes::RESOLVE_AIR;
	}
}

bool Resolver::IsSideways(float angle, LagRecord* record) {
	ang_t  away;
	math::VectorAngles(g_cl.m_shoot_pos - record->m_pred_origin, away);
	const float diff = math::AngleDiff(away.y, angle);
	return diff > 25.f && diff < 135.f;
}

void Resolver::SetupResolveChance(AimPlayer* data, LagRecord* record) {
	record->m_resolver_chance = "LOW";

	const float length2d = record->m_velocity.length_2d();

	if (data->m_records.size() >= 2) {

		LagRecord* previous = data->m_records[1]->dormant() ? nullptr : data->m_records[1].get();
		const bool run = length2d > 35.f && record->m_ground_for_two_ticks;
		const float time_since_moving = record->m_anim_time - data->m_move_data.m_time;
		const float eye_delta = std::abs(math::AngleDiff(record->m_eye_angles.y, record->m_body));

		if (previous) {
			if (record->m_flags & FL_ONGROUND) {
				if (length2d > 0.1f) {
					record->m_priority = run ? priority_t::PRIORITY_RESOLVED : priority_t::PRIORITY_HIGH;
					record->m_resolver_chance = run ? "MAX" : "HIGH";
					return;
				}

				const float body_diff = std::abs(math::AngleDiff(record->m_body, previous->m_body));

				if (body_diff > 0.5f) {
					record->m_priority = body_diff > 35.f && record->m_ground_for_two_ticks ? priority_t::PRIORITY_RESOLVED : priority_t::PRIORITY_HIGH;
					record->m_resolver_chance = body_diff > 35.f && record->m_ground_for_two_ticks ? "MAX" : "VERY HIGH";
					return;
				}

				if (time_since_moving > 0.f
					&& time_since_moving < 0.22f
					&& eye_delta <= 10.f) {
					record->m_priority = priority_t::PRIORITY_HIGH;
					record->m_resolver_chance = "HIGH";
					return;
				}

				if (record->m_mode == RESOLVE_LBY_PRED) {
					record->m_priority = priority_t::FLICK;
					record->m_resolver_chance = "HIGH";
					return;
				}

				if (data->m_change_stored > 2) {
					record->m_priority = priority_t::PRIORITY_VERY_LOW;
					record->m_resolver_chance = "VERY LOW";
					return;
				}

				if (eye_delta <= 35.f) {
					record->m_priority = priority_t::PRIORITY_MEDIUM;
					record->m_resolver_chance = "MEDIUM";
					return;
				}
			}
			else {
				if (record->m_mode == Modes::RESOLVE_LBY) {
					record->m_priority = priority_t::PRIORITY_HIGH;
					record->m_resolver_chance = "HIGH";
					return;
				}

				if (time_since_moving <= 0.25f && time_since_moving > 0.f) {

					if (std::fabs(record->m_eye_angles.y - data->m_move_data.m_body) <= 15.f
						&& std::fabs(record->m_eye_angles.y - record->m_body) <= 15.f) {
						record->m_priority = priority_t::PRIORITY_RESOLVED;
						record->m_resolver_chance = "MAX";
						return;
					}

					record->m_priority = priority_t::PRIORITY_MEDIUM;
					record->m_resolver_chance = "MEDIUM";
					return;
				}

				if (data->m_move_data.m_sim_time <= 0.f) {
					record->m_priority = priority_t::PRIORITY_VERY_LOW;
					record->m_resolver_chance = "VERY LOW";
					return;
				}
			}
		}
	}
}

void Resolver::UpdateLBYPrediction(AimPlayer* data, LagRecord* record)
{
	// Check if the player's animation velocity is significant
	if (record->m_anim_velocity.length() > 0.1f) {
		data->m_body_timer = (record->m_flags & FL_ONGROUND) ? (record->m_anim_time + 0.22f) : FLT_MAX;
		data->m_detected_body_upd = 0;
		data->m_updated = false;
		data->m_triggered_979 = false;
		return;
	}

	// Check for a significant difference in the body yaw
	if (data->m_old_body != FLT_MAX && fabs(math::AngleDiff(record->m_body, data->m_old_body)) > 30.f) {
		data->m_detected_body_upd = 1;
		data->m_body_timer = record->m_anim_time + 1.1f;
		data->m_updated = true;
	}

	// Update the old body yaw
	data->m_old_body = record->m_body;

	// Get the activity of layer 3
	int layer_3_activity = record->m_player->GetSequenceActivity(record->m_layers[3].sequence);

	// Check for LBY updates through activity detection
	if (data->m_detected_body_upd != 1
		&& data->m_last_lby_activity != ACT_CSGO_IDLE_TURN_BALANCEADJUST
		&& layer_3_activity == ACT_CSGO_IDLE_TURN_BALANCEADJUST
		&& data->m_last_lby_activity != layer_3_activity
		&& record->m_layers[3].cycle > 0.0f
		&& record->m_layers[3].weight > 0.0f)
	{
		data->m_detected_body_upd = 2;
		data->m_body_timer = record->m_anim_time - (record->m_layers[3].cycle / record->m_layers[3].playback_rate);
		data->m_triggered_979 = true;
	}

	// Update the last LBY activity
	data->m_last_lby_activity = layer_3_activity;

	// Improved LBY prediction
	if (record->m_anim_time >= data->m_body_timer && data->m_body_timer < FLT_MAX) {
		record->m_eye_angles.y = record->m_body;
		data->m_body_timer = record->m_anim_time + 1.1f;
		data->m_updated = true;
		record->m_mode = Modes::RESOLVE_LBY_PRED;
		return;
	}

	// Get the active weapon and check its last shot time
	Weapon* weapon = data->m_player->GetActiveWeapon();
	if (weapon) {
		float shoot_time = weapon->m_fLastShotTime() + g_csgo.m_globals->m_interval_per_tick;
		// Convert times to ticks and compare
		if (game::TIME_TO_TICKS(shoot_time) == game::TIME_TO_TICKS(record->m_sim_time)) {
			// Handle the event when the shoot time matches the record simulation time
			data->m_shot_time = shoot_time; // Log the shot time
			data->m_shots = true; // Update player's state to reflect that a shot was taken

			// Additional handling logic can be added here as needed
			// For example, adjusting the resolver's behavior based on the shot time
		}
	}

	// Additional checks for on-ground and air state
	if (record->m_flags & FL_ONGROUND) {
		if (data->m_air_index) {
			data->m_body_timer = record->m_anim_time + 0.22f;
			data->m_air_index = false;
		}
	}
	else {
		data->m_air_index = true;
	}

	// Additional check for player crouching
	if (record->m_flags & FL_DUCKING) {
		data->m_body_timer = record->m_anim_time + 1.1f;
	}
}

void Resolver::ResolveAngles(Player* player, LagRecord* record) {

	AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

	// set to none
	record->m_resolver_mode = "NONE";

	// run this before our pitch 0 resolver so it doesnt mess up anything
	OnPlayerFire(record, data);

	// next up mark this record with a resolver mode that will be used.
	SetMode(record);

	// update it
	UpdateLBYPrediction(data, record);

	// yep
	record->m_unsafe_vel = UnsafeVelocityTransitionDetection(record, data);
	record->m_fake_flick = IsFakeFlick(record, data);

	data->m_override = false;

	switch (record->m_mode) {
	case RESOLVE_WALK:
		ResolveWalk(data, record);
		break;
	case RESOLVE_STAND:
		ResolveStand(data, record);
		break;
	case RESOLVE_AIR:
		record->m_eye_angles.y = record->m_body;
		break;
	case RESOLVE_OVERRIDE:
		ResolveOverride(data, record, player);
		break;
	}

	if (data->m_old_stand_move_idx != data->m_stand_move_idx
		|| data->m_old_stand_no_move_idx != data->m_stand_no_move_idx) {

		data->m_old_stand_move_idx = data->m_stand_move_idx;
		data->m_old_stand_no_move_idx = data->m_stand_no_move_idx;
	}

	// setup our resolve chance flag
	//SetupResolveChance(data, record);

	// normalize the eye angles, doesn't really matter but its clean.
	math::NormalizeAngle(record->m_eye_angles.y);
}

void Resolver::ResolveAir(AimPlayer* data, LagRecord* record) {

	data->m_updated = false;
	data->m_stand_index = 0;

	// if this is true it means they did not choke on land
	if (record->m_player->m_PlayerAnimState()->m_bLanding)
	{
		record->m_eye_angles.y = record->m_body;
		record->m_mode = Modes::RESOLVE_LBY;
		record->m_resolver_mode = "A:LBYUPDATE";
		return;
	}

	if (data->m_records.size() >= 2) {
		LagRecord* previous = data->m_records[1].get();
		const float lby_delta = math::AngleDiff(record->m_body, previous->m_body);

		const bool prev_ground = (previous->m_flags & FL_ONGROUND);
		const bool curr_ground = (record->m_flags & FL_ONGROUND);

		if (std::fabs(lby_delta) > 12.5f
			&& !previous->m_dormant
			&& data->m_body_idx <= 0
			&& prev_ground != curr_ground) {
			record->m_eye_angles.y = record->m_body;
			record->m_mode = Modes::RESOLVE_LBY;
			record->m_resolver_mode = "A:LBYCHANGE";
			return;
		}
	}

	// kys this is so dumb
	const float back = math::CalcAngle(g_cl.m_shoot_pos, record->m_pred_origin).y;
	const vec3_t delta = record->m_origin - data->m_walk_record.m_origin;
	const float back_lby_delta = math::AngleDiff(back, record->m_body);
	const bool avoid_lastmove = delta.length() >= 128.f;

	// try to predict the direction of the player based on his velocity direction.
	// this should be a rough estimation of where he is looking.
	const float velyaw = math::rad_to_deg(std::atan2(record->m_velocity.y, record->m_velocity.x));

	// gay
	const bool high_lm_delta = std::abs(math::AngleDiff(record->m_body, data->m_walk_record.m_body)) > 90.f;
	const float back_lm_delta = data->m_walk_record.m_sim_time > 0.f ? math::AngleDiff(back, data->m_walk_record.m_body) : FLT_MAX;

	switch (data->m_air_idx % 3) {
	case 0:
		if (((avoid_lastmove || high_lm_delta)
			&& std::fabs(record->m_sim_time - data->m_walk_record.m_sim_time) > 1.5f)
			|| data->m_walk_record.m_sim_time <= 0.f) {

			// angle too low to overlap with
			if (std::fabs(back_lby_delta) <= 22.5f || std::abs(back_lm_delta) <= 22.5f) {
				record->m_eye_angles.y = back;
				record->m_resolver_mode = "A:BACK";
			}
			else {

				// angle high enough to do some overlappings.
				if (std::fabs(back_lby_delta) <= 60.f || std::abs(back_lm_delta) <= 60.f) {

					const float overlap = std::abs(back_lm_delta) <= 60.f ? (std::abs(back_lm_delta) / 2.f) : (std::abs(back_lby_delta) / 2.f);

					if (back_lby_delta < 0.f) {
						record->m_eye_angles.y = back - overlap;
						record->m_resolver_mode = "A:OVERLAP-LEFT";
					}
					else {
						record->m_eye_angles.y = back + overlap;
						record->m_resolver_mode = "A:OVERLAP-RIGHT";
					}
				}
				else {

					if (std::abs(math::AngleDiff(data->m_walk_record.m_body, velyaw + 180.f)) <= 22.5f) {
						record->m_eye_angles.y = data->m_walk_record.m_body;
						record->m_resolver_mode = "A:TEST-LBY";
					}
					else {
						record->m_eye_angles.y = record->m_body;
						record->m_resolver_mode = "A:LBY";
					}
				}
			}
		}
		else {


			if (data->m_walk_record.m_sim_time > 0.f) {
				record->m_eye_angles.y = data->m_walk_record.m_body;
				record->m_resolver_mode = "A:LAST";

			}
			else {
				record->m_eye_angles.y = back;
				record->m_resolver_mode = "A:FALLBACK";
			}
		}
		break;
	case 1:
		record->m_eye_angles.y = back;
		record->m_resolver_mode = "A:BACK-BRUTE";
		break;
	case 2:
		record->m_eye_angles.y = record->m_body;
		record->m_resolver_mode = "A:LBY-BRUTE";
		break;
	}
}

void Resolver::ResolveWalk(AimPlayer* data, LagRecord* record) {
	// apply lby to eyeangles.
	record->m_eye_angles.y = record->m_body;

	if (record->m_velocity.length_2d() > 70.f)
		data->m_body_index = data->m_old_stand_move_idx = data->m_old_stand_no_move_idx = data->m_stand_move_idx = data->m_stand_no_move_idx = 0;

	data->m_body_updated_idk = 0;
	data->m_update_captured = 0;
	data->m_has_updated = 0;
	data->m_last_body = FLT_MIN;
	data->m_overlap_offset = 0.f;
	data->m_updated = false;
	data->m_triggered_late = false;
	data->m_stand_index = 0;

	if (data->m_update_count < 3) {
		data->m_update_count = 0;
		data->m_upd_time = FLT_MIN;
	}

	// copy the last record that this player was walking
	// we need it later on because it gives us crucial data.
	data->m_move_data.m_body = record->m_body;
	data->m_move_data.m_origin = record->m_origin;
	data->m_move_data.m_time = record->m_anim_time;
	data->m_move_data.m_sim_time = record->m_sim_time;
	data->m_move_data.m_server_tick = record->m_tick;

	record->m_resolver_mode = "WALK";
}

int Resolver::GetNearestEntity(Player* target, LagRecord* record) {
	// best data
	int idx = g_csgo.m_engine->GetLocalPlayer();
	float best_distance = g_cl.m_processing ? g_cl.m_local->m_vecOrigin().dist_to(record->m_pred_origin) : FLT_MAX;

	// cur data
	Player* curr_player = nullptr;
	vec3_t  curr_origin{};
	float   curr_dist = 0.f;
	AimPlayer* data = nullptr;

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		curr_player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!curr_player
			|| !curr_player->IsPlayer()
			|| curr_player->index() > 64
			|| curr_player->index() <= 0
			|| !curr_player->enemy(target)
			|| curr_player->dormant()
			|| !curr_player->alive()
			|| curr_player == target)
			continue;

		curr_origin = curr_player->m_vecOrigin();
		curr_dist = (record->m_pred_origin).dist_to(curr_origin);

		if (curr_dist < best_distance) {
			idx = i;
			best_distance = curr_dist;
		}
	}

	return idx;
}

float GetRotationYaw(LagRecord* pRecord, float flYaw)
{
	return math::NormalizedAngle(math::CalcAngle(pRecord->m_origin, g_cl.m_shoot_pos).y - flYaw);
}

void Resolver::ResolveStand(AimPlayer* data, LagRecord* record) {

	const float away = GetAwayAngle(record);
	vec3_t enemy_shootpos = record->m_player->m_vecOrigin() + record->m_player->m_vecViewOffset();

	data->m_moved = false;

	LagRecord* previous = data->m_records.size() >= 2 ? data->m_records[1].get() : nullptr;

	// check if it's been atleast 3 ticks since they started moving
	if (data->m_move_data.m_sim_time > 0.f) {

		float delta = data->m_move_data.m_origin.dist_to(record->m_origin);

		// check if moving record is close.
		if (delta <= 45.f)
			data->m_moved = true;
	}


	float time_since_moving = data->m_moved ? data->m_move_data.m_time - record->m_anim_time : FLT_MAX;

	if (time_since_moving < 0.22f && data->m_body_index <= 0) {
		record->m_eye_angles.y = record->m_body;
		record->m_mode = Modes::RESOLVE_LBY;
		return;
	}

	float anti_freestand = AntiFreestand(record->m_player, record, enemy_shootpos, g_cl.m_shoot_pos, true, away + 180.f, 70.f);
	float anti_freestand_2 = AntiFreestand(record->m_player, record, enemy_shootpos, g_cl.m_shoot_pos, true, away, 70.f);
	float last_fs_angle = FLT_MAX;
	float body_diff = data->m_moved ? std::abs(math::AngleDiff(record->m_body, data->m_move_data.m_body)) : FLT_MAX;
	float back_diff = data->m_moved ? std::abs(math::AngleDiff(away + 180.f, data->m_move_data.m_body)) : FLT_MAX;

	if (record->m_mode != Modes::RESOLVE_LBY_PRED)
	{
		// lol
		if (body_diff <= 5.f)
		{
			record->m_eye_angles.y = data->m_move_data.m_body;
			record->m_mode = Modes::RESOLVE_LBY;
			return;
		}

		record->m_mode = Modes::RESOLVE_STAND;

		// reset it.
		if (data->m_stand_index > 4)
			data->m_stand_index = 0;

		switch (data->m_stand_index)
		{
		case 0:
		{
			if (!data->m_moved)
			{
				record->m_eye_angles.y = away + 180.f;
				return;
			}

			if (back_diff <= 10.f) {
				record->m_eye_angles.y = away + 180.f;
				return;
			}

			record->m_eye_angles.y = last_fs_angle = anti_freestand;
		}
		break;
		case 1:
		{
			if (!data->m_moved)
			{
				record->m_eye_angles.y = last_fs_angle = anti_freestand;
				return;
			}

			if (last_fs_angle != FLT_MAX)
			{
				record->m_eye_angles.y = last_fs_angle - 70.f;
				return;
			}

			record->m_eye_angles.y = last_fs_angle = anti_freestand_2;
		}
		break;
		case 2:
		{
			if (!data->m_moved)
			{
				record->m_eye_angles.y = last_fs_angle - 70.f;
				return;
			}

			if (last_fs_angle != FLT_MAX)
			{
				record->m_eye_angles.y = last_fs_angle + 70.f;
				return;
			}

			record->m_eye_angles.y = record->m_body;
		}
		break;
		case 3:
		{
			if (!data->m_moved)
			{
				record->m_eye_angles.y = away;
				return;
			}

			record->m_eye_angles.y = record->m_body + body_diff;
		}
		break;
		default:
			break;
		}
	}
}

void Resolver::ResolveOverride(AimPlayer* data, LagRecord* record, Player* player)
{
	// get predicted away angle for the player.
	float away = GetAwayAngle(record);

	C_AnimationLayer* curr = &record->m_layers[3];
	int act = data->m_player->GetSequenceActivity(curr->sequence);
	float choosen_angle = 0.0f;

	if (g_input.GetKeyState(g_menu.main.aimbot.override.get()))
	{
		ang_t                          viewangles;
		g_csgo.m_engine->GetViewAngles(viewangles);

		//auto yaw = math::clamp (g_cl.m_local->GetAbsOrigin(), Player->origin()).y;
		const float at_target_yaw = math::CalcAngle(g_cl.m_local->m_vecOrigin(), player->m_vecOrigin()).y;
		const float dist = math::NormalizedAngle(viewangles.y - at_target_yaw);

		if (std::abs(dist) <= 1.f) {
			choosen_angle = at_target_yaw;
		}
		else if (dist > 0) {
			choosen_angle = at_target_yaw + 90.f;
		}
		else {
			choosen_angle = at_target_yaw - 90.f;
		}

		data->m_override_angle = choosen_angle;
	}

	data->m_override = true;
	record->m_eye_angles.y = data->m_override_angle;
}

void InvalidatePhysicsRecursive(void* player, int change_flags) {
	static auto CBaseEntity_InvalidatePhysicsRecursive = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56")).as< void(__thiscall*)(void*, int) >();
	CBaseEntity_InvalidatePhysicsRecursive(player, change_flags);
}

void SetWeight(CCSGOGamePlayerAnimState* state, int layer_idx, float weight) {
	auto layer = state->m_pPlayer->m_AnimOverlay() + layer_idx;

	if (layer->owner && layer->weight != weight) {
		if (weight == 0.0f) {
			InvalidatePhysicsRecursive(state->m_pPlayer, 10);
		}
	}

	layer->weight = weight;
}

void SetCycle(CCSGOGamePlayerAnimState* state, int layer_idx, float cycle) {
	auto layer = state->m_pPlayer->m_AnimOverlay() + layer_idx;

	auto clamp_cycle = [](float in) {
		in -= int(in);

		if (in < 0.0f)
			in += 1.0f;
		else if (in > 1.0f)
			in -= 1.0f;

		return in;
		};

	const auto clamped_cycle = clamp_cycle(cycle);

	if (layer->owner && layer->cycle != clamped_cycle) {
		InvalidatePhysicsRecursive(state->m_pPlayer, 8);
	}

	layer->cycle = clamped_cycle;
}

void SetSequence(CCSGOGamePlayerAnimState* state, int layer_idx, int sequence) {
	const auto player = state->m_pPlayer;

	if (!player || !state)
		return;

	static auto CCSGOPlayerAnimState_UpdateLayerOrderPreset = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 51 53 56 57 8B F9 83 7F 60 00 0F 84 ? ? ? ? 83")).as< void(__thiscall*)(CCSGOGamePlayerAnimState*, int, int) >();

	if (sequence > 1) {
		g_csgo.m_model_cache->BeginLock();

		const auto layer = player->m_AnimOverlay() + layer_idx;

		if (!layer)
			return;

		if (layer->owner && layer->sequence != sequence)
			InvalidatePhysicsRecursive(state->m_pPlayer, 16);

		layer->sequence = sequence;
		layer->playback_rate = player->GetSequenceCycleRate(sequence);

		SetCycle(state, layer_idx, 0);
		SetWeight(state, layer_idx, 0);

		CCSGOPlayerAnimState_UpdateLayerOrderPreset(state, layer_idx, sequence);

		g_csgo.m_model_cache->EndLock();
	}
}

void Resolver::ResolvePoses(Player* player, LagRecord* record) {

	auto anim_state = player->m_PlayerAnimState();

	if (!anim_state)
		return;

	auto state = (CCSGOGamePlayerAnimState*)anim_state;

	if (!state)
		return;

}
