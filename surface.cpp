#include "includes.h"

void Hooks::LockCursor( ) {
	if( g_gui.m_open ) 
		return g_csgo.m_surface->UnlockCursor( );

	g_hooks.m_surface.GetOldMethod< LockCursor_t >( ISurface::LOCKCURSOR )( this );
}

void Hooks::PlaySound( const char* name ) {
	g_hooks.m_surface.GetOldMethod< PlaySound_t >( ISurface::PLAYSOUND )( this, name );
}

void Hooks::OnScreenSizeChanged( int oldwidth, int oldheight ) {
	g_hooks.m_surface.GetOldMethod< OnScreenSizeChanged_t >( ISurface::ONSCREENSIZECHANGED )( this, oldwidth, oldheight );

	render::init( );

	Visuals::ModulateWorld();
}