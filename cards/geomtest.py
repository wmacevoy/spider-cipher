#!/usr/bin/env python3

import unittest,math

from geom import Point

class PointTest(unittest.TestCase):

    def testGetX(self):
        assert Point.getX(Point(3,4)) == 3
        assert Point.getX({'x':3,'y':4}) == 3
        assert Point.getX([3,4]) == 3

    def testGetY(self):
        assert Point.getY(Point(3,4)) == 4
        assert Point.getY({'x':3,'y':4}) == 4
        assert Point.getY([3,4]) == 4

    def testGetY(self):
        assert Point.getY(Point(3,4)) == 4
        assert Point.getY({'x':3,'y':4}) == 4
        assert Point.getY([3,4]) == 4
        
    def testGetR(self):
        x = 3
        y = 4
        r = 5
        theta =  53.130102354155978703144387440906589342408428599290442055972032630
        assert abs(Point.getR(Point(3,4)) - r) < 1e-10
        assert abs(Point.getR({'r':r,'theta':theta}) - r) < 1e-10

    def testGetTheta(self):
        x = 3
        y = 4
        r = 5
        theta =  53.130102354155978703144387440906589342408428599290442055972032630
        assert abs(Point.getTheta(Point(3,4)) - theta) < 1e-10
        assert abs(Point.getTheta({'r':r,'theta':theta}) - theta) < 1e-10

    def testPoint1(self):
        x = 3
        y = 4
        p = Point(x,y)
        r = 5
        theta =  53.130102354155978703144387440906589342408428599290442055972032630
        assert p.x == x
        assert p.y == y
        assert abs(p.r - 5.0) < 1e-10
        assert abs(p.theta - theta) < 1e-10

    def testPoint2(self):
        x = -3
        y =  4
        p = Point(x,y)
        r = 5
        theta =  180.0-53.130102354155978703144387440906589342408428599290442055972032630
        assert p.x == x
        assert p.y == y
        assert abs(p.r - 5.0) < 1e-10
        assert abs(p.theta - theta) < 1e-10
        
    def testPoint3(self):
        x = -3
        y = -4
        p = Point(x,y)
        r = 5
        theta = -180+53.130102354155978703144387440906589342408428599290442055972032630
        assert p.x == x
        assert p.y == y
        assert abs(p.r - 5.0) < 1e-10
        assert abs(p.theta - theta) < 1e-10
        
    def testPoint4(self):
        x = 3
        y = -4
        p = Point(x,y)
        r = 5
        theta = -53.130102354155978703144387440906589342408428599290442055972032630
        assert p.x == x
        assert p.y == y
        assert abs(p.r - 5.0) < 1e-10
        assert abs(p.theta - theta) < 1e-10
        
    def testTranslate(self):
        p = Point(3,4).translate(Point(2,-2))
        assert p.x == 5
        assert p.y == 2

    def testScale(self):
        p = Point(3,4).scale(2,Point(5,1))
        assert p.x == 1
        assert p.y == 7

    def testRotate(self):
        p = Point(3,4).rotate(90,Point(5,1))
        ox = 5
        oy = 1
        dx = 3-5
        dy = 4-1
        assert abs(ox - dy - p.x) < 1e-10
        assert abs(oy + dx - p.y) < 1e-10
        
if __name__ == '__main__':
    unittest.main()
    
