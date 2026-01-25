# gassim kernel

from dsl_core.epsilod_ctypes import *

if old[0] != FLAG_KEEP_VELOCITY:
	# Load cell
	new = lbm_load_neigh(Q, offsets)
	if old[0] == FLAG_OBSTACLE:
		# Collision reorder
		new = lbm_bounce(new, Q, opposite)
	else:
		# Compute
		p: float = 0
		v: vec3f
		for i in range(Q):
			p += new[i]
			v += offsets[i] * cellwidth * new[i]
		if p != 0:
			v *= (1 / p)
		for i in range(Q):
			new[i] += deltaT / tau * (feq(i, p, v) - new[i])
