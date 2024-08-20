#include "includes.h"

struct CIKTarget {
    int m_iFrameCount;

private:
    char pad_00004[0x51];
};

void* CIKContext::operator new(size_t size)
{
    CIKContext* ptr = (CIKContext*)g_csgo.m_mem_alloc->Alloc(size);
    Construct(ptr);

    return ptr;
}

void CIKContext::operator delete(void* ptr)
{
    g_csgo.m_mem_alloc->Free(ptr);
}

void CIKContext::Construct(CIKContext* ik)
{
    typedef void(__thiscall* IKConstruct)(CIKContext*);
    static auto ik_ctor = pattern::find(g_csgo.m_client_dll, "56 8B F1 0F 57 C0 C7 86 ?? ?? ?? ?? ?? ?? ?? ??").as< IKConstruct>();

    ik_ctor(ik);
}

void CIKContext::Destructor()
{
    typedef void(__thiscall* IKDestructor)(CIKContext*);
    auto ik_dector = pattern::find(g_csgo.m_client_dll, "56 8B F1 57 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 8D 8E ?? ?? ?? ?? E8 ?? ?? ?? ?? 83 BE ?? ?? ?? ?? ??").as< IKDestructor>();
    ik_dector(this);
}

// This somehow got inlined so we need to rebuild it
void CIKContext::ClearTargets()
{
    int m_iTargetCount = *(int*)((uintptr_t)this + 0xFF0);
    auto m_pIkTarget = (CIKTarget*)((uintptr_t)this + 0xD0);
    for (int i = 0; i < m_iTargetCount; i++) {
        m_pIkTarget->m_iFrameCount = -9999;
        m_pIkTarget++;
    }
}

void CIKContext::Init(CStudioHdr* hdr, ang_t& angles, vec3_t& origin, float currentTime, int frames, int boneMask)
{
    typedef void(__thiscall* Init_t)(void*, CStudioHdr*, ang_t&, vec3_t&, float, int, int);
    static auto ik_init = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 EC 08 8B 45 08 56 57 8B F9 8D").as<Init_t>();

    ik_init(this, hdr, angles, origin, currentTime, frames, boneMask);
}

void CIKContext::UpdateTargets(vec3_t* pos, quaternion_t* qua, matrix3x4_t* matrix, uint8_t* boneComputed)
{
    typedef void(__thiscall* UpdateTargets_t)(void*, vec3_t*, quaternion_t*, matrix3x4_t*, uint8_t*);
    static auto  ik_update_targets = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F0 81 EC ?? ?? ?? ?? 33 D2").as<UpdateTargets_t>();
    ik_update_targets(this, pos, qua, matrix, boneComputed);
}

void CIKContext::SolveDependencies(vec3_t* pos, quaternion_t* qua, matrix3x4_t* matrix, uint8_t* boneComputed)
{
    typedef void(__thiscall* SolveDependencies_t)(void*, vec3_t*, quaternion_t*, matrix3x4_t*, uint8_t*);
    static auto  ik_solve_dependencies = pattern::find(g_csgo.m_client_dll, "55 8B EC 83 E4 F0 81 EC ?? ?? ?? ?? 8B 81").as< SolveDependencies_t>();
    ik_solve_dependencies(this, pos, qua, matrix, boneComputed);
}

void CIKContext::CopyTo(CIKContext* other, const unsigned short* iRemapping)
{
    using CopyToFn = void(__thiscall*)(CIKContext*, CIKContext*, const unsigned short*);
    static auto copy_to = pattern::find(g_csgo.m_server_dll, "55 8B EC 83 EC 24 8B 45 08 57 8B F9 89 7D F4 85 C0").as<CopyToFn>();

    copy_to(this, other, iRemapping);
}