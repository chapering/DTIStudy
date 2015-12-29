// ----------------------------------------------------------------------------
// point.h : template class definition describing a structure for 2D/3D/4D
//			point or vector
//
// Creation : Feb. 20th 2011
// revision : 
//  @Feb. 21th 
//		fix bug: in calculating vector angle, as the value feeding acos, the 
//		result from dotproduct should be divided by the product of magnitudes of
//		the two vectors, since they are not normalized before.
//	@April. 15th
//		Add operator [] for an array-access syntax
//	@April. 23th
//		Add point to line distance calculation
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _POINT_H_
#define _POINT_H_

#include <iostream>
using std::ostream;

template <class _T = double>
struct _point_t {
	_T x,y,z,w;

	_point_t(_T x = 0, _T y = 0, _T z = 0, _T w = 0) {
		this->x = x, this->y = y, this->z = z, this->w = w;
	}

	_point_t(const _point_t& other) :
		x(other.x), y(other.y),
		z(other.z), w(other.w){
	}

	_point_t& operator = (const _point_t& other) {
		this->x = other.x, this->y = other.y,
		this->z = other.z, this->w = other.w;
		return *this;
	}

	bool operator == (const _point_t& other) const {
		return (this->x == other.x &&
				this->y == other.y &&
				this->z == other.z &&
				this->w == other.w);
	}

	bool operator != (const _point_t& other) {
		return !(*this == other);
	}

	const _T& operator [] (int idx) const {
		assert ( idx >= 0 && idx <= 3 );
		return (0 == idx)?x:(1 == idx)?y:(2 == idx)?z:w;
	}

	_T& operator [] (int idx) {
		assert ( idx >= 0 && idx <= 3 );
		return *((_T*)this +  idx);
	}

	// vector arithmetics
	_point_t operator + (const _point_t& other) const {
		return _point_t(this->x + other.x, this->y + other.y, 
						this->z + other.z, this->w + other.w);
	}

	_point_t operator - (const _point_t& other) const {
		return _point_t(this->x - other.x, this->y - other.y, 
						this->z - other.z, this->w - other.w);
	}

	template <typename _T2>
	_point_t operator * (const _T2& other) const {
		return _point_t(this->x * other, this->y * other, 
						this->z * other, this->w * other);
	}

	void update(_T x = 0, _T y = 0, _T z = 0, _T w = 0) {
		this->x = x, this->y = y, this->z = z, this->w = w;
	}

	_T magnitude() const {
		return sqrt( x*x + y*y + z*z + w*w );
	}

	_point_t normalize(bool updatself = true) {
		_T mag = magnitude();
		_point_t org (*this);
		if ( fabs(mag) > 1e-6 ) {
			org.x /= mag;
			org.y /= mag;
			org.z /= mag;
			org.w /= mag;

			if (updatself) {
				*this = org;
			}
		}
		return org;
	}

	_T distanceTo(const _point_t& other) {
		return (*this - other).magnitude();
	}


	_T dotproduct (const _point_t& other) { // dot product
		return this->x * other.x + this->y * other.y +
						this->z * other.z +  this->w * other.w;
	}

	_point_t crossproduct(const _point_t& other) { // cross product, w ignored
		return _point_t( this->y * other.z - this->z * other.y,
						this->z * other.x - this->x * other.z,
						this->x * other.y - this->y * other.x );
	}

	bool isparallelTo(const _point_t& other) { // is parallel to other, w ignored
		if (fabs(this->x) > 1e-6)  {
			if ( fabs(this->y) > 1e-6 ) {
				if ( fabs(this->z) > 1e-6 ) {
					return other.x/this->x == other.y/this->y && 
						other.y/this->y == other.z/this->z;
				}
				else {
					return fabs(other.z) <= 1e-6 && other.x/this->x == other.y/this->y;
				}
			}
			else {
				if ( fabs(this->z) > 1e-6 ) {
					return fabs(other.y) <= 1e-6 && other.x/this->x == other.z/this->z;
				}
				else {
					return fabs(other.y) <= 1e-6 && fabs(other.z) <= 1e-6;
				}
			}
		}

		// now that fabs(this->x) <= 1e-6
		if ( fabs(this->y) > 1e-6 ) {
			if ( fabs(this->z) > 1e-6 ) {
				return fabs(other.x) <=1e-6 && other.y/this->y == other.z/this->z;
			}
			else {
				return fabs(other.x) <=1e-6 && fabs(other.z) <= 1e-6;
			}
		}
		else {
			return fabs(other.x) <= 1e-6 && fabs(other.y) <= 1e-6;
		}

		// undoable
		return true;
	}

	// is perpendicular to other, w ignored
	bool isperpendicularTo (const _point_t& other) { 
		return 0 == dotproduct(other);
	}

	_T angleTo(const _point_t& other) {
		if (isparallelTo(other)) {
			//return 2 << (sizeof(_T)*8); // mimic infinite
			return 0;
		}
		
		return acos( dotproduct(other) / ( magnitude()*other.magnitude() ) );
	}

	// distance from a point to a straight line in 2D Euclidean space
	_T dist2line(const _point_t& start, const _point_t& end) {
		if ( start.x == end.x ) {
			return fabs(x - start.x);
		}

		if ( start.y == end.y ) {
			return fabs(y - start.y);
		}

		_T a = - (end.y - start.y)/(end.x - start.x), b = 1, c = (-a)*start.x - start.y;
		return fabs(a*x + b*y + c)/sqrt(a*a+b*b);
	}
};

template <class _T>
ostream& operator << (ostream& os, const _point_t<_T>& other) {
	os << "(" << other.x << "," << other.y << "," << other.z << ")\n";
	return os;
}

typedef _point_t<GLdouble> point_t, vector_t;

#endif // _POINT_H_

/* set ts=4 sts=4 tw=80 sw=4 */

