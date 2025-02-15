#include "includes.h"
#include "playsoundf.h"

Client g_cl{ };

// init routine.
ulong_t __stdcall Client::init(void* arg) {
	// stop here if we failed to acquire all the data needed from csgo.
	if (!g_csgo.init())
		return 0;

	// welcome the user.
	g_notify.add(tfm::format(XOR("welcome %s\n"), g_cl.m_user));

	//PlaySoundA(reinterpret_cast<char*>(soundh), nullptr, SND_ASYNC | SND_MEMORY);

	g_cl.UnlockHiddenConvars();

	callbacks::ConfigLoadSkins();

	return 1;
}

void Client::UnlockHiddenConvars()
{
	// crashing :(

	if (g_csgo.m_cvar && !g_cl.m_unlocked) {

		ConVar* p = **reinterpret_cast<ConVar***> (g_csgo.m_cvar + 0x34);
		for (ConVar* c = p->m_next; c != nullptr; c = c->m_next) {
			c->m_flags &= ~FCVAR_DEVELOPMENTONLY;
			c->m_flags &= ~FCVAR_HIDDEN;
			c->m_flags &= ~FCVAR_CHEAT;
		}

		g_cl.m_unlocked = true;
	}
}

void Client::DrawHUD() {
	if (!g_csgo.m_engine->IsInGame())
		return;

	// get time.
	time_t t = std::time(nullptr);
	std::ostringstream time;
	time << std::put_time(std::localtime(&t), ("%H:%M:%S"));

	// get round trip time in milliseconds.
	int ms = std::max(0, (int)std::round(g_cl.m_latency * 1000.f));

	// get tickrate.
	int rate = (int)std::round(1.f / g_csgo.m_globals->m_interval);

	std::string text = tfm::format(XOR("freakmacy | rtt: %ims"), ms);
	render::FontSize_t size = render::hud.size(text);

	// background.
	render::rect_filled(m_width - size.m_width - 20, 10, size.m_width + 10, size.m_height + 2, { 240, 110, 140, 130 });

	// text.
	render::hud.string(m_width - 15, 10, { 240, 160, 180, 250 }, text, render::ALIGN_RIGHT);
}

void Client::KillFeed() {
	if (!g_menu.main.misc.preserve_killfeed.get())
		return;

	if (!g_csgo.m_engine->IsInGame())
		return;

	// get the addr of the killfeed.
	KillFeed_t* feed = (KillFeed_t*)g_csgo.m_hud->FindElement(HASH("SFHudDeathNoticeAndBotStatus"));
	if (!feed)
		return;

	int size = feed->notices.Count();
	if (!size)
		return;

	for (int i{ }; i < size; ++i) {
		NoticeText_t* notice = &feed->notices[i];

		// this is a local player kill, delay it.
		if (notice->fade == 1.5f)
			notice->fade = FLT_MAX;
	}
}

void Client::Cvars() {
		g_csgo.sv_party_mode->SetValue(1);
}

float old_volume = g_menu.main.misc.hitsound_volume.get();
int old_sound = g_menu.main.misc.hitsound_name.get();

void Client::OnPaint() {
	// update screen size.
	g_csgo.m_engine->GetScreenSize(m_width, m_height);

	// render stuff.
	g_visuals.think();
	g_grenades.paint();
	g_notify.think();

	if (old_volume != g_menu.main.misc.hitsound_volume.get() || old_sound != g_menu.main.misc.hitsound_name.get()) {
		g_cl.m_update_sound = true;
	}


	KillFeed();
	DrawHUD();

	// menu goes last.
	g_gui.think();
}

void Client::OnMapload() {
	// store class ids.
	g_netvars.SetupClassData();

	// createmove will not have been invoked yet.
	// but at this stage entites have been created.
	// so now we can retrive the pointer to the local player.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	// world materials.
	Visuals::ModulateWorld();
	m_tick_rate = static_cast<int>((1.0f) / g_csgo.m_globals->m_interval);

	// init knife shit.
	g_skins.load();

	g_cl.m_setupped = false;
	m_sequences.clear();

	// if the INetChannelInfo pointer has changed, store it for later.
	g_csgo.m_net = g_csgo.m_engine->GetNetChannelInfo();

	if (g_csgo.m_net) {
		g_hooks.m_net_channel.reset();
		g_hooks.m_net_channel.init(g_csgo.m_net);
		g_hooks.m_net_channel.add(INetChannel::PROCESSPACKET, util::force_cast(&Hooks::ProcessPacket));
		g_hooks.m_net_channel.add(INetChannel::SENDDATAGRAM, util::force_cast(&Hooks::SendDatagram));
		
		// testing
		g_hooks.m_net_channel.add(INetChannel::SENDNETMSG, util::force_cast(&Hooks::SendNetMsg));

		m_server_ip = g_csgo.m_net->GetAddress();

		g_notify.add(tfm::format("joining %s\n", m_server_ip));

		if (!strcmp(m_server_ip, "178.32.80.148:27015")) {
			g_notify.add("successfully connected to mrx - autostop works now, avoid fakewalking and manually shooting in order to prevent getting slayed.");
		}
	}
}

void Client::StartMove(CUserCmd* cmd) {
	// save some usercmd stuff.
	m_cmd = cmd;
	m_tick = cmd->m_tick;
	m_view_angles = cmd->m_view_angles;
	m_buttons = cmd->m_buttons;

	// get local ptr.
	m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());
	if (!m_local) {
		m_setupped = false;
		return;
	}

	if (m_local->m_fFlags() & FL_FROZEN || m_local->m_iTeamNum() < 2) {
		m_setupped = false;
	}

	m_pressing_move = (m_buttons & (IN_LEFT) || m_buttons & (IN_FORWARD) || m_buttons & (IN_BACK) ||
		m_buttons & (IN_RIGHT) || m_buttons & (IN_MOVELEFT) || m_buttons & (IN_MOVERIGHT) ||
		m_buttons & (IN_JUMP));

	// store max choke
	// TODO; 11 -> m_bIsValveDS
	m_max_lag = (m_local->m_fFlags() & FL_ONGROUND) ? 16 : 15;
	m_lag = g_csgo.m_cl->m_choked_commands;
	m_latency = g_csgo.m_net->GetLatency(INetChannel::FLOW_OUTGOING);
	m_latency2 = g_csgo.m_net->GetLatency(INetChannel::FLOW_INCOMING);
	m_latency_ticks = game::TIME_TO_TICKS(m_latency);
	math::clamp(m_latency, 0.f, 1.f);
	m_lerp = game::GetClientInterpAmount();
	m_server_tick = g_csgo.m_cl->m_server_tick;
	m_arrival_tick = m_server_tick + m_latency_ticks;
	m_flicking = false;

	Cvars();

	// processing indicates that the localplayer is valid and alive.
	m_processing = m_local && m_local->alive();
	if (!m_processing) {
		m_setupped = false;
		return;
	}

	// update our prediction
	// https://www.unknowncheats.me/forum/counterstrike-global-offensive/261943-fix-prediction-fps-lower-tickrate.html
	g_inputpred.update();

	// store some stuff about the local player.
	m_flags = m_local->m_fFlags();

	m_frame_changed = g_csgo.m_globals->m_frame != m_old_frame;
	m_old_frame = g_csgo.m_globals->m_frame;
	m_shot = false;
}

void Client::BackupPlayers(bool restore) {
	// restore stuff.
	for (int i{ 1 }; i <= g_csgo.m_globals->m_max_clients; ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!g_aimbot.IsValidTarget(player))
			continue;

		if (restore)
			g_aimbot.m_backup[i - 1].restore(player);
		else
			g_aimbot.m_backup[i - 1].store(player);
	}
}

void Client::DoMove() {
	// backup strafe angles (we need them for input prediction)
	m_strafe_angles = m_cmd->m_view_angles;
	g_movement.wish_angle = m_cmd->m_view_angles;

	// run movement code before input prediction.
	g_movement.JumpRelated();
	g_movement.Strafe(m_cmd, m_strafe_angles);
	g_movement.FakeWalk();
	g_movement.AutoStop();
	g_movement.FastStop();
	g_movement.AutoPeek(m_cmd, m_strafe_angles.y);

	if (!(m_flags & FL_ONGROUND) 
		&& g_input.GetKeyState(g_menu.main.movement.instant_stop_in_air.get()))
		g_aimbot.m_stop_air = true;

	// INSTANT STOP IN AIR
	if (g_aimbot.m_stop_air) {
		if (m_local->m_vecVelocity().length_2d() > 15.f)
			g_movement.NullVelocity(m_cmd);
		else
			m_cmd->m_move.x = m_cmd->m_move.y = 0.f;
	}
	
	g_aimbot.m_stop_air = false;

	auto old_angle = m_cmd->m_view_angles;

	// predict input.
	g_inputpred.run();
	
	// convert viewangles to directional forward vector.
	math::AngleVectors(m_view_angles, &m_forward_dir);

	//g_inputpred.UpdatePitch(g_cl.m_real_angle.x);
	g_cl.m_shoot_pos = g_cl.m_local->GetShootPosition(g_cl.m_matrix);

	// restore original angles after input prediction
	m_cmd->m_view_angles = old_angle;
	
	// reset shit.
	m_weapon = nullptr;
	m_weapon_info = nullptr;
	m_weapon_id = -1;
	m_weapon_type = WEAPONTYPE_UNKNOWN;
	m_player_fire = m_weapon_fire = false;
	
	// store weapon stuff.
	m_weapon = m_local->GetActiveWeapon();
	
	if (m_weapon) {
		m_weapon_info = m_weapon->GetWpnData();
		m_weapon_id = m_weapon->m_iItemDefinitionIndex();
		m_weapon_type = m_weapon_info->m_weapon_type;
	
	
		float damage = g_cl.m_weapon_info->m_damage;
		int remain_pen = 4;
		int hitgroup = HITGROUP_GENERIC;
		int hitbox = HITBOX_HEAD;
		bool visible = true;
	
		m_awall_hit = g_auto_wall.get()->fire_bullet(g_cl.m_weapon,
			g_cl.m_forward_dir,
			visible,
			damage,
			remain_pen,
			hitgroup,
			hitbox,
			nullptr,
			g_cl.m_weapon_info->m_range,
			g_cl.m_shoot_pos);
	
		m_adjusted_curtime = game::TICKS_TO_TIME(g_cl.m_local->m_nTickBase());
		m_awall_pen = remain_pen != 4;
	
		// can the player fire.
		m_player_fire = g_csgo.m_globals->m_curtime >= m_local->m_flNextAttack() && !g_csgo.m_gamerules->m_bFreezePeriod() && !(m_flags & FL_FROZEN);
	
		UpdateRevolverCock();
		m_weapon_fire = CanFireWeapon(g_csgo.m_globals->m_curtime);
	}
	
	// grenade prediction.
	g_grenades.think();
	
	// run fakelag.
	g_hvh.SendPacket();
	
	// run aimbot.
	g_aimbot.think();
	
	// run antiaims.
	g_hvh.AntiAim();
	
	// local anims
	update_shot_cmd();
}


void Client::print_clr(Color color, const std::string text, ...) {
	va_list     list;
	int         size;
	std::string buf;

	if (text.empty())
		return;

	va_start(list, text);

	// count needed size.
	size = std::vsnprintf(0, 0, text.c_str(), list);

	// allocate.
	buf.resize(size);

	// print to buffer.
	std::vsnprintf(buf.data(), size + 1, text.c_str(), list);

	va_end(list);

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf(g_gui.m_color, XOR("[freakmacy] "));
	g_csgo.m_cvar->ConsoleColorPrintf(color, buf.c_str());
}


void Client::EndMove(CUserCmd* cmd) {

	// fix animations after all movement related functions have been called
	//g_ServerAnimations.HandleAnimations(g_cl.m_packet, cmd);

	// anti untrust clamp.
	cmd->m_view_angles.SanitizeAngle();

	// fix our movement.
	g_movement.FixMove(cmd);
	g_aimbot.handle_fakelatency(g_csgo.m_globals->m_tick_count, g_aimbot.m_fake_latency2, g_menu.main.misc.sec_ping_spike_amt.get(), g_menu.main.misc.ping_spike_amt.get());

	// store this when choke cycle reset.
	if (!g_csgo.m_cl->m_choked_commands) {

		HandleAnimations();
		HandleBodyYaw();

		m_real_angle_old = m_real_angle;

		// get current origin.
		vec3_t cur = m_local->m_vecOrigin();

		// get prevoius origin.
		vec3_t prev = m_net_pos.empty() ? cur : m_net_pos.front().m_pos;

		// check if we broke lagcomp.
		m_lagcomp = (cur - prev).length_sqr() > 4096.f;

		// save sent origin and time.
		m_net_pos.emplace_front(g_csgo.m_globals->m_curtime, cur);

		// why not do this on first choked command so it does funny fluctuate
		g_hvh.m_step_switch = (bool)g_csgo.RandomInt(0, 1);

		// save our speed on anim update
		g_cl.m_speed = g_cl.m_local->m_vecVelocity().length_2d();
	}

	// this packet will be sent.
	if (*m_packet) {

		m_old_lag = m_lag;
		m_old_lagcomp = m_lagcomp;
		g_cl.m_sideways = !g_cl.m_sideways;

		m_cmds.push_back(cmd->m_command_number);

		// get radar angles.
		m_radar = cmd->m_view_angles;
		m_radar.normalize();
	}

	// store some values for next tick.
	m_old_packet = *m_packet;
	m_old_shot = m_shot;
	m_old_flags = m_flags;
}


void Client::OnTick(CUserCmd* cmd) {
	// TODO; add this to the menu.
	if (g_menu.main.misc.ranks.get() && cmd->m_buttons & IN_SCORE) {
		static CCSUsrMsg_ServerRankRevealAll msg{ };
		g_csgo.ServerRankRevealAll(&msg);
	}

	// store some data and update prediction.
	StartMove(cmd);

	// not much more to do here.
	if (!m_processing)
		return;

	// save the original state of players.
	BackupPlayers(false);

	// run all movement related code.
	DoMove();

	// store stome additonal stuff for next tick
	// sanetize our usercommand if needed and fix our movement.
	EndMove(cmd);

	// restore the players.
	BackupPlayers(true);

	// restore input
	g_inputpred.restore();

	if (*m_packet) {
		g_cl.m_outgoing_cmd_nums.emplace_front(cmd->m_command_number);
	}
	else {
		auto nc = g_csgo.m_cl->m_net_channel;

		if (nc) {
			const int backup_choked = nc->m_choked_packets;

			nc->m_choked_packets = 0;
			nc->SendDatagram(nullptr);
			--*(int*)(std::uintptr_t(nc) + 0x18);
			--*(int*)(std::uintptr_t(nc) + 0x2C);
			nc->m_choked_packets = backup_choked;
		}
	}
}

void Client::print(const std::string text, ...) {
	va_list     list;
	int         size;
	std::string buf;

	if (text.empty())
		return;

	va_start(list, text);

	// count needed size.
	size = std::vsnprintf(0, 0, text.c_str(), list);

	// allocate.
	buf.resize(size);

	// print to buffer.
	std::vsnprintf(buf.data(), size + 1, text.c_str(), list);

	va_end(list);

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf(g_gui.m_color, XOR("[freakmacy] "));
	g_csgo.m_cvar->ConsoleColorPrintf(colors::white, buf.c_str());
}

void Client::printcustom(Color color, const std::string text, ...) {
	va_list     list;
	int         size;
	std::string buf;

	if (text.empty())
		return;

	va_start(list, text);

	// count needed size.
	size = std::vsnprintf(0, 0, text.c_str(), list);

	// allocate.
	buf.resize(size);

	// print to buffer.
	std::vsnprintf(buf.data(), size + 1, text.c_str(), list);

	va_end(list);

	// print to console.
	g_csgo.m_cvar->ConsoleColorPrintf(color, buf.c_str());
}


bool Client::CanFireWeapon( float curtime ) {

	// the player cant fire.
	if (!m_player_fire)
		return false;

	if (m_weapon_type == WEAPONTYPE_GRENADE)
		return false;

	// if we have no bullets, we cant shoot.
	if (m_weapon_type != WEAPONTYPE_KNIFE && m_weapon->m_iClip1() < 1)
		return false;

	// do we have any burst shots to handle?
	if ((m_weapon_id == GLOCK || m_weapon_id == FAMAS) && m_weapon->m_iBurstShotsRemaining() > 0) {
		// new burst shot is coming out.
		if (curtime >= m_weapon->m_fNextBurstShot())
			return true;
	}

	// r8 revolver.
	if (m_weapon_id == REVOLVER) {
		int act = m_weapon->m_Activity();

		// mouse1.
		if (!m_revolver_fire) {
			if ((act == 185 || act == 193) && m_revolver_cock == 0)
				return curtime >= m_weapon->m_flNextPrimaryAttack();

			return false;
		}
	}

	// yeez we have a normal gun.
	if (curtime >= m_weapon->m_flNextPrimaryAttack())
		return true;

	return false;
}

void Client::UpdateRevolverCock() {
	// default to false.
	m_revolver_fire = false;

	// reset properly.
	if (m_revolver_cock == -1)
		m_revolver_cock = 0;

	// we dont have a revolver.
	// we have no ammo.
	// player cant fire
	// we are waiting for we can shoot again.
	if (m_weapon_id != REVOLVER || m_weapon->m_iClip1() < 1 || !m_player_fire || g_csgo.m_globals->m_curtime < m_weapon->m_flNextPrimaryAttack()) {
		// reset.
		m_revolver_cock = 0;
		m_revolver_query = 0;
		return;
	}

	// calculate max number of cocked ticks.
	// round to 6th decimal place for custom tickrates..
	int shoot = (int)(0.25f / (std::round(g_csgo.m_globals->m_interval * 1000000.f) / 1000000.f));

	// amount of ticks that we have to query.
	m_revolver_query = shoot - 1;

	// we held all the ticks we needed to hold.
	if (m_revolver_query == m_revolver_cock) {
		// reset cocked ticks.
		m_revolver_cock = -1;

		// we are allowed to fire, yay.
		m_revolver_fire = true;
	}

	else {
		// we still have ticks to query.
		// apply inattack.
		if (m_revolver_query > m_revolver_cock)
			m_cmd->m_buttons |= IN_ATTACK;

		// count cock ticks.
		// do this so we can also count 'legit' ticks
		// that didnt originate from the hack.
		if (m_cmd->m_buttons & IN_ATTACK)
			m_revolver_cock++;

		// inattack was not held, reset.
		else m_revolver_cock = 0;
	}

	// remove inattack2 if cocking.
	if (m_revolver_cock > 0)
		m_cmd->m_buttons &= ~IN_ATTACK2;
}

void Client::UpdateIncomingSequences() {
	if (!g_csgo.m_net)
		return;

	if (m_sequences.empty() || g_csgo.m_net->m_in_seq > m_sequences.front().m_seq) {
		// store new stuff.
		m_sequences.emplace_front(g_csgo.m_globals->m_realtime, g_csgo.m_net->m_in_rel_state, g_csgo.m_net->m_in_seq);
	}

	// do not save too many of these.
	while (m_sequences.size() > 2048)
		m_sequences.pop_back();
}