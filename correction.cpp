#include "includes.h"

Correction g_correction;

void Correction::FindBestAngle(Player* player, LagRecord* record)
{
	auto data = this->data[player->index()];

	// constants.
	constexpr float STEP{ 4.f };
	constexpr float RANGE{ 28.f };

	// get the away angle for this record.
	float away = GetAwayAngle(record);

	vec3_t enemy_eyepos = player->GetShootPosition();

	// construct vector of angles to test.
	std::vector< AdaptiveAngle > angles{ };
	angles.emplace_back(away + 90.f);
	angles.emplace_back(away - 90.f);

	// start the trace at the enemy shoot pos.
	vec3_t start = g_cl.m_local->GetShootPosition();

	// see if we got any valid result.
	// if this is false the path was not obstructed with anything.
	bool valid{ false };

	// iterate vector of angles.
	for (auto it = angles.begin(); it != angles.end(); ++it) {

		// compute the 'rough' estimation of where our head will be.
		vec3_t end{ enemy_eyepos.x + std::cos(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemy_eyepos.y + std::sin(math::deg_to_rad(it->m_yaw)) * RANGE,
			enemy_eyepos.z };

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
				mult = 1.5f;

			// over 90% of the total length, prioritize this shit.
			if (i > (len * 0.9f))
				mult = 2.f;

			// append 'penetrated distance'.
			it->m_dist += (STEP * mult);

			// mark that we found anything.
			valid = true;
		}
	}

	if (!valid) {
		data.m_freestand_data = false;
		data.m_anti_fs_angle = away + 180.f;
		return;
	}

	// put the most distance at the front of the container.
	std::sort(angles.begin(), angles.end(),
		[](const AdaptiveAngle& a, const AdaptiveAngle& b) {
			return a.m_dist > b.m_dist;
		});

	// the best angle should be at the front now.
	AdaptiveAngle* best = &angles.front();

	data.m_anti_fs_angle = math::NormalizedAngle(best->m_yaw);
	data.m_freestand_data = true;
}

float Correction::AntiFreestand(Player* player, LagRecord* record, vec3_t start_, vec3_t end, bool include_base, float base_yaw, float delta)
{
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

float Correction::GetAwayAngle(LagRecord* record)
{
	float  delta{ std::numeric_limits< float >::max() };
	vec3_t pos;
	ang_t  away;

	math::VectorAngles(g_cl.m_local->m_vecOrigin() - record->m_pred_origin, away);
	return away.y;
}

void Correction::CorrectPitch(Player* player, LagRecord* record)
{
	auto& data = this->data[player->index()];

}

void Correction::CorrectStand(Player* player, LagRecord* record)
{
	int idx = g_resolver.GetNearestEntity(record->m_player, record);

	Player* nearest_entity = (Player*)g_csgo.m_entlist->GetClientEntity(idx);

	if (!nearest_entity)
		return;

	auto& data = this->data[player->index()];
	auto aim_data = &g_aimbot.m_players[player->index() - 1];

	// reset it
	data.m_has_moved = false;

	// check if we have a last move to use
	if (data.move_data.m_sim_time > 0.f)
		data.m_has_moved = true;

	// make sure it's somewhat safe to use..
	float distance = data.m_has_moved ? (data.move_data.m_origin - record->m_origin).length() : FLT_MAX;
	float old_difference = math::AngleDiff(data.m_old_body, record->m_body);
	float body_yaw_difference = data.m_body_yaw - data.m_old_body_yaw;
	LagRecord* previous = aim_data->m_records.size() >= 2 ? aim_data->m_records[1].get() : nullptr;
	float away = GetAwayAngle(record);
	float last_unmoved_fs = FLT_MAX;
	const vec3_t current_origin = record->m_origin + record->m_player->m_vecViewOffset();
	const vec3_t nearest_origin = nearest_entity->m_vecOrigin() + nearest_entity->m_vecViewOffset();

	data.m_body_before_leaving_ground = record->m_body;

	// safe to use
	if (distance <= 20.f)
		data.m_moved = true;

	// lby hasn't updated
	if (!data.m_has_updated)
	{
		// update angles
		record->m_eye_angles.y = record->m_body;

		// set up mode
		data.m_mode = RModes::STAND_LBY;

		// exit out
		return;
	}

	bool is_sideways = IsYawSideways(player, data.move_data.m_body);
	bool is_backwards = !is_sideways;

	const float back = away + 180.f;
	const float forward_body_diff = math::AngleDiff(away, record->m_body);

	// reset overlap delta amount
	data.m_overlap_offset = 0.f;

	const float back_delta = math::AngleDiff(record->m_body, back);

	if (std::fabs(back_delta) >= 25.f) {
		if (back_delta < 0.f)
			data.m_overlap_offset = std::clamp(-(std::fabs(back_delta) / 2.f), -60.f, 60.f);
		else
			data.m_overlap_offset = std::clamp((std::fabs(back_delta) / 2.f), -60.f, 60.f);
	}

	// no lastmove to use
	if (!data.m_moved)
	{
		// set mode
		data.m_mode = RModes::STAND_UNMOVED;

		// print to debug

		switch (data.m_unmoved_misses)
		{
		case 0:
			// record->m_eye_angles.y = data->freestand_data ? data->m_anti_fs_angle : away + 180.f;
			// if angle invalid we still set it to backward so :shrug:
			// ^ means if no fs data angle is applied to backward -> no need to check if data valid or not
			record->m_eye_angles.y = away + 180.f;
			break;
		case 1:
			// record->m_eye_angles.y = away + 180.f;
			record->m_eye_angles.y = data.m_anti_fs_angle;
			break;
		case 2:
			// record->m_eye_angles.y = away;
			record->m_eye_angles.y = data.m_anti_fs_angle - 90.f;
			break;
		case 3:
			// lol this average between 90 and 45
			record->m_eye_angles.y = record->m_body - 67.f;
			break;
		case 4:
			record->m_eye_angles.y = record->m_body + 67.f;
			break;
		case 5:
			record->m_eye_angles.y = data.m_anti_fs_angle + 180.f;
			break;
		case 6:
			record->m_eye_angles.y = away;
			break;
		default:
			break;
		}

		// exit out
		return;
	}
	
	// we have lastmove to use
	float move_difference = std::abs(record->m_body - data.move_data.m_body);
	float lby_delta = data.m_pre_update_body - data.move_data.m_body;
	float back_overlap = back - record->m_body;
	float back_diff = math::AngleDiff(back, data.m_pre_update_body);
	float last_fs_angle = FLT_MAX;

	if (data.m_reversefs_index < 1 && is_sideways && data.m_freestand_data) {
		data.m_mode = RModes::STAND_FREESTAND;
		record->m_eye_angles.y = data.m_anti_fs_angle;
		record->m_resolver_mode = "RFS";
	}
	else if (data.m_lastmove_index < 1 && move_difference < 15.f) {
		data.m_mode = RModes::STAND_LASTMOVE;
		record->m_eye_angles.y = data.move_data.m_body;
		record->m_resolver_mode = "LM";
	}
	else if (data.m_back_index < 1 && (is_backwards || !data.m_freestand_data)) {
		data.m_mode = RModes::STAND_BACK;
		record->m_resolver_mode = "B";
		record->m_eye_angles.y = away + 180.0f;
	}
	else if (data.m_lastmove_index >= 1 || data.m_back_index >= 1 && is_backwards && data.m_stand_index2 < 3)
	{
		//set resolve mode
		data.m_mode = RModes::STAND_1;
		record->m_resolver_mode = "ST2";
		switch (data.m_stand_index2 % 3) {
		case 0:
			record->m_eye_angles.y = away + 135.f;
			break;
		case 1:
			record->m_eye_angles.y = away + 225.f;
			break;
		case 2:
			record->m_eye_angles.y = away;
			break;
		}
	}
	else if (data.m_reversefs_index >= 1 && is_sideways && data.m_stand_index1 < 8)
	{
		data.m_mode = RModes::STAND_2;
		record->m_resolver_mode = "ST1";
		switch (data.m_stand_index1 % 8) {
		case 0:
			record->m_eye_angles.y = away + 180.f;
			break;
		case 1:
			record->m_eye_angles.y = away - 135.f;
			break;
		case 2:
			record->m_eye_angles.y = away + 225.f;
			break;
		case 3:
			record->m_eye_angles.y = data.m_anti_fs_angle + 180.f;
			break;
		case 4:
			record->m_eye_angles.y = away + 90.f;
			break;
		case 5:
			record->m_eye_angles.y = away - 90.f;
			break;
		case 6:
			record->m_eye_angles.y = away + 110.f;
			break;
		case 7:
			record->m_eye_angles.y = away - 110.f;
			break;
		default:
			break;
		}
	}
}

void Correction::CorrectMove(Player* player, LagRecord* record)
{
	auto& data = this->data[player->index()];

	// set these up
	record->m_eye_angles.y = record->m_body;

	// set up timer
	data.m_body_timer = record->m_anim_time + 0.22f;

	// reset these
	data.air_misses = 0;
	data.unknown_misses = 0;
	data.m_moved = false;
	data.m_has_updated = false;

	// save the move record
	data.move_data.m_sim_time = record->m_sim_time;
	data.move_data.m_anim_time = record->m_anim_time;
	data.move_data.m_body = record->m_body;
	data.move_data.m_origin = record->m_origin;
}

void Correction::CorrectAir(Player* player, LagRecord* record)
{
	auto& data = this->data[player->index()];
	auto anim_state = player->m_PlayerAnimState();

	// reset it
	data.m_moved = false;
	data.m_body_timer = FLT_MAX;
	data.m_has_updated = true;

	// set the mode
	data.m_mode = RModes::AIR_;

	// if this is true it means the enemy isn't fakelagging on air
	if (anim_state->m_bLanding)
	{
		// set eye angles to last received lby
		record->m_eye_angles.y = record->m_body;

		// update it
		data.m_body_before_leaving_ground = record->m_body;

		// exit out
		return;
	}

	float overlap_amount = 0.0f;
	float away = GetAwayAngle(record);

	// if we're here the difference of their current lby to the last they were on ground is some what high
	switch (data.air_misses)
	{
	case 0:
	{
		// get the overlap amount backwards
		overlap_amount = math::AngleDiff(away + 180.f, record->m_body);

		// try it
		record->m_eye_angles.y = record->m_body + overlap_amount;
	}
	break;
	case 1:
	{
		// let's try forcing lby
		record->m_eye_angles.y = record->m_body;
	}
	break;
	case 2:
	{
		// ok nigga is probably randomizing his angles or something
		record->m_eye_angles.y = record->m_body - 90.f;
	}
	break;
	case 3:
	{
		// ok nigga is probably randomizing his angles or something
		record->m_eye_angles.y = record->m_body + 90.f;
	}
	break;
	default:
		break;
	}
}

void Correction::PredictUpdates(Player* player, LagRecord* record)
{
	auto& data = this->data[player->index()];
	float delta = FLT_MAX;

	// only do it on stand
	if (record->m_mode != RRModes::R_STAND) return;

	// reset
	data.m_trigger_timer = false;

	// set up the timer even if lby didn't change
	data.m_body_timer = record->m_anim_time + 1.1f;

	// check if lby changed
	if (record->m_body != data.m_old_body)
	{
		// update it
		data.m_old_body = record->m_body;

		// lby changed
		data.m_has_updated = true;

		// mark as updated
		data.m_trigger_timer = true;
	}

	// check if we have moved so we can use the 0.22 timer
	if (data.move_data.m_anim_time > 0.f)
	{
		// save delta
		delta = record->m_anim_time - data.move_data.m_anim_time;

		if (delta < 0.22f)
			data.m_pre_update_body = record->m_body;

		// check if it's less than 0.22 seconds
		if (delta <= 0.22f)
		{
			// update eye angles
			record->m_eye_angles.y = data.move_data.m_body;

			// set up mode
			data.m_mode = RModes::STAND_LBY_PRE_UPDATE;

			// exit out
			return;
		}
	}

	// check if it's time to update the lby
	if (data.m_body_timer >= record->m_anim_time && data.m_trigger_timer)
	{
		// update eye angles
		record->m_eye_angles.y = record->m_body;

		// make sure to update the timer again
		data.m_body_timer = record->m_anim_time + 1.1f;

		// set up mode
		data.m_mode = RModes::STAND_LBY_UPDATE;

		// exit out
		return;
	}
}

void Correction::Run(Player* player, LagRecord* record)
{
	auto& data = this->data[player->index()];

	const bool OnGround = record->m_flags & FL_ONGROUND;
	const bool Standing = record->m_anim_velocity.length() <= 0.1f || record->m_fake_walk;
	const bool Moving = record->m_anim_velocity.length() > 0.1f && !record->m_fake_walk;

	// set mode.
	{
		if (OnGround)
			record->m_mode = Standing ? RRModes::R_STAND : RRModes::R_MOVE;
		else
			record->m_mode = RRModes::R_AIR;
	}

	// fix pitch on shot
	CorrectPitch(player, record);

	// run antifreestand
	FindBestAngle(player, record);

	// resolve
	PredictUpdates(player, record);

	// resolve
	switch (record->m_mode)
	{
	case RRModes::R_STAND:
		CorrectStand(player, record);
		break;
	case RRModes::R_MOVE:
		CorrectMove(player, record);
		break;
	case RRModes::R_AIR:
		CorrectAir(player, record);
		break;
	}

	// normalize the eye angles, doesn't really matter but its clean.
	math::NormalizeAngle(record->m_eye_angles.y);
}

bool Correction::IsYawSideways(Player* entity, float yaw)
{
	auto local_player = g_cl.m_local;
	if (!local_player)
		return false;

	const auto at_target_yaw = math::CalcAngle(local_player->m_vecOrigin(), entity->m_vecOrigin()).y;
	const float delta = fabs(math::AngleDiff(at_target_yaw, yaw));

	return delta > 35.f && delta < 145.f;
}
