#include "includes.h"

bool Hooks::ShouldDrawParticles( ) {
	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawParticles_t >( IClientMode::SHOULDDRAWPARTICLES )( this );
}

bool Hooks::ShouldDrawFog( ) {
	// remove fog.
	if( g_menu.main.visuals.removals.get( 2 ) )
		return false;

	return g_hooks.m_client_mode.GetOldMethod< ShouldDrawFog_t >( IClientMode::SHOULDDRAWFOG )( this );
}

void Hooks::OverrideView(CViewSetup* view) {
	// damn son.
	g_cl.m_local = g_csgo.m_entlist->GetClientEntity< Player* >(g_csgo.m_engine->GetLocalPlayer());

	// g_grenades.think( );
	g_visuals.ThirdpersonThink();

	// call original.
	g_hooks.m_client_mode.GetOldMethod< OverrideView_t >(IClientMode::OVERRIDEVIEW)(this, view);
}

void Hooks::OnRenderStart() {
	g_hooks.m_view_render.GetOldMethod<OnRenderStart_t>(CViewRender::ONRENDERSTART)(this);

    float scoped_fov{ g_menu.main.misc.fov_scoped_percent.get() / 100.f };
    float fov{ g_menu.main.misc.fov_amount.get() };

    if (g_cl.m_local && g_cl.m_local->m_bIsScoped()) {
        if (g_cl.m_weapon && g_cl.m_weapon->m_zoomLevel() != 2)
            fov /= (1.f + 1.f * scoped_fov);
        else
            fov /= (1.f + 2.f * scoped_fov);
    }
	
	g_csgo.m_view_render->m_view.m_fov = fov;

	if (g_menu.main.misc.viewmodel_fov.get())
		g_csgo.m_view_render->m_view.m_viewmodel_fov = g_menu.main.misc.viewmodel_fov_percent.get();
}

bool Hooks::CreateMove( float time, CUserCmd* cmd ) {
	Stack   stack;
	bool    ret;

	ret = g_hooks.m_client_mode.GetOldMethod< CreateMove_t >( IClientMode::CREATEMOVE )( this, time, cmd );

	if( !cmd->m_command_number )
		return ret;

	g_cl.m_orig_ang = cmd->m_view_angles;

	if (ret) {
		g_csgo.m_engine->SetViewAngles(cmd->m_view_angles);
	}

	if (g_gui.m_open)
		cmd->m_buttons &= ~(IN_ATTACK | IN_ATTACK2);

	cmd->m_random_seed = g_csgo.MD5_PseudoRandom( cmd->m_command_number ) & 0x7fffffff;

	// get bSendPacket off the stack.
	g_cl.m_packet = stack.next().local(0x1c).as< bool* >();

	// get bFinalTick off the stack.
	g_cl.m_final_packet = stack.next().local(0x1b).as< bool* >();

	g_cl.m_old_duck = g_cl.m_local->m_flDuckAmount();

	// invoke move function.
	g_cl.OnTick( cmd );

	return false;
}

bool Hooks::DoPostScreenSpaceEffects( CViewSetup* setup ) {
	g_visuals.RenderGlow( );

	return g_hooks.m_client_mode.GetOldMethod< DoPostScreenSpaceEffects_t >( IClientMode::DOPOSTSPACESCREENEFFECTS )( this, setup );
}