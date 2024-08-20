#pragma once

class quaternion_t;

namespace math {
    // pi constants.
    constexpr float pi = 3.1415926535897932384f; // pi
    constexpr float pi_2 = pi * 2.f;               // pi * 2
    constexpr float XM_PI = 3.141592654f;
    constexpr float XM_2PI = 6.283185307f;
    constexpr float XM_1DIVPI = 0.318309886f;
    constexpr float XM_1DIV2PI = 0.159154943f;
    constexpr float XM_PIDIV2 = 1.570796327f;
    constexpr float XM_PIDIV4 = 0.785398163f;

    // degrees to radians.
    __forceinline constexpr float deg_to_rad( float val ) {
        return val * ( pi / 180.f );
    }

    // radians to degrees.
    __forceinline constexpr float rad_to_deg( float val ) {
        return val * ( 180.f / pi );
    }

    __forceinline float SmoothStepBounds(float edge0, float edge1, float x) {
        float v1 = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
        return v1 * v1 * (3 - 2 * v1);
    }

    __forceinline float Approach(float target, float value, float speed) {
        float delta = target - value;

        if (delta > speed)
            value += speed;
        else if (delta < -speed)
            value -= speed;
        else
            value = target;

        return value;
    }

    __forceinline float ClampCycle(float flCycleIn) {
        flCycleIn -= int(flCycleIn);

        if (flCycleIn < 0) {
            flCycleIn += 1;
        }
        else if (flCycleIn > 1) {
            flCycleIn -= 1;
        }

        return flCycleIn;
    }

    inline float smoothstep_bounds(float edge0, float edge1, float x)
    {
        x = std::clamp((x - edge0) / (edge1 - edge0), 0.f, 1.f);
        return x * x * (3.f - 2.f * x);
    }

    // angle mod ( shitty normalize ).
    __forceinline float AngleMod( float angle ) {
        return ( 360.f / 65536 ) * ( ( int )( angle * ( 65536.f / 360.f ) ) & 65535 );
    }

    typedef __declspec(align(16)) union {
        float f[4];
        __m128 v;
    } m128;

    inline __m128 sqrt_ps(const __m128 squared) {
        return _mm_sqrt_ps(squared);
    }

    void AngleMatrix( const ang_t& ang, const vec3_t& pos, matrix3x4_t& out );

    inline void ScalarSinCos
    (
        float* pSin,
        float* pCos,
        float  Value
    ) noexcept
    {
        assert(pSin);
        assert(pCos);

        // Map Value to y in [-pi,pi], x = 2*pi*quotient + remainder.
        float quotient = XM_1DIV2PI * Value;
        if (Value >= 0.0f)
        {
            quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
        }
        else
        {
            quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
        }
        float y = Value - XM_2PI * quotient;

        // Map y to [-pi/2,pi/2] with sin(y) = sin(Value).
        float sign;
        if (y > XM_PIDIV2)
        {
            y = XM_PI - y;
            sign = -1.0f;
        }
        else if (y < -XM_PIDIV2)
        {
            y = -XM_PI - y;
            sign = -1.0f;
        }
        else
        {
            sign = +1.0f;
        }

        float y2 = y * y;

        // 11-degree minimax approximation
        *pSin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

        // 10-degree minimax approximation
        float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
        *pCos = sign * p;
    }

    // normalizes an angle.
    void NormalizeAngle( float &angle );

    __forceinline float NormalizedAngle( float angle ) {
        NormalizeAngle( angle );
        return angle;
    }

    float ApproachAngle( float target, float value, float speed );
    void change_matrix_position(matrix3x4_t* bones, size_t msize, vec3_t current_position, vec3_t new_position);
    void VectorAngles( const vec3_t& forward, ang_t& angles, vec3_t* up = nullptr );
    inline void SinCos(float radians, float* sine, float* cosine);
    void matrix_set_column(const vec3_t& in, int column, matrix3x4_t& out);
    vec3_t vector_rotate(const vec3_t& in1, const matrix3x4_t& in2);
    void angle_matrix(const ang_t& angles, const vec3_t& position, matrix3x4_t& matrix);
    void angle_matrix(const ang_t& angles, matrix3x4_t& matrix);
    vec3_t vector_angles(const vec3_t& v);
    vec3_t angle_vectors(const vec3_t& angles);
    vec3_t VectorAmogus(const vec3_t& transform, const matrix3x4_t& matrix);
    void AngleVectors(const ang_t& angles, vec3_t& forward);
    void  AngleVectors( const ang_t& angles, vec3_t* forward, vec3_t* right = nullptr, vec3_t* up = nullptr );
    void AngleVectorKidua(ang_t& vAngle, vec3_t& vForward);
    float GetFOV( const ang_t &view_angles, const vec3_t &start, const vec3_t &end );
    void  VectorTransform( const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out );
    void  VectorITransform( const vec3_t& in, const matrix3x4_t& matrix, vec3_t& out );
    void  MatrixAngles( const matrix3x4_t& matrix, ang_t& angles );
    float AngleDiff(float destAngle, float srcAngle);
    void  MatrixCopy( const matrix3x4_t &in, matrix3x4_t &out );
    vec3_t extrapolate_pos(vec3_t pos, vec3_t extension, int amount, float interval);
    float NormalizeYaw(float angle);
    void  ConcatTransforms( const matrix3x4_t &in1, const matrix3x4_t &in2, matrix3x4_t &out );
    bool IntersectSegmentToSegment(vec3_t s1, vec3_t s2, vec3_t k1, vec3_t k2, float radius);
    bool IntersectionBoundingBox(const vec3_t& start, const vec3_t& dir, const vec3_t& min, const vec3_t& max, vec3_t* hit_point = nullptr);

    // computes the intersection of a ray with a box ( AABB ).
    float SegmentToSegment(const vec3_t s1, const vec3_t s2, const vec3_t k1, const vec3_t k2);
    bool IntersectRayWithBox( const vec3_t &start, const vec3_t &delta, const vec3_t &mins, const vec3_t &maxs, float tolerance, BoxTraceInfo_t *out_info );
    bool IntersectRayWithBox( const vec3_t &start, const vec3_t &delta, const vec3_t &mins, const vec3_t &maxs, float tolerance, CBaseTrace *out_tr, float *fraction_left_solid = nullptr );

    // computes the intersection of a ray with a oriented box ( OBB ).
    bool IntersectRayWithOBB( const vec3_t &start, const vec3_t &delta, const matrix3x4_t &obb_to_world, const vec3_t &mins, const vec3_t &maxs, float tolerance, CBaseTrace *out_tr );
    bool IntersectRayWithOBB( const vec3_t &start, const vec3_t &delta, const vec3_t &box_origin, const ang_t &box_rotation, const vec3_t &mins, const vec3_t &maxs, float tolerance, CBaseTrace *out_tr );

    // returns whether or not there was an intersection of a sphere against an infinitely extending ray. 
    // returns the two intersection points.
    bool IntersectInfiniteRayWithSphere( const vec3_t &start, const vec3_t &delta, const vec3_t &sphere_center, float radius, float *out_t1, float *out_t2 );

    // returns whether or not there was an intersection, also returns the two intersection points ( clamped 0.f to 1.f. ).
    // note: the point of closest approach can be found at the average t value.
    bool IntersectRayWithSphere( const vec3_t &start, const vec3_t &delta, const vec3_t &sphere_center, float radius, float *out_t1, float *out_t2 );

    float DotProduct(const vec3_t& a, const vec3_t& b);

    float DistanceToRay(const vec3_t& pos, const vec3_t& rayStart, const vec3_t& rayEnd, float* along = NULL, vec3_t* pointOnRay = NULL);

	vec3_t Interpolate( const vec3_t from, const vec3_t to, const float percent );
    float normalize(float ang);
    void angle_to_vectors(vec3_t angles, vec3_t& forward);
    vec3_t CalcAngle(const vec3_t& vecSource, const vec3_t& vecDestination);

    void SetMatrixPosition(vec3_t pos, matrix3x4_t& matrix);
    vec3_t GetMatrixPosition(const matrix3x4_t& src);
    void QuaternionMatrix(const quaternion_t& q, const vec3_t& pos, matrix3x4_t& matrix);
    void QuaternionMatrix(const quaternion_t& q, const vec3_t& pos, const vec3_t& vScale, matrix3x4_t& mat);
    void QuaternionMatrix(const quaternion_t& q, matrix3x4_t& matrix);

    template < typename t >
    __forceinline void clamp( t& n, const t& lower, const t& upper ) {
        n = std::max( lower, std::min( n, upper ) );
    }

    // float Lerp( float flPercent, float A, float B );
    template <class T>
    __forceinline T Lerp(float flPercent, T const& A, T const& B)
    {
        return A + (B - A) * flPercent;
    }

    // Similar to Gain() above, but very cheap to call
    // value should be between 0 & 1 inclusive
    inline float SimpleSpline( float value ) {
        float valueSquared = value * value;

        // Nice little ease-in, ease-out spline-like curve
        return ( 3 * valueSquared - 2 * valueSquared * value );
    }

    // remaps a value in [startInterval, startInterval+rangeInterval] from linear to
    // spline using SimpleSpline
    inline float SimpleSplineRemapValClamped( float val, float A, float B, float C, float D ) {
        if( A == B )
            return val >= B ? D : C;
        float cVal = ( val - A ) / ( B - A );
        cVal = std::clamp( cVal, 0.0f, 1.0f );
        return C + ( D - C ) * SimpleSpline( cVal  );
    }

	// mixed types involved.
	template < typename T >
	T Clamp(const T& val, const T& minVal, const T& maxVal) {
		if ((T)val < minVal)
			return minVal;
		else if ((T)val > maxVal)
			return maxVal;
		else
			return val;
	}

    template< class T, class Y >
    FORCEINLINE T clamp2(T const& val, Y const& minVal, Y const& maxVal)
    {
        if (val < (T const&)minVal)
            return (T const&)minVal;
        else if (val > (T const&)maxVal)
            return (T const&)maxVal;
        else
            return val;
    }

    template<class T, class U>
    static T dont_break(const T& in, const U& low, const U& high)
    {

        if (in <= low)
            return low;

        if (in >= high)
            return high;

        return in;
    }
    vec3_t Approach(vec3_t target, vec3_t value, float speed);
    float DistanceBetweenSegments(vec3_t s1, vec3_t s2, vec3_t k1, vec3_t k2);
    bool DoesIntersectCapsule(vec3_t eyePos, vec3_t myDir, vec3_t capsuleA, vec3_t capsuleB, float radius);
    bool IntersectLineWithBB(vec3_t& vStart, vec3_t& vEndDelta, vec3_t& vMin, vec3_t& vMax);
}