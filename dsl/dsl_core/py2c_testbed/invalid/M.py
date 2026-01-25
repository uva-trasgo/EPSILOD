# wrong array constructor type

from epsilod_ctypes import *

arr: carray[int] = carray("not a number")
