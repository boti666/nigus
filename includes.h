#pragma once

#pragma warning( disable : 4307 ) // '*': integral constant overflow
#pragma warning( disable : 4244 ) // possible loss of data
#pragma warning( disable : 4800 ) // forcing value to bool 'true' or 'false'
#pragma warning( disable : 4838 ) // conversion from '::size_t' to 'int' requires a narrowing conversion

// You can define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING or _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS to acknowledge that you have received this warning.
#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#define VC_EXTRALEAN
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define CHECK_VALID(_v) 0

#define	CS_MAX_BONES 128

const float CS_PLAYER_SPEED_DUCK_MODIFIER = 0.34f;
const float CS_PLAYER_SPEED_WALK_MODIFIER = 0.52f;
const float CS_PLAYER_SPEED_CLIMB_MODIFIER = 0.34f;
const float CS_PLAYER_SPEED_RUN = 260.0f;

#define seq_byte -133729344
#define crash_key 55555
#define normal_key 12345
#define MIN_SIMULATION_TICKS 1
#define	MAX_SIMULATION_TICKS 17


#define M_PI		3.14159265358979323846f
#define M_RADPI		57.295779513082f
#define M_PIRAD     0.01745329251f
#define RAD2DEG( x  )  ( (float)(x) * (float)(180.f / M_PI) )
#define DEG2RAD( x  )  ( (float)(x) * (float)(M_PI / 180.f) )

using ulong_t = unsigned long;

// windows / stl includes.
#include <Windows.h>
#include <cstdint>
#include <intrin.h>
#include <xmmintrin.h>
#include <array>
#include <vector>
#include <algorithm>
#include <cctype>
#include <string>
#include <execution>
#include <chrono>
#include <thread>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <deque>
#include <functional>
#include <map>
#include <shlobj.h>
#include <filesystem>
#include <streambuf>
#include <optional>

// our custom wrapper.
#include "unique_vector.h"
#include "tinyformat.h"

// other includes.
#include "hash.h"
#include "xorstr.h"
#include "pe.h"
#include "winapir.h"

#include "address.h"
#include "util.h"
#include "modules.h"
#include "pattern.h"
#include "vmt.h"
#include "stack.h"
#include "nt.h"
#include "x86.h"
#include "syscall.h"
#include "offsets.h"

#define add_hash() ^ (uint64_t)2166136261u
#define make_offset( modulename, offset ) []() -> uintptr_t { static volatile uintptr_t module_base =  ( uintptr_t ) GetModuleHandleA( modulename ); return module_base + (uint32_t) ( offsets::offset ^ static_cast<uint64_t>( offsets::randomized_hash::offset ) add_hash() ); }()

#define MEMEBR_FUNC_ARGS(...) ( this, __VA_ARGS__ ); }
#define FUNCARGS(...) ( __VA_ARGS__ ); }
#define MFUNC(func, sig, offset) auto func { static auto _offset = offset; return reinterpret_cast< sig >( _offset ) MEMEBR_FUNC_ARGS
#define _(n, s) auto (n) = std::string(s)

// hack includes.
#include "interfaces.h"
#include "sdk.h"
#include "csgo.h"
#include "netvars.h"
#include "entoffsets.h"
#include "entity.h"
#include "ikcontext.h"
#include "autowall.h"
#include "client.h"
#include "server_animations.h"
#include "gamerules.h"
#include "networking.h"
#include "hooks.h"
#include "render.h"
#include "CPlayerResource.h"
#include "C_VoiceCommunication.h"
#include "DormantSystem.h"
#include "pred.h"
#include "lagrecord.h"
#include "visuals.h"
#include "movement.h"
#include "hvh.h"
#include "lagcomp.h"
#include "aimbot.h"
#include "ragebot.h"
#include "anim_sync.h"
#include "netdata.h"
#include "chams.h"
#include "notify.h"
#include "resolver.h"
#include "correction.h"
#include "grenades.h"
#include "skins.h"
#include "events.h"
#include "shots.h"
#include "bonesetup.h"
#include "deps/minhook/MinHook.h"
#include "helpers.h"
#include "grenade_path.h"
#include "localize.h"

// gui includes.
#include "json.h"
#include "base64.h"
#include "element.h"
#include "checkbox.h"
#include "dropdown.h"
#include "multidropdown.h"
#include "slider.h"
#include "colorpicker.h"
#include "edit.h"
#include "keybind.h"
#include "button.h"
#include "tab.h"
#include "form.h"
#include "gui.h"
#include "callbacks.h"
#include "menu.h"
#include "config.h"
