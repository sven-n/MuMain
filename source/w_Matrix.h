#pragma once
//////////////////////////////////////////////////////////////////////
//
// w_Matrix.h: interface for the Matrix class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UTIL

namespace util
{
	class Vector3D;
	class Matrix
	{
	public:
		union
		{
			struct
			{
				Vector3D xa;
				Vector3D ya;
				Vector3D za;
			};

			struct
			{
				float _11, _12, _13;
			    float _21, _22, _23; 
			    float _31, _32, _33; 
			};

			struct
			{
				float   xx, yx, zx;
				float   xy, yy, zy;
				float   xz, yz, zz;
			};

			float m[3][3];
		};

	public:
		Matrix();

		Matrix( float _11, float _12, float _13, 
			    float _21, float _22, float _23, 
				float _31, float _32, float _33 );

		Matrix( const Vector3D& ixa, const Vector3D& iya, const Vector3D& izd );

		~Matrix();

		bool operator==(const Matrix& mat) const;
		bool operator!=(const Matrix& mat) const { return !(*this == mat); }

		Matrix operator+(const Matrix& mat) const
		{
			Matrix result = *this;
			result.m[0][0] += mat.m[0][0]; result.m[0][1] += mat.m[0][1];
			result.m[0][2] += mat.m[0][2];

			result.m[1][0] += mat.m[1][0]; result.m[1][1] += mat.m[1][1];
			result.m[1][2] += mat.m[1][2];

			result.m[2][0] += mat.m[2][0]; result.m[2][1] += mat.m[2][1];
			result.m[2][2] += mat.m[2][2];
			return result;
		}

		Matrix operator-(const Matrix& mat) const
		{
			Matrix result = *this;
			result.m[0][0] -= mat.m[0][0]; result.m[0][1] -= mat.m[0][1];
			result.m[0][2] -= mat.m[0][2];

			result.m[1][0] -= mat.m[1][0]; result.m[1][1] -= mat.m[1][1];
			result.m[1][2] -= mat.m[1][2];

			result.m[2][0] -= mat.m[2][0]; result.m[2][1] -= mat.m[2][1];
			result.m[2][2] -= mat.m[2][2];

			return result;
		}

		Matrix operator*(const Matrix& mat) const
		{
			Matrix		prd;

			prd.m[0][0] = m[0][0]*mat.m[0][0]+m[0][1]*mat.m[1][0]+m[0][2]*mat.m[2][0]+m[0][3]*mat.m[3][0];
			prd.m[1][0] = m[1][0]*mat.m[0][0]+m[1][1]*mat.m[1][0]+m[1][2]*mat.m[2][0]+m[1][3]*mat.m[3][0];
			prd.m[2][0] = m[2][0]*mat.m[0][0]+m[2][1]*mat.m[1][0]+m[2][2]*mat.m[2][0]+m[2][3]*mat.m[3][0];

			prd.m[0][1] = m[0][0]*mat.m[0][1]+m[0][1]*mat.m[1][1]+m[0][2]*mat.m[2][1]+m[0][3]*mat.m[3][1];
			prd.m[1][1] = m[1][0]*mat.m[0][1]+m[1][1]*mat.m[1][1]+m[1][2]*mat.m[2][1]+m[1][3]*mat.m[3][1];
			prd.m[2][1] = m[2][0]*mat.m[0][1]+m[2][1]*mat.m[1][1]+m[2][2]*mat.m[2][1]+m[2][3]*mat.m[3][1];

			prd.m[0][2] = m[0][0]*mat.m[0][2]+m[0][1]*mat.m[1][2]+m[0][2]*mat.m[2][2]+m[0][3]*mat.m[3][2];
			prd.m[1][2] = m[1][0]*mat.m[0][2]+m[1][1]*mat.m[1][2]+m[1][2]*mat.m[2][2]+m[1][3]*mat.m[3][2];
			prd.m[2][2] = m[2][0]*mat.m[0][2]+m[2][1]*mat.m[1][2]+m[2][2]*mat.m[2][2]+m[2][3]*mat.m[3][2];
			return prd;
		}

		Matrix Matrix::operator* (float fScalar) const
		{
			Matrix result;
			result.m[0][0] = m[0][0] * fScalar;
			result.m[0][1] = m[0][1] * fScalar;
			result.m[0][2] = m[0][2] * fScalar;

			result.m[1][0] = m[1][0] * fScalar;
			result.m[1][1] = m[1][1] * fScalar;
			result.m[1][2] = m[1][2] * fScalar;

			result.m[2][0] = m[2][0] * fScalar;
			result.m[2][1] = m[2][1] * fScalar;
			result.m[2][2] = m[2][2] * fScalar;
			return result;
		}

		void GetRow( unsigned int uiRow, Vector3D& row ) const;

		void SetRow( unsigned int uiRow, const Vector3D& row );

		void Identity()
		{
			m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f;
			m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
			m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
		}

		void Diagonal(float fDiag0, float fDiag1, float fDiag2)
		{
			m[0][0] = fDiag0; m[0][1] = 0.0f; m[0][2] = 0.0f;
			m[1][0] = 0.0f; m[1][1] = fDiag1; m[1][2] = 0.0f;
			m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = fDiag2;
		}

		void XRotation(float fAngle)
		{
			float sn, cs;
			SinCos(fAngle, sn, cs);
    
			m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[1][0] = 0.0f;
			m[1][1] = cs; m[1][2] = sn; m[2][0] = 0.0f;
			m[2][1] = -sn; m[2][2] = cs;
		}

		void YRotation(float fAngle)
		{
			float sn, cs;
			SinCos(fAngle, sn, cs);
    
			m[0][0] = cs; m[0][1] = 0.0f; m[0][2] = -sn;
			m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f;
			m[2][0] = sn; m[2][1] = 0.0f; m[2][2] = cs;
		}

		void ZRotation(float fAngle)
		{
			float sn, cs;
			SinCos(fAngle, sn, cs);

			m[0][0] = cs; m[0][1] = sn; m[0][2] = 0.0f;
			m[1][0] = -sn; m[1][1] = cs; m[1][2] = 0.0f;
			m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f;
		}

		void Rotation(float fAngle, float x, float y, float z)
		{
			float sn, cs;
			SinCos(fAngle, sn, cs);

			float omcs = 1.0f-cs;
			float x2 = x*x;
			float y2 = y*y;
			float z2 = z*z;
			float xym = x*y*omcs;
			float xzm = x*z*omcs;
			float yzm = y*z*omcs;
			float xsin = x*sn;
			float ysin = y*sn;
			float zsin = z*sn;
    
			m[0][0] = x2*omcs+cs;
			m[0][1] = xym+zsin;
			m[0][2] = xzm-ysin;
			m[1][0] = xym-zsin;
			m[1][1] = y2*omcs+cs;
			m[1][2] = yzm+xsin;
			m[2][0] = xzm+ysin;
			m[2][1] = yzm-xsin;
			m[2][2] = z2*omcs+cs;
		}


		bool Matrix::Inverse (Matrix& inv) const
		{
			inv.m[0][0] = m[1][1]*m[2][2]-
				m[1][2]*m[2][1];
			inv.m[0][1] = m[0][2]*m[2][1]-
				m[0][1]*m[2][2];
			inv.m[0][2] = m[0][1]*m[1][2]-
				m[0][2]*m[1][1];
			inv.m[1][0] = m[1][2]*m[2][0]-
				m[1][0]*m[2][2];
			inv.m[1][1] = m[0][0]*m[2][2]-
				m[0][2]*m[2][0];
			inv.m[1][2] = m[0][2]*m[1][0]-
				m[0][0]*m[1][2];
			inv.m[2][0] = m[1][0]*m[2][1]-
				m[1][1]*m[2][0];
			inv.m[2][1] = m[0][1]*m[2][0]-
				m[0][0]*m[2][1];
			inv.m[2][2] = m[0][0]*m[1][1]-
				m[0][1]*m[1][0];

			float fDet = m[0][0]*inv.m[0][0]+
				m[0][1]*inv.m[1][0]+
				m[0][2]*inv.m[2][0];
			if ( abs(fDet) <= 1e-06f )
				return false;

			float fInvdet = 1.0f/fDet;
			for (int row = 0; row < 3; row++)
			{
				for (int col = 0; col < 3; col++)
				{
					inv.m[row][col] *= fInvdet;
				}
			}

			return true;
		}

		Matrix Inverse () const
		{
			Matrix inv;
    
			if ( Inverse(inv) == false )
				inv.Identity( );

			return inv;
		}

		Matrix Transpose () const
		{
			Matrix row[3];

			GetRow( 0, row[0] );
			GetRow( 1, row[1] );
			GetRow( 2, row[2] );

			return Matrix( row[0], row[1], row[2] );
		}
	};

	inline
	Matrix::Matrix()
	{

	}

	inline
	Matrix::Matrix( float i_11, float i_12, float i_13, float i_14, 
					float i_21, float i_22, float i_23, float i_24, 
					float i_31, float i_32, float i_33, float i_34, 
					float i_41, float i_42, float i_43, float i_44 ) : _11(i_11), _12(i_12), _13(i_13),
																	   _21(i_21), _22(i_22), _23(i_23),
																	   _31(i_31), _32(i_32), _33(i_33),
	{

	}

	inline
	Matrix::Matrix( const Vector3D& ixa, float ixd, 
			const Vector3D& iya, float iyd, 
			const Vector3D& iza, float izd, 
			const Vector3D& ipivot, float ipd )	: xa(ixa), ya(iya), za(iza)
	{

	}

	inline
	Matrix::~Matrix()
	{

	}

	inline
	bool Matrix::operator!=(const Matrix& mat) const 
	{ 
		return !(*this == mat); 
	}

	inline 
	void Matrix::GetRow( unsigned int uiRow, Vector3D& row ) const
	{
		assert( uiRow <= 2 );
		row.x   = m[uiRow][0];
		row.y   = m[uiRow][1];
		row.z   = m[uiRow][2];
	}

	inline 
	void Matrix::SetRow( unsigned int uiRow, const Vector3D& row )
	{
		assert( uiRow <= 2 );
		m[uiRow][0]  = row.x;
		m[uiRow][1]  = row.y;
		m[uiRow][2]  = row.z;
	}
};

#endif //NEW_USER_INTERFACE_UTIL
