# arrays

from epsilod_ctypes import *

nums: carray[int] = carray([1, 2, 3, 4])
squares: carray[int] = carray(4)
i: int
for i in range(4):
	squares[i] = nums[i] * nums[i]
