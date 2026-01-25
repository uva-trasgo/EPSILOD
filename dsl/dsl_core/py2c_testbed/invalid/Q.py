# in place ops on carray
from epsilod_ctypes import *

arr: carray[int] = carray(4)
arr += arr  # disallowed
