#include "includes.h"
#include "autowall.h"

bool CGameTrace::DidHitWorld() {
	if (!m_entity)
		return false;

	return m_entity == g_csgo.m_entlist->GetClientEntity(0);
}

bool CGameTrace::DidHitNonWorldEntity() {
	return m_entity != nullptr && !DidHitWorld();
}


pen_data_t c_auto_wall::simulate_fire(const vec3_t& shoot_pos, const vec3_t& point, Player* enemy, bool strict, int hitgroup) {
	const auto who_tf = point - shoot_pos;
	ang_t angle = {};
	math::VectorAngles(who_tf, angle);

	vec3_t dir = {};
	math::AngleVectors(angle, &dir);

	dir.normalize();

	auto dmg{ 0.f };
	auto hitbox{ -1 };
	auto remain_pen{ -1 };
	auto hit_group{ -1 };
	bool dont_care{};
	if (g_cl.m_weapon && fire_bullet(g_cl.m_weapon, dir, dont_care, dmg, remain_pen, hit_group, hitbox, enemy, 0.0f, shoot_pos, strict, hitgroup))
		return pen_data_t(static_cast <int> (dmg), hitbox, hit_group, remain_pen);
	else
		return pen_data_t(-1, -1, -1, -1);
}

__forceinline bool has_armor(Player* player, int hit_group) {
	const bool has_helmet = player->m_bHasHelmet();
	const bool has_heavy_armor = player->m_bHasHeavyArmor();
	const float armor_val = player->m_ArmorValue();

	if (armor_val > 0.f) {
		switch (hit_group) {
		case 2:
		case 3:
		case 4:
		case 5:
			return true;
			break;
		case 1:
			return has_helmet || has_heavy_armor;
			break;
		default:
			return has_heavy_armor;
			break;
		}
	}

	return false;
}

void c_auto_wall::scale_dmg(Player* player, CGameTrace& trace, WeaponInfo* wpn_info, float& dmg, const int hit_group) {
	if (!player->IsPlayer())
		return;

	const auto team = player->m_iTeamNum();
	const auto armored = has_armor(player, hit_group);
	const bool is_zeus = g_cl.m_local->GetActiveWeapon() ? g_cl.m_local->GetActiveWeapon()->m_iItemDefinitionIndex() == ZEUS : false;

	const auto armor_val = static_cast <float> (player->m_ArmorValue());

	if (!is_zeus) {
		switch (hit_group) {
		case 1:
			dmg *= 4.f;
			break;
		case 3:
			dmg *= 1.25f;
			break;
		case 6:
		case 7:
			dmg *= 0.75f;
			break;
		default:
			break;
		}
	}

	if (!g_cl.m_local
		|| !g_cl.m_weapon
		|| !g_cl.m_weapon_info)
		return;

	const float armor_ratio = g_cl.m_weapon_info->m_armor_ratio;

	if (armored) {
		float armor_scale = 1.f;
		float armor_bonus_ratio = 0.5f;
		float armor_ratio_calced = armor_ratio * 0.5f;
		float dmg_to_health = dmg * armor_ratio_calced;
		float dmg_to_armor = (dmg - dmg_to_health) * (armor_scale * armor_bonus_ratio);

		if (dmg_to_armor > armor_val)
			dmg_to_health = dmg - (armor_val / armor_bonus_ratio);

		dmg = dmg_to_health;
	}

	dmg = std::floor(dmg);
}

static bool TraceToExitGame(vec3_t start, vec3_t dir, vec3_t& end, CGameTrace& trEnter, CGameTrace& trExit, float flStepSize, float flMaxDistance)
{
	float flDistance = 0;
	vec3_t last = start;
	int nStartContents = 0;
	CTraceFilterSimple_game filter{};

	while (flDistance <= flMaxDistance)
	{
		flDistance += flStepSize;

		end = start + (flDistance * dir);

		vec3_t vecTrEnd = end - (flStepSize * dir);

		int nCurrentContents = g_csgo.m_engine_trace->GetPointContents(end, CS_MASK_SHOOT | CONTENTS_HITBOX);

		if (nStartContents == 0)
			nStartContents = nCurrentContents;

		if ((nCurrentContents & CS_MASK_SHOOT) == 0 || ((nCurrentContents & CONTENTS_HITBOX) && nStartContents != nCurrentContents))
		{
			// this gets a bit more complicated and expensive when we have to deal with displacements
			g_csgo.m_engine_trace->TraceRay(Ray(end, vecTrEnd), CS_MASK_SHOOT | CONTENTS_HITBOX, NULL, &trExit);

			// we exited the wall into a player's hitbox
			if (trExit.m_startsolid == true && (trExit.m_surface.m_flags & SURF_HITBOX))
			{
				// do another trace, but skip the player to get the actual exit surface 
				filter.SetPassEntity(trExit.m_entity);
				g_csgo.m_engine_trace->TraceRay(Ray(end, start), CS_MASK_SHOOT, (ITraceFilter*)&filter, &trExit);
				if (trExit.hit() && trExit.m_startsolid == false)
				{
					end = trExit.m_endpos;
					return true;
				}
			}
			else if (trExit.hit() && trExit.m_startsolid == false)
			{
				bool bStartIsNodraw = !!(trEnter.m_surface.m_flags & (SURF_NODRAW));
				bool bExitIsNodraw = !!(trExit.m_surface.m_flags & (SURF_NODRAW));
				if (bExitIsNodraw && game::IsBreakable(trExit.m_entity) && game::IsBreakable(trEnter.m_entity))
				{
					// we have a case where we have a breakable object, but the mapper put a nodraw on the backside
					end = trExit.m_endpos;
					return true;
				}
				else if (bExitIsNodraw == false || (bStartIsNodraw && bExitIsNodraw)) // exit nodraw is only valid if our entrace is also nodraw
				{
					vec3_t vecNormal = trExit.m_plane.m_normal;
					float flDot = dir.dot(vecNormal);
					if (flDot <= 1.0f)
					{
						// get the real end pos
						end = end - ((flStepSize * trExit.m_fraction) * dir);
						return true;
					}
				}
			}
			else if (trEnter.DidHitNonWorldEntity() && game::IsBreakable(trEnter.m_entity))
			{
				// if we hit a breakable, make the assumption that we broke it if we can't find an exit (hopefully..)
				// fake the end pos
				trExit = trEnter;
				trExit.m_endpos = start + (1.0f * dir);
				return true;
			}
		}
	}

	return false;
}

bool c_auto_wall::handle_bullet_penetration(
	WeaponInfo* wpn_data, CGameTrace& enter_trace, vec3_t& eye_pos, const vec3_t& direction, int& possible_hits_remain,
	float& cur_dmg, float penetration_power, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration
) {
	if (wpn_data->m_penetration <= 0.0f)
		return false;

	if (possible_hits_remain <= 0)
		return false;

	const bool contents_grate = (enter_trace.m_contents & CONTENTS_GRATE);
	const bool surf_nodraw = (enter_trace.m_surface.m_flags & SURF_NODRAW);

	surfacedata_t* enter_surf_data = g_csgo.m_phys_props->GetSurfaceData(enter_trace.m_surface.m_surface_props);
	const uint16_t enter_material = enter_surf_data->m_game.m_material;

	const bool is_solid_surf = (enter_trace.m_contents >> 3 & CONTENTS_SOLID);
	const bool is_light_surf = (enter_trace.m_surface.m_flags >> 7 & 0x0001);

	CGameTrace exit_trace;

	//if ( !trace_to_exit( enter_trace.m_endpos, direction, enter_trace, exit_trace )
	//	&& !( g_csgo.m_engine_trace->GetPointContents( enter_trace.m_endpos, MASK_SHOT_HULL ) & MASK_SHOT_HULL ) )
	//	return false;

	vec3_t pen_end;
	if (!TraceToExitGame(enter_trace.m_endpos, direction, pen_end, enter_trace, exit_trace, 4.f, 90.f)) {
		if (!(g_csgo.m_engine_trace->GetPointContents(pen_end, CS_MASK_SHOOT) & CS_MASK_SHOOT)) {
			return false;
		}
	}

	const float enter_penetration_modifier = enter_surf_data->m_game.m_penetration_modifier;
	surfacedata_t* exit_surface_data = g_csgo.m_phys_props->GetSurfaceData(exit_trace.m_surface.m_surface_props);

	if (!exit_surface_data)
		return false;

	const uint16_t exit_material = exit_surface_data->m_game.m_material;
	const float exit_penetration_modifier = exit_surface_data->m_game.m_penetration_modifier;

	float combined_damage_modifier = 0.16f;
	float combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;

	if (enter_material == CHAR_TEX_GRATE
		|| enter_material == CHAR_TEX_GLASS) {
		combined_penetration_modifier = 3.0f;
		combined_damage_modifier = 0.05f;
	}
	else if (contents_grate || surf_nodraw) {
		combined_penetration_modifier = 1.0f;
		combined_damage_modifier = 0.16f;
	}

	// thin metals, wood and plastic get a penetration bonus.
	if (enter_material == exit_material) {

		if (exit_material == CHAR_TEX_CARDBOARD
			|| exit_material == CHAR_TEX_WOOD)
			combined_penetration_modifier = 3.f;

		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.f;
	}

	const float trace_len = (exit_trace.m_endpos - enter_trace.m_endpos).length();
	const float total_pen_mod = std::max(0.f, (1.f / combined_penetration_modifier));
	const float percent_dmg_chunk = cur_dmg * combined_damage_modifier;
	const float total_wpn_pen_mod = percent_dmg_chunk + std::max(0.f, (3.f / wpn_data->m_penetration) * 1.25f) * (total_pen_mod * 3.f);
	const float lost_dmg_obj = ((total_pen_mod * (trace_len * trace_len)) / 24.f);
	const float damage_lost = total_wpn_pen_mod + lost_dmg_obj;

	if (damage_lost > cur_dmg)
		return false;

	cur_dmg -= std::max(0.f, damage_lost);

	if (cur_dmg <= 1.f)
		return false;

	eye_pos = exit_trace.m_endpos;
	--possible_hits_remain;

	return true;
}

void c_auto_wall::ClipTraceToPlayer(const vec3_t& src,
	const vec3_t& dst,
	CGameTrace& trace,
	Player* const player,
	ShouldHitFunc_t should_hit_fn) {
	vec3_t     pos, to, dir, on_ray;
	float      len, range_along, range;
	Ray        ray;
	CGameTrace new_trace;

	// reference: https://github.com/alliedmodders/hl2sdk/blob/3957adff10fe20d38a62fa8c018340bf2618742b/game/shared/util_shared.h#L381

	// set some local vars.
	pos = player->m_vecOrigin() + ((player->m_vecMins() + player->m_vecMaxs()) * 0.5f);
	to = pos - src;
	dir = src - dst;
	len = dir.normalize();
	range_along = dir.dot(to);

	// off start point.
	if (range_along < 0.f)
		range = -(to).length();

	// off end point.
	else if (range_along > len)
		range = -(pos - dst).length();

	// within ray bounds.
	else {
		on_ray = src + (dir * range_along);
		range = (pos - on_ray).length();
	}

	if (range <= 60.f) {
		// clip to player.
		g_csgo.m_engine_trace->ClipRayToEntity(Ray(src, dst), MASK_SHOT, player, &new_trace);

		if (trace.m_fraction > new_trace.m_fraction)
			trace = new_trace;
	}
}

bool c_auto_wall::fire_bullet(Weapon* wpn,
	const vec3_t& direction,
	bool& visible,
	float& cur_dmg,
	int& remaining_pen,
	int& hit_group,
	int& hitbox,
	Player* entity,
	float length,
	const vec3_t& pos,
	bool strict,
	int hitgroup) {

	constexpr float pen_dist = 3000.f;
	static ConVar* dmg_reduction_bullets = g_csgo.m_cvar->FindVar(HASH("ff_damage_reduction_bullets"));
	static ConVar* dmg_bullet_pen = g_csgo.m_cvar->FindVar(HASH("ff_damage_bullet_penetration"));

	CTraceFilterSkipTwoEntities_game filter{};
	if (!wpn)
		return false;

	WeaponInfo* wpn_data = wpn->GetWpnData();
	if (!wpn_data)
		return false;

	CGameTrace enter_trace;
	cur_dmg = static_cast<float>(wpn_data->m_damage);

	vec3_t eye_pos = pos;
	float cur_dist = 0.0f;
	float max_range = wpn_data->m_range;
	const float pen_power = wpn_data->m_penetration;
	int possible_hit_remain = 4;
	remaining_pen = 4;

	filter.SetPassEntity(g_cl.m_local);
	filter.SetPassEntity2(nullptr);

	while (cur_dmg > 0.f)
	{
		max_range -= cur_dist;
		vec3_t end = eye_pos + direction * max_range;

		g_csgo.m_engine_trace->TraceRay(Ray(eye_pos, end), MASK_SHOT, (ITraceFilter*)&filter, &enter_trace);

		if (entity)
			ClipTraceToPlayer(eye_pos,
				end + (direction * 40.f),
				enter_trace,
				entity,
				filter.m_shouldhit_check_fn);
		else
			game::UTIL_ClipTraceToPlayers(eye_pos,
				end + direction * 40.f,
				MASK_SHOT,
				(ITraceFilter*)&filter,
				&enter_trace,
				60.f);

		surfacedata_t* enter_surf_data = g_csgo.m_phys_props->GetSurfaceData(enter_trace.m_surface.m_surface_props);
		const float enter_surf_pen_mod = enter_surf_data->m_game.m_penetration_modifier;
		const uint16_t enter_mat = enter_surf_data->m_game.m_material;

		if (enter_trace.m_fraction == 1.0f)
			break;

		cur_dist += enter_trace.m_fraction * max_range;
		cur_dmg *= std::powf(wpn_data->m_range_modifier, cur_dist / 500.f);

		Player* hit_player = enter_trace.m_entity->as< Player* >();
		const bool valid_player = entity && entity->IsPlayer() && hit_player->index() == entity->index();
		if ((hit_player && hit_player->IsPlayer()) && valid_player && (!strict || hitgroup == enter_trace.m_hitgroup)) {

			const bool can_do_dmg = enter_trace.m_hitgroup <= HITGROUP_RIGHTLEG;
			const bool is_enemy = hit_player->m_iTeamNum() != g_cl.m_local->m_iTeamNum();

			if (can_do_dmg && is_enemy) {
				scale_dmg(hit_player, enter_trace, wpn_data, cur_dmg, enter_trace.m_hitgroup);
				hitbox = enter_trace.m_hitbox;
				hit_group = enter_trace.m_hitgroup;
				return true;
			}
		}

		if (cur_dist > pen_dist && (wpn_data->m_penetration > 0.f || enter_surf_pen_mod < 0.1f)) {
			possible_hit_remain = remaining_pen = 0;
			break;
		}

		if (!possible_hit_remain) {
			remaining_pen = 0;
			break;
		}

		if (!handle_bullet_penetration(wpn_data, enter_trace, eye_pos, direction,
			possible_hit_remain, cur_dmg, pen_power,
			dmg_reduction_bullets->GetFloat(), dmg_bullet_pen->GetFloat())) {
			remaining_pen = possible_hit_remain;
			break;
		}

		remaining_pen = possible_hit_remain;
		visible = false;
	}

	return false;
}

bool c_auto_wall::trace_to_exit_short(vec3_t& point, vec3_t& dir, const float step_size, float max_distance)
{
	float flDistance = 0;

	while (flDistance <= max_distance)
	{
		flDistance += step_size;

		point += dir * flDistance;
		int point_contents = g_csgo.m_engine_trace->GetPointContents(point, MASK_SHOT_HULL);
		if (!(point_contents & MASK_SHOT_HULL))
		{
			// found first free point
			return true;
		}
	}

	return false;
}

float c_auto_wall::get_thickness(vec3_t& start, vec3_t& end, float distance) {
	vec3_t dir = end - start;
	vec3_t step = start;
	if (dir.length() > distance && distance != -1)
		return -1;
	dir.normalize_in_place();
	CTraceFilterWorldOnly filter;
	CGameTrace trace;
	Ray ray;
	float thickness = 0;
	while (true) {
		g_csgo.m_engine_trace->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);

		if (!trace.hit())
			return thickness;

		const vec3_t lastStep = trace.m_endpos;
		step = trace.m_endpos;

		if ((end - start).length() <= (step - start).length())
			break;


		if (!trace_to_exit_short(step, dir, 5, 90))
			return FLT_MAX;

		thickness += (step - lastStep).length();
	}
	return thickness;
}
