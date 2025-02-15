#pragma once

namespace callbacks {
	bool IsArrowsOn();
	bool IsAnyHitSoundOn();
	bool IsCustomHitsound();
	void HitsoundReload();
	void SkinUpdate( );
	void ForceFullUpdate( );
	bool is_acc_boost();
	void ToggleLeft();
	void ToggleRight();
	void ToggleBack();
	void ToggleForward();
	bool droppedwpnson();
	bool is_fluctuate();
	bool droppedglowewpaon();
	void toggle_override();
	void connect();
	void connect2();
	void connect3();
	bool quick_stop_on();
	bool ammoon();
	bool boxon();
	bool healthon();
	bool overridehealthon();
	bool nameon();
	bool weaponiconon();
	bool is_skeleton_on();
	bool is_history_skeleton_on();
	bool are_oof_arrow_on();
	bool is_sound_esp_on();
	bool lbytimeron();
	bool glowon();
	bool isbullettracer();
	bool islaseron();
	bool grenadeson();
	bool bombon();
	bool trajectoryon();
	bool hitmarker_on();
	bool peneton();
	void ToggleForceBodyAim();
	bool IsNightMode();
	bool isambient();
	bool iscustommolo();
	void ToggleThirdPerson( );
	bool ToggleFakeWalkOn();
	void toggle_fake_ping( );
	bool is_fake_ping_bound();
	void toggle_sec_fake_ping();
	bool is_sec_fake_ping_bound();
	bool IsProjectiles();
	bool is_auto_buy_on();
	bool isdynamiccolor();
    void clear_killfeed( );
	void SaveHotkeys( );
	void ConfigLoad1( );
	void ConfigLoad2( );
	bool IsDelayShot();
	bool IsPFBOn();
	bool is_shot_matrix_on();
	bool IsAstopOn();
	bool IsInAirAstop();
	bool override_pistol_on();
	bool is_instant_stop_air_on();
	void ConfigLoad3( );
	void ConfigLoad4( );
	void ConfigLoad5( );
	void ConfigLoad6( );
	void ConfigLoad7();
	void ConfigLoad8();
	void ConfigLoad9();
	void ConfigLoad10();
	void ConfigLoad( );
	void ConfigSave( );
	bool is_crooked();
	bool is_hit_chance_on( );
	bool IsMultipointOn( );
	bool IsMultipointBodyOn( );
	bool IsMultipointLimbsOn();
	bool IsAntiAimModeStand( );
	bool IsLbyOn();
	bool IsSwitchFakeBody();
	bool IsCustomBody();
	bool IsCustomTwist();
	bool HasStandYaw( );
	bool IsStandYawJitter( );
	bool IsStandYawRotate( );
	bool IsStandYawRnadom( );
	bool IsStandDirAuto( );
	bool IsStandDirCustom( );
	bool IsAntiAimModeUse();
	bool UseHasYaw();
	bool IsUseYawJitter();
	bool IsUseYawRotate();
	bool IsUseYawRandom();
	bool IsUseDirAuto();
	bool IsUseDirCustom();
	bool IsAntiAimModeWalk( );
	bool WalkHasYaw( );
	bool IsWalkYawJitter( );
	bool IsWalkYawRotate( );
	bool IsWalkYawRandom( );
	bool IsWalkDirAuto( );
	bool IsWalkDirCustom( );
	bool IsAntiAimModeAir( );
	bool AirHasYaw( );
	bool IsAirYawJitter( );
	bool IsAirYawRotate( );
	bool IsAirYawRandom( );
	bool IsAirDirAuto( );
	bool IsAirDirCustom( );
	bool IsFakeAntiAimRelative( );
	bool IsFakeAntiAimJitter( );
	bool IsConfig1( );
	bool IsConfig2( );
	bool IsConfig3( );
	bool IsConfig4( );
	bool IsConfig5( );
	bool IsConfig6( );
	bool IsConfig7();
	bool IsConfig8();
	bool IsConfig9();
	bool IsConfig10();

	// weapon cfgs.
	bool DEAGLE( );
	bool ELITE( );
	bool FIVESEVEN( );
	bool GLOCK( );
	bool AK47( );
	bool AUG( );
	bool AWP( );
	bool FAMAS( );
	bool G3SG1( );
	bool GALIL( );
	bool M249( );
	bool M4A4( );
	bool MAC10( );
	bool P90( );
	bool UMP45( );
	bool XM1014( );
	bool BIZON( );
	bool MAG7( );
	bool NEGEV( );
	bool SAWEDOFF( );
	bool TEC9( );
	bool P2000( );
	bool MP7( );
	bool MP9( );
	bool NOVA( );
	bool P250( );
	bool SCAR20( );
	bool SG553( );
	bool SSG08( );
	bool M4A1S( );
	bool USPS( );
	bool CZ75A( );
	bool REVOLVER( );
	bool KNIFE_BAYONET( );
	bool KNIFE_FLIP( );
	bool KNIFE_GUT( );
	bool KNIFE_KARAMBIT( );
	bool KNIFE_M9_BAYONET( );
	bool KNIFE_HUNTSMAN( );
	bool KNIFE_FALCHION( );
	bool KNIFE_BOWIE( );
	bool KNIFE_BUTTERFLY( );
	bool KNIFE_SHADOW_DAGGERS( );
	bool IsOverrideDamage();

	void ToggleDMG();
	void ToggleOverrideRes();
	bool ammpespon();
	bool is_chams_mode_local();
	bool is_glow_mode_local();
	bool IsChamsSelection1();
	bool EnemyGlowChamsSelection();
	bool EnemyMetallicChamsSelection();
	bool IsChamsSelection2();
	bool fluctuate_stand();
	bool shot_matrixes_on();
	bool prefer_body_on();
	bool autopeek_on();
	bool is_chams_selection_history();
	bool is_chams_selection_shot();
	bool IsChamsSelection6();
	bool is_tab_hitscan();
	bool is_tab_not_hitscan();
	bool is_tab_exploits();
	void is_dt_on();
	bool dt_null();
	bool not_fluctuate_stand();
	bool is_lby_chams_glow();
	bool HasWeapon();
	void UpdateSkinColor();
	void ForceWeaponColor();
	void ConfigLoadSkins();
	void ConfigSaveSkins();
	bool is_viewmodel_fov();
	bool EuSvList();
	bool NaSvList();
	void dicks2();
	void dickswm();
	void mrx();
	void mrx2();

	void ConfigReset();
	void ConfigImport();
	void ConfigExport();
	// to do lol ^^

	bool IsEnemyChams();
	bool IsFriendlyChams();
	bool IsLocalChams();
	bool IsFakeChams();

	bool enableenemychams();
	bool enablefriendlychams();
	bool enablelocalchams();
	bool scopealphazoomXD();
	bool baktrankcharms();

	void print_ids();
	void addtowhitelist();
	void clearwhitelist();
	
	bool isfogon();
	bool IsBaimHealth();		
}