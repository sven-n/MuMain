#pragma once
//////////////////////////////////////////////////////////////////////
//
// w_Vector3D.h: interface for the Vector3D class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
	class Vector3D
	{
	public:
		union
		{
			struct 
			{
				float x, y, z;
			};
			
			float m[3];
		};
		
	public:
		Vector3D ();
		Vector3D (float fX, float fY, float fZ);

		const float& operator[] (int i) const;
		float& operator[] (int i);

		bool operator== (const Vector3D& pt) const;
		bool operator!= (const Vector3D& pt) const;
	};

	inline
	Vector3D::Vector3D() : x(0.0f), y(0.0f), z(0.0f)
	{
	}

	inline
	Vector3D::Vector3D(float fX, float fY, float fZ)
	{
		x = fX; y = fY; z = fZ;
	}

	inline
    const float& Vector3D::operator[] (int i) const
    {
        const float* base = &x;
        return (float&) base[i];
    }

	inline
    float& Vector3D::operator[] (int i)
    {
        float* base = &x;
        return (float&) base[i];
    }

	inline
	bool Vector3D::operator== (const Vector3D& pt) const
	{
		return (x == pt.x && y == pt.y && z == pt.z);
	}

	inline
    bool Vector3D::operator!= (const Vector3D& pt) const 
	{
		return !(*this == pt); 
	}

	};
};

#endif //NEW_USER_INTERFACE_UTIL