from dsl_core.epsilod_dsl import Stencil

cc2d4 = Stencil(
    name = "2d4",
    basetype = "float",
    shape = [(-1, 1), (-1, 1)],
    weights = {
        (-1, 0): 1,
        (0, -1): 1,
        (0, 1): 1,
        (1, 0): 1,
    },
)
cc2d4.gen_stencil_files("./gen_files_2d4")
