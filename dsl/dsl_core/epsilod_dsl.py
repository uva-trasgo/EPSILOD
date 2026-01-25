from collections import defaultdict
from dataclasses import InitVar, dataclass, field
import errno
import itertools
import os
from typing import Iterator
from dsl_core.py2c import PyToC
import ast


@dataclass
class Stencil:
	name: str
	shape: list[tuple[int, int]]
	weights: InitVar[dict[tuple[int, ...], float] | list[float]]
	weights_dict: defaultdict[tuple[int, ...], float] = field(init = False)
	basetype: str = "float"
	weight_divisor: float = 0
	defs: list[str] = field(default_factory = list)
	ext_params: list[str] = field(default_factory = list)
	kernel: str = ""

	def __post_init__(self, weights: dict[tuple[int, ...], float] | list[float]):
		# normalize weights to dict
		if type(weights) == list:
			d = dict(zip(self.shape_iter(), weights))
			self.weights_dict = defaultdict(float, {k: v for k, v in d.items() if float(v) != 0.0})
		elif type(weights) == dict:
			self.weights_dict = defaultdict(float, weights)
		else:
			raise ValueError(f"Error unknown weights spec type {type(weights)}")

		if self.weight_divisor == 0:
			self.weight_divisor = sum(self.weights_dict.values())

	@property
	def dims(self) -> int:
		return len(self.shape)

	def shape_iter(self) -> Iterator[tuple[int, ...]]:
		ranges = [range(int(start), int(end) + 1) for start, end in self.shape]
		return itertools.product(*ranges)

	def gen_kernel(self):
		ijk = [chr(ord('i') + i) for i in range(self.dims)]
		access_ijk = ", ".join(f"thr_{c}" for c in ijk)

		if self.kernel != "":
			# add implicit stuff to custom kernel and translate from python to C
			self.kernel = f"{self.basetype} new = old;\n" + PyToC(self.ext_params).translate(ast.parse(self.kernel))
			self.kernel += f"hit( matrix, {access_ijk} ) = new;\n"
			return

		self.kernel += f"hit(matrix, {access_ijk}) = (\n"
		for coord, weight in self.weights_dict.items():
			# coord parenthesis come from str repr
			self.kernel += f"neigh{coord} * {weight} + \n"
		self.kernel = self.kernel.removesuffix(" + \n")
		self.kernel += f") / {self.weight_divisor};"

	def gen_kernel_file(self, path: str):
		# generate kernel code if custom kernel was not specified
		self.gen_kernel()

		with open(os.path.join(path, f"{self.name}.c"), "w") as f:
			if self.ext_params != "":
				f.write(f'#include "{self.name}_ext_type.h"\n')
			f.write('#include "epsilod_kernels.h"\n')

			# TODO what to do for 4d coords? thr_l is not generated

			ijk = [chr(ord('i') + i) for i in range(self.dims)]
			arg_ijk = ", ".join([f"_{c}" for c in ijk])
			offset_ijk = ", ".join([f"_{c} + thr_{c}" for c in ijk])
			zeroes = ", ".join("0" * self.dims)

			for d in self.defs:
				f.write(f"#define {d}\n")

			f.write(f"#define neigh({arg_ijk}) hit(matrixCopy, {offset_ijk})\n")
			f.write(f"#define old neigh({zeroes})\n")

			f.write(
			    f"CTRL_KERNEL({self.name}, GENERIC, DEFAULT, KHitTile_{self.basetype} matrix, KHitTile_{self.basetype} matrixCopy, EpsilodCoords global_coords, KHitTile_float stencil, float factor, const Epsilod_ext ext_params, {{\n"
			)
			f.write(f"{self.kernel}\n")
			f.write("});\n")
			f.write("#undef neigh\n")
			f.write("#undef old\n")
			# TODO undef defs?
		# symlinks for CUDA and HIP kernel files
		os.symlink(f"{self.name}.c", os.path.join(path, f"{self.name}.cu"))
		os.symlink(f"{self.name}.c", os.path.join(path, f"{self.name}.cpp"))

	def gen_data_header(self, path: str):
		with open(os.path.join(path, f"{self.name}_data.h"), "w") as f:
			# string without []
			shp_str = ', '.join(map(str, self.shape))

			f.write(f"REGISTER_STENCIL({self.name}, GENERIC, DEFAULT);\n")
			f.write(f"HitShape shp_{self.name} = hitShape({shp_str});\n")

			geometry = ", ".join([str(self.weights_dict[c]) for c in self.shape_iter()])
			f.write(f"float stencilData_{self.name}[] = {{{geometry}}};\n")
			f.write(f"float factor_{self.name} = {self.weight_divisor};\n")

	def gen_exttype_header(self, path: str):
		if not self.ext_params:
			return
		with open(os.path.join(path, f"{self.name}_ext_type.h"), "w") as f:
			f.write("#define EPSILOD_USER_TYPES typedef struct {")
			for param in self.ext_params:
				f.write(f"{param};")
			f.write("} Epsilod_ext;")

	def gen_stencil_files(self, path: str = "."):
		if not os.path.exists(path):
			try:
				os.makedirs(path)
			except OSError as exc:  # Guard against race condition
				if exc.errno != errno.EEXIST:
					raise
		self.gen_kernel_file(path)
		self.gen_exttype_header(path)
		self.gen_data_header(path)
