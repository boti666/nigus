#include "includes.h"

C_AnimationSync g_anims{ };;

VelocityDetail_t C_AnimationSync::UpdateVelocity(C_AnimationLayer* animlayers, LagRecord* previous, Player* player)
{
	if (!animlayers)
		return DETAIL_NONE;

	const auto& jump_or_fall_layer = animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL];
	const auto& move_layer = animlayers[ANIMATION_LAYER_MOVEMENT_MOVE];
	const auto& land_or_climb_layer = animlayers[ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB];
	const auto& alive_loop_layer = animlayers[ANIMATION_LAYER_ALIVELOOP];

	const bool on_ground = (player->m_fFlags() & FL_ONGROUND) != 0;
	vec3_t& velocity = player->m_vecVelocity();

	/* standing still on ground */
	if (on_ground && move_layer.weight == 0.0f) {
		//csgo::csgo.engine_client->client_cmd_unrestricted ( fmt::format ( "echo pbr: {:.4f}\n", move_layer.playback_rate * 100.0f ).c_str ( ) );
		velocity.zero();
		return DETAIL_ZERO;
	}

	const float dt = std::max(g_csgo.m_globals->m_interval, player->m_flSimulationTime() - previous->m_sim_time);
	vec3_t avg_velocity = (player->m_vecOrigin() - previous->m_origin) / dt;

	/* if we just started moving, we can probably use this velocity (they probably havent had time to switch directions) */
	/* not choking. an average will be pretty close */
	if (previous->m_velocity.length_2d() <= 0.1f || dt <= g_csgo.m_globals->m_interval) {
		velocity = avg_velocity;
		return DETAIL_PERFECT;
	}

	const auto* weapon_info = player->GetActiveWeapon() ? player->GetActiveWeapon()->GetWpnData() : nullptr;
	float max_weapon_speed = weapon_info ? (player->m_bIsScoped() ? weapon_info->m_max_player_speed : weapon_info->m_max_player_speed_alt) : 250.0f;

	if (on_ground) {
		avg_velocity.z = 0.0f;
		velocity = avg_velocity;

		VelocityDetail_t detail = DETAIL_NONE;

		/* SKEET VELOCITY FIX */
		if (move_layer.playback_rate > 0.0f) {
			vec3_t direction = velocity.normalized();
			direction.z = 0.0f;

			const float avg_speed_xy = velocity.length_2d();
			const float move_weight_with_air_smooth = move_layer.weight;
			const float target_move_weight_to_speed_xy = max_weapon_speed * math::Lerp(CS_PLAYER_SPEED_WALK_MODIFIER, CS_PLAYER_SPEED_DUCK_MODIFIER, player->m_flDuckAmount()) * move_weight_with_air_smooth;
			const float speed_as_portion_of_run_top_speed = 0.35f * (1.0f - alive_loop_layer.weight);

			if (alive_loop_layer.weight > 0.0f && alive_loop_layer.weight < 1.0f) {
				const float speed_xy = max_weapon_speed * (speed_as_portion_of_run_top_speed + 0.55f);
				velocity = direction * speed_xy;
				detail = DETAIL_RUNNING;
			}
			else if (move_weight_with_air_smooth < 0.95f || target_move_weight_to_speed_xy > avg_speed_xy) {
				velocity = direction * target_move_weight_to_speed_xy;

				const float prev_move_weight = previous->m_layers[ANIMATION_LAYER_MOVEMENT_MOVE].weight;
				const float weight_delta_rate = (move_layer.weight - prev_move_weight) / dt;
				const bool walking_speed = velocity.length_2d() > max_weapon_speed * CS_PLAYER_SPEED_WALK_MODIFIER;
				const bool constant_speed = abs(weight_delta_rate) < (walking_speed ? 0.9f : 0.15f);
				const bool accelerating = weight_delta_rate > 1.0f;

				///* move weight hasn't changed. we are confident there speed is correct/constant */
				//if ( constant_speed )
				//	detail = lagcomp::VelocityDetail::Constant;
				///* player is accelerating */
				//else if ( accelerating )
				//	detail = lagcomp::VelocityDetail::Accelerating;

				/* move weight hasn't changed. we are confident there speed is correct/constant */
				if (move_layer.weight == prev_move_weight)
					detail = DETAIL_CONSTANT;
				/* player is accelerating */
				else if (move_layer.weight > prev_move_weight)
					detail = DETAIL_ACCELERATING;
			}
			else {
				//float target_move_weight_adjusted_speed_xy = std::min ( max_weapon_speed, deployable_limit_max_speed(player) );
				float target_move_weight_adjusted_speed_xy = max_weapon_speed * move_weight_with_air_smooth;

				if ((player->m_fFlags() & FL_DUCKING) != 0)
					target_move_weight_adjusted_speed_xy *= CS_PLAYER_SPEED_DUCK_MODIFIER;
				else if (player->m_bIsWalking())
					target_move_weight_adjusted_speed_xy *= CS_PLAYER_SPEED_WALK_MODIFIER;

				const float prev_move_weight = previous->m_layers[ANIMATION_LAYER_MOVEMENT_MOVE].weight;

				if (avg_speed_xy > target_move_weight_adjusted_speed_xy) {
					velocity = direction * target_move_weight_adjusted_speed_xy;

					const float weight_delta_rate = (move_layer.weight - prev_move_weight) / dt;
					const bool walking_speed = velocity.length_2d() > max_weapon_speed * CS_PLAYER_SPEED_WALK_MODIFIER;
					const bool constant_speed = abs(weight_delta_rate) < (walking_speed ? 0.9f : 0.15f);
					const bool accelerating = weight_delta_rate > 1.0f;

					///* move weight hasn't changed. we are confident there speed is correct/constant */
					//if ( constant_speed )
					//	detail = lagcomp::VelocityDetail::Constant;
					///* player is accelerating */
					//else if ( accelerating )
					//	detail = lagcomp::VelocityDetail::Accelerating;
				}

				/* move weight hasn't changed. we are confident there speed is correct/constant */
				if (move_layer.weight == prev_move_weight)
					detail = DETAIL_CONSTANT;
				/* player is accelerating */
				else if (move_layer.weight > prev_move_weight)
					detail = DETAIL_ACCELERATING;
			}
		}

		return detail;
	}
	else {
		int seq = -1;

		const bool crouch = player->m_flDuckAmount() > 0.0f;
		const float speed_as_portion_of_walk_top_speed = avg_velocity.length_2d() / (max_weapon_speed * CS_PLAYER_SPEED_WALK_MODIFIER);
		const bool moving = speed_as_portion_of_walk_top_speed > 0.25f;

		seq = moving + 17;
		if (!crouch)
			seq = moving + 15;

		velocity = avg_velocity;

		if (jump_or_fall_layer.weight > 0.0f
			&& jump_or_fall_layer.playback_rate > 0.0f
			&& jump_or_fall_layer.sequence == seq) {
			const float time_since_jump = jump_or_fall_layer.cycle * jump_or_fall_layer.playback_rate;
			velocity.z = g_csgo.sv_jump_impulse->GetFloat() - time_since_jump * g_csgo.sv_gravity->GetFloat() * 0.5f;
		}

		return DETAIL_PERFECT;
	}

	player->m_vecVelocity() = velocity;

	return DETAIL_NONE;
}

void FixVelocity(Player* m_player, LagRecord* record, LagRecord* previous, float max_speed) {
	if (!previous) {
		if (record->m_layers[6].playback_rate > 0.0f && record->m_layers[6].weight != 0.f && record->m_velocity.length() > 0.1f) {
			auto v30 = max_speed;

			if (record->m_flags & 6)
				v30 *= 0.34f;
			else if (m_player->m_bIsWalking())
				v30 *= 0.52f;

			auto v35 = record->m_layers[6].weight * v30;
			record->m_velocity *= v35 / record->m_velocity.length();
		}
		else
			record->m_velocity.clear();

		if (record->m_flags & 1)
			record->m_velocity.z = 0.f;

		record->m_anim_velocity = record->m_velocity;
		return;
	}

	if ((m_player->m_fEffects() & 8) != 0
		|| m_player->m_ubEFNoInterpParity() != m_player->m_ubEFNoInterpParityOld()) {
		record->m_velocity.clear();
		record->m_anim_velocity.clear();
		return;
	}

	auto is_jumping = !(record->m_flags & FL_ONGROUND && previous->m_flags & FL_ONGROUND);

	if (record->m_lag > 1) {
		record->m_velocity.clear();
		auto origin_delta = (record->m_origin - previous->m_origin);

		if (!(previous->m_flags & FL_ONGROUND || record->m_flags & FL_ONGROUND))// if not previous on ground or on ground
		{
			auto currently_ducking = record->m_flags & FL_DUCKING;
			if ((previous->m_flags & FL_DUCKING) != currently_ducking) {
				float duck_modifier = 0.f;

				if (currently_ducking)
					duck_modifier = 9.f;
				else
					duck_modifier = -9.f;

				origin_delta.z -= duck_modifier;
			}
		}

		auto sqrt_delta = origin_delta.length_2d_sqr();

		if (sqrt_delta > 0.f && sqrt_delta < 1000000.f)
			record->m_velocity = origin_delta / game::TICKS_TO_TIME(record->m_lag);

		record->m_velocity.validate_vec();

		if (is_jumping) {
			if (record->m_flags & FL_ONGROUND && !g_csgo.sv_enablebunnyhopping->GetInt()) {

				// 260 x 1.1 = 286 units/s.
				float max = m_player->m_flMaxspeed() * 1.1f;

				// get current velocity.
				float speed = record->m_velocity.length();

				// reset velocity to 286 units/s.
				if (max > 0.f && speed > max)
					record->m_velocity *= (max / speed);
			}

			// assume the player is bunnyhopping here so set the upwards impulse.
			record->m_velocity.z = g_csgo.sv_jump_impulse->GetFloat();
		}
		// we are not on the ground
		// apply gravity and airaccel.
		else if (!(record->m_flags & FL_ONGROUND)) {
			// apply one tick of gravity.
			record->m_velocity.z -= g_csgo.sv_gravity->GetFloat() * g_csgo.m_globals->m_interval;
		}
	}

	record->m_anim_velocity = record->m_velocity;

	record->m_fake_walk = record->m_layers[6].weight == 0.f && record->m_anim_velocity.length_2d() > 0.1f;

	if (!record->m_fake_walk) {
		if (record->m_anim_velocity.length_2d() > 0 && (record->m_flags & FL_ONGROUND)) {
			float anim_speed = 0.f;

			if (!is_jumping
				&& record->m_layers[11].weight > 0.0f
				&& record->m_layers[11].weight < 1.0f
				&& record->m_layers[11].playback_rate == previous->m_layers[11].playback_rate) {
				// calculate animation speed yielded by anim overlays
				auto flAnimModifier = 0.35f * (1.0f - record->m_layers[11].weight);
				if (flAnimModifier > 0.0f && flAnimModifier < 1.0f)
					anim_speed = max_speed * (flAnimModifier + 0.55f);
			}

			// this velocity is valid ONLY IN ANIMFIX UPDATE TICK!!!
			// so don't store it to record as m_vecVelocity
			// -L3D451R7
			if (anim_speed > 0.0f) {
				anim_speed /= record->m_anim_velocity.length_2d();
				record->m_anim_velocity.x *= anim_speed;
				record->m_anim_velocity.y *= anim_speed;
			}
		}
	}
	else
		record->m_anim_velocity = record->m_velocity = { 0.f, 0.f, 0.f };

	record->m_anim_velocity.validate_vec();
}


void C_AnimationSync::Update(AimPlayer* entry,
	LagRecord* record,
	LagRecord* prev) {

	CCSGOPlayerAnimState* state = entry->m_player->m_PlayerAnimState();
	if (!state)
		return;

	// player respawned.
	if (entry->m_player->m_flSpawnTime() != entry->m_spawn) {
		// reset animation state.
		game::ResetAnimationState(state);

		// note new spawn time.
		entry->m_spawn = entry->m_player->m_flSpawnTime();
	}

	// backup curtime.
	const float m_flCurtime = g_csgo.m_globals->m_curtime;
	const float m_flFrametime = g_csgo.m_globals->m_frametime;

	// set curtime to sim time.
	// set frametime to ipt just like on the server during simulation.
	g_csgo.m_globals->m_curtime = record->m_anim_time;
	g_csgo.m_globals->m_frametime = g_csgo.m_globals->m_interval;

	// backup stuff that we do not want to fuck with.
	AnimationBackup_t backup;

	backup.m_origin = entry->m_player->m_vecOrigin();
	backup.m_abs_origin = entry->m_player->GetAbsOrigin();
	backup.m_velocity = entry->m_player->m_vecVelocity();
	backup.m_abs_velocity = entry->m_player->m_vecAbsVelocity();
	backup.m_flags = entry->m_player->m_fFlags();
	backup.m_eflags = entry->m_player->m_iEFlags();
	backup.m_duck = entry->m_player->m_flDuckAmount();
	backup.m_body = entry->m_player->m_flLowerBodyYawTarget();
	backup.m_abs_angle = entry->m_player->GetAbsAngles();
	entry->m_player->GetAnimLayers(backup.m_layers);

	// is player a bot?
	bool bot = game::IsFakePlayer(entry->m_player->index());

	// reset fakewalk state.
	record->m_mode = Resolver::Modes::RESOLVE_NONE;
	record->m_resolver_mode = XOR("NONE");
	record->m_anim_vel_type = 0;
	record->m_ground_for_two_ticks = record->m_flags & FL_ONGROUND; // set this incase we have no previous record to work with.
	record->m_sim_tick = record->m_shot = record->m_fixing_pitch = false;
	record->m_broke_lc = false;

	entry->m_player->SetAbsOrigin(record->m_origin);

	// should be a possible fix for people using fluc or dynamic idfk XD?
	if (prev)
	{
		const vec3_t previous_net_delta = prev->m_origin - prev->m_old_origin;

		if (previous_net_delta.length_sqr() > 4096.f)
			record->m_broke_lc = true;
	}

	const vec3_t net_delta = record->m_origin - record->m_old_origin;

	// check if lc broken.
	if (net_delta.length_sqr() > 4096.f)
		record->m_broke_lc = true;

	// get player weapon 
	Weapon* wpn = entry->m_player->GetActiveWeapon();

	// if player weapon is valid
	if (wpn) {

		// get data of the player's weapon
		WeaponInfo* wpn_data = wpn->GetWpnData();

		// get max velocity the player can reach
		if (wpn_data)
			record->m_max_speed = entry->m_player->m_bIsScoped() ? wpn_data->m_max_player_speed_alt : wpn_data->m_max_player_speed;
	}

	if (prev) {
		record->m_ground_for_two_ticks = (record->m_flags & FL_ONGROUND) && (prev->m_flags & FL_ONGROUND);
		record->m_cl_lag = game::TIME_TO_TICKS(record->m_sim_time - prev->m_sim_time);
	}

	// fix velocity.
	if (record->m_lag > 1)
		FixVelocity(entry->m_player, record, prev, record->max_speed);

	// fix various issues with the game
	// these issues can only occur when a player is choking data.
	if (record->m_lag > 1 && !bot) {
		// detect fakewalk.
		float speed = record->m_velocity.length();

		if (prev && !prev->dormant()) {

			// strip the on ground flag.
			entry->m_player->m_fFlags() &= ~FL_ONGROUND;

			// been onground for 2 consecutive ticks? fuck yeah.
			if (record->m_flags & FL_ONGROUND && prev->m_flags & FL_ONGROUND)
				entry->m_player->m_fFlags() |= FL_ONGROUND;

			// he's on ground
			if (state->m_bLanding)
				entry->m_player->m_fFlags() |= FL_ONGROUND;

			// fix jump_fall.
			if (record->m_layers[4].weight != 1.f && prev->m_layers[4].weight == 1.f && record->m_layers[5].weight != 0.f)
				entry->m_player->m_fFlags() |= FL_ONGROUND;

			if (record->m_flags & FL_ONGROUND && !(prev->m_flags & FL_ONGROUND))
				entry->m_player->m_fFlags() &= ~FL_ONGROUND;

			// delta in duckamt and delta in time..
			float duck = record->m_duck - prev->m_duck;
			float time = record->m_sim_time - prev->m_sim_time;

			// get the duckamt change per tick.
			float change = (duck / time) * g_csgo.m_globals->m_interval;

			// fix crouching players.
			entry->m_player->m_flDuckAmount() = prev->m_duck + change;
		}
	}

	// set stuff before animating.
	entry->m_player->m_vecOrigin() = record->m_origin;
	entry->m_player->m_vecVelocity() = entry->m_player->m_vecAbsVelocity() = record->m_anim_velocity;
	entry->m_player->m_flLowerBodyYawTarget() = record->m_body;

	// resolve players
	if (!game::IsFakePlayer(entry->m_player->index()))
		g_resolver.ResolveAngles(entry->m_player, record);

	// skip call to C_BaseEntity::CalcAbsoluteVelocity
	entry->m_player->m_iEFlags() &= ~(0x1000 | 0x800);

	// write potentially resolved angles.
	entry->m_player->m_angEyeAngles() = record->m_eye_angles;

	// fix animating in same frame. 
	if (state->m_nLastUpdateFrame == g_csgo.m_globals->m_frame)
		state->m_nLastUpdateFrame = g_csgo.m_globals->m_frame - 1;

	// 'm_animating' returns true if being called from SetupVelocity, passes raw velocity to animstate.
	auto backup_csa = entry->m_player->m_bClientSideAnimation();
	g_hooks.m_bUpdatingCSA[entry->m_player->index()] = true;
	entry->m_player->m_bClientSideAnimation() = true;
	entry->m_player->UpdateClientSideAnimation();
	entry->m_player->m_bClientSideAnimation() = backup_csa;
	g_hooks.m_bUpdatingCSA[entry->m_player->index()] = false;

	// store updated/animated poses and rotation in lagrecord.
	entry->m_player->SetAnimLayers(record->m_layers);
	entry->m_player->GetPoseParameters(record->m_poses);

	// save abs yaw
	record->m_abs_ang = ang_t{ 0.f, state->m_flFootYaw, 0.f };

	// generate aimbot matrix
	record->m_setup = g_bones.SetupBones(entry->m_player, record->m_bones, 128, 0x7FF00, record->m_anim_time);

	// restore data to not mess with game animations
	entry->m_player->m_vecOrigin() = backup.m_origin;
	entry->m_player->m_vecVelocity() = backup.m_velocity;
	entry->m_player->m_vecAbsVelocity() = backup.m_abs_velocity;
	entry->m_player->m_fFlags() = backup.m_flags;
	entry->m_player->m_iEFlags() = backup.m_eflags;
	entry->m_player->m_flDuckAmount() = backup.m_duck;
	entry->m_player->m_flLowerBodyYawTarget() = backup.m_body;
	entry->m_player->SetAbsOrigin(backup.m_abs_origin);
	entry->m_player->SetAnimLayers(backup.m_layers);

	// restore globals.
	g_csgo.m_globals->m_curtime = m_flCurtime;
	g_csgo.m_globals->m_frametime = m_flFrametime;
}