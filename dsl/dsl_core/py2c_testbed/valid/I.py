# Using obj.arr[i] in assignment and expression
from epsilod_ctypes import *

# obj.arr: carray[int] = carray([1, 2, 3, 4])
x: int = obj.arr[2]
obj.arr[1] = x + 5
