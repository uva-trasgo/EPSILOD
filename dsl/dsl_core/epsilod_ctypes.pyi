from typing import Generic, Self, TypeVar

# -------------------------------------------------------------------
# Generic C array
# -------------------------------------------------------------------
# NOTE nov 2025, using old generic type syntax because yapf does not support the new one yet
_T = TypeVar("_T")


class carray(Generic[_T]):
	"""Stub-type for fixed-size C-style array."""

	def __init__(self, init: int | list[_T]):
		...

	def __getitem__(self, i) -> _T:
		...

	def __setitem__(self, i, elem):
		...


# -------------------------------------------------------------------
# vec3f
# -------------------------------------------------------------------
class vec3f:
	"""Stub-type for vec3f type"""

	def __init__(self, x: float, y: float, z: float):
		...

	def __add__(self, other: Self) -> Self:
		...

	def __mult__(self, scalar: float) -> Self:
		...

	def __iadd__(self, other: Self) -> Self:
		...

	def __imul__(self, scalar: float) -> Self:
		...


# -------------------------------------------------------------------
# Standard integer types
# -------------------------------------------------------------------
int8_t = int
int16_t = int
int32_t = int
int64_t = int
uint8_t = int
uint16_t = int
uint32_t = int
uint64_t = int
size_t = int
long_t = int
ulong_t = int

# -------------------------------------------------------------------
# Floating point types
# -------------------------------------------------------------------
float32 = float
float64 = float
double = float64
# long_double = float64 <- is this a thing? isn't it long double

# -------------------------------------------------------------------
# Boolean type
# -------------------------------------------------------------------
bool_t = bool

# -------------------------------------------------------------------
# Common C aliases
# -------------------------------------------------------------------
c_int = int
c_long = int
c_float = float
c_double = float
c_bool = bool
