#include "includes.h"

Movement g_movement{ };;

void Movement::JumpRelated( ) {
	if( g_cl.m_local->m_MoveType( ) == MOVETYPE_NOCLIP )
		return;

	if( ( g_cl.m_cmd->m_buttons & IN_JUMP ) && !( g_cl.m_flags & FL_ONGROUND ) ) {
		// bhop.
		if( g_menu.main.movement.bunny_hop.get( ) )
			g_cl.m_cmd->m_buttons &= ~IN_JUMP;
	}
}

void Movement::Strafe(CUserCmd* cmd, const ang_t& old_angs) {
	// don't strafe while noclipping or on ladders..
	if (!g_cl.m_local || g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	// disable strafing while pressing shift.
	// don't strafe if not holding primary jump key.
	if ((g_cl.m_buttons & IN_SPEED) || !(g_cl.m_buttons & IN_JUMP) || (g_cl.m_flags & FL_ONGROUND))
		return;

	static ConVar *cl_sidespeed = g_csgo.m_cvar->FindVar( HASH ( "cl_sidespeed" ) );
	static ConVar *cl_forwardspeed = g_csgo.m_cvar->FindVar( HASH ( "cl_forwardspeed" ) );

	float     target_yaw = cmd->m_view_angles.y;
	vec2_t direction = vec2_t{ cmd->m_move.x, cmd->m_move.y };
	if (!std::isnan(direction.x) && !std::isnan(direction.y) && (direction.x != 0.f || direction.y != 0.f))
	{
		target_yaw -= math::rad_to_deg(atan2(direction.y, direction.x));
	}

	float     scaled_smoothing_factor = (g_menu.main.movement.astrafe_smoothness.get() / 3.f) + 66.6666666666f;
	vec2_t velocity = vec2_t(g_cl.m_local->m_vecVelocity().x, g_cl.m_local->m_vecVelocity().y);
	float     difference = math::NormalizeYaw(target_yaw - math::rad_to_deg(atan2(velocity.y, velocity.x))) *
		(scaled_smoothing_factor / 100.f);
	cmd->m_move = vec3_t{ 0.f, (difference > 0.f) ? -450.f : 450.f, 0.f };
	wish_angle.y = math::NormalizeYaw(target_yaw - difference);
}

void Movement::DoPrespeed( ) {
	float   mod, min, max, step, strafe, time, angle;
	vec3_t  plane;

	// min and max values are based on 128 ticks.
	mod = g_csgo.m_globals->m_interval * 128.f;

	// scale min and max based on tickrate.
	min = 2.25f * mod;
	max = 5.f * mod;

	// compute ideal strafe angle for moving in a circle.
	strafe = m_ideal * 2.f;

	// clamp ideal strafe circle value to min and max step.
	math::clamp( strafe, min, max );

	// calculate time.
	time = 320.f / m_speed;

	// clamp time.
	math::clamp( time, 0.35f, 1.f );

	// init step.
	step = strafe;

	while( true ) {
		// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
		if( !WillCollide( time, step ) || max <= step )
			break;

		// if we will collide with an object with the current strafe step then increment step to prevent a collision.
		step += 0.2f;
	}

	if( step > max ) {
		// reset step.
		step = strafe;

		while( true ) {
			// if we will not collide with an object or we wont accelerate from such a big step anymore then stop.
			if( !WillCollide( time, step ) || step <= -min )
				break;

			// if we will collide with an object with the current strafe step decrement step to prevent a collision.
			step -= 0.2f;
		}

		if( step < -min ) {
			if( GetClosestPlane( plane ) ) {
				// grab the closest object normal
				// compute the angle of the normal
				// and push us away from the object.
				angle = math::rad_to_deg( std::atan2( plane.y, plane.x ) );
				step = -math::NormalizedAngle( m_circle_yaw - angle ) * 0.1f;
			}
		}

		else
			step -= 0.2f;
	}

	else
		step += 0.2f;

	// add the computed step to the steps of the previous circle iterations.
	m_circle_yaw = math::NormalizedAngle( m_circle_yaw + step );

	// apply data to usercmd.
	g_cl.m_cmd->m_view_angles.y = m_circle_yaw;
	g_cl.m_cmd->m_move.y = ( step >= 0.f ) ? -450.f : 450.f;
}

void Movement::DirectionalStrafe(CUserCmd* cmd, const ang_t& old_angs)
{
	static int strafer_flags = 0;

	if (!!(g_cl.m_local->m_fFlags() & FL_ONGROUND)) {
		strafer_flags = 0;
		return;
	}

	auto velocity = g_cl.m_local->m_vecVelocity();
	auto velocity_len = velocity.length_2d();

	if (velocity_len <= 0.0f) {
		strafer_flags = 0;
		return;
	}

	auto ideal_step = std::min < float >(90.0f, 845.5f / velocity_len);
	auto velocity_yaw = (velocity.y || velocity.x) ? math::rad_to_deg(atan2f(velocity.y, velocity.x)) : 0.0f;

	auto unmod_angles = old_angs;
	auto angles = old_angs;

	if (velocity_len < 2.0f && !!(cmd->m_buttons & IN_JUMP))
		cmd->m_move.x = 450.0f;

	auto forward_move = cmd->m_move.x;
	auto onground = !!(g_cl.m_local->m_fFlags() & FL_ONGROUND);

	if (forward_move || cmd->m_move.y) {
		cmd->m_move.x = 0.0f;

		if (velocity_len != 0.0f && abs(velocity.z) != 0.0f) {
			if (!onground) {
			DO_IT_AGAIN:
				vec3_t fwd;
				math::AngleVectors(angles, &fwd);
				auto right = fwd.cross(vec3_t(0.0f, 0.0f, 1.0f));

				auto v262 = (fwd.x * forward_move) + (cmd->m_move.y * right.x);
				auto v263 = (right.y * cmd->m_move.y) + (fwd.y * forward_move);

				angles.y = (v262 || v263) ? math::rad_to_deg(atan2f(v263, v262)) : 0.0f;
			}
		}
	}

	auto yaw_to_use = 0.0f;

	strafer_flags &= ~4;

	if (!onground) {
		auto clamped_angles = angles.y;

		if (clamped_angles < -180.0f) clamped_angles += 360.0f;
		if (clamped_angles > 180.0f) clamped_angles -= 360.0f;

		yaw_to_use = old_angs.y;

		strafer_flags |= 4;
	}

	if (strafer_flags & 4) {
		auto diff = angles.y - yaw_to_use;

		if (diff < -180.0f) diff += 360.0f;
		if (diff > 180.0f) diff -= 360.0f;

		if (abs(diff) > ideal_step && abs(diff) <= 30.0f) {
			auto move = 450.0f;

			if (diff < 0.0f)
				move *= -1.0f;

			cmd->m_move.y = move;
			return;
		}
	}

	auto diff = angles.y - velocity_yaw;

	if (diff < -180.0f) diff += 360.0f;
	if (diff > 180.0f) diff -= 360.0f;

	auto step = 0.6f * (ideal_step + ideal_step);
	auto side_move = 0.0f;

	if (abs(diff) > 170.0f && velocity_len > 80.0f || diff > step && velocity_len > 80.0f) {
		angles.y = step + velocity_yaw;
		cmd->m_move.y = -450.0f;
	}
	else if (-step <= diff || velocity_len <= 80.0f) {
		if (strafer_flags & 1) {
			angles.y -= ideal_step;
			cmd->m_move.y = -450.0f;
		}
		else {
			angles.y += ideal_step;
			cmd->m_move.y = 450.0f;
		}
	}
	else {
		angles.y = velocity_yaw - step;
		cmd->m_move.y = 450.0f;
	}

	if (!(cmd->m_buttons & IN_BACK) && !cmd->m_move.y)
		goto DO_IT_AGAIN;

	strafer_flags ^= (strafer_flags ^ ~strafer_flags) & 1;

	if (angles.y < -180.0f) angles.y += 360.0f;
	if (angles.y > 180.0f) angles.y -= 360.0f;
}

bool Movement::GetClosestPlane( vec3_t &plane ) {
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;
	vec3_t                start{ m_origin };
	float                 smallest{ 1.f };
	const float		      dist{ 75.f };

	// trace around us in a circle
	for( float step{ }; step <= math::pi_2; step += ( math::pi / 10.f ) ) {
		// extend endpoint x units.
		vec3_t end = start;
		end.x += std::cos( step ) * dist;
		end.y += std::sin( step ) * dist;

		g_csgo.m_engine_trace->TraceRay( Ray( start, end, m_mins, m_maxs ), CONTENTS_SOLID, &filter, &trace );

		// we found an object closer, then the previouly found object.
		if( trace.m_fraction < smallest ) {
			// save the normal of the object.
			plane = trace.m_plane.m_normal;
			smallest = trace.m_fraction;
		}
	}

	// did we find any valid object?
	return smallest != 1.f && plane.z < 0.1f;
}

bool Movement::WillCollide( float time, float change ) {
	struct PredictionData_t {
		vec3_t start;
		vec3_t end;
		vec3_t velocity;
		float  direction;
		bool   ground;
		float  predicted;
	};

	PredictionData_t      data;
	CGameTrace            trace;
	CTraceFilterWorldOnly filter;

	// set base data.
	data.ground = g_cl.m_flags & FL_ONGROUND;
	data.start = m_origin;
	data.end = m_origin;
	data.velocity = g_cl.m_local->m_vecVelocity( );
	data.direction = math::rad_to_deg( std::atan2( data.velocity.y, data.velocity.x ) );

	for( data.predicted = 0.f; data.predicted < time; data.predicted += g_csgo.m_globals->m_interval ) {
		// predict movement direction by adding the direction change.
		// make sure to normalize it, in case we go over the -180/180 turning point.
		data.direction = math::NormalizedAngle( data.direction + change );

		// pythagoras.
		float hyp = data.velocity.length_2d( );

		// adjust velocity for new direction.
		data.velocity.x = std::cos( math::deg_to_rad( data.direction ) ) * hyp;
		data.velocity.y = std::sin( math::deg_to_rad( data.direction ) ) * hyp;

		// assume we bhop, set upwards impulse.
		if( data.ground )
			data.velocity.z = g_csgo.sv_jump_impulse->GetFloat( );

		else
			data.velocity.z -= g_csgo.sv_gravity->GetFloat( ) * g_csgo.m_globals->m_interval;

		// we adjusted the velocity for our new direction.
		// see if we can move in this direction, predict our new origin if we were to travel at this velocity.
		data.end += ( data.velocity * g_csgo.m_globals->m_interval );

		// trace
		g_csgo.m_engine_trace->TraceRay( Ray( data.start, data.end, m_mins, m_maxs ), MASK_PLAYERSOLID, &filter, &trace );

		// check if we hit any objects.
		if( trace.m_fraction != 1.f && trace.m_plane.m_normal.z <= 0.9f )
			return true;
		if( trace.m_startsolid || trace.m_allsolid )
			return true;

		// adjust start and end point.
		data.start = data.end = trace.m_endpos;

		// move endpoint 2 units down, and re-trace.
		// do this to check if we are on th floor.
		g_csgo.m_engine_trace->TraceRay( Ray( data.start, data.end - vec3_t{ 0.f, 0.f, 2.f }, m_mins, m_maxs ), MASK_PLAYERSOLID, &filter, &trace );

		// see if we moved the player into the ground for the next iteration.
		data.ground = trace.hit( ) && trace.m_plane.m_normal.z > 0.7f;
	}

	// the entire loop has ran
	// we did not hit shit.
	return false;
}

#pragma optimize( "", off )


void Movement::LegMovement(CUserCmd* cmd) {

	if (g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
		return;

	const float forwardmove = cmd->m_move.x;
	const float sidemove = cmd->m_move.y;
	int new_buttons = cmd->m_buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

	if (!g_menu.main.movement.leg_movement.get()) {

		if (forwardmove <= 0.f) {
			if (forwardmove < 0.f)
				new_buttons |= IN_BACK;
		}
		else
			new_buttons |= IN_FORWARD;

		if (sidemove >= 0.f) {

			if (sidemove > 0.f)
				goto LABEL_15;

			goto LABEL_18;
		}

		goto LABEL_17;
	}

	if (g_menu.main.movement.leg_movement.get() != 1)
		goto LABEL_18;

	if (forwardmove <= 0.f)
	{
		if (forwardmove < 0.f)
			new_buttons |= IN_FORWARD;
	}
	else
		new_buttons |= IN_BACK;

	if (sidemove > 0.f) {

	LABEL_17:
		new_buttons |= IN_MOVELEFT;
		goto LABEL_18;
	}

	if (sidemove < 0.f) {
	LABEL_15:
		new_buttons |= IN_MOVERIGHT;
	}

LABEL_18:
	cmd->m_buttons = new_buttons;
}

void Movement::FixMove(CUserCmd* user_cmd)
{
	user_cmd->m_move.x = std::clamp(user_cmd->m_move.x, -450.f, 450.f);
	user_cmd->m_move.y = std::clamp(user_cmd->m_move.y, -450.f, 450.f);

	float     y;                 // xmm3_4
	float     length_2d;         // xmm4_4
	float     x;                 // xmm0_4
	float     v11;               // xmm3_4
	float     v12;               // xmm4_4
	float     v13;               // xmm0_4
	float     v14;               // xmm5_4
	float     v15;               // xmm5_4
	float     v16;               // xmm5_4
	float     v17;               // xmm7_4
	float     v18;               // xmm6_4
	float     v19;               // xmm3_4
	float     v21;               // [esp+Ch] [ebp-74h]
	float     v22;               // [esp+Ch] [ebp-74h]
	float     z;                 // [esp+10h] [ebp-70h]
	float     v24;               // [esp+10h] [ebp-70h]
	float     v25;               // [esp+14h] [ebp-6Ch]
	float     v26;               // [esp+14h] [ebp-6Ch]
	float     v27;               // [esp+18h] [ebp-68h]
	float     v28;               // [esp+1Ch] [ebp-64h]
	float     v29;               // [esp+20h] [ebp-60h]
	float     v30;               // [esp+24h] [ebp-5Ch]
	float     v31;               // [esp+28h] [ebp-58h]
	float     v32;               // [esp+28h] [ebp-58h]
	float     v33;               // [esp+2Ch] [ebp-54h]
	float     v34;               // [esp+30h] [ebp-50h]
	float     x_len;             // [esp+34h] [ebp-4Ch]
	float     v36;               // [esp+38h] [ebp-48h]
	float     v37;               // [esp+40h] [ebp-40h]
	ang_t input;             // [esp+44h] [ebp-3Ch] BYREF
	vec3_t input_forward;     // [esp+50h] [ebp-30h] BYREF
	vec3_t input_right;       // [esp+5Ch] [ebp-24h] BYREF
	ang_t local_view_angles; // [esp+68h] [ebp-18h] BYREF
	vec3_t input_up;          // [esp+74h] [ebp-Ch] BYREF

	input = wish_angle;
	local_view_angles = user_cmd->m_view_angles;
	int move_type = g_cl.m_local->m_MoveType();

	if (wish_angle.x == local_view_angles.x && wish_angle.y == local_view_angles.y &&
		wish_angle.z == local_view_angles.z ||
		sqrt((user_cmd->m_move.x * user_cmd->m_move.x) + (user_cmd->m_move.y * user_cmd->m_move.y)) == 0.f || move_type == 8 ||
		move_type == 9)
	{
		return;
	}

	math::AngleVectors(input, &input_forward, &input_right, &input_up);
	y = input_forward.y;
	z = input_forward.z;
	v37 = 1.0;
	if (input_forward.z == 0.0)
	{
		x = input_forward.x;
	}
	else
	{
		z = 0.0;
		length_2d = sqrt((input_forward.x * input_forward.x) + (input_forward.y * input_forward.y));
		if (length_2d >= 0.00000011920929)
		{
			y = input_forward.y * (1.0 / length_2d);
			x_len = input_forward.x * (1.0 / length_2d);
			goto LABEL_13;
		}
		y = 0.0;
		x = 0.0;
	}
	x_len = x;
LABEL_13:
	v36 = y;
	v11 = input_right.y;
	v31 = input_right.z;
	if (input_right.z == 0.0)
	{
		v13 = input_right.x;
		goto LABEL_18;
	}
	v31 = 0.0;
	v12 = sqrt((input_right.x * input_right.x) + (input_right.y * input_right.y));
	if (v12 < 0.00000011920929)
	{
		v11 = 0.0;
		v13 = 0.0;
	LABEL_18:
		v21 = v13;
		goto LABEL_19;
	}
	v11 = input_right.y * (1.0 / v12);
	v21 = input_right.x * (1.0 / v12);
LABEL_19:
	if (input_up.z < 0.00000011920929)
	{
		v25 = 0.0;
	}
	else
	{
		v25 = 1.0;
	}
	/*if (local_view_angles.z == 180.0 && weapon_accuracy_nospread) TODO
		LODWORD(cmd->forwardmove) ^= NegateMask;*/
	math::AngleVectors(local_view_angles, &input_right, &input_forward, &input_up);
	v33 = input_right.z;
	if (input_right.z == 0.0)
	{
		v27 = input_right.y;
		v28 = input_right.x;
	}
	else
	{
		v33 = 0.0;
		v14 = sqrt((input_right.x * input_right.x) + (input_right.y * input_right.y));
		if (v14 < 0.00000011920929)
		{
			v27 = 0.0;
			v28 = 0.0;
		}
		else
		{
			v28 = input_right.x * (1.0 / v14);
			v27 = input_right.y * (1.0 / v14);
		}
	}
	v34 = input_forward.z;
	if (input_forward.z == 0.0)
	{
		v29 = input_forward.y;
		v30 = input_forward.x;
	}
	else
	{
		v34 = 0.0;
		v15 = sqrt((input_forward.x * input_forward.x) + (input_forward.y * input_forward.y));
		if (v15 < 0.00000011920929)
		{
			v29 = 0.0;
			v30 = 0.0;
		}
		else
		{
			v30 = input_forward.x * (1.0 / v15);
			v29 = input_forward.y * (1.0 / v15);
		}
	}
	if (input_up.z < 0.00000011920929)
	{
		v37 = 0.0;
	}
	v16 = v11 * user_cmd->m_move.y;
	v24 = z * user_cmd->m_move.x;
	v22 = v21 * user_cmd->m_move.y;
	v17 = x_len * user_cmd->m_move.x;
	v18 = v36 * user_cmd->m_move.x;
	v26 = v25 * user_cmd->m_move.z;
	v32 = v31 * user_cmd->m_move.y;
	v19 = user_cmd->m_move.z * 0.0;
	user_cmd->m_move.x = ((((v16 * v27) + (v22 * v28)) + (v32 * v33)) + (((v18 * v27) + (v17 * v28)) + (v24 * v33))) +
		(((v19 * v27) + (v19 * v28)) + (v26 * v33));
	user_cmd->m_move.y = ((((v16 * v29) + (v22 * v30)) + (v32 * v34)) + (((v18 * v29) + (v17 * v30)) + (v24 * v34))) +
		(((v19 * v29) + (v19 * v30)) + (v26 * v34));
	user_cmd->m_move.z = v32 * v37 + v24 * v37 + v26 * v37;

	FixButtonMoves(user_cmd); 
	LegMovement(user_cmd);
}

void Movement::FixButtonMoves(CUserCmd* user_cmd)
{
	user_cmd->m_buttons &= ~(IN_FORWARD | IN_BACK | IN_MOVELEFT | IN_MOVERIGHT);
	if (user_cmd->m_move.y < 0.f)
	{
		user_cmd->m_buttons |= IN_MOVELEFT;
	}
	else if (user_cmd->m_move.y > 0.f)
	{
		user_cmd->m_buttons |= IN_MOVERIGHT;
	}

	if (user_cmd->m_move.x < 0.f)
	{
		user_cmd->m_buttons |= IN_BACK;
	}
	else if (user_cmd->m_move.x > 0.f)
	{
		user_cmd->m_buttons |= IN_FORWARD;
	}
}


//void Movement::AutoPeek(CUserCmd* cmd, float wish_yaw) {
//	if (g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
//		if (start_position.IsZero()) {
//			start_position = g_cl.m_local->m_vecOrigin();
//
//			if (!(g_cl.m_flags & FL_ONGROUND)) {
//				CTraceFilterWorldOnly filter;
//				CGameTrace trace;
//
//				g_csgo.m_engine_trace->TraceRay(Ray(start_position, start_position - vec3_t(0.0f, 0.0f, 1000.0f)), MASK_SOLID, &filter, &trace);
//
//				if (trace.m_fraction < 1.0f)
//					start_position = trace.m_endpos + vec3_t(0.0f, 0.0f, 2.0f);
//			}
//		}
//		else {
//			bool revolver_shoot = g_cl.m_weapon_id == REVOLVER && !g_cl.m_revolver_fire && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);
//
//			if (fired_shot) {
//				vec3_t current_position = g_cl.m_local->m_vecOrigin();
//				vec3_t difference = current_position - start_position;
//
//				if (difference.length() > 0.1f) {
//					cmd->m_move.x = 450.f;
//					cmd->m_move.y = 0.f;
//
//					vec3_t ang = math::CalcAngle( g_cl.m_local->m_vecOrigin( ), g_movement.start_position );
//					g_cl.m_strafe_angles.y = ang.y;
//				}
//				else {
//					fired_shot = false;
//				}
//			}
//		}
//	}
//
//	else {
//		fired_shot = false;
//		start_position.clear();
//	}
//}

void Movement::AutoPeek(CUserCmd* cmd, float wish_yaw) {
	if (g_input.GetKeyState(g_menu.main.movement.autopeek.get())) {
		if (start_position.is_zero()) {
			start_position = g_cl.m_local->GetAbsOrigin();

			if (!(g_cl.m_flags & FL_ONGROUND)) {
				CTraceFilterWorldOnly filter;
				CGameTrace trace;

				g_csgo.m_engine_trace->TraceRay(Ray(start_position, start_position - vec3_t(0.0f, 0.0f, 1000.0f)), MASK_SOLID, &filter, &trace);

				if (trace.m_fraction < 1.0f)
					start_position = trace.m_endpos + vec3_t(0.0f, 0.0f, 2.0f);
			}
		}
		else {
			bool revolver_shoot = g_cl.m_weapon_id == REVOLVER && !g_cl.m_revolver_fire && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

			if (g_cl.m_old_shot)
				m_invert = true;

			if (m_invert) {
				vec3_t current_position = g_cl.m_local->GetAbsOrigin();
				vec3_t difference = current_position - start_position;

				if (difference.length_2d() > 5.0f) {
					vec3_t velocity = vec3_t(difference.x * cos(wish_yaw / 180.0f * math::pi) + difference.y * sin(wish_yaw / 180.0f * math::pi), difference.y * cos(wish_yaw / 180.0f * math::pi) - difference.x * sin(wish_yaw / 180.0f * math::pi), difference.z);

					if (difference.length_2d() < 50.0f) {
						cmd->m_move.x = -velocity.x * 20.0f;
						cmd->m_move.z = velocity.y * 20.0f;
					}
					else if (difference.length_2d() < 100.0f) {
						cmd->m_move.x = -velocity.x * 10.0f;
						cmd->m_move.z = velocity.y * 10.0f;
					}
					else if (difference.length_2d() < 150.0f) {
						cmd->m_move.x = -velocity.x * 5.0f;
						cmd->m_move.z = velocity.y * 5.0f;
					}
					else if (difference.length_2d() < 250.0f) {
						cmd->m_move.x = -velocity.x * 2.0f;
						cmd->m_move.z = velocity.y * 2.0f;
					}
					else {
						cmd->m_move.x = -velocity.x * 1.0f;
						cmd->m_move.z = velocity.y * 1.0f;
					}
				}
				else {
					m_invert = false;
					start_position.clear();
				}
			}
		}
	}
	else {
		m_invert = false;
		start_position.clear();

		return;
	}
}

void Movement::NullVelocity(CUserCmd* cmd) {
	vec3_t vel = g_cl.m_local->m_vecVelocity();

	if (vel.length_2d() < 15.f) {
		cmd->m_move.x = cmd->m_move.y = 0.f;
		return;
	}

	ang_t direction;
	math::VectorAngles(vel, direction);

	ang_t view_angles;
	g_csgo.m_engine->GetViewAngles(view_angles);

	direction.y = view_angles.y - direction.y;

	vec3_t forward;
	math::AngleVectors(direction, &forward);

	static ConVar* cl_forwardspeed = g_csgo.m_cvar->FindVar(HASH("cl_forwardspeed"));
	static ConVar* cl_sidespeed = g_csgo.m_cvar->FindVar(HASH("cl_sidespeed"));

	const vec3_t negative_forward_direction = forward * -cl_forwardspeed->GetFloat();
	const vec3_t negative_side_direction = forward * -cl_sidespeed->GetFloat();

	cmd->m_move.x = negative_forward_direction.x;
	cmd->m_move.y = negative_side_direction.y;
}

void Movement::AutoStop() {

	if (!(g_cl.m_local->m_fFlags() & FL_ONGROUND))
		return;

	if (g_cl.m_cmd->m_buttons & IN_JUMP)
		return;

	if (!g_cl.m_weapon_info)
		return;

	if (!g_cl.m_weapon) // sanity check
		return;

	if (!g_cl.m_local || !g_cl.m_processing)
		return;

	if (g_menu.main.aimbot.quick_stop_mode.get() <= 0 || !g_aimbot.m_stop)
		return;

	Weapon* wpn = g_cl.m_local->GetActiveWeapon();

	if (!wpn)
		return;

	WeaponInfo* wpn_data = wpn->GetWpnData();


	if (!wpn_data)
		return;

	bool full_stop = g_menu.main.aimbot.quick_stop_type.get() == 0;
	bool minimal_speed = g_menu.main.aimbot.quick_stop_type.get() == 1;
	bool fakewalk_stop = g_menu.main.aimbot.quick_stop_type.get() == 2;
	float max_speed = std::floor(0.33f * (g_cl.m_local->m_bIsScoped() ? wpn_data->m_max_player_speed_alt : wpn_data->m_max_player_speed));
	if (full_stop)
	{
		if (full_stop || g_cl.m_weapon_id == WEAPON_ZEUS || !(g_cl.m_flags & FL_ONGROUND))
			max_speed = 10.f;

		if (g_cl.m_local->m_vecVelocity().length_2d() < max_speed) {

			if (full_stop)
				g_cl.m_cmd->m_move.x = g_cl.m_cmd->m_move.y = 0.f;
			else
				ClampMovementSpeed(max_speed);
		}
		else {
			NullVelocity(g_cl.m_cmd);
		}
	}
	else if (minimal_speed)
	{
		if (g_cl.m_local->m_vecVelocity().length_2d() < max_speed)
			ClampMovementSpeed(max_speed);
		else
			NullVelocity(g_cl.m_cmd);
	}
	else if (fakewalk_stop)
	{
		FakeWalk(true);
	}
}

int get_ticks_to_stop()
{
	static auto predict_velocity = [](vec3_t* velocity)
		{
			float speed = velocity->length_2d();
			if (speed >= 1.f)
			{
				float friction = g_csgo.sv_friction->GetFloat();
				float stop_speed = std::max< float >(speed, g_csgo.sv_stopspeed->GetFloat());
				float time = std::max< float >(g_csgo.m_globals->m_interval, g_csgo.m_globals->m_frametime);
				*velocity *= std::max< float >(0.f, speed - friction * stop_speed * time / speed);
			}
		};

	auto vel = g_cl.m_local->m_vecVelocity();
	int ticks_to_stop = 0;
	do
	{
		if (vel.length_2d() < 1.f)
			break;

		predict_velocity(&vel);
		ticks_to_stop++;
	} while (ticks_to_stop < 15);

	return ticks_to_stop;
}

void InstantStop(CUserCmd* cmd) {
	ang_t angle;
	math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);

	float speed = g_cl.m_local->m_vecVelocity().length();

	angle.y = g_cl.m_orig_ang.y - angle.y;

	vec3_t direction;
	math::AngleVectors(angle, direction);

	vec3_t stop = direction * -speed;

	if (speed > 13.f)
	{
		cmd->m_move.x = stop.x;
		cmd->m_move.y = stop.y;
	}
	else
	{
		cmd->m_move.x = 0.f;
		cmd->m_move.y = 0.f;
	}
}

vec3_t PredictVelocity(vec3_t vel)
{
	float speed = vel.length();
	if (speed < 0.1f)
		return {};

	return vel * std::max(0.f,
		speed - g_csgo.sv_friction->GetFloat() * std::max(speed, g_csgo.sv_stopspeed->GetFloat()) *
		std::max(g_csgo.m_globals->m_interval, g_csgo.m_globals->m_frametime) / speed);
}

void Movement::FakeWalk(bool forced) {
	if (!g_input.GetKeyState(g_menu.main.movement.fake_walk.get()))
		return;

	if (!(g_cl.m_cmd->m_buttons & IN_SPEED) || !g_cl.m_local->GetGroundEntity())
		return;

	*g_cl.m_packet = g_csgo.m_cl->m_choked_commands >= 15;

	vec3_t velocity{ g_cl.m_local->m_vecVelocity() };
	float speed = velocity.length();

	// calculate friction.
	float friction = g_csgo.sv_friction->GetFloat() * g_cl.m_local->m_surfaceFriction();

	int ticks_to_stop{};
	for (; ticks_to_stop < 15; ++ticks_to_stop) {
		// calculate speed.
		float speed = velocity.length();

		// if too slow return.
		if (speed <= 0.1f)
			break;

		// bleed off some speed, but if we have less than the bleed, threshold, bleed the threshold amount.
		float control = std::max(speed, g_csgo.sv_stopspeed->GetFloat());

		// calculate the drop amount.
		float drop = control * friction * g_csgo.m_globals->m_interval;

		// scale the velocity.
		float newspeed = std::max(0.f, speed - drop);

		if (newspeed != speed) {
			// determine proportion of old speed we are using.
			newspeed /= speed;

			// adjust velocity according to proportion.
			velocity *= newspeed;
		}
	}

	if (ticks_to_stop + 1 >= (15 - g_csgo.m_cl->m_choked_commands)
		|| !g_csgo.m_cl->m_choked_commands)
	{
		g_cl.m_cmd->m_move = {};
	}

	g_cl.m_cmd->m_buttons &= ~IN_SPEED;
}

void Movement::FastStop() {
	if (!g_cl.m_cmd || !g_cl.m_local || !g_cl.m_local->alive())
		return;

	if (!g_cl.m_pressing_move && g_menu.main.movement.fast_stop.get() && g_cl.m_speed > 15.f && !g_input.GetKeyState(g_menu.main.movement.fake_walk.get())) {
		auto weapon = g_cl.m_local->GetActiveWeapon();

		// don't fake movement while noclipping or on ladders..
		if (!weapon || !weapon->GetWpnData() || g_cl.m_local->m_MoveType() == MOVETYPE_NOCLIP || g_cl.m_local->m_MoveType() == MOVETYPE_LADDER)
			return;

		if (!(g_cl.m_local->m_fFlags() & FL_ONGROUND))
			return;

		if (g_cl.m_cmd->m_buttons & IN_JUMP)
			return;

		auto move_speed = sqrtf((g_cl.m_cmd->m_move.x * g_cl.m_cmd->m_move.x) + (g_cl.m_cmd->m_move.y * g_cl.m_cmd->m_move.y));
		auto pre_prediction_velocity = g_cl.m_local->m_vecVelocity().length_2d();

		auto v58 = g_csgo.sv_stopspeed->GetFloat();
		v58 = fmaxf(v58, pre_prediction_velocity);
		v58 = g_csgo.sv_friction->GetFloat() * v58;
		auto slow_walked_speed = fmaxf(pre_prediction_velocity - (v58 * g_csgo.m_globals->m_interval), 0.0f);

		if (slow_walked_speed <= 0 || pre_prediction_velocity <= slow_walked_speed) {
			g_cl.m_cmd->m_move.x = 0;
			g_cl.m_cmd->m_move.y = 0;
			return;
		}

		ang_t angle;
		math::VectorAngles(g_cl.m_local->m_vecVelocity(), angle);

		// get our current speed of travel.
		float speed = g_cl.m_local->m_vecVelocity().length();

		// fix direction by factoring in where we are looking.
		angle.y = g_cl.m_view_angles.y - angle.y;

		// convert corrected angle back to a direction.
		vec3_t direction;
		math::AngleVectors(angle, &direction);

		vec3_t stop = direction * -speed;

		g_cl.m_cmd->m_move.x = stop.x;
		g_cl.m_cmd->m_move.y = stop.y;
	}
}

void Movement::ClampMovementSpeed(float speed)
{
	// thanks onetap.
	if (!g_cl.m_cmd || !g_cl.m_processing)
		return;

	g_cl.m_cmd->m_buttons |= IN_SPEED;

	float squirt = std::sqrtf((g_cl.m_cmd->m_move.x * g_cl.m_cmd->m_move.x) + (g_cl.m_cmd->m_move.y * g_cl.m_cmd->m_move.y));

	if (squirt > speed) {
		float squirt2 = std::sqrtf((g_cl.m_cmd->m_move.x * g_cl.m_cmd->m_move.x) + (g_cl.m_cmd->m_move.y * g_cl.m_cmd->m_move.y));

		float cock1 = g_cl.m_cmd->m_move.x / squirt2;
		float cock2 = g_cl.m_cmd->m_move.y / squirt2;

		if (speed + 1.f <= g_cl.m_local->m_vecVelocity().length_2d()) {
			g_cl.m_cmd->m_move.x = g_cl.m_cmd->m_move.y = 0.f;
		}
		else {
			g_cl.m_cmd->m_move.x = cock1 * speed;
			g_cl.m_cmd->m_move.y = cock2 * speed;
		}
	}
}
