#include "includes.h"

inline bool isWeapon(std::string& modelName) {
	if ((modelName.find("v_") != std::string::npos || modelName.find("uid") != std::string::npos || modelName.find("stattrack") != std::string::npos) && modelName.find("arms") == std::string::npos) {
		return true;
	}

	return false;
}

void Hooks::DrawModelExecute(uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone) {

	static bool run_once = false;
	static bool run_once2 = false;

	if (g_menu.main.visuals.removals.get(4)) {
		if (!run_once) {
			auto blur_overlay = g_csgo.m_material_system->FindMaterial(("dev/scope_bluroverlay"), XOR("Other textures"));
			auto lens_dirt = g_csgo.m_material_system->FindMaterial(("models/weapons/shared/scope/scope_lens_dirt"), XOR("Other textures"));
			auto xblur_mat = g_csgo.m_material_system->FindMaterial(("dev/blurfilterx_nohdr"), XOR("Other textures"));
			auto yblur_mat = g_csgo.m_material_system->FindMaterial(("dev/blurfiltery_nohdr"), XOR("Other textures"));

			xblur_mat->SetFlag(MATERIAL_VAR_NO_DRAW, true);
			yblur_mat->SetFlag(MATERIAL_VAR_NO_DRAW, true);
			blur_overlay->SetFlag(MATERIAL_VAR_NO_DRAW, true);
			lens_dirt->SetFlag(MATERIAL_VAR_NO_DRAW, true);
			run_once = true;
			run_once2 = false;
		}
	}
	else {
		if (!run_once2) {
			auto blur_overlay = g_csgo.m_material_system->FindMaterial(("dev/scope_bluroverlay"), XOR("Other textures"));
			auto lens_dirt = g_csgo.m_material_system->FindMaterial(("models/weapons/shared/scope/scope_lens_dirt"), XOR("Other textures"));
			auto xblur_mat = g_csgo.m_material_system->FindMaterial(("dev/blurfilterx_nohdr"), XOR("Other textures"));
			auto yblur_mat = g_csgo.m_material_system->FindMaterial(("dev/blurfiltery_nohdr"), XOR("Other textures"));

			xblur_mat->SetFlag(MATERIAL_VAR_NO_DRAW, false);
			yblur_mat->SetFlag(MATERIAL_VAR_NO_DRAW, false);
			blur_overlay->SetFlag(MATERIAL_VAR_NO_DRAW, false);
			lens_dirt->SetFlag(MATERIAL_VAR_NO_DRAW, false);
			run_once = false;
			run_once2 = true;
		}
	}

	if (!info.m_model)
		return g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);

	if (g_csgo.m_engine->IsInGame()) {
		if (strstr(info.m_model->m_name, XOR("player/contactshadow")) != nullptr) {
			return;
		}
	}

	// do chams.
	if (g_chams.DrawModel(ctx, state, info, bone)) {
		g_hooks.m_model_render.GetOldMethod< Hooks::DrawModelExecute_t >(IVModelRender::DRAWMODELEXECUTE)(this, ctx, state, info, bone);
	}

	// disable material force for next call.
	//g_csgo.m_studio_render->ForcedMaterialOverride( nullptr );
}