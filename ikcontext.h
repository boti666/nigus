#pragma once

class CIKContext {
    // Not sure of the correct size, also don't care
    uint8_t pad[0x1070];

public:
    void* operator new(size_t size);
    void  operator delete(void* ptr);

    static void Construct(CIKContext* ik);
    void Destructor();

    void ClearTargets();
    void Init(CStudioHdr* hdr, ang_t& angles, vec3_t& origin, float currentTime, int frames, int boneMask);
    void UpdateTargets(vec3_t* pos, quaternion_t* qua, matrix3x4_t* matrix, uint8_t* boneComputed);
    void SolveDependencies(vec3_t* pos, quaternion_t* qua, matrix3x4_t* matrix, uint8_t* boneComputed);
    void CopyTo(CIKContext* other, const unsigned short* iRemapping);
};