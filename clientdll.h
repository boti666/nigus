#pragma once
#include "engineclient.h"


enum Stage_t {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

enum Hitgroups_t {
	HITGROUP_GENERIC  = 0,
	HITGROUP_HEAD     = 1,
	HITGROUP_CHEST    = 2,
	HITGROUP_STOMACH  = 3,
	HITGROUP_LEFTARM  = 4,
	HITGROUP_RIGHTARM = 5,
	HITGROUP_LEFTLEG  = 6,
	HITGROUP_RIGHTLEG = 7,
	HITGROUP_NECK     = 8,
	HITGROUP_GEAR     = 10,
	HITGROUP_LEGS     = 99,
};

class CHLClient {
public:
	enum indices : size_t {
		INIT                = 0,
		LEVELINITPREENTITY  = 5,
		LEVELINITPOSTENTITY = 6,
		LEVELSHUTDOWN		= 7,
		GETALLCLASSES       = 8,
		HUDPROCESSINPUT     = 10,
		INACTIVATEMOUSE     = 15,
		INKEYEVENT          = 20,
		CREATEMOVE          = 21,
		write_user_cmd_delta_to_buffer = 23,
		RENDERVIEW          = 27,
		FRAMESTAGENOTIFY    = 36,
		DISPATCHUSERMESSAGE = 37,
	};

public:
	__forceinline ClientClass* GetAllClasses( ) {
		return util::get_method< ClientClass*( __thiscall* )( decltype( this ) )>( this, GETALLCLASSES )( this );
	}
	__forceinline bool WriteUserCmdDeltaToBuffer(int slot, bf_write* buf, int from, int to, bool isnewcommand) {
		return util::get_method< bool(__thiscall*)(void*, int, bf_write*, int, int, bool) >(this, write_user_cmd_delta_to_buffer)(this, slot, buf, from, to, isnewcommand);
	}
};