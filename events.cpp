#include "includes.h"

Listener g_listener{};;

void events::round_start(IGameEvent* evt) {
	// new round has started. no longer round end.
	g_cl.m_round_end = false;

	// fix fix?
	g_cl.m_body_pred = g_csgo.m_globals->m_curtime;

	// remove notices.
	if (g_menu.main.misc.preserve_killfeed.get()) {
		KillFeed_t* feed = (KillFeed_t*)g_csgo.m_hud->FindElement(HASH("SFHudDeathNoticeAndBotStatus"));
		if (feed)
			g_csgo.ClearNotices(feed);
	}

	// reset hvh / aa stuff.
	g_hvh.m_next_random_update = 0.f;
	g_hvh.m_auto_last = 0.f;

	// reset bomb stuff.
	g_visuals.m_c4_planted = false;
	g_visuals.m_planted_c4 = nullptr;
	g_cl.m_setupped = false;

	// reset dormant esp.
	g_visuals.m_draw.fill(false);
	g_visuals.m_opacities.fill(0.f);

	if (g_menu.main.misc.auto_buy.get())
	{
		auto auto_buy_misc = g_menu.main.misc.auto_buy_misc.GetActiveItems();

		if (g_menu.main.misc.auto_buy_primary.get() == 1) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy awp")).data());
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy awp")).data());
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy awp")).data());
		}

		if (g_menu.main.misc.auto_buy_primary.get() == 2) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy ssg08")).data());
		}

		if (g_menu.main.misc.auto_buy_primary.get() == 3) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy scar20")).data());
		}

		if (g_menu.main.misc.auto_buy_secondary.get() == 1) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy elite")).data());
		}

		if (g_menu.main.misc.auto_buy_secondary.get() == 2) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy p250")).data());
		}

		if (g_menu.main.misc.auto_buy_secondary.get() == 3) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy tec9")).data());
		}

		if (g_menu.main.misc.auto_buy_secondary.get() == 4) {
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy deagle")).data());
		}


		for (auto it = auto_buy_misc.begin(); it != auto_buy_misc.end(); ++it)
			g_csgo.m_engine->ExecuteClientCmd(tfm::format(XOR("buy %s"), *it).data());
	}




	// update all players.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
		if (!player || player->m_bIsLocalPlayer())
			continue;

		AimPlayer* data = &g_aimbot.m_players[i - 1];
		data->OnRoundStart(player);
	}

	// clear origins.
	g_cl.m_net_pos.clear();
}

void events::round_end(IGameEvent* evt) {
	if (!g_cl.m_local)
		return;

	g_cl.m_setupped = false;

	// get the reason for the round end.
	int reason = evt->m_keys->FindKey(HASH("reason"))->GetInt();

	// reset.
	g_cl.m_round_end = false;

	if (g_cl.m_local->m_iTeamNum() == TEAM_COUNTERTERRORISTS && reason == CSRoundEndReason::CT_WIN)
		g_cl.m_round_end = true;

	else if (g_cl.m_local->m_iTeamNum() == TEAM_TERRORISTS && reason == CSRoundEndReason::T_WIN)
		g_cl.m_round_end = true;

}

void events::player_hurt(IGameEvent* evt) {
	int attacker, victim;

	// offscreen esp damage stuff.
	if (evt) {
		attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt());
		victim = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
		// a player damaged the local player.
		//if( attacker > 0 && attacker < 64 && victim == g_csgo.m_engine->GetLocalPlayer( ) )
		g_shots.OnHurt(evt);

		if (!strcmp(evt->GetName(), XOR("player_hurt"))
			&& g_menu.main.players.footstep.get()) {
			auto ent = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetPlayerForUserID(evt->GetInt(XOR("userid"))));
			if (ent == nullptr)
				return;

			if (!ent && ent == g_cl.m_local || g_cl.m_local->m_iTeamNum() == ent->m_iTeamNum() || ent->dormant() || !ent->alive())
				return;

			g_visuals.Add({ g_csgo.m_globals->m_realtime, ent->GetAbsOrigin(), { }, g_menu.main.players.footstepcol.get(), ent->index(), ent->m_nTickBase(), false, true });
		}
	}

	if (evt) {
		Player* attacker = g_csgo.m_entlist->GetClientEntity<Player*>(g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt()));
		Player* victim = g_csgo.m_entlist->GetClientEntity<Player*>(g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt()));

		if (!attacker || !victim || attacker != g_cl.m_local)
			return;

		vec3_t enemypos = victim->GetAbsOrigin();
		ImpactInfo best_impact;
		float best_impact_distance = -1;
		float time = g_csgo.m_globals->m_curtime;


		for (int i = 0; i < g_visuals.m_impacts.size(); i++) {
			auto iter = g_visuals.m_impacts[i];
			if (time > iter.time + 1.f) {
				g_visuals.m_impacts.erase(g_visuals.m_impacts.begin() + i);
				continue;
			}
			vec3_t position = vec3_t(iter.x, iter.y, iter.z);
			float distance = position.dist_to(enemypos);
			if (distance < best_impact_distance || best_impact_distance == -1) {
				best_impact_distance = distance;
				best_impact = iter;
			}
		}
		if (best_impact_distance == -1)
			return;

		HitmarkerWorld_t info;
		info.m_pos = best_impact;
		info.m_alpha = 255;
		info.m_time = g_csgo.m_globals->m_curtime;
		g_shots.m_hitmarkers.push_back(info);
	}

	// Check if the local player was hurt
	if (victim == g_csgo.m_engine->GetLocalPlayer()) {
		// Get the attacker
		if (attacker > 0 && attacker < 64) {
			// Get attacker's name
			player_info_t attacker_info;
			if (g_csgo.m_engine->GetPlayerInfo(attacker, &attacker_info)) {
				// Get hitgroup
				int hitgroup = evt->m_keys->FindKey(HASH("hitgroup"))->GetInt();
				std::string hitgroup_str;
				switch (hitgroup) {
				case HITGROUP_GENERIC:
					hitgroup_str = "body";
					break;
				case HITGROUP_HEAD:
					hitgroup_str = "head";
					break;
				case HITGROUP_CHEST:
					hitgroup_str = "chest";
					break;
				case HITGROUP_STOMACH:
					hitgroup_str = "stomach";
					break;
				case HITGROUP_LEFTARM:
					hitgroup_str = "left arm";
					break;
				case HITGROUP_RIGHTARM:
					hitgroup_str = "right arm";
					break;
				case HITGROUP_LEFTLEG:
					hitgroup_str = "left leg";
					break;
				case HITGROUP_RIGHTLEG:
					hitgroup_str = "right leg";
					break;
				default:
					hitgroup_str = "unknown";
					break;
				}

				// Notify that the attacker hurt the local player and include hitgroup information
				std::string out = tfm::format(XOR("hurt by %s in the %s for %d damage\n"), std::string(attacker_info.m_name).substr(0, 24), hitgroup_str, evt->m_keys->FindKey(HASH("dmg_health"))->GetInt());
				g_notify.add(out, colors::gray);
			}
		}
	}
}

void events::player_footstep(IGameEvent* evt) {
	if (!g_menu.main.players.footstep.get())
		return;

	if (!strcmp(evt->GetName(), XOR("player_footstep"))) {
		auto ent = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetPlayerForUserID(evt->GetInt(XOR("userid"))));
		if (!ent
			|| ent == g_cl.m_local
			|| g_cl.m_local->m_iTeamNum() == ent->m_iTeamNum()
			|| ent->dormant()
			|| !ent->alive()
			|| ent->m_flSimulationTime() < ent->m_flOldSimulationTime())
			return;

		g_visuals.Add({ g_csgo.m_globals->m_realtime, ent->GetAbsOrigin(), { }, g_menu.main.players.footstepcol.get(), ent->index(), ent->m_nTickBase(), false, true });
	}
}

void events::bullet_impact(IGameEvent* evt) {
	// forward event to resolver impact processing.
	g_shots.OnImpact(evt);

	if (g_menu.main.visuals.impact_beams.get()
		&& g_menu.main.visuals.impact_beams_entities.get(0)) {
		Player* enemy = g_csgo.m_entlist->GetClientEntity<Player*>(g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt()));
		if (g_cl.m_processing && enemy && enemy->m_iTeamNum() != g_cl.m_local->m_iTeamNum() && !enemy->dormant()) {
			Beam_t* beam;

			if (!g_cl.m_local || !evt)
				return;

			vec3_t pos(evt->GetFloat("x"), evt->GetFloat("y"), evt->GetFloat("z"));
			vec3_t start;
			enemy->GetEyePos(&start);

			auto color = g_menu.main.visuals.impact_beams_color.get();
			color.a() = g_menu.main.visuals.alpha2.get();
			BeamInfo_t beamInfo;
			if (g_menu.main.visuals.beam_type.get() == 0) {
				beamInfo.m_vecStart = start;
				beamInfo.m_vecEnd = pos;
				beamInfo.m_nType = 0;
				beamInfo.m_pszModelName = XOR("sprites/purplelaser1.vmt");
				beamInfo.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/purplelaser1.vmt"));
				beamInfo.m_flHaloScale = 0.0f;
				beamInfo.m_flLife = 3.f;
				beamInfo.m_flWidth = 4.0f;
				beamInfo.m_flEndWidth = 4.0f;
				beamInfo.m_flFadeLength = 0.0f;
				beamInfo.m_flAmplitude = 2.0f;
				beamInfo.m_flBrightness = color.a();
				beamInfo.m_flSpeed = 0.2f;
				beamInfo.m_nStartFrame = 0;
				beamInfo.m_flFrameRate = 0.f;
				beamInfo.m_flRed = color.r();
				beamInfo.m_flGreen = color.g();
				beamInfo.m_flBlue = color.b();
				beamInfo.m_nSegments = 2;
				beamInfo.m_bRenderable = true;
				beamInfo.m_nFlags = 0x100 | 0x200 | 0x8000;
			}
			else if (g_menu.main.visuals.beam_type.get() == 1) {
				beamInfo.m_vecStart = start;
				beamInfo.m_vecEnd = pos;
				beamInfo.m_nType = 0;
				beamInfo.m_pszModelName = XOR("sprites/physbeam.vmt");
				beamInfo.m_nModelIndex = g_csgo.m_model_info->GetModelIndex(XOR("sprites/physbeam.vmt"));
				beamInfo.m_flHaloScale = 0.0f;
				beamInfo.m_flLife = 3.f;
				beamInfo.m_flWidth = 1.0f;
				beamInfo.m_flEndWidth = 1.f;
				beamInfo.m_flFadeLength = 0.0f;
				beamInfo.m_flAmplitude = 0.f;
				beamInfo.m_flBrightness = color.a();
				beamInfo.m_flSpeed = g_menu.main.visuals.speed_beam.get();
				beamInfo.m_nStartFrame = 0;
				beamInfo.m_flFrameRate = 0.f;
				beamInfo.m_flRed = color.r();
				beamInfo.m_flGreen = color.g();
				beamInfo.m_flBlue = color.b();
				beamInfo.m_nSegments = 2;
				beamInfo.m_bRenderable = true;
			}

			// attempt to render the beam.
			beam = g_csgo.m_beams->CreateBeamPoints(beamInfo);
			if (beam) {
				g_csgo.m_beams->DrawBeam(beam);
			}
		}
	}

	int attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (attacker != g_csgo.m_engine->GetLocalPlayer())
		return;

	Player* shooter = g_csgo.m_entlist->GetClientEntity<Player*>(g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt()));
	if (!shooter || shooter != g_cl.m_local)
		return;

	ImpactInfo info;
	info.x = evt->GetFloat(XOR("x"));
	info.y = evt->GetFloat(XOR("y"));
	info.z = evt->GetFloat(XOR("z"));
	info.time = g_csgo.m_globals->m_curtime;
	g_visuals.m_impacts.push_back(info);

	vec3_t pos = {
			evt->m_keys->FindKey(HASH("x"))->GetFloat(),
			evt->m_keys->FindKey(HASH("y"))->GetFloat(),
			evt->m_keys->FindKey(HASH("z"))->GetFloat()
	};

	// draw our (server-side) bullet impacts.
	if (g_menu.main.visuals.bullet_impacts.get())
		g_csgo.m_debug_overlay->AddBoxOverlay(vec3_t(info.x, info.y, info.z), vec3_t(-2, -2, -2), vec3_t(2, 2, 2), ang_t(0, 0, 0), 0, 0, 255, 125, 4.f);
}

void events::item_purchase(IGameEvent* evt) {
	int           team, purchaser;
	player_info_t info;

	if (!g_cl.m_local || !evt)
		return;

	if (!g_menu.main.misc.logs_buy.get())
		return;

	// only log purchases of the enemy team.
	team = evt->m_keys->FindKey(HASH("team"))->GetInt();
	if (team == g_cl.m_local->m_iTeamNum())
		return;

	// get the player that did the purchase.
	purchaser = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());

	// get player info of purchaser.
	if (!g_csgo.m_engine->GetPlayerInfo(purchaser, &info))
		return;

	std::string weapon = evt->m_keys->FindKey(HASH("weapon"))->m_string;
	if (weapon == XOR("weapon_unknown"))
		return;

	std::string out = tfm::format(XOR("%s bought %s\n"), std::string{ info.m_name }.substr(0, 24), weapon);
	g_notify.add(out);
}



void events::player_death(IGameEvent* evt) {
	// get index of player that died.
	int index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	int attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt());
}

void events::player_given_c4(IGameEvent* evt) {
	player_info_t info;

	// get the player who received the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (index == g_csgo.m_engine->GetLocalPlayer())
		return;

	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;
}

void events::bomb_beginplant(IGameEvent* evt) {
	player_info_t info;

	// get the player who played the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (index == g_csgo.m_engine->GetLocalPlayer())
		return;

	// get player info of purchaser.
	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;
}

void events::bomb_abortplant(IGameEvent* evt) {
	player_info_t info;

	// get the player who stopped planting the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (index == g_csgo.m_engine->GetLocalPlayer())
		return;

	// get player info of purchaser.
	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

}

void events::bomb_planted(IGameEvent* evt) {
	Entity* bomb_target;
	std::string   site_name;
	int           player_index;
	player_info_t info;
	std::string   out;

	// get the func_bomb_target entity and store info about it.
	bomb_target = g_csgo.m_entlist->GetClientEntity(evt->m_keys->FindKey(HASH("site"))->GetInt());
	if (bomb_target) {
		auto origin = bomb_target->GetAbsOrigin();
		const auto& bomb_a = (*g_cl.m_resource)->m_bombsiteCenterA();
		const auto& bomb_b = (*g_cl.m_resource)->m_bombsiteCenterB();
		const auto dist_a = origin.Distance(bomb_a);
		const auto dist_b = origin.Distance(bomb_b);
		site_name = dist_a < dist_b ? XOR("A") : XOR("B");
		g_visuals.m_last_bombsite = site_name;
	}

	player_index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (player_index == g_csgo.m_engine->GetLocalPlayer())
		out = tfm::format(XOR("you planted the bomb at %s\n"), site_name.c_str());

	else {
		g_csgo.m_engine->GetPlayerInfo(player_index, &info);

		out = tfm::format(XOR("the bomb was planted at %s by %s\n"), site_name.c_str(), std::string(info.m_name).substr(0, 24));
	}

}

void events::bomb_beep(IGameEvent* evt) {
	Entity* c4;
	vec3_t             explosion_origin, explosion_origin_adjusted;
	CTraceFilterSimple filter;
	CGameTrace         tr;

	// we have a bomb ent already, don't do anything else.
	if (g_visuals.m_c4_planted)
		return;

	// bomb_beep is called once when a player plants the c4 and contains the entindex of the C4 weapon itself, we must skip that here.
	c4 = g_csgo.m_entlist->GetClientEntity(evt->m_keys->FindKey(HASH("entindex"))->GetInt());
	if (!c4 || !c4->is(HASH("CPlantedC4")))
		return;

	// planted bomb is currently active, grab some extra info about it and set it for later.
	g_visuals.m_c4_planted = true;
	g_visuals.m_planted_c4 = c4;
	g_visuals.m_planted_c4_explode_time = c4->m_flC4Blow();

	// the bomb origin is adjusted slightly inside CPlantedC4::C4Think, right when it's about to explode.
	// we're going to do that here.
	explosion_origin = c4->GetAbsOrigin();
	explosion_origin_adjusted = explosion_origin;
	explosion_origin_adjusted.z += 8.f;

	// setup filter and do first trace.
	filter.SetPassEntity(c4);

	g_csgo.m_engine_trace->TraceRay(
		Ray(explosion_origin_adjusted, explosion_origin_adjusted + vec3_t(0.f, 0.f, -40.f)),
		MASK_SOLID,
		&filter,
		&tr
	);

	// pull out of the wall a bit.
	if (tr.m_fraction != 1.f)
		explosion_origin = tr.m_endpos + (tr.m_plane.m_normal * 0.6f);

	// this happens inside CCSGameRules::RadiusDamage.
	explosion_origin.z += 1.f;

	// set all other vars.
	g_visuals.m_planted_c4_explosion_origin = explosion_origin;

	// todo - dex;  get this radius dynamically... seems to only be available in map bsp file, search string: "info_map_parameters"
	//              info_map_parameters is an entity created on the server, it doesnt seem to have many useful networked vars for clients.
	//
	//              swapping maps between de_dust2 and de_nuke and scanning for 500 and 400 float values will leave you one value.
	//              need to figure out where it's written from.
	//
	// server.dll uses starting 'radius' as damage... the real radius passed to CCSGameRules::RadiusDamage is actually multiplied by 3.5.
	g_visuals.m_planted_c4_damage = 500.f;
	g_visuals.m_planted_c4_radius = g_visuals.m_planted_c4_damage * 3.5f;
	g_visuals.m_planted_c4_radius_scaled = g_visuals.m_planted_c4_radius / 3.f;
}

void events::bomb_begindefuse(IGameEvent* evt) {
	player_info_t info;

	// get index of player that started defusing the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (index == g_csgo.m_engine->GetLocalPlayer())
		return;

	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

	bool kit = evt->m_keys->FindKey(HASH("haskit"))->GetBool();

	if (kit) {
		std::string out = tfm::format(XOR("%s started defusing with a kit\n"), std::string(info.m_name).substr(0, 24));
	}

	else {
		std::string out = tfm::format(XOR("%s started defusing without a kit\n"), std::string(info.m_name).substr(0, 24));
	}
}

void events::bomb_abortdefuse(IGameEvent* evt) {
	player_info_t info;

	// get index of player that stopped defusing the bomb.
	int index = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (index == g_csgo.m_engine->GetLocalPlayer())
		return;

	if (!g_csgo.m_engine->GetPlayerInfo(index, &info))
		return;

	std::string out = tfm::format(XOR("%s stopped defusing\n"), std::string(info.m_name).substr(0, 24));
	g_notify.add(out);
}

void events::bomb_exploded(IGameEvent* evt) {
	g_visuals.m_c4_planted = false;
	g_visuals.m_planted_c4 = nullptr;
}

void events::bomb_defused(IGameEvent* evt) {
	g_visuals.m_c4_planted = false;
	g_visuals.m_planted_c4 = nullptr;
}

void events::weapon_fire(IGameEvent* evt) {
	g_shots.OnWeaponFire(evt);
}

void Listener::init() {
	// link events with callbacks.
	add(XOR("round_start"), events::round_start);
	add(XOR("round_end"), events::round_end);
	add(XOR("player_hurt"), events::player_hurt);
	add(XOR("bullet_impact"), events::bullet_impact);
	add(XOR("item_purchase"), events::item_purchase);
	add(XOR("player_death"), events::player_death);
	add(XOR("player_given_c4"), events::player_given_c4);
	add(XOR("bomb_beginplant"), events::bomb_beginplant);
	add(XOR("bomb_abortplant"), events::bomb_abortplant);
	add(XOR("bomb_planted"), events::bomb_planted);
	add(XOR("bomb_beep"), events::bomb_beep);
	add(XOR("bomb_begindefuse"), events::bomb_begindefuse);
	add(XOR("bomb_abortdefuse"), events::bomb_abortdefuse);
	add(XOR("bomb_exploded"), events::bomb_exploded);
	add(XOR("bomb_defused"), events::bomb_defused);
	add(XOR("weapon_fire"), events::weapon_fire);

	register_events();
}