#ifndef __MATHLIB__
#define __MATHLIB__

#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define	Q_PI         	3.1415926535897932384626433832795029f
#define	ON_EPSILON		0.01
#define	EQUAL_EPSILON	0.001f
#define EPSILON_V       0.0001f
#define ANGLE_TO_RAD	0.017453292519943294f
#define RAD_TO_ANGLE	57.29577951308232089f

#define swaps( a, b)		( ( a) ^= ( b) ^= ( a) ^= ( b))
    bool VectorCompare(const vec3_t v1, const vec3_t v2);
    bool QuaternionCompare(const vec4_t v1, const vec4_t v2);

#define Vector(a,b,c,d) {(d)[0]=a;(d)[1]=b;(d)[2]=c;}
#define Vector4(a,b,c,d,target) {(target)[0]=a;(target)[1]=b;(target)[2]=c;(target)[3]=d;}
#define VectorAvg(a) ( ( (a)[0] + (a)[1] + (a)[2] ) / 3 )
#define VectorSubtract(a,b,c) {(c)[0]=(a)[0]-(b)[0];(c)[1]=(a)[1]-(b)[1];(c)[2]=(a)[2]-(b)[2];}
#define VectorSubtractScaled(a,b,c,d) {(c)[0]=(a)[0]-(b)[0]*(d);(c)[1]=(a)[1]-(b)[1]*(d);(c)[2]=(a)[2]-(b)[2]*(d);}
#define VectorAdd(a,b,c) {(c)[0]=(a)[0]+(b)[0];(c)[1]=(a)[1]+(b)[1];(c)[2]=(a)[2]+(b)[2];}
#define VectorAddScaled(a,b,c,d) {(c)[0]=(a)[0]+(b)[0]*(d);(c)[1]=(a)[1]+(b)[1]*(d);(c)[2]=(a)[2]+(b)[2]*(d);}
#define VectorCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];}
#define QuaternionCopy(a,b) {(b)[0]=(a)[0];(b)[1]=(a)[1];(b)[2]=(a)[2];(b)[3]=(a)[3];}
#define VectorScale(a,b,c) {(c)[0]=(b)*(a)[0];(c)[1]=(b)*(a)[1];(c)[2]=(b)*(a)[2];}
#define DotProduct(x,y) ((x)[0]*(y)[0]+(x)[1]*(y)[1]+(x)[2]*(y)[2])
#define VectorFill(a,b) { (a)[0]=(b); (a)[1]=(b); (a)[2]=(b);}

    inline void SETLIMITS(float& VALUE_, const float MAX_, const float MIN_)
    {
        if (VALUE_ > MAX_)
        {
            VALUE_ = MAX_;
        }
        else if (VALUE_ < MIN_)
        {
            VALUE_ = MIN_;
        }
    }

    inline void LInterpolationF(float& fout, const float& f01, const float& f02, const float fWeight)
    {
        fout = f01 + ((f02 - f01) * fWeight);
    }

    void VectorInterpolation(vec3_t& v_out, const vec3_t& v_1, const vec3_t& v_2, const float fWeight);
    void VectorInterpolation_F(vec3_t& v_out,
        const vec3_t& v_1,
        const vec3_t& v_2,
        const float fArea,
        const float fCurrent);
    void VectorInterpolation_W(vec3_t& v_out,
        const vec3_t& v_1,
        const vec3_t& v_2,
        const float fWeight);
    void VectorDistanceInterpolation_F(vec3_t& v_out,
        const vec3_t& v_in,
        const float fRate);
    float VectorDistance3D(const vec3_t& vPosStart, const vec3_t& vPosEnd);
    void VectorDistance3D_Dir(const vec3_t& vPosStart, const vec3_t& vPosEnd, vec3_t& vOut);
    float VectorDistance3D_DirDist(const vec3_t& vPosStart, const vec3_t& vPosEnd, vec3_t& vOut);

    vec_t Q_rint(vec_t in);
    inline float VectorLength(vec3_t v) { return sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); }

    void VectorMul(vec3_t va, vec3_t vb, vec3_t vc);
    void VectorMulF(const vec3_t vIn01, const float fIn01, vec3_t vOut);
    void VectorDivF(const vec3_t vIn01, const float fIn01, vec3_t vOut);
    void VectorDivFSelf(vec3_t vInOut, const float fIn01);
    void VectorDistNormalize(const vec3_t vInFrom, const vec3_t vInTo, vec3_t vOut);
    void VectorMA(vec3_t va, float scale, vec3_t vb, vec3_t vc);

    void CrossProduct(vec3_t v1, vec3_t v2, vec3_t cross);
    vec_t VectorNormalize(vec3_t v);
    void VectorInverse(vec3_t v);

    void ClearBounds(vec3_t mins, vec3_t maxs);
    void AddPointToBounds(vec3_t v, vec3_t mins, vec3_t maxs);

    void AngleMatrix(const vec3_t angles, float matrix[3][4]);
    void AngleIMatrix(const vec3_t angles, float matrix[3][4]);
    void R_ConcatTransforms(const float in1[3][4], const float in2[3][4], float out[3][4]);

    void VectorIRotate(const vec3_t in1, const float in2[3][4], vec3_t out);
    void VectorRotate(const vec3_t in1, const float in2[3][4], vec3_t out);
    void VectorTranslate(const vec3_t in1, const float in2[3][4], vec3_t out);
    void VectorTransform(const vec3_t in1, const float in2[3][4], vec3_t out);

    void AngleQuaternion(const vec3_t angles, vec4_t quaternion);
    void QuaternionMatrix(const vec4_t quaternion, float(*matrix)[4]);
    void QuaternionSlerp(const vec4_t p, vec4_t q, float t, vec4_t qt);

    void FaceNormalize(vec3_t v1, vec3_t v2, vec3_t v3, vec3_t Normal);

    float VectorDistance2D(vec3_t va, vec3_t vb);
#ifdef __cplusplus
}
#endif

#endif
