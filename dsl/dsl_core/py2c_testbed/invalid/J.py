# Non constant list
from epsilod_ctypes import *

x: int = 3
# aux_arr: carray[int] = carray([1, 2, 3])
arr: carray[int] = carray([a for a in range(x)])
