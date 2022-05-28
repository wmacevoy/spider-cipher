#!/usr/bin/env python3

import unittest

from maybe import Maybe

class P:
    @property
    def x(self): return 'px'
    @property
    def y(self): return 'py'

p=P()
q={'x':'qx','y':'qy'}
r=['rx','ry']
s={'a':1,'b':2}

x = lambda arg : (Maybe(lambda arg : arg.x)|Maybe(lambda arg : arg['x'])|Maybe(lambda arg : arg[0])).so(arg)
y = lambda arg : (Maybe(lambda arg : arg.y)|Maybe(lambda arg : arg['y'])|Maybe(lambda arg : arg[1])).so(arg)

class MaybeTest(unittest.TestCase):
    def testX(self):

        assert x(p)=='px'
        assert x(q)=='qx'
        assert x(r)=='rx'
    def testY(self):
        assert y(p)=='py'
        assert y(q)=='qy'
        assert y(r)=='ry'
if __name__ == '__main__':
    unittest.main()
    
