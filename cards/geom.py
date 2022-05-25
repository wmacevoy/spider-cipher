import math
from maybe import Maybe

class Geom:
    def __init__(self):
        pass
    def copy(self):
        raise ValueError("unsupported")
    def scale(self,factor,origin):
        raise ValueError("unsuported")
    def rotate(self,anticlockwise,origin):
        raise ValueError("unsupported")
    def translate(self,offset):
        raise ValueError("unsupported")

class Point(Geom):
    def __init__(self,x,y):
        Geom.__init__(self)

        self._x = float(x)
        self._y = float(y)

    @staticmethod
    def getX(p):
        return (Maybe(lambda arg : arg.x)|Maybe(lambda arg : arg['x'])|Maybe(lambda arg : arg[0])).so(p)

    @staticmethod
    def getY(p):
        return (Maybe(lambda arg : arg.y)|Maybe(lambda arg : arg['y'])|Maybe(lambda arg : arg[1])).so(p)

    @staticmethod
    def getR(p):
        return (Maybe(lambda arg : arg.r)|Maybe(lambda arg : arg['r'])).so(p)

    @staticmethod
    def getTheta(p):
        return (Maybe(lambda arg : arg.theta)|Maybe(lambda arg : arg['theta'])).so(p)

    @staticmethod
    def cartesian(x,y):
        return Point(x,y)

    @staticmethod    
    def polar(r,theta):
        return Point(r*math.cos(math.radians(theta)),r*math.sin(math.radians(theta)))

    @property
    def x(self):
        return self._x
    
    @property
    def y(self):
        return self._y
    
    @property
    def r(self):
        return math.sqrt(self._x*self._x+self._y*self._y)

    @property
    def theta(self):
        return math.degrees(math.atan2(self._y,self._x))
    
    def __add__(self,rhs):
        return Point(self._x+rhs._x,self._y+rhs._y)

    def __sub__(self,rhs):
        return Point(self._x-rhs._x,self._y-rhs._y)

    def copy(self):
        return Point(self._x,self._y)

    def translate(self,p):
        return Point(self._x+p.x,self._y+p.y)

    def scale(self,factor,origin=None):
        ox = (Maybe(lambda arg : Point.getX(arg)) | Maybe(lambda arg : 0.0)).so(origin)
        oy = (Maybe(lambda arg : Point.getY(arg)) | Maybe(lambda arg : 0.0)).so(origin)

        fx = (Maybe(lambda arg : Point.getX(arg))|Maybe(lambda arg : float(arg))).so(factor)
        fy = (Maybe(lambda arg : Point.getY(arg))|Maybe(lambda arg : float(arg))).so(factor)
        
        dx=self._x - ox
        dy=self._y - oy
        
        return Point(ox+fx*dx,oy+fy*dy)

    def rotate(self,anticlockwise,origin=None):
        ox = (Maybe(lambda arg : Point.getX(arg)) | Maybe(lambda arg : 0.0)).so(origin)
        oy = (Maybe(lambda arg : Point.getY(arg)) | Maybe(lambda arg : 0.0)).so(origin)
        
        c=math.cos(math.radians(anticlockwise))
        s=math.sin(math.radians(anticlockwise))

        dx=self._x - ox
        dy=self._y - oy

        return Point(ox+c*dx-s*dy,oy+s*dx+c*dy)

    def __str__(self):
        return "(" + str(self._x) + "," + str(self._y) + ")"

    
