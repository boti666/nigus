#pragma once

class studiohdr_t;
struct studiohwdata_t;
struct vcollide_t;
struct virtualmodel_t;
struct vertexFileHeader_t;

namespace OptimizedModel
{
	struct FileHeader_t;
}

typedef unsigned short MDLHandle_t;

enum
{
	MDLHANDLE_INVALID = (MDLHandle_t)~0
};

enum MDLCacheDataType_t
{
	// Callbacks to get called when data is loaded or unloaded for these:
	MDLCACHE_STUDIOHDR = 0,
	MDLCACHE_STUDIOHWDATA,
	MDLCACHE_VCOLLIDE,

	// Callbacks NOT called when data is loaded or unloaded for these:
	MDLCACHE_ANIMBLOCK,
	MDLCACHE_VIRTUALMODEL,
	MDLCACHE_VERTEXES,
	MDLCACHE_DECODEDANIMBLOCK,
};

class IMDLCacheNotify
{
public:
	// Called right after the data is loaded
	virtual void OnDataLoaded(MDLCacheDataType_t type, MDLHandle_t handle) = 0;

	// Called right before the data is unloaded
	virtual void OnDataUnloaded(MDLCacheDataType_t type, MDLHandle_t handle) = 0;
};

enum MDLCacheFlush_t
{
	MDLCACHE_FLUSH_STUDIOHDR = 0x01,
	MDLCACHE_FLUSH_STUDIOHWDATA = 0x02,
	MDLCACHE_FLUSH_VCOLLIDE = 0x04,
	MDLCACHE_FLUSH_ANIMBLOCK = 0x08,
	MDLCACHE_FLUSH_VIRTUALMODEL = 0x10,
	MDLCACHE_FLUSH_AUTOPLAY = 0x20,
	MDLCACHE_FLUSH_VERTEXES = 0x40,

	MDLCACHE_FLUSH_IGNORELOCK = 0x80000000,
	MDLCACHE_FLUSH_ALL = 0xFFFFFFFF
};

class IMDLCache
{
	enum indices : size_t {
		GETVIRTUALMODEL = 19,
		BEGINLOCK = 32,
		ENDLOCK = 33
	};
public:
	__forceinline studiohdr_t* GetStudioHdr(void* handle) {
		return util::get_method < studiohdr_t * (__thiscall*)(void*, void*) >(this, GETVIRTUALMODEL)(this, handle);
	}

	__forceinline void* GetVirtualModel(void* handle) {
		return util::get_method < void* (__thiscall*)(void*, void*) >(this, GETVIRTUALMODEL)(this, handle);
	}

	__forceinline void BeginLock() {
		return util::get_method < void(__thiscall*)(void*) >(this, BEGINLOCK)(this);
	}

	__forceinline void EndLock() {
		return util::get_method < void(__thiscall*)(void*) >(this, ENDLOCK)(this);
	}
};