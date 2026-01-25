import ast
from py2c import PyToC
import os


def test(path: str):
	with open(path) as f:
		print(f"{os.path.basename(path)}: {f.readline().strip()}")
		try:
			tree = ast.parse(f.read())
			c_code = PyToC([
			    "vec3f         offsets[Q]",
			    "unsigned char opposite[Q]",
			    "float         wis[Q]",
			    "float         cellwidth",
			    "float         deltaT",
			    "float         tau",
			]).translate(tree)
			print(c_code)
		except Exception as e:
			print(f"[Error] {e}")
	print()


# beds = ["valid", "invalid", "extra"]

# for bed in beds:
# 	print(f"{bed}:")
# 	for case in sorted(os.scandir(os.path.join("py2c_testbed", bed)), key = lambda f: f.name):
# 		test(case.path)

# test(os.path.join("py2c_testbed", "valid", "J.py"))
# test(os.path.join("wavesim_dsl_kernel.py"))
test(os.path.join("gassim_dsl_kernel.py"))
