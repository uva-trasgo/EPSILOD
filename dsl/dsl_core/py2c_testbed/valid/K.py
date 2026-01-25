# in place ops (basic and carrys)
from epsilod_ctypes import *

a: int = 5
b: int = 2
arr: carray[int] = carray(4)

arr[0] = 1
a += b
arr[0] += a
