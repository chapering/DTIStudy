// ----------------------------------------------------------------------------
// colorscheme.h : class declaration and definition of coloring schemes
// supported as yet
//
// this is the template class definition describing implementations of
// different coloring scheme, it is to describe, in actuality, a set of mappings
// that convert a tuple symbolizing a geometric point to another tuple that will
// tell the color corresponding to that point
//
// Creation : Feb. 27th 2011
// revision : 
//	@Feb. 28th 
//		1.the CLSCHM_ANATOMY_SYMMETRIC coloring scheme is improved to discern the
//		color encoded on the Y axis and that on the Z axis
//		2.CLSCHM_ANATOMY_SYMMETRIC mapping but in Lab color space is added as
//		another choice of coloring
//	@Mar. 7th
//		.Add Hue Ball coloring
//	@Mar. 8th
//		.Add the Heuristic orientation color encoding
//  @Mar. 9th
//		.Reconstitute coloring schemes, with useless ones removed
//	@Mar. 14th
//		.Add FA coloring scheme
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _COLORSCHEME_H_
#define _COLORSCHEME_H_

#include "glrand.h"

#include <iostream>
#include <map>

using std::ostream;
using std::map;
using std::max;
using std::min;

typedef enum _colorscheme_t {
#ifdef _TEST_COLORING_
	CLSCHM_COORDINATE_MAPPING = 0,
	CLSCHM_YZ_WATERSHED,
	CLSCHM_ANATOMY_SYMMETRIC_LAB,
	CLSCHM_HEURISTIC_MIRROR,
#endif
	CLSCHM_ALL_SYMMETRIC = 0,
	CLSCHM_ANATOMY_SYMMETRIC,
	CLSCHM_HEURISTIC,
	CLSCHM_ANATOMY_HEURISTIC,
	CLSCHM_HUE_BALL,
	CLSCHM_FA,
	CLSCHM_CUSTOM
}colorscheme_t;

const char* const g_colorschemes[] = {
#ifdef _TEST_COLORING_
	"direct coordinate mapping",
	"YZ watershed",
	"anatomy symmetric in Lab space",
	"heuristic with mirror symmetry",
#endif
	"all symmetric",
	"anatomy symmetric",
	"heuristic",
	"anatomy heuristic",
	"hue ball",
	"FA coloring",
	"custom" // vacuous scheme
};

const GLfloat g_rgb2xyz_cie_mat[16] = {
	0.4887180, 0.3106803,  0.2006017, 0.0,
	0.1762044, 0.8129847,  0.0108109, 0.0,
	0.0000000, 0.0102048,  0.9897952, 0.0,
	0.0000000, 0.0000000,  0.0000000, 0.0
};

const GLfloat g_xyz2lab_epsilon = 0.008856;
const GLfloat g_xyz2lab_k = 903.3;

template <class _T = double>
class CColorMapper {
public:

public:
	CColorMapper(int nScheme = CLSCHM_ALL_SYMMETRIC);
	~CColorMapper();

	int setColorScheme(int nScheme);
	const char* getName() const;

	// do the mapping
	// @return 0 for success and other values for nothing done
	int getColor(const _T& x, const _T& y, const _T& z,
				_T& r, _T& g, _T& b) const;

private:
	// pinpoint a coloring scheme
	int			m_colorschemeIdx;
};

// just to be used as factory functions
inline int getNumberOfColorSchemes()
{
	return ARRAY_SIZE(g_colorschemes);
}

inline int isColorSchemeSupported(int nScheme)
{
	return nScheme >= 0 && 
		nScheme <= (ARRAY_SIZE(g_colorschemes)-1);
}

inline const char* getColorSchemeName(int nScheme)
{
	if ( !isColorSchemeSupported(nScheme) ) {
		return "~~N/A~~";
	}

	return g_colorschemes[nScheme];
}

template <typename _T>
void rgb2xyz(const _T& r, const _T& g, const _T& b,
			_T& x, _T& y, _T& z)
{
	_T w = 0.0;
	x = r, y = g, z = b;
	transpoint(g_rgb2xyz_cie_mat, x, y, z, w);
}

template <typename _T>
void xyz2lab(const _T& x, const _T& y, const _T& z,
			_T& l, _T& a, _T& b)
{
	_T fx, fy, fz;
	fx = x > g_xyz2lab_epsilon ? (_T)powf( (float)x, 1.0/3.0 ) :
	   (g_xyz2lab_k*x + 16)/116.0;
	fy = y > g_xyz2lab_epsilon ? (_T)powf( (float)y, 1.0/3.0 ) :
	   (g_xyz2lab_k*y + 16)/116.0;
	fz = z > g_xyz2lab_epsilon ? (_T)powf( (float)z, 1.0/3.0 ) :
	   (g_xyz2lab_k*z + 16)/116.0;

	l = 116*fy - 16;
	a = 500 * (fx - fy);
	b = 200 * (fy - fz);
}

template <typename _T>
void rgb2lab(const _T& r, const _T& g, const _T& b,
			_T& l, _T& a, _T& b1)
{
	_T x = 0, y = 0, z = 0;
	rgb2xyz(r,g,b,x,y,z);
	xyz2lab(x,y,z,l,a,b1);
}


template <typename _T>
void hsv2rgb(const _T& H, const _T& S, const _T& V,
			_T& r, _T& g, _T& b)
{
	_T C = V*S;
	_T X = C*(1 - abs((int)H % 2 - 1)), m = V - C;

	if ( H >= 0 && H < 1 ) {
		r = C, g = X, b = 0;
	}
	else if ( H >= 1 && H < 2 ) {
		r = X, g = C, b = 0;
	}
	else if ( H >= 2 && H < 3 ) {
		r = 0, g = C, b = X;
	}
	else if ( H >= 3 && H < 4 ) {
		r = 0, g = X, b = C;
	}
	else if ( H >= 4 && H < 5 ) {
		r = X, g = 0, b = C;
	}
	else if ( H >= 5 && H < 6 ) {
		r = C, g = 0, b = X;
	}
	else {
		r = g = b = 0;
	}

	r += m, g += m, b += m;
}

//////////////////////////////////////////////////////////////////////////////
//
// in-place template class implementation
//
template <class _T>
CColorMapper<_T>::CColorMapper(int nScheme) :
	m_colorschemeIdx(nScheme) 
{
}

template <class _T>
CColorMapper<_T>::~CColorMapper()
{
}

template <class _T>
int CColorMapper<_T>::setColorScheme(int nScheme)
{
	if ( !isColorSchemeSupported(nScheme) ) {
		return -1;
	}

	m_colorschemeIdx = nScheme;
	return 0;
}

template <class _T>
const char* CColorMapper<_T>::getName() const
{
	return getColorSchemeName(m_colorschemeIdx);
}

template <class _T>
int CColorMapper<_T>::getColor(const _T& x, const _T& y, const _T& z,
				_T& r, _T& g, _T& b) const
{
	// these are intuitive mapping from cartesian space to RGB color space
	// from x-axis to the r-axis, y to g and z to b.
	r = x, g = y, b = z;
	normalize(r, g, b);
	
	switch (m_colorschemeIdx) {
#ifdef _TEST_COLORING_
		case CLSCHM_COORDINATE_MAPPING:
			/* coloring scheme -1.*/
			break;
		case CLSCHM_YZ_WATERSHED:
			{
				/* coloring scheme -2.*/
				r = fabs(r);
				g = g>0?g:(1.0+g);
				b = b>0?b:(1.0+b);
			}
			break;
		case CLSCHM_ANATOMY_SYMMETRIC_LAB:
			{
				/* coloring scheme -3.*/
				r = fabs(r);
				g = (g+1.0)/2.0;
				b = 1- (b+1.0)/2.0;

				_T l,a,b1;
				rgb2lab(r,g,b,l,a,b1);
				r = l, g = a, b = b1;
				normalize(r, g, b);
				r = fabs(r);
				g = (g+1.0)/2.0;
				b = 1- (b+1.0)/2.0;
			}
			break;
		case CLSCHM_HEURISTIC_MIRROR:
			{
				_T Pe = .5, Pb = 0.15/Pe, Le = .65, Pc = .5, Ps = .5, 
				   Pbeta=.75, Beta = .4, Gamma = 2.2;

				r = fabs(r), g = g, b = b;
				_T radius = sqrt(r*r+g*g+b*b),
					theta = acos(g/radius),
					fi = (M_PI+atan2(b,r))*(180/M_PI),
					fir = (M_PI+atan2(0,r))*(180/M_PI);
				_T H = fi - fir + M_PI;
				if ( H >= M_PI ) H -= M_PI;
				H *= 2;
				_T S = sin(Ps*theta) / sin(Ps*M_PI/2.0), V = 1.0;

				hsv2rgb(H, S, V, r, g, b);

				_T bi = b/(r+g+b), Cb = max(1.5*Pb*(bi-1.0/3.0)*Pc, .0);
				r = Cb*b + (1-Cb)*r;
				g = Cb*b + (1-Cb)*g;
				b = b;

				_T Aw = powf(0.3*r + 0.59*g + 0.11*b, .4);
				_T c1 = 1.0/3.0 - Pe/25.0, c2 = 1.0/3.0 + Pe/4.0, 
				   c3 = 1 - c1 - c2;

				_T Lm = max(r, max(g, b));

				_T Fl = min( _T((c1*r + c2*g + c3*b)/powf(Le, 1.0/Beta)), _T(1.0)),
				   Lf = Pc*Fl + (1 - Pc)*Lm;

			   //_T Fmax = 255;
			   r = powf( powf(Aw, Pbeta) * (r/Lf), 1.0/Gamma );
			   g = powf( powf(Aw, Pbeta) * (g/Lf), 1.0/Gamma );
			   b = powf( powf(Aw, Pbeta) * (b/Lf), 1.0/Gamma );
			}
			break;
#endif
		case CLSCHM_ALL_SYMMETRIC:
			{
				/* coloring scheme 0.*/
				r = fabs(r);
				g = fabs(g);
				b = fabs(b);
			}
			break;
		case CLSCHM_HEURISTIC:
			{
				/* coloring scheme 1.*/
				_T Pe = .5, Pb = 0.15/Pe, Le = .71, Pc = .5, 
				   Pbeta=.75, Beta = .4, Gamma = 2.2;

				r = fabs(r), g = fabs(g), b = fabs(b);

				_T bi = b/(r+g+b), Cb = max(1.5*Pb*(bi-1.0/3.0)*Pc, .0);
				r = Cb*b + (1-Cb)*r;
				g = Cb*b + (1-Cb)*g;
				b = b;

				_T Aw = powf(0.3*r + 0.59*g + 0.11*b, .4);
				_T c1 = 1.0/3.0 - Pe/25.0, c2 = 1.0/3.0 + Pe/4.0, 
				   c3 = 1 - c1 - c2;

				_T Lm = max(r, max(g, b));

				_T Fl = min( _T((c1*r + c2*g + c3*b)/powf(Le, 1.0/Beta)), _T(1.0)),
				   Lf = Pc*Fl + (1 - Pc)*Lm;

			   //_T Fmax = 255;
			   r = powf( powf(Aw, Pbeta) * (r/Lf), 1.0/Gamma );
			   g = powf( powf(Aw, Pbeta) * (g/Lf), 1.0/Gamma );
			   b = powf( powf(Aw, Pbeta) * (b/Lf), 1.0/Gamma );
			}
			break;
		case CLSCHM_ANATOMY_HEURISTIC:
			{
				/* coloring scheme 1.*/
				_T Pe = .5, Pb = 0.15/Pe, Le = .71, Pc = .5, 
				   Pbeta=.75, Beta = .4, Gamma = 2.2;

				r = fabs(r), g = (1+g)/2.0, b = (1+b)/2.0;

				_T bi = b/(r+g+b), Cb = max(1.5*Pb*(bi-1.0/3.0)*Pc, .0);
				r = Cb*b + (1-Cb)*r;
				g = Cb*b + (1-Cb)*g;
				b = b;

				_T Aw = powf(0.3*r + 0.59*g + 0.11*b, .4);
				_T c1 = 1.0/3.0 - Pe/25.0, c2 = 1.0/3.0 + Pe/4.0, 
				   c3 = 1 - c1 - c2;

				_T Lm = max(r, max(g, b));

				_T Fl = min( _T((c1*r + c2*g + c3*b)/powf(Le, 1.0/Beta)), _T(1.0)),
				   Lf = Pc*Fl + (1 - Pc)*Lm;

			   //_T Fmax = 255;
			   r = powf( powf(Aw, Pbeta) * (r/Lf), 1.0/Gamma );
			   g = powf( powf(Aw, Pbeta) * (g/Lf), 1.0/Gamma );
			   b = powf( powf(Aw, Pbeta) * (b/Lf), 1.0/Gamma );
			}
			break;
		case CLSCHM_ANATOMY_SYMMETRIC:
			{
				/* coloring scheme 2.*/
				r = fabs(r);
				g = (g+1.0)/2.0;
				b = (b+1.0)/2.0;
				//b = 1- (b+1.0)/2.0;
			}
			break;
		case CLSCHM_HUE_BALL:
			{
				/* coloring scheme 3.
				 * Reference: http://en.wikipedia.org/wiki/HSL_and_HSV
				 */
				/*
				_T colors[16][3] = {
						{255,255,255},
						{255,255,0},
						{255,0,255},
						{255,0,0},
						{192,192,192},
						{128,128,128},
						{128,128,0},
						{128,0,128},
						{128,0,0},
						{0,255,255},
						{0,255,0},
						{0,128,128},
						{0,128,0},
						{0,0,255},
						{0,0,128},
						{0,0,0}
				};*/
				/*
				b = (M_PI+atan2(z,fabs(x)))*(180/M_PI);
				int idx = floor(b / (360.0/16)) - 1;
				r = colors[idx][0]/255.0;
				g = colors[idx][1]/255.0;
				b = colors[idx][2]/255.0;
				*/

				_T //M = max( fabs(r), max(fabs(g), fabs(b)) ),
				   //C = M - min( fabs(r), min(fabs(g),fabs(b)) ),
				   V = sqrt(r*r+g*g+b*b);
				   //V = M;
				r = sqrt(x*x+y*y+z*z);
				g = acos(y/r);
				b = (M_PI+atan2(z,fabs(x)))*(180/M_PI);

				_T H = b/60.0f, 
				   //S = g / M_PI;
				   S = sin( .5*g ) / sin(.5*(M_PI/2.0));
				   //S = C==0?0:C/V;
				hsv2rgb(H, S, V, r, g, b);
			}
			break;
		case CLSCHM_FA:
			{
				/*
				r = (2+g)/3.0;
				if ( r > 1e-6 ) {
					g = b = 0.0;
				}
				else {
					r = g = b = 1.0;
				}
				*/
				r = 1;
				g = b = 1 - y;
			}
			break;
		case CLSCHM_CUSTOM:
			// the caller will implement a custom scheme itself
			r = 1.0;//0.92;//0.7;
			g = 1.0;//0.58;//0.7;
			b = 1.0;//0.47;//0.7;
			return 1;
		default:
			cerr << "Unrecognized color encoding.\n";
			return -1;
	}
	return 0;
}

#endif // _COLORSCHEME_H_

/* set ts=4 sts=4 tw=80 sw=4 */

