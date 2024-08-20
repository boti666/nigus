#pragma once
#define MAX_PARTICLE_ATTRIBUTES 24

// Max Possible Splitpacket Count (-1 as it sets up the transfer)
#define MAX_VOICETRANSFER_SPLITPACKET 255
// Max Possible Data Per 1 voicepacket
#define MAX_VOICETRANSFER_PACKET_SIZE 24
// Max Possible Data we can send with 1 non-splitpacket packet
#define MAX_VOICETRANSFER_PACKET_DATA (MAX_VOICETRANSFER_PACKET_SIZE - 5)
// Max Possible Data we can send per split packet
#define MAX_VOICETRANSFER_SPLITPACKET_DATA (MAX_VOICETRANSFER_PACKET_DATA - 1)
// Max Possible Amount we can network
#define MAX_VOICETRANSFER_NETWORKABLE (MAX_VOICETRANSFER_SPLITPACKET_DATA * MAX_VOICETRANSFER_SPLITPACKET)

struct VoiceDataCustom
{
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	int32_t sequence_bytes{};
	uint32_t section_number{};
	uint32_t uncompressed_sample_offset{};

	__forceinline uint8_t* get_raw_data()
	{
		return (uint8_t*)this;
	}
};

enum VoiceTransferPacketFlags {
	ReliableSend = 0b00000001,
	SplitPacket = 0b00000010,
	ReliableAck = 0b00000100,
};

// Some tranfer struct
struct VoiceTransferDataTransmit_t
{
	char m_szID[4]; // Default: Over
	int8_t m_nDataType;
	char m_szDataPacket[19];
};

struct CCLCMsg_VoiceData_Legacy
{
	uint32_t INetMessage_Vtable; //0x0000
	char pad_0004[4]; //0x0004
	uint32_t CCLCMsg_VoiceData_Vtable; //0x0008
	char pad_000C[8]; //0x000C
	void* data; //0x0014
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	int32_t format; //0x0020
	int32_t sequence_bytes; //0x0024
	uint32_t section_number; //0x0028
	uint32_t uncompressed_sample_offset; //0x002C
	int32_t cached_size; //0x0030

	uint32_t flags; //0x0034

	uint8_t no_stack_overflow[0xFF];

	__forceinline void set_data(VoiceDataCustom* cdata)
	{
		xuid_low = cdata->xuid_low;
		xuid_high = cdata->xuid_high;
		sequence_bytes = cdata->sequence_bytes;
		section_number = cdata->section_number;
		uncompressed_sample_offset = cdata->uncompressed_sample_offset;
	}
};

struct lame_string_t

{
	char data[16]{};
	uint32_t current_len = 0;
	uint32_t max_len = 15;
};

struct CSVCMsg_VoiceData_Legacy
{
	char pad_0000[8]; //0x0000
	int32_t client; //0x0008
	int32_t audible_mask; //0x000C
	uint32_t xuid_low{};
	uint32_t xuid_high{};
	void* voide_data_; //0x0018
	int32_t proximity; //0x001C
	//int32_t caster; //0x0020
	int32_t format; //0x0020
	int32_t sequence_bytes; //0x0024
	uint32_t section_number; //0x0028
	uint32_t uncompressed_sample_offset; //0x002C

	__forceinline VoiceDataCustom get_data()
	{
		VoiceDataCustom cdata;
		cdata.xuid_low = xuid_low;
		cdata.xuid_high = xuid_high;
		cdata.sequence_bytes = sequence_bytes;
		cdata.section_number = section_number;
		cdata.uncompressed_sample_offset = uncompressed_sample_offset;
		return cdata;
	}
};

class C_INIT_RandomColor {
	BYTE pad_0[92];
public:
	vec3_t m_normal_color_min;
	vec3_t m_normal_color_max;
};

struct CParticleAttributeAddressTable {
	float* m_pAttributes[MAX_PARTICLE_ATTRIBUTES];
	size_t m_nFloatStrides[MAX_PARTICLE_ATTRIBUTES];

	__forceinline float* FloatAttributePtr(int nAttribute, int nParticleNumber) const {
		int block_ofs = nParticleNumber / 4;
		return m_pAttributes[nAttribute] +
			m_nFloatStrides[nAttribute] * block_ofs +
			(nParticleNumber & 3);
	}
};

struct CUtlString_simple {
	char* buffer;
	int capacity;
	int grow_size;
	int length;
};

template<class T> struct CUtlReference {
	CUtlReference* m_pNext;
	CUtlReference* m_pPrev;
	T* m_pObject;
};

template<class T> struct CUtlIntrusiveList {
	T* m_pHead;
};
template<class T> struct CUtlIntrusiveDList : public CUtlIntrusiveList<T> {};
template<class T> struct CUtlReferenceList : public CUtlIntrusiveDList< CUtlReference<T> > {};

class CParticleSystemDefinition {
	BYTE pad_0[308];
public:
	CUtlString_simple m_Name;
};

class CParticleCollection {
	BYTE pad_0[48];//0
public:
	int m_nActiveParticles;//48
private:
	BYTE pad_1[12];//52
public:
	CUtlReference<CParticleSystemDefinition> m_pDef;//64
private:
	BYTE pad_2[60];//80
public:
	CParticleCollection* m_pParent;//136
private:
	BYTE pad_3[84];//140
public:
	CParticleAttributeAddressTable m_ParticleAttributes;//224
};

void __fastcall CParticleCollection_Simulate(CParticleCollection* thisPtr, void* edx);
inline decltype(&CParticleCollection_Simulate) OrigCParticleCollection_Simulate{ };

struct network_data_t {
	int sequence;
	int cmd;
};

class Hooks {
public:
	void init( );

	bool m_updating_anims[65];

public:
	// forward declarations
	class IRecipientFilter;

	// prototypes.
	using PaintTraverse_t              = void( __thiscall* )( void*, VPANEL, bool, bool );
	using DoPostScreenSpaceEffects_t   = bool( __thiscall* )( void*, CViewSetup* );
	using CreateMove_t                 = bool( __thiscall* )( void*, float, CUserCmd* );
	using LevelInitPostEntity_t        = void( __thiscall* )( void* );
	using LevelShutdown_t              = void( __thiscall* )( void* );
	using LevelInitPreEntity_t         = void( __thiscall* )( void*, const char* );
	using IN_KeyEvent_t                = int( __thiscall* )( void*, int, int, const char* );
	using FrameStageNotify_t           = void( __thiscall* )( void*, Stage_t );
	using UpdateClientSideAnimation_t  = void( __thiscall* )( void* );
    using GetActiveWeapon_t            = Weapon*( __thiscall * )( void* );
	using DoExtraBoneProcessing_t      = void( __thiscall* )( void*, int, int, int, int, int, int );
	using StandardBlendingRules_t = void(__thiscall*)(void*, CStudioHdr*, int, int, int, int);
	using CalcView_t = void(__thiscall*)(void*, vec3_t&, vec3_t&, float&, float&, float&);
	using BuildTransformations_t       = void( __thiscall* )( void*, int, int, int, int, int, int );
	using CalcViewModelView_t          = void( __thiscall* )( void*, vec3_t&, ang_t& );
	using InPrediction_t               = bool( __thiscall* )( void* );
	using OverrideView_t               = void( __thiscall* )( void*, CViewSetup* );
	using LockCursor_t                 = void( __thiscall* )( void* );
	using RunCommand_t                 = void( __thiscall* )( void*, Entity*, CUserCmd*, IMoveHelper* );
	using ProcessPacket_t              = void( __thiscall* )( void*, void*, bool );
	using SendDatagram_t               = int( __thiscall* )( void*, void* );
	// using CanPacket_t                = bool( __thiscall* )( void* );
	using PlaySound_t                  = void( __thiscall* )( void*, const char* );
	using GetScreenSize_t              = void( __thiscall* )( void*, int&, int& );
	using Push3DView_t                 = void( __thiscall* )( void*, CViewSetup&, int, void*, void* );
	using SceneEnd_t                   = void( __thiscall* )( void* );
	using DrawModelExecute_t           = void( __thiscall* )( void*, uintptr_t, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t* );
	using ComputeShadowDepthTextures_t = void( __thiscall* )( void*, const CViewSetup&, bool );
	using GetInt_t                     = int( __thiscall* )( void* );
	using GetBool_t                    = bool( __thiscall* )( void* );
	using IsConnected_t                = bool( __thiscall* )( void* );
	using IsHLTV_t                     = bool( __thiscall* )( void* );
	using IsPaused_t = bool(__thiscall*)(void*);
	using OnEntityCreated_t            = void( __thiscall* )( void*, Entity* );
	using OnEntityDeleted_t            = void( __thiscall* )( void*, Entity* );
	using RenderSmokeOverlay_t         = void( __thiscall* )( void*, bool );
	using ShouldDrawFog_t              = bool( __thiscall* )( void* );
	using ShouldDrawParticles_t        = bool( __thiscall* )( void* );
	using Render2DEffectsPostHUD_t     = void( __thiscall* )( void*, const CViewSetup& );
	using OnRenderStart_t              = void( __thiscall* )( void* );
    using RenderView_t                 = void( __thiscall* )( void*, const CViewSetup &, const CViewSetup &, int, int );
	using GetMatchSession_t            = CMatchSessionOnlineHost*( __thiscall* )( void* );
	using OnScreenSizeChanged_t        = void( __thiscall* )( void*, int, int );
	using OverrideConfig_t             = bool( __thiscall* )( void*, MaterialSystem_Config_t*, bool );
	using PostDataUpdate_t             = void( __thiscall* )( void*, DataUpdateType_t );
	using TempEntities_t               = bool( __thiscall* )( void*, void * );
	using PacketStart_t					= void(__thiscall*)(void*, int, int);
	using EmitSound_t                  = void( __thiscall* )( void*, IRecipientFilter&, int, int, const char*, unsigned int, const char*, float, float, int, int, int, const vec3_t*, const vec3_t*, void*, bool, float, int );
	// using PreDataUpdate_t            = void( __thiscall* )( void*, DataUpdateType_t );
	using FnVoiceData = void(__thiscall*)(void*, void*);
	using BeginFrameFn = void(__thiscall*)(void*, float);
	using PacketEnd_T = void(__thiscall*)(void*);
	using SVCMsg_GameEvent_t = bool(__thiscall*)(void*, void*);
	using SendNetMsg_t = bool(__thiscall*)(void*, INetMessage&, bool, bool);

public:
	bool					 SVCMsg_GameEvent(void* message);
	bool                     TempEntities( void *msg );
	int					 PacketStart(int incoming_sequence, int outgoing_acknowledged);
	void __fastcall          hkVoiceData(void* msg);
	void                     PaintTraverse( VPANEL panel, bool repaint, bool force );
	bool                     DoPostScreenSpaceEffects( CViewSetup* setup );
	bool                     CreateMove( float input_sample_time, CUserCmd* cmd );
	void                     LevelInitPostEntity( );
	void                     LevelShutdown( );
	//int                      IN_KeyEvent( int event, int key, const char* bind );
	void                     LevelInitPreEntity( const char* map );
	void                     FrameStageNotify( Stage_t stage );
	void                     UpdateClientSideAnimation( );
    Weapon*                  GetActiveWeapon( );
	bool                     InPrediction( );
	bool                     ShouldDrawParticles( );
	bool                     ShouldDrawFog( );
	void                     OverrideView( CViewSetup* view );
	void                     LockCursor( );
	void                     PlaySound( const char* name );
	void                     OnScreenSizeChanged( int oldwidth, int oldheight );
	void                     RunCommand( Entity* ent, CUserCmd* cmd, IMoveHelper* movehelper );
	int                      SendDatagram( void* data );
	void                     ProcessPacket( void* packet, bool header );
	//void                     GetScreenSize( int& w, int& h );
	void                     SceneEnd( );
	void                     PacketEnd();
	void                     DrawModelExecute( uintptr_t ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone );
	void                     ComputeShadowDepthTextures( const CViewSetup& view, bool unk );
	int                      DebugSpreadGetInt( );
	bool					 NetEarlierTempEntsGetBool();
	bool                     NetShowFragmentsGetBool( );
	bool                     IsConnected( );
	bool                     IsHLTV( );
	bool					 IsPaused();
	void                     EmitSound( IRecipientFilter& filter, int iEntIndex, int iChannel, const char* pSoundEntry, unsigned int nSoundEntryHash, const char* pSample, float flVolume, float flAttenuation, int nSeed, int iFlags, int iPitch, const vec3_t* pOrigin, const vec3_t* pDirection, void* pUtlVecOrigins, bool bUpdatePositions, float soundtime, int speakerentity );
	void                     RenderSmokeOverlay( bool unk );
    void                     RenderView( const CViewSetup &view, const CViewSetup &hud_view, int clear_flags, int what_to_draw );
	void				     OnRenderStart();
	void                     Render2DEffectsPostHUD( const CViewSetup& setup );
	CMatchSessionOnlineHost* GetMatchSession( );
	bool                     OverrideConfig( MaterialSystem_Config_t* config, bool update );
	void __fastcall                     BeginFrame(float ft);
	void                     PostDataUpdate( DataUpdateType_t type );
	bool					 SendNetMsg(INetMessage& msg, bool reliable, bool voice);

	static LRESULT WINAPI WndProc( HWND wnd, uint32_t msg, WPARAM wp, LPARAM lp );



public:
	// vmts.
	VMT m_panel;
	VMT m_client_mode;
	VMT m_client;
	VMT m_client_state;
	VMT m_engine;
	VMT m_engine_sound;
	VMT m_prediction;
	VMT m_surface;
	VMT m_render;
	VMT cl_pred_doresetlatch;
	VMT m_net_channel;
	VMT m_render_view;
	VMT m_model_render;
	VMT m_shadow_mgr;
	VMT m_view_render;
	VMT m_match_framework;
	VMT m_material_system;
	VMT m_fire_bullets;
	VMT m_net_show_fragments;
	VMT m_net_earlier_temp_ents;

	// cvars
	VMT m_debug_spread;

	std::array< VMT, 64 > m_player;
	bool m_updating_bones[65];

	// font.
	DWORD manual_arrows;

	// wndproc old ptr.
	WNDPROC m_old_wndproc;

	bool m_bUpdatingCSA[65];

	// netvar proxies.
	RecvVarProxy_t m_Pitch_original;
	RecvVarProxy_t m_Yaw_original;
	RecvVarProxy_t m_Body_original;
	RecvVarProxy_t m_Force_original;
	RecvVarProxy_t m_AbsYaw_original;
	RecvVarProxy_t m_SimTime_original;
};

// note - dex; these are defined in player.cpp.
class CustomEntityListener : public IEntityListener {
public:
    virtual void OnEntityCreated( Entity *ent );
    virtual void OnEntityDeleted( Entity *ent );

    __forceinline void init( ) {
        g_csgo.AddListenerEntity( this );
    }
};


struct CBaseCSGrenadeProjectile
{
public:
	DWORD* dword0{};
private:
	char gap4[168];
public:
	vec3_t m_local_origin{};
private:
	char gapB8[10476];
public:
	GlowObjectDefinition_t* m_glow_object{};
private:
	char gap29A8[96];
public:
	vec3_t m_last_trail_line_pos{};
	float m_next_trail_line_time{};
};

namespace mhooks {
	void __fastcall ModifyEyePos(CCSGOPlayerAnimState* ecx, void* edx, vec3_t* pos);
	inline decltype(&ModifyEyePos) oModifyEyePos{ };



	bool __fastcall SendWeaponAnim(Weapon* weapon, uint32_t edx, uint32_t act);
	inline decltype(&SendWeaponAnim) oSendWeaponAnim{ };

	void CL_FireEvents();
	inline decltype(&CL_FireEvents) oCL_FireEvents{ };

	void __cdecl InterpolateServerEntities(void);
	inline decltype(&InterpolateServerEntities) oInterpolateServerEntities{ };

	void __fastcall UpdateActivityModifiers(void* rcx);
	inline decltype(&UpdateActivityModifiers) oUpdateActivityModifiers{ };


	int __fastcall RunSimulation(void* ecx, void* edx, int current_command, CUserCmd* cmd, Player* localplayer);
	inline decltype(&RunSimulation) oRunSimulation{ };

	bool __fastcall interpolate(void* ecx, void* edx, float time);
	inline decltype(&interpolate) oInterpolate{ };

	void __fastcall CalcView(void* ecx, const std::uintptr_t edx, vec3_t& eye_origin, const ang_t& eye_ang, float& z_near, float& z_far, float& fov);
	inline decltype(&CalcView) oCalcView{ };

	void __fastcall InitNewParticlesScalar(C_INIT_RandomColor* ecx, void* edx, CParticleCollection* particles, int startPosition, int particlesCount, int attributeWriteMask, void* ctx);
	inline decltype(&InitNewParticlesScalar) oInitNewParticlesScalar{ };

	void __fastcall CreateGrenadeTrail(void* ecx);
	inline decltype(&CreateGrenadeTrail) oCreateGrenadeTrail{ };

	void __fastcall MaintainSeqTrans(void* ecx, void* edx, void* boneSetup, float cycle, vec3_t pos[], quaternion_t q[]);
	inline decltype(&MaintainSeqTrans) oMaintainSeqTrans{ };

	void __fastcall DoExtraBoneProcessing(void* ecx, void* edx, int a2, int a3, int a4, int a5, int a6, int a7);
	inline decltype(&DoExtraBoneProcessing) oDoExtraBoneProcessing{ };


	int __fastcall BaseInterpolatePart1(void* ecx, void* edx, float& curTime, vec3_t& oldOrigin, ang_t& oldAngs, int& noMoreChanges);
	inline decltype(&BaseInterpolatePart1) oBaseInterpolate{ };

	void __fastcall UpdateClientSideAnim(Player* const player, const std::uintptr_t edx);
	inline decltype(&UpdateClientSideAnim) oUpdateClientSideAnim{ };

	void __fastcall BuildTransformations(Player* ecx, void* edx, CStudioHdr* hdr, int a3, int a4, int a5, int a6, int a7);
	inline decltype(&BuildTransformations) oBuildTransformations{ };

	bool __fastcall ShouldSkipAnimationFrame(void* thisPointer, void* edx);
	inline decltype(&ShouldSkipAnimationFrame) oShouldSkipAnimationFrame{ };

	void __fastcall CheckForSequenceChange(void* thisPointer, void* edx, void* hdr, int curSequence, bool forceNewSequence, bool interpolate);
	inline decltype(&CheckForSequenceChange) oCheckForSeqChange{ };

	void __fastcall StandardBlendingRules(Player* const ecx, const std::uintptr_t edx, CStudioHdr* const mdlData, int a1, int a2, float a3, int mask);
	inline decltype(&StandardBlendingRules) oStandardBlendingRules{ };

	void __cdecl ProcessInterpolatedList();
	inline decltype(&ProcessInterpolatedList) oProcessInterpolatedList{ };

	void __fastcall PhysicsSimulate(Player* ecx, void* edx);
	inline decltype(&PhysicsSimulate) oPhysicsSimulate{ };

	void __vectorcall update_animation_state(void* this_pointer, void* unknown, float z, float y, float x, void* unknown1);
	inline decltype(&update_animation_state) original_update_animation_state{ };

	bool __fastcall Teleported(void* ecx, void* edx);
	inline decltype(&Teleported) oTeleported{ };

	void __fastcall AccumulateLayers(Player* const player, const uint32_t edx, void** const setup, vec3_t* const pos, const float time, quaternion_t* const q);
	inline decltype(&AccumulateLayers) oAccumulateLayers{ };

	void __fastcall HkSetVisualsData(void* ecx, void* edx, const char* compostingShaderName);
	inline decltype(&HkSetVisualsData) oSetVisualsData{ };

	bool __fastcall SetupBones(void* ecx, void* edx, matrix3x4_t* out, int max, int mask, float curtime);
	inline decltype(&SetupBones) oSetupBones{ };

	void __stdcall PreEntityPacketReceived(void* pred, int commands_acknowledged, int current_world_update_packet, int server_ticks_elapsed);
	inline decltype(&PreEntityPacketReceived) oPreEntityPacketReceived{ }; 

	void __stdcall PostNetworkDataReceived(void* pred, int commands_acknowledged);
	inline decltype(&PostNetworkDataReceived) oPostNetworkDataReceived{ };
	
	void __stdcall PostEntityPacketReceived(void* pred);
	inline decltype(&PostEntityPacketReceived) oPostEntityPacketReceived;
};


extern Hooks                g_hooks;
extern CustomEntityListener g_custom_entity_listener;