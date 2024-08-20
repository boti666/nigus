#include "includes.h"
#include "hitsounds.h"
#include "playsoundapi.h"

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "Winmm.lib")
#pragma comment( lib, "Iphlpapi.lib" )
using namespace std::pmr;

Shots g_shots{ };

static int ClipRayToHB(const Ray& ray, mstudiobbox_t* hitbox, matrix3x4_t& matrix, CGameTrace& trace)
{
	static auto fn = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 F3 0F 10 42"));

	if (!fn || !hitbox)
		return -1;

	trace.m_fraction = 1.0f;
	trace.m_startsolid = false;

	return fn.as<int(__fastcall*)(const Ray&, mstudiobbox_t*, matrix3x4_t&, CGameTrace&)>()(ray, hitbox, matrix, trace);
}


void Shots::OnImpact(IGameEvent* evt)
{
	int        attacker;
	vec3_t     pos, dir, start, end;
	float      time;
	CGameTrace trace;

	// screw this.
	if (!evt || !g_cl.m_local)
		return;

	// get attacker, if its not us, screw it.
	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (attacker != g_csgo.m_engine->GetLocalPlayer())
		return;

	// decode impact coordinates and convert to vec3.
	pos = {
		evt->GetFloat(XOR("x")),
		evt->GetFloat(XOR("y")),
		evt->GetFloat(XOR("z"))
	};

	// get prediction time at this point.
	time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());

	if (const auto shot = last_unprocessed())
		shot->m_server_info.m_impact_pos = pos;
}

void Shots::OnHurt(IGameEvent* evt) {
	int attacker, victim, group, hp;
	float time, damage;
	std::string name;

	if (!evt || !g_cl.m_local)
		return;

	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("attacker"))->GetInt());
	victim = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());

	// skip invalid player indexes.
	// should never happen? world entity could be attacker, or a nade that hits you.
	if (attacker < 1 || attacker > 64 || victim < 1 || victim > 64)
		return;

	// we were not the attacker or we hurt ourselves.
	else if (attacker != g_csgo.m_engine->GetLocalPlayer() || victim == g_csgo.m_engine->GetLocalPlayer())
		return;

	// get hitgroup.
	// players that get naded ( DMG_BLAST ) or stabbed seem to be put as HITGROUP_GENERIC.
	group = evt->m_keys->FindKey(HASH("hitgroup"))->GetInt();

	// invalid hitgroups ( note - dex; HITGROUP_GEAR isn't really invalid, seems to be set for hands and stuff? ).
	if (group == HITGROUP_GEAR)
		return;

	// get the player that was hurt.
	Player* target = g_csgo.m_entlist->GetClientEntity<Player*>(victim);
	if (!target)
		return;

	// get player info.
	player_info_t info;
	if (!g_csgo.m_engine->GetPlayerInfo(victim, &info))
		return;

	// get player name;
	name = std::string(info.m_name).substr(0, 24);

	// get damage reported by the server.
	damage = (float)evt->m_keys->FindKey(HASH("dmg_health"))->GetInt();

	// get remaining hp.
	hp = evt->m_keys->FindKey(HASH("health"))->GetInt();

	// get prediction time at this point.
	time = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
	Color clr;
	// hitmarker.
	if (callbacks::hitmarker_on()) {
		g_visuals.m_hit_duration = 1.f;
		g_visuals.m_hit_start = g_csgo.m_globals->m_curtime;
		g_visuals.m_hit_end = g_visuals.m_hit_start + g_visuals.m_hit_duration;
		if (hp < 1 && group == HITGROUP_HEAD)
			clr = colors::transparent_red;
		else if (hp < 1 && group != HITGROUP_HEAD)
			clr = colors::transparent_orange;
		else
			clr = colors::transparent_white;
		g_visuals.m_hit_color = clr;
	}

	if (g_menu.main.misc.increase.get()) {
		if (target->m_bHasHelmet() && group == HITGROUP_HEAD)
			g_csgo.m_effects->Sparks(target->GetHitboxPosition(group == HITGROUP_HEAD).value(), 2, 1);
	}

	if (g_menu.main.misc.hitmarker.get(3)) {
		g_csgo.m_sound->EmitAmbientSound(XOR("buttons/arena_switch_press_02.wav"), g_menu.main.misc.hitsound_volume.get() / 100.f);
	}

	else if (!g_cl.m_hitsounds.empty() && g_menu.main.misc.hitmarker.get(4)) {
		char* user = getenv("username");
		std::string str = std::string(user);
		// setup the sound's directory.
		std::string sound = std::string(XOR("C:\\nigus\\sounds")).append(g_cl.m_hitsounds[g_menu.main.misc.hitsound_name.get()]);
		if (sound.size() > 2) {
			auto ReadWavFileIntoMemory = [&](std::string fname, BYTE** pb, DWORD* fsize) {
				std::ifstream f(fname, std::ios::binary);

				f.seekg(0, std::ios::end);
				int lim = f.tellg();
				*fsize = lim;

				*pb = new BYTE[lim];
				f.seekg(0, std::ios::beg);

				f.read((char*)*pb, lim);

				f.close();
				};

			DWORD dwFileSize;
			BYTE* pFileBytes;
			ReadWavFileIntoMemory(sound.data(), &pFileBytes, &dwFileSize);

			// danke anarh1st47, ich liebe dich
			// dieses code snippet hat mir so sehr geholfen https://i.imgur.com/ybWTY2o.png
			// thanks anarh1st47, you are the greatest
			// loveeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee
			// kochamy anarh1st47
			auto modify_volume = [&](BYTE* bytes) {
				int offset = 0;
				for (int i = 0; i < dwFileSize / 2; i++) {
					if (bytes[i] == 'd' && bytes[i + 1] == 'a'
						&& bytes[i + 2] == 't' && bytes[i + 3] == 'a')
					{
						offset = i;
						break;
					}
				}

				if (!offset)
					return;

				BYTE* pDataOffset = (bytes + offset);
				DWORD dwNumSampleBytes = *(DWORD*)(pDataOffset + 4);
				DWORD dwNumSamples = dwNumSampleBytes / 2;

				SHORT* pSample = (SHORT*)(pDataOffset + 8);
				for (DWORD dwIndex = 0; dwIndex < dwNumSamples; dwIndex++)
				{
					SHORT shSample = *pSample;
					shSample = (SHORT)(shSample * (g_menu.main.misc.hitsound_volume.get() / 100.f));
					*pSample = shSample;
					pSample++;
					if (((BYTE*)pSample) >= (bytes + dwFileSize - 1))
						break;
				}
				};

			if (pFileBytes) {
				modify_volume(pFileBytes);
				PlaySoundA((LPCSTR)pFileBytes, NULL, SND_MEMORY | SND_ASYNC);
			}
		}
	}


	std::string damageType = evt->m_keys->FindKey(HASH("weapon"))->m_string;

	if (damageType == XOR("hegrenade"))
		g_notify.add(tfm::format("naded %s for %i damage (%i health remaining)\n", name, (int)damage, hp), colors::gray, 5.f);
	else if (damageType == XOR("inferno"))
		g_notify.add(tfm::format("burned %s for %i damage (%i health remaining)\n", name, (int)damage, hp), colors::gray, 5.f);
	else {
		g_notify.add(tfm::format("hit %s in the %s for %i damage (%i health remaining)\n", name, get_group(group), (int)damage, hp), colors::white, 5.f);
		g_cl.print("hit ");
		g_cl.printcustom(colors::red, tfm::format("%s ", name));
		g_cl.printcustom(colors::white, "in the ");
		g_cl.printcustom(colors::red, tfm::format("%s", get_group(group)));
		g_cl.printcustom(colors::white, " for ");
		g_cl.printcustom(colors::light_blue, tfm::format("%i", (int)damage));
		g_cl.printcustom(colors::white, " damage (");
		g_cl.printcustom(colors::light_blue, tfm::format("%i", hp));
		g_cl.printcustom(colors::white, " health remaining)\n");
	}

	const auto shot = last_unprocessed();
	if (!shot
		|| (shot->m_target && shot->m_target_index != victim))
		return;

	shot->m_server_info.m_hitgroup = group;
	shot->m_server_info.m_dmg = evt->GetInt(XOR("dmg_health"));
	shot->m_server_info.m_hurt_tick = g_csgo.m_cl->m_server_tick;
}

void Shots::OnWeaponFire(IGameEvent* evt)
{
	int        attacker;

	// screw this.
	if (!evt || !g_cl.m_local)
		return;

	// get attacker, if its not us, screw it.
	attacker = g_csgo.m_engine->GetPlayerForUserID(evt->m_keys->FindKey(HASH("userid"))->GetInt());
	if (attacker != g_csgo.m_engine->GetLocalPlayer())
		return;

	const auto shot = std::find_if(
		m_shots.begin(), m_shots.end(),
		[](const ShotRecord& shot)
		{
			return shot.m_cmd_number != -1 && !shot.m_server_info.m_fire_tick
				&& std::abs(g_csgo.m_cl->m_last_command_ack - shot.m_cmd_number) <= 20;
		}
	);

	if (shot == m_shots.end())
		return;

	// this shot was matched.
	shot->m_process_tick = g_csgo.m_globals->m_realtime + 2.5f;
	shot->m_server_info.m_fire_tick = g_csgo.m_cl->m_server_tick;
}

void Shots::Think()
{
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_local) {
		return m_shots.clear();
	}

	for (auto& shot : m_shots) {
		if (shot.m_processed
			|| g_csgo.m_globals->m_realtime > shot.m_process_tick)
			continue;

		if (shot.m_target
			&& shot.m_record) {

			if (!shot.m_target->alive()) {
			}
			else {
				AimPlayer* data = &g_aimbot.m_players[shot.m_target->index() - 1];
				if (!data)
					return;

				if (!shot.m_server_info.m_hurt_tick) {

					auto pModel = shot.m_target->GetModel();
					if (!pModel)
						return;

					auto pHdr = g_csgo.m_model_info->GetStudioModel(pModel);
					if (!pHdr)
						return;

					auto pHitboxSet = pHdr->GetHitboxSet(shot.m_target->m_nHitboxSet());

					if (!pHitboxSet)
						return;

					auto pHitbox = pHitboxSet->GetHitbox(shot.m_hitbox);

					if (!pHitbox)
						return;

					BackupRecord backup;
					backup.store(shot.m_target);

					// cache
					shot.m_record->cache();

					CGameTrace trace{};
					g_csgo.m_engine_trace->ClipRayToEntity(Ray(shot.m_src, shot.m_server_info.m_impact_pos), MASK_SHOT_HULL | CONTENTS_HITBOX, shot.m_target, &trace); //ClipRayToHB(Ray(shot.m_src, shot.m_server_info.m_impact_pos), pHitbox, shot.m_record->m_bones[pHitbox->m_bone], trace) >= 0;

					if (trace.m_entity != shot.m_target || !trace.m_entity) {
						g_notify.add(XOR("missed shot due to spread\n"));
						g_cl.print("missed shot due to ");
						g_cl.printcustom(colors::transparent_yellow, "spread\n");
					}
					else {

						size_t mode = shot.m_record->m_mode;
						int curr_mode_miss = 0;

						bool broke_lc = shot.m_record->m_broke_lc;

						// if we miss a shot on body update.
						// we can chose to stop shooting at them.
						if (mode == Resolver::Modes::RESOLVE_DATA) {
							if (shot.m_record->m_flag & Resolver::Flags::HIGH_LBY) {
								++data->m_high_lby_idx;
								curr_mode_miss = data->m_high_lby_idx;
							}
							else {
								++data->m_stand_move_idx;
								curr_mode_miss = data->m_stand_move_idx;
							}
						}
						else if (mode == Resolver::Modes::RESOLVE_NO_DATA) {
							++data->m_stand_no_move_idx;
							curr_mode_miss = data->m_stand_no_move_idx;
						}
						else if (mode == Resolver::Modes::RESOLVE_LBY) {
							++data->m_body_index;
							curr_mode_miss = data->m_body_index;

						}
						else if (mode == Resolver::Modes::RESOLVE_NO_FLICK) {
							++data->m_low_lby_idx;
							curr_mode_miss = data->m_low_lby_idx;
						}
						else if (mode == Resolver::Modes::RESOLVE_STAND) {
							++data->m_stand_index;
							curr_mode_miss = data->m_stand_index;
						}
						else if (mode == Resolver::Modes::RESOLVE_AIR) {
							++data->m_air_idx;
							curr_mode_miss = data->m_air_idx;

						}
						else if (mode == Resolver::Modes::RESOLVE_LBY_PRED) {
							// increment lby pred miss
							++data->m_body_pred_index;
							curr_mode_miss = data->m_body_pred_index;

							// if we mispredict it means hes not at his lby
							// in that case, blacklist lby 
							++data->m_body_index;
						}

						// we will not shoot this shitty mode twice
						if (shot.m_record->m_flag & Resolver::Flags::INVERT_FS)
							data->m_missed_invertfs = true;

						if (std::abs(math::AngleDiff(shot.m_record->m_back, shot.m_record->m_eye_angles.y)) <= 15.f)
							data->m_missed_back = true;

						if (std::abs(math::AngleDiff(shot.m_record->m_air_back, shot.m_record->m_eye_angles.y)) <= 15.f
							&& !(shot.m_record->m_pred_flags & FL_ONGROUND))
							data->m_missed_air_back = true;

						if (data->m_move_data.m_sim_time > 0.f && std::abs(math::AngleDiff(data->m_move_data.m_body, shot.m_record->m_eye_angles.y)) <= 15.f)
							data->m_missed_last = true;

						// if mode isnt lby nor walk
						if (mode != Resolver::Modes::RESOLVE_LBY
							&& mode != Resolver::Modes::RESOLVE_WALK) {

							const float diff = std::abs(math::AngleDiff(shot.m_record->m_body, shot.m_record->m_eye_angles.y));

							// but delta is really close
							// then lets pretend we missed it
							// so we dont shoot the same angle twice
							if (diff <= 10.f)
								++data->m_body_index;
						}

						std::string reason = "unknown";

						if (shot.m_record->m_velocity.length() > 0.1f
							&& shot.m_record->m_ground_for_two_ticks
							&& std::abs(math::AngleDiff(shot.m_record->m_body, shot.m_record->m_eye_angles.y)) < 10.f
							&& !shot.m_record->m_broke_lc) {
							reason = "animations";
							g_cl.print("missed shot due to ");
							g_cl.printcustom(colors::light_blue, "animations\n");
						}
						else
							reason = "resolver";
						g_cl.print("missed shot due to ");
						g_cl.printcustom(colors::orange, "resolver\n");

						g_notify.add(tfm::format("missed shot due to %s\n", reason), colors::white, 8.f);
					}

					backup.restore(shot.m_record->m_player);
				}
			}
		}

		shot.m_processed = true;
	}

	m_shots.erase(std::remove_if(
		m_shots.begin(), m_shots.end(),
		[&](const ShotRecord& shot)
		{
			if (!shot.m_processed
				&& shot.m_process_tick)
				return false;

			if (std::abs(g_csgo.m_globals->m_realtime - shot.m_shot_time) >= 2.5f)
				return true;

			if (shot.m_processed
				|| shot.m_process_tick
				|| shot.m_target_index == -1
				|| shot.m_cmd_number == -1
				|| std::abs(g_csgo.m_cl->m_last_command_ack - shot.m_cmd_number) <= 20)
				return false;

			const auto delta = std::abs(g_csgo.m_globals->m_realtime - shot.m_sent_time);
			if (delta > 2.5f)
				return true;

			if (delta <= shot.m_latency)
				return false;

			return true;
		}
	),
		m_shots.end()
	);
}