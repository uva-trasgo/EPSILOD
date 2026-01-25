import ast
from typing import Any

import dsl_core.epsilod_ctypes


def get_allowed_types() -> set[str]:
	# native python types with the same name need to be added manually
	allowed_types = {"int", "float", "bool"}
	for name in dir(dsl_core.epsilod_ctypes):
		if name.startswith("_"):
			continue
		obj = getattr(dsl_core.epsilod_ctypes, name)
		# Include classes and aliases
		if isinstance(obj, type) or not callable(obj):
			allowed_types.add(name)
	return allowed_types


class PyToC(ast.NodeVisitor):
	"""
    A Python → C translator for a small subset of Python.

    Supported constructs:
      - Variable declarations with type hints (AnnAssign)
      - Reassignments (Assign)
      - if / else
      - while
      - for i in range(...)
      - Arithmetic and logical expressions
    """

	def __init__(self, ext_params: list[str]) -> None:
		self.indent: int = 0
		self.output: list[str] = []
		# TODO should not be float, where to get the type from?
		basetype = ("float", False)
		self.symbols: dict[str, tuple[str, bool]] = {"new": basetype, "old": basetype, "old2": basetype}
		self.ext_names: list[str] = list()
		# TODO do we care about spaces in the type?
		for param in ext_params:
			type, name = " ".join(param.split()[:-1]), param.split()[-1]
			# remove [n] from name if present
			clean_name = name[:name.index("[")] if "[" in name else name
			# TODO we currently don't care if things are arrays here
			self.symbols[clean_name] = type, False
			self.ext_names.append(clean_name)

		self.allowed_types = get_allowed_types()

	# -------------------------------------------------------------------------
	# Utility helpers
	# -------------------------------------------------------------------------
	def emit(self, line: str = "") -> None:
		"""Emit a line with current indentation."""
		self.output.append("\t" * self.indent + line)

	def translate(self, node: ast.AST) -> str:
		"""Translate a parsed AST into C source."""
		self.visit(node)
		return "\n".join(self.output)

	# -------------------------------------------------------------------------
	# Top-level
	# -------------------------------------------------------------------------
	def visit_Module(self, node: ast.Module):
		"""Translate a module (top-level list of statements)."""
		for stmt in node.body:
			self.visit(stmt)

	def visit_ImportFrom(self, node: ast.ImportFrom):
		"""Ignore import nodes"""
		pass

	def generic_visit(self, node: ast.AST) -> Any:
		raise NotImplementedError(f"Unsupported node type: {type(node).__name__}")

	# -------------------------------------------------------------------------
	# Assignments
	# -------------------------------------------------------------------------
	def visit_AnnAssign(self, node: ast.AnnAssign) -> Any:
		if not isinstance(node.target, ast.Name):
			raise NotImplementedError("Only simple variable names allowed")

		name = node.target.id
		ctype, is_array = self.get_ctype(node.annotation)
		self.symbols[name] = ctype, is_array

		# --- carray(...) initialization ---
		if is_array:
			if not isinstance(node.value, ast.Call) or not isinstance(node.value.func, ast.Name) or node.value.func.id != "carray":
				raise SyntaxError("Expected carray() constructor for carray type")

			call = node.value
			if len(call.args) != 1:
				raise SyntaxError("carray() takes exactly one argument")

			arg = call.args[0]

			# Case 1: size literal → carray(4)
			if isinstance(arg, ast.Constant) and isinstance(arg.value, int) and not isinstance(arg.value, bool):
				size = arg.value
				if size <= 0:
					raise SyntaxError("carray size must be greater than 0")
				init = "{0}"
			# Case 2: list literal → carray([1, 2, 3])
			elif isinstance(arg, ast.List):
				elems = [self.visit(e) for e in arg.elts]
				size = len(elems)
				if size <= 0:
					raise SyntaxError("carray size must be greater than 0")
				init = f"{{{', '.join(elems)}}}"
			else:
				raise SyntaxError("carray() argument must be an int or list literal")
			self.emit(f"{ctype} {name}[{size}] = {init};")
		else:
			if node.value is None:
				self.emit(f"{ctype} {name};")
			else:
				self.emit(f"{ctype} {name} = {self.visit(node.value)};")

	def visit_Assign(self, node: ast.Assign) -> Any:
		"""Handle assignments like x = y, arr[i] = y, obj.field = y, etc."""
		# Reject chained assignments: a = b = c = 0
		if len(node.targets) != 1:
			raise SyntaxError("Chained assignments are not supported")

		target = node.targets[0]

		# Reject tuple/list unpacking: x, y = ...
		if isinstance(target, (ast.Tuple, ast.List)):
			raise SyntaxError("Tuple or list unpacking is not supported")

		value_code = self.visit(node.value)
		target_code = self.visit(target)

		base_name = self.get_base_name(target)
		# TODO something if base_name is none?
		if base_name and base_name in self.symbols:
			_, is_array = self.symbols[base_name]
			# NOTE: arrays inside other stuff (eg. x.arr=arr2) are not captured
			if is_array and isinstance(target, ast.Name):
				raise SyntaxError(f"Cannot assign to full array '{base_name}', assign to elements instead")
		elif base_name:
			raise SyntaxError(f"Variable '{base_name}' must be declared and annotated before use")

		# Emit the actual assignment
		self.emit(f"{target_code} = {value_code};")

	def visit_AugAssign(self, node: ast.AugAssign) -> Any:
		lhs_code = self.visit(node.target)
		rhs_code = self.visit(node.value)
		op = self.get_op(node.op)
		lhs_type = self.get_expr_ctype(node.target)

		# TODO Disallow modifying a full carray
		# if is_array and not isinstance(node.target, ast.Subscript):
		# 	raise SyntaxError("Cannot use augmented assignment on a whole carray")

		# vec3f special case
		if lhs_type == "vec3f":
			if isinstance(node.op, ast.Add):
				code = f"{lhs_code} = vec3f_add({lhs_code}, {rhs_code});"
			elif isinstance(node.op, ast.Mult):
				code = f"{lhs_code} = vec3f_scale({lhs_code}, {rhs_code});"
			else:
				raise NotImplementedError(f"Operator {op} not supported for vec3f")
			self.emit(code)
			return

		# Fallback: regular +=, -=, etc.
		self.emit(f"{lhs_code} {op}= {rhs_code};")

	def visit_Subscript(self, node: ast.Subscript) -> str:
		"""Translate arr[i] → arr[i]"""
		arr = self.visit(node.value)
		index = self.visit(node.slice)
		# add implicit .data field to cell_t
		# TODO this should be handled by type
		if ((isinstance(node.value, ast.Name) and node.value.id in ("new", "old", "old2")) or
		    (isinstance(node.value, ast.Call) and isinstance(node.value.func, ast.Name) and node.value.func.id == "neigh")):
			return f"{arr}.data[{index}]"
		return f"{arr}[{index}]"

	# -------------------------------------------------------------------------
	# Control Flow
	# -------------------------------------------------------------------------
	def visit_If(self, node: ast.If) -> Any:
		cond = self.visit(node.test)
		self.emit(f"if ({cond}) {{")
		self.indent += 1
		for stmt in node.body:
			self.visit(stmt)
		self.indent -= 1

		if node.orelse:
			self.emit("} else {")
			self.indent += 1
			for stmt in node.orelse:
				self.visit(stmt)
			self.indent -= 1
		self.emit("}")

	def visit_While(self, node: ast.While) -> Any:
		if node.orelse:
			raise SyntaxError("while else not allowed")
		cond = self.visit(node.test)
		self.emit(f"while ({cond}) {{")
		self.indent += 1
		for stmt in node.body:
			self.visit(stmt)
		self.indent -= 1
		self.emit("}")

	def visit_For(self, node: ast.For) -> Any:
		""" Only supports: for i in range(start, stop, step) """
		# Ensure `target` is a Name node
		if not isinstance(node.target, ast.Name):
			raise NotImplementedError("Loop variable must be a simple name")
		var = node.target.id

		# Ensure iteration is range()
		if not (isinstance(node.iter, ast.Call) and isinstance(node.iter.func, ast.Name) and node.iter.func.id == "range"):
			raise NotImplementedError("Only range() loops are supported")

		args = [self.visit(arg) for arg in node.iter.args]
		if len(args) == 1:
			start, end, step = "0", args[0], "1"
		elif len(args) == 2:
			start, end, step = args[0], args[1], "1"
		elif len(args) == 3:
			start, end, step = args
		else:
			raise SyntaxError("range() takes 1-3 arguments")

		if var not in self.symbols:
			self.symbols[var] = "int", False
			self.emit(f"int {var};")

		self.emit(f"for ({var} = {start}; {var} < {end}; {var} += {step}) {{")
		self.indent += 1
		for stmt in node.body:
			self.visit(stmt)
		self.indent -= 1
		self.emit("}")

	def visit_Break(self, node: ast.Break):
		self.emit("break;")

	def visit_Continue(self, node: ast.Continue):
		self.emit("continue;")

	# -------------------------------------------------------------------------
	# Expressions
	# -------------------------------------------------------------------------
	def visit_BinOp(self, node: ast.BinOp) -> str:
		left_code = self.visit(node.left)
		right_code = self.visit(node.right)
		left_type = self.get_expr_ctype(node.left)
		right_type = self.get_expr_ctype(node.right)

		# Special handling for vec3f
		if "vec3f" in (left_type, right_type):
			if isinstance(node.op, ast.Add):
				return f"vec3f_add({left_code}, {right_code})"
			elif isinstance(node.op, ast.Mult):
				# Detect which side is scalar
				if left_type == "vec3f":
					return f"vec3f_scale({left_code}, {right_code})"
				elif right_type == "vec3f":
					return f"vec3f_scale({right_code}, {left_code})"
				else:
					raise SyntaxError("Invalid operands for vec3f Mult")
			else:
				raise SyntaxError(f"Operator {type(node.op).__name__} not supported for vec3f")

		# Fallback to normal arithmetic
		return f"({left_code} {self.get_op(node.op)} {right_code})"

	def visit_BoolOp(self, node: ast.BoolOp) -> str:
		op = "&&" if isinstance(node.op, ast.And) else "||"
		values = [self.visit(v) for v in node.values]
		return f"({f'  {op}  '.join(values)})"

	def visit_Compare(self, node: ast.Compare) -> str:
		left = self.visit(node.left)
		comparisons = []
		for op, right in zip(node.ops, node.comparators):
			op_str = self.get_op(op)
			right_str = self.visit(right)
			comparisons.append(f"{left} {op_str} {right_str}")
			left = right_str
		return f'({" && ".join(comparisons)})'

	def visit_UnaryOp(self, node: ast.UnaryOp) -> str:
		op = self.get_op(node.op)
		operand = self.visit(node.operand)
		return f"({op}{operand})"

	def visit_Name(self, node: ast.Name) -> str:
		if node.id in self.ext_names:
			return f"ext_params.{node.id}"
		return node.id

	def visit_Constant(self, node: ast.Constant) -> str:
		return repr(node.value)

	def visit_Expr(self, node: ast.Expr) -> Any:
		code = self.visit(node.value)
		self.emit(f"{code};")

	def visit_Call(self, node: ast.Call) -> str:
		# Recognize vec3f(...) → (vec3f){...}
		if isinstance(node.func, ast.Name) and node.func.id == "vec3f":
			args = [self.visit(a) for a in node.args]
			return f"(({node.func.id}){{{', '.join(args)}}})"

		func = self.visit(node.func)
		args = ", ".join(self.visit(a) for a in node.args)
		return f"{func}({args})"

	def visit_Attribute(self, node: ast.Attribute) -> str:
		value = self.visit(node.value)
		return f"{value}.{node.attr}"

	def visit_IfExp(self, node: ast.IfExp) -> str:
		"""Translate Python's ternary 'x if cond else y' into C's 'cond ? x : y'."""
		test_code = self.visit(node.test)
		body_code = self.visit(node.body)
		orelse_code = self.visit(node.orelse)
		return f"({test_code} ? {body_code} : {orelse_code})"

	# -------------------------------------------------------------------------
	# Helpers
	# -------------------------------------------------------------------------
	def get_ctype(self, annotation: ast.expr) -> tuple[str, bool]:
		"""return [type, is_array]"""
		if isinstance(annotation, ast.Name):
			if annotation.id in self.allowed_types:
				return annotation.id, False
		elif isinstance(annotation, ast.Subscript) and isinstance(annotation.value, ast.Name) and annotation.value.id == "carray":
			elem_ctype, is_array = self.get_ctype(annotation.slice)
			if is_array:
				raise SyntaxError("Multidimensional carray types are not supported")
			return elem_ctype, True
		raise ValueError(f"Unsupported type annotation: {ast.dump(annotation)}")

	def get_op(self, op: ast.AST) -> str:
		mapping: dict[type, str] = {
		    ast.Add: "+",
		    ast.Sub: "-",
		    ast.Mult: "*",
		    ast.Div: "/",
		    ast.Mod: "%",
		    ast.UAdd: "+",
		    ast.USub: "-",
		    ast.Not: "!",
		    ast.Lt: "<",
		    ast.Gt: ">",
		    ast.LtE: "<=",
		    ast.GtE: ">=",
		    ast.Eq: "==",
		    ast.NotEq: "!=",
		    ast.And: "&&",
		    ast.Or: "||",
		}
		return mapping.get(type(op), "?")

	def get_expr_ctype(self, node: ast.AST) -> str | None:
		"""Infer the C type of an expression node if possible."""
		# Simple literals and names
		if isinstance(node, ast.Name):
			return self.symbols.get(node.id, (None, None))[0]
		elif isinstance(node, ast.Constant):
			if isinstance(node.value, bool):
				return "int"
			elif isinstance(node.value, int):
				return "int"
			elif isinstance(node.value, float):
				return "float"
			return None

		# Binary operation — check overload or normal arithmetic
		elif isinstance(node, ast.BinOp):
			left_t = self.get_expr_ctype(node.left)
			right_t = self.get_expr_ctype(node.right)

			# --- Overloaded type rules ---
			if "vec3f" in (left_t, right_t):
				# vec3f + vec3f → vec3f
				# vec3f * scalar or scalar * vec3f → vec3f
				if isinstance(node.op, ast.Add) or isinstance(node.op, ast.Mult):
					return "vec3f"
				# Unsupported ops
				return None

			# --- Regular arithmetic promotion ---
			if None in (left_t, right_t):
				return None
			if left_t == right_t:
				return left_t
			if "float" in (left_t, right_t):
				return "float"
			return "int"

		if isinstance(node, ast.IfExp):
			t_body = self.get_expr_ctype(node.body)
			t_else = self.get_expr_ctype(node.orelse)
			# If both sides same type → that type
			if t_body == t_else:
				return t_body
			# vec3f and others — if either is vec3f, promote to vec3f
			if "vec3f" in (t_body, t_else):
				return "vec3f"
			# Promote numeric types
			if "float" in (t_body, t_else):
				return "float"
			if "int" in (t_body, t_else):
				return "int"
			return None

		# Subscript (array access)
		if isinstance(node, ast.Subscript):
			return self.get_expr_ctype(node.value)

		# Attribute access (obj.field)
		if isinstance(node, ast.Attribute):
			# TODO for now assume float
			return "float"

		return None

	def get_base_name(self, node: ast.AST) -> str | None:
		"""
		Recursively extract the base variable name from an expression.

		Examples:
		x → "x"
		arr[i] → "arr"
		obj.field → "obj"
		obj.arr[i] → "obj"
		f().x → None
		"""
		if isinstance(node, ast.Name):
			return node.id
		elif isinstance(node, ast.Attribute):
			# e.g. obj.field → recurse into obj
			return self.get_base_name(node.value)
		elif isinstance(node, ast.Subscript):
			# e.g. arr[i] → recurse into arr
			return self.get_base_name(node.value)
		else:
			return None
