# vec3f arrays
from epsilod_ctypes import *

arr: carray[vec3f] = carray(2)
arr[0] = vec3f(1, 0.2, 3)
arr[0] *= 5
