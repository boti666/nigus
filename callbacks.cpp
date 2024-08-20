#include "includes.h"

// execution callbacks..
// execution callbacks..
void callbacks::SkinUpdate() {
	g_skins.m_update = true;
}

void callbacks::ForceWeaponColor() {
	// p100 skin color changer fix; essentially create new material to cache from on skin. 
	// we should do this dynamically though, detecting current material and removing it from material cache list, so that it re-caches with our new skin colors

	ForceFullUpdate();
}

void callbacks::ForceFullUpdate() {
	//static DWORD tick{};
	//
	//if( tick != g_winapi.GetTickCount( ) ) {
	//	g_csgo.cl_fullupdate->m_callback( );
	//	tick = g_winapi.GetTickCount( );
	//

	g_csgo.m_cl->m_delta_tick = -1;
}

void callbacks::ConfigSaveSkins() {
	std::string file = XOR("skins.cfg");


	g_config.save(&g_menu.main, file, true);
	g_notify.add(tfm::format(XOR("saved skin config\n")));
}

void callbacks::ConfigLoadSkins() {
	std::string file = XOR("skins.cfg");

	g_config.load(&g_menu.main, file, true);
	g_notify.add(XOR("loaded skin config\n"));
}

bool callbacks::IsCustomHitsound() {
	return g_menu.main.misc.hitmarker.get(4);
}

void callbacks::HitsoundReload() {
	g_cl.m_update_sound = true;
}

bool callbacks::is_viewmodel_fov() {
	return g_menu.main.misc.viewmodel_fov.get();
}

bool callbacks::HasWeapon() {
	if (!g_csgo.m_engine->IsInGame() || !g_cl.m_processing)
		return false;

	return g_cl.m_weapon_id;
}

bool callbacks::prefer_body_on() {
	return g_menu.main.aimbot.stuff.get();
}

bool callbacks::autopeek_on() {
	return g_menu.main.movement.autopeek.get() != -1;
}

bool callbacks::hitmarker_on() {
	return g_menu.main.misc.hitmarker.get(0) || g_menu.main.misc.hitmarker.get(1);
}

void callbacks::UpdateSkinColor() {
	g_skins.m_update_color = true;
}

bool callbacks::quick_stop_on() {
	return g_menu.main.movement.quick_stop.get();
}

void callbacks::ToggleLeft() {
	g_hvh.m_left = !g_hvh.m_left;
	g_hvh.m_right = false;
	g_hvh.m_back = false;
	g_hvh.m_forward = false;
}

void callbacks::ToggleRight() {
	g_hvh.m_right = !g_hvh.m_right;
	g_hvh.m_left = false;
	g_hvh.m_back = false;
	g_hvh.m_forward = false;
}

void callbacks::ToggleBack() {
	g_hvh.m_back = !g_hvh.m_back;
	g_hvh.m_left = false;
	g_hvh.m_right = false;
	g_hvh.m_forward = false;
}

void callbacks::ToggleForward() {
	g_hvh.m_forward = !g_hvh.m_forward;
	g_hvh.m_left = false;
	g_hvh.m_right = false;
	g_hvh.m_back = false;
}

bool callbacks::is_fluctuate() {
	return g_menu.main.antiaim.lag_mode.get() == 4;
}

bool callbacks::droppedwpnson() {
	return g_menu.main.visuals.items.get() || g_menu.main.visuals.items_distance.get() || g_menu.main.visuals.itemsglow.get();
}

bool callbacks::droppedglowewpaon() {
	return g_menu.main.visuals.itemsglow.get();
}

bool callbacks::ammoon() {
	return g_menu.main.visuals.ammo.get();
}

bool callbacks::isbullettracer() {
	return g_menu.main.visuals.impact_beams.get();
}

bool callbacks::islaseron() {
	return g_menu.main.visuals.beam_type.get() == 1 && g_menu.main.visuals.impact_beams.get();
}

bool callbacks::grenadeson() {
	return g_menu.main.visuals.proj.get();
}

bool callbacks::bombon() {
	return g_menu.main.visuals.planted_c4.get();
}

bool callbacks::boxon() {
	return g_menu.main.players.box.get();
}

bool callbacks::healthon() {
	return g_menu.main.players.health.get();
}

bool callbacks::overridehealthon() {
	return g_menu.main.players.override_health.get() && g_menu.main.players.health.get();
}

bool callbacks::nameon() {
	return g_menu.main.players.name.get();
}

bool callbacks::weaponiconon() {
	return g_menu.main.players.weaponicon.get();
}



bool callbacks::is_skeleton_on() {
	return g_menu.main.players.skeleton.get(0);
}


bool callbacks::is_history_skeleton_on() {
	return g_menu.main.players.skeleton.get(1);
}

bool callbacks::are_oof_arrow_on() {
	return g_menu.main.players.offscreen.get();
}

bool callbacks::is_sound_esp_on() {
	return g_menu.main.players.footstep.get();
}

bool callbacks::lbytimeron() {
	return g_menu.main.players.lby_update.get();
}

bool callbacks::not_fluctuate_stand() {
	return g_menu.main.antiaim.yaw_stand.get() != 5;
}

bool callbacks::glowon() {
	return g_menu.main.players.glow.get();
}

bool callbacks::ammpespon() {
	return g_menu.main.players.ammo.get();
}

bool callbacks::trajectoryon() {
	return g_menu.main.visuals.tracers.get();
}

bool callbacks::peneton() {
	return g_menu.main.visuals.pen_crosshair.get();
}

void callbacks::ToggleForceBodyAim() {
	g_aimbot.m_force_body = !g_aimbot.m_force_body;
}

bool callbacks::IsPFBOn() {
	return true;
}

bool callbacks::is_shot_matrix_on() {
	return g_menu.main.aimbot.matrix_shot.get() != 0;
}

bool callbacks::IsAstopOn() {
	return g_menu.main.aimbot.quick_stop_mode.get() > 0;
}

bool callbacks::IsDelayShot() {
	return g_menu.main.aimbot.delay_shot.get(1);
}

bool callbacks::IsNightMode() {
	return g_menu.main.visuals.world.get(0);
}

bool callbacks::isambient() {
	return g_menu.main.visuals.world.get(2);
}

bool callbacks::iscustommolo() {
	return g_menu.main.visuals.modulate_molo.get();
}

void callbacks::ToggleThirdPerson( ) {
	g_visuals.m_thirdperson = !g_visuals.m_thirdperson;
}

bool callbacks::ToggleFakeWalkOn() {
	return true;
}

bool callbacks::IsProjectiles() {
	return g_menu.main.visuals.proj.get();
}

void callbacks::toggle_fake_ping( ) {
	g_aimbot.m_fake_latency = !g_aimbot.m_fake_latency;
	g_aimbot.m_fake_latency2 = false;
}

bool callbacks::is_fake_ping_bound() {
	return g_menu.main.misc.ping_spike.get();
}

bool callbacks::override_pistol_on() {
	return g_menu.main.aimbot.override_pistol.get();
}

bool callbacks::is_acc_boost() {
	return g_menu.main.aimbot.accuracy_boost.get();
}

bool callbacks::is_instant_stop_air_on() {
	return g_menu.main.movement.instant_stop_in_air.get() != -1;
}

void callbacks::toggle_sec_fake_ping() {
	g_aimbot.m_fake_latency2 = !g_aimbot.m_fake_latency2;
	g_aimbot.m_fake_latency = false;
}

bool callbacks::is_sec_fake_ping_bound() {
	return g_menu.main.misc.sec_ping_spike.get() != -1;
}

bool callbacks::is_auto_buy_on() {
	return g_menu.main.misc.auto_buy.get();
}

bool callbacks::isdynamiccolor() {
	return g_menu.main.misc.hitmarker.get(2);
}

void callbacks::clear_killfeed( ) {
    KillFeed_t* feed = ( KillFeed_t* )g_csgo.m_hud->FindElement( HASH( "SFHudDeathNoticeAndBotStatus" ) );
    if( feed )
        g_csgo.ClearNotices( feed );
}

void callbacks::SaveHotkeys( ) {
	g_config.SaveHotkeys( );
}

void callbacks::ConfigLoad1() {
	g_config.load(&g_menu.main, XOR("auto.cfg"));
	g_menu.main.misc.config.select(1 - 1);

	g_notify.add(tfm::format(XOR("loaded auto.cfg\n")));
}

void callbacks::ConfigLoad2() {
	g_config.load(&g_menu.main, XOR("scout.cfg"));
	g_menu.main.misc.config.select(2 - 1);
	g_notify.add(tfm::format(XOR("loaded scout.cfg\n")));
}

void callbacks::ConfigLoad3() {
	g_config.load(&g_menu.main, XOR("awp.cfg"));
	g_menu.main.misc.config.select(3 - 1);
	g_notify.add(tfm::format(XOR("loaded awp.cfg\n")));
}

void callbacks::ConfigLoad4() {
	g_config.load(&g_menu.main, XOR("pistol.cfg"));
	g_menu.main.misc.config.select(4 - 1);
	g_notify.add(tfm::format(XOR("loaded pistol.cfg\n")));
}

void callbacks::ConfigLoad5() {
	g_config.load(&g_menu.main, XOR("echo.cfg"));
	g_menu.main.misc.config.select(5 - 1);
	g_notify.add(tfm::format(XOR("loaded echo.cfg\n")));
}

void callbacks::ConfigLoad6() {
	g_config.load(&g_menu.main, XOR("foxtrot.cfg"));
	g_menu.main.misc.config.select(6 - 1);
	g_notify.add(tfm::format(XOR("loaded foxtrot.cfg\n")));
}


void callbacks::ConfigLoad7() {
	g_config.load(&g_menu.main, XOR("gamma.cfg"));
	g_menu.main.misc.config.select(7 - 1);
	g_notify.add(tfm::format(XOR("loaded gamma.cfg\n")));
}


void callbacks::ConfigLoad8() {
	g_config.load(&g_menu.main, XOR("hotel.cfg"));
	g_menu.main.misc.config.select(8 - 1);
	g_notify.add(tfm::format(XOR("loaded hotel.cfg\n")));
}


void callbacks::ConfigLoad9() {
	g_config.load(&g_menu.main, XOR("india.cfg"));
	g_menu.main.misc.config.select(9 - 1);
	g_notify.add(tfm::format(XOR("loaded india.cfg\n")));
}


void callbacks::ConfigLoad10() {
	g_config.load(&g_menu.main, XOR("juliett.cfg"));
	g_menu.main.misc.config.select(10 - 1);
	g_notify.add(tfm::format(XOR("loaded juliett.cfg\n")));
}

void callbacks::ConfigLoad() {
	std::string config = g_menu.main.misc.config.GetActiveItem();
	std::string file = tfm::format(XOR("%s.cfg"), config.data());

	g_config.load(&g_menu.main, file);
	g_notify.add(tfm::format(XOR("loaded config %s\n"), config.data()));
}

void callbacks::ConfigSave() {
	std::string config = g_menu.main.misc.config.GetActiveItem();
	std::string file = tfm::format(XOR("%s.cfg"), config.data());

	g_config.save(&g_menu.main, file);
	g_notify.add(tfm::format(XOR("saved config %s\n"), config.data()));
}

bool callbacks::is_hit_chance_on() {
	return g_menu.main.aimbot.hit_chance.get() > 0;
}

bool callbacks::EuSvList() {
	return g_menu.main.misc.serverlist.get() == 0;
}

bool callbacks::NaSvList() {
	return g_menu.main.misc.serverlist.get() == 1;
}

void callbacks::dicks2() {
	g_csgo.m_engine->ExecuteClientCmd("connect na.2018hvh.com:27015");
}

void callbacks::dickswm() {
	g_csgo.m_engine->ExecuteClientCmd("connect 104.128.58.125:27020");
}

void callbacks::mrx() {
	g_csgo.m_engine->ExecuteClientCmd("connect 178.32.80.148:27015");
}

void callbacks::mrx2() {
	g_csgo.m_engine->ExecuteClientCmd("connect 178.32.80.148:27030");
}

bool callbacks::IsMultipointOn( ) {
	return !g_menu.main.aimbot.multipoints.GetActiveIndices().empty();
}

bool callbacks::IsMultipointBodyOn() {
	return g_menu.main.aimbot.multipoints.get(2);
}

bool callbacks::IsMultipointLimbsOn() {
	return g_menu.main.aimbot.multipoints.get(4);
}

bool callbacks::IsAntiAimModeStand( ) {
	return g_menu.main.antiaim.mode.get( ) == 0;
}

bool callbacks::IsLbyOn() {
	return g_menu.main.antiaim.break_lby_stand.get();
}

bool callbacks::dt_null() {
	return g_menu.main.aimbot.double_tap.get() != -1;
}

bool callbacks::HasStandYaw( ) {
	return g_menu.main.antiaim.yaw_stand.get( ) > 0;
}


bool callbacks::IsStandYawJitter( ) {
	return g_menu.main.antiaim.yaw_stand.get( ) == 2;
}

bool callbacks::IsStandYawRotate( ) {
	return g_menu.main.antiaim.yaw_stand.get( ) == 3;
}

bool callbacks::IsStandYawRnadom( ) {
	return g_menu.main.antiaim.yaw_stand.get( ) == 4;
}

bool callbacks::IsStandDirAuto( ) {
	return g_menu.main.antiaim.dir_stand.get( ) == 0;
}

bool callbacks::IsStandDirCustom( ) {
	return g_menu.main.antiaim.dir_stand.get( ) == 4;
}



bool callbacks::IsAntiAimModeWalk( ) {
	return g_menu.main.antiaim.mode.get( ) == 1;
}

bool callbacks::WalkHasYaw( ) {
	return g_menu.main.antiaim.yaw_walk.get( ) > 0;
}

bool callbacks::IsWalkYawJitter( ) {
	return g_menu.main.antiaim.yaw_walk.get( ) == 2;
}

bool callbacks::IsWalkYawRotate( ) {
	return g_menu.main.antiaim.yaw_walk.get( ) == 3;
}



bool callbacks::IsWalkYawRandom( ) {
	return g_menu.main.antiaim.yaw_walk.get( ) == 4;
}

bool callbacks::IsWalkDirAuto( ) {
	return g_menu.main.antiaim.dir_walk.get( ) == 0;
}

bool callbacks::IsWalkDirCustom( ) {
	return g_menu.main.antiaim.dir_walk.get( ) == 4;
}

bool callbacks::IsAntiAimModeAir( ) {
	return g_menu.main.antiaim.mode.get( ) == 2;
}

bool callbacks::AirHasYaw( ) {
	return g_menu.main.antiaim.yaw_air.get( ) > 0;
}

bool callbacks::IsAirYawJitter( ) {
	return g_menu.main.antiaim.yaw_air.get( ) == 2;
}

bool callbacks::IsAirYawRotate( ) {
	return g_menu.main.antiaim.yaw_air.get( ) == 3;
}

bool callbacks::IsAirYawRandom( ) {
	return g_menu.main.antiaim.yaw_air.get( ) == 4;
}

bool callbacks::IsAirDirAuto( ) {
	return g_menu.main.antiaim.dir_air.get( ) == 0;
}

bool callbacks::IsAirDirCustom( ) {
	return g_menu.main.antiaim.dir_air.get( ) == 4;
}

bool callbacks::IsFakeAntiAimRelative() {
	return g_menu.main.antiaim.fake_yaw.get() == 2;
}

bool callbacks::IsFakeAntiAimJitter() {
	return g_menu.main.antiaim.fake_yaw.get() == 3;
}

bool callbacks::IsConfig1( ) {
	return g_menu.main.misc.config.get( ) == 0;
}

bool callbacks::IsConfig2( ) {
	return g_menu.main.misc.config.get( ) == 1;
}

bool callbacks::IsConfig3( ) {
	return g_menu.main.misc.config.get( ) == 2;
}

bool callbacks::IsConfig4( ) {
	return g_menu.main.misc.config.get( ) == 3;
}

bool callbacks::IsConfig5( ) {
	return g_menu.main.misc.config.get( ) == 4;
}

bool callbacks::IsConfig6( ) {
	return g_menu.main.misc.config.get( ) == 5;
}

bool callbacks::IsConfig7() {
	return g_menu.main.misc.config.get() == 6;
}

bool callbacks::IsConfig8() {
	return g_menu.main.misc.config.get() == 7;
}

bool callbacks::IsConfig9() {
	return g_menu.main.misc.config.get() == 8;
}

bool callbacks::IsConfig10() {
	return g_menu.main.misc.config.get() == 9;
}

// weaponcfgs callbacks.
bool callbacks::DEAGLE( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::DEAGLE;
}

bool callbacks::ELITE( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::ELITE;
}

bool callbacks::FIVESEVEN( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::FIVESEVEN;
}

bool callbacks::GLOCK( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::GLOCK;
}

bool callbacks::AK47( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::AK47;
}

bool callbacks::AUG( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::AUG;
}

bool callbacks::AWP( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::AWP;
}

bool callbacks::FAMAS( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::FAMAS;
}

bool callbacks::G3SG1( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::G3SG1;
}

bool callbacks::GALIL( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::GALIL;
}

bool callbacks::M249( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::M249;
}

bool callbacks::M4A4( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::M4A4;
}

bool callbacks::MAC10( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::MAC10;
}

bool callbacks::P90( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::P90;
}

bool callbacks::UMP45( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::UMP45;
}

bool callbacks::XM1014( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::XM1014;
}

bool callbacks::BIZON( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::BIZON;
}

bool callbacks::MAG7( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::MAG7;
}

bool callbacks::NEGEV( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::NEGEV;
}

bool callbacks::SAWEDOFF( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::SAWEDOFF;
}

bool callbacks::TEC9( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::TEC9;
}

bool callbacks::P2000( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::P2000;
}

bool callbacks::MP7( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::MP7;
}

bool callbacks::MP9( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::MP9;
}

bool callbacks::NOVA( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::NOVA;
}

bool callbacks::P250( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::P250;
}

bool callbacks::SCAR20( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::SCAR20;
}

bool callbacks::SG553( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::SG553;
}

bool callbacks::SSG08( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::SSG08;
}

bool callbacks::M4A1S( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::M4A1S;
}

bool callbacks::USPS( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::USPS;
}

bool callbacks::CZ75A( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::CZ75A;
}

bool callbacks::REVOLVER( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::REVOLVER;
}

bool callbacks::KNIFE_BAYONET( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_BAYONET;
}

bool callbacks::KNIFE_FLIP( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_FLIP;
}

bool callbacks::KNIFE_GUT( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_GUT;
}

bool callbacks::KNIFE_KARAMBIT( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_KARAMBIT;
}

bool callbacks::KNIFE_M9_BAYONET( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_M9_BAYONET;
}

bool callbacks::KNIFE_HUNTSMAN( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_HUNTSMAN;
}

bool callbacks::KNIFE_FALCHION( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_FALCHION;
}

bool callbacks::KNIFE_BOWIE( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_BOWIE;
}

bool callbacks::KNIFE_BUTTERFLY( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_BUTTERFLY;
}

bool callbacks::KNIFE_SHADOW_DAGGERS( ) {
	if( !g_csgo.m_engine->IsInGame( ) || !g_cl.m_processing )
		return false;

	return g_cl.m_weapon_id == Weapons_t::KNIFE_SHADOW_DAGGERS;
}

bool callbacks::IsOverrideDamage() {
	return g_menu.main.aimbot.damage_override.get() != -1;
}

void callbacks::ToggleDMG() {
	g_aimbot.m_damage_toggle = !g_aimbot.m_damage_toggle;
}

void callbacks::ToggleOverrideRes()
{
	g_aimbot.m_override = !g_aimbot.m_override;
}

bool callbacks::IsEnemyChams() {
	return g_menu.main.players.chams_selection.get() == 0;
}
bool callbacks::IsFriendlyChams() {
	return g_menu.main.players.chams_selection.get() == 1;
}

bool callbacks::IsLocalChams() {
	return g_menu.main.players.chams_selection.get() == 2;
}

bool callbacks::IsFakeChams() {
	return g_menu.main.players.chams_selection.get() == 3;
}

bool callbacks::enableenemychams() {
	return g_menu.main.players.chams_enemy_enable.get();
}

bool callbacks::enablefriendlychams() {
	return g_menu.main.players.chams_friendly_enable.get();
}

bool callbacks::enablelocalchams() {
	return g_menu.main.players.chams_local_enable.get();
}

bool callbacks::scopealphazoomXD() {
	return g_menu.main.players.chams_local_scope.get();
}

bool callbacks::baktrankcharms() {
	return g_menu.main.players.chams_enemy_history_enable.get();
}

bool callbacks::IsAnyHitSoundOn() {
	return g_menu.main.misc.hitmarker.get(4) || g_menu.main.misc.hitmarker.get(5);
}

bool callbacks::IsArrowsOn() {
	return g_menu.main.visuals.indicators.get(5);
}

void callbacks::print_ids() {
	if (!g_menu.main.aimbot.enabled.get() || !g_csgo.m_engine->IsInGame()) {
		g_notify.add("aimbot disabled or not ingame, not running.\n");
		return;
	}

	for (int i{ 1 }; i <= g_aimbot.m_players.size(); ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!g_aimbot.IsValidTarget(player))
			continue;

		AimPlayer* data = &g_aimbot.m_players[i - 1];
		if (!data)
			continue;

		player_info_t info;
		g_csgo.m_engine->GetPlayerInfo(player->index(), &info);
		g_notify.add(tfm::format("[%i] %s", i - 1, info.m_name));
	}
}

void callbacks::addtowhitelist() {
	if (!g_menu.main.aimbot.enabled.get() || !g_csgo.m_engine->IsInGame()) {
		g_notify.add("aimbot disabled or not ingame, not running.\n");
		return;
	}

	for (int i{ 1 }; i <= g_aimbot.m_players.size(); ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		if (!g_aimbot.IsValidTarget(player))
			continue;

		AimPlayer* data = &g_aimbot.m_players[i - 1];
		if (!data)
			continue;

		if (i - 1 == g_menu.main.plist.player_to_insert.get()) {
			data->m_ignore = true;
			player_info_t info;
			g_csgo.m_engine->GetPlayerInfo(player->index(), &info);
			g_notify.add(tfm::format("whitelisting player: %s", info.m_name));
			g_cl.print("whitelisting player: ");
			g_cl.printcustom(colors::light_blue, tfm::format("%s", info.m_name));
			break;
		}
	}

}

void callbacks::clearwhitelist() {
	if (!g_menu.main.aimbot.enabled.get() || !g_csgo.m_engine->IsInGame()) {
		g_notify.add("aimbot disabled or not ingame, not running.\n");
		return;
	}

	for (int i{ 1 }; i <= g_aimbot.m_players.size(); ++i) {
		Player* player = g_csgo.m_entlist->GetClientEntity< Player* >(i);

		AimPlayer* data = &g_aimbot.m_players[i - 1];
		if (!data)
			continue;

		data->m_ignore = false;
	}

	g_notify.add("cleared.");
}

bool callbacks::isfogon() {
	return g_menu.main.visuals.world.get(4);
}

bool callbacks::IsBaimHealth() {
	return g_menu.main.aimbot.baim2.get(1);
}