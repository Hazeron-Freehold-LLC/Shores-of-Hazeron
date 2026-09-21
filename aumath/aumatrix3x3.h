//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|
// $Id: aumatrix3x3.h,v 1.3 2022/02/19 18:38:01 daveh Exp $
// Copyright (c) 2022 Hazeron Freehold LLC
// All rights reserved.
//-+---+---+---+---+---+---+---+---+---|---+---+---+---+---+---+-==+---+---+---|---+---+---+---+---+---+---+---+---+---+---+---+---|

#ifndef AuMatrix3x3_H
#define AuMatrix3x3_H

#include "aupoint.h"

class AUMATH_DLL AuMatrix3x3
{
public:
	AuPoint v[3];
	inline AuMatrix3x3() { memset(&v[0].y,0,7*sizeof(double)); v[0].x=v[1].y=v[2].z=1;}
	inline ~AuMatrix3x3() {}

	// Rotation sequence: 1. z  2. y  3. x
	inline AuMatrix3x3(const AuPoint &p)
	{
		double CosX,SinX,CosY,SinY,CosZ,SinZ;

		//qDebug("AuMatrix3x3 p.x is %g", p.x);
		CosX = cos(p.x);
		if (fabs(CosX) == 1.0)
			SinX = 0.0;
		else
		{
			SinX = sin(p.x);
			if (fabs(SinX) == 1.0)
				CosX = 0.0;
		}

		CosY = cos(p.y);
		if (fabs(CosY) == 1.0)
			SinY = 0.0;
		else
		{
			SinY = sin(p.y);
			if (fabs(SinY) == 1.0)
				CosY = 0.0;
		}

		CosZ = cos(p.z);
		if (fabs(CosZ) == 1.0)
			SinZ = 0.0;
		else
		{
			SinZ = sin(p.z);
			if (fabs(SinZ) == 1.0)
				CosZ = 0.0;
		}

		double sysz=SinY*SinZ;
		double sycz=SinY*CosZ;

		v[0]=AuPoint(CosY*CosZ,-CosY*SinZ,SinY);
		v[1]=AuPoint(CosX*SinZ+SinX*sycz,CosX*CosZ-SinX*sysz,-SinX*CosY);
		v[2]=AuPoint(SinX*SinZ-CosX*sycz,SinX*CosZ+CosX*sysz,CosX*CosY);
	}

	inline AuMatrix3x3(const double &x,const double &y=0,const double &z=0)
	{ AuMatrix3x3(AuPoint(x,y==0?x:y,z==0?x:z)); }
	inline AuMatrix3x3(const AuPoint &x,const AuPoint &y,const AuPoint &z)
	{ v[0]=x; v[1]=y; v[2]=z; }
	inline AuMatrix3x3 operator*(const double &a) const
	{ return AuMatrix3x3(v[0]*a,v[1]*a,v[2]*a); }

	inline AuMatrix3x3 operator-(const AuMatrix3x3 &a) const
	{ return AuMatrix3x3(v[0]-a.v[0],v[1]-a.v[1],v[2]-a.v[2]); }
	inline AuMatrix3x3 operator+(const AuMatrix3x3 &a) const
	{ return AuMatrix3x3(v[0]+a.v[0],v[1]+a.v[1],v[2]+a.v[2]); }
	inline void operator+=(const AuMatrix3x3 &a)
	{ v[0]+=a.v[0],v[1]+=a.v[1],v[2]+=a.v[2]; }
	inline void operator-=(const AuMatrix3x3 &a)
	{ v[0]-=a.v[0],v[1]-=a.v[1],v[2]-=a.v[2]; }
	inline void operator*=(const AuMatrix3x3 &a)
	{ *this = *this*a; }
	inline void operator^=(const AuMatrix3x3 &a)
	{ *this = a * *this*a; }
	inline void operator/=(const AuMatrix3x3 &a)
	{ *this = *this / a; }
	inline void operator%=(const AuMatrix3x3 &a)
	{ *this = *this % a; }

	inline AuMatrix3x3 operator/(const double &a) const
	{ return AuMatrix3x3(v[0]/a,v[1]/a,v[2]/a); }
	inline AuPoint operator*(const AuPoint &x) const
	{
		return AuPoint(x.Dot(v[0]),x.Dot(v[1]),x.Dot(v[2]));
	}
	inline AuMatrix3x3 operator*(const AuMatrix3x3 &a) const
	{
		AuMatrix3x3 c;

		c.v[0].x=v[0].x*a.v[0].x + v[0].y*a.v[1].x + v[0].z*a.v[2].x;
		c.v[0].y=v[0].x*a.v[0].y + v[0].y*a.v[1].y + v[0].z*a.v[2].y;
		c.v[0].z=v[0].x*a.v[0].z + v[0].y*a.v[1].z + v[0].z*a.v[2].z;

		c.v[1].x=v[1].x*a.v[0].x + v[1].y*a.v[1].x + v[1].z*a.v[2].x;
		c.v[1].y=v[1].x*a.v[0].y + v[1].y*a.v[1].y + v[1].z*a.v[2].y;
		c.v[1].z=v[1].x*a.v[0].z + v[1].y*a.v[1].z + v[1].z*a.v[2].z;

		c.v[2].x=v[2].x*a.v[0].x + v[2].y*a.v[1].x + v[2].z*a.v[2].x;
		c.v[2].y=v[2].x*a.v[0].y + v[2].y*a.v[1].y + v[2].z*a.v[2].y;
		c.v[2].z=v[2].x*a.v[0].z + v[2].y*a.v[1].z + v[2].z*a.v[2].z;

		return c;
	}
	inline AuMatrix3x3 operator^(const AuMatrix3x3 &a) const
	{ return a * *this; }

	inline AuMatrix3x3 operator-() const
	{
		AuMatrix3x3 h;
		h.v[0]=-v[0];
		h.v[1]=-v[1];
		h.v[2]=-v[2];
		return h;
	}

	inline bool operator==(const AuMatrix3x3 &a) const
	{ return v[0]==a.v[0] && v[1]==a.v[1] && v[2]==a.v[2]; }
	inline bool FuzzyEquals(const AuMatrix3x3 &a) const
	{ return v[0].FuzzyEquals(a.v[0]) && v[1].FuzzyEquals(a.v[1]) && v[2].FuzzyEquals(a.v[2]); }
	inline AuMatrix3x3 Transpose() const
	{
		AuMatrix3x3 h;
		h.v[0].x=v[0].x;
		h.v[0].y=v[1].x;
		h.v[0].z=v[2].x;
		h.v[1].x=v[0].y;
		h.v[1].y=v[1].y;
		h.v[1].z=v[2].y;
		h.v[2].x=v[0].z;
		h.v[2].y=v[1].z;
		h.v[2].z=v[2].z;
		return h;
	}

	inline AuMatrix3x3 Inverse() const
	{
		double d=Det();
		if (fabs(d-1)<=FUZZ)
			return Adjoint();
		if (fabs(d)>FUZZ)
			return Adjoint()/d;

		AuMatrix3x3 ident=AuMatrix3x3(AuPoint(1,0,0),AuPoint(0,1,0),AuPoint(0,0,1));
		return ident;
	}

	inline AuMatrix3x3 Adjoint() const
	{
		return AuMatrix3x3(
			AuPoint(v[1].y*v[2].z-v[2].y*v[1].z,v[2].y*v[0].z-v[0].y*v[2].z,v[0].y*v[1].z-v[1].y*v[0].z),
			AuPoint(v[1].z*v[2].x-v[2].z*v[1].x,v[2].z*v[0].x-v[0].z*v[2].x,v[0].z*v[1].x-v[1].z*v[0].x),
			AuPoint(v[1].x*v[2].y-v[2].x*v[1].y,v[2].x*v[0].y-v[0].x*v[2].y,v[0].x*v[1].y-v[1].x*v[0].y));
	}

	// a must be unit matrix - no scaling
	inline AuMatrix3x3 operator/(const AuMatrix3x3 &a) const
	{
		AuMatrix3x3 c;

		c.v[0].x=a.v[0].x*v[0].x + a.v[1].x*v[1].x + a.v[2].x*v[2].x;
		c.v[0].y=a.v[0].x*v[0].y + a.v[1].x*v[1].y + a.v[2].x*v[2].y;
		c.v[0].z=a.v[0].x*v[0].z + a.v[1].x*v[1].z + a.v[2].x*v[2].z;

		c.v[1].x=a.v[0].y*v[0].x + a.v[1].y*v[1].x + a.v[2].y*v[2].x;
		c.v[1].y=a.v[0].y*v[0].y + a.v[1].y*v[1].y + a.v[2].y*v[2].y;
		c.v[1].z=a.v[0].y*v[0].z + a.v[1].y*v[1].z + a.v[2].y*v[2].z;

		c.v[2].x=a.v[0].z*v[0].x + a.v[1].z*v[1].x + a.v[2].z*v[2].x;
		c.v[2].y=a.v[0].z*v[0].y + a.v[1].z*v[1].y + a.v[2].z*v[2].y;
		c.v[2].z=a.v[0].z*v[0].z + a.v[1].z*v[1].z + a.v[2].z*v[2].z;

		return c;
	}
	inline AuMatrix3x3 operator%(const AuMatrix3x3 &a) const // a may have scaling, rotation, skew, etc.
		{ return a.Inverse() * *this; };
	inline void RotateX(const double &x)
	{
		if (x==0) return;
		double c=cos(x);
		double s=sin(x);
		if (fabs(s)==1.)
			c=0;
		else if (fabs(c)==1.)
			s=0;
		AuPoint vv=v[1]*c-v[2]*s;
		v[2]=v[1]*s+v[2]*c;
		v[1]=vv;
	}
	inline void RotateY(const double &y)
	{
		if (y==0) return;
		double c=cos(y);
		double s=sin(y);
		if (fabs(s)==1.)
			c=0;
		else if (fabs(c)==1.)
			s=0;
		AuPoint vv=v[2]*c-v[0]*s;
		v[0]=v[2]*s+v[0]*c;
		v[2]=vv;

	}
	inline void RotateZ(const double &z)
	{
		if (z==0) return;
		double c=cos(z);
		double s=sin(z);
		if (fabs(s)==1.)
			c=0;
		else if (fabs(c)==1.)
			s=0;
		AuPoint vv=v[0]*c-v[1]*s;
		v[1]=v[0]*s+v[1]*c;
		v[0]=vv;
	}
	inline AuPoint GetRotation() const
	{
		AuPoint tem;
		double l=v[0].Length();
		if (l==0)
			return AuPoint(0,0,0);
		if (fabs(v[0].z)<l)
		{
			tem.y=asin(v[0].z/l);
			if (fabs(v[0].z)<=FUZZ)
				tem.y=0;
			if (v[0].x<0 && v[2].z<0)
			{
				if (tem.y<0)
					tem.y=-PI-tem.y;
				else
					tem.y=PI-tem.y;
				tem.x=atan2(v[1].z,-v[2].z);
				tem.z=atan2(v[0].y,-v[0].x);
			}
			else
			{
				tem.x=atan2(-v[1].z,v[2].z);
				tem.z=atan2(-v[0].y,v[0].x);
			}
			if (fabs(tem.z)<=FUZZ)
				tem.z=0;
			else if (tem.z<=-PI)
				tem.z+=PI+PI;
		}
		else
		{
			tem.x=atan2(v[2].y,v[1].y);
			tem.y= v[0].z>0 ? PI*.5 : -PI*.5;
			tem.z=0.;
		}
		if (fabs(tem.x)<=FUZZ)
			tem.x=0;
		else if (tem.x<=-PI)
			tem.x+=PI+PI;
		return tem;
	}

	inline double Det() const { return v[0].Dot(v[1].Cross(v[2])); };
	inline double GetScale() const
	{
		return v[0].Length();
	}
	inline AuPoint GetScales() const
	{
		return AuPoint(v[0].Length(),v[1].Length(),v[2].Length());
	}
	inline bool IsUnit() const
	{
		return fabs(GetScale()-1)<=FUZZ;
	}
	inline bool IsUniform() const
	{
		AuPoint l=GetScales();
		return fabs(l.x-l.y)<=FUZZ && fabs(l.x-l.y)<=FUZZ;
	}
	inline AuMatrix3x3 Unit() const
	{
		double s=GetScale();
		if (fabs(s)<=FUZZ)
		{
			AuMatrix3x3 ident =
				AuMatrix3x3(AuPoint(1,0,0),AuPoint(0,1,0),AuPoint(0,0,1));
			return ident;
		}
		if (fabs(s-1)>FUZZ)
			return *this/s;
		return *this;
	}
	/*
	inline AuMatrix3x3 Decompose() const
	{
		AuMatrix3x3 q,r,h,b;
		double c,s;

		h.v[0].z=atan2(v[1].x,v[0].x);
		if (fabs(h.v[0].z)>FUZZ)
		{
			c=cos(h.v[0].z);
			s=sin(h.v[0].z);
			r=AuMatrix3x3(AuPoint(c,s,0),AuPoint(-s,c,0),AuPoint(0,0,1));
			b=r* *this;
		}
		else
		{
			h.v[0].z=0;
			r=AuMatrix3x3(AuPoint(1,0,0),AuPoint(0,1,0),AuPoint(0,0,1));
			b=*this;
		}

		h.v[0].y=atan2(b.v[2].x,b.v[0].x);
		if (fabs(h.v[0].y)>FUZZ)
		{
			c=cos(h.v[0].y);
			s=sin(h.v[0].y);
			r=AuMatrix3x3(AuPoint(c,0,s),AuPoint(0,1,0),AuPoint(-s,0,c))*r;
			b=r* *this;
		}
		else
			h.v[0].y=0;
		h.v[0].x=atan2(b.v[2].y,b.v[1].y);
		if (fabs(h.v[0].x)>FUZZ)
		{
			c=cos(h.v[0].x);
			s=sin(h.v[0].x);
			r=AuMatrix3x3(AuPoint(1,0,0),AuPoint(0,c,s),AuPoint(0,-s,c))*r;
			b=r* *this;
		}
		else
			h.v[0].x=0.;
		h.v[0]=r.Transpose().GetRotation();
		h.v[1]=AuPoint(defuzz(b.v[0].y),defuzz(b.v[0].z),defuzz(b.v[1].z));
		h.v[2]=AuPoint(defuzz(b.v[0].x-1),defuzz(b.v[1].y-1),defuzz(b.v[2].z-1));
		return h;
	}
	*/

	inline AuMatrix3x3 Recompose() const
	{
		AuPoint p(v[2].x+1, v[1].x, v[1].y);
		AuMatrix3x3 h(p, AuPoint(0,v[2].y+1,v[1].z), AuPoint(0,0,v[2].z+1));
		h.RotateZ(v[0].z);
		h.RotateY(v[0].y);
		h.RotateX(v[0].x);
		return h;
	}

private:
	/*
	inline static double defuzz(double x)
	{
			double s;
			if (x<0)
					return -defuzz(-x);
			if (x<FUZZ)
					return 0;
			s=x;

#ifndef _M_M68K
			static long fac[2]={0x8000l,0x3ff00000l};
			while (*((short*)&s+1)==-1)
					s*=*(double*)fac;
			if (*((short*)&s+1))
					return x;
			*(short*)&s=0;
#else
			static long fac[2]={0x3ff00000l,0x8000l};
			while (*((short*)&s+2)==-1)
					s*=*(double*)fac;
			if (*((short*)&s+2))
					return x;
			*((short*)&s+3)=0;
#endif // _M_M68K
			return s;
	}
	*/
};

#endif // AuMatrix3x3_H
