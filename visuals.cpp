#include "includes.h"

Visuals g_visuals{ };;

void Visuals::ModulateWorld() {
	std::vector< IMaterial* > world, props;

	// iterate material handles.
	for (uint16_t h{ g_csgo.m_material_system->FirstMaterial() }; h != g_csgo.m_material_system->InvalidMaterial(); h = g_csgo.m_material_system->NextMaterial(h)) {
		// get material from handle.
		IMaterial* mat = g_csgo.m_material_system->GetMaterial(h);
		if (!mat)
			continue;

		// store world materials.
		if (FNV1a::get(mat->GetTextureGroupName()) == HASH("World textures"))
			world.push_back(mat);

		// store props.
		else if (FNV1a::get(mat->GetTextureGroupName()) == HASH("StaticProp textures"))
			props.push_back(mat);
	}

	if (g_menu.main.visuals.world.get(0)) {
		float nightamt = g_menu.main.visuals.nightamt.get() / 100.0f;
		float purpleamt = g_menu.main.visuals.purpleamt.get() / 100.0f;

		float red = 0.2f + (nightamt - 0.5f) * 0.4f;
		float green = 0.2f + (nightamt - 0.5f) * 0.4f;
		float blue = 0.22f + (nightamt - 0.5f) * 0.44f;

		float purpleAdjustmentRed = 0.02f * purpleamt;
		float purpleAdjustmentBlue = 0.03f * purpleamt;

		red = std::min(1.0f, red + purpleAdjustmentRed);
		blue = std::min(1.0f, blue + purpleAdjustmentBlue);

		for (const auto& w : world)
			w->ColorModulate(red, green, blue);

		for (const auto& p : props)
			p->ColorModulate(0.5f, 0.5f, 0.5f);

		g_csgo.LoadNamedSky(XOR("sky_csgo_night02"));
	}

	else {

		for (const auto& w : world)
			w->ColorModulate(1.f, 1.f, 1.f);

		for (const auto& p : props)
			p->ColorModulate(1.f, 1.f, 1.f);

		g_csgo.LoadNamedSky(g_csgo.sv_skyname->GetString());
	}

	static const auto fog_override = g_csgo.m_cvar->FindVar(HASH("fog_override"));
	fog_override->SetValue(g_menu.main.visuals.world.get(4) ? 1 : 0);
	static const auto fog_start = g_csgo.m_cvar->FindVar(HASH("fog_start"));
	fog_start->SetValue(g_menu.main.visuals.fog_start.get());
	static const auto fog_end = g_csgo.m_cvar->FindVar(HASH("fog_end"));
	fog_start->SetValue(g_menu.main.visuals.fog_end.get());
	static const auto fog_color = g_csgo.m_cvar->FindVar(HASH("fog_color"));
	fog_color->SetValue(std::string(std::to_string(g_menu.main.visuals.fog_color.get().r()) + " " + std::to_string(g_menu.main.visuals.fog_color.get().g()) + " " + std::to_string(g_menu.main.visuals.fog_color.get().b())).c_str());
	static const auto fog_destiny = g_csgo.m_cvar->FindVar(HASH("fog_maxdensity"));
	fog_destiny->SetValue(g_menu.main.visuals.fog_color.get().a());


	// transparent props.
	float alpha2 = g_menu.main.visuals.walls_amount.get() / 100;
	for (const auto& w : world)
		w->AlphaModulate(alpha2);

	float alpha = g_menu.main.visuals.transparent_props_amount.get() / 100;
	for (const auto& p : props)
		p->AlphaModulate(alpha);

	if (g_csgo.r_DrawSpecificStaticProp->GetInt() != 0) {
		g_csgo.r_DrawSpecificStaticProp->SetValue(0);
	}
}


void Visuals::ThirdpersonThink() {
	ang_t                          offset;
	vec3_t                         origin, forward;
	static CTraceFilterSimple_game filter{ };
	CGameTrace                     tr;

	// for whatever reason overrideview also gets called from the main menu.
	if (!g_csgo.m_engine->IsInGame())
		return;

	// check if we have a local player and he is alive.
	bool alive = g_cl.m_local && g_cl.m_local->alive();

	// camera should be in thirdperson.
	if (m_thirdperson) {

		// if alive and not in thirdperson already switch to thirdperson.
		if (alive && !g_csgo.m_input->CAM_IsThirdPerson())
			g_csgo.m_input->CAM_ToThirdPerson();

		// if dead and spectating in firstperson switch to thirdperson.
		else if (g_cl.m_local->m_iObserverMode() == 4) {

			// if in thirdperson, switch to firstperson.
			// we need to disable thirdperson to spectate properly.
			if (g_csgo.m_input->CAM_IsThirdPerson()) {
				g_csgo.m_input->CAM_ToFirstPerson();
				g_csgo.m_input->m_camera_offset.z = 0.f;
			}

			g_cl.m_local->m_iObserverMode() = 5;
		}
	}

	// camera should be in firstperson.
	else if (g_csgo.m_input->CAM_IsThirdPerson()) {
		g_csgo.m_input->CAM_ToFirstPerson();
		g_csgo.m_input->m_camera_offset.z = 0.f;
	}

	// if after all of this we are still in thirdperson.
	if (g_csgo.m_input->CAM_IsThirdPerson()) {
		// get camera angles.
		g_csgo.m_engine->GetViewAngles(offset);

		// get our viewangle's forward directional vector.
		math::AngleVectors(offset, &forward);

		// cam_idealdist convar.
		offset.z = g_csgo.m_cvar->FindVar(HASH("cam_idealdist"))->GetFloat();

		// start pos.
		origin = g_cl.m_shoot_pos;

		// setup trace filter and trace.
		filter.SetPassEntity(g_cl.m_local);

		g_csgo.m_engine_trace->TraceRay(
			Ray(origin, origin - (forward * offset.z), { -16.f, -16.f, -16.f }, { 16.f, 16.f, 16.f }),
			MASK_NPCWORLDSTATIC,
			(ITraceFilter*)&filter,
			&tr
		);

		// adapt distance to travel time.
		math::clamp(tr.m_fraction, 0.f, 1.f);
		offset.z *= tr.m_fraction;

		// override camera angles.
		g_csgo.m_input->m_camera_offset = { offset.x, offset.y, offset.z };
	}
}

void Visuals::Hitmarker() {
	static auto cross = g_csgo.m_cvar->FindVar(HASH("weapon_debug_spread_show"));
	cross->SetValue(g_menu.main.visuals.force_xhair.get() && !g_cl.m_local->m_bIsScoped() ? 3 : 0); // force crosshair
	constexpr int length = 8, gap = 4;

	bool dynamic = (g_menu.main.misc.hitmarker.get(2) && g_menu.main.misc.hitmarker.get(0));

	if (!g_menu.main.misc.hitmarker.get(0))
		return;

	if (g_csgo.m_globals->m_curtime > m_hit_end)
		return;

	if (m_hit_duration <= 0.f)
		return;

	float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
	int x = g_cl.m_width,
		y = g_cl.m_height,
		alpha = (1.f - complete) * 240;

	Color hitcolor;


	if (dynamic) {
		hitcolor = { m_hit_color.r(), m_hit_color.g(), m_hit_color.b(), alpha };
	}
	else {
		hitcolor = { 200, 200, 200, alpha };
	}

	constexpr int line{ 4 };

	render::line(x / 2 - length, y / 2 - length, x / 2 - gap, y / 2 - gap, hitcolor);
	render::line(x / 2 - length, y / 2 + length, x / 2 - gap, y / 2 + gap, hitcolor);
	render::line(x / 2 + length, y / 2 + length, x / 2 + gap, y / 2 + gap, hitcolor);
	render::line(x / 2 + length, y / 2 - length, x / 2 + gap, y / 2 - gap, hitcolor);
}

void Visuals::HitmarkerWorld() {

	constexpr int line_size{ 4 };
	constexpr int gap{ 4 };

	bool dynamic = (g_menu.main.misc.hitmarker.get(2) && g_menu.main.misc.dynamic_col.get(1));

	if (g_shots.m_hitmarkers.empty() || !g_menu.main.misc.hitmarker.get(1))
		return;

	for (int i = 0; i < g_shots.m_hitmarkers.size(); i++) {

		HitmarkerWorld_t& hitmarker = g_shots.m_hitmarkers[i];
		vec2_t screen_pos;

		if (std::abs(g_csgo.m_globals->m_realtime - hitmarker.m_time) > 1.f)
			hitmarker.m_alpha -= 1;

		if (hitmarker.m_alpha <= 0.f) {
			g_shots.m_hitmarkers.erase(g_shots.m_hitmarkers.begin() + i);
			continue;
		}

		vec3_t pos3D = vec3_t(g_shots.m_hitmarkers[i].m_pos.x, g_shots.m_hitmarkers[i].m_pos.y, g_shots.m_hitmarkers[i].m_pos.z);

		if (!render::WorldToScreen(pos3D, screen_pos))
			continue;

		int x = screen_pos.x, y = screen_pos.y;

		int alpha = static_cast<int>(hitmarker.m_alpha);

		Color hitcolor;


		if (dynamic) {
			hitcolor = { m_hit_color.r(), m_hit_color.g(), m_hit_color.b(), alpha };
		}
		else {
			hitcolor = { 200, 200, 200, alpha };
		}


		/* bottom left */
		const vec2_t bottom_left_start(x - gap, y + gap);
		const vec2_t bottom_left_end(bottom_left_start.x - line_size, bottom_left_start.y + line_size);
		render::line(bottom_left_end.x, bottom_left_end.y, bottom_left_start.x, bottom_left_start.y, hitcolor);

		/* top left */
		const vec2_t top_left_start(x - gap, y - gap);
		const vec2_t top_left_end(top_left_start.x - line_size, top_left_start.y - line_size);
		render::line(top_left_end.x, top_left_end.y, top_left_start.x, top_left_start.y, hitcolor);

		/* top right */
		const vec2_t top_right_start(x + gap, y - gap);
		const vec2_t top_right_end(top_right_start.x + line_size, top_right_start.y - line_size);
		render::line(top_right_end.x, top_right_end.y, top_right_start.x, top_right_start.y, hitcolor);

		/* bottom right */
		const vec2_t bottom_right_start(x + gap, y + gap);
		const vec2_t bottom_right_end(bottom_right_start.x + line_size, bottom_right_start.y + line_size);
		render::line(bottom_right_end.x, bottom_right_end.y, bottom_right_start.x, bottom_right_start.y, hitcolor);
	}
}

void Visuals::DmgIndicator() {
	bool dynamic = (g_menu.main.misc.hitmarker.get(3));

	if (!g_menu.main.misc.hitmarker.get(2))
		return;

	if (g_csgo.m_globals->m_curtime > m_hit_end)
		return;

	if (m_hit_duration <= 0.f)
		return;

	float complete = (g_csgo.m_globals->m_curtime - m_hit_start) / m_hit_duration;
	int x = g_cl.m_width / 2;
	int alpha = (1.f - complete) * 240;

	Color hitcolor;

	if (dynamic) {
		hitcolor = { m_hit_color.r(), m_hit_color.g(), m_hit_color.b(), alpha };
	}
	else {
		hitcolor = { 200, 200, 200, alpha };
	}

	static float flyUpY = 0.0f;

}

void Visuals::NoSmoke() {

	if (!smoke1)
		smoke1 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_fire"), XOR("Other textures"));

	if (!smoke2)
		smoke2 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_smokegrenade"), XOR("Other textures"));

	if (!smoke3)
		smoke3 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_emods"), XOR("Other textures"));

	if (!smoke4)
		smoke4 = g_csgo.m_material_system->FindMaterial(XOR("particle/vistasmokev1/vistasmokev1_emods_impactdust"), XOR("Other textures"));

	if (g_menu.main.visuals.removals.get(1)) {
		if (!smoke1->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke1->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke2->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke2->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke3->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke3->SetFlag(MATERIAL_VAR_NO_DRAW, true);

		if (!smoke4->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke4->SetFlag(MATERIAL_VAR_NO_DRAW, true);
	}

	else {
		if (smoke1->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke1->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke2->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke2->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke3->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke3->SetFlag(MATERIAL_VAR_NO_DRAW, false);

		if (smoke4->GetFlag(MATERIAL_VAR_NO_DRAW))
			smoke4->SetFlag(MATERIAL_VAR_NO_DRAW, false);
	}

	// godbless alpha and led for adding post process removal to RemoveSmoke.
	// 
	// nitro code (alt to forcing cvar etc)
	static auto DisablePostProcess = g_csgo.postproc;

	// get post process address
	static bool* disable_post_process = *reinterpret_cast<bool**>(DisablePostProcess);

	// set it.
	if (*disable_post_process != g_menu.main.visuals.postprocess.get())
		*disable_post_process = g_menu.main.visuals.postprocess.get();
}

bool Visuals::ForceDormant(Player* pPlayer)
{
	float flTimeInDormant = g_DormantSystem->GetTimeInDormant(pPlayer->index());
	float flSinceLastData = g_DormantSystem->GetTimeSinceLastData(pPlayer->index());

	if (flTimeInDormant < flSinceLastData)
	{
		if (flTimeInDormant < 8.0f)
			pPlayer->SetAbsOrigin(g_DormantSystem->GetLastNetworkOrigin(pPlayer->index()));
		else if (flSinceLastData < 8.0f)
			pPlayer->SetAbsOrigin(g_DormantSystem->GetLastDormantOrigin(pPlayer->index()));
	}
	else
	{
		if (flSinceLastData < 8.0f)
			pPlayer->SetAbsOrigin(g_DormantSystem->GetLastDormantOrigin(pPlayer->index()));
		else if (flTimeInDormant < 8.0f)
			pPlayer->SetAbsOrigin(g_DormantSystem->GetLastNetworkOrigin(pPlayer->index()));
	}

	return flTimeInDormant < 8.0f || flSinceLastData < 8.0f;
}

void Visuals::think() {
	// don't run anything if our local player isn't valid.
	if (!g_cl.m_local)
		return;

	if (g_menu.main.visuals.removals.get(4)
		&& g_cl.m_local->alive()
		&& g_cl.m_local->GetActiveWeapon()
		&& g_cl.m_local->GetActiveWeapon()->GetWpnData()->m_weapon_type == CSWeaponType::WEAPONTYPE_SNIPER_RIFLE
		&& g_cl.m_local->m_bIsScoped()) {

		// rebuild the original scope lines.
		int w = g_cl.m_width,
			h = g_cl.m_height,
			x = w / 2,
			y = h / 2,
			size = g_csgo.cl_crosshair_sniper_width->GetInt();

		// Here We Use The Euclidean distance To Get The Polar-Rectangular Conversion Formula.
		if (size > 1) {
			x -= (size / 2);
			y -= (size / 2);
		}

		// draw our lines.
		render::rect_filled(0, y, w, size, colors::black);
		render::rect_filled(x, 0, size, h, colors::black);
	}

	for (auto iPlayerIndex = 1; iPlayerIndex < g_csgo.m_globals->m_max_clients; iPlayerIndex++) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(iPlayerIndex);

		if (!player || !player->alive() || !player->IsPlayer())
		{
			if (player && player->IsPlayer())
				g_DormantSystem->ResetPlayer(player);

			continue;
		}

		if (player->m_bIsLocalPlayer())
			continue;

		//if (c_playerlist::get()->should_disable_visuals(player))
			//continue;

		if (player->dormant())
		{
			if (!ForceDormant(player))
				continue;
		}
		else
		{
			g_DormantSystem->ResetPlayer(player);
		}


		DrawPlayer(player);
		//g_sound.Finish( );
	}

	// draw esp on ents.
	for (int i{ 1 }; i <= g_csgo.m_entlist->GetHighestEntityIndex(); ++i) {
		Entity* ent = g_csgo.m_entlist->GetClientEntity(i);
		if (!ent)
			continue;

		draw(ent);
	}

	// draw everything else.
	StatusIndicators();
	PenetrationCrosshair();
	Spectators();
	ImpactData();
	ManualAntiAim();
	Hitmarker();
	HitmarkerWorld();
	DrawPlantedC4();
	DmgIndicator();
}

void Visuals::Spectators() {
	if (!g_menu.main.visuals.spectators.get())
		return;

	std::vector< std::string > spectators{ XOR("spectators") };
	int h = render::menu_shade.m_size.m_height;

	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player)
			continue;

		if (player->m_bIsLocalPlayer())
			continue;

		if (player->dormant())
			continue;

		if (player->alive())
			continue;

		if (player->GetObserverTarget() != g_cl.m_local)
			continue;

		player_info_t info;
		if (!g_csgo.m_engine->GetPlayerInfo(i, &info))
			continue;

		spectators.push_back(std::string(info.m_name).substr(0, 24));
	}

	size_t total_size = spectators.size() * (h - 1);

	for (size_t i{ }; i < spectators.size(); ++i) {
		const std::string& name = spectators[i];

		render::menu_shade.string(g_cl.m_width - 20, (g_cl.m_height / 2) - (total_size / 2) + (i * (h - 1)),
			{ 255, 255, 255, 179 }, name, render::ALIGN_RIGHT);
	}
}

Color LerpRGB(Color a, Color b, float t)
{
	return Color
	(
		a.r() + (b.r() - a.r()) * t,
		a.g() + (b.g() - a.g()) * t,
		a.b() + (b.b() - a.b()) * t,
		a.a() + (b.a() - a.a()) * t
	);
}

void Visuals::StatusIndicators() {
	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	struct Indicator_t { Color color; std::string text; };
	std::vector< Indicator_t > indicators{ };

	// LBY
	if (g_menu.main.visuals.indicators.get(0)) {
		// get the absolute change between current lby and animated angle.
		float change = std::abs(math::AngleDiff(g_cl.m_abs_yaw, g_cl.m_real_angle.y));

		Indicator_t ind{ };
		ind.color = change > 35.f ? 0xff15c27b : 0xff0000ff;
		ind.text = XOR("LBY");
		indicators.push_back(ind);
	}

	// LC
	if (g_menu.main.visuals.indicators.get(1)) {
		if (g_cl.m_local->m_vecVelocity().length_2d() > 270.f || g_cl.m_lagcomp) {
			Indicator_t ind{ };
			ind.color = g_cl.m_lagcomp ? 0xff15c27b : 0xff0000ff;
			ind.text = XOR("LC");

			indicators.push_back(ind);
		}
	}

	if (g_menu.main.visuals.indicators.get(2) && g_menu.main.misc.ping_spike.get()) {
		Indicator_t ind{};
		Color Ping_Transition = Color(255, 255, 255, 210);
		Color Ting_Pransition = 0xff15c27b;
		float PatrickDog = 0.012f;

		static float transitionProgress = 0.0f;

		if (g_aimbot.m_fake_latency2) {
			transitionProgress += PatrickDog;
			if (transitionProgress > 1.0f)
				transitionProgress = 1.0f;
		}
		else {
			transitionProgress -= PatrickDog;
			if (transitionProgress < 0.0f)
				transitionProgress = 0.0f;
		}

		ind.color = Color(
			Ping_Transition.r() + std::round((Ting_Pransition.r() - Ping_Transition.r()) * transitionProgress),
			Ping_Transition.g() + std::round((Ting_Pransition.g() - Ping_Transition.g()) * transitionProgress),
			Ping_Transition.b() + std::round((Ting_Pransition.b() - Ping_Transition.b()) * transitionProgress),
			Ping_Transition.a() + std::round((Ting_Pransition.a() - Ping_Transition.a()) * transitionProgress)
		);

		ind.text = XOR("PING");
		indicators.push_back(ind);
	}

	// DMG
	if (g_menu.main.visuals.indicators.get(3) && g_aimbot.m_damage_toggle) {
		Indicator_t ind{ };
		ind.color = Color(255, 255, 255, 210);
		ind.text = XOR("DMG");
		indicators.push_back(ind);
	}

	if (g_menu.main.visuals.indicators.get(4) && g_aimbot.m_force_body) {
		Indicator_t ind{ };
		ind.color = Color(255, 255, 255, 210);
		ind.text = XOR("BODY");
		indicators.push_back(ind);
	}

	if (g_menu.main.visuals.indicators.get(5) && g_aimbot.m_override) {
		Indicator_t ind{ };
		ind.color = Color(255, 255, 255, 210);
		ind.text = XOR("OVERRIDE");
		indicators.push_back(ind);
	}

	if (g_menu.main.visuals.indicators.get(6) && g_cl.m_local->m_flVelocityModifier() < 1) {
		Indicator_t ind{ };
		float alpha = g_cl.m_local->m_flVelocityModifier();
		ind.color = Color(255, 255, 255, 100 * alpha);
		ind.text = XOR("SLOW");
		indicators.push_back(ind);
	}

	//printf("%f\n", g_cl.m_local->m_flVelocityModifier());

	if (indicators.empty())
		return;

	// iterate and draw indicators.
	for (size_t i{ }; i < indicators.size(); ++i) {
		auto& indicator = indicators[i];

		render::indicator.string(20, g_cl.m_height - 80 - (30 * i), indicator.color, indicator.text);
	}
}

void Visuals::ImpactData() {
	if (!g_cl.m_processing) return;

	if (!g_menu.main.visuals.bullet_impacts.get()) return;

	static auto last_count = 0;
	auto& client_impact_list = *(CUtlVector< ClientHitVerify_T >*)((uintptr_t)g_cl.m_local + 0xBA84);

	for (auto i = client_impact_list.Count(); i > last_count; i--)
	{
		g_csgo.m_debug_overlay->AddBoxOverlay(client_impact_list[i - 1].pos, vec3_t(-2, -2, -2), vec3_t(2, 2, 2), ang_t(0, 0, 0), 255, 0, 0, 125, 4.f);
	}

	if (client_impact_list.Count() != last_count)
		last_count = client_impact_list.Count();
}

void RenderPolygonTapalation(float x, float y, float size, float rotation, Color color) {
	Vertex verts[3], verts2[3];


	verts[0] = { x, y };        // 0,  0
	verts[1] = { x, y + (18.f * size / 8.f) }; // -1, 1
	verts[2] = { x + (14.f * size / 8.f), y + (24.f * size / 8.f) }; // 1,  1

	verts2[0] = { x, y };        // 0,  0
	verts2[1] = { x, y + (18.f * size / 8.f) }; // -1, 1
	verts2[2] = { x - (14.f * size / 8.f), y + (24.f * size / 8.f) }; // 1,  1

	verts[0] = render::RotateVertex(vec2_t(x, y), verts[0], rotation * 90);
	verts[1] = render::RotateVertex(vec2_t(x, y), verts[1], rotation * 90);
	verts[2] = render::RotateVertex(vec2_t(x, y), verts[2], rotation * 90);

	verts2[0] = render::RotateVertex(vec2_t(x, y), verts2[0], rotation * 90);
	verts2[1] = render::RotateVertex(vec2_t(x, y), verts2[1], rotation * 90);
	verts2[2] = render::RotateVertex(vec2_t(x, y), verts2[2], rotation * 90);

	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawTexturedPolygon(3, verts);
	g_csgo.m_surface->DrawTexturedPolygon(3, verts2);

}

void RenderPolygonSup(float x, float y, float rotation, Color color) {

	Vertex verts[3];

	//ignore SGHUIT CODE i was lazy affffffffffff idgaf abt i looking good
	float size = 0.8f;

	verts[0] = { x, y };        // 0,  0
	verts[1] = { x - (12.f * size), y + (24.f * size) }; // -1, 1
	verts[2] = { x + (12.f * size), y + (24.f * size) }; // 1,  1


	verts[0] = render::RotateVertex(vec2_t(x, y), verts[0], rotation * 90);
	verts[1] = render::RotateVertex(vec2_t(x, y), verts[1], rotation * 90);
	verts[2] = render::RotateVertex(vec2_t(x, y), verts[2], rotation * 90);


	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawTexturedPolygon(3, verts);

}

void RenderPolygonOnetap(float x, float y, float rotation, Color color, Color outline)
{
	Vertex verts[3];

	verts[0] = { x, y + 70 };
	verts[1] = { x - 10, y + 50 };
	verts[2] = { x + 10, y + 50 };

	verts[0] = render::RotateVertex(vec2_t(x, y), verts[0], rotation * 90);
	verts[1] = render::RotateVertex(vec2_t(x, y), verts[1], rotation * 90);
	verts[2] = render::RotateVertex(vec2_t(x, y), verts[2], rotation * 90);

	static int texture = g_csgo.m_surface->CreateNewTextureID(true);
	unsigned char buffer[4] = { 255, 255, 255, 255 };

	g_csgo.m_surface->DrawSetTextureRGBA(texture, buffer, 1, 1);

	g_csgo.m_surface->DrawSetColor(color);
	g_csgo.m_surface->DrawSetTexture(texture);

	g_csgo.m_surface->DrawTexturedPolygon(3, verts);

	g_csgo.m_surface->DrawSetColor(outline); // filled
	g_csgo.m_surface->DrawTexturedPolyLine(3, verts); // outline
}

void Visuals::ManualAntiAim() {
	int   x, y;

	// dont do if dead.
	if (!g_cl.m_processing)
		return;

	if (!g_menu.main.visuals.indicators.get(5))
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	Color color = g_menu.main.visuals.manual_anti_aim_col.get();


	color.a() = g_menu.main.visuals.manual_anti_aim_col.get().a();

	if (g_hvh.m_left) {
		RenderPolygonTapalation(x - 61, y + 1, 5.f, 3, Color(0, 0, 0, color.a() / 1.5));
		RenderPolygonTapalation(x - 60, y, 5.f, 3, color);
	}

	if (g_hvh.m_right) {
		RenderPolygonTapalation(x + 61, y + 1, 5.f, 1, Color(0, 0, 0, color.a() / 1.5));
		RenderPolygonTapalation(x + 60, y, 5.f, 1, color);
	}

	if (g_hvh.m_back) {
		RenderPolygonTapalation(x + 1, y + 61, 5.f, 2, Color(0, 0, 0, color.a() / 1.5));
		RenderPolygonTapalation(x, y + 60, 5.f, 2, color);
	}

	if (g_hvh.m_forward) {
		RenderPolygonTapalation(x + 1, y - 61, 5.f, 4, Color(0, 0, 0, color.a() / 1.5));
		RenderPolygonTapalation(x, y - 60, 5.f, 4, color);
	}

}

void Visuals::PenetrationCrosshair() {
	int   x, y;
	Color final_color;

	if (!g_menu.main.visuals.pen_crosshair.get()
		|| !g_cl.m_processing
		|| !g_cl.m_weapon
		|| !g_cl.m_weapon_info)
		return;

	x = g_cl.m_width / 2;
	y = g_cl.m_height / 2;

	if (g_cl.m_awall_hit)
		final_color = colors::transparent_yellow;
	else if (g_cl.m_awall_pen)
		final_color = colors::transparent_green;
	else
		final_color = colors::transparent_red;

	render::rect_filled(x - 1, y - 1, 3, 3, final_color);
}

void Visuals::draw(Entity* ent) {

	if (!g_cl.m_local || !g_csgo.m_engine->IsInGame()) {
		return;
	}

	if (ent->IsBaseCombatWeapon() && !ent->dormant())
		DrawItem(ent->as< Weapon* >());

	if (g_menu.main.visuals.proj.get())
		DrawProjectile(ent->as< Weapon* >());
}

template <typename T>
std::string to_string_with_precision(const T a_value, const int n = 3)
{
	std::ostringstream out;
	out.precision(n);
	out << std::fixed << a_value;
	return out.str();
}

void Visuals::DrawProjectile(Weapon* ent) {
	vec2_t screen;
	vec3_t origin = ent->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	Color col = g_menu.main.visuals.proj_color.get();
	col.a() = 0xb4;

	if (ent->is(HASH("CDecoyProjectile")))
		render::small_fonts.string(screen.x, screen.y, col, XOR("DECOY"), render::ALIGN_CENTER);

	else if (ent->is(HASH("CMolotovProjectile")))
		render::small_fonts.string(screen.x, screen.y, col, XOR("MOLLY"), render::ALIGN_CENTER);

	else if (ent->is(HASH("CBaseCSGrenadeProjectile"))) {
		const model_t* model = ent->GetModel();

		if (model) {
			std::string name{ ent->GetModel()->m_name };

			if (name.find(XOR("flashbang")) != std::string::npos)
				render::small_fonts.string(screen.x, screen.y, col, XOR("FLASH"), render::ALIGN_CENTER);

			else if (name.find(XOR("fraggrenade")) != std::string::npos) {
				// Grenade range.
				if (g_menu.main.visuals.spheres.get(0))
					render::sphere(origin, 20.f, 5.f, 1.f, g_menu.main.visuals.spheres_col.get());

				render::small_fonts.string(screen.x, screen.y, col, XOR("FRAG"), render::ALIGN_CENTER);
			}
		}
	}

	else if (ent->is(HASH("CInferno"))) {
		// Fire range.
		if (g_menu.main.visuals.spheres.get(2))
			render::flatSphere(origin, 150.f, 5.f, 1.f, g_menu.main.visuals.spheres_col.get());

		if (g_menu.main.visuals.spheres.get(1))
			render::sphere(origin, 20.f, 5.f, 1.f, g_menu.main.visuals.spheres_col.get());

		render::small_fonts.string(screen.x, screen.y, col, XOR("FIRE"), render::ALIGN_CENTER);
	}

	else if (ent->is(HASH("CSmokeGrenadeProjectile")))
		render::small_fonts.string(screen.x, screen.y, col, XOR("SMOKE"), render::ALIGN_CENTER);
}

void Visuals::DrawItem(Weapon* item) {
	// we only want to draw shit without owner.
	Entity* owner = g_csgo.m_entlist->GetClientEntityFromHandle(item->m_hOwnerEntity());
	if (owner)
		return;

	// is the fucker even on the screen?
	vec2_t screen;
	vec3_t origin = item->GetAbsOrigin();
	if (!render::WorldToScreen(origin, screen))
		return;

	WeaponInfo* data = item->GetWpnData();
	if (!data)
		return;

	Color col = g_menu.main.visuals.item_color.get();
	int alpha1 = g_menu.main.visuals.item_color_alpha.get();

	Color col2 = g_menu.main.visuals.bomb_col.get();
	int alpha2 = g_menu.main.visuals.bomb_col_slider.get();

	Color col3 = g_menu.main.visuals.ammo_color.get();
	int alpha3 = g_menu.main.visuals.ammo_color_alpha.get();

	std::string distance;
	int dist = (((item->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
	//if (dist > 0)
	//distance = tfm::format(XOR("%i FT"), dist);
	if (dist > 0) {
		if (dist > 5) {
			while (!(dist % 5 == 0)) {
				dist = dist - 1;
			}

			if (dist % 5 == 0)
				distance = tfm::format(XOR("%i FT"), dist);
		}
		else
			distance = tfm::format(XOR("%i FT"), dist);
	}

	// render bomb in green.
	if (g_menu.main.visuals.planted_c4.get() && item->is(HASH("CC4")))
		render::small_fonts.string(screen.x, screen.y, Color(col2.r(), col2.g(), col2.b(), alpha2), XOR("BOMB"), render::ALIGN_CENTER);

	if (item->is(HASH("CC4")))
		return;

	if (dist > 25)
		return;

	std::string name{ item->GetLocalizedName() };
	std::transform(std::execution::par, name.begin(), name.end(), name.begin(), ::toupper);

	auto font = render::small_fonts;

	if (g_menu.main.visuals.items.get())
		font.string(screen.x, screen.y, g_menu.main.visuals.item_color.get(), name, render::ALIGN_CENTER);

	if (g_menu.main.visuals.items_distance.get())
		font.string(screen.x, screen.y - 10, Color(col.r(), col.g(), col.b(), alpha1), distance, render::ALIGN_CENTER);

	if (g_menu.main.visuals.ammo.get()) {

		// nades do not have ammo.
		if (data->m_weapon_type == WEAPONTYPE_GRENADE || data->m_weapon_type == WEAPONTYPE_KNIFE)
			return;

		if (item->m_iItemDefinitionIndex() == 0 || item->m_iItemDefinitionIndex() == C4)
			return;

		std::string ammo = tfm::format(XOR("[ %i/%i ]"), item->m_iClip1(), item->m_iPrimaryReserveAmmoCount());
		std::string icon = tfm::format(XOR("%c"), m_weapon_icons[item->m_iItemDefinitionIndex()]);

		const int current = item->m_iClip1();
		const int max = data->m_max_clip1;
		const float scale = (float)current / max;
		const int width_ = font.size(name.c_str()).m_width;

		int bar = (int)std::round((width_ - 1) * scale);
		render::rect_filled(screen.x - int(width_ / 2.f), screen.y + 12, render::esp_small.size(name.c_str()).m_width + 1, 4, Color(0, 0, 0, alpha3));
		render::rect_filled(screen.x - int(width_ / 2.f) + 1, screen.y + 1 + 12, bar, 2, Color(col3.r(), col3.g(), col3.b(), alpha3));
	}
}

void Visuals::OffScreen(Player* player, int alpha) {
	vec3_t view_origin, target_pos, delta;
	vec2_t screen_pos, offscreen_pos;
	float  leeway_x, leeway_y, radius, offscreen_rotation;
	bool   is_on_screen;
	Vertex verts[3], verts_outline[3];
	Color  color;

	static auto get_offscreen_data = [](const vec3_t& delta, float radius, vec2_t& out_offscreen_pos, float& out_rotation) {
		ang_t  view_angles(g_csgo.m_view_render->m_view.m_angles);
		vec3_t fwd, right, up(0.f, 0.f, 1.f);
		float  front, side, yaw_rad, sa, ca;

		math::AngleVectors(view_angles, &fwd);

		fwd.z = 0.f;
		fwd.normalize();

		right = up.cross(fwd);
		front = delta.dot(fwd);
		side = delta.dot(right);

		out_offscreen_pos.x = radius * -side;
		out_offscreen_pos.y = radius * -front;

		out_rotation = math::rad_to_deg(std::atan2(out_offscreen_pos.x, out_offscreen_pos.y) + math::pi);

		yaw_rad = math::deg_to_rad(-out_rotation);
		sa = std::sin(yaw_rad);
		ca = std::cos(yaw_rad);

		out_offscreen_pos.x = (int)((g_cl.m_width / 2.f) + (radius * sa));
		out_offscreen_pos.y = (int)((g_cl.m_height / 2.f) - (radius * ca));
		};

	if (!g_menu.main.players.offscreen.get())
		return;

	if (!g_cl.m_processing || !g_cl.m_local->enemy(player))
		return;

	target_pos = player->WorldSpaceCenter();
	is_on_screen = render::WorldToScreen(target_pos, screen_pos);

	leeway_x = g_cl.m_width / 18.f;
	leeway_y = g_cl.m_height / 18.f;

	bool is_grenade = (player->GetActiveWeapon() && player->GetActiveWeapon()->GetWpnData()->m_weapon_type == CSWeaponType::WEAPONTYPE_GRENADE);

	Color arrow_color = is_grenade ? g_menu.main.players.proj_offscreen_color.get() : g_menu.main.players.offscreen_color.get();

	if (!is_on_screen || screen_pos.x < -leeway_x || screen_pos.x >(g_cl.m_width + leeway_x) ||
		screen_pos.y < -leeway_y || screen_pos.y >(g_cl.m_height + leeway_y)) {

		view_origin = g_csgo.m_view_render->m_view.m_origin;
		delta = (target_pos - view_origin).normalized();
		radius = 200.f * (g_cl.m_height / 480.f);

		get_offscreen_data(delta, radius, offscreen_pos, offscreen_rotation);
		offscreen_rotation = -offscreen_rotation;

		verts[0] = { offscreen_pos.x, offscreen_pos.y };
		verts[1] = { offscreen_pos.x - 12.f, offscreen_pos.y + 24.f };
		verts[2] = { offscreen_pos.x + 12.f, offscreen_pos.y + 24.f };

		verts_outline[0] = { verts[0].m_pos.x - 1.f, verts[0].m_pos.y - 1.f };
		verts_outline[1] = { verts[1].m_pos.x - 1.f, verts[1].m_pos.y + 1.f };
		verts_outline[2] = { verts[2].m_pos.x + 1.f, verts[2].m_pos.y + 1.f };

		verts[0] = render::RotateVertex(offscreen_pos, verts[0], offscreen_rotation);
		verts[1] = render::RotateVertex(offscreen_pos, verts[1], offscreen_rotation);
		verts[2] = render::RotateVertex(offscreen_pos, verts[2], offscreen_rotation);

		color = arrow_color;
		color.a() = (alpha == 255) ? alpha : alpha / 2;

		g_csgo.m_surface->DrawSetColor(color);
		g_csgo.m_surface->DrawTexturedPolygon(3, verts);
	}
}

std::string Visuals::GetWeaponIcon(const int id) {
	auto search = m_weapon_icons.find(id);
	if (search != m_weapon_icons.end())
		return std::string(&search->second, 1);

	return XOR("");
}

void Visuals::DrawPlayer(Player* player) {


	Rect		  box;
	player_info_t info;
	Color		  color;

	std::string distance;
	int dist = (((player->m_vecOrigin() - g_cl.m_local->m_vecOrigin()).length_sqr()) * 0.0625) * 0.001;
	int  distance1337{ 0 };
	if (dist < 0)
		distance1337 = 0;

	if (dist > 0) {
		distance1337 = 9 + 0;
		if (dist > 5) {
			while (!(dist % 5 == 0)) {
				dist = dist - 1;
			}

			if (dist % 5 == 0)
				distance = tfm::format(XOR("%i FT"), dist);
		}
		else
			distance = tfm::format(XOR("%i FT"), dist);
	}

	// get player index.
	int index = player->index();

	// get reference to array variable.
	float& opacity = m_opacities[index - 1];
	bool& draw = m_draw[index - 1];

	// opacity should reach 1 in 300 milliseconds.
	constexpr int frequency = 1.f / 0.3f;

	// the increment / decrement per frame.
	float step = frequency * g_csgo.m_globals->m_frametime;

	// is player enemy.
	bool enemy = player->enemy(g_cl.m_local);
	bool dormant = player->dormant();

	if (g_menu.main.players.enemy_radar.get() && enemy && !dormant)
		player->m_bSpotted() = true;

	// we can draw this player again.
	//if (!dormant)
	draw = true;

	if (!draw)
		return;

	// if non-dormant	-> increment
	// if dormant		-> decrement
	dormant ? opacity -= step : opacity += step;

	// is dormant esp enabled for this player.
	bool dormant_esp = enemy && g_menu.main.players.dormant.get();

	// clamp the opacity.
	math::clamp(opacity, 0.f, 1.f);
	if (!opacity && !dormant_esp)
		return;

	// stay for x seconds max.


	// calculate alpha channels.
	int alpha = (int)(255.f * opacity);
	int low_alpha = (int)(179.f * opacity);
	int low_alpha_text = (int)(179.f * opacity);
	int low_alpha2 = (int)(179.f * opacity);

	// get color based on enemy or not.
	color = enemy ? g_menu.main.players.box_enemy.get() : g_menu.main.players.box_enemy.get();

	// nigger kurwa >_< :3 uwu
	constexpr float MAX_DORMANT_TIME = 20.f;
	constexpr float DORMANT_FADE_TIME = MAX_DORMANT_TIME / 2.f;
	constexpr float DORMANT_INACC = 5.f;

	float dt = g_csgo.m_globals->m_curtime - player->m_flSimulationTime();
	if (dormant && g_DormantSystem->GetTimeSinceLastData(player->index()) > MAX_DORMANT_TIME)
		return;

	if (dormant && dormant_esp) {
		alpha = 112;
		low_alpha = 80;
		low_alpha_text = 80;
		low_alpha2 = 80;

		//// fade.
		if (g_DormantSystem->GetTimeSinceLastData(player->index()) > DORMANT_INACC) {
			// for how long have we been fading?
			//float faded = (dt - DORMANT_INACC);
			//float scale = 1.f - (faded / DORMANT_INACC);

			// alpha *= scale;
			low_alpha_text = 0;
			// low_alpha2 *= scale;
		}

		// override color.
		color = { 112, 112, 112 };
	}

	// override alpha.
	color.a() = alpha;

	// get player info.
	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

	// run offscreen ESP.
	OffScreen(player, alpha);

	// attempt to get player box.
	if (!GetPlayerBoxRect(player, box)) {
		// OffScreen( player );
		return;
	}

	const bool bone_esp = (enemy && g_menu.main.players.skeleton.get(0));
	const bool history_bone_esp = (enemy && g_menu.main.players.skeleton.get(1));

	if (bone_esp)
		DrawSkeleton(player, alpha);

	if (history_bone_esp)
		DrawHistorySkeleton(player, alpha);

	// DebugAimbotPoints( player );

	// is box esp enabled for this player.
	const bool box_esp = (enemy && g_menu.main.players.box.get());

	// render box if specified.
	if (box_esp) {
		if (dormant)
			render::rect_outlined(box.x, box.y, box.w, box.h, Color(g_menu.main.players.box_enemy.get().r(), g_menu.main.players.box_enemy.get().g(), g_menu.main.players.box_enemy.get().b(), alpha), { 0,0,0, low_alpha });
		else
			render::rect_outlined(box.x, box.y, box.w, box.h, Color(g_menu.main.players.box_enemy.get().r(), g_menu.main.players.box_enemy.get().g(), g_menu.main.players.box_enemy.get().b(), g_menu.main.players.box_enemy.get().a()), Color(0, 0, 0, g_menu.main.players.box_enemy.get()).a());
	}

	// is name esp enabled for this player.
	const bool name_esp = (enemy && g_menu.main.players.name.get()) || (!enemy && g_menu.main.players.teammates.get());

	// draw name esp
	if (name_esp) {
		// fix retards with their namechange meme 
		// the point of this is overflowing unicode compares with hardcoded buffers, good hvh strat
		std::string name{ std::string(info.m_name).substr(0, 12) };

		Color clr = g_menu.main.players.name_color.get();
		if (dormant)
			clr.a() = low_alpha;
		else
			clr.a() = g_menu.main.players.name_color.get().a();

		render::verdana.string(box.x + box.w / 2, box.y - render::verdana.m_size.m_height, clr, name, render::ALIGN_CENTER);

	}

	// is health esp enabled for this player.
	const bool health_esp = (enemy && g_menu.main.players.health.get());

	if (health_esp) {
		int y = box.y + 1;
		int h = box.h - 2;

		// retarded servers that go above 100 hp..
		int hp = std::min(100, player->m_iHealth());

		// calculate hp bar color.
		int r = std::min((510 * (100 - hp)) / 100, 255);
		int g = std::min((510 * hp) / 100, 255);
		int b = 100.f;

		// get hp bar height.
		int fill = (int)std::round(hp * h / 100.f);

		if (g_menu.main.players.override_health.get()) {
			r = g_menu.main.players.health_color.get().r();
			g = g_menu.main.players.health_color.get().g();
			b = g_menu.main.players.health_color.get().b();
		}

		// render background.
		render::rect_filled(box.x - 6, y - 1, 4, h + 2, { 10, 10, 10, low_alpha });

		// render actual bar.
		render::rect(box.x - 5, y + h - fill, 2, fill, { r, g, b, alpha });

		// if hp is below max, draw a string.
		if (hp < 90)
			render::small_fonts.string(box.x - 5, y + (h - fill) - 5, { 255, 255, 255, low_alpha }, std::to_string(hp), render::ALIGN_CENTER);
	}

	// draw flags.
	{

		int hp = std::min(100, player->m_iHealth());
		AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
		std::vector< std::pair< std::string, Color > > flags;

		auto items = enemy ? g_menu.main.players.flags_enemy.GetActiveIndices() : g_menu.main.players.flags_friendly.GetActiveIndices();

		// NOTE FROM NITRO TO DEX -> stop removing my iterator loops, i do it so i dont have to check the size of the vector
		// with range loops u do that to do that.
		for (auto it = items.begin(); it != items.end(); ++it) {

			if (player->alive() && enemy && !dormant) {
				// money.
				if (*it == 0)
					flags.push_back({ tfm::format(XOR("$%i"), player->m_iAccount()), { 155, 210, 100, low_alpha_text } });

				// reload.
				if (*it == 1) {
					// get ptr to layer 1.
					C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

					// check if reload animation is going on.
					if (layer1->weight != 0.f && player->GetSequenceActivity(layer1->sequence) == 967 /* ACT_CSGO_RELOAD */)
						flags.push_back({ XOR("RELOAD"), {  0, 175, 255, low_alpha_text } });
				}

				// armor.
				if (*it == 2) {
					if (player->m_ArmorValue() > 0) {
						if (player->m_bHasHelmet())
							flags.push_back({ XOR("HK"), {  255, 255, 255, low_alpha_text } });
						else
							flags.push_back({ XOR("K"), {  255, 255, 255, low_alpha_text } });
					}
				}

				// flashed.
				if (*it == 3)
					if (player->m_flFlashBangTime() > 0.f)
						flags.push_back({ XOR("BLIND"), Color(255, 255, 0, low_alpha_text) });

				// scoped.
				if (*it == 4)
					if (player->m_bIsScoped())
						flags.push_back({ XOR("SCOPED"), {  0, 175, 255, low_alpha_text } });

				// bomb.
				if (*it == 5)
					if (player->HasC4())
						flags.push_back({ XOR("BOMB"), { 255, 0, 0, low_alpha_text } });

				// fake angles.
				if (*it == 6) {
					if (data && data->m_records.size() && !dormant) {
						LagRecord* current = data->m_records.front().get();
						Color clr;
						clr = { 255, 255, 255 };
						if (!game::IsFakePlayer(current->m_player->index())
							&& current->m_body != current->m_eye_angles.y) {
							flags.push_back({ XOR("FAKE"), { clr.r(),clr.g(),clr.b(), low_alpha_text}});
						}
					}
				}

				// lethal.
				if (*it == 7) {

					auto m_weapon = g_cl.m_local->GetActiveWeapon();

					if (g_cl.m_weapon_id == SSG08 && hp <= 93) {
						flags.push_back({ XOR("LETHAL"), { 150, 200, 60, low_alpha_text } });
					}
					else if (g_cl.m_weapon_id == AWP && hp <= 100) {
						flags.push_back({ XOR("LETHAL"), { 0, 200, 0, low_alpha_text } });
					}
					else if (g_cl.m_weapon_id == G3SG1 || g_cl.m_weapon_id == SCAR20 && hp <= 40) {
						flags.push_back({ XOR("LETHAL"), { 150, 200, 60, low_alpha_text } });
					}
					else if (g_cl.m_weapon_id == G3SG1 || g_cl.m_weapon_id == SCAR20 && hp <= 81) {
						flags.push_back({ XOR("LETHAL"), { 252, 161, 3, low_alpha_text } });
					}
					else if (m_weapon && !m_weapon->IsKnife()) {
						auto data = m_weapon->GetWpnData();
						if (data->m_damage >= (int)std::round(player->m_iHealth()))
							flags.push_back({ XOR("LETHAL"), Color(252, 103, 3, low_alpha_text) });
					}
				}

				// tickbase.
				if (*it == 8) {
					if (data && data->m_records.size() && !dormant) {
						LagRecord* current = data->m_records.front().get();
						const int shift_type = current->shift_type();
						Color clr = Color(255, 255, 255, low_alpha_text);
						if (shift_type == shift_type_t::SHIFT_BREAK_LC)
							flags.push_back({ "DEFENSIVE", !current->m_invalid ? colors::transparent_white : clr });
						else if (shift_type == shift_type_t::SHIFT_DEFAULT)
							flags.push_back({ "DT", clr });
					}
				}

				// resolver mode.
				if (*it == 9) {
					if (data && data->m_records.size() && !dormant) {
						LagRecord* current = data->m_records.front().get();
						flags.push_back({ current->m_resolver_mode, { 155, 210, 100, low_alpha_text} });
					}
				}

				// resolver chance.
				if (*it == 10) {
					if (data && data->m_records.size() && !dormant) {
						LagRecord* current = data->m_records.front().get();
						flags.push_back({ current->m_resolver_chance, { 255, 255, 255, low_alpha_text} });
					}
				}

				if (*it == 11 && g_cl.m_resource) {
					auto player_resource = *(g_cl.m_resource);
					int ping = round(player_resource->GetPlayerPing(player->index()));

					//Color ping_flag;
					//if (ping <= 70) {
					//	ping_flag = Color(150, 200, 60, low_alpha_text);
					//}
					//else if (ping <= 300) {
					//	ping_flag = Color(225, 137, 49, low_alpha_text);
					//}
					//else {
					//	ping_flag = Color(255, 0, 0, low_alpha_text);
					//}


					float ping_percent = std::clamp(ping, 0, 750) / 750.f;

					Color start_color(157, 219, 90, low_alpha);
					Color middle_color(242, 151, 51, low_alpha);
					Color end_color(242, 51, 51, low_alpha);

					int r, g, b;
					if (ping <= 100) {
						// Green color
						r = start_color.r();
						g = start_color.g();
						b = start_color.b();
					}
					else if (ping <= 300) {
						float t = (ping - 150) / 200.0f;
						r = start_color.r() + static_cast<int>((middle_color.r() - start_color.r()) * t);
						g = start_color.g() + static_cast<int>((middle_color.g() - start_color.g()) * t);
						b = start_color.b() + static_cast<int>((middle_color.b() - start_color.b()) * t);
					}
					else {
						float t = (ping - 300) / 300.0f;
						r = middle_color.r() + static_cast<int>((end_color.r() - middle_color.r()) * t);
						g = middle_color.g() + static_cast<int>((end_color.g() - middle_color.g()) * t);
						b = middle_color.b() + static_cast<int>((end_color.b() - middle_color.b()) * t);
					}

					Color ping_flag(r, g, b, low_alpha);

					flags.push_back({ tfm::format(XOR("%iMS"), ping), ping_flag });
					//flags.push_back({ tfm::format(XOR("PING")), ping_flag });

				}

				// distortion.
				if (*it == 12) {
					if (data && !dormant) {
						if (data->m_update_count >= 1
							&& data->m_change_stored >= 1) {
							flags.push_back({ "DISTORTION", {255, 255, 255, low_alpha_text} });
						}
					}
				}

				if (*it == 13) {
					if (data && !dormant) {
						if (data->m_hit) {
							flags.push_back({ "HIT", {255, 255, 255, low_alpha_text} });
						}
					}
				}

				if (*it == 14) {
					if (data && data->m_records.size() && !dormant) {
						LagRecord* current = data->m_records.front().get();
						const int shift_type = current->shift_type();
						if (!current->m_velocity.length() <= 0.1f) {
							if (shift_type == shift_type_t::SHIFT_BREAK_LC || current->lagcomp())
								flags.push_back({ "LC", {255, 0, 0, low_alpha_text} });
							else
								flags.push_back({ "LC", {0, 255, 0, low_alpha_text} });
						}
					}
				}
			}
		}

		if (enemy
			&& data
			&& player->alive()
			&& !dormant) {

			if (g_aimbot.m_override_target
				&& g_aimbot.m_override_target->m_override
				&& player == g_aimbot.m_override_target->m_player) {
				flags.push_back({ "OVERRIDE", Color(150, 200, 60, low_alpha_text) });
			}

		}

		// iterate flags.
		for (size_t i{ }; i < flags.size(); ++i) {
			// get flag job (pair).
			auto& f = flags[i];

			int offset = i * (render::small_fonts.m_size.m_height);

			// draw flag.
			render::small_fonts.string(box.x + box.w + 2, box.y + offset, f.second, f.first);
		}
	}


	// draw bottom bars.
	{
		int  offset{ 0 };

		// draw lby update bar.
		if (enemy && g_menu.main.players.lby_update.get()) {
			AimPlayer* data = &g_aimbot.m_players[player->index() - 1];

			// make sure everything is valid.
			if (data && data->m_records.size()) {
				// grab lag record.
				LagRecord* current = data->m_records.front().get();

				if (current) {
					// calculate box width
					float cycle = std::clamp<float>(data->m_body_timer - current->m_anim_time, 0.f, 1.0f);
					float width = !(current->m_velocity.length_2d() > 0.1 && !current->m_fake_walk) ? (box.w * cycle) / 1.1f : box.w / 1.1f;
					if (width > 0.f) {
						// draw.
						render::rect_filled(box.x, box.y + box.h + 2, box.w, 4, { 10, 10, 10, low_alpha });

						Color clr = g_menu.main.players.lby_update_color.get();
						clr.a() = alpha;
						render::rect(box.x + 1, box.y + box.h + 3, width, 2, clr);

						// move down the offset to make room for the next bar.
						offset += 5;

					}
				}
			}
		}

		// draw weapon.
		if (enemy && (g_menu.main.players.weaponicon.get() || g_menu.main.players.weapontext.get())) {
			Weapon* weapon = player->GetActiveWeapon();
			if (weapon) {
				WeaponInfo* data = weapon->GetWpnData();
				if (data) {
					int bar;
					float scale;

					// the maxclip1 in the weaponinfo
					int max = data->m_max_clip1;
					int current = weapon->m_iClip1();

					C_AnimationLayer* layer1 = &player->m_AnimOverlay()[1];

					// set reload state.
					bool reload = (layer1->weight != 0.f) && (player->GetSequenceActivity(layer1->sequence) == 967);

					// ammo bar.
					if (max != -1 && g_menu.main.players.ammo.get()) {
						// check for reload.
						if (reload)
							scale = layer1->cycle;
						else
							scale = (float)current / max;


						// relative to bar.
						bar = (int)std::round((box.w - 2) * scale);

						// draw.
						render::rect_filled(box.x, box.y + box.h + 2 + offset, box.w, 4, { 10, 10, 10, low_alpha });

						Color clr = g_menu.main.players.ammo_color.get();
						clr.a() = alpha;
						render::rect(box.x + 1, box.y + box.h + 3 + offset, bar, 2, clr);


						std::string bullets_str = std::to_string(current);

						// less then a 5th of the bullets left.
						if (current <= (int)std::round(max / 5) && !reload)
							render::small_fonts.string(box.x + bar, box.y + box.h + offset, { 255, 255, 255, low_alpha_text }, std::to_string(current), render::ALIGN_CENTER);

						offset += 6;
					}

					// text.
					if (g_menu.main.players.weapontext.get()) {
						// construct std::string instance of localized weapon name.
						std::string name{ weapon->GetLocalizedName() };

						std::transform(name.begin(), name.end(), name.begin(), ::toupper);

						render::small_fonts.string(box.x + box.w / 2, box.y + box.h + offset, { 255, 255, 255, low_alpha_text }, name, render::ALIGN_CENTER);
					}

					// icons.
					else if (g_menu.main.players.weaponicon.get()) {

						// icons are super fat..
						// move them back up.
						offset -= 5;

						std::string icon = tfm::format(XOR("%c"), m_weapon_icons[weapon->m_iItemDefinitionIndex()]);
						render::cs.string(box.x + box.w / 2, box.y + box.h + offset, { 255, 255, 255, low_alpha_text }, icon, render::ALIGN_CENTER);
					}
				}
			}
		}
	}
}

void Visuals::DrawHitboxMatrix(LagRecord* record, float time) {
	if (g_menu.main.aimbot.matrix_shot.get() == 0)
		return;

	const model_t* model;
	studiohdr_t* hdr;
	mstudiohitboxset_t* set;
	mstudiobbox_t* bbox;
	vec3_t             mins, maxs, origin;
	ang_t			   angle;

	model = record->m_player->GetModel();
	if (!model)
		return;

	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	set = hdr->GetHitboxSet(record->m_player->m_nHitboxSet());
	if (!set)
		return;

	bool behind_wall = g_menu.main.aimbot.matrix_behind_walls.get();

	if (g_menu.main.aimbot.matrix_shot.get() == 2) {
		bbox = set->GetHitbox(g_aimbot.m_hitbox);

		if (!bbox)
			return;


		Color col = g_menu.main.aimbot.matrix_shot_clr_target.get();

		// bbox.
		if (bbox->m_radius <= 0.f) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms(record->m_bones[bbox->m_bone], rot_matrix, matrix);

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles(matrix, bbox_angle);

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin();

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay(origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r(), col.g(), col.b(), 0, time);
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix(bbox->m_angle, matrix);

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms(record->m_bones[bbox->m_bone], matrix, matrix);

			// get world positions from new matrix.
			math::VectorTransform(bbox->m_mins, matrix, mins);
			math::VectorTransform(bbox->m_maxs, matrix, maxs);

			g_csgo.m_debug_overlay->AddCapsuleOverlay(mins, maxs, bbox->m_radius, col.r(), col.g(), col.b(), col.a(), time, 0, behind_wall);
		}

		return;
	}

	for (int i{ }; i < set->m_hitboxes; ++i) {

		bbox = set->GetHitbox(i);
		if (!bbox)
			continue;

		Color col = i == g_aimbot.m_hitbox
			&& g_menu.main.aimbot.matrix_shot.get() == 3 ?
			g_menu.main.aimbot.matrix_shot_clr_target.get() : g_menu.main.aimbot.matrix_shot_clr.get();

		// bbox.
		if (bbox->m_radius <= 0.f) {
			// https://developer.valvesoftware.com/wiki/Rotation_Tutorial

			// convert rotation angle to a matrix.
			matrix3x4_t rot_matrix;
			g_csgo.AngleMatrix(bbox->m_angle, rot_matrix);

			// apply the rotation to the entity input space (local).
			matrix3x4_t matrix;
			math::ConcatTransforms(record->m_bones[bbox->m_bone], rot_matrix, matrix);

			// extract the compound rotation as an angle.
			ang_t bbox_angle;
			math::MatrixAngles(matrix, bbox_angle);

			// extract hitbox origin.
			vec3_t origin = matrix.GetOrigin();

			// draw box.
			g_csgo.m_debug_overlay->AddBoxOverlay(origin, bbox->m_mins, bbox->m_maxs, bbox_angle, col.r(), col.g(), col.b(), 0, time);
		}

		// capsule.
		else {
			// NOTE; the angle for capsules is always 0.f, 0.f, 0.f.

			// create a rotation matrix.
			matrix3x4_t matrix;
			g_csgo.AngleMatrix(bbox->m_angle, matrix);

			// apply the rotation matrix to the entity output space (world).
			math::ConcatTransforms(record->m_bones[bbox->m_bone], matrix, matrix);

			// get world positions from new matrix.
			math::VectorTransform(bbox->m_mins, matrix, mins);
			math::VectorTransform(bbox->m_maxs, matrix, maxs);

			g_csgo.m_debug_overlay->AddCapsuleOverlay(mins, maxs, bbox->m_radius, col.r(), col.g(), col.b(), col.a(), time, 0, behind_wall);
		}
	}
}

void Visuals::DrawBeams() {
	size_t     impact_count;
	float      curtime, dist;
	bool       is_final_impact;
	vec3_t     va_fwd, start, dir, end;
	BeamInfo_t beam_info;
	Beam_t* beam;

	if (!g_cl.m_local)
		return;

	if (!g_menu.main.visuals.impact_beams.get() || !g_menu.main.visuals.impact_beams_entities.get(1))
		return;

	auto vis_impacts = &g_shots.m_vis_impacts;

	// the local player is dead, clear impacts.
	if (!g_cl.m_processing) {
		if (!vis_impacts->empty())
			vis_impacts->clear();
	}

	else {
		impact_count = vis_impacts->size();
		if (!impact_count)
			return;

		curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

		for (size_t i{ impact_count }; i-- > 0; ) {
			auto impact = &vis_impacts->operator[ ](i);
			if (!impact)
				continue;

			// impact is too old, erase it.
			if (std::abs(curtime - game::TICKS_TO_TIME(impact->m_tickbase)) > g_menu.main.visuals.impact_beams_time.get()) {
				vis_impacts->erase(vis_impacts->begin() + i);

				continue;
			}

			// already rendering this impact, skip over it.
			if (impact->m_ignore)
				continue;

			// is this the final impact?
			// last impact in the vector, it's the final impact.
			if (i == (impact_count - 1))
				is_final_impact = true;

			// the current impact's tickbase is different than the next, it's the final impact.
			else if ((i + 1) < impact_count && impact->m_tickbase != vis_impacts->operator[ ](i + 1).m_tickbase)
				is_final_impact = true;

			else
				is_final_impact = false;

			// is this the final impact?
			// is_final_impact = ( ( i == ( impact_count - 1 ) ) || ( impact->m_tickbase != vis_impacts->at( i + 1 ).m_tickbase ) );

			if (is_final_impact) {
				// calculate start and end position for beam.
				start = impact->m_shoot_pos;

				dir = (impact->m_impact_pos - start).normalized();
				dist = (impact->m_impact_pos - start).length();

				end = start + (dir * dist);

				// setup beam info.
				// note - dex; possible beam models: sprites/physbeam.vmt | sprites/white.vmt
				Color beam_info_color = g_menu.main.visuals.impact_beams_color.get();

				beam_info.m_vecStart = start;
				beam_info.m_vecEnd = end;
				beam_info.m_nType = 0;
				beam_info.m_pszModelName = ("sprites/purplelaser1.vmt");
				beam_info.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(("sprites/purplelaser1.vmt"));
				beam_info.m_flHaloScale = 0.0f;
				beam_info.m_flLife = 3.f;
				beam_info.m_flWidth = 4.0f;
				beam_info.m_flEndWidth = 4.0f;
				beam_info.m_flFadeLength = 0.0f;
				beam_info.m_flAmplitude = 2.0f;
				beam_info.m_flBrightness = beam_info_color.a();
				beam_info.m_flSpeed = 0.2f;
				beam_info.m_nStartFrame = 0;
				beam_info.m_flFrameRate = 0.f;
				beam_info.m_nSegments = 2;
				beam_info.m_bRenderable = true;
				beam_info.m_nFlags = 0x100 | 0x200 | 0x8000;
				beam_info.m_flRed = beam_info_color.r();
				beam_info.m_flGreen = beam_info_color.g();
				beam_info.m_flBlue = beam_info_color.b();


				// attempt to render the beam.
				beam = game::CreateGenericBeam(beam_info);
				if (beam) {
					g_csgo.m_beams->DrawBeam(beam);

					// we only want to render a beam for this impact once.
					impact->m_ignore = true;
				}
			}
		}
	}
}

void Visuals::DrawPlantedC4() {
	bool    mode_2d, is_visible;
	float    explode_time_diff, dist, range_damage;
	vec3_t   dst, to_target;
	std::string time_str, damage_str;
	Color    damage_color;
	vec2_t   screen_pos;

	static auto scale_damage = [](float damage, int armor_value) {
		float new_damage, armor;

		if (armor_value > 0) {
			new_damage = damage * 0.5f;
			armor = (damage - new_damage) * 0.5f;

			if (armor > (float)armor_value) {
				armor = (float)armor_value * 2.f;
				new_damage = damage - armor;
			}

			damage = new_damage;
		}

		return std::max(0, (int)std::floor(damage));
	};

	// store menu vars for later.
	mode_2d = g_menu.main.visuals.planted_c4.get();
	Color col2 = g_menu.main.visuals.bomb_col.get();
	int alpha2 = g_menu.main.visuals.bomb_col_slider.get();

	// bomb not currently active, do nothing.
	if (!m_c4_planted)
		return;

	// calculate bomb damage.
	// references:
	//   https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L271
	//   https://github.com/VSES/SourceEngine2007/blob/43a5c90a5ada1e69ca044595383be67f40b33c61/se2007/game/shared/cstrike/weapon_c4.cpp#L437
	//   https://github.com/ValveSoftware/source-sdk-2013/blob/master/sp/src/game/shared/sdk/sdk_gamerules.cpp#L173
	{
		// get our distance to the bomb.
		// todo - dex; is dst right? might need to reverse CBasePlayer::BodyTarget...
		dst = g_cl.m_local->WorldSpaceCenter();
		to_target = m_planted_c4_explosion_origin - dst;
		dist = to_target.length();

		// calculate the bomb damage based on our distance to the C4's explosion.
		range_damage = m_planted_c4_damage * std::exp((dist * dist) / ((m_planted_c4_radius_scaled * -2.f) * m_planted_c4_radius_scaled));

		// now finally, scale the damage based on our armor (  if we have any ).
		m_final_damage = scale_damage(range_damage, g_cl.m_local->m_ArmorValue());
	}

	if (!mode_2d)
		return;

	if (mode_2d) {
		float complete = (g_csgo.m_globals->m_curtime - m_plant_start) / m_plant_duration;
		int alpha = (1.f - complete) * g_cl.m_height;

		if (m_planted_c4_planting && complete > 0) {
			std::string b_time_str = tfm::format(XOR("PLANT - %.1fs"), 3.f * complete);
			Color color = { 0xff15c27b };
			color.a() = 125;

			render::indicator.string(6, 11, { 0,0, 0, 125 }, b_time_str);
			render::indicator.string(5, 10, color, b_time_str);

			render::rect_filled(0, 0, 21, g_cl.m_height, { 0, 0, 0, 80 });
			render::rect_filled(0, g_cl.m_height - alpha / 2, 20, alpha, { color });
		}
	}


	// m_flC4Blow is set to gpGlobals->curtime + m_flTimerLength inside CPlantedC4.
	explode_time_diff = m_planted_c4_explode_time - g_csgo.m_globals->m_curtime;

	// get formatted strings for bomb.
	//time_str = tfm::format( XOR( "%.2f" ), explode_time_diff );
	time_str = tfm::format(XOR("%s - %.1fs"), m_last_bombsite.substr(0, 1), explode_time_diff);
	damage_str = tfm::format(XOR("- %iHP"), m_final_damage);
	if (g_cl.m_local->m_iHealth() <= m_final_damage) damage_str = tfm::format(XOR("FATAL"));
	m_c4_damage = m_final_damage;

	Color colortimer = { 0xff15c27b };
	if (explode_time_diff < 10) colortimer = { 200, 200, 110, 125 };
	if (explode_time_diff < 5) colortimer = { 0xff0000ff };
	colortimer.a() = 125;

	static Color defusetimer;
	if (explode_time_diff < 5 && !m_planted_c4_indefuse) defusetimer = { 0xff0000ff };
	else if (explode_time_diff < 10 && !m_planted_c4_indefuse) defusetimer = { 0xff0000ff };
	else if (m_planted_c4_defuse == 1 && explode_time_diff >= 5) defusetimer = { 0xff15c27b };
	else if (m_planted_c4_defuse == 2 && explode_time_diff >= 10) defusetimer = { 0xff15c27b };
	defusetimer.a() = 125;

	// get damage color.
	damage_color = (m_final_damage < g_cl.m_local->m_iHealth()) ? colors::white : colors::red;

	// finally do all of our rendering.
	is_visible = render::WorldToScreen(m_planted_c4_explosion_origin, screen_pos);

	float complete = (g_csgo.m_globals->m_curtime - m_defuse_start) / m_defuse_duration;
	int alpha = (1.f - complete) * g_cl.m_height;

	Color site_color = Color(150, 200, 60, 220);
	if (explode_time_diff <= 10.f) {
		site_color = Color(255, 255, 185, 220);

		if (explode_time_diff <= 5.f) {
			site_color = Color(255, 0, 0, 220);
		}
	}

	// 'on screen (  2D )'.
	if (mode_2d) {
		if (m_c4_planted && m_planted_c4_indefuse && explode_time_diff > 0) {

			if (m_planted_c4_defuse == 1) {
				render::rect_filled(0, 0, 21, g_cl.m_height, { 0, 0, 0, 100 });
				render::rect_filled(0, g_cl.m_height - alpha / 2, 20, alpha, { 30, 170, 30 });
			}
			else {
				render::rect_filled(0, 0, 21, g_cl.m_height, { 0, 0, 0, 100 });
				render::rect_filled(0, (g_cl.m_height - alpha), 20, alpha, { 30, 170, 30 });
			}
		}

		if (explode_time_diff > 0.f) {
			render::indicator.string(8, 8, site_color, time_str, render::ALIGN_LEFT);
		}

		if (g_cl.m_local->alive() && m_final_damage > 0) {
			render::indicator.string(8, 36, m_final_damage >= g_cl.m_local->m_iHealth() ? Color(255, 0, 0) : Color(255, 255, 185), m_final_damage >= g_cl.m_local->m_iHealth() ? XOR("fatal") : damage_str, render::ALIGN_LEFT);
		}
	}

	// 'on bomb (  3D )'.
	if (mode_2d && is_visible) {
		if (explode_time_diff > 0.f)
			render::small_fonts.string(screen_pos.x, screen_pos.y - render::esp_small.m_size.m_height - 1, Color(col2.r(), col2.g(), col2.b(), alpha2), "bomb", render::ALIGN_CENTER);
	}
}

bool Visuals::GetPlayerBoxRect(Player* player, Rect& box) {
	vec3_t min, max, out_vec;
	float left, bottom, right, top;
	matrix3x4_t& tran_frame = player->m_pCoordFrame();

	// get hitbox bounds.
	min = player->m_vecMins();
	max = player->m_vecMaxs();

	vec2_t screen_boxes[8];

	// transform mins and maxes to points. 
	vec3_t points[] =
	{
		{ min.x, min.y, min.z },
		{ min.x, max.y, min.z },
		{ max.x, max.y, min.z },
		{ max.x, min.y, min.z },
		{ max.x, max.y, max.z },
		{ min.x, max.y, max.z },
		{ min.x, min.y, max.z },
		{ max.x, min.y, max.z }
	};

	// transform points to 3-dimensional space.
	for (int i = 0; i <= 7; i++) {
		math::VectorTransform(points[i], tran_frame, out_vec);
		if (!render::WorldToScreen(out_vec, screen_boxes[i]))
			return false;
	}

	// generate an array to clamp later.
	vec2_t box_array[] = {
		screen_boxes[3],
		screen_boxes[5],
		screen_boxes[0],
		screen_boxes[4],
		screen_boxes[2],
		screen_boxes[1],
		screen_boxes[6],
		screen_boxes[7]
	};

	// state the position and size of the box.
	left = screen_boxes[3].x,
		bottom = screen_boxes[3].y,
		right = screen_boxes[3].x,
		top = screen_boxes[3].y;

	// clamp the box sizes.
	for (int i = 0; i <= 7; i++) {
		if (left > box_array[i].x)
			left = box_array[i].x;

		if (bottom < box_array[i].y)
			bottom = box_array[i].y;

		if (right < box_array[i].x)
			right = box_array[i].x;

		if (top > box_array[i].y)
			top = box_array[i].y;
	}

	// state the box bounds.
	box.x = left;
	box.y = top;
	box.w = right - left;
	box.h = (bottom - top);

	return true;
}

void Visuals::OverrideMaterial(bool ignoreZ, bool use_env, Color& color, IMaterial* material) {
	material->SetFlag(MATERIAL_VAR_IGNOREZ, ignoreZ);
	material->IncrementReferenceCount();

	bool found;
	auto var = material->FindVar("$envmaptint", &found);

	if (found)
		var->SetVecValue(color.r(), color.g(), color.b());

	g_csgo.m_studio_render->ForcedMaterialOverride(material);
}

void Visuals::DrawSkeleton(Player* player, int opacity) {
	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[128];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;
	Color		  color;

	// get player's model.
	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	// get bone matrix.
	if (!player->SetupBones(matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime))
		return;

	if (!player->alive())
		return;

	if (player->dormant())
		return;

	for (int i{ }; i < hdr->m_num_bones; ++i) {
		// get bone.
		bone = hdr->GetBone(i);
		if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
			continue;

		// get parent bone.
		parent = bone->m_parent;
		if (parent == -1)
			continue;

		// resolve main bone and parent bone positions.
		matrix->get_bone(bone_pos, i);
		matrix->get_bone(parent_pos, parent);

		Color clr = g_menu.main.players.skeleton_col.get();

		// world to screen both the bone parent bone then draw.
		if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
			render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);
	}
}

bool IsGrenade(const int id)
{
	return id == 9 || id == 98 || id == 134;
}

bool isc4(const int id)
{
	return id == 29 || id == 108;
}

void Visuals::RenderGlow() {
	Color   color;
	Player* player;

	if (!g_cl.m_local)
		return;

	if (!g_csgo.m_glow->m_object_definitions.Count())
		return;

	for (int i{ }; i < g_csgo.m_glow->m_object_definitions.Count(); ++i) {
		GlowObjectDefinition_t* obj = &g_csgo.m_glow->m_object_definitions[i];

		// skip non-players.
		if (!obj->m_pEntity || !obj->m_pEntity->IsPlayer())
			continue;

		// get player ptr.
		player = obj->m_pEntity->as< Player* >();

		//g_notify.add(tfm::format("%i", obj->m_nRenderStyle));

		// get reference to array variable.
		float& opacity = m_opacities[player->index() - 1];

		bool enemy = player->enemy(g_cl.m_local);

		if (enemy && g_menu.main.players.glow.get() == 0)
			continue;

		if (!enemy)
			continue;

		// enemy color
		if (enemy)
			color = g_menu.main.players.glow_enemy.get();

		static IMaterial* pMatTeamIdShape;
		static IMaterial* pmat2;
		static IMaterial* pmat_weirdhealthboxthing;
		static bool ran = false; if (!ran) {
			pMatTeamIdShape = g_csgo.m_material_system->FindMaterial(XOR("dev/glow_edge_highlight"), TEXTURE_GROUP_OTHER);
			pmat2 = g_csgo.m_material_system->FindMaterial(XOR("dev/glow_color"), TEXTURE_GROUP_OTHER);
			pmat_weirdhealthboxthing = g_csgo.m_material_system->FindMaterial(XOR("dev/glow_health_color"), TEXTURE_GROUP_OTHER); // box that appears on players being hurt

			pMatTeamIdShape->SetFlag(MATERIAL_VAR_IGNOREZ, true);
			pmat2->SetFlag(MATERIAL_VAR_IGNOREZ, true);
			pmat_weirdhealthboxthing->SetFlag(MATERIAL_VAR_NO_DRAW, true);
			ran = true;
		}

		obj->m_bFullBloomRender = g_menu.main.players.glow.get() == 1;
		obj->m_bRenderWhenOccluded = true;
		obj->m_bRenderWhenUnoccluded = g_menu.main.players.glow.get() > 2;
		obj->m_nRenderStyle = g_menu.main.players.glow.get() < 2 ? 0 : g_menu.main.players.glow.get() - 2;
		obj->m_vGlowColor = { (float)color.r() / 255.f, (float)color.g() / 255.f, (float)color.b() / 255.f };
		obj->m_flGlowAlpha = opacity * ((float)color.a() / 255.f);
	}
}

void Visuals::Add(BulletImpactInfo beamEffect) {
	bulletImpactInfo.push_back(beamEffect);
}

void Visuals::DrawHistorySkeleton(Player* player, int opacity) {

	const model_t* model;
	studiohdr_t* hdr;
	mstudiobone_t* bone;
	int           parent;
	BoneArray     matrix[128];
	vec3_t        bone_pos, parent_pos;
	vec2_t        bone_pos_screen, parent_pos_screen;

	if (!player || !player->alive() || player->m_iHealth() < 0 || player->dormant())
		return;

	model = player->GetModel();
	if (!model)
		return;

	// get studio model.
	hdr = g_csgo.m_model_info->GetStudioModel(model);
	if (!hdr)
		return;

	if (!player->SetupBones(matrix, 128, BONE_USED_BY_ANYTHING, g_csgo.m_globals->m_curtime))
		return;

		if (g_aimbot.IsValidTarget(player)) {
			AimPlayer* data = &g_aimbot.m_players[player->index() - 1];
			if (data && !data->m_records.empty()) {
				LagRecord* record = g_resolver.FindLastRecord(data);
				if (record && (!record->m_broke_lc)) {
					BoneArray arr[128];
					if (g_chams.GenerateLerpedMatrix(player->index(), arr)) {

						for (int i{ }; i < hdr->m_num_bones; ++i) {
							// get bone.
							bone = hdr->GetBone(i);
							if (!bone || !(bone->m_flags & BONE_USED_BY_HITBOX))
								continue;

							// get parent bone.
							parent = bone->m_parent;
							if (parent == -1)
								continue;


							// resolve main bone and parent bone positions.
							arr->get_bone(bone_pos, i);
							arr->get_bone(parent_pos, parent);

							Color clr = g_menu.main.players.history_skeleton_col.get();

							clr.a() = std::clamp(g_menu.main.players.history_skeleton_col.get().a() * g_chams.dist_alpha, 0.f, 255.f);

							if (render::WorldToScreen(bone_pos, bone_pos_screen) && render::WorldToScreen(parent_pos, parent_pos_screen))
								render::line(bone_pos_screen.x, bone_pos_screen.y, parent_pos_screen.x, parent_pos_screen.y, clr);

					}					
				}
			}
		}
	}
}

