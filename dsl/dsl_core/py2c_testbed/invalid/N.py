# assignment to carray
from epsilod_ctypes import *

arr: carray[int] = carray([1, 2, 3])
arr2: carray[int] = carray([1, 2, 3])
arr = arr2
