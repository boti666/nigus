#include "includes.h"

void Hooks::LevelInitPreEntity(const char* map) {
	float rate{ 1.f / g_csgo.m_globals->m_interval };

	// set rates when joining a server.
	g_csgo.cl_updaterate->SetValue(rate);
	g_csgo.cl_cmdrate->SetValue(rate);

	g_aimbot.reset();
	g_cl.m_setupped = false;
	g_visuals.m_hit_start = g_visuals.m_hit_end = g_visuals.m_hit_duration = 0.f;

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPreEntity_t >(CHLClient::LEVELINITPREENTITY)(this, map);
}

void Hooks::LevelInitPostEntity() {
	g_cl.OnMapload();
	g_cl.m_cmds.clear();
	g_cl.m_setupped = false;

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelInitPostEntity_t >(CHLClient::LEVELINITPOSTENTITY)(this);
}

void Hooks::LevelShutdown() {
	g_aimbot.reset();

	g_cl.m_local = nullptr;
	g_cl.m_weapon = nullptr;
	g_cl.m_processing = false;
	g_cl.m_weapon_info = nullptr;
	g_cl.m_round_end = false;
	g_cl.m_setupped = false;

	g_cl.m_sequences.clear();

	// invoke original method.
	g_hooks.m_client.GetOldMethod< LevelShutdown_t >(CHLClient::LEVELSHUTDOWN)(this);
}

struct var_map_entry_t {
public:
	unsigned short type;
	unsigned short need_interp;
	void* data;
	void* watcher;
};

struct varmapping_t {
	varmapping_t() {
		interp_entries = 0;
	}

	var_map_entry_t* entries;
	int interp_entries;
	float last_interp_time;
};

varmapping_t* get_var_map(void* base_ent) {
	return reinterpret_cast<varmapping_t*>((DWORD)base_ent + 0x24); //0x4C );
}

void disable_interpolation(Player* ent, bool interp) {
	varmapping_t* map = get_var_map(ent);

	if (!map)
		return;

	for (int i = 0; i < map->interp_entries; i++) {
		var_map_entry_t* e = &map->entries[i];
		e->need_interp = false;
	}
}

void draw_server_hitboxes(int index)
{
	auto get_player_by_index = [](int index) -> Player*
		{ //i dont need this shit func for anything else so it can be lambda
			typedef Player* (__fastcall* player_by_index)(int);
			static auto player_index = pattern::find(g_csgo.m_server_dll, XOR("85 C9 7E 2A A1")).as<player_by_index>();

			if (!player_index)
				return false;

			return player_index(index);
		};

	static auto fn = pattern::find(g_csgo.m_server_dll, XOR("55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE"));
	auto duration = -1.f;
	PVOID entity = nullptr;

	entity = get_player_by_index(index);

	if (!entity)
		return;

	__asm {
		pushad
		movss xmm1, duration
		push 0 // 0 - colored, 1 - blue
		mov ecx, entity
		call fn
		popad
	}
}

void Hooks::FrameStageNotify(Stage_t stage) {

	enum PostProcessParameterNames_t
	{
		PPPN_FADE_TIME = 0,
		PPPN_LOCAL_CONTRAST_STRENGTH,
		PPPN_LOCAL_CONTRAST_EDGE_STRENGTH,
		PPPN_VIGNETTE_START,
		PPPN_VIGNETTE_END,
		PPPN_VIGNETTE_BLUR_STRENGTH,
		PPPN_FADE_TO_BLACK_STRENGTH,
		PPPN_DEPTH_BLUR_FOCAL_DISTANCE,
		PPPN_DEPTH_BLUR_STRENGTH,
		PPPN_SCREEN_BLUR_STRENGTH,
		PPPN_FILM_GRAIN_STRENGTH,

		POST_PROCESS_PARAMETER_COUNT
	};

	struct PostProcessParameters_t
	{
		PostProcessParameters_t()
		{
			memset(m_flParameters, 0, sizeof(m_flParameters));
			m_flParameters[PPPN_VIGNETTE_START] = 0.8f;
			m_flParameters[PPPN_VIGNETTE_END] = 1.1f;
		}

		float m_flParameters[POST_PROCESS_PARAMETER_COUNT];
	};

	// save stage.
	if (stage != FRAME_START)
		g_cl.m_stage = stage;

	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	if (g_menu.main.visuals.removals.get(4)) {
		static auto PostProcessParameters = *reinterpret_cast<PostProcessParameters_t**>((uintptr_t)pattern::find(g_csgo.m_client_dll, ("0F 11 05 ? ? ? ? 0F 10 87")) + 3);
		static float backupblur = PostProcessParameters->m_flParameters[PPPN_VIGNETTE_BLUR_STRENGTH];

		float blur = g_menu.main.visuals.removals.get(4) ? 0.f : backupblur;
		if (PostProcessParameters->m_flParameters[PPPN_VIGNETTE_BLUR_STRENGTH] != blur)
			PostProcessParameters->m_flParameters[PPPN_VIGNETTE_BLUR_STRENGTH] = blur;
	}

	if (stage == FRAME_RENDER_START) {
		g_visuals.ImpactData();
		g_shots.Think();
		g_cl.RestoreData();
	}
	
	static bool turnedon = false;
	if (g_menu.main.visuals.world.get(2)) {
		Color clr = g_menu.main.visuals.ambient_color.get();
		turnedon = false;
		if (g_csgo.mat_ambient_light_r->GetFloat() != clr.r())
			g_csgo.mat_ambient_light_r->SetValue(clr.r() / g_menu.main.visuals.ambient_alpha.get());

		if (g_csgo.mat_ambient_light_g->GetFloat() != clr.g())
			g_csgo.mat_ambient_light_g->SetValue(clr.g() / g_menu.main.visuals.ambient_alpha.get());
		
		if (g_csgo.mat_ambient_light_b->GetFloat() != clr.b())
			g_csgo.mat_ambient_light_b->SetValue(clr.b() / g_menu.main.visuals.ambient_alpha.get());
	}
	else {
		if (!turnedon) {
			g_csgo.mat_ambient_light_r->SetValue(0.f);
			g_csgo.mat_ambient_light_g->SetValue(0.f);
			g_csgo.mat_ambient_light_b->SetValue(0.f);
			turnedon = true;
		}
	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		/*const auto viewmodel = g_csgo.m_entlist->GetClientEntityFromHandle< ViewModel* >(g_cl.m_local->m_hViewModel());

		if (g_cl.m_local && g_cl.m_local->m_hViewModel() != 0xFFFFFFF) {
			// restore viewmodel when model renders a scene
			viewmodel->m_flCycle() = g_inputpred.m_weapon_cycle;
			viewmodel->m_nSequence() = g_inputpred.m_weapon_sequence;
			viewmodel->m_flAnimTime() = g_inputpred.m_weapon_anim_time;
		}*/
	}

	// call og.
	g_hooks.m_client.GetOldMethod< FrameStageNotify_t >(CHLClient::FRAMESTAGENOTIFY)(this, stage);

	if (stage == FRAME_RENDER_START) {
		g_visuals.DrawBeams();
	}


	else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START) {

		g_skins.think();
	}

	else if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_END) {
		g_visuals.NoSmoke();
	}

	else if (stage == FRAME_NET_UPDATE_END) {

		// update all players.
		for (int i{1}; i <= g_csgo.m_globals->m_max_clients; ++i) {
			Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);
			if (!player || player->m_bIsLocalPlayer())
				continue;

			AimPlayer* data = &g_aimbot.m_players[i - 1];
			data->OnNetUpdate(player);
		}
		g_VoiceCommunication->RunSharedESP();
	}
}