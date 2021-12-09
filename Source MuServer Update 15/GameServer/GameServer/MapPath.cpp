// MapPath.cpp: implementation of the CMapPath class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapPath.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMapPath::CMapPath() // OK
{
	this->m_PathDir[0] = -1;
	this->m_PathDir[1] = -1;
	this->m_PathDir[2] = 0;
	this->m_PathDir[3] = -1;
	this->m_PathDir[4] = 1;
	this->m_PathDir[5] = -1;
	this->m_PathDir[6] = 1;
	this->m_PathDir[7] = 0;
	this->m_PathDir[8] = 1;
	this->m_PathDir[9] = 1;
	this->m_PathDir[10] = 0;
	this->m_PathDir[11] = 1;
	this->m_PathDir[12] = -1;
	this->m_PathDir[13] = 1;
	this->m_PathDir[14] = -1;
	this->m_PathDir[15] = 0;
}

CMapPath::~CMapPath() // OK
{

}

int CMapPath::GetDist(int x1,int y1,int x2,int y2) // OK
{
	int distx = x2 - x1;
	int disty = y2 - y1;
	return (distx * distx) + (disty * disty);
}

int CMapPath::VerifyThatOnPath(int x,int y) // OK
{
	for ( int i = 0 ; i < this->m_NumPath ; i++ )
	{
		if ( x ==  this->m_PathX[i] && y == this->m_PathY[i] )
		{
			return i;
		}
	}
	return -1;
}

BOOL CMapPath::CanWeMoveForward(int x,int y) // OK
{
	int which = x+ y * this->m_width  ;

	if ( x >= this->m_width || x < 0 || y >= this->m_height || y < 0 )
	{
		return false;
	}

	if ( this->m_MapAttr[which] > 1 || this->m_HitAttr[which] != 0 )
	{
		this->m_HitAttr[which] = 1;
		return false;
	}
	return TRUE;
}

BOOL CMapPath::CanWeMoveForward2(int x,int y) // OK
{
	int which = x +  y* this->m_width ;

	if ( x >= this->m_width || x < 0 || y >= this->m_height || y < 0 )
	{
		return false;
	}

	if ( this->m_MapAttr[which] >= 1 || this->m_HitAttr[which] != 0 )
	{
		this->m_HitAttr[which] = 1;
		return false;
	}
	return TRUE;
}

BOOL CMapPath::CanWeMoveForward3(int x,int y) // OK
{
	int which = x +  y* this->m_width ;

	if ( x >= this->m_width || x < 0 || y >= this->m_height || y < 0 )
	{
		return false;
	}

	if ( this->m_MapAttr[which] >= 1 || this->m_HitAttr[which] != 0 )
	{
		this->m_HitAttr[which] = 1;
		return false;
	}
	return TRUE;
}

BOOL CMapPath::IsThisSpotOK(int x,int y) // OK
{
	int pos;

	pos = x+ y * this->m_width ;

	if ( x < 0 || x >= this->m_width || y < 0 || y >= this->m_height )
	{
		return false;
	}
	if ( this->m_HitAttr[pos] != 0 )
	{
		return false;
	}
	return true;
}

int CMapPath::FindNextDir(int sx,int sy,int dx,int dy,int dirstart,bool First,bool ErrorCheck) // OK
{
	int MinDist = 10000000;
	long ldir;
	int WhichDir = 0;
	int i;

	if ( First != 0 )
	{
		for ( i = 0 ; i<8 ; i++ )
		{
			ldir = i % 8;
			int endx = sx + this->m_PathDir[ldir*2];
			int endy = sy + this->m_PathDir[ldir*2+1];
			int dist = this->GetDist(endx, endy, dx, dy);

			if ( MinDist > dist )
			{
				if ( this->IsThisSpotOK(endx, endy) != FALSE )
				{
					MinDist = dist;
					WhichDir = ldir;
				}
			}
		}
	}
	else
	{
		for ( i = dirstart+7 ; i <= dirstart+9 ; i++ )
		{
			ldir = i % 8;
			int endx = sx + this->m_PathDir[ldir*2];
			int endy = sy + this->m_PathDir[ldir*2+1];
			int dist = this->GetDist(endx, endy, dx, dy);

			if ( MinDist > dist )
			{
				if ( this->IsThisSpotOK(endx, endy) != FALSE )
				{
					MinDist = dist;
					WhichDir = ldir;
				}
			}
		}

		if ( MinDist == 10000000 )
		{
			for ( i = dirstart+2 ; i <= dirstart+6 ; i++ )
			{
				ldir = i % 8;
				int endx = sx + this->m_PathDir[ldir*2];
				int endy = sy + this->m_PathDir[ldir*2+1];
				int dist = this->GetDist(endx, endy, dx, dy);

				if ( MinDist > dist )
				{
					if ( this->IsThisSpotOK(endx, endy) != FALSE )
					{
						MinDist = dist;
						WhichDir = ldir;
					}
				}
			}
		}
	}
	int LastDist;

	this->m_LastDir = dirstart;
	LastDist = this->GetDist(sx, sy, sx + this->m_PathDir[this->m_LastDir*2], sy + this->m_PathDir[this->m_LastDir*2+1]);

	if ( ( LastDist > MinDist - 6 ) && ( LastDist < MinDist + 6 ) && ( MinDist >= 25 ) )
	{
		WhichDir = this->m_LastDir;
	}

	if ( MinDist == 10000000 )
	{
		return -1;
	}

	if ( this->CanWeMoveForward(sx+this->m_PathDir[((WhichDir*2))], sy+this->m_PathDir[((WhichDir*2))+1]) != FALSE )
	{
		int path = this->VerifyThatOnPath(sx, sy);

		if ( path != -1 )
		{
			this->m_HitAttr[sx + sy * this->m_width ] = 1;
			this->m_NumPath = path;
		}

		return WhichDir;
	}

	if ( ErrorCheck != 0 )
	{
		int dir = WhichDir+7;

		dir %= 8;

		if (this->CanWeMoveForward(sx + this->m_PathDir[dir*2], sy + this->m_PathDir[dir*2+1]) != FALSE )
		{
			int path = this->VerifyThatOnPath(sx, sy);

			if ( path != -1 )
			{
				this->m_HitAttr[sx  + sy * this->m_width] = 1;
				this->m_NumPath = path;
			}
			return dir;
		}

		int dir2 = WhichDir+9;
		dir2 %= 8;
		if (CanWeMoveForward(sx+this->m_PathDir[dir2*2], sy+this->m_PathDir[dir2*2+1]) != FALSE )
		{
			int path = this->VerifyThatOnPath(sx, sy);

			if ( path != -1 )
			{
				this->m_HitAttr[sx + sy * this->m_width] = 1;
				this->m_NumPath = path;
			}
			return dir2;
		}
	}

	this->m_HitAttr[sx + sy*this->m_width]=1;
	return -1;
}

int CMapPath::FindNextDir2(int sx,int sy,int dx,int dy,int dirstart,bool First,bool ErrorCheck) // OK
{
	int MinDist = 10000000;
	long ldir;
	int WhichDir = 0;
	int i;

	if ( First != 0 )
	{
		for ( i = 0 ; i<8 ; i++ )
		{
			ldir = i % 8 ;
			int endx = sx + this->m_PathDir[ldir*2];
			int endy = sy + this->m_PathDir[ldir*2+1];
			int dist = this->GetDist(endx, endy, dx, dy);

			if ( MinDist > dist )
			{
				if ( this->IsThisSpotOK(endx, endy) != FALSE )
				{
					MinDist = dist;
					WhichDir = ldir;
				}
			}
		}
	}
	else
	{
		for ( i = dirstart+7 ; i <= dirstart+9 ; i++ )
		{
			ldir = i % 8;
			int endx = sx + this->m_PathDir[ldir*2];
			int endy = sy + this->m_PathDir[ldir*2+1];
			int dist = this->GetDist(endx, endy, dx, dy);

			if ( MinDist > dist )
			{
				if ( this->IsThisSpotOK(endx, endy) != FALSE )
				{
					MinDist = dist;
					WhichDir = ldir;
				}
			}
		}

		if ( MinDist == 10000000 )
		{
			for ( i = dirstart+2 ; i <= dirstart+6 ; i++ )
			{
				ldir = i % 8;
				int endx = sx + this->m_PathDir[ldir*2];
				int endy = sy + this->m_PathDir[ldir*2+1];
				int dist = this->GetDist(endx, endy, dx, dy);

				if ( MinDist > dist )
				{
					if ( this->IsThisSpotOK(endx, endy) != FALSE )
					{
						MinDist = dist;
						WhichDir = ldir;
					}
				}
			}
		}
	}
	int LastDist;

	this->m_LastDir = dirstart;
	LastDist = this->GetDist(sx, sy, sx + this->m_PathDir[this->m_LastDir*2], sy + this->m_PathDir[this->m_LastDir*2+1]);

	if ( ( LastDist > MinDist - 6 ) && ( LastDist < MinDist + 6 ) && ( MinDist >= 25 ) )
	{
		WhichDir = this->m_LastDir;
	}

	if ( MinDist == 10000000 )
	{
		return -1;
	}

	if ( this->CanWeMoveForward2(sx+this->m_PathDir[WhichDir*2],sy+ this->m_PathDir[WhichDir*2+1]) != FALSE )
	{
		int path = this->VerifyThatOnPath(sx, sy);

		if ( path != -1 )
		{
			this->m_HitAttr[sx+ sy * this->m_width ] = 1;
			this->m_NumPath = path;
		}

		return WhichDir;
	}

	if ( ErrorCheck != 0 )
	{
		int dir = WhichDir+7;

		dir %= 8;

		if (this->CanWeMoveForward2(sx + this->m_PathDir[dir*2], sy + this->m_PathDir[dir*2+1]) != FALSE )
		{
			int path = this->VerifyThatOnPath(sx, sy);

			if ( path != -1 )
			{
				this->m_HitAttr[sx+ sy * this->m_width] = 1;
				this->m_NumPath = path;
			}
			return dir;
		}

		int dir2 = WhichDir+9;
		dir2 %= 8;

		if (this->CanWeMoveForward2(sx+this->m_PathDir[dir2*2], sy+this->m_PathDir[dir2*2+1]) != FALSE )
		{
			int path = this->VerifyThatOnPath(sx, sy);

			if ( path != -1 )
			{
				this->m_HitAttr[sx + sy * this->m_width] = 1;
				this->m_NumPath = path;
			}
			return dir2;
		}
	}

	this->m_HitAttr[sx + sy*this->m_width]=1;
	return -1;
}

int CMapPath::FindNextDir3(int sx,int sy,int dx,int dy,int dirstart,bool First,bool ErrorCheck) // OK
{
	int MinDist = 10000000;
	long ldir;
	int WhichDir = 0;
	int i;

	if ( First != 0 )
	{
		for ( i = 0 ; i<8 ; i++ )
		{
			ldir = i % 8 ;
			int endx = sx + this->m_PathDir[ldir*2];
			int endy = sy + this->m_PathDir[ldir*2+1];
			int dist = this->GetDist(endx, endy, dx, dy);

			if ( MinDist > dist )
			{
				if ( this->IsThisSpotOK(endx, endy) != FALSE && this->CanWeMoveForward3(endx, endy) != FALSE)
				{
					MinDist = dist;
					WhichDir = ldir;
				}
			}
		}
	}
	else
	{
		for ( i = dirstart+7 ; i <= dirstart+9 ; i++ )
		{
			ldir = i % 8;
			int endx = sx + this->m_PathDir[ldir*2];
			int endy = sy + this->m_PathDir[ldir*2+1];
			int dist = this->GetDist(endx, endy, dx, dy);

			if ( MinDist > dist )
			{
				if ( this->IsThisSpotOK(endx, endy) != FALSE && this->CanWeMoveForward3(endx, endy) != FALSE)
				{
					MinDist = dist;
					WhichDir = ldir;
				}
			}
		}

		if ( MinDist == 10000000 )
		{
			for ( i = dirstart+2 ; i <= dirstart+6 ; i++ )
			{
				ldir = i % 8;
				int endx = sx + this->m_PathDir[ldir*2];
				int endy = sy + this->m_PathDir[ldir*2+1];
				int dist = this->GetDist(endx, endy, dx, dy);

				if ( MinDist > dist )
				{
					if ( this->IsThisSpotOK(endx, endy) != FALSE && this->CanWeMoveForward3(endx, endy) != FALSE)
					{
						MinDist = dist;
						WhichDir = ldir;
					}
				}
			}
		}
	}

	if ( MinDist == 10000000 )
	{
		return -1;
	}

	if ( this->CanWeMoveForward3(sx+this->m_PathDir[WhichDir*2],sy+ this->m_PathDir[WhichDir*2+1]) != FALSE )
	{
		int path = this->VerifyThatOnPath(sx, sy);

		if ( path != -1 )
		{
			this->m_HitAttr[sx+ sy * this->m_width ] = 1;
			this->m_NumPath = path;
		}

		return WhichDir;
	}

	if ( ErrorCheck != 0 )
	{
		int dir = WhichDir+7;

		dir %= 8;

		if (this->CanWeMoveForward3(sx + this->m_PathDir[dir*2], sy + this->m_PathDir[dir*2+1]) != FALSE )
		{
			int path = this->VerifyThatOnPath(sx, sy);

			if ( path != -1 )
			{
				this->m_HitAttr[sx+ sy * this->m_width] = 1;
				this->m_NumPath = path;
			}
			return dir;
		}

		int dir2 = WhichDir+9;
		dir2 %= 8;

		if (this->CanWeMoveForward3(sx+this->m_PathDir[dir2*2], sy+this->m_PathDir[dir2*2+1]) != FALSE )
		{
			int path = this->VerifyThatOnPath(sx, sy);

			if ( path != -1 )
			{
				this->m_HitAttr[sx + sy * this->m_width] = 1;
				this->m_NumPath = path;
			}
			return dir2;
		}
	}

	this->m_HitAttr[sx + sy*this->m_width]=1;
	return -1;
}

bool CMapPath::FindPath(int startx,int starty,int endx,int endy,bool ErrorCheck) // OK
{
	this->m_NumPath = 0;

	int WhichDir = 0;
	int NumFails = 0;
	bool First;


	memset(this->m_HitAttr, 0, this->m_width * this->m_height);
	First = true;

	while ( (startx != endx) || (starty != endy) )
	{
		WhichDir = this->FindNextDir( startx, starty, endx, endy, WhichDir, First, ErrorCheck);
		First = 0;

		if ( WhichDir >= 0 )
		{
			this->m_PathX[this->m_NumPath] = startx;
			this->m_PathY[this->m_NumPath] = starty;
			startx += this->m_PathDir[WhichDir*2];
			starty += this->m_PathDir[WhichDir*2+1];
			this->m_NumPath++;

			if ( this->m_NumPath >= 15 )
			{
				return false;
			}
		}
		else if ( ErrorCheck != false )
		{
			this->m_NumPath--;

			if ( this->m_NumPath < 0 )
			{
				return false;
			}

			startx = this->m_PathX[this->m_NumPath];
			starty = this->m_PathY[this->m_NumPath];
			NumFails++;

			if ( NumFails >= 10 )
			{
				return false;
			}
		}
		else
		{
			break;
		}

	}

	this->m_PathX[this->m_NumPath] = startx;
	this->m_PathY[this->m_NumPath] = starty;
	this->m_NumPath++;

	if ( this->m_NumPath >= 15 )
	{
		return false;
	}

	return true;
}

bool CMapPath::FindPath2(int startx,int starty,int endx,int endy,bool ErrorCheck) // OK
{
	this->m_NumPath = 0;

	int WhichDir=0;
	int NumFails=0;
	bool First;


	memset(this->m_HitAttr, 0, this->m_width * this->m_height);
	First = true;

	while ( (startx != endx) || (starty != endy) )
	{
		WhichDir = this->FindNextDir2(startx, starty, endx, endy, WhichDir, First, ErrorCheck);
		First = 0;

		if ( WhichDir >= 0 )
		{
			this->m_PathX[this->m_NumPath] = startx;
			this->m_PathY[this->m_NumPath] = starty;
			startx += this->m_PathDir[WhichDir*2];
			starty += this->m_PathDir[WhichDir*2+1];
			this->m_NumPath++;

			if ( this->m_NumPath >= 14 )
			{
				this->m_NumPath--;
				break;
			}
		}
		else if ( ErrorCheck != FALSE )
		{
			this->m_NumPath--;

			if ( this->m_NumPath < 0 )
			{
				return false;
			}

			startx = this->m_PathX[this->m_NumPath];
			starty = this->m_PathY[this->m_NumPath];
			NumFails++;

			if ( NumFails >= 10 )
			{
				return false;
			}
		}
		else
		{
			break;
		}

	}

	this->m_PathX[this->m_NumPath] = startx;
	this->m_PathY[this->m_NumPath] = starty;
	this->m_NumPath++;

	if ( this->m_NumPath >= 15 )
	{
		return false;
	}

	return true;
}

bool CMapPath::FindPath3(int startx,int starty,int endx,int endy,bool ErrorCheck) // OK
{
	this->m_NumPath = 0;

	int WhichDir=0;
	int NumFails=0;
	bool First;


	memset(this->m_HitAttr, 0, this->m_width * this->m_height);
	First = true;

	while ( (startx != endx) || (starty != endy) )
	{
		WhichDir = this->FindNextDir3(startx, starty, endx, endy, WhichDir, First, ErrorCheck);
		First = 0;

		if ( WhichDir >= 0 )
		{
			this->m_PathX[this->m_NumPath] = startx;
			this->m_PathY[this->m_NumPath] = starty;
			startx += this->m_PathDir[WhichDir*2];
			starty += this->m_PathDir[WhichDir*2+1];
			this->m_NumPath++;

			if ( this->m_NumPath >= 14 )
			{
				this->m_NumPath--;
				break;
			}
		}
		else if ( ErrorCheck != FALSE )
		{
			this->m_NumPath--;

			if ( this->m_NumPath < 0 )
			{
				return false;
			}

			startx = this->m_PathX[this->m_NumPath];
			starty = this->m_PathY[this->m_NumPath];
			NumFails++;

			if ( NumFails >= 10 )
			{
				return false;
			}
		}
		else
		{
			break;
		}

	}

	this->m_PathX[this->m_NumPath] = startx;
	this->m_PathY[this->m_NumPath] = starty;
	this->m_NumPath++;

	if ( this->m_NumPath >= 15 )
	{
		return false;
	}

	return true;
}

void CMapPath::SetMapDimensions(int width,int height,BYTE* MapAttr) // OK
{
	this->m_width = width;

	this->m_height = height;

	this->m_MapAttr = MapAttr;

	this->m_HitAttr = new BYTE[this->m_width*this->m_height];
}

int CMapPath::GetPath() // OK
{
	return this->m_NumPath;
}

BYTE* CMapPath::GetPathX() // OK
{
	return this->m_PathX;
}

BYTE* CMapPath::GetPathY() // OK
{
	return this->m_PathY;
}
