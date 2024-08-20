#pragma once

class AimbotTab : public Tab {
public:
	// col1.
	Checkbox	  enabled;
	Checkbox	  silent_aim;
	MultiDropdown hitboxes;
	MultiDropdown multipoints;

	Slider		  head_scale;
	Slider        head_air_scale;
	Slider		  body_scale;
	Slider		  feet_scale;
	Checkbox      adaptive;
	Slider		  min_damage;
	Checkbox      auto_scope;

	// col2.
	Slider	      hit_chance;
	Checkbox	  override_pistol;
	Slider	      pistol_hc;
	Checkbox	  stuff;
	MultiDropdown prefer_body_disablers;
	Keybind       baim_key;
	Keybind		  override;
	Keybind       damage_override;
	Slider        damage_override_amt;
	Keybind		  double_tap;
	Slider		  dt_hc;
	Dropdown	  dt_quickstop;
	MultiDropdown delay_shot;
	Slider        delay_shot_trace;
	Dropdown	  quick_stop_mode;
	Dropdown	  quick_stop_type;
	Checkbox      wait_accurate;
	Checkbox	  whitelist;
	Checkbox	  accuracy_boost;
	Slider		  accuracy_boost_amount;
	Checkbox      autofire;
	Checkbox      shot_matrix;
	Dropdown      matrix_shot;
	Colorpicker   matrix_shot_clr_target;
	Colorpicker   matrix_shot_clr;
	Checkbox      matrix_behind_walls;

	MultiDropdown baim1;
	MultiDropdown baim2;
	Slider        baim_hp;

public:
	void init() {
		// title.
		SetTitle(XOR("aimbot"));

		enabled.setup(XOR("enabled"), XOR("enabled"));
		RegisterElement(&enabled);

		autofire.setup(XOR("automatic fire"), XOR("autofire"));
		RegisterElement(&autofire);

		silent_aim.setup(XOR("silent aim"), XOR("silent_aim"));
		RegisterElement(&silent_aim);

		auto_scope.setup(XOR("automatic scope"), XOR("auto_scope"));
		RegisterElement(&auto_scope);

		hitboxes.setup(XOR("target hitboxes"), XOR("hitboxes"), { XOR("head"), XOR("chest"), XOR("stomach"), XOR("arms"), XOR("legs"), XOR("feet") });
		RegisterElement(&hitboxes);

		multipoints.setup(XOR("multi-point"), XOR("multipoint"), { XOR("head"), XOR("chest"), XOR("body"), XOR("legs"), XOR("feet") });
		RegisterElement(&multipoints);

		head_scale.setup("head pointscale", XOR("hitbox_scale"), 30.f, 100.f, false, 0, 90.f, 1.f, XOR(L"%"));
		head_scale.AddShowCallback(callbacks::IsMultipointOn);
		RegisterElement(&head_scale);

		body_scale.setup(XOR("body pointscale"), XOR("body_hitbox_scale"), 30.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		body_scale.AddShowCallback(callbacks::IsMultipointBodyOn);
		RegisterElement(&body_scale);

		feet_scale.setup(XOR("limbs scale"), XOR("feet_scale"), 1.f, 100.f, true, 0, 45.f, 1.f, XOR(L"%"));
		feet_scale.AddShowCallback(callbacks::IsMultipointLimbsOn);
		RegisterElement(&feet_scale);

		hit_chance.setup("hit chance", XOR("hit_chance"), 0.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		RegisterElement(&hit_chance);

		pistol_hc.setup("pistol hit chance", XOR("pistol_hc"), 0.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		RegisterElement(&pistol_hc);

		min_damage.setup(XOR("minimum damage"), XOR("min_damage"), 1.f, 126.f, true, 0, 40.f, 1.f, XOR(L"dmg"));
		RegisterElement(&min_damage);

		damage_override_amt.setup(" ", XOR("damage_override_amt"), 1.f, 126.f, false, 0, 1.f, 1.f, XOR(L"dmg"));
		damage_override_amt.AddShowCallback(callbacks::IsOverrideDamage);
		RegisterElement(&damage_override_amt);

		quick_stop_mode.setup(XOR("auto stop"), XOR("quick_stop_mode"), { "off", "on", "between shots" }, true);
		RegisterElement(&quick_stop_mode, 1);

		quick_stop_type.setup(XOR(""), XOR("quick_stop_modifiers"), { "full stop", "minimal speed", "fakewalk" }, false);
		quick_stop_type.AddShowCallback(callbacks::IsAstopOn);
		RegisterElement(&quick_stop_type, 1);

		baim_key.setup(XOR("force body-aim on key"), XOR("body aim on key"));
		baim_key.SetToggleCallback(callbacks::ToggleForceBodyAim);
		RegisterElement(&baim_key, 1);

		override.setup(XOR("override correction"), XOR("override"));
		override.SetToggleCallback(callbacks::ToggleOverrideRes);
		RegisterElement(&override, 1);

		damage_override.setup(XOR("minimum damage override"), XOR("damage_override"));
		damage_override.SetToggleCallback(callbacks::ToggleDMG);
		RegisterElement(&damage_override, 1);

		matrix_shot.setup(XOR("hitbox matrix"), XOR("shot_matrix"), { XOR("off"), XOR("all"), XOR("target only"), XOR("target only + all") });
		RegisterElement(&matrix_shot, 1);

		matrix_shot_clr_target.setup(XOR("target hitbox color"), XOR("matrix_shot_clr_target"), Color(255, 0, 0, 150));
		matrix_shot_clr_target.AddShowCallback(callbacks::is_shot_matrix_on);
		RegisterElement(&matrix_shot_clr_target, 1);

		matrix_shot_clr.setup(XOR("hitbox color"), XOR("matrix_shot_clr"), Color(150, 150, 150, 20));
		matrix_shot_clr.AddShowCallback(callbacks::is_shot_matrix_on);
		RegisterElement(&matrix_shot_clr, 1);

		matrix_behind_walls.setup(XOR("draw matrix behind walls"), XOR("matrix_behind_walls"));
		matrix_behind_walls.AddShowCallback(callbacks::is_shot_matrix_on);
		RegisterElement(&matrix_behind_walls, 1);
	}
};

class AntiAimTab : public Tab {
public:
	// col 1.
	Checkbox enable;
	Checkbox edge;
	Dropdown mode;

	Dropdown pitch_stand;
	Dropdown yaw_stand;
	Slider   jitter_range_stand;
	Slider   rot_range_stand;
	Slider   rot_speed_stand;
	Slider   rand_update_stand;
	Dropdown dir_stand;
	Slider   dir_time_stand;
	Slider   dir_custom_stand;
	Checkbox dir_lock;
	Dropdown base_angle_stand;
	Checkbox break_lby_stand;
	Dropdown body_fake_stand;

	Dropdown pitch_walk;
	Dropdown yaw_walk;
	Slider   jitter_range_walk;
	Slider   rot_range_walk;
	Slider   rot_speed_walk;
	Slider   rand_update_walk;
	Dropdown dir_walk;
	Slider	 dir_time_walk;
	Slider   dir_custom_walk;
	Dropdown base_angle_walk;

	Dropdown pitch_air;
	Dropdown yaw_air;
	Slider   jitter_range_air;
	Slider   rot_range_air;
	Slider   rot_speed_air;
	Slider   rand_update_air;
	Dropdown dir_air;
	Slider   dir_time_air;
	Slider   dir_custom_air;
	Dropdown base_angle_air;
	Dropdown body_fake_air;

	// col 2.
	Dropdown fake_yaw;
	Slider	 fake_relative;
	Slider	 fake_jitter_range;

	Checkbox      lag_enable;
	MultiDropdown lag_active;
	Dropdown      lag_mode;
	Slider        lag_limit;
	Checkbox      lag_land;
	Keybind       lag_disable;

	Keybind       left;
	Keybind       right;
	Keybind       backwards;
	Keybind       forward;

public:
	void init() {
		SetTitle(XOR("anti-aim"));

		enable.setup(XOR("enable"), XOR("enable"));
		RegisterElement(&enable);

		// stand.
		pitch_stand.setup(XOR("pitch"), XOR("pitch_stnd"), { XOR("off"), XOR("down"), XOR("up") });
		RegisterElement(&pitch_stand);

		yaw_stand.setup(XOR("standing anti-aim yaw"), XOR("yaw_stnd"), { XOR("off"), XOR("direction"), XOR("jitter"), XOR("rotate"), XOR("random") });
		RegisterElement(&yaw_stand);

		jitter_range_stand.setup("", XOR("jitter_range_stnd"), 1.f, 180.f, false, 0, 45.f, 5.f, XOR(L" "));
		jitter_range_stand.AddShowCallback(callbacks::IsStandYawJitter);
		RegisterElement(&jitter_range_stand);

		rot_range_stand.setup("", XOR("rot_range_stnd"), 0.f, 360.f, false, 0, 360.f, 5.f, XOR(L" "));
		rot_range_stand.AddShowCallback(callbacks::IsStandYawRotate);
		RegisterElement(&rot_range_stand);

		rot_speed_stand.setup("", XOR("rot_speed_stnd"), 1.f, 100.f, false, 0, 10.f, 1.f, XOR(L"%"));
		rot_speed_stand.AddShowCallback(callbacks::IsStandYawRotate);
		RegisterElement(&rot_speed_stand);

		rand_update_stand.setup("", XOR("rand_update_stnd"), 0.f, 1.f, false, 1, 0.f, 0.1f);
		rand_update_stand.AddShowCallback(callbacks::IsStandYawRnadom);
		RegisterElement(&rand_update_stand);

		dir_stand.setup(XOR("direction"), XOR("dir_stnd"), { XOR("auto"), XOR("backwards"), XOR("to do..") });
		dir_stand.AddShowCallback(callbacks::HasStandYaw);
		RegisterElement(&dir_stand);

		break_lby_stand.setup(XOR("break lowerbody yaw"), XOR("break_lby_stand"));
		RegisterElement(&break_lby_stand);

		body_fake_stand.setup(XOR(""), XOR("body_fake_stnd"), { XOR("off"), XOR("break"), XOR("right"), XOR("opposite"), XOR("z") }, false);
		body_fake_stand.AddShowCallback(callbacks::IsLbyOn);
		body_fake_stand.AddShowCallback(callbacks::HasStandYaw);
		RegisterElement(&body_fake_stand);

		// walk.
		yaw_walk.setup(XOR("moving anti-aim yaw"), XOR("yaw_walk"), { XOR("off"), XOR("direction"), XOR("jitter"), XOR("rotate"), XOR("random") });
		RegisterElement(&yaw_walk);

		jitter_range_walk.setup("", XOR("jitter_range_walk"), 1.f, 180.f, false, 0, 45.f, 5.f, XOR(L" "));
		jitter_range_walk.AddShowCallback(callbacks::IsWalkYawJitter);
		RegisterElement(&jitter_range_walk);

		rot_range_walk.setup("", XOR("rot_range_walk"), 0.f, 360.f, false, 0, 360.f, 5.f, XOR(L" "));
		rot_range_walk.AddShowCallback(callbacks::IsWalkYawRotate);
		RegisterElement(&rot_range_walk);

		rot_speed_walk.setup("", XOR("rot_speed_walk"), 1.f, 100.f, false, 0, 10.f, 1.f, XOR(L"%"));
		rot_speed_walk.AddShowCallback(callbacks::IsWalkYawRotate);
		RegisterElement(&rot_speed_walk);

		rand_update_walk.setup("", XOR("rand_update_walk"), 0.f, 1.f, false, 1, 0.f, 0.1f);
		rand_update_walk.AddShowCallback(callbacks::IsWalkYawRandom);
		RegisterElement(&rand_update_walk);

		dir_walk.setup(XOR("direction"), XOR("dir_walk"), { XOR("auto"), XOR("backwards"), XOR("to do..") });
		dir_walk.AddShowCallback(callbacks::WalkHasYaw);
		RegisterElement(&dir_walk);

		dir_custom_walk.setup("", XOR("dir_custom_walk"), -180.f, 180.f, false, 0, 0.f, 5.f, XOR(L" "));
		dir_custom_walk.AddShowCallback(callbacks::WalkHasYaw);
		dir_custom_walk.AddShowCallback(callbacks::IsWalkDirCustom);
		RegisterElement(&dir_custom_walk);

		// air.
		yaw_air.setup(XOR("in air anti-aim yaw"), XOR("yaw_air"), { XOR("off"), XOR("direction"), XOR("jitter"), XOR("rotate"), XOR("random") });
		RegisterElement(&yaw_air);

		jitter_range_air.setup("", XOR("jitter_range_air"), 1.f, 180.f, false, 0, 45.f, 5.f, XOR(L" "));
		jitter_range_air.AddShowCallback(callbacks::IsAirYawJitter);
		RegisterElement(&jitter_range_air);

		rot_range_air.setup("", XOR("rot_range_air"), 0.f, 360.f, false, 0, 360.f, 5.f, XOR(L" "));
		rot_range_air.AddShowCallback(callbacks::IsAirYawRotate);
		RegisterElement(&rot_range_air);

		rot_speed_air.setup("", XOR("rot_speed_air"), 1.f, 100.f, false, 0, 10.f, 1.f, XOR(L"%"));
		rot_speed_air.AddShowCallback(callbacks::IsAirYawRotate);
		RegisterElement(&rot_speed_air);

		rand_update_air.setup("", XOR("rand_update_air"), 0.f, 1.f, false, 1, 0.f, 0.1f);
		rand_update_air.AddShowCallback(callbacks::IsAirYawRandom);
		RegisterElement(&rand_update_air);

		dir_air.setup(XOR("direction"), XOR("dir_air"), { XOR("auto"), XOR("backwards"), XOR("to do..") });
		dir_air.AddShowCallback(callbacks::AirHasYaw);
		RegisterElement(&dir_air);

		dir_custom_air.setup("", XOR("dir_custom_air"), -180.f, 180.f, false, 0, 0.f, 5.f, XOR(L" "));
		dir_custom_air.AddShowCallback(callbacks::AirHasYaw);
		dir_custom_air.AddShowCallback(callbacks::IsAirDirCustom);
		RegisterElement(&dir_custom_air);

		// col2.
		fake_yaw.setup(XOR("fake yaw"), XOR("fake_yaw"), { XOR("off"), XOR("default"), XOR("random") });
		RegisterElement(&fake_yaw, 1);

		fake_relative.setup("", XOR("fake_relative"), -90.f, 90.f, false, 0, 0.f, 5.f, XOR(L" "));
		fake_relative.AddShowCallback(callbacks::IsFakeAntiAimRelative);
		RegisterElement(&fake_relative, 1);

		fake_jitter_range.setup("", XOR("fake_jitter_range"), 1.f, 90.f, false, 0, 0.f, 5.f, XOR(L" "));
		fake_jitter_range.AddShowCallback(callbacks::IsFakeAntiAimJitter);
		RegisterElement(&fake_jitter_range, 1);

		// col 2.
		lag_enable.setup(XOR("fake-lag"), XOR("lag_enable"));
		RegisterElement(&lag_enable, 1);

		lag_active.setup("", XOR("lag_active"), { XOR("move"), XOR("air"), XOR("crouch") }, false);
		RegisterElement(&lag_active, 1);

		lag_mode.setup("", XOR("lag_mode"), { XOR("max"), XOR("fluctuate"), XOR("random"), XOR("adaptive") }, false);
		RegisterElement(&lag_mode, 1);

		lag_limit.setup(XOR("limit"), XOR("lag_limit"), 2, 16, true, 0, 2, 1.f);
		RegisterElement(&lag_limit, 1);

		lag_disable.setup(XOR("disable lag on key"), XOR("lag_disable"));
		RegisterElement(&lag_disable, 1);

		left.setup(XOR("left"), XOR("left"));
		left.SetToggleCallback(callbacks::ToggleLeft);
		RegisterElement(&left, 1);

		right.setup(XOR("right"), XOR("right"));
		right.SetToggleCallback(callbacks::ToggleRight);
		RegisterElement(&right, 1);

		backwards.setup(XOR("back"), XOR("backwards"));
		backwards.SetToggleCallback(callbacks::ToggleBack);
		RegisterElement(&backwards, 1);

		forward.setup(XOR("forward"), XOR("forward"));
		forward.SetToggleCallback(callbacks::ToggleForward);
		RegisterElement(&forward, 1);
	}
};


class PlayersTab : public Tab {
public:
	Checkbox teammates;
	Checkbox box;
	Colorpicker   box_enemy;
	Slider name_esp_alpha;
	Slider box_color_alpha;
	Checkbox      dormant;
	Checkbox      offscreen;
	Colorpicker   offscreen_color;
	Colorpicker   proj_offscreen_color;
	Checkbox      footstep;
	Colorpicker   footstepcol;
	Checkbox name;
	Colorpicker   name_color;
	Slider name_color_alpha;
	Checkbox health;
	Checkbox override_health;
	Colorpicker   health_color;
	Slider ammo_alpha;
	MultiDropdown flags_enemy;
	Checkbox weaponicon;
	Checkbox weapontext;
	Colorpicker weaponcolor;
	Slider weaponcoloralpha;
	Checkbox      ammo;
	Checkbox distance;
	Colorpicker   ammo_color;
	Checkbox      lby_update;
	Colorpicker   lby_update_color;
	MultiDropdown flags_friendly;
	Slider box_esp_alpha;


	// col2.
	MultiDropdown skeleton;
	Colorpicker   skeleton_col;
	Colorpicker history_skeleton_col;
	MultiDropdown rainbow_visuals;
	Dropdown	  glow;
	Colorpicker   glow_enemy;
	Slider        glow_blend;
	Slider lby_timer_alpha;
	//Slider skeleton_alpha;
	Dropdown      chamstype;

	// chams general.
	Dropdown      chams_selection;

	// chams enemy.
	Checkbox      chams_enemy_enable;
	MultiDropdown chams_enemy;
	Dropdown      chams_enemy_mat;
	Colorpicker   chams_enemy_vis;
	Colorpicker   chams_enemy_invis;
	Checkbox      chams_enemy_wireframe;
	Colorpicker      chams_enemy_override_color;

	// chams history.
	Checkbox      chams_enemy_history_enable;
	Dropdown      chams_enemy_history_mat;
	Colorpicker   chams_enemy_history_col;
	Checkbox      chams_enemy_history_wireframe;
	Checkbox      chams_enemy_history_dist_alpha;

	// chams friendly.
	Checkbox      chams_friendly_enable;
	MultiDropdown chams_friendly;
	Dropdown      chams_friendly_mat;
	Colorpicker   chams_friendly_vis;
	Colorpicker   chams_friendly_invis;
	Checkbox      chams_friendly_wireframe;

	// chams local.
	Checkbox      chams_local_enable;
	Dropdown      chams_local_mat;
	Colorpicker   chams_local_col;
	Checkbox      chams_local_wireframe;
	Checkbox      chams_local_scope;
	Checkbox      chams_remove_scope;
	Slider		  scope_blend;

	Checkbox      enemy_radar;



public:
	void init() {
		SetTitle(XOR("players"));

		teammates.setup(XOR("teammates"), XOR("teammates"));
		RegisterElement(&teammates);

		dormant.setup(XOR("dormant"), XOR("dormant"));
		RegisterElement(&dormant);

		enemy_radar.setup(XOR("radar"), XOR("enemy_radar"));
		RegisterElement(&enemy_radar);

		box.setup(XOR("bounding box"), XOR("box"));
		RegisterElement(&box);

		box_enemy.setup(XOR("bounding box color"), XOR("box_enemy"), { 255, 255, 255, 180 });
		box_enemy.AddShowCallback(callbacks::boxon);
		RegisterElement(&box_enemy);


		health.setup(XOR("health bar"), XOR("health"));
		RegisterElement(&health);

		override_health.setup(XOR("override health bar color"), XOR("override_health"));
		override_health.AddShowCallback(callbacks::healthon);
		RegisterElement(&override_health);

		health_color.setup(XOR("health color"), XOR("health_color"), { 255, 255, 255, 180 });
		health_color.AddShowCallback(callbacks::overridehealthon);
		RegisterElement(&health_color);

		name.setup(XOR("name esp"), XOR("name"));
		RegisterElement(&name);

		name_color.setup(XOR("name esp color"), XOR("name_color"), { 255, 255, 255 });
		name_color.AddShowCallback(callbacks::nameon);
		RegisterElement(&name_color);

		weapontext.setup(XOR("weapon text"), XOR("weapontext"));
		RegisterElement(&weapontext);

		weaponicon.setup(XOR("weapon icon"), XOR("weaponicon"));
		RegisterElement(&weaponicon);

		weaponcolor.setup(XOR("weapon icon color"), XOR("weaponcolor"), { 255, 255, 255 });
		weaponcolor.AddShowCallback(callbacks::weaponiconon);
		RegisterElement(&weaponcolor);

		ammo.setup(XOR("ammo"), XOR("ammo"));
		RegisterElement(&ammo);

		ammo_color.setup(XOR("ammo color"), XOR("ammo_color"), { 80, 140, 200, 235 });
		ammo_color.AddShowCallback(callbacks::ammpespon);
		RegisterElement(&ammo_color);

		lby_update.setup(XOR("lby timer"), XOR("lby_update"));
		RegisterElement(&lby_update);

		lby_update_color.setup(XOR("lby timer color"), XOR("lby_update_color"), { 255, 0, 255 });
		lby_update_color.AddShowCallback(callbacks::lbytimeron);
		RegisterElement(&lby_update_color);

		offscreen.setup(XOR("offscreen esp"), XOR("offscreen"));
		RegisterElement(&offscreen);

		offscreen_color.setup(XOR("offscreen esp color"), XOR("offscreen_color"), colors::white);
		offscreen_color.AddShowCallback(callbacks::are_oof_arrow_on);
		RegisterElement(&offscreen_color);

		proj_offscreen_color.setup(XOR("projectile override color"), XOR("proj_offscreen_color"), colors::red);
		proj_offscreen_color.AddShowCallback(callbacks::are_oof_arrow_on);
		RegisterElement(&proj_offscreen_color);

		// col 2

		flags_enemy.setup(XOR("flags enemy"), XOR("flags_enemy"), { XOR("money"), XOR("reload"), XOR("armor"), XOR("flashed"), XOR("zoom"), XOR("bomb"), XOR("fake"), XOR("lethal"), XOR("tickbase"), XOR("(!) resolver mode"), XOR("(!) resolve chance"), XOR("latency"), XOR("distortion"), XOR("hit"), XOR("lagcomp") });
		RegisterElement(&flags_enemy, 1);

		glow.setup(XOR("glow"), XOR("glow"), { XOR("off"), XOR("filled blur"), XOR("blur outline"), XOR("model"), XOR("edge"), XOR("edge pulse") });
		RegisterElement(&glow, 1);

		glow_enemy.setup(XOR("glow color"), XOR("glow_enemy"), { 180, 57, 119 });
		glow_enemy.AddShowCallback(callbacks::glowon);
		RegisterElement(&glow_enemy, 1);

		skeleton.setup(XOR("skeleton"), XOR("skeleton"), { XOR("normal"), XOR("history") });
		RegisterElement(&skeleton, 1);

		skeleton_col.setup(XOR("skeleton color"), XOR("skeleton_col"), { 255, 255, 255, 160 });
		skeleton_col.AddShowCallback(callbacks::is_skeleton_on);
		RegisterElement(&skeleton_col, 1);

		history_skeleton_col.setup(XOR("history skeleton color"), XOR("history_skeleton_col"), { 255, 255, 255, 160 });
		history_skeleton_col.AddShowCallback(callbacks::is_history_skeleton_on);
		RegisterElement(&history_skeleton_col, 1);

		chams_selection.setup(XOR("cham options"), XOR("chams_selection"), { XOR("enemy"), XOR("friendly"), XOR("local") });
		RegisterElement(&chams_selection, 1);

		chams_enemy_enable.setup(XOR("enable"), XOR("chams_enemy_enable"));
		chams_enemy_enable.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_enable, 1);

		chams_enemy.setup(XOR("enemy chams"), XOR("chams_enemy"), { XOR("visible"), XOR("invisible") });
		chams_enemy.AddShowCallback(callbacks::enableenemychams);
		chams_enemy.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy, 1);

		chams_enemy_mat.setup(XOR("material"), XOR("chams_enemy_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost") });
		chams_enemy_mat.AddShowCallback(callbacks::enableenemychams);
		chams_enemy_mat.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_mat, 1);

		chams_enemy_vis.setup(XOR("color visible"), XOR("chams_enemy_vis"), { 150, 200, 60 });
		chams_enemy_vis.AddShowCallback(callbacks::enableenemychams);
		chams_enemy_vis.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_vis, 1);

		chams_enemy_invis.setup(XOR("color invisible"), XOR("chams_enemy_invis"), { 60, 180, 225 });
		chams_enemy_invis.AddShowCallback(callbacks::enableenemychams);
		chams_enemy_invis.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_invis, 1);

		chams_enemy_override_color.setup(XOR("color override"), XOR("chams_enemy_override_color"), { 60, 180, 225 });
		chams_enemy_override_color.AddShowCallback(callbacks::enableenemychams);
		chams_enemy_override_color.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_override_color, 1);

		// history chams.
		chams_enemy_history_enable.setup(XOR("backtrack chams"), XOR("chams_enemy_history_enable"));
		chams_enemy_history_enable.AddShowCallback(callbacks::enableenemychams);
		chams_enemy_history_enable.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_history_enable, 1);

		chams_enemy_history_mat.setup(XOR("material"), XOR("chams_enemy_history_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost") });
		chams_enemy_history_mat.AddShowCallback(callbacks::baktrankcharms);
		chams_enemy_history_mat.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_history_mat, 1);

		chams_enemy_history_col.setup(XOR("color"), XOR("chams_history_col"), { 255, 255, 200 });
		chams_enemy_history_col.AddShowCallback(callbacks::baktrankcharms);
		chams_enemy_history_col.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_history_col, 1);

		chams_enemy_history_dist_alpha.setup(XOR("fade based on distance"), XOR("chams_enemy_history_dist_alpha"));
		chams_enemy_history_dist_alpha.AddShowCallback(callbacks::baktrankcharms);
		chams_enemy_history_dist_alpha.AddShowCallback(callbacks::IsEnemyChams);
		RegisterElement(&chams_enemy_history_dist_alpha, 1);

		// friendly chams.
		chams_friendly_enable.setup(XOR("enable"), XOR("chams_friendly_enable"));
		chams_friendly_enable.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_enable, 1);

		chams_friendly.setup(XOR("teammate chams"), XOR("chams_friendly"), { XOR("visible"), XOR("invisible") });
		chams_friendly.AddShowCallback(callbacks::enablefriendlychams);
		chams_friendly.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly, 1);

		chams_friendly_mat.setup(XOR("material"), XOR("chams_friendly_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost") });
		chams_friendly_mat.AddShowCallback(callbacks::enablefriendlychams);
		chams_friendly_mat.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_mat, 1);

		chams_friendly_vis.setup(XOR("color visible"), XOR("chams_friendly_vis"), { 255, 200, 0 });
		chams_friendly_vis.AddShowCallback(callbacks::enablefriendlychams);
		chams_friendly_vis.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_vis, 1);

		chams_friendly_invis.setup(XOR("color invisible"), XOR("chams_friendly_invis"), { 255, 50, 0 });
		chams_friendly_invis.AddShowCallback(callbacks::enablefriendlychams);
		chams_friendly_invis.AddShowCallback(callbacks::IsFriendlyChams);
		RegisterElement(&chams_friendly_invis, 1);

		// local chams.
		chams_local_enable.setup(XOR("enable"), XOR("chams_local_enable"));
		chams_local_enable.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_enable, 1);

		chams_local_mat.setup(XOR("material"), XOR("chams_local_mat"), { XOR("default"), XOR("flat"), XOR("metallic"), XOR("outline"), XOR("ghost") });
		chams_local_mat.AddShowCallback(callbacks::enablelocalchams);
		chams_local_mat.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_mat, 1);

		chams_local_col.setup(XOR("color"), XOR("chams_local_col"), { 255, 255, 200 });
		chams_local_col.AddShowCallback(callbacks::enablelocalchams);
		chams_local_col.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_col, 1);

		chams_local_scope.setup(XOR("transparent scope"), XOR("chams_local_scope"));
		chams_local_scope.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_local_scope, 1);

		chams_remove_scope.setup(XOR("remove chams on scope"), XOR("chams_remove_scope"));
		chams_remove_scope.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&chams_remove_scope, 1);

		scope_blend.setup("", XOR("scope_blend"), 1.f, 100.f, false, 0, 100.f, 1.f, XOR(L"%"));
		scope_blend.AddShowCallback(callbacks::scopealphazoomXD);
		scope_blend.AddShowCallback(callbacks::IsLocalChams);
		RegisterElement(&scope_blend, 1);
	}
};

class VisualsTab : public Tab {
public:
	Checkbox      items;
	Checkbox      itemsglow;
	Checkbox items_distance;
	Checkbox      ammo;
	Colorpicker   item_color;
	Slider        item_color_alpha;
	Slider        glow_color_alpha;;
	Colorpicker   ammo_color;
	Slider        ammo_color_alpha;
	Checkbox      proj;
	Colorpicker   proj_color;
	Checkbox      grenade_path;
	Colorpicker grenade_path_col;
	Checkbox planted_c4;
	Colorpicker         bomb_col;
	Slider        bomb_col_slider;
	Slider        bomb_col_glow_slider;
	Slider        tracers_color_alpha;
	Colorpicker   tracers_color;
	Checkbox      disableteam;
	MultiDropdown	  world;
	Colorpicker        nightcolor;
	Checkbox      transparent_props;
	Slider		  transparent_props_amount;
	Colorpicker   propscolor;
	Colorpicker   ambient_color;
	Slider ambient_alpha;
	Slider        walls_amount;

	// col2.
	MultiDropdown removals;
	Checkbox      spectators;
	Checkbox      force_xhair;
	Checkbox postprocess;
	Checkbox      pen_crosshair;
	MultiDropdown indicators;
	Checkbox      tracers;
	Checkbox      impact_beams;
	MultiDropdown      impact_beams_entities;
	Dropdown      beam_type;
	Colorpicker   impact_beams_color;
	Colorpicker impact_beams_local_color;
	Slider        impact_beams_time;
	Keybind       thirdperson;
	Slider speed_beam;
	Slider alpha2;
	Checkbox bullet_impacts;

	Checkbox	manual_anti_aim_indic;
	Colorpicker manual_anti_aim_col;
	Dropdown arrow_type;
	Checkbox    modulate_molo;
	Colorpicker molocol;

	MultiDropdown spheres;
	Colorpicker spheres_col;
	Slider nightamt;
	Slider purpleamt;

	Colorpicker fog_color;
	Slider      fog_start;
	Slider      fog_end;

public:
	void init() {
		SetTitle(XOR("visuals"));

		items.setup(XOR("dropped weapons text"), XOR("items"));
		RegisterElement(&items);

		item_color.setup(XOR("dropped weapons color"), XOR("item_color"), colors::white);
		item_color.AddShowCallback(callbacks::droppedwpnson);
		RegisterElement(&item_color);

		proj.setup(XOR("projectiles"), XOR("proj"));
		RegisterElement(&proj);

		proj_color.setup(XOR("color"), XOR("proj_color"), colors::white);
		proj_color.AddShowCallback(callbacks::IsProjectiles);
		RegisterElement(&proj_color);

		spheres.setup(XOR("grenade spheres"), XOR("spheres"), { XOR("grenade"), XOR("molotov"), XOR("molotov flat") });
		spheres.AddShowCallback(callbacks::IsProjectiles);
		RegisterElement(&spheres);

		spheres_col.setup(XOR("spheres color"), XOR("spheres_col"), { 50, 149, 255 });
		spheres_col.AddShowCallback(callbacks::IsProjectiles);
		RegisterElement(&spheres_col);

		modulate_molo.setup(XOR("custom molotov color"), XOR("modulate_molo"));
		modulate_molo.AddShowCallback(callbacks::IsProjectiles);
		RegisterElement(&modulate_molo);

		molocol.setup("molotov color", XOR("molocol"), { colors::light_blue });
		molocol.AddShowCallback(callbacks::IsProjectiles);
		molocol.AddShowCallback(callbacks::iscustommolo);
		RegisterElement(&molocol);

		grenade_path.setup(XOR("grenade path"), XOR("grenade_path"));
		grenade_path.AddShowCallback(callbacks::grenadeson);
		RegisterElement(&grenade_path);

		grenade_path_col.setup(XOR("grenade path color"), XOR("grenade_path_col"), { 151, 130, 255 });
		grenade_path_col.AddShowCallback(callbacks::grenadeson);
		RegisterElement(&grenade_path_col);

		force_xhair.setup(XOR("crosshair"), XOR("force_xhair"));
		RegisterElement(&force_xhair);

		planted_c4.setup(XOR("bomb"), XOR("planted_c4"));
		RegisterElement(&planted_c4);

		bomb_col.setup(XOR("bomb color"), XOR("bomb_col"), { 151, 200, 60 });
		bomb_col.AddShowCallback(callbacks::bombon);
		RegisterElement(&bomb_col);

		tracers.setup(XOR("grenade trajectory"), XOR("tracers"));
		RegisterElement(&tracers);

		tracers_color.setup(XOR("grenade trajectory color"), XOR("tracers_color"), { 50, 149, 255 });
		tracers_color.AddShowCallback(callbacks::trajectoryon);
		RegisterElement(&tracers_color);

		spectators.setup(XOR("spectators"), XOR("spectators"));
		RegisterElement(&spectators);

		pen_crosshair.setup(XOR("penetration reticle"), XOR("pen_xhair"));
		RegisterElement(&pen_crosshair);

		impact_beams.setup(XOR("bullet tracers"), XOR("impact_beams"));
		RegisterElement(&impact_beams);

		impact_beams_entities.setup(XOR("entity selection"), XOR("impact_beams_entities"), { XOR("enemy"), XOR("local") });
		impact_beams_entities.AddShowCallback(callbacks::isbullettracer);
		RegisterElement(&impact_beams_entities);

		beam_type.setup(XOR("beam style"), XOR("beam_type"), { XOR("kaaba"), XOR("laser") });
		beam_type.AddShowCallback(callbacks::isbullettracer);
		RegisterElement(&beam_type);

		speed_beam.setup("speed", XOR("speed_beam"), 0.0f, 1.5, true, 1, 0.9f, 0.1f, XOR(L"s"));
		speed_beam.AddShowCallback(callbacks::islaseron);
		RegisterElement(&speed_beam);

		impact_beams_color.setup(XOR("bullet tracer enemy color"), XOR("impact_beams_color"), { 151, 130, 255 });
		impact_beams_color.AddShowCallback(callbacks::isbullettracer);
		RegisterElement(&impact_beams_color);

		impact_beams_local_color.setup(XOR("bullet tracer local color"), XOR("impact_beams_local_color"), { 151, 130, 255 });
		impact_beams_local_color.AddShowCallback(callbacks::isbullettracer);
		RegisterElement(&impact_beams_local_color);

		bullet_impacts.setup(XOR("bullet impacts"), XOR("bullet_impacts"));
		RegisterElement(&bullet_impacts);

		indicators.setup(XOR("indicators"), XOR("indicators"), { XOR("lower body yaw"), XOR("lagcomp"), XOR("ping spike"), XOR("damage override"), XOR("force body aim"), XOR("anti-aim arrows"), XOR("override correction"), XOR("velocity modifier")});
		RegisterElement(&indicators, 1);

		manual_anti_aim_col.setup(XOR("arrows color"), XOR("manual_anti_aim_col"), colors::light_blue);
		manual_anti_aim_col.AddShowCallback(callbacks::IsArrowsOn);
		RegisterElement(&manual_anti_aim_col, 1);

		removals.setup(XOR("removals"), XOR("removals"), { XOR("visual recoil"), XOR("smoke"), XOR("fog"), XOR("flashbang"), XOR("scope"), XOR("team rendering") });
		RegisterElement(&removals, 1);

		world.setup(XOR("world modifications"), XOR("world"), { XOR("night"), XOR("fullbright"), XOR("ambient"), XOR("night sky"), XOR("fog")});
		world.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&world, 1);

		nightamt.setup("nightmode darkness", XOR("nightamt"), 0.f, 100, true, 0, 100, 1.f, XOR(L"%"));
		nightamt.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&nightamt, 1);

		purpleamt.setup("purple amount", XOR("purpleamt"), 0.f, 100, true, 0, 100, 1.f, XOR(L"%"));
		purpleamt.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&purpleamt, 1);

		fog_color.setup(XOR("color fog"), XOR("fog_color"), { 235, 225, 225, 105 }, nullptr);
		fog_color.AddShowCallback(callbacks::isfogon);
		RegisterElement(&fog_color);

		fog_start.setup(XOR("start fog"), XOR("fog_start"), -5000.f, 15000.f, true, 0.f, 0.f, 100.f);
		fog_start.AddShowCallback(callbacks::isfogon);
		RegisterElement(&fog_start);

		fog_end.setup(XOR("end fog"), XOR("fog_end"), -5000.f, 15000.f, true, 0.f, 7500.f, 100.f);
		fog_end.AddShowCallback(callbacks::isfogon);
		RegisterElement(&fog_end);

		ambient_color.setup("ambient color", XOR("ambient_color"), { 25, 25, 25 });
		ambient_color.AddShowCallback(callbacks::isambient);
		RegisterElement(&ambient_color, 1);

		walls_amount.setup("transparent walls", XOR("walls_amount"), 0.f, 100, true, 0, 100, 1.f, XOR(L"%"));
		walls_amount.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&walls_amount, 1);

		transparent_props_amount.setup("transparent props", XOR("transparent_props_amount"), 0.f, 100.f, true, 0, 75.f, 1.f, XOR(L"%"));
		transparent_props_amount.SetCallback(Visuals::ModulateWorld);
		RegisterElement(&transparent_props_amount, 1);

		ambient_alpha.setup("", XOR("ambient_alpha"), 0.f, 255.f, false, 0, 255.f, 1.f, XOR(L"%"));
		ambient_alpha.AddShowCallback(callbacks::isambient);
		RegisterElement(&ambient_alpha, 1);

		thirdperson.setup(XOR("force thirdperson"), XOR("thirdperson"));
		thirdperson.SetToggleCallback(callbacks::ToggleThirdPerson);
		RegisterElement(&thirdperson, 1);
	}
};

class SkinsTab : public Tab {
public:
	Checkbox enable;

	Edit     id_deagle;
	Checkbox stattrak_deagle;
	Slider   quality_deagle;
	Slider	 seed_deagle;
	Colorpicker color_one_deagle;
	Colorpicker color_two_deagle;
	Colorpicker color_three_deagle;
	Colorpicker color_four_deagle;

	Edit     id_elite;
	Checkbox stattrak_elite;
	Slider   quality_elite;
	Slider	 seed_elite;
	Colorpicker color_one_elite;
	Colorpicker color_two_elite;
	Colorpicker color_three_elite;
	Colorpicker color_four_elite;

	Edit     id_fiveseven;
	Checkbox stattrak_fiveseven;
	Slider   quality_fiveseven;
	Slider	 seed_fiveseven;
	Colorpicker color_one_fiveseven;
	Colorpicker color_two_fiveseven;
	Colorpicker color_three_fiveseven;
	Colorpicker color_four_fiveseven;

	Edit     id_glock;
	Checkbox stattrak_glock;
	Slider   quality_glock;
	Slider	 seed_glock;
	Colorpicker color_one_glock;
	Colorpicker color_two_glock;
	Colorpicker color_three_glock;
	Colorpicker color_four_glock;

	Edit     id_ak47;
	Checkbox stattrak_ak47;
	Slider   quality_ak47;
	Slider	 seed_ak47;
	Colorpicker color_one_ak47;
	Colorpicker color_two_ak47;
	Colorpicker color_three_ak47;
	Colorpicker color_four_ak47;

	Edit     id_aug;
	Checkbox stattrak_aug;
	Slider   quality_aug;
	Slider	 seed_aug;
	Colorpicker color_one_aug;
	Colorpicker color_two_aug;
	Colorpicker color_three_aug;
	Colorpicker color_four_aug;

	Edit     id_awp;
	Checkbox stattrak_awp;
	Slider   quality_awp;
	Slider	 seed_awp;
	Colorpicker color_one_awp;
	Colorpicker color_two_awp;
	Colorpicker color_three_awp;
	Colorpicker color_four_awp;

	Edit     id_famas;
	Checkbox stattrak_famas;
	Slider   quality_famas;
	Slider	 seed_famas;
	Colorpicker color_one_famas;
	Colorpicker color_two_famas;
	Colorpicker color_three_famas;
	Colorpicker color_four_famas;

	Edit     id_g3sg1;
	Checkbox stattrak_g3sg1;
	Slider   quality_g3sg1;
	Slider	 seed_g3sg1;
	Colorpicker color_one_g3sg1;
	Colorpicker color_two_g3sg1;
	Colorpicker color_three_g3sg1;
	Colorpicker color_four_g3sg1;

	Edit     id_galil;
	Checkbox stattrak_galil;
	Slider   quality_galil;
	Slider	 seed_galil;
	Colorpicker color_one_galil;
	Colorpicker color_two_galil;
	Colorpicker color_three_galil;
	Colorpicker color_four_galil;

	Edit     id_m249;
	Checkbox stattrak_m249;
	Slider   quality_m249;
	Slider	 seed_m249;
	Colorpicker color_one_m249;
	Colorpicker color_two_m249;
	Colorpicker color_three_m249;
	Colorpicker color_four_m249;

	Edit     id_m4a4;
	Checkbox stattrak_m4a4;
	Slider   quality_m4a4;
	Slider	 seed_m4a4;
	Colorpicker color_one_m4a4;
	Colorpicker color_two_m4a4;
	Colorpicker color_three_m4a4;
	Colorpicker color_four_m4a4;

	Edit     id_mac10;
	Checkbox stattrak_mac10;
	Slider   quality_mac10;
	Slider	 seed_mac10;
	Colorpicker color_one_mac10;
	Colorpicker color_two_mac10;
	Colorpicker color_three_mac10;
	Colorpicker color_four_mac10;

	Edit     id_p90;
	Checkbox stattrak_p90;
	Slider   quality_p90;
	Slider	 seed_p90;
	Colorpicker color_one_p90;
	Colorpicker color_two_p90;
	Colorpicker color_three_p90;
	Colorpicker color_four_p90;

	Edit     id_ump45;
	Checkbox stattrak_ump45;
	Slider   quality_ump45;
	Slider	 seed_ump45;
	Colorpicker color_one_ump45;
	Colorpicker color_two_ump45;
	Colorpicker color_three_ump45;
	Colorpicker color_four_ump45;

	Edit     id_xm1014;
	Checkbox stattrak_xm1014;
	Slider   quality_xm1014;
	Slider	 seed_xm1014;
	Colorpicker color_one_xm1014;
	Colorpicker color_two_xm1014;
	Colorpicker color_three_xm1014;
	Colorpicker color_four_xm1014;

	Edit     id_bizon;
	Checkbox stattrak_bizon;
	Slider   quality_bizon;
	Slider	 seed_bizon;
	Colorpicker color_one_bizon;
	Colorpicker color_two_bizon;
	Colorpicker color_three_bizon;
	Colorpicker color_four_bizon;

	Edit     id_mag7;
	Checkbox stattrak_mag7;
	Slider   quality_mag7;
	Slider	 seed_mag7;
	Colorpicker color_one_mag7;
	Colorpicker color_two_mag7;
	Colorpicker color_three_mag7;
	Colorpicker color_four_mag7;

	Edit     id_negev;
	Checkbox stattrak_negev;
	Slider   quality_negev;
	Slider	 seed_negev;
	Colorpicker color_one_negev;
	Colorpicker color_two_negev;
	Colorpicker color_three_negev;
	Colorpicker color_four_negev;

	Edit     id_sawedoff;
	Checkbox stattrak_sawedoff;
	Slider   quality_sawedoff;
	Slider	 seed_sawedoff;
	Colorpicker color_one_sawedoff;
	Colorpicker color_two_sawedoff;
	Colorpicker color_three_sawedoff;
	Colorpicker color_four_sawedoff;

	Edit     id_tec9;
	Checkbox stattrak_tec9;
	Slider   quality_tec9;
	Slider	 seed_tec9;
	Colorpicker color_one_tec9;
	Colorpicker color_two_tec9;
	Colorpicker color_three_tec9;
	Colorpicker color_four_tec9;

	Edit     id_p2000;
	Checkbox stattrak_p2000;
	Slider   quality_p2000;
	Slider	 seed_p2000;
	Colorpicker color_one_p2000;
	Colorpicker color_two_p2000;
	Colorpicker color_three_p2000;
	Colorpicker color_four_p2000;

	Edit     id_mp7;
	Checkbox stattrak_mp7;
	Slider   quality_mp7;
	Slider	 seed_mp7;
	Colorpicker color_one_mp7;
	Colorpicker color_two_mp7;
	Colorpicker color_three_mp7;
	Colorpicker color_four_mp7;

	Edit     id_mp9;
	Checkbox stattrak_mp9;
	Slider   quality_mp9;
	Slider	 seed_mp9;
	Colorpicker color_one_mp9;
	Colorpicker color_two_mp9;
	Colorpicker color_three_mp9;
	Colorpicker color_four_mp9;

	Edit     id_nova;
	Checkbox stattrak_nova;
	Slider   quality_nova;
	Slider	 seed_nova;
	Colorpicker color_one_nova;
	Colorpicker color_two_nova;
	Colorpicker color_three_nova;
	Colorpicker color_four_nova;

	Edit     id_p250;
	Checkbox stattrak_p250;
	Slider   quality_p250;
	Slider	 seed_p250;
	Colorpicker color_one_p250;
	Colorpicker color_two_p250;
	Colorpicker color_three_p250;
	Colorpicker color_four_p250;

	Edit     id_scar20;
	Checkbox stattrak_scar20;
	Slider   quality_scar20;
	Slider	 seed_scar20;
	Colorpicker color_one_scar20;
	Colorpicker color_two_scar20;
	Colorpicker color_three_scar20;
	Colorpicker color_four_scar20;

	Edit     id_sg553;
	Checkbox stattrak_sg553;
	Slider   quality_sg553;
	Slider	 seed_sg553;
	Colorpicker color_one_sg553;
	Colorpicker color_two_sg553;
	Colorpicker color_three_sg553;
	Colorpicker color_four_sg553;

	Edit     id_ssg08;
	Checkbox stattrak_ssg08;
	Slider   quality_ssg08;
	Slider	 seed_ssg08;
	Colorpicker color_one_ssg08;
	Colorpicker color_two_ssg08;
	Colorpicker color_three_ssg08;
	Colorpicker color_four_ssg08;

	Edit     id_m4a1s;
	Checkbox stattrak_m4a1s;
	Slider   quality_m4a1s;
	Slider	 seed_m4a1s;
	Colorpicker color_one_m4a1s;
	Colorpicker color_two_m4a1s;
	Colorpicker color_three_m4a1s;
	Colorpicker color_four_m4a1s;

	Edit     id_usps;
	Checkbox stattrak_usps;
	Slider   quality_usps;
	Slider	 seed_usps;
	Colorpicker color_one_usps;
	Colorpicker color_two_usps;
	Colorpicker color_three_usps;
	Colorpicker color_four_usps;

	Edit     id_cz75a;
	Checkbox stattrak_cz75a;
	Slider   quality_cz75a;
	Slider	 seed_cz75a;
	Colorpicker color_one_cz75a;
	Colorpicker color_two_cz75a;
	Colorpicker color_three_cz75a;
	Colorpicker color_four_cz75a;

	Edit     id_revolver;
	Checkbox stattrak_revolver;
	Slider   quality_revolver;
	Slider	 seed_revolver;
	Colorpicker color_one_revolver;
	Colorpicker color_two_revolver;
	Colorpicker color_three_revolver;
	Colorpicker color_four_revolver;

	Edit     id_bayonet;
	Checkbox stattrak_bayonet;
	Slider   quality_bayonet;
	Slider	 seed_bayonet;
	Colorpicker color_one_bayonet;
	Colorpicker color_two_bayonet;
	Colorpicker color_three_bayonet;
	Colorpicker color_four_bayonet;

	Edit     id_flip;
	Checkbox stattrak_flip;
	Slider   quality_flip;
	Slider	 seed_flip;
	Colorpicker color_one_flip;
	Colorpicker color_two_flip;
	Colorpicker color_three_flip;
	Colorpicker color_four_flip;

	Edit     id_gut;
	Checkbox stattrak_gut;
	Slider   quality_gut;
	Slider	 seed_gut;
	Colorpicker color_one_gut;
	Colorpicker color_two_gut;
	Colorpicker color_three_gut;
	Colorpicker color_four_gut;

	Edit     id_karambit;
	Checkbox stattrak_karambit;
	Slider   quality_karambit;
	Slider	 seed_karambit;
	Colorpicker color_one_karambit;
	Colorpicker color_two_karambit;
	Colorpicker color_three_karambit;
	Colorpicker color_four_karambit;

	Edit     id_m9;
	Checkbox stattrak_m9;
	Slider   quality_m9;
	Slider	 seed_m9;
	Colorpicker color_one_m9;
	Colorpicker color_two_m9;
	Colorpicker color_three_m9;
	Colorpicker color_four_m9;

	Edit     id_huntsman;
	Checkbox stattrak_huntsman;
	Slider   quality_huntsman;
	Slider	 seed_huntsman;
	Colorpicker color_one_huntsman;
	Colorpicker color_two_huntsman;
	Colorpicker color_three_huntsman;
	Colorpicker color_four_huntsman;

	Edit     id_falchion;
	Checkbox stattrak_falchion;
	Slider   quality_falchion;
	Slider	 seed_falchion;
	Colorpicker color_one_falchion;
	Colorpicker color_two_falchion;
	Colorpicker color_three_falchion;
	Colorpicker color_four_falchion;

	Edit     id_bowie;
	Checkbox stattrak_bowie;
	Slider   quality_bowie;
	Slider	 seed_bowie;
	Colorpicker color_one_bowie;
	Colorpicker color_two_bowie;
	Colorpicker color_three_bowie;
	Colorpicker color_four_bowie;

	Edit     id_butterfly;
	Checkbox stattrak_butterfly;
	Slider   quality_butterfly;
	Slider	 seed_butterfly;
	Colorpicker color_one_butterfly;
	Colorpicker color_two_butterfly;
	Colorpicker color_three_butterfly;
	Colorpicker color_four_butterfly;

	Edit     id_daggers;
	Checkbox stattrak_daggers;
	Slider   quality_daggers;
	Slider	 seed_daggers;
	Colorpicker color_one_daggers;
	Colorpicker color_two_daggers;
	Colorpicker color_three_daggers;
	Colorpicker color_four_daggers;

	// col 2.
	Dropdown knife;
	Dropdown glove;
	Edit	 glove_id;
	Button color_update;

	Checkbox modulation;
	Button   save;
	Button   load;

public:
	void init() {
		SetTitle(XOR("skins"));

		enable.setup(XOR("enable"), XOR("skins_enable"));
		enable.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&enable);

		// weapons...
		id_deagle.setup(XOR("paintkit id"), XOR("id_deagle"), 3);
		id_deagle.SetCallback(callbacks::SkinUpdate);
		id_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&id_deagle);

		stattrak_deagle.setup(XOR("stattrak"), XOR("stattrak_deagle"));
		stattrak_deagle.SetCallback(callbacks::SkinUpdate);
		stattrak_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&stattrak_deagle);

		quality_deagle.setup(XOR("quality"), XOR("quality_deagle"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_deagle.SetCallback(callbacks::SkinUpdate);
		quality_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&quality_deagle);

		seed_deagle.setup(XOR("seed"), XOR("seed_deagle"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_deagle.SetCallback(callbacks::SkinUpdate);
		seed_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&seed_deagle);

		color_one_deagle.setup(XOR("color one"), XOR("color_one_deagle"), colors::white);
		color_one_deagle.SetCallback(callbacks::UpdateSkinColor);
		color_one_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&color_one_deagle);

		color_two_deagle.setup(XOR("color two"), XOR("color_two_deagle"), colors::white);
		color_two_deagle.SetCallback(callbacks::UpdateSkinColor);
		color_two_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&color_two_deagle);

		color_three_deagle.setup(XOR("color three"), XOR("color_three_deagle"), colors::white);
		color_three_deagle.SetCallback(callbacks::UpdateSkinColor);
		color_three_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&color_three_deagle);

		color_four_deagle.setup(XOR("color four"), XOR("color_four_deagle"), colors::white);
		color_four_deagle.SetCallback(callbacks::UpdateSkinColor);
		color_four_deagle.AddShowCallback(callbacks::DEAGLE);
		RegisterElement(&color_four_deagle);

		id_elite.setup(XOR("paintkit id"), XOR("id_elite"), 3);
		id_elite.SetCallback(callbacks::SkinUpdate);
		id_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&id_elite);

		stattrak_elite.setup(XOR("stattrak"), XOR("stattrak_elite"));
		stattrak_elite.SetCallback(callbacks::SkinUpdate);
		stattrak_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&stattrak_elite);

		quality_elite.setup(XOR("quality"), XOR("quality_elite"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_elite.SetCallback(callbacks::SkinUpdate);
		quality_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&quality_elite);

		seed_elite.setup(XOR("seed"), XOR("seed_elite"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_elite.SetCallback(callbacks::SkinUpdate);
		seed_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&seed_elite);

		color_one_elite.setup(XOR("color one"), XOR("color_one_elite"), colors::white);
		color_one_elite.SetCallback(callbacks::UpdateSkinColor);
		color_one_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&color_one_elite);

		color_two_elite.setup(XOR("color two"), XOR("color_two_elite"), colors::white);
		color_two_elite.SetCallback(callbacks::UpdateSkinColor);
		color_two_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&color_two_elite);

		color_three_elite.setup(XOR("color three"), XOR("color_three_elite"), colors::white);
		color_three_elite.SetCallback(callbacks::UpdateSkinColor);
		color_three_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&color_three_elite);

		color_four_elite.setup(XOR("color four"), XOR("color_four_elite"), colors::white);
		color_four_elite.SetCallback(callbacks::UpdateSkinColor);
		color_four_elite.AddShowCallback(callbacks::ELITE);
		RegisterElement(&color_four_elite);

		id_fiveseven.setup(XOR("paintkit id"), XOR("id_fiveseven"), 3);
		id_fiveseven.SetCallback(callbacks::SkinUpdate);
		id_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&id_fiveseven);

		stattrak_fiveseven.setup(XOR("stattrak"), XOR("stattrak_fiveseven"));
		stattrak_fiveseven.SetCallback(callbacks::SkinUpdate);
		stattrak_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&stattrak_fiveseven);

		quality_fiveseven.setup(XOR("quality"), XOR("quality_fiveseven"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_fiveseven.SetCallback(callbacks::SkinUpdate);
		quality_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&quality_fiveseven);

		seed_fiveseven.setup(XOR("seed"), XOR("seed_fiveseven"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_fiveseven.SetCallback(callbacks::SkinUpdate);
		seed_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&seed_fiveseven);

		color_one_fiveseven.setup(XOR("color one"), XOR("color_one_fiveseven"), colors::white);
		color_one_fiveseven.SetCallback(callbacks::UpdateSkinColor);
		color_one_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&color_one_fiveseven);

		color_two_fiveseven.setup(XOR("color two"), XOR("color_two_fiveseven"), colors::white);
		color_two_fiveseven.SetCallback(callbacks::UpdateSkinColor);
		color_two_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&color_two_fiveseven);

		color_three_fiveseven.setup(XOR("color three"), XOR("color_three_fiveseven"), colors::white);
		color_three_fiveseven.SetCallback(callbacks::UpdateSkinColor);
		color_three_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&color_three_fiveseven);

		color_four_fiveseven.setup(XOR("color four"), XOR("color_four_fiveseven"), colors::white);
		color_four_fiveseven.SetCallback(callbacks::UpdateSkinColor);
		color_four_fiveseven.AddShowCallback(callbacks::FIVESEVEN);
		RegisterElement(&color_four_fiveseven);

		id_glock.setup(XOR("paintkit id"), XOR("id_glock"), 3);
		id_glock.SetCallback(callbacks::SkinUpdate);
		id_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&id_glock);

		stattrak_glock.setup(XOR("stattrak"), XOR("stattrak_glock"));
		stattrak_glock.SetCallback(callbacks::SkinUpdate);
		stattrak_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&stattrak_glock);

		quality_glock.setup(XOR("quality"), XOR("quality_glock"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_glock.SetCallback(callbacks::SkinUpdate);
		quality_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&quality_glock);

		seed_glock.setup(XOR("seed"), XOR("seed_glock"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_glock.SetCallback(callbacks::SkinUpdate);
		seed_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&seed_glock);

		color_one_glock.setup(XOR("color one"), XOR("color_one_glock"), colors::white);
		color_one_glock.SetCallback(callbacks::UpdateSkinColor);
		color_one_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&color_one_glock);

		color_two_glock.setup(XOR("color two"), XOR("color_two_glock"), colors::white);
		color_two_glock.SetCallback(callbacks::UpdateSkinColor);
		color_two_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&color_two_glock);

		color_three_glock.setup(XOR("color three"), XOR("color_three_glock"), colors::white);
		color_three_glock.SetCallback(callbacks::UpdateSkinColor);
		color_three_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&color_three_glock);

		color_four_glock.setup(XOR("color four"), XOR("color_four_glock"), colors::white);
		color_four_glock.SetCallback(callbacks::UpdateSkinColor);
		color_four_glock.AddShowCallback(callbacks::GLOCK);
		RegisterElement(&color_four_glock);

		id_ak47.setup(XOR("paintkit id"), XOR("id_ak47"), 3);
		id_ak47.SetCallback(callbacks::SkinUpdate);
		id_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&id_ak47);

		stattrak_ak47.setup(XOR("stattrak"), XOR("stattrak_ak47"));
		stattrak_ak47.SetCallback(callbacks::SkinUpdate);
		stattrak_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&stattrak_ak47);

		quality_ak47.setup(XOR("quality"), XOR("quality_ak47"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_ak47.SetCallback(callbacks::SkinUpdate);
		quality_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&quality_ak47);

		seed_ak47.setup(XOR("seed"), XOR("seed_ak47"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_ak47.SetCallback(callbacks::SkinUpdate);
		seed_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&seed_ak47);

		color_one_ak47.setup(XOR("color one"), XOR("color_one_ak47"), colors::white);
		color_one_ak47.SetCallback(callbacks::UpdateSkinColor);
		color_one_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&color_one_ak47);

		color_two_ak47.setup(XOR("color two"), XOR("color_two_ak47"), colors::white);
		color_two_ak47.SetCallback(callbacks::UpdateSkinColor);
		color_two_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&color_two_ak47);

		color_three_ak47.setup(XOR("color three"), XOR("color_three_ak47"), colors::white);
		color_three_ak47.SetCallback(callbacks::UpdateSkinColor);
		color_three_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&color_three_ak47);

		color_four_ak47.setup(XOR("color four"), XOR("color_four_ak47"), colors::white);
		color_four_ak47.SetCallback(callbacks::UpdateSkinColor);
		color_four_ak47.AddShowCallback(callbacks::AK47);
		RegisterElement(&color_four_ak47);

		id_aug.setup(XOR("paintkit id"), XOR("id_aug"), 3);
		id_aug.SetCallback(callbacks::SkinUpdate);
		id_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&id_aug);

		stattrak_aug.setup(XOR("stattrak"), XOR("stattrak_aug"));
		stattrak_aug.SetCallback(callbacks::SkinUpdate);
		stattrak_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&stattrak_aug);

		quality_aug.setup(XOR("quality"), XOR("quality_aug"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_aug.SetCallback(callbacks::SkinUpdate);
		quality_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&quality_aug);

		seed_aug.setup(XOR("seed"), XOR("seed_aug"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_aug.SetCallback(callbacks::SkinUpdate);
		seed_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&seed_aug);

		color_one_aug.setup(XOR("color one"), XOR("color_one_aug"), colors::white);
		color_one_aug.SetCallback(callbacks::UpdateSkinColor);
		color_one_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&color_one_aug);

		color_two_aug.setup(XOR("color two"), XOR("color_two_aug"), colors::white);
		color_two_aug.SetCallback(callbacks::UpdateSkinColor);
		color_two_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&color_two_aug);

		color_three_aug.setup(XOR("color three"), XOR("color_three_aug"), colors::white);
		color_three_aug.SetCallback(callbacks::UpdateSkinColor);
		color_three_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&color_three_aug);

		color_four_aug.setup(XOR("color four"), XOR("color_four_aug"), colors::white);
		color_four_aug.SetCallback(callbacks::UpdateSkinColor);
		color_four_aug.AddShowCallback(callbacks::AUG);
		RegisterElement(&color_four_aug);

		id_awp.setup(XOR("paintkit id"), XOR("id_awp"), 3);
		id_awp.SetCallback(callbacks::SkinUpdate);
		id_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&id_awp);

		stattrak_awp.setup(XOR("stattrak"), XOR("stattrak_awp"));
		stattrak_awp.SetCallback(callbacks::SkinUpdate);
		stattrak_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&stattrak_awp);

		quality_awp.setup(XOR("quality"), XOR("quality_awp"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_awp.SetCallback(callbacks::SkinUpdate);
		quality_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&quality_awp);

		seed_awp.setup(XOR("seed"), XOR("seed_awp"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_awp.SetCallback(callbacks::SkinUpdate);
		seed_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&seed_awp);

		color_one_awp.setup(XOR("color one"), XOR("color_one_awp"), colors::white);
		color_one_awp.SetCallback(callbacks::UpdateSkinColor);
		color_one_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&color_one_awp);

		color_two_awp.setup(XOR("color two"), XOR("color_two_awp"), colors::white);
		color_two_awp.SetCallback(callbacks::UpdateSkinColor);
		color_two_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&color_two_awp);

		color_three_awp.setup(XOR("color three"), XOR("color_three_awp"), colors::white);
		color_three_awp.SetCallback(callbacks::UpdateSkinColor);
		color_three_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&color_three_awp);

		color_four_awp.setup(XOR("color four"), XOR("color_four_awp"), colors::white);
		color_four_awp.SetCallback(callbacks::UpdateSkinColor);
		color_four_awp.AddShowCallback(callbacks::AWP);
		RegisterElement(&color_four_awp);

		id_famas.setup(XOR("paintkit id"), XOR("id_famas"), 3);
		id_famas.SetCallback(callbacks::SkinUpdate);
		id_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&id_famas);

		stattrak_famas.setup(XOR("stattrak"), XOR("stattrak_famas"));
		stattrak_famas.SetCallback(callbacks::SkinUpdate);
		stattrak_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&stattrak_famas);

		quality_famas.setup(XOR("quality"), XOR("quality_famas"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_famas.SetCallback(callbacks::SkinUpdate);
		quality_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&quality_famas);

		seed_famas.setup(XOR("seed"), XOR("seed_famas"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_famas.SetCallback(callbacks::SkinUpdate);
		seed_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&seed_famas);

		color_one_famas.setup(XOR("color one"), XOR("color_one_famas"), colors::white);
		color_one_famas.SetCallback(callbacks::UpdateSkinColor);
		color_one_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&color_one_famas);

		color_two_famas.setup(XOR("color two"), XOR("color_two_famas"), colors::white);
		color_two_famas.SetCallback(callbacks::UpdateSkinColor);
		color_two_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&color_two_famas);

		color_three_famas.setup(XOR("color three"), XOR("color_three_famas"), colors::white);
		color_three_famas.SetCallback(callbacks::UpdateSkinColor);
		color_three_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&color_three_famas);

		color_four_famas.setup(XOR("color four"), XOR("color_four_famas"), colors::white);
		color_four_famas.SetCallback(callbacks::UpdateSkinColor);
		color_four_famas.AddShowCallback(callbacks::FAMAS);
		RegisterElement(&color_four_famas);

		id_g3sg1.setup(XOR("paintkit id"), XOR("id_g3sg1"), 3);
		id_g3sg1.SetCallback(callbacks::SkinUpdate);
		id_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&id_g3sg1);

		stattrak_g3sg1.setup(XOR("stattrak"), XOR("stattrak_g3sg1"));
		stattrak_g3sg1.SetCallback(callbacks::SkinUpdate);
		stattrak_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&stattrak_g3sg1);

		quality_g3sg1.setup(XOR("quality"), XOR("quality_g3sg1"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_g3sg1.SetCallback(callbacks::SkinUpdate);
		quality_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&quality_g3sg1);

		seed_g3sg1.setup(XOR("seed"), XOR("seed_g3sg1"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_g3sg1.SetCallback(callbacks::SkinUpdate);
		seed_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&seed_g3sg1);

		color_one_g3sg1.setup(XOR("color one"), XOR("color_one_g3sg1"), colors::white);
		color_one_g3sg1.SetCallback(callbacks::UpdateSkinColor);
		color_one_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&color_one_g3sg1);

		color_two_g3sg1.setup(XOR("color two"), XOR("color_two_g3sg1"), colors::white);
		color_two_g3sg1.SetCallback(callbacks::UpdateSkinColor);
		color_two_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&color_two_g3sg1);

		color_three_g3sg1.setup(XOR("color three"), XOR("color_three_g3sg1"), colors::white);
		color_three_g3sg1.SetCallback(callbacks::UpdateSkinColor);
		color_three_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&color_three_g3sg1);

		color_four_g3sg1.setup(XOR("color four"), XOR("color_four_g3sg1"), colors::white);
		color_four_g3sg1.SetCallback(callbacks::UpdateSkinColor);
		color_four_g3sg1.AddShowCallback(callbacks::G3SG1);
		RegisterElement(&color_four_g3sg1);

		id_galil.setup(XOR("paintkit id"), XOR("id_galil"), 3);
		id_galil.SetCallback(callbacks::SkinUpdate);
		id_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&id_galil);

		stattrak_galil.setup(XOR("stattrak"), XOR("stattrak_galil"));
		stattrak_galil.SetCallback(callbacks::SkinUpdate);
		stattrak_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&stattrak_galil);

		quality_galil.setup(XOR("quality"), XOR("quality_galil"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_galil.SetCallback(callbacks::SkinUpdate);
		quality_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&quality_galil);

		seed_galil.setup(XOR("seed"), XOR("seed_galil"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_galil.SetCallback(callbacks::SkinUpdate);
		seed_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&seed_galil);

		color_one_galil.setup(XOR("color one"), XOR("color_one_galil"), colors::white);
		color_one_galil.SetCallback(callbacks::UpdateSkinColor);
		color_one_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&color_one_galil);

		color_two_galil.setup(XOR("color two"), XOR("color_two_galil"), colors::white);
		color_two_galil.SetCallback(callbacks::UpdateSkinColor);
		color_two_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&color_two_galil);

		color_three_galil.setup(XOR("color three"), XOR("color_three_galil"), colors::white);
		color_three_galil.SetCallback(callbacks::UpdateSkinColor);
		color_three_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&color_three_galil);

		color_four_galil.setup(XOR("color four"), XOR("color_four_galil"), colors::white);
		color_four_galil.SetCallback(callbacks::UpdateSkinColor);
		color_four_galil.AddShowCallback(callbacks::GALIL);
		RegisterElement(&color_four_galil);

		id_m249.setup(XOR("paintkit id"), XOR("id_m249"), 3);
		id_m249.SetCallback(callbacks::SkinUpdate);
		id_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&id_m249);

		stattrak_m249.setup(XOR("stattrak"), XOR("stattrak_m249"));
		stattrak_m249.SetCallback(callbacks::SkinUpdate);
		stattrak_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&stattrak_m249);

		quality_m249.setup(XOR("quality"), XOR("quality_m249"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m249.SetCallback(callbacks::SkinUpdate);
		quality_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&quality_m249);

		seed_m249.setup(XOR("seed"), XOR("seed_m249"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m249.SetCallback(callbacks::SkinUpdate);
		seed_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&seed_m249);

		color_one_m249.setup(XOR("color one"), XOR("color_one_m249"), colors::white);
		color_one_m249.SetCallback(callbacks::UpdateSkinColor);
		color_one_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&color_one_m249);

		color_two_m249.setup(XOR("color two"), XOR("color_two_m249"), colors::white);
		color_two_m249.SetCallback(callbacks::UpdateSkinColor);
		color_two_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&color_two_m249);

		color_three_m249.setup(XOR("color three"), XOR("color_three_m249"), colors::white);
		color_three_m249.SetCallback(callbacks::UpdateSkinColor);
		color_three_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&color_three_m249);

		color_four_m249.setup(XOR("color four"), XOR("color_four_m249"), colors::white);
		color_four_m249.SetCallback(callbacks::UpdateSkinColor);
		color_four_m249.AddShowCallback(callbacks::M249);
		RegisterElement(&color_four_m249);

		id_m4a4.setup(XOR("paintkit id"), XOR("id_m4a4"), 3);
		id_m4a4.SetCallback(callbacks::SkinUpdate);
		id_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&id_m4a4);

		stattrak_m4a4.setup(XOR("stattrak"), XOR("stattrak_m4a4"));
		stattrak_m4a4.SetCallback(callbacks::SkinUpdate);
		stattrak_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&stattrak_m4a4);

		quality_m4a4.setup(XOR("quality"), XOR("quality_m4a4"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m4a4.SetCallback(callbacks::SkinUpdate);
		quality_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&quality_m4a4);

		seed_m4a4.setup(XOR("seed"), XOR("seed_m4a4"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m4a4.SetCallback(callbacks::SkinUpdate);
		seed_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&seed_m4a4);

		color_one_m4a4.setup(XOR("color one"), XOR("color_one_m4a4"), colors::white);
		color_one_m4a4.SetCallback(callbacks::UpdateSkinColor);
		color_one_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&color_one_m4a4);

		color_two_m4a4.setup(XOR("color two"), XOR("color_two_m4a4"), colors::white);
		color_two_m4a4.SetCallback(callbacks::UpdateSkinColor);
		color_two_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&color_two_m4a4);

		color_three_m4a4.setup(XOR("color three"), XOR("color_three_m4a4"), colors::white);
		color_three_m4a4.SetCallback(callbacks::UpdateSkinColor);
		color_three_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&color_three_m4a4);

		color_four_m4a4.setup(XOR("color four"), XOR("color_four_m4a4"), colors::white);
		color_four_m4a4.SetCallback(callbacks::UpdateSkinColor);
		color_four_m4a4.AddShowCallback(callbacks::M4A4);
		RegisterElement(&color_four_m4a4);

		id_mac10.setup(XOR("paintkit id"), XOR("id_mac10"), 3);
		id_mac10.SetCallback(callbacks::SkinUpdate);
		id_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&id_mac10);

		stattrak_mac10.setup(XOR("stattrak"), XOR("stattrak_mac10"));
		stattrak_mac10.SetCallback(callbacks::SkinUpdate);
		stattrak_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&stattrak_mac10);

		quality_mac10.setup(XOR("quality"), XOR("quality_mac10"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mac10.SetCallback(callbacks::SkinUpdate);
		quality_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&quality_mac10);

		seed_mac10.setup(XOR("seed"), XOR("seed_mac10"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mac10.SetCallback(callbacks::SkinUpdate);
		seed_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&seed_mac10);

		color_one_mac10.setup(XOR("color one"), XOR("color_one_mac10"), colors::white);
		color_one_mac10.SetCallback(callbacks::UpdateSkinColor);
		color_one_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&color_one_mac10);

		color_two_mac10.setup(XOR("color two"), XOR("color_two_mac10"), colors::white);
		color_two_mac10.SetCallback(callbacks::UpdateSkinColor);
		color_two_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&color_two_mac10);

		color_three_mac10.setup(XOR("color three"), XOR("color_three_mac10"), colors::white);
		color_three_mac10.SetCallback(callbacks::UpdateSkinColor);
		color_three_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&color_three_mac10);

		color_four_mac10.setup(XOR("color four"), XOR("color_four_mac10"), colors::white);
		color_four_mac10.SetCallback(callbacks::UpdateSkinColor);
		color_four_mac10.AddShowCallback(callbacks::MAC10);
		RegisterElement(&color_four_mac10);

		id_p90.setup(XOR("paintkit id"), XOR("id_p90"), 3);
		id_p90.SetCallback(callbacks::SkinUpdate);
		id_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&id_p90);

		stattrak_p90.setup(XOR("stattrak"), XOR("stattrak_p90"));
		stattrak_p90.SetCallback(callbacks::SkinUpdate);
		stattrak_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&stattrak_p90);

		quality_p90.setup(XOR("quality"), XOR("quality_p90"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_p90.SetCallback(callbacks::SkinUpdate);
		quality_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&quality_p90);

		seed_p90.setup(XOR("seed"), XOR("seed_p90"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_p90.SetCallback(callbacks::SkinUpdate);
		seed_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&seed_p90);

		color_one_p90.setup(XOR("color one"), XOR("color_one_p90"), colors::white);
		color_one_p90.SetCallback(callbacks::UpdateSkinColor);
		color_one_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&color_one_p90);

		color_two_p90.setup(XOR("color two"), XOR("color_two_p90"), colors::white);
		color_two_p90.SetCallback(callbacks::UpdateSkinColor);
		color_two_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&color_two_p90);

		color_three_p90.setup(XOR("color three"), XOR("color_three_p90"), colors::white);
		color_three_p90.SetCallback(callbacks::UpdateSkinColor);
		color_three_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&color_three_p90);

		color_four_p90.setup(XOR("color four"), XOR("color_four_p90"), colors::white);
		color_four_p90.SetCallback(callbacks::UpdateSkinColor);
		color_four_p90.AddShowCallback(callbacks::P90);
		RegisterElement(&color_four_p90);

		id_ump45.setup(XOR("paintkit id"), XOR("id_ump45"), 3);
		id_ump45.SetCallback(callbacks::SkinUpdate);
		id_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&id_ump45);

		stattrak_ump45.setup(XOR("stattrak"), XOR("stattrak_ump45"));
		stattrak_ump45.SetCallback(callbacks::SkinUpdate);
		stattrak_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&stattrak_ump45);

		quality_ump45.setup(XOR("quality"), XOR("quality_ump45"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_ump45.SetCallback(callbacks::SkinUpdate);
		quality_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&quality_ump45);

		seed_ump45.setup(XOR("seed"), XOR("seed_ump45"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_ump45.SetCallback(callbacks::SkinUpdate);
		seed_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&seed_ump45);

		color_one_ump45.setup(XOR("color one"), XOR("color_one_ump45"), colors::white);
		color_one_ump45.SetCallback(callbacks::UpdateSkinColor);
		color_one_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&color_one_ump45);

		color_two_ump45.setup(XOR("color two"), XOR("color_two_ump45"), colors::white);
		color_two_ump45.SetCallback(callbacks::UpdateSkinColor);
		color_two_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&color_two_ump45);

		color_three_ump45.setup(XOR("color three"), XOR("color_three_ump45"), colors::white);
		color_three_ump45.SetCallback(callbacks::UpdateSkinColor);
		color_three_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&color_three_ump45);

		color_four_ump45.setup(XOR("color four"), XOR("color_four_ump45"), colors::white);
		color_four_ump45.SetCallback(callbacks::UpdateSkinColor);
		color_four_ump45.AddShowCallback(callbacks::UMP45);
		RegisterElement(&color_four_ump45);

		id_xm1014.setup(XOR("paintkit id"), XOR("id_xm1014"), 3);
		id_xm1014.SetCallback(callbacks::SkinUpdate);
		id_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&id_xm1014);

		stattrak_xm1014.setup(XOR("stattrak"), XOR("stattrak_xm1014"));
		stattrak_xm1014.SetCallback(callbacks::SkinUpdate);
		stattrak_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&stattrak_xm1014);

		quality_xm1014.setup(XOR("quality"), XOR("quality_xm1014"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_xm1014.SetCallback(callbacks::SkinUpdate);
		quality_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&quality_xm1014);

		seed_xm1014.setup(XOR("seed"), XOR("seed_xm1014"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_xm1014.SetCallback(callbacks::SkinUpdate);
		seed_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&seed_xm1014);

		color_one_xm1014.setup(XOR("color one"), XOR("color_one_xm1014"), colors::white);
		color_one_xm1014.SetCallback(callbacks::UpdateSkinColor);
		color_one_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&color_one_xm1014);

		color_two_xm1014.setup(XOR("color two"), XOR("color_two_xm1014"), colors::white);
		color_two_xm1014.SetCallback(callbacks::UpdateSkinColor);
		color_two_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&color_two_xm1014);

		color_three_xm1014.setup(XOR("color three"), XOR("color_three_xm1014"), colors::white);
		color_three_xm1014.SetCallback(callbacks::UpdateSkinColor);
		color_three_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&color_three_xm1014);

		color_four_xm1014.setup(XOR("color four"), XOR("color_four_xm1014"), colors::white);
		color_four_xm1014.SetCallback(callbacks::UpdateSkinColor);
		color_four_xm1014.AddShowCallback(callbacks::XM1014);
		RegisterElement(&color_four_xm1014);

		id_bizon.setup(XOR("paintkit id"), XOR("id_bizon"), 3);
		id_bizon.SetCallback(callbacks::SkinUpdate);
		id_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&id_bizon);

		stattrak_bizon.setup(XOR("stattrak"), XOR("stattrak_bizon"));
		stattrak_bizon.SetCallback(callbacks::SkinUpdate);
		stattrak_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&stattrak_bizon);

		quality_bizon.setup(XOR("quality"), XOR("quality_bizon"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_bizon.SetCallback(callbacks::SkinUpdate);
		quality_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&quality_bizon);

		seed_bizon.setup(XOR("seed"), XOR("seed_bizon"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_bizon.SetCallback(callbacks::SkinUpdate);
		seed_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&seed_bizon);

		color_one_bizon.setup(XOR("color one"), XOR("color_one_bizon"), colors::white);
		color_one_bizon.SetCallback(callbacks::UpdateSkinColor);
		color_one_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&color_one_bizon);

		color_two_bizon.setup(XOR("color two"), XOR("color_two_bizon"), colors::white);
		color_two_bizon.SetCallback(callbacks::UpdateSkinColor);
		color_two_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&color_two_bizon);

		color_three_bizon.setup(XOR("color three"), XOR("color_three_bizon"), colors::white);
		color_three_bizon.SetCallback(callbacks::UpdateSkinColor);
		color_three_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&color_three_bizon);

		color_four_bizon.setup(XOR("color four"), XOR("color_four_bizon"), colors::white);
		color_four_bizon.SetCallback(callbacks::UpdateSkinColor);
		color_four_bizon.AddShowCallback(callbacks::BIZON);
		RegisterElement(&color_four_bizon);

		id_mag7.setup(XOR("paintkit id"), XOR("id_mag7"), 3);
		id_mag7.SetCallback(callbacks::SkinUpdate);
		id_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&id_mag7);

		stattrak_mag7.setup(XOR("stattrak"), XOR("stattrak_mag7"));
		stattrak_mag7.SetCallback(callbacks::SkinUpdate);
		stattrak_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&stattrak_mag7);

		quality_mag7.setup(XOR("quality"), XOR("quality_mag7"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mag7.SetCallback(callbacks::SkinUpdate);
		quality_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&quality_mag7);

		seed_mag7.setup(XOR("seed"), XOR("seed_mag7"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mag7.SetCallback(callbacks::SkinUpdate);
		seed_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&seed_mag7);

		color_one_mag7.setup(XOR("color one"), XOR("color_one_mag7"), colors::white);
		color_one_mag7.SetCallback(callbacks::UpdateSkinColor);
		color_one_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&color_one_mag7);

		color_two_mag7.setup(XOR("color two"), XOR("color_two_mag7"), colors::white);
		color_two_mag7.SetCallback(callbacks::UpdateSkinColor);
		color_two_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&color_two_mag7);

		color_three_mag7.setup(XOR("color three"), XOR("color_three_mag7"), colors::white);
		color_three_mag7.SetCallback(callbacks::UpdateSkinColor);
		color_three_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&color_three_mag7);

		color_four_mag7.setup(XOR("color four"), XOR("color_four_mag7"), colors::white);
		color_four_mag7.SetCallback(callbacks::UpdateSkinColor);
		color_four_mag7.AddShowCallback(callbacks::MAG7);
		RegisterElement(&color_four_mag7);

		id_negev.setup(XOR("paintkit id"), XOR("id_negev"), 3);
		id_negev.SetCallback(callbacks::SkinUpdate);
		id_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&id_negev);

		stattrak_negev.setup(XOR("stattrak"), XOR("stattrak_negev"));
		stattrak_negev.SetCallback(callbacks::SkinUpdate);
		stattrak_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&stattrak_negev);

		quality_negev.setup(XOR("quality"), XOR("quality_negev"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_negev.SetCallback(callbacks::SkinUpdate);
		quality_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&quality_negev);

		seed_negev.setup(XOR("seed"), XOR("seed_negev"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_negev.SetCallback(callbacks::SkinUpdate);
		seed_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&seed_negev);

		color_one_negev.setup(XOR("color one"), XOR("color_one_negev"), colors::white);
		color_one_negev.SetCallback(callbacks::UpdateSkinColor);
		color_one_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&color_one_negev);

		color_two_negev.setup(XOR("color two"), XOR("color_two_negev"), colors::white);
		color_two_negev.SetCallback(callbacks::UpdateSkinColor);
		color_two_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&color_two_negev);

		color_three_negev.setup(XOR("color three"), XOR("color_three_negev"), colors::white);
		color_three_negev.SetCallback(callbacks::UpdateSkinColor);
		color_three_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&color_three_negev);

		color_four_negev.setup(XOR("color four"), XOR("color_four_negev"), colors::white);
		color_four_negev.SetCallback(callbacks::UpdateSkinColor);
		color_four_negev.AddShowCallback(callbacks::NEGEV);
		RegisterElement(&color_four_negev);

		id_sawedoff.setup(XOR("paintkit id"), XOR("id_sawedoff"), 3);
		id_sawedoff.SetCallback(callbacks::SkinUpdate);
		id_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&id_sawedoff);

		stattrak_sawedoff.setup(XOR("stattrak"), XOR("stattrak_sawedoff"));
		stattrak_sawedoff.SetCallback(callbacks::SkinUpdate);
		stattrak_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&stattrak_sawedoff);

		quality_sawedoff.setup(XOR("quality"), XOR("quality_sawedoff"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_sawedoff.SetCallback(callbacks::SkinUpdate);
		quality_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&quality_sawedoff);

		seed_sawedoff.setup(XOR("seed"), XOR("seed_sawedoff"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_sawedoff.SetCallback(callbacks::SkinUpdate);
		seed_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&seed_sawedoff);

		color_one_sawedoff.setup(XOR("color one"), XOR("color_one_sawedoff"), colors::white);
		color_one_sawedoff.SetCallback(callbacks::UpdateSkinColor);
		color_one_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_one_sawedoff);

		color_two_sawedoff.setup(XOR("color two"), XOR("color_two_sawedoff"), colors::white);
		color_two_sawedoff.SetCallback(callbacks::UpdateSkinColor);
		color_two_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_two_sawedoff);

		color_three_sawedoff.setup(XOR("color three"), XOR("color_three_sawedoff"), colors::white);
		color_three_sawedoff.SetCallback(callbacks::UpdateSkinColor);
		color_three_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_three_sawedoff);

		color_four_sawedoff.setup(XOR("color four"), XOR("color_four_sawedoff"), colors::white);
		color_four_sawedoff.SetCallback(callbacks::UpdateSkinColor);
		color_four_sawedoff.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_four_sawedoff);

		id_tec9.setup(XOR("paintkit id"), XOR("id_tec9"), 3);
		id_tec9.SetCallback(callbacks::SkinUpdate);
		id_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&id_tec9);

		stattrak_tec9.setup(XOR("stattrak"), XOR("stattrak_tec9"));
		stattrak_tec9.SetCallback(callbacks::SkinUpdate);
		stattrak_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&stattrak_tec9);

		quality_tec9.setup(XOR("quality"), XOR("quality_tec9"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_tec9.SetCallback(callbacks::SkinUpdate);
		quality_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&quality_tec9);

		seed_tec9.setup(XOR("seed"), XOR("seed_tec9"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_tec9.SetCallback(callbacks::SkinUpdate);
		seed_tec9.AddShowCallback(callbacks::TEC9);
		RegisterElement(&seed_tec9);

		color_one_tec9.setup(XOR("color one"), XOR("color_one_tec9"), colors::white);
		color_one_tec9.SetCallback(callbacks::UpdateSkinColor);
		color_one_tec9.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_one_tec9);

		color_two_tec9.setup(XOR("color two"), XOR("color_two_tec9"), colors::white);
		color_two_tec9.SetCallback(callbacks::UpdateSkinColor);
		color_two_tec9.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_two_tec9);

		color_three_tec9.setup(XOR("color three"), XOR("color_three_tec9"), colors::white);
		color_three_tec9.SetCallback(callbacks::UpdateSkinColor);
		color_three_tec9.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_three_tec9);

		color_four_tec9.setup(XOR("color four"), XOR("color_four_tec9"), colors::white);
		color_four_tec9.SetCallback(callbacks::UpdateSkinColor);
		color_four_tec9.AddShowCallback(callbacks::SAWEDOFF);
		RegisterElement(&color_four_tec9);

		id_p2000.setup(XOR("paintkit id"), XOR("id_p2000"), 3);
		id_p2000.SetCallback(callbacks::SkinUpdate);
		id_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&id_p2000);

		stattrak_p2000.setup(XOR("stattrak"), XOR("stattrak_p2000"));
		stattrak_p2000.SetCallback(callbacks::SkinUpdate);
		stattrak_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&stattrak_p2000);

		quality_p2000.setup(XOR("quality"), XOR("quality_p2000"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_p2000.SetCallback(callbacks::SkinUpdate);
		quality_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&quality_p2000);

		seed_p2000.setup(XOR("seed"), XOR("seed_p2000"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_p2000.SetCallback(callbacks::SkinUpdate);
		seed_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&seed_p2000);

		color_one_p2000.setup(XOR("color one"), XOR("color_one_p2000"), colors::white);
		color_one_p2000.SetCallback(callbacks::UpdateSkinColor);
		color_one_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&color_one_p2000);

		color_two_p2000.setup(XOR("color two"), XOR("color_two_p2000"), colors::white);
		color_two_p2000.SetCallback(callbacks::UpdateSkinColor);
		color_two_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&color_two_p2000);

		color_three_p2000.setup(XOR("color three"), XOR("color_three_p2000"), colors::white);
		color_three_p2000.SetCallback(callbacks::UpdateSkinColor);
		color_three_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&color_three_p2000);

		color_four_p2000.setup(XOR("color four"), XOR("color_four_p2000"), colors::white);
		color_four_p2000.SetCallback(callbacks::UpdateSkinColor);
		color_four_p2000.AddShowCallback(callbacks::P2000);
		RegisterElement(&color_four_p2000);

		id_mp7.setup(XOR("paintkit id"), XOR("id_mp7"), 3);
		id_mp7.SetCallback(callbacks::SkinUpdate);
		id_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&id_mp7);

		stattrak_mp7.setup(XOR("stattrak"), XOR("stattrak_mp7"));
		stattrak_mp7.SetCallback(callbacks::SkinUpdate);
		stattrak_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&stattrak_mp7);

		quality_mp7.setup(XOR("quality"), XOR("quality_mp7"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mp7.SetCallback(callbacks::SkinUpdate);
		quality_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&quality_mp7);

		seed_mp7.setup(XOR("seed"), XOR("seed_mp7"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mp7.SetCallback(callbacks::SkinUpdate);
		seed_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&seed_mp7);

		color_one_mp7.setup(XOR("color one"), XOR("color_one_mp7"), colors::white);
		color_one_mp7.SetCallback(callbacks::UpdateSkinColor);
		color_one_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&color_one_mp7);

		color_two_mp7.setup(XOR("color two"), XOR("color_two_mp7"), colors::white);
		color_two_mp7.SetCallback(callbacks::UpdateSkinColor);
		color_two_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&color_two_mp7);

		color_three_mp7.setup(XOR("color three"), XOR("color_three_mp7"), colors::white);
		color_three_mp7.SetCallback(callbacks::UpdateSkinColor);
		color_three_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&color_three_mp7);

		color_four_mp7.setup(XOR("color four"), XOR("color_four_mp7"), colors::white);
		color_four_mp7.SetCallback(callbacks::UpdateSkinColor);
		color_four_mp7.AddShowCallback(callbacks::MP7);
		RegisterElement(&color_four_mp7);

		id_mp9.setup(XOR("paintkit id"), XOR("id_mp9"), 3);
		id_mp9.SetCallback(callbacks::SkinUpdate);
		id_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&id_mp9);

		stattrak_mp9.setup(XOR("stattrak"), XOR("stattrak_mp9"));
		stattrak_mp9.SetCallback(callbacks::SkinUpdate);
		stattrak_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&stattrak_mp9);

		quality_mp9.setup(XOR("quality"), XOR("quality_mp9"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_mp9.SetCallback(callbacks::SkinUpdate);
		quality_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&quality_mp9);

		seed_mp9.setup(XOR("seed"), XOR("seed_mp9"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_mp9.SetCallback(callbacks::SkinUpdate);
		seed_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&seed_mp9);

		color_one_mp9.setup(XOR("color one"), XOR("color_one_mp9"), colors::white);
		color_one_mp9.SetCallback(callbacks::UpdateSkinColor);
		color_one_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&color_one_mp9);

		color_two_mp9.setup(XOR("color two"), XOR("color_two_mp9"), colors::white);
		color_two_mp9.SetCallback(callbacks::UpdateSkinColor);
		color_two_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&color_two_mp9);

		color_three_mp9.setup(XOR("color three"), XOR("color_three_mp9"), colors::white);
		color_three_mp9.SetCallback(callbacks::UpdateSkinColor);
		color_three_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&color_three_mp9);

		color_four_mp9.setup(XOR("color four"), XOR("color_four_mp9"), colors::white);
		color_four_mp9.SetCallback(callbacks::UpdateSkinColor);
		color_four_mp9.AddShowCallback(callbacks::MP9);
		RegisterElement(&color_four_mp9);

		id_nova.setup(XOR("paintkit id"), XOR("id_nova"), 3);
		id_nova.SetCallback(callbacks::SkinUpdate);
		id_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&id_nova);

		stattrak_nova.setup(XOR("stattrak"), XOR("stattrak_nova"));
		stattrak_nova.SetCallback(callbacks::SkinUpdate);
		stattrak_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&stattrak_nova);

		quality_nova.setup(XOR("quality"), XOR("quality_nova"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_nova.SetCallback(callbacks::SkinUpdate);
		quality_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&quality_nova);

		seed_nova.setup(XOR("seed"), XOR("seed_nova"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_nova.SetCallback(callbacks::SkinUpdate);
		seed_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&seed_nova);

		color_one_nova.setup(XOR("color one"), XOR("color_one_nova"), colors::white);
		color_one_nova.SetCallback(callbacks::UpdateSkinColor);
		color_one_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&color_one_nova);

		color_two_nova.setup(XOR("color two"), XOR("color_two_nova"), colors::white);
		color_two_nova.SetCallback(callbacks::UpdateSkinColor);
		color_two_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&color_two_nova);

		color_three_nova.setup(XOR("color three"), XOR("color_three_nova"), colors::white);
		color_three_nova.SetCallback(callbacks::UpdateSkinColor);
		color_three_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&color_three_nova);

		color_four_nova.setup(XOR("color four"), XOR("color_four_nova"), colors::white);
		color_four_nova.SetCallback(callbacks::UpdateSkinColor);
		color_four_nova.AddShowCallback(callbacks::NOVA);
		RegisterElement(&color_four_nova);

		id_p250.setup(XOR("paintkit id"), XOR("id_p250"), 3);
		id_p250.SetCallback(callbacks::SkinUpdate);
		id_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&id_p250);

		stattrak_p250.setup(XOR("stattrak"), XOR("stattrak_p250"));
		stattrak_p250.SetCallback(callbacks::SkinUpdate);
		stattrak_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&stattrak_p250);

		quality_p250.setup(XOR("quality"), XOR("quality_p250"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_p250.SetCallback(callbacks::SkinUpdate);
		quality_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&quality_p250);

		seed_p250.setup(XOR("seed"), XOR("seed_p250"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_p250.SetCallback(callbacks::SkinUpdate);
		seed_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&seed_p250);

		color_one_p250.setup(XOR("color one"), XOR("color_one_p250"), colors::white);
		color_one_p250.SetCallback(callbacks::UpdateSkinColor);
		color_one_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&color_one_p250);

		color_two_p250.setup(XOR("color two"), XOR("color_two_p250"), colors::white);
		color_two_p250.SetCallback(callbacks::UpdateSkinColor);
		color_two_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&color_two_p250);

		color_three_p250.setup(XOR("color three"), XOR("color_three_p250"), colors::white);
		color_three_p250.SetCallback(callbacks::UpdateSkinColor);
		color_three_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&color_three_p250);

		color_four_p250.setup(XOR("color four"), XOR("color_four_p250"), colors::white);
		color_four_p250.SetCallback(callbacks::UpdateSkinColor);
		color_four_p250.AddShowCallback(callbacks::P250);
		RegisterElement(&color_four_p250);

		id_scar20.setup(XOR("paintkit id"), XOR("id_scar20"), 3);
		id_scar20.SetCallback(callbacks::SkinUpdate);
		id_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&id_scar20);

		stattrak_scar20.setup(XOR("stattrak"), XOR("stattrak_scar20"));
		stattrak_scar20.SetCallback(callbacks::SkinUpdate);
		stattrak_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&stattrak_scar20);

		quality_scar20.setup(XOR("quality"), XOR("quality_scar20"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_scar20.SetCallback(callbacks::SkinUpdate);
		quality_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&quality_scar20);

		seed_scar20.setup(XOR("seed"), XOR("seed_scar20"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_scar20.SetCallback(callbacks::SkinUpdate);
		seed_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&seed_scar20);

		color_one_scar20.setup(XOR("color one"), XOR("color_one_scar20"), colors::white);
		color_one_scar20.SetCallback(callbacks::UpdateSkinColor);
		color_one_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&color_one_scar20);

		color_two_scar20.setup(XOR("color two"), XOR("color_two_scar20"), colors::white);
		color_two_scar20.SetCallback(callbacks::UpdateSkinColor);
		color_two_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&color_two_scar20);

		color_three_scar20.setup(XOR("color three"), XOR("color_three_scar20"), colors::white);
		color_three_scar20.SetCallback(callbacks::UpdateSkinColor);
		color_three_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&color_three_scar20);

		color_four_scar20.setup(XOR("color four"), XOR("color_four_scar20"), colors::white);
		color_four_scar20.SetCallback(callbacks::UpdateSkinColor);
		color_four_scar20.AddShowCallback(callbacks::SCAR20);
		RegisterElement(&color_four_scar20);

		id_sg553.setup(XOR("paintkit id"), XOR("id_sg553"), 3);
		id_sg553.SetCallback(callbacks::SkinUpdate);
		id_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&id_sg553);

		stattrak_sg553.setup(XOR("stattrak"), XOR("stattrak_sg553"));
		stattrak_sg553.SetCallback(callbacks::SkinUpdate);
		stattrak_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&stattrak_sg553);

		quality_sg553.setup(XOR("quality"), XOR("quality_sg553"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_sg553.SetCallback(callbacks::SkinUpdate);
		quality_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&quality_sg553);

		seed_sg553.setup(XOR("seed"), XOR("seed_sg553"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_sg553.SetCallback(callbacks::SkinUpdate);
		seed_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&seed_sg553);

		color_one_sg553.setup(XOR("color one"), XOR("color_one_sg553"), colors::white);
		color_one_sg553.SetCallback(callbacks::UpdateSkinColor);
		color_one_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&color_one_sg553);

		color_two_sg553.setup(XOR("color two"), XOR("color_two_sg553"), colors::white);
		color_two_sg553.SetCallback(callbacks::UpdateSkinColor);
		color_two_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&color_two_sg553);

		color_three_sg553.setup(XOR("color three"), XOR("color_three_sg553"), colors::white);
		color_three_sg553.SetCallback(callbacks::UpdateSkinColor);
		color_three_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&color_three_sg553);

		color_four_sg553.setup(XOR("color four"), XOR("color_four_sg553"), colors::white);
		color_four_sg553.SetCallback(callbacks::UpdateSkinColor);
		color_four_sg553.AddShowCallback(callbacks::SG553);
		RegisterElement(&color_four_sg553);

		id_ssg08.setup(XOR("paintkit id"), XOR("id_ssg08"), 3);
		id_ssg08.SetCallback(callbacks::SkinUpdate);
		id_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&id_ssg08);

		stattrak_ssg08.setup(XOR("stattrak"), XOR("stattrak_ssg08"));
		stattrak_ssg08.SetCallback(callbacks::SkinUpdate);
		stattrak_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&stattrak_ssg08);

		quality_ssg08.setup(XOR("quality"), XOR("quality_ssg08"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_ssg08.SetCallback(callbacks::SkinUpdate);
		quality_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&quality_ssg08);

		seed_ssg08.setup(XOR("seed"), XOR("seed_ssg08"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_ssg08.SetCallback(callbacks::SkinUpdate);
		seed_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&seed_ssg08);

		color_one_ssg08.setup(XOR("color one"), XOR("color_one_ssg08"), colors::white);
		color_one_ssg08.SetCallback(callbacks::UpdateSkinColor);
		color_one_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&color_one_ssg08);

		color_two_ssg08.setup(XOR("color two"), XOR("color_two_ssg08"), colors::white);
		color_two_ssg08.SetCallback(callbacks::UpdateSkinColor);
		color_two_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&color_two_ssg08);

		color_three_ssg08.setup(XOR("color three"), XOR("color_three_ssg08"), colors::white);
		color_three_ssg08.SetCallback(callbacks::UpdateSkinColor);
		color_three_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&color_three_ssg08);

		color_four_ssg08.setup(XOR("color four"), XOR("color_four_ssg08"), colors::white);
		color_four_ssg08.SetCallback(callbacks::UpdateSkinColor);
		color_four_ssg08.AddShowCallback(callbacks::SSG08);
		RegisterElement(&color_four_ssg08);

		id_m4a1s.setup(XOR("paintkit id"), XOR("id_m4a1s"), 3);
		id_m4a1s.SetCallback(callbacks::SkinUpdate);
		id_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&id_m4a1s);

		stattrak_m4a1s.setup(XOR("stattrak"), XOR("stattrak_m4a1s"));
		stattrak_m4a1s.SetCallback(callbacks::SkinUpdate);
		stattrak_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&stattrak_m4a1s);

		quality_m4a1s.setup(XOR("quality"), XOR("quality_m4a1s"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m4a1s.SetCallback(callbacks::SkinUpdate);
		quality_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&quality_m4a1s);

		seed_m4a1s.setup(XOR("seed"), XOR("seed_m4a1s"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m4a1s.SetCallback(callbacks::SkinUpdate);
		seed_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&seed_m4a1s);

		color_one_m4a1s.setup(XOR("color one"), XOR("color_one_m4a1s"), colors::white);
		color_one_m4a1s.SetCallback(callbacks::UpdateSkinColor);
		color_one_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&color_one_m4a1s);

		color_two_m4a1s.setup(XOR("color two"), XOR("color_two_m4a1s"), colors::white);
		color_two_m4a1s.SetCallback(callbacks::UpdateSkinColor);
		color_two_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&color_two_m4a1s);

		color_three_m4a1s.setup(XOR("color three"), XOR("color_three_m4a1s"), colors::white);
		color_three_m4a1s.SetCallback(callbacks::UpdateSkinColor);
		color_three_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&color_three_m4a1s);

		color_four_m4a1s.setup(XOR("color four"), XOR("color_four_m4a1s"), colors::white);
		color_four_m4a1s.SetCallback(callbacks::UpdateSkinColor);
		color_four_m4a1s.AddShowCallback(callbacks::M4A1S);
		RegisterElement(&color_four_m4a1s);

		id_usps.setup(XOR("paintkit id"), XOR("id_usps"), 3);
		id_usps.SetCallback(callbacks::SkinUpdate);
		id_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&id_usps);

		stattrak_usps.setup(XOR("stattrak"), XOR("stattrak_usps"));
		stattrak_usps.SetCallback(callbacks::SkinUpdate);
		stattrak_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&stattrak_usps);

		quality_usps.setup(XOR("quality"), XOR("quality_usps"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_usps.SetCallback(callbacks::SkinUpdate);
		quality_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&quality_usps);

		seed_usps.setup(XOR("seed"), XOR("seed_usps"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_usps.SetCallback(callbacks::SkinUpdate);
		seed_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&seed_usps);

		color_one_usps.setup(XOR("color one"), XOR("color_one_usps"), colors::white);
		color_one_usps.SetCallback(callbacks::UpdateSkinColor);
		color_one_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&color_one_usps);

		color_two_usps.setup(XOR("color two"), XOR("color_two_usps"), colors::white);
		color_two_usps.SetCallback(callbacks::UpdateSkinColor);
		color_two_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&color_two_usps);

		color_three_usps.setup(XOR("color three"), XOR("color_three_usps"), colors::white);
		color_three_usps.SetCallback(callbacks::UpdateSkinColor);
		color_three_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&color_three_usps);

		color_four_usps.setup(XOR("color four"), XOR("color_four_usps"), colors::white);
		color_four_usps.SetCallback(callbacks::UpdateSkinColor);
		color_four_usps.AddShowCallback(callbacks::USPS);
		RegisterElement(&color_four_usps);

		id_cz75a.setup(XOR("paintkit id"), XOR("id_cz75a"), 3);
		id_cz75a.SetCallback(callbacks::SkinUpdate);
		id_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&id_cz75a);

		stattrak_cz75a.setup(XOR("stattrak"), XOR("stattrak_cz75a"));
		stattrak_cz75a.SetCallback(callbacks::SkinUpdate);
		stattrak_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&stattrak_cz75a);

		quality_cz75a.setup(XOR("quality"), XOR("quality_cz75a"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_cz75a.SetCallback(callbacks::SkinUpdate);
		quality_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&quality_cz75a);

		seed_cz75a.setup(XOR("seed"), XOR("seed_cz75a"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_cz75a.SetCallback(callbacks::SkinUpdate);
		seed_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&seed_cz75a);

		color_one_cz75a.setup(XOR("color one"), XOR("color_one_cz75a"), colors::white);
		color_one_cz75a.SetCallback(callbacks::UpdateSkinColor);
		color_one_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&color_one_cz75a);

		color_two_cz75a.setup(XOR("color two"), XOR("color_two_cz75a"), colors::white);
		color_two_cz75a.SetCallback(callbacks::UpdateSkinColor);
		color_two_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&color_two_cz75a);

		color_three_cz75a.setup(XOR("color three"), XOR("color_three_cz75a"), colors::white);
		color_three_cz75a.SetCallback(callbacks::UpdateSkinColor);
		color_three_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&color_three_cz75a);

		color_four_cz75a.setup(XOR("color four"), XOR("color_four_cz75a"), colors::white);
		color_four_cz75a.SetCallback(callbacks::UpdateSkinColor);
		color_four_cz75a.AddShowCallback(callbacks::CZ75A);
		RegisterElement(&color_four_cz75a);

		id_revolver.setup(XOR("paintkit id"), XOR("id_revolver"), 3);
		id_revolver.SetCallback(callbacks::SkinUpdate);
		id_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&id_revolver);

		stattrak_revolver.setup(XOR("stattrak"), XOR("stattrak_revolver"));
		stattrak_revolver.SetCallback(callbacks::SkinUpdate);
		stattrak_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&stattrak_revolver);

		quality_revolver.setup(XOR("quality"), XOR("quality_revolver"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_revolver.SetCallback(callbacks::SkinUpdate);
		quality_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&quality_revolver);

		seed_revolver.setup(XOR("seed"), XOR("seed_revolver"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_revolver.SetCallback(callbacks::SkinUpdate);
		seed_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&seed_revolver);

		color_one_revolver.setup(XOR("color one"), XOR("color_one_revolver"), colors::white);
		color_one_revolver.SetCallback(callbacks::UpdateSkinColor);
		color_one_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&color_one_revolver);

		color_two_revolver.setup(XOR("color two"), XOR("color_two_revolver"), colors::white);
		color_two_revolver.SetCallback(callbacks::UpdateSkinColor);
		color_two_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&color_two_revolver);

		color_three_revolver.setup(XOR("color three"), XOR("color_three_revolver"), colors::white);
		color_three_revolver.SetCallback(callbacks::UpdateSkinColor);
		color_three_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&color_three_revolver);

		color_four_revolver.setup(XOR("color four"), XOR("color_four_revolver"), colors::white);
		color_four_revolver.SetCallback(callbacks::UpdateSkinColor);
		color_four_revolver.AddShowCallback(callbacks::REVOLVER);
		RegisterElement(&color_four_revolver);

		id_bayonet.setup(XOR("paintkit id"), XOR("id_bayonet"), 3);
		id_bayonet.SetCallback(callbacks::SkinUpdate);
		id_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&id_bayonet);

		stattrak_bayonet.setup(XOR("stattrak"), XOR("stattrak_bayonet"));
		stattrak_bayonet.SetCallback(callbacks::SkinUpdate);
		stattrak_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&stattrak_bayonet);

		quality_bayonet.setup(XOR("quality"), XOR("quality_bayonet"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_bayonet.SetCallback(callbacks::SkinUpdate);
		quality_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&quality_bayonet);

		seed_bayonet.setup(XOR("seed"), XOR("seed_bayonet"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_bayonet.SetCallback(callbacks::SkinUpdate);
		seed_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&seed_bayonet);

		color_one_bayonet.setup(XOR("color one"), XOR("color_one_bayonet"), colors::white);
		color_one_bayonet.SetCallback(callbacks::UpdateSkinColor);
		color_one_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&color_one_bayonet);

		color_two_bayonet.setup(XOR("color two"), XOR("color_two_bayonet"), colors::white);
		color_two_bayonet.SetCallback(callbacks::UpdateSkinColor);
		color_two_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&color_two_bayonet);

		color_three_bayonet.setup(XOR("color three"), XOR("color_three_bayonet"), colors::white);
		color_three_bayonet.SetCallback(callbacks::UpdateSkinColor);
		color_three_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&color_three_bayonet);

		color_four_bayonet.setup(XOR("color four"), XOR("color_four_bayonet"), colors::white);
		color_four_bayonet.SetCallback(callbacks::UpdateSkinColor);
		color_four_bayonet.AddShowCallback(callbacks::KNIFE_BAYONET);
		RegisterElement(&color_four_bayonet);

		id_flip.setup(XOR("paintkit id"), XOR("id_flip"), 3);
		id_flip.SetCallback(callbacks::SkinUpdate);
		id_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&id_flip);

		stattrak_flip.setup(XOR("stattrak"), XOR("stattrak_flip"));
		stattrak_flip.SetCallback(callbacks::SkinUpdate);
		stattrak_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&stattrak_flip);

		quality_flip.setup(XOR("quality"), XOR("quality_flip"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_flip.SetCallback(callbacks::SkinUpdate);
		quality_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&quality_flip);

		seed_flip.setup(XOR("seed"), XOR("seed_flip"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_flip.SetCallback(callbacks::SkinUpdate);
		seed_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&seed_flip);

		color_one_flip.setup(XOR("color one"), XOR("color_one_flip"), colors::white);
		color_one_flip.SetCallback(callbacks::UpdateSkinColor);
		color_one_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&color_one_flip);

		color_two_flip.setup(XOR("color two"), XOR("color_two_flip"), colors::white);
		color_two_flip.SetCallback(callbacks::UpdateSkinColor);
		color_two_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&color_two_flip);

		color_three_flip.setup(XOR("color three"), XOR("color_three_flip"), colors::white);
		color_three_flip.SetCallback(callbacks::UpdateSkinColor);
		color_three_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&color_three_flip);

		color_four_flip.setup(XOR("color four"), XOR("color_four_flip"), colors::white);
		color_four_flip.SetCallback(callbacks::UpdateSkinColor);
		color_four_flip.AddShowCallback(callbacks::KNIFE_FLIP);
		RegisterElement(&color_four_flip);

		id_gut.setup(XOR("paintkit id"), XOR("id_gut"), 3);
		id_gut.SetCallback(callbacks::SkinUpdate);
		id_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&id_gut);

		stattrak_gut.setup(XOR("stattrak"), XOR("stattrak_gut"));
		stattrak_gut.SetCallback(callbacks::SkinUpdate);
		stattrak_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&stattrak_gut);

		quality_gut.setup(XOR("quality"), XOR("quality_gut"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_gut.SetCallback(callbacks::SkinUpdate);
		quality_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&quality_gut);

		seed_gut.setup(XOR("seed"), XOR("seed_gut"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_gut.SetCallback(callbacks::SkinUpdate);
		seed_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&seed_gut);

		color_one_gut.setup(XOR("color one"), XOR("color_one_gut"), colors::white);
		color_one_gut.SetCallback(callbacks::UpdateSkinColor);
		color_one_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&color_one_gut);

		color_two_gut.setup(XOR("color two"), XOR("color_two_gut"), colors::white);
		color_two_gut.SetCallback(callbacks::UpdateSkinColor);
		color_two_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&color_two_gut);

		color_three_gut.setup(XOR("color three"), XOR("color_three_gut"), colors::white);
		color_three_gut.SetCallback(callbacks::UpdateSkinColor);
		color_three_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&color_three_gut);

		color_four_gut.setup(XOR("color four"), XOR("color_four_gut"), colors::white);
		color_four_gut.SetCallback(callbacks::UpdateSkinColor);
		color_four_gut.AddShowCallback(callbacks::KNIFE_GUT);
		RegisterElement(&color_four_gut);

		id_karambit.setup(XOR("paintkit id"), XOR("id_karambit"), 3);
		id_karambit.SetCallback(callbacks::SkinUpdate);
		id_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&id_karambit);

		stattrak_karambit.setup(XOR("stattrak"), XOR("stattrak_karambit"));
		stattrak_karambit.SetCallback(callbacks::SkinUpdate);
		stattrak_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&stattrak_karambit);

		quality_karambit.setup(XOR("quality"), XOR("quality_karambit"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_karambit.SetCallback(callbacks::SkinUpdate);
		quality_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&quality_karambit);

		seed_karambit.setup(XOR("seed"), XOR("seed_karambit"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_karambit.SetCallback(callbacks::SkinUpdate);
		seed_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&seed_karambit);

		color_one_karambit.setup(XOR("color one"), XOR("color_one_karambit"), colors::white);
		color_one_karambit.SetCallback(callbacks::UpdateSkinColor);
		color_one_karambit.SetCallback(callbacks::SkinUpdate);
		color_one_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&color_one_karambit);

		color_two_karambit.setup(XOR("color two"), XOR("color_two_karambit"), colors::white);
		color_two_karambit.SetCallback(callbacks::UpdateSkinColor);
		color_two_karambit.SetCallback(callbacks::SkinUpdate);
		color_two_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&color_two_karambit);

		color_three_karambit.setup(XOR("color three"), XOR("color_three_karambit"), colors::white);
		color_three_karambit.SetCallback(callbacks::UpdateSkinColor);
		color_three_karambit.SetCallback(callbacks::SkinUpdate);
		color_three_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&color_three_karambit);

		color_four_karambit.setup(XOR("color four"), XOR("color_four_karambit"), colors::white);
		color_four_karambit.SetCallback(callbacks::UpdateSkinColor);
		color_four_karambit.SetCallback(callbacks::SkinUpdate);
		color_four_karambit.AddShowCallback(callbacks::KNIFE_KARAMBIT);
		RegisterElement(&color_four_karambit);

		id_m9.setup(XOR("paintkit id"), XOR("id_m9"), 3);
		id_m9.SetCallback(callbacks::SkinUpdate);
		id_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&id_m9);

		stattrak_m9.setup(XOR("stattrak"), XOR("stattrak_m9"));
		stattrak_m9.SetCallback(callbacks::SkinUpdate);
		stattrak_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&stattrak_m9);

		quality_m9.setup(XOR("quality"), XOR("quality_m9"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_m9.SetCallback(callbacks::SkinUpdate);
		quality_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&quality_m9);

		seed_m9.setup(XOR("seed"), XOR("seed_m9"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_m9.SetCallback(callbacks::SkinUpdate);
		seed_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&seed_m9);

		color_one_m9.setup(XOR("color one"), XOR("color_one_m9"), colors::white);
		color_one_m9.SetCallback(callbacks::UpdateSkinColor);
		color_one_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&color_one_m9);

		color_two_m9.setup(XOR("color two"), XOR("color_two_m9"), colors::white);
		color_two_m9.SetCallback(callbacks::UpdateSkinColor);
		color_two_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&color_two_m9);

		color_three_m9.setup(XOR("color three"), XOR("color_three_m9"), colors::white);
		color_three_m9.SetCallback(callbacks::UpdateSkinColor);
		color_three_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&color_three_m9);

		color_four_m9.setup(XOR("color four"), XOR("color_four_m9"), colors::white);
		color_four_m9.SetCallback(callbacks::UpdateSkinColor);
		color_four_m9.AddShowCallback(callbacks::KNIFE_M9_BAYONET);
		RegisterElement(&color_four_m9);

		id_huntsman.setup(XOR("paintkit id"), XOR("id_huntsman"), 3);
		id_huntsman.SetCallback(callbacks::SkinUpdate);
		id_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&id_huntsman);

		stattrak_huntsman.setup(XOR("stattrak"), XOR("stattrak_huntsman"));
		stattrak_huntsman.SetCallback(callbacks::SkinUpdate);
		stattrak_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&stattrak_huntsman);

		quality_huntsman.setup(XOR("quality"), XOR("quality_huntsman"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_huntsman.SetCallback(callbacks::SkinUpdate);
		quality_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&quality_huntsman);

		seed_huntsman.setup(XOR("seed"), XOR("seed_huntsman"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_huntsman.SetCallback(callbacks::SkinUpdate);
		seed_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&seed_huntsman);

		color_one_huntsman.setup(XOR("color one"), XOR("color_one_huntsman"), colors::white);
		color_one_huntsman.SetCallback(callbacks::UpdateSkinColor);
		color_one_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&color_one_huntsman);

		color_two_huntsman.setup(XOR("color two"), XOR("color_two_huntsman"), colors::white);
		color_two_huntsman.SetCallback(callbacks::UpdateSkinColor);
		color_two_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&color_two_huntsman);

		color_three_huntsman.setup(XOR("color three"), XOR("color_three_huntsman"), colors::white);
		color_three_huntsman.SetCallback(callbacks::UpdateSkinColor);
		color_three_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&color_three_huntsman);

		color_four_huntsman.setup(XOR("color four"), XOR("color_four_huntsman"), colors::white);
		color_four_huntsman.SetCallback(callbacks::UpdateSkinColor);
		color_four_huntsman.AddShowCallback(callbacks::KNIFE_HUNTSMAN);
		RegisterElement(&color_four_huntsman);

		id_falchion.setup(XOR("paintkit id"), XOR("id_falchion"), 3);
		id_falchion.SetCallback(callbacks::SkinUpdate);
		id_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&id_falchion);

		stattrak_falchion.setup(XOR("stattrak"), XOR("stattrak_falchion"));
		stattrak_falchion.SetCallback(callbacks::SkinUpdate);
		stattrak_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&stattrak_falchion);

		quality_falchion.setup(XOR("quality"), XOR("quality_falchion"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_falchion.SetCallback(callbacks::SkinUpdate);
		quality_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&quality_falchion);

		seed_falchion.setup(XOR("seed"), XOR("seed_falchion"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_falchion.SetCallback(callbacks::SkinUpdate);
		seed_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&seed_falchion);

		color_one_falchion.setup(XOR("color one"), XOR("color_one_falchion"), colors::white);
		color_one_falchion.SetCallback(callbacks::UpdateSkinColor);
		color_one_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&color_one_falchion);

		color_two_falchion.setup(XOR("color two"), XOR("color_two_falchion"), colors::white);
		color_two_falchion.SetCallback(callbacks::UpdateSkinColor);
		color_two_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&color_two_falchion);

		color_three_falchion.setup(XOR("color three"), XOR("color_three_falchion"), colors::white);
		color_three_falchion.SetCallback(callbacks::UpdateSkinColor);
		color_three_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&color_three_falchion);

		color_four_falchion.setup(XOR("color four"), XOR("color_four_falchion"), colors::white);
		color_four_falchion.SetCallback(callbacks::UpdateSkinColor);
		color_four_falchion.AddShowCallback(callbacks::KNIFE_FALCHION);
		RegisterElement(&color_four_falchion);

		id_bowie.setup(XOR("paintkit id"), XOR("id_bowie"), 3);
		id_bowie.SetCallback(callbacks::SkinUpdate);
		id_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&id_bowie);

		stattrak_bowie.setup(XOR("stattrak"), XOR("stattrak_bowie"));
		stattrak_bowie.SetCallback(callbacks::SkinUpdate);
		stattrak_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&stattrak_bowie);

		quality_bowie.setup(XOR("quality"), XOR("quality_bowie"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_bowie.SetCallback(callbacks::SkinUpdate);
		quality_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&quality_bowie);

		seed_bowie.setup(XOR("seed"), XOR("seed_bowie"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_bowie.SetCallback(callbacks::SkinUpdate);
		seed_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&seed_bowie);

		color_one_bowie.setup(XOR("color one"), XOR("color_one_bowie"), colors::white);
		color_one_bowie.SetCallback(callbacks::UpdateSkinColor);
		color_one_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&color_one_bowie);

		color_two_bowie.setup(XOR("color two"), XOR("color_two_bowie"), colors::white);
		color_two_bowie.SetCallback(callbacks::UpdateSkinColor);
		color_two_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&color_two_bowie);

		color_three_bowie.setup(XOR("color three"), XOR("color_three_bowie"), colors::white);
		color_three_bowie.SetCallback(callbacks::UpdateSkinColor);
		color_three_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&color_three_bowie);

		color_four_bowie.setup(XOR("color four"), XOR("color_four_bowie"), colors::white);
		color_four_bowie.SetCallback(callbacks::UpdateSkinColor);
		color_four_bowie.AddShowCallback(callbacks::KNIFE_BOWIE);
		RegisterElement(&color_four_bowie);

		id_butterfly.setup(XOR("paintkit id"), XOR("id_butterfly"), 3);
		id_butterfly.SetCallback(callbacks::SkinUpdate);
		id_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&id_butterfly);

		stattrak_butterfly.setup(XOR("stattrak"), XOR("stattrak_butterfly"));
		stattrak_butterfly.SetCallback(callbacks::SkinUpdate);
		stattrak_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&stattrak_butterfly);

		quality_butterfly.setup(XOR("quality"), XOR("quality_butterfly"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_butterfly.SetCallback(callbacks::SkinUpdate);
		quality_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&quality_butterfly);

		seed_butterfly.setup(XOR("seed"), XOR("seed_butterfly"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_butterfly.SetCallback(callbacks::SkinUpdate);
		seed_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&seed_butterfly);

		color_one_butterfly.setup(XOR("color one"), XOR("color_one_butterfly"), colors::white);
		color_one_butterfly.SetCallback(callbacks::UpdateSkinColor);
		color_one_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&color_one_butterfly);

		color_two_butterfly.setup(XOR("color two"), XOR("color_two_butterfly"), colors::white);
		color_two_butterfly.SetCallback(callbacks::UpdateSkinColor);
		color_two_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&color_two_butterfly);

		color_three_butterfly.setup(XOR("color three"), XOR("color_three_butterfly"), colors::white);
		color_three_butterfly.SetCallback(callbacks::UpdateSkinColor);
		color_three_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&color_three_butterfly);

		color_four_butterfly.setup(XOR("color four"), XOR("color_four_butterfly"), colors::white);
		color_four_butterfly.SetCallback(callbacks::UpdateSkinColor);
		color_four_butterfly.AddShowCallback(callbacks::KNIFE_BUTTERFLY);
		RegisterElement(&color_four_butterfly);

		id_daggers.setup(XOR("paintkit id"), XOR("id_daggers"), 3);
		id_daggers.SetCallback(callbacks::SkinUpdate);
		id_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&id_daggers);

		stattrak_daggers.setup(XOR("stattrak"), XOR("stattrak_daggers"));
		stattrak_daggers.SetCallback(callbacks::SkinUpdate);
		stattrak_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&stattrak_daggers);

		quality_daggers.setup(XOR("quality"), XOR("quality_daggers"), 1.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		quality_daggers.SetCallback(callbacks::SkinUpdate);
		quality_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&quality_daggers);

		seed_daggers.setup(XOR("seed"), XOR("seed_daggers"), 0.f, 255.f, true, 0, 0.f, 1.f);
		seed_daggers.SetCallback(callbacks::SkinUpdate);
		seed_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&seed_daggers);

		color_one_daggers.setup(XOR("color one"), XOR("color_one_daggers"), colors::white);
		color_one_daggers.SetCallback(callbacks::UpdateSkinColor);
		color_one_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&color_one_daggers);

		color_two_daggers.setup(XOR("color two"), XOR("color_two_daggers"), colors::white);
		color_two_daggers.SetCallback(callbacks::UpdateSkinColor);
		color_two_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&color_two_daggers);

		color_three_daggers.setup(XOR("color three"), XOR("color_three_daggers"), colors::white);
		color_three_daggers.SetCallback(callbacks::UpdateSkinColor);
		color_three_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&color_three_daggers);

		color_four_daggers.setup(XOR("color four"), XOR("color_four_daggers"), colors::white);
		color_four_daggers.SetCallback(callbacks::UpdateSkinColor);
		color_four_daggers.AddShowCallback(callbacks::KNIFE_SHADOW_DAGGERS);
		RegisterElement(&color_four_daggers);

		color_update.setup(XOR("update skin colors"));
		color_update.SetCallback(callbacks::ForceWeaponColor);
		color_update.AddShowCallback(callbacks::HasWeapon);
		RegisterElement(&color_update);

		// col 2.
		knife.setup(XOR("knife model"), XOR("skins_knife_model"), { XOR("off"), XOR("bayonet"), XOR("bowie"), XOR("butterfly"), XOR("falchion"), XOR("flip"), XOR("gut"), XOR("huntsman"), XOR("karambit"), XOR("m9 bayonet"), XOR("daggers") });
		knife.SetCallback(callbacks::SkinUpdate);
		RegisterElement(&knife, 1);

		glove.setup(XOR("glove model"), XOR("skins_glove_model"), { XOR("off"), XOR("bloodhound"), XOR("sport"), XOR("driver"), XOR("handwraps"), XOR("moto"), XOR("specialist") });
		glove.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&glove, 1);

		glove_id.setup(XOR("glove paintkit id"), XOR("skins_glove_id"), 2);
		glove_id.SetCallback(callbacks::ForceFullUpdate);
		RegisterElement(&glove_id, 1);

		modulation.setup(XOR("skin modulation"), XOR("modulation"));
		RegisterElement(&modulation, 1);

		load.setup(XOR("load skin cfg"));
		load.SetCallback(callbacks::ConfigLoadSkins);
		RegisterElement(&load, 1);

		save.setup(XOR("save skin cfg"));
		save.SetCallback(callbacks::ConfigSaveSkins);
		RegisterElement(&save, 1);
	}
};

class MovementTab : public Tab {
public:
	Keybind       instant_stop_in_air;
	Checkbox quick_stop;
	Checkbox between_shots;
	Checkbox in_air;
	Slider air_hc;
	Checkbox	  fast_stop;
	Checkbox      bunny_hop;
	Checkbox      auto_strafe;
	Slider astrafe_smoothness;
	Dropdown strafe_type;
	Keybind       fake_walk;
	Dropdown	leg_movement;
	Keybind autopeek;
	Checkbox knife_autopeek;
public:
	void init() {
		SetTitle(XOR("movement"));

		fast_stop.setup(XOR("fast stop"), XOR("fast_stop"));
		RegisterElement(&fast_stop);

		instant_stop_in_air.setup(XOR("instant stop in air"), XOR("instant_stop_in_air"));
		RegisterElement(&instant_stop_in_air);

		air_hc.setup("in air hit-chance", XOR("air_hc"), 0.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		air_hc.AddShowCallback(callbacks::is_instant_stop_air_on);
		RegisterElement(&air_hc);

		bunny_hop.setup(XOR("bunny hop"), XOR("bunny_hop"));
		RegisterElement(&bunny_hop);

		auto_strafe.setup(XOR("automatic strafe"), XOR("auto_strafe"));
		RegisterElement(&auto_strafe);

		astrafe_smoothness.setup("smoothness", XOR("astrafe_smoothness"), 0.f, 100.f, true, 0, 50.f, 1.f, XOR(L"%"));
		RegisterElement(&astrafe_smoothness);


		leg_movement.setup(XOR("leg movement"), XOR("fix_leg_movement"), { "off", "always slide", "never slide" });
		RegisterElement(&leg_movement);

		// 2nd colon.
		autopeek.setup(XOR("auto peek"), XOR("autopeek"));
		RegisterElement(&autopeek, 1);

		knife_autopeek.setup(XOR("switch to knife after shot"), XOR("knife_autopeek"));
		knife_autopeek.AddShowCallback(callbacks::autopeek_on);
		RegisterElement(&knife_autopeek, 1);

		fake_walk.setup(XOR("fake walk"), XOR("fake_walk"));
		RegisterElement(&fake_walk, 1);
	}
};

class MiscTab : public Tab {
public:
	// col1.
	Slider   fov_amount;
	Slider   fov_scoped_percent;
	Checkbox viewmodel_fov;
	Slider	viewmodel_fov_percent;
	Checkbox      auto_buy;
	Dropdown      auto_buy_primary;
	Dropdown	  auto_buy_secondary;
	MultiDropdown auto_buy_misc;
	Checkbox      logs_buy;
	Checkbox      logs_damage;
	Checkbox       ping_spike;
	Slider		  ping_spike_amt;
	Keybind       sec_ping_spike;
	Slider		  sec_ping_spike_amt;
	Checkbox preserve_killfeed;

	Checkbox ranks;
	Checkbox whitelist;
	Button connect_1;
	Button connect_2;
	Button connect_3;

	// col2.
	Colorpicker menu_color;
	Dropdown	  main_esp_font;
	Dropdown	  main_esp_font_case;
	Dropdown	  secondary_esp_font;
	Dropdown	  secondary_esp_font_case;
	Dropdown	  projectile_esp_font;
	Dropdown	  projectile_esp_font_case;
	Slider		  spacing;
	Dropdown config;
	Keybind  key1;
	Keybind  key2;
	Keybind  key3;
	Keybind  key4;
	Keybind  key5;
	Keybind  key6;
	Keybind  key7;
	Keybind  key8;
	Keybind  key9;
	Keybind  key10;
	Button   save;
	Button   load;
	Button   cfg_import;
	Button   cfg_export;
	Button	 reset;
	Dropdown serverlist;
	Button   connectna;
	Button	 connect1na;
	Button   connecteu;
	Button   connect1eu;
	Checkbox increase;
	MultiDropdown     hitmarker;
	MultiDropdown     dynamic_col;
	Checkbox		  hitmarker_sound;
	Slider		      hitsound_volume;
	Dropdown	      hitsound_name;
	Checkbox disable_blur;
	Checkbox debugg;

public:
	void init() {
		SetTitle(XOR("misc"));

		fov_amount.setup("override fov", XOR("fov_amount"), 60.f, 140.f, true, 0, 90.f, 1.f, XOR(L"°"));
		RegisterElement(&fov_amount);

		fov_scoped_percent.setup("override scoped fov", XOR("fov_scoped_percent"), 0.f, 100.f, true, 0, 100.f, 1.f, XOR(L"%"));
		RegisterElement(&fov_scoped_percent);

		viewmodel_fov.setup(XOR("override viewmodel fov"), XOR("viewmodel_fov"));
		RegisterElement(&viewmodel_fov);

		viewmodel_fov_percent.setup("", XOR("viewmodel_fov_percent"), 60.f, 140.f, false, 0, 90.f, 1.f, XOR(L"°"));
		viewmodel_fov_percent.AddShowCallback(callbacks::is_viewmodel_fov);
		RegisterElement(&viewmodel_fov_percent);

		ping_spike.setup(XOR("fake latency"), XOR("ping_spike"));
		RegisterElement(&ping_spike);

		ping_spike_amt.setup("", XOR("fake_latency_amt"), 50.f, 1000.f / 2.f, false, 0, 200.f, 10.f, XOR(L"ms"));
		ping_spike_amt.AddShowCallback(callbacks::is_fake_ping_bound);
		RegisterElement(&ping_spike_amt);

		sec_ping_spike.setup(XOR("secondary fake latency"), XOR("sec_ping_spike"));
		sec_ping_spike.SetToggleCallback(callbacks::toggle_sec_fake_ping);
		sec_ping_spike.AddShowCallback(callbacks::is_fake_ping_bound);
		RegisterElement(&sec_ping_spike);

		sec_ping_spike_amt.setup("", XOR("sec_ping_spike_amt"), 50.f, 1000.f, false, 0, 200.f, 10.f, XOR(L"ms"));
		sec_ping_spike_amt.AddShowCallback(callbacks::is_sec_fake_ping_bound);
		sec_ping_spike_amt.AddShowCallback(callbacks::is_fake_ping_bound);
		RegisterElement(&sec_ping_spike_amt);

		logs_buy.setup(XOR("log weapon purchases"), XOR("logs_buy"));
		RegisterElement(&logs_buy);

		logs_damage.setup(XOR("log damage dealt"), XOR("logs_damage"));
		RegisterElement(&logs_damage);

		preserve_killfeed.setup(XOR("persistent killfeed"), XOR("preserve_killfeed"));
		preserve_killfeed.SetCallback(callbacks::clear_killfeed);
		RegisterElement(&preserve_killfeed);

		disable_blur.setup(XOR("disable menu background"), XOR("disable_blur"));
		RegisterElement(&disable_blur);

		debugg.setup(XOR("debug!"), XOR("debugg"));
		RegisterElement(&debugg);

		auto_buy.setup(XOR("auto buy"), XOR("auto_buy"));
		RegisterElement(&auto_buy);

		auto_buy_primary.setup(XOR(""), XOR("auto_buy_primary"),
			{
				XOR("none"),
				XOR("awp"),
				XOR("ssg08"),
				XOR("g3sg1/scar-20"),
			}, false);

		auto_buy_primary.AddShowCallback(callbacks::is_auto_buy_on);
		RegisterElement(&auto_buy_primary);

		auto_buy_secondary.setup("", XOR("auto_buy_secondary"),
			{
				XOR("none"),
				XOR("elite"),
				XOR("p250"),
				XOR("tec9/five-seven"),
				XOR("deagle/r8"),
			}, false);

		auto_buy_secondary.AddShowCallback(callbacks::is_auto_buy_on);
		RegisterElement(&auto_buy_secondary);

		auto_buy_misc.setup("", XOR("auto_buy3"),
			{
				XOR("vesthelm"),
				XOR("taser"),
				XOR("defuser"),
				XOR("molotov"),
				XOR("hegrenade"),
				XOR("smokegrenade"),
			}, false);

		auto_buy_misc.AddShowCallback(callbacks::is_auto_buy_on);
		RegisterElement(&auto_buy_misc);

		hitmarker.setup(XOR("hitmarker"), XOR("hitmarker"), { XOR("screen"), XOR("bullet"), XOR("dynamic color"), XOR("hit-sound"), XOR("custom hit-sound")});
		RegisterElement(&hitmarker);

		dynamic_col.setup(XOR("dynamic color"), XOR("dynamic_col"), { XOR("screen"), XOR("bullet")});
		dynamic_col.AddShowCallback(callbacks::isdynamiccolor);
		RegisterElement(&dynamic_col);

		const auto GetScripts = [&]() {
			std::string dir = XOR("C:\\nigus\\sounds");

			if (!std::filesystem::exists(dir))
				std::filesystem::create_directories(dir);

			for (auto& file_path : std::filesystem::directory_iterator(dir)) {
				if (!file_path.path().string().empty()) {
					if (file_path.path().string().find(".wav") != std::string::npos) {
						g_cl.m_hitsounds.emplace_back(file_path.path().string().erase(0, dir.length()));
						g_cl.m_menu.emplace_back(file_path.path().string().erase(0, dir.length()));

						for (int i = 0; i < g_cl.m_menu.size(); i++)
							if (g_cl.m_menu.at(i).front() == '\\')
								g_cl.m_menu.at(i).erase(0, 1);
					}
				}
			}
		};

		if (g_cl.m_hitsounds.empty()) {
			GetScripts();
		}

		if (g_cl.m_hitsounds.empty()) {
			hitsound_name.setup(XOR("custom hitsounds"), XOR("custom_hitsound"), { XOR("none in folder") });
			hitsound_name.AddShowCallback(callbacks::IsCustomHitsound);
			hitsound_name.SetCallback(callbacks::HitsoundReload);
			RegisterElement(&hitsound_name);
		}
		else {
			hitsound_name.setup(XOR("custom hitsounds"), XOR("custom_hitsound"), g_cl.m_menu);
			hitsound_name.AddShowCallback(callbacks::IsCustomHitsound);
			hitsound_name.SetCallback(callbacks::HitsoundReload);
			RegisterElement(&hitsound_name);
		}

		hitsound_volume.setup(XOR("hitsound volume"), XOR("hitsound_volume"), 1.f, 100.f, XOR("%.0f%%"));
		hitsound_volume.AddShowCallback(callbacks::IsAnyHitSoundOn);
		hitsound_volume.SetCallback(callbacks::HitsoundReload);
		RegisterElement(&hitsound_volume);

		// col2. 

		menu_color.setup(XOR("menu color"), XOR("menu_color"), colors::burgundy, &g_gui.m_color);
		RegisterElement(&menu_color, 1);

		config.setup(XOR("configuration"), XOR("config"), { XOR("auto"), XOR("scout"), XOR("awp"), XOR("pistol"), XOR("echo"), XOR("foxtrot"), XOR("gamma"), XOR("hotel"), XOR("india"), ("juliett") });
		config.RemoveFlags(ElementFlags::SAVE);
		RegisterElement(&config, 1);

		key1.setup(XOR("auto configuration key"), XOR("cfg_key1"));
		key1.RemoveFlags(ElementFlags::SAVE);
		key1.SetCallback(callbacks::SaveHotkeys);
		key1.AddShowCallback(callbacks::IsConfig1);
		key1.SetToggleCallback(callbacks::ConfigLoad1);
		RegisterElement(&key1, 1);

		key2.setup(XOR("scout configuration key"), XOR("cfg_key2"));
		key2.RemoveFlags(ElementFlags::SAVE);
		key2.SetCallback(callbacks::SaveHotkeys);
		key2.AddShowCallback(callbacks::IsConfig2);
		key2.SetToggleCallback(callbacks::ConfigLoad2);
		RegisterElement(&key2, 1);

		key3.setup(XOR("awp configuration key"), XOR("cfg_key3"));
		key3.RemoveFlags(ElementFlags::SAVE);
		key3.SetCallback(callbacks::SaveHotkeys);
		key3.AddShowCallback(callbacks::IsConfig3);
		key3.SetToggleCallback(callbacks::ConfigLoad3);
		RegisterElement(&key3, 1);

		key4.setup(XOR("pistol configuration key"), XOR("cfg_key4"));
		key4.RemoveFlags(ElementFlags::SAVE);
		key4.SetCallback(callbacks::SaveHotkeys);
		key4.AddShowCallback(callbacks::IsConfig4);
		key4.SetToggleCallback(callbacks::ConfigLoad4);
		RegisterElement(&key4, 1);

		key5.setup(XOR("echo configuration key"), XOR("cfg_key5"));
		key5.RemoveFlags(ElementFlags::SAVE);
		key5.SetCallback(callbacks::SaveHotkeys);
		key5.AddShowCallback(callbacks::IsConfig5);
		key5.SetToggleCallback(callbacks::ConfigLoad5);
		RegisterElement(&key5, 1);

		key6.setup(XOR("foxtrot configuration key"), XOR("cfg_key6"));
		key6.RemoveFlags(ElementFlags::SAVE);
		key6.SetCallback(callbacks::SaveHotkeys);
		key6.AddShowCallback(callbacks::IsConfig6);
		key6.SetToggleCallback(callbacks::ConfigLoad6);
		RegisterElement(&key6, 1);

		key7.setup(XOR("gamma configuration key"), XOR("cfg_key7"));
		key7.RemoveFlags(ElementFlags::SAVE);
		key7.SetCallback(callbacks::SaveHotkeys);
		key7.AddShowCallback(callbacks::IsConfig7);
		key7.SetToggleCallback(callbacks::ConfigLoad7);
		RegisterElement(&key7, 1);

		key8.setup(XOR("hotel configuration key"), XOR("cfg_key8"));
		key8.RemoveFlags(ElementFlags::SAVE);
		key8.SetCallback(callbacks::SaveHotkeys);
		key8.AddShowCallback(callbacks::IsConfig8);
		key8.SetToggleCallback(callbacks::ConfigLoad8);
		RegisterElement(&key8, 1);

		key9.setup(XOR("india configuration key"), XOR("cfg_key9"));
		key9.RemoveFlags(ElementFlags::SAVE);
		key9.SetCallback(callbacks::SaveHotkeys);
		key9.AddShowCallback(callbacks::IsConfig9);
		key9.SetToggleCallback(callbacks::ConfigLoad9);
		RegisterElement(&key9, 1);

		key10.setup(XOR("juliett configuration key"), XOR("cfg_key10"));
		key10.RemoveFlags(ElementFlags::SAVE);
		key10.SetCallback(callbacks::SaveHotkeys);
		key10.AddShowCallback(callbacks::IsConfig10);
		key10.SetToggleCallback(callbacks::ConfigLoad10);
		RegisterElement(&key10, 1);

		save.setup(XOR("save"));
		save.SetCallback(callbacks::ConfigSave);
		RegisterElement(&save, 1);

		load.setup(XOR("load"));
		load.SetCallback(callbacks::ConfigLoad);
		RegisterElement(&load, 1);

		serverlist.setup(XOR("choose server region"), XOR("serverlist"), { "eu", "na" }, true);
		RegisterElement(&serverlist, 1);

		connect1na.setup(XOR("dick's 2 [$16k / 15v15]"));
		connect1na.SetCallback(callbacks::dicks2);
		connect1na.AddShowCallback(callbacks::NaSvList);
		RegisterElement(&connect1na, 1);

		connectna.setup(XOR("dick's wingman [$800 / 2v2]"));
		connectna.SetCallback(callbacks::dickswm);
		connectna.AddShowCallback(callbacks::NaSvList);
		RegisterElement(&connectna, 1);

		connecteu.setup(XOR("mrx [$16k / 15v15]"));
		connecteu.SetCallback(callbacks::mrx);
		connecteu.AddShowCallback(callbacks::EuSvList);
		RegisterElement(&connecteu, 1);

		connect1eu.setup(XOR("mrx 1 [$16k / 15v15]"));
		connect1eu.SetCallback(callbacks::mrx2);
		connect1eu.AddShowCallback(callbacks::EuSvList);
		RegisterElement(&connect1eu, 1);

	}
};

class PlayerListTab : public Tab {
public:
	Edit player_to_insert;
	Button print_ids;
	Button clear_whitelistees;
public:
	void init() {
		SetTitle(XOR("whitelist"));

		player_to_insert.setup(XOR("insert player id to whitelist"), XOR("player_to_insert"), 3);
		player_to_insert.SetCallback(callbacks::addtowhitelist);
		RegisterElement(&player_to_insert);

		print_ids.setup(XOR("print player ids"));
		print_ids.SetCallback(callbacks::print_ids);
		RegisterElement(&print_ids, 1);

		clear_whitelistees.setup(XOR("clear whitelist"));
		clear_whitelistees.SetCallback(callbacks::clearwhitelist);
		RegisterElement(&clear_whitelistees, 1);
	}
};

class MainForm : public Form {
public:
	// aimbot.
	AimbotTab    aimbot;
	AntiAimTab   antiaim;

	// visuals.
	PlayersTab	 players;
	VisualsTab	 visuals;

	// misc.
	SkinsTab     skins;
	MovementTab	 movement;
	MiscTab	     misc;
	PlayerListTab plist;
public:
	void init() {
		SetPosition(50, 50);
		SetSize(630, 500);;

		// aim.
		RegisterTab(&aimbot);
		aimbot.init();

		RegisterTab(&antiaim);
		antiaim.init();

		// visuals.
		RegisterTab(&players);
		players.init();

		RegisterTab(&visuals);
		visuals.init();

		// misc.
		RegisterTab(&movement);
		movement.init();

		RegisterTab(&skins);
		skins.init();

		RegisterTab(&misc);
		misc.init();

		RegisterTab(&plist);
		plist.init();
	}
};

class Menu {
public:
	MainForm main;

public:
	void init() {
		Colorpicker::init();

		main.init();
		g_gui.RegisterForm(&main, VK_INSERT);
	}
};

extern Menu g_menu;