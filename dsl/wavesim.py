from dsl_core.epsilod_dsl import Stencil

wavesim = Stencil(
 name = "wavesim",
 basetype = "float",
 shape = [(-1, 1), (-1, 1)],
 weights = [
 0, 1, 0,
 1, 1, 1,
 0, 1, 0
 ],
 ext_params = [
 "float dt",
 "float dx",
 "float dy",
 "vec2f center",
 "float amplitud",
 "vec2f sigma",
 ],
 kernel = open("wavesim_dsl_kernel.py").read()
)#fmt:off

wavesim.gen_stencil_files("./gen_files_wave")
