#!/usr/bin/env python
import sys
from math import cos, sin, sqrt,pi,e

class Point:
	def __init__(self, _x=.0, _y=.0, _z=.0):
		self.x = _x
		self.y = _y
		self.z = _z

	def update(self, tp):
		self.x = float(tp[0])
		self.y = float(tp[1])
		self.z = float(tp[2])
		return self
	
	def __repr__(self):
		return "%f %f %f" % (self.x, self.y, self.z)

	def __add__(self, other):
		return Point(self.x + other.x, self.y + other.y, self.z + other.z)

	def __neg__(self):
		return Point(-self.x, -self.y, -self.z)

	def __sub__(self, other):
		return self + (-other)

def PointRotate3D(p1, p2, p0, theta):

    # Translate so axis is at origin    
    p = p0 - p1
    # Initialize point q
    q = Point(0.0,0.0,0.0)
    N = (p2-p1)
    Nm = sqrt(N.x**2 + N.y**2 + N.z**2)
    if Nm == 0:
        Nm = 1
    
    # Rotation axis unit vector
    n = Point(N.x/Nm, N.y/Nm, N.z/Nm)

    # Matrix common factors     
    c = cos(theta)
    t = (1 - cos(theta))
    s = sin(theta)
    X = n.x
    Y = n.y
    Z = n.z

    # Matrix 'M'
    d11 = t*X**2 + c
    d12 = t*X*Y - s*Z
    d13 = t*X*Z + s*Y
    d21 = t*X*Y + s*Z
    d22 = t*Y**2 + c
    d23 = t*Y*Z - s*X
    d31 = t*X*Z - s*Y
    d32 = t*Y*Z + s*X
    d33 = t*Z**2 + c

    #            |p.x|
    # Matrix 'M'*|p.y|
    #            |p.z|
    q.x = d11*p.x + d12*p.y + d13*p.z
    q.y = d21*p.x + d22*p.y + d23*p.z
    q.z = d31*p.x + d32*p.y + d33*p.z

    # Translate axis and rotated point back to original location    
    return q + p1
    
def test_PointRotate3D():
	p0 = Point().update((12.34, 1.45, 9.01))
	p1 = Point()
	p2 = Point(0,1,0)
	theta = 90.0 * pi / 180.0
	print "before rotation: %s" % (p0)
	p0r = PointRotate3D(p1,p2,p0, theta)
	print "after rotation around Y-axis by 90 degree: %s" % (p0r)

if __name__ == '__main__':
    try:
        test_PointRotate3D()
    finally:
        del test_PointRotate3D
