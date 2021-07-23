#pragma once
//////////////////////////////////////////////////////////////////////
//
// w_Quaternion.h: interface for the Quaternion class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
	class Quaternion  
	{
	public:
		union
		{
			struct 
			{
				float   fx, fy, fz, fw;
			};
		};
	public:
		Quaternion();
		Quaternion( float x, float y, float z, float w );
		~Quaternion();

		void SetW (float w);
		void SetX (float x);
		void SetY (float y);
		void SetZ (float z);

		float GetW () const;
		float GetX () const;
		float GetY () const;
		float GetZ () const;

		void SetValues(float w, float x, float y, float z);
		void GetValues(float& w, float& x, float& y, float& z) const;

		Quaternion operator+ (const Quaternion& q) const;
		Quaternion operator- (const Quaternion& q) const;
		Quaternion operator- () const;
		Quaternion operator* (const Quaternion& q) const;
		Quaternion operator* (float c) const;
    
		bool operator== (const Quaternion& q) const;
		bool operator!= (const Quaternion& q) const;

		float Dot(const Quaternion& p, const Quaternion& q);
	};

	inline
	Quaternion::Quaternion()
	{
		fx = fy = fz  = fw = 0.0f;
	}

	inline
	Quaternion::~Quaternion()
	{

	}

	inline
	Quaternion::Quaternion( float x, float y, float z, float w )
	{
		fx = x;
		fy = y;
		fz = z;
		fw = w;
	}

	inline 
	void Quaternion::SetW(float w)
	{
		fw = w;
	}

	inline 	
	void Quaternion::SetX(float x)
	{
		fx = x;
	}

	inline 
	void Quaternion::SetY(float y)
	{
		fy = y;
	}

	inline 
	void Quaternion::SetZ(float z)
	{
		fz = z;
	}

	inline 	
	float Quaternion::GetW() const
	{
		return fw;
	}

	inline 
	float Quaternion::GetX() const
	{
		return fx;
	}

	inline 
	float Quaternion::GetY() const
	{
		return fy;
	}

	inline 
	float Quaternion::GetZ() const
	{
		return fz;
	}

	inline 
	void Quaternion::SetValues(float w, float x, float y, float z)
	{
		fw = w;
		fx = x;
		fy = y;
		fz = z;
	}

	inline 
	void Quaternion::GetValues(float& w, float& x, float& y, float& z) const
	{
		w = fw;
		x = fx;
		y = fy;
		z = fz;
	}

	inline
	Quaternion Quaternion::operator+ (const Quaternion& q) const
	{
		return Quaternion( fw + q.fw, fx + q.fx, fy + q.fy, fz + q.fz );
	}

	inline
	Quaternion Quaternion::operator- (const Quaternion& q) const
	{
		return Quaternion( fw - q.fw, fx - q.fx, fy - q.fy, fz - q.fz );
	}

	inline
	Quaternion Quaternion::operator- () const
	{
		return Quaternion( -fw, -fx, -fy, -fz );
	}

	inline
	Quaternion Quaternion::operator* (const Quaternion& q) const
	{
		return Quaternion(
			fw * q.fw - fx * q.fx - fy * q.fy - fz * q.fz,
			fw * q.fx + fx * q.fw + fy * q.fz - fz * q.fy,
			fw * q.fy + fy * q.fw + fz * q.fx - fx * q.fz,
			fw * q.fz + fz * q.fw + fx * q.fy - fy * q.fx
		);
	}

	inline
	Quaternion Quaternion::operator* (float c) const
	{
		return Quaternion( fw * c, fx * c, fy * c, fz * c );
	}

	Quaternion operator* (float c, const Quaternion& q);

	inline 
	bool Quaternion::operator== (const Quaternion& q) const
	{
		return fw == q.fw && fx == q.fx && fy == q.fy && fz == q.fz;
	}

	inline 
	bool Quaternion::operator!= (const Quaternion& q) const
	{
		return fw != q.fw || fx != q.fx || fy != q.fy || fz != q.fz;
	}

	inline 
	float Quaternion::Dot(const Quaternion& p, const Quaternion& q)
	{
		return p.fw * q.fw + p.fx * q.fx + p.fy * q.fy + p.fz * q.fz;
	}
}

#endif //NEW_USER_INTERFACE_UTIL