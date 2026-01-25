from dsl_core.epsilod_dsl import Stencil

ccgas = Stencil(
 name = "ccgas",
 basetype = "cell_t",
 shape = [(-1, 1), (-1, 1), (-1, 1)],
 weights = [
 0, 1, 0,    1, 1, 1,    0, 1, 0,
 1, 1, 1,    1, 1, 1,    1, 1, 1,
 0, 1, 0,    1, 1, 1,    0, 1, 0
 ],
 defs = [
 "FLAG_KEEP_VELOCITY INFINITY",
 "FLAG_OBSTACLE      -INFINITY",
 "Q                  19",
 ],
 ext_params = [
 "vec3f         offsets[Q]",
 "unsigned char opposite[Q]",
 "float         wis[Q]",
 "float         cellwidth",
 "float         deltaT",
 "float         tau",
 ],
 kernel = open("gassim_dsl_kernel.py").read(),
)#fmt:off
ccgas.gen_stencil_files("./gen_files_gas")
