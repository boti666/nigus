#include "includes.h"
#include "engineclient.h"

Hooks                g_hooks{ };;
CustomEntityListener g_custom_entity_listener{ };;

// server.dll 55 8B EC 83 E4 F8 83 EC 1C 56 8B F1 8B 56 50
void __fastcall mhooks::ModifyEyePos(CCSGOPlayerAnimState* animstate, void* edx, vec3_t* pos) {

	static auto lookup_bone = pattern::find(g_csgo.m_client_dll, "E8 ? ? ? ? 89 44 24 5C").rel32(1).as<int(__thiscall*)(void*, const char*)>();

	if ((animstate->m_bLanding || animstate->m_flAnimDuckAmount != 0.f) && animstate->m_pPlayer->GetGroundEntity())
	{
		int bone_id = lookup_bone(animstate->m_pPlayer, "head_0");
		if (bone_id == -1)
			return;

		auto bone_pos = vec3_t(
			animstate->m_pPlayer->m_BoneCache().m_pCachedBones[8][0][3],
			animstate->m_pPlayer->m_BoneCache().m_pCachedBones[8][1][3],
			animstate->m_pPlayer->m_BoneCache().m_pCachedBones[8][2][3]);

		bone_pos.z += 1.7f;

		if (pos->z > bone_pos.z)
		{
			auto some_factor = 0.f;

			const auto delta = (*pos).z - bone_pos.z;
			const auto some_offset = (delta - 4.f) / 6.f;

			if (some_offset >= 0.f)
				some_factor = std::fminf(some_offset, 1.f);

			pos->z += (bone_pos.z - pos->z) * (some_factor * some_factor * 3.f - some_factor * some_factor * 2.f * some_factor);
		}
	}
}

bool __fastcall mhooks::SendWeaponAnim(Weapon* weapon, uint32_t edx, uint32_t act)
{
	const auto owner = reinterpret_cast<Player*>(g_csgo.m_entlist->GetClientEntityFromHandle(weapon->m_hOwnerEntity()));
	if (owner && owner->IsPlayer() && owner->alive() && owner->index() == g_csgo.m_engine->GetLocalPlayer())
		g_cl.vm = act;

	return oSendWeaponAnim(weapon, edx, act);
}

void mhooks::CL_FireEvents()
{
	CEventInfo* ei = g_csgo.m_cl->m_events;
	CEventInfo* next = nullptr;

	if (!ei) {
		return oCL_FireEvents();
	}

	do {
		next = *reinterpret_cast<CEventInfo**>(reinterpret_cast<uintptr_t>(ei) + 0x38);

		uint16_t classID = ei->m_class_id - 1;

		auto m_pCreateEventFn = ei->m_client_class->m_pCreateEvent;
		if (!m_pCreateEventFn) {
			continue;
		}

		void* pCE = m_pCreateEventFn();
		if (!pCE) {
			continue;
		}

		if (classID == 170) {
			ei->m_fire_delay = 0.0f;
		}
		ei = next;
	} while (next != nullptr);

	oCL_FireEvents();
}

using AttachmentHelper_t = void(__thiscall*)(Player*, CStudioHdr*);
void __cdecl mhooks::InterpolateServerEntities()
{
	static auto AttachmentHelperFn = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 48 53 8B 5D 08 89 4D F4")).as< AttachmentHelper_t>();
	Player* pLocal = g_cl.m_local;
	if (!pLocal || !g_csgo.m_engine->IsInGame())
		return oInterpolateServerEntities();

	oInterpolateServerEntities();

	auto pStudioHdr = pLocal->m_studioHdr();

	// fix server model origin
	{
		pLocal->SetAbsAngles(ang_t(0.0f, g_cl.m_abs_yaw, 0.0f));

		matrix3x4_t matWorldMatrix{ };
		math::AngleMatrix(ang_t(0.f, g_cl.m_abs_yaw, 0.f), pLocal->GetAbsOrigin(), matWorldMatrix);

		if (pStudioHdr) {
			uint8_t uBoneComputed[0x20] = { 0 };
			pLocal->BuildTransformations(pStudioHdr, g_ServerAnimations.m_uServerAnimations.m_vecBonePos, g_ServerAnimations.m_uServerAnimations.m_quatBoneRot,
				matWorldMatrix, BONE_USED_BY_ANYTHING, uBoneComputed);
		}

		pLocal->InvalidateBoneCache();

		auto pBackupBones = pLocal->m_BoneAccessor().m_pBones;
		pLocal->m_BoneAccessor().m_pBones = pLocal->m_BoneCache().m_pCachedBones;

		if (pStudioHdr)
			AttachmentHelperFn(pLocal, pStudioHdr);

		pLocal->m_BoneAccessor().m_pBones = pBackupBones;
	}
}

void __fastcall mhooks::UpdateActivityModifiers(void* rcx)
{
	printf("%p\n", rcx);

	return oUpdateActivityModifiers(rcx);
}

int __fastcall mhooks::RunSimulation(void* ecx, void* edx, int current_command, CUserCmd* cmd, Player* localplayer)
{
	if (!g_cl.m_local || !g_cl.m_local->alive())
		g_cl.last_cmd_num = 0;

	if (!localplayer || localplayer != g_cl.m_local)
		return oRunSimulation(ecx, edx, current_command, cmd, localplayer);

	oRunSimulation(ecx, edx, current_command, cmd, localplayer);

	/* local anims */
	if (cmd->m_command_number > g_cl.last_cmd_num) {
		g_cl.HandleAnimations();
		g_cl.HandleBodyYaw();

		// update cmd number
		g_cl.last_cmd_num = cmd->m_command_number;
	}
}

bool __fastcall mhooks::interpolate(void* ecx, void* edx, float time)
{
	auto base_entity = (Entity*)ecx;

	auto owner = (Player*)g_csgo.m_entlist->GetClientEntityFromHandle(base_entity->m_hViewModel());
	if (!owner || owner->index() != g_cl.m_local->index())
		return oInterpolate(ecx, edx, time);

	float interpolation_amount = g_csgo.m_globals->m_interp_amt;

	g_csgo.m_globals->m_interp_amt = 0.f;

	bool ret = oInterpolate(ecx, edx, time);

	g_csgo.m_globals->m_interp_amt = interpolation_amount;

	return ret;
}

void __fastcall mhooks::CalcView(void* ecx, const std::uintptr_t edx, vec3_t& eye_origin, const ang_t& eye_ang, float& z_near, float& z_far, float& fov)
{
	if (!ecx
		|| ecx != g_cl.m_local)
		return oCalcView(ecx, edx, eye_origin, eye_ang, z_near, z_far, fov);

	Player* player = (Player*)ecx;
	const bool backup = player->m_bUseNewAnimState();
	player->m_bUseNewAnimState() = false;
	oCalcView(ecx, edx, eye_origin, eye_ang, z_near, z_far, fov);
	player->m_bUseNewAnimState() = backup;
}


bool __fastcall mhooks::Teleported(void* ecx, void* edx) {
	if (g_bones.m_running)
		return true; // force uninterpolated bones and ik targets to be cleared.

	return oTeleported(ecx, edx);
}

int last_command_num = 0;
void __fastcall mhooks::PhysicsSimulate(Player* player, void* edx) {

	if (!player || !player->alive() || ((*(int*)(std::uintptr_t(player) + 0x2A8) == g_csgo.m_globals->m_tick_count) || !(*(bool*)(std::uintptr_t(player) + 0x34D0))) || !g_cl.m_cmd)
		oPhysicsSimulate(player, edx);

	const auto backup_velocity_modifier = g_cl.m_local->m_flVelocityModifier();

	if (player == g_cl.m_local) {
		if (g_cl.m_cmd->m_command_number > last_command_num) {
			auto nci = g_csgo.m_engine->GetNetChannelInfo();

			// don't do this if our choke cycle resets.
			if (nci) {
				const auto latency = game::TIME_TO_TICKS(g_cl.m_latency);

				// recalculate velocity modifier.
				if (g_inputpred.stored.m_velocity_modifier < 1.0f)
					g_inputpred.stored.m_velocity_modifier = std::clamp < float >(g_inputpred.stored.m_velocity_modifier + (game::TICKS_TO_TIME(1) + latency) * (1.0f / 2.5f), 0.0f, 1.0f);

				if (g_cl.m_lag == 0)
					player->m_flVelocityModifier() = g_inputpred.stored.m_old_velocity_modifier; // set to value received from server.
			}

			last_command_num = g_cl.m_cmd->m_command_number;
		}

		*(int*)(std::uintptr_t(player) + 0x3238) = 0;
	}

	oPhysicsSimulate(player, edx);

	if (player == g_cl.m_local) {
	player->m_flVelocityModifier() = backup_velocity_modifier;
	}
}

void __vectorcall mhooks::update_animation_state(void* this_pointer, void* unknown, float z, float y, float x, void* unknown1)
{
	if (!this_pointer || !g_cl.m_local) return original_update_animation_state(this_pointer, unknown, z, y, x, unknown1);

	const auto animation_state = reinterpret_cast<CCSGOPlayerAnimState*>(this_pointer);

	return original_update_animation_state(this_pointer, unknown, z, y, x, unknown1);
}

void __fastcall mhooks::UpdateClientSideAnim(Player* const player, const std::uintptr_t edx) {
	if (g_hooks.m_bUpdatingCSA[player->index()] || !player->enemy(g_cl.m_local) && player != g_cl.m_local) {
		return oUpdateClientSideAnim(player, edx);
	}

	if (player == g_cl.m_local)
	{
		player->GetPoseParameters(g_cl.m_backup_poses);
		player->GetAnimLayers(g_cl.m_backup_layers);

		player->SetPoseParameters(g_cl.m_poses);
		player->SetAnimLayers(g_cl.m_layers);

		// setup new bones every tick
		//g_bones.PerformBoneSetupLocal(g_cl.m_local, g_cl.m_matrix, 0x7FF00, ang_t{ 0.f, g_cl.m_abs_yaw, 0.f }, g_cl.m_local->GetAbsOrigin(), g_cl.m_local->m_flSimulationTime(), nullptr);

		// update bones
		g_bones.SetupBones(player, g_cl.m_matrix, 128, 0x7FF00, player->m_flSimulationTime());

		player->SetPoseParameters(g_cl.m_backup_poses);
		player->SetAnimLayers(g_cl.m_backup_layers);
	}
}

class WeaponVisualData_t {
public:
	char padding1[0x77C];
	char weapon_path[260];
	char padding2[0x10C];

	vec3_t color1;
	vec3_t color2;
	vec3_t color3;
	vec3_t color4;

	int nPhongAlbedoBoost;
	int nPhongExponent;
	int nPhongIntensity;

	float flPhongAlbedoFactor;

	float flWearProgress;

	float flPatternScale;
	float flPatternOffsetX;
	float flPatternOffsetY;
	float flPatternRot;

	float flWearScale;
	float flWearOffsetX;
	float flWearOffsetY;
	float flWearRot;

	float flGrungeScale;
	float flGrungeOffsetX;
	float flGrungeOffsetY;
	float flGrungeRot;
};

void ColorChange(int pointer, Color color_one, Color color_two, Color color_three, Color color_four) {
	*(float*)(pointer + 0x98C) = color_one.b();
	*(float*)(pointer + 0x990) = color_one.r();
	*(float*)(pointer + 0x994) = color_one.g();

	/* color 2 */
	*(float*)(pointer + 0x998) = color_two.b();
	*(float*)(pointer + 0x99C) = color_two.r();
	*(float*)(pointer + 0x9A0) = color_two.g();

	/* color 3 */
	*(float*)(pointer + 0x9A4) = color_three.b();
	*(float*)(pointer + 0x9A8) = color_three.r();
	*(float*)(pointer + 0x9AC) = color_three.g();

	/* color 4 */
	*(float*)(pointer + 0x9B0) = color_four.b();
	*(float*)(pointer + 0x9B4) = color_four.r();
	*(float*)(pointer + 0x9B8) = color_four.g();
};

void __fastcall mhooks::HkSetVisualsData(void* ecx, void* edx, const char* composting_shader_name) {

	Player* local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	if (!local)
		return oSetVisualsData(ecx, edx, composting_shader_name);

	Weapon* weapon = local->GetActiveWeapon();
	if (!weapon)
		return oSetVisualsData(ecx, edx, composting_shader_name);

	if (!g_menu.main.skins.modulation.get())
		return oSetVisualsData(ecx, edx, composting_shader_name);

	oSetVisualsData(ecx, edx, composting_shader_name);

	int pointer = (uintptr_t(ecx) - 0x4);

	if (!pointer)
		return oSetVisualsData(ecx, edx, composting_shader_name);

	switch (g_cl.m_weapon_id) {
	case DEAGLE:
		ColorChange(pointer, g_menu.main.skins.color_one_deagle.get(), g_menu.main.skins.color_two_deagle.get(), g_menu.main.skins.color_three_deagle.get(), g_menu.main.skins.color_four_deagle.get());
		break;
	case ELITE:
		ColorChange(pointer, g_menu.main.skins.color_one_elite.get(), g_menu.main.skins.color_two_elite.get(), g_menu.main.skins.color_three_elite.get(), g_menu.main.skins.color_four_elite.get());
		break;
	case FIVESEVEN:
		ColorChange(pointer, g_menu.main.skins.color_one_fiveseven.get(), g_menu.main.skins.color_two_fiveseven.get(), g_menu.main.skins.color_three_fiveseven.get(), g_menu.main.skins.color_four_fiveseven.get());
		break;
	case GLOCK:
		ColorChange(pointer, g_menu.main.skins.color_one_glock.get(), g_menu.main.skins.color_two_glock.get(), g_menu.main.skins.color_three_glock.get(), g_menu.main.skins.color_four_glock.get());
		break;
	case AK47:
		ColorChange(pointer, g_menu.main.skins.color_one_ak47.get(), g_menu.main.skins.color_two_ak47.get(), g_menu.main.skins.color_three_ak47.get(), g_menu.main.skins.color_four_ak47.get());
		break;
	case AUG:
		ColorChange(pointer, g_menu.main.skins.color_one_aug.get(), g_menu.main.skins.color_two_aug.get(), g_menu.main.skins.color_three_aug.get(), g_menu.main.skins.color_four_aug.get());
		break;
	case AWP:
		ColorChange(pointer, g_menu.main.skins.color_one_awp.get(), g_menu.main.skins.color_two_awp.get(), g_menu.main.skins.color_three_awp.get(), g_menu.main.skins.color_four_awp.get());
		break;
	case FAMAS:
		ColorChange(pointer, g_menu.main.skins.color_one_famas.get(), g_menu.main.skins.color_two_famas.get(), g_menu.main.skins.color_three_famas.get(), g_menu.main.skins.color_four_famas.get());
		break;
	case G3SG1:
		ColorChange(pointer, g_menu.main.skins.color_one_g3sg1.get(), g_menu.main.skins.color_two_g3sg1.get(), g_menu.main.skins.color_three_g3sg1.get(), g_menu.main.skins.color_four_g3sg1.get());
		break;
	case GALIL:
		ColorChange(pointer, g_menu.main.skins.color_one_galil.get(), g_menu.main.skins.color_two_galil.get(), g_menu.main.skins.color_three_galil.get(), g_menu.main.skins.color_four_galil.get());
		break;
	case M249:
		ColorChange(pointer, g_menu.main.skins.color_one_m249.get(), g_menu.main.skins.color_two_m249.get(), g_menu.main.skins.color_three_m249.get(), g_menu.main.skins.color_four_m249.get());
		break;
	case M4A4:
		ColorChange(pointer, g_menu.main.skins.color_one_m4a4.get(), g_menu.main.skins.color_two_m4a4.get(), g_menu.main.skins.color_three_m4a4.get(), g_menu.main.skins.color_four_m4a4.get());
		break;
	case MAC10:
		ColorChange(pointer, g_menu.main.skins.color_one_mac10.get(), g_menu.main.skins.color_two_mac10.get(), g_menu.main.skins.color_three_mac10.get(), g_menu.main.skins.color_four_mac10.get());
		break;
	case P90:
		ColorChange(pointer, g_menu.main.skins.color_one_p90.get(), g_menu.main.skins.color_two_p90.get(), g_menu.main.skins.color_three_p90.get(), g_menu.main.skins.color_four_p90.get());
		break;
	case UMP45:
		ColorChange(pointer, g_menu.main.skins.color_one_ump45.get(), g_menu.main.skins.color_two_ump45.get(), g_menu.main.skins.color_three_ump45.get(), g_menu.main.skins.color_four_ump45.get());
		break;
	case XM1014:
		ColorChange(pointer, g_menu.main.skins.color_one_xm1014.get(), g_menu.main.skins.color_two_xm1014.get(), g_menu.main.skins.color_three_xm1014.get(), g_menu.main.skins.color_four_xm1014.get());
		break;
	case BIZON:
		ColorChange(pointer, g_menu.main.skins.color_one_bizon.get(), g_menu.main.skins.color_two_bizon.get(), g_menu.main.skins.color_three_bizon.get(), g_menu.main.skins.color_four_bizon.get());
		break;
	case MAG7:
		ColorChange(pointer, g_menu.main.skins.color_one_mag7.get(), g_menu.main.skins.color_two_mag7.get(), g_menu.main.skins.color_three_mag7.get(), g_menu.main.skins.color_four_mag7.get());
		break;
	case NEGEV:
		ColorChange(pointer, g_menu.main.skins.color_one_negev.get(), g_menu.main.skins.color_two_negev.get(), g_menu.main.skins.color_three_negev.get(), g_menu.main.skins.color_four_negev.get());
		break;
	case SAWEDOFF:
		ColorChange(pointer, g_menu.main.skins.color_one_sawedoff.get(), g_menu.main.skins.color_two_sawedoff.get(), g_menu.main.skins.color_three_sawedoff.get(), g_menu.main.skins.color_four_sawedoff.get());
		break;
	case TEC9:
		ColorChange(pointer, g_menu.main.skins.color_one_tec9.get(), g_menu.main.skins.color_two_tec9.get(), g_menu.main.skins.color_three_tec9.get(), g_menu.main.skins.color_four_tec9.get());
		break;
	case P2000:
		ColorChange(pointer, g_menu.main.skins.color_one_p2000.get(), g_menu.main.skins.color_two_p2000.get(), g_menu.main.skins.color_three_p2000.get(), g_menu.main.skins.color_four_p2000.get());
		break;
	case MP7:
		ColorChange(pointer, g_menu.main.skins.color_one_mp7.get(), g_menu.main.skins.color_two_mp7.get(), g_menu.main.skins.color_three_mp7.get(), g_menu.main.skins.color_four_mp7.get());
		break;
	case MP9:
		ColorChange(pointer, g_menu.main.skins.color_one_mp9.get(), g_menu.main.skins.color_two_mp9.get(), g_menu.main.skins.color_three_mp9.get(), g_menu.main.skins.color_four_mp9.get());
		break;
	case NOVA:
		ColorChange(pointer, g_menu.main.skins.color_one_nova.get(), g_menu.main.skins.color_two_nova.get(), g_menu.main.skins.color_three_nova.get(), g_menu.main.skins.color_four_nova.get());
		break;
	case P250:
		ColorChange(pointer, g_menu.main.skins.color_one_p250.get(), g_menu.main.skins.color_two_p250.get(), g_menu.main.skins.color_three_p250.get(), g_menu.main.skins.color_four_p250.get());
		break;
	case SCAR20:
		ColorChange(pointer, g_menu.main.skins.color_one_scar20.get(), g_menu.main.skins.color_two_scar20.get(), g_menu.main.skins.color_three_scar20.get(), g_menu.main.skins.color_four_scar20.get());
		break;
	case SG553:
		ColorChange(pointer, g_menu.main.skins.color_one_sg553.get(), g_menu.main.skins.color_two_sg553.get(), g_menu.main.skins.color_three_sg553.get(), g_menu.main.skins.color_four_sg553.get());
		break;
	case SSG08:
		ColorChange(pointer, g_menu.main.skins.color_one_ssg08.get(), g_menu.main.skins.color_two_ssg08.get(), g_menu.main.skins.color_three_ssg08.get(), g_menu.main.skins.color_four_ssg08.get());
		break;
	case M4A1S:
		ColorChange(pointer, g_menu.main.skins.color_one_m4a1s.get(), g_menu.main.skins.color_two_m4a1s.get(), g_menu.main.skins.color_three_m4a1s.get(), g_menu.main.skins.color_four_m4a1s.get());
		break;
	case USPS:
		ColorChange(pointer, g_menu.main.skins.color_one_usps.get(), g_menu.main.skins.color_two_usps.get(), g_menu.main.skins.color_three_usps.get(), g_menu.main.skins.color_four_usps.get());
		break;
	case CZ75A:
		ColorChange(pointer, g_menu.main.skins.color_one_cz75a.get(), g_menu.main.skins.color_two_cz75a.get(), g_menu.main.skins.color_three_cz75a.get(), g_menu.main.skins.color_four_cz75a.get());
		break;
	case REVOLVER:
		ColorChange(pointer, g_menu.main.skins.color_one_revolver.get(), g_menu.main.skins.color_two_revolver.get(), g_menu.main.skins.color_three_revolver.get(), g_menu.main.skins.color_four_revolver.get());
		break;
	case KNIFE_BAYONET:
		ColorChange(pointer, g_menu.main.skins.color_one_bayonet.get(), g_menu.main.skins.color_two_bayonet.get(), g_menu.main.skins.color_three_bayonet.get(), g_menu.main.skins.color_four_bayonet.get());
		break;
	case KNIFE_FLIP:
		ColorChange(pointer, g_menu.main.skins.color_one_flip.get(), g_menu.main.skins.color_two_flip.get(), g_menu.main.skins.color_three_flip.get(), g_menu.main.skins.color_four_flip.get());
		break;
	case KNIFE_GUT:
		ColorChange(pointer, g_menu.main.skins.color_one_gut.get(), g_menu.main.skins.color_two_gut.get(), g_menu.main.skins.color_three_gut.get(), g_menu.main.skins.color_four_gut.get());
		break;
	case KNIFE_KARAMBIT:
		ColorChange(pointer, g_menu.main.skins.color_one_karambit.get(), g_menu.main.skins.color_two_karambit.get(), g_menu.main.skins.color_three_karambit.get(), g_menu.main.skins.color_four_karambit.get());
		break;
	case KNIFE_M9_BAYONET:
		ColorChange(pointer, g_menu.main.skins.color_one_m9.get(), g_menu.main.skins.color_two_m9.get(), g_menu.main.skins.color_three_m9.get(), g_menu.main.skins.color_four_m9.get());
		break;
	case KNIFE_HUNTSMAN:
		ColorChange(pointer, g_menu.main.skins.color_one_huntsman.get(), g_menu.main.skins.color_two_huntsman.get(), g_menu.main.skins.color_three_huntsman.get(), g_menu.main.skins.color_four_huntsman.get());
		break;
	case KNIFE_FALCHION:
		ColorChange(pointer, g_menu.main.skins.color_one_falchion.get(), g_menu.main.skins.color_two_falchion.get(), g_menu.main.skins.color_three_falchion.get(), g_menu.main.skins.color_four_falchion.get());
		break;
	case KNIFE_BOWIE:
		ColorChange(pointer, g_menu.main.skins.color_one_bowie.get(), g_menu.main.skins.color_two_bowie.get(), g_menu.main.skins.color_three_bowie.get(), g_menu.main.skins.color_four_bowie.get());
		break;
	case KNIFE_BUTTERFLY:
		ColorChange(pointer, g_menu.main.skins.color_one_butterfly.get(), g_menu.main.skins.color_two_butterfly.get(), g_menu.main.skins.color_three_butterfly.get(), g_menu.main.skins.color_four_butterfly.get());
		break;
	case KNIFE_SHADOW_DAGGERS:
		ColorChange(pointer, g_menu.main.skins.color_one_daggers.get(), g_menu.main.skins.color_two_daggers.get(), g_menu.main.skins.color_three_daggers.get(), g_menu.main.skins.color_four_daggers.get());
		break;
	default:
		break;
	}
}

void __fastcall mhooks::BuildTransformations(Player* ecx, void* edx, CStudioHdr* hdr, int a3, int a4, int a5, int a6, int a7) {

	// get bone jiggle.
	int bone_jiggle = *reinterpret_cast<int*>(uintptr_t(ecx) + 0x291C);

	// null bone jiggle to prevent attachments from jiggling around.
	*reinterpret_cast<int*>(uintptr_t(ecx) + 0x291C) = 0;
	
	oBuildTransformations(ecx, edx, hdr, a3, a4, a5, a6, a7);

	// restore bone jiggle.
	*reinterpret_cast<int*>(uintptr_t(ecx) + 0x291C) = bone_jiggle;
}

bool __fastcall mhooks::ShouldSkipAnimationFrame(void* this_pointer, void* edx) {
	return false;
}

void __fastcall mhooks::CheckForSequenceChange(void* this_pointer, void* edx, void* hdr, int cur_sequence, bool force_new_sequence, bool interpolate) {

	return oCheckForSeqChange(this_pointer, edx, hdr, cur_sequence, force_new_sequence, false);
}

void __fastcall mhooks::StandardBlendingRules(Player* const pPlayer, const std::uintptr_t edx, CStudioHdr* const mdl_data, int a1, int a2, float a3, int mask) {
	if (!pPlayer || !g_cl.m_local || !pPlayer->IsPlayer() || pPlayer->index() == g_cl.m_local->index()) {
		return oStandardBlendingRules(pPlayer, edx, mdl_data, a1, a2, a3, mask);
	}

	// fix arms.
	if (pPlayer->enemy(g_cl.m_local) || pPlayer->m_bIsLocalPlayer())
		mask = BONE_USED_BY_SERVER;

	if (pPlayer->m_bIsLocalPlayer())
		mask |= BONE_USED_BY_BONE_MERGE;

	pPlayer->m_fEffects() |= EF_NOINTERP;

	oStandardBlendingRules(pPlayer, edx, mdl_data, a1, a2, a3, mask);

	pPlayer->m_fEffects() &= ~EF_NOINTERP;
}

void __cdecl mhooks::ProcessInterpolatedList()
{
	static auto s_bAllowExtrapolation = pattern::find(g_csgo.m_client_dll, XOR("A2 ? ? ? ? 8B 45 E8")) + 1;;

	**(bool**)(s_bAllowExtrapolation) = false;
	oProcessInterpolatedList();
}

void __fastcall mhooks::MaintainSeqTrans(void* ecx, void* edx, void* bone_setup, float cycle, vec3_t pos[], quaternion_t q[]) {
	if (!g_bones.m_running)
		return mhooks::oMaintainSeqTrans(ecx, edx, bone_setup, cycle, pos, q);
}

void __fastcall mhooks::DoExtraBoneProcessing(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7)
{
	// server does not call this function.
	return;
}

struct incoming_seq_t {
	std::ptrdiff_t m_in_seq{ };
	std::ptrdiff_t m_reliable_state{ };
};

std::vector < incoming_seq_t > incoming_seq{ };

int __fastcall mhooks::BaseInterpolatePart1(void* ecx, void* edx, float& curtime, vec3_t& old_origin, ang_t& old_angs, int& no_more_changes) {
	const auto player = reinterpret_cast<Player*>(ecx);

	if (!player)
		return mhooks::oBaseInterpolate(ecx, edx, curtime, old_origin, old_angs, no_more_changes);

	static auto CBaseEntity__MoveToLastReceivedPos = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 80 7D 08 00 56 8B F1 75 0D 80 BE ? ? ? ? ? 0F")).as < void(__thiscall*) (void*, bool) >();

	if (player->IsPlayer() && player != g_cl.m_local) {
		no_more_changes = 1;
		CBaseEntity__MoveToLastReceivedPos(player, false);
		return 0;
	}

	return mhooks::oBaseInterpolate(ecx, edx, curtime, old_origin, old_angs, no_more_changes);
}

bool __fastcall mhooks::SetupBones(void* ecx, void* edx, matrix3x4_t* matrix, int bone_count, int mask, float time)
{
	Player* player = (Player*)(uintptr_t(ecx) - 4);

	if (player->index() > 64 || player->index() < 0)
		return oSetupBones(ecx, edx, matrix, bone_count, mask, time);

	if (!g_hooks.m_updating_bones) {
		if (!matrix || bone_count <= 0)
			return true;

		if (matrix) {
			memcpy(matrix, player->m_BoneCache().m_pCachedBones, sizeof(matrix3x4a_t) * std::min(bone_count, player->m_BoneCache().m_CachedBoneCount));
			return true;
		}

		return false;
	}

	for (int i = 0; i < player->m_AnimOverlayCount(); ++i) {
		auto& elem = player->m_AnimOverlay()[i];

		if (player != elem.owner)
			elem.owner = player;
	}

	auto prev_p = (player->m_PlayerAnimState() == nullptr) ? 0 : player->m_PlayerAnimState()->m_pWeaponLastBoneSetup;

	if (player->m_PlayerAnimState() != nullptr)
		player->m_PlayerAnimState()->m_pWeaponLastBoneSetup = player->m_PlayerAnimState()->m_pWeapon;

	auto og = oSetupBones(ecx, edx, matrix, bone_count, mask, time);

	if (player->m_PlayerAnimState() != nullptr)
		player->m_PlayerAnimState()->m_pWeaponLastBoneSetup = prev_p;

	return og;
}

// https://www.unknowncheats.me/forum/counterstrike-global-offensive/332952-fixing-accumulatelayers.html
void __fastcall mhooks::AccumulateLayers(Player* const player, const uint32_t edx, void** const setup, vec3_t* const pos, const float time, quaternion_t* const q) {

	//static auto iks_off = pattern::find(g_csgo.m_client_dll, ("8D 47 FC 8B 8F")).add(5).deref().add(4).get< uint32_t >();
	static auto accumulate_pose = pattern::find(g_csgo.m_server_dll, ("55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? A1 ? ? ? ? 56 57 8B F9")).as<void(__thiscall*)(void*, vec3_t*, quaternion_t*, int, float, float, float, void*)>();

	if (!player || !player->IsPlayer() || player->m_iHealth() <= 0 || !player->m_AnimOverlay())
		return oAccumulateLayers(player, edx, setup, pos, time, q);

	std::array<size_t, 13> layers;
	layers.fill(layers.size());

	for (auto i = 0u; i < 14; i++)
	{
		const auto& layer = player->m_AnimOverlay()[i];

		if (layer.weight > 0 && layer.order >= 0 && layer.order < layers.size())
			layers[layer.order] = i;
	}

	for (auto i = 0u; i < layers.size(); i++)
	{
		const auto layer_num = layers[i];

		if (layer_num < 0 || layer_num >= 14)
			continue;

		const auto& layer = player->m_AnimOverlay()[layer_num];

		if (layer.weight <= 1)
			continue;

		accumulate_pose(*reinterpret_cast<void**>(edx), pos, q, layer.sequence, layer.cycle, layer.weight, g_csgo.m_globals->m_curtime, reinterpret_cast<void*>(player + 0x3C0));
	}
}

constexpr uint32_t HashRunTimeForMain(const char* str) noexcept
{
	auto value = 0x811c9dc5;

	while (*str)
	{
		value ^= *str++;
		value *= 0x1000193;
	}

	return value;
}

void __fastcall mhooks::InitNewParticlesScalar(C_INIT_RandomColor* ecx, void* edx, CParticleCollection* particles, int start_position, int particles_count, int attribute_write_mask, void* ctx)
{
	if (!g_menu.main.visuals.modulate_molo.get())
		return oInitNewParticlesScalar(ecx, edx, particles, start_position, particles_count, attribute_write_mask, ctx);

	vec3_t o_min = ecx->m_normal_color_min;
	vec3_t o_max = ecx->m_normal_color_max;

	const char* mat_name = *(char**)(*(uintptr_t*)((uintptr_t)particles + 0x48) + 0x40);
	assert(mat_name);

	auto nigga = g_menu.main.visuals.molocol.get();
	//auto niggab = g_menu.main.visuals.smoke_color.get();

	switch (HashRunTimeForMain(mat_name))
	{
	case HASH("particle\\fire_burning_character\\fire_env_fire.vmt"):
	case HASH("particle\\fire_burning_character\\fire_env_fire_depthblend.vmt"):
	case HASH("particle\\fire_burning_character\\fire_burning_character_depthblend.vmt"):
	case HASH("particle\\fire_burning_character\\fire_burning_character.vmt"):
	case HASH("particle\\fire_burning_character\\fire_burning_character_nodepth.vmt"):
	case HASH("particle\\particle_flares\\particle_flare_001.vmt"):
	case HASH("particle\\particle_flares\\particle_flare_004.vmt"):
	case HASH("particle\\particle_flares\\particle_flare_004b_mod_ob.vmt"):
	case HASH("particle\\particle_flares\\particle_flare_004b_mod_z.vmt"):
	case HASH("particle\\fire_explosion_1\\fire_explosion_1_bright.vmt"):
	case HASH("particle\\fire_explosion_1\\fire_explosion_1b.vmt"):
	case HASH("particle\\fire_particle_4\\fire_particle_4.vmt"):
	case HASH("particle\\fire_explosion_1\\fire_explosion_1_oriented.vmt"):
		ecx->m_normal_color_min = ecx->m_normal_color_max = vec3_t(nigga.r() / 255.f, nigga.g() / 255.f, nigga.b() / 255.f);
		break;
	}

	oInitNewParticlesScalar(ecx, edx, particles, start_position, particles_count, attribute_write_mask, ctx);

	ecx->m_normal_color_min = o_min;
	ecx->m_normal_color_max = o_max;
}


void DrawBeamPazw(vec3_t src, vec3_t end, Color color)
{
	BeamInfo_t beamInfo;
	beamInfo.m_nType = 0;
	beamInfo.m_nModelIndex = -1;
	beamInfo.m_flHaloScale = 0.f;
	beamInfo.m_flLife = 0.02f;
	beamInfo.m_flFadeLength = 10.f;
	beamInfo.m_flWidth = 2.f;
	beamInfo.m_flEndWidth = 2.f;
	beamInfo.m_pszModelName = "sprites/purplelaser1.vmt";
	beamInfo.m_flAmplitude = 0.f;
	beamInfo.m_flSpeed = 0.01f;
	beamInfo.m_nStartFrame = 0;
	beamInfo.m_flFrameRate = 0.f;
	beamInfo.m_flRed = color.r();
	beamInfo.m_flGreen = color.g();
	beamInfo.m_flBlue = color.b();
	beamInfo.m_flBrightness = color.a();
	beamInfo.m_nSegments = 2;
	beamInfo.m_bRenderable = true;
	beamInfo.m_nFlags = 0;
	beamInfo.m_vecStart = src;
	beamInfo.m_vecEnd = end;


	Beam_t* myBeam = g_csgo.m_beams->CreateBeamPoints(beamInfo);
	if (myBeam)
		g_csgo.m_beams->DrawBeam(myBeam);
}

void __fastcall mhooks::CreateGrenadeTrail(void* ecx)
{
	const auto this_pointer = static_cast<CBaseCSGrenadeProjectile*>(ecx);
	if (!g_cl.m_local || !this_pointer || !g_menu.main.visuals.grenade_path.get()) return;

	if (this_pointer->m_next_trail_line_time > g_csgo.m_globals->m_curtime) return;

	vec3_t temp_orientation = (this_pointer->m_last_trail_line_pos - this_pointer->m_local_origin);

	DrawBeamPazw(this_pointer->m_local_origin, temp_orientation, g_menu.main.visuals.grenade_path_col.get());

	this_pointer->m_last_trail_line_pos = this_pointer->m_local_origin;
	this_pointer->m_next_trail_line_time = g_csgo.m_globals->m_curtime + 0.05f;
}

using FnGetExposureRange = void(__fastcall*)(float*, float*);
FnGetExposureRange oGetExposureRange;
void __fastcall hkGetExposureRange(float* pflAutoExposureMin, float* pflAutoExposureMax) {
	*pflAutoExposureMin = 1.f;
	*pflAutoExposureMax = 1.f;

	oGetExposureRange(pflAutoExposureMin, pflAutoExposureMax);
}

using ProcessMovementFn = void(__fastcall*)(void*, void*, Entity*, CMoveData*);
ProcessMovementFn oProcessMovement;
void __fastcall hkProcessMovement(void* ecx, void* edx, Entity* player, CMoveData* data) {
	data->m_bGameCodeMovedPlayer = false;
	oProcessMovement(ecx, edx, player, data);
}

using ResetLatchFn = bool(__fastcall*)(void*, void*);
ResetLatchFn oResetLatch;
bool __fastcall hkResetLatch(void* ecx, void* edx) {
	static const auto compute_first_command_to_execute = pattern::find(g_csgo.m_client_dll, XOR("85 C0 75 28 8B 0D ? ? ? ? 81")).as<void*>();

	if (_ReturnAddress() == compute_first_command_to_execute)
		return false;

	return oResetLatch(ecx, edx);
}

void Body_proxy(CRecvProxyData* data, Address ptr, Address out) {
	Stack stack;

	static Address RecvTable_Decode{ pattern::find(g_csgo.m_engine_dll, XOR("EB 0D FF 77 10")) };

	// call from entity going into pvs.
	if (stack.next().next().ReturnAddress() != RecvTable_Decode) {
		// convert to player.
		Player* player = ptr.as< Player* >();

		// store data about the update.
		g_resolver.OnBodyUpdate(player, data->m_Value.m_Float);
	}

	// call original proxy.
	if (g_hooks.m_Body_original)
		g_hooks.m_Body_original(data, ptr, out);
}

void SimulationTime_proxy(CRecvProxyData* data, Address ptr, Address out)
{
	if (data->m_Value.m_Int == 0)
		return;

	g_hooks.m_SimTime_original(data, ptr, out);
}

using FnPreEntityPacketReceived = void(__thiscall*)(void*, int, int, int);
FnPreEntityPacketReceived oPreEntityPacketReceived;

void __stdcall hkPreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed) {
	if (!g_csgo.m_entlist || !g_csgo.m_prediction || g_csgo.m_cl->m_delta_tick == -1)
		return oPreEntityPacketReceived(g_csgo.m_prediction, commands_acknowledged, current_world_update_packet, server_ticks_elapsed);

	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	if (g_cl.m_local && g_cl.m_local->alive() && commands_acknowledged > 0) {
		g_predmanager.PreUpdate(g_cl.m_local);
	}

	oPreEntityPacketReceived(g_csgo.m_prediction, commands_acknowledged, current_world_update_packet, server_ticks_elapsed);
}

using FnPostNetworkDataReceived = void(__thiscall*)(void*, int);
FnPostNetworkDataReceived oPostNetworkDataReceived;

void __stdcall hkPostNetworkDataReceived(int commands_acknowledged) {
	if (!g_csgo.m_entlist || !g_csgo.m_prediction || !g_csgo.m_engine || g_csgo.m_cl->m_delta_tick == -1)
		return oPostNetworkDataReceived(g_csgo.m_prediction, commands_acknowledged);

	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	if (g_cl.m_local) {
		const auto map = g_cl.m_local->GetPredDescMap();
		if (map) {
			if (!g_cl.m_map_setup) {
				const typedescription_t type_description{ FIELD_FLOAT,
														  "m_flVelocityModifier",
														  static_cast<int>(g_entoffsets.m_flVelocityModifier),
														  1,
														  0x100,
														  "",
														  sizeof(float),
														  0.00390625f };
				const auto type_array = new typedescription_t[map->m_num_fields + 1];
				memcpy(type_array, map->m_desc, sizeof(typedescription_t) * map->m_num_fields);
				type_array[map->m_num_fields] = type_description;
				map->m_desc = type_array;
				map->m_optimized_map = nullptr;
				map->m_num_fields++;
				map->m_packed_size = 0;
				g_cl.m_map_setup = CPredictionCopy::PrepareDataMap(map);
				if (g_cl.m_map_setup)
					g_predmanager.init(map);
			}
		}
	}

	if (g_cl.m_processing) {
		if (commands_acknowledged > 0) {


			g_predmanager.PostUpdate(g_cl.m_local);

			if (g_inputpred.m_had_pred_error) {
				g_csgo.m_prediction->m_previous_startframe = -1;
				g_csgo.m_prediction->m_commands_predicted = 0;
				g_csgo.m_prediction->m_bPreviousAckHadErrors() = true;

				g_inputpred.m_had_pred_error = false;
			}
		}
	}

	oPostNetworkDataReceived(g_csgo.m_prediction, commands_acknowledged);
}

using FnPostEntityPacketReceived = void(__thiscall*)(void*);
FnPostEntityPacketReceived oPostEntityPacketReceived;

void __stdcall hkPostEntityPacketReceived() {
	if (!g_csgo.m_entlist || !g_csgo.m_prediction)
		return oPostEntityPacketReceived(g_csgo.m_prediction);

	oPostEntityPacketReceived(g_csgo.m_prediction);
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());
}

using FnUpdateAnimationState = void(__vectorcall*)(CCSGOPlayerAnimState*, void*, ang_t);
FnUpdateAnimationState oUpdateAnimationState;

void Force_proxy(CRecvProxyData* data, Address ptr, Address out) {
	// convert to ragdoll.
	Ragdoll* ragdoll = ptr.as< Ragdoll* >();

	// get ragdoll owner.
	Player* player = ragdoll->GetPlayer();

	// we only want this happening to noobs we kill.
	if (g_cl.m_local && player && player->enemy(g_cl.m_local)) {
		// get m_vecForce.
		vec3_t vel = { data->m_Value.m_Vector[0], data->m_Value.m_Vector[1], data->m_Value.m_Vector[2] };

		// give some speed to all directions.
		vel *= 3.f;

		// boost z up a bit.
		if (vel.z <= 1.f)
			vel.z = 1.1f;

		vel.z *= 1.5f;

		// don't want crazy values for this... probably unlikely though?
		math::clamp(vel.x, std::numeric_limits< float >::lowest(), std::numeric_limits< float >::max());
		math::clamp(vel.y, std::numeric_limits< float >::lowest(), std::numeric_limits< float >::max());
		math::clamp(vel.z, std::numeric_limits< float >::lowest(), std::numeric_limits< float >::max());

		// set new velocity.
		data->m_Value.m_Vector[0] = vel.x;
		data->m_Value.m_Vector[1] = vel.y;
		data->m_Value.m_Vector[2] = vel.z;
	}

	if (g_hooks.m_Force_original)
		g_hooks.m_Force_original(data, ptr, out);
}

void Hooks::init() {

	// hook wndproc.
	m_old_wndproc = (WNDPROC)g_winapi.SetWindowLongA(g_csgo.m_game->m_hWindow, GWL_WNDPROC, util::force_cast<LONG>(Hooks::WndProc));

	// setup normal VMT hooks.
	m_panel.init(g_csgo.m_panel);
	m_panel.add(IPanel::PAINTTRAVERSE, util::force_cast(&Hooks::PaintTraverse));

	m_client.init(g_csgo.m_client);
	m_client.add(CHLClient::LEVELINITPREENTITY, util::force_cast(&Hooks::LevelInitPreEntity));
	m_client.add(CHLClient::LEVELINITPOSTENTITY, util::force_cast(&Hooks::LevelInitPostEntity));
	m_client.add(CHLClient::LEVELSHUTDOWN, util::force_cast(&Hooks::LevelShutdown));
	m_client.add(CHLClient::FRAMESTAGENOTIFY, util::force_cast(&Hooks::FrameStageNotify));

	m_engine.init(g_csgo.m_engine);
	m_engine.add(IVEngineClient::ISCONNECTED, util::force_cast(&Hooks::IsConnected));
	m_engine.add(IVEngineClient::ISHLTV, util::force_cast(&Hooks::IsHLTV));
	m_engine.add(IVEngineClient::ISPAUSED, util::force_cast(&Hooks::IsPaused));

	m_prediction.init(g_csgo.m_prediction);
	m_prediction.add(CPrediction::INPREDICTION, util::force_cast(&Hooks::InPrediction));
	m_prediction.add(CPrediction::RUNCOMMAND, util::force_cast(&Hooks::RunCommand));

	m_client_mode.init(g_csgo.m_client_mode);
	m_client_mode.add(IClientMode::SHOULDDRAWFOG, util::force_cast(&Hooks::ShouldDrawFog));
	m_client_mode.add(IClientMode::OVERRIDEVIEW, util::force_cast(&Hooks::OverrideView));
	m_client_mode.add(IClientMode::CREATEMOVE, util::force_cast(&Hooks::CreateMove));
	m_client_mode.add(IClientMode::DOPOSTSPACESCREENEFFECTS, util::force_cast(&Hooks::DoPostScreenSpaceEffects));

	m_surface.init(g_csgo.m_surface);
	m_surface.add(ISurface::LOCKCURSOR, util::force_cast(&Hooks::LockCursor));
	m_surface.add(ISurface::PLAYSOUND, util::force_cast(&Hooks::PlaySound));
	m_surface.add(ISurface::ONSCREENSIZECHANGED, util::force_cast(&Hooks::OnScreenSizeChanged));

	m_model_render.init(g_csgo.m_model_render);
	m_model_render.add(IVModelRender::DRAWMODELEXECUTE, util::force_cast(&Hooks::DrawModelExecute));

	m_render_view.init(g_csgo.m_render_view);
	m_render_view.add(IVRenderView::SCENEEND, util::force_cast(&Hooks::SceneEnd));


	m_shadow_mgr.init(g_csgo.m_shadow_mgr);
	m_shadow_mgr.add(IClientShadowMgr::COMPUTESHADOWDEPTHTEXTURES, util::force_cast(&Hooks::ComputeShadowDepthTextures));

	m_view_render.init(g_csgo.m_view_render);
	m_view_render.add(CViewRender::RENDER2DEFFECTSPOSTHUD, util::force_cast(&Hooks::Render2DEffectsPostHUD));
	m_view_render.add(CViewRender::RENDERSMOKEOVERLAY, util::force_cast(&Hooks::RenderSmokeOverlay));
	m_view_render.add(CViewRender::ONRENDERSTART, util::force_cast(&Hooks::OnRenderStart));

	m_material_system.init(g_csgo.m_material_system);
	m_material_system.add(IMaterialSystem::OVERRIDECONFIG, util::force_cast(&Hooks::OverrideConfig));

	m_fire_bullets.init(g_csgo.TEFireBullets);
	m_fire_bullets.add(7, util::force_cast(&Hooks::PostDataUpdate));

	m_client_state.init(g_csgo.m_hookable_cl);
	m_client_state.add(CClientState::PACKETSTART, util::force_cast(&Hooks::PacketStart));

	// register our custom entity listener.
	// todo - dex; should we push our listeners first? should be fine like this.
	g_custom_entity_listener.init();

	// cvar hooks.
	m_debug_spread.init(g_csgo.weapon_debug_spread_show);
	m_debug_spread.add(ConVar::GETINT, util::force_cast(&Hooks::DebugSpreadGetInt));

	// set netvar proxies.
	g_netvars.SetProxy(HASH("DT_CSPlayer"), HASH("m_flLowerBodyYawTarget"), Body_proxy, m_Body_original);
	g_netvars.SetProxy(HASH("DT_CSRagdoll"), HASH("m_vecForce"), Force_proxy, m_Force_original);
	g_netvars.SetProxy(HASH("DT_BaseEntity"), HASH("m_flSimulationTime"), SimulationTime_proxy, m_SimTime_original);

#define hook( target, hook, original ) \
   MH_CreateHook( Address{ target }.as< LPVOID >( ), \
   reinterpret_cast< LPVOID >( &hook ), reinterpret_cast< LPVOID* >( &original ) )

	static auto ModifEyePosAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F8 83 EC 58 56 57 8B F9 83 7F 60 00");
	static auto InterpAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 ? 83 EC ? 53 56 8B F1 57 83 BE ? ? ? ? ? 75 ? 8B 46 ? 8D 4E ? FF 50 ? 85 C0 74 ? 8B CE E8 ? ? ? ? 8B 9E");
	static auto updateCsaAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 51 56 8B F1 80 BE ?? ?? 00 00 00 74 36 8B 06 FF 90 ?? ?? 00 00");
	static auto doExtraBoneProcAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C");
	static auto BuildTransAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F0 81 EC ? ? ? ? 56 57 8B F9 8B 0D ? ? ? ? 89 7C 24 1C");
	static auto CheckSeqChangeAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 51 53 8B 5D 08 56 8B F1 57 85");
	static auto UpdateANimState = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 F3");
	static auto calcViewAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 53 8B 5D 08 56 57 FF 75 18");
	static auto SkipFrameAddr = pattern::find(g_csgo.m_client_dll, "57 8B F9 8B 07 8B 80 ? ? ? ? FF D0 84 C0 75 02");
	static auto StandardBlendingRulesAddr = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F0 B8 ? ? ? ? E8 ? ? ? ? 56 8B 75 08 57 8B F9 85 F6");
	static auto _run_simulation = pattern::find(g_csgo.m_client_dll, ("E8 ? ? ? ? A1 ? ? ? ? F3 0F 10 45 ? F3 0F 11 40")).rel32(1).as< void* >();
	static auto SetVisualsDataAddr = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 53 8B D9 56 57 8B 53 5C"));
	static auto PhysicsSimulate = pattern::find(g_csgo.m_client_dll, "56 8B F1 8B 8E ? ? ? ? 83 F9 FF 74 21");
	static auto SetupBones = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 51 56 8B F1 80 BE ? ? ? ? ? 74 36")).as<void*>();
	static auto	processMovement = util::get_method<void*>(g_csgo.m_game_movement, CGameMovement::PROCESSMOVEMENT);
	static auto	aCLReadPackets = pattern::find(g_csgo.m_engine_dll, XOR("53 8A D9 8B 0D ? ? ? ? 56 57 8B B9")).as<void*>();
	static auto	aHudScopePaint = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 83 EC 78 56 57 8B 3D ? ? ? ? 89 4C 24 18")).as<void*>();
	static auto	ClFireevents = pattern::find(g_csgo.m_engine_dll, XOR("55 8B EC 83 EC 08 53 8B 1D ? ? ? ? 56 57 83 BB ? ? ? ? ? 74")).as<void*>();
	static auto	Teleported = pattern::find(g_csgo.m_client_dll, XOR("E8 ? ? ? ? 84 C0 75 0D 8B 87 ? ? ? ? C1 E8 03 A8 01 74 2E 8B")).rel32(0x1).as<void*>();
	static auto	BaseInterpolatePart1 = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 51 8B 45 14 56 8B F1 C7 00 ? ? ? ? 8B 06 8B 80")).as<void*>();
	static auto	ProcessInterpList = pattern::find(g_csgo.m_client_dll, XOR("0F ? ? ? ? ? ? 3D ? ? ? ? 74 3F")).as<void*>();
	static auto	aInterpolateServerEntities = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56")).as<void*>();
	static auto	aGetExposureRange = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 51 80 3D ? ? ? ? ? 0F 57")).as<void*>();
	static auto AccumulateLayers = pattern::find(g_csgo.m_client_dll, "55 8B EC 57 8B F9 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ?");
	static auto ProcessInterp = pattern::find(g_csgo.m_client_dll, "0F B7 05 ? ? ? ? 3D ? ? ? ? 74 3F");
	static auto	DoExtraBoneProcessing = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 81 EC ? ? ? ? 53 56 8B F1 57 89 74 24 1C")).as<void*>();
	static auto InitNewParticlesScalarAddr = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 18 56 8B F1 C7 45"));
	static auto CreateGrenadeTrail = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 E4 F8 83 EC 40 56 8B 35 ? ? ? ?"));
	static auto InterpolateServerEnts = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 1C 8B 0D ? ? ? ? 53 56"));
	static auto CL_FireEvents = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 83 EC 08 53 8B 1D ? ? ? ? 56 57 83 BB ? ? ? ? ? 74"));
	static auto UpdateActivityModifiers = pattern::find(g_csgo.m_server_dll, XOR("E8 ? ? ? ? 8B 7D 08 83 FF 12")).rel32(1);
	static auto SendWeaponAnim = pattern::find(g_csgo.m_client_dll, XOR("55 8B EC 53 56 8B ? ? ? ? ? 57 8B F9"));
	static auto	aPreEntityPacketReceived = util::get_method<void*>(g_csgo.m_prediction, 4);
	static auto	aPostEntityPacketReceived = util::get_method<void*>(g_csgo.m_prediction, 5);
	static auto	aPostNetworkDataReceived = util::get_method<void*>(g_csgo.m_prediction, 6);


	MH_Initialize();
	hook(ModifEyePosAddr, mhooks::ModifyEyePos, mhooks::oModifyEyePos);
	hook(SendWeaponAnim, mhooks::SendWeaponAnim, mhooks::oSendWeaponAnim);
	//hook(InterpolateServerEnts, mhooks::InterpolateServerEntities, mhooks::oInterpolateServerEntities);
	//hook(UpdateActivityModifiers, mhooks::UpdateActivityModifiers, mhooks::oUpdateActivityModifiers);
	//hook(ProcessInterp, mhooks::ProcessInterpolatedList, mhooks::oProcessInterpolatedList);
	hook(updateCsaAddr, mhooks::UpdateClientSideAnim, mhooks::oUpdateClientSideAnim);
	//hook(CreateGrenadeTrail, mhooks::CreateGrenadeTrail, mhooks::oCreateGrenadeTrail);
	//hook(InterpAddr, mhooks::interpolate, mhooks::oInterpolate);
	hook(BuildTransAddr, mhooks::BuildTransformations, mhooks::oBuildTransformations);
	//hook(CheckSeqChangeAddr, mhooks::CheckForSequenceChange, mhooks::oCheckForSeqChange);
	hook(SkipFrameAddr, mhooks::ShouldSkipAnimationFrame, mhooks::oShouldSkipAnimationFrame);
	hook(calcViewAddr, mhooks::CalcView, mhooks::oCalcView);
	//hook(BaseInterpolatePart1, mhooks::BaseInterpolatePart1, mhooks::oBaseInterpolate);
	hook(StandardBlendingRulesAddr, mhooks::StandardBlendingRules, mhooks::oStandardBlendingRules);
	//hook(UpdateANimState, mhooks::update_animation_state, mhooks::original_update_animation_state);
	//hook(PhysicsSimulate, mhooks::PhysicsSimulate, mhooks::oPhysicsSimulate);
	hook(SetVisualsDataAddr, mhooks::HkSetVisualsData, mhooks::oSetVisualsData);
	hook(processMovement, hkProcessMovement, oProcessMovement);
	//hook(Teleported,					mhooks::Teleported, mhooks::oTeleported);
	hook(SetupBones, mhooks::SetupBones, mhooks::oSetupBones);
	hook(aGetExposureRange, hkGetExposureRange, oGetExposureRange);
	//hook(_run_simulation, mhooks::RunSimulation, mhooks::oRunSimulation);
	//hook(AccumulateLayers, mhooks::AccumulateLayers, mhooks::oAccumulateLayers);
	hook(DoExtraBoneProcessing, mhooks::DoExtraBoneProcessing, mhooks::oDoExtraBoneProcessing);
	hook(aPreEntityPacketReceived, hkPreEntityPacketReceived, oPreEntityPacketReceived);
	hook(aPostEntityPacketReceived, hkPostEntityPacketReceived, oPostEntityPacketReceived);
	hook(aPostNetworkDataReceived, hkPostNetworkDataReceived, oPostNetworkDataReceived);
	MH_EnableHook(MH_ALL_HOOKS);
}