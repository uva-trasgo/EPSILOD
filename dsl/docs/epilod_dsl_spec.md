
# EPSILOD-DSL

It is a Domain Specific Language (DSL) to describe many types of Iterative Stencil Loop (ISL) applications. It is designed to express stencils in a form that can be easily translated to C-language code and integrated with the EPSILOD framework, a tool to build ISL applications targeting heterogenous distributed platforms.

A description of a stencil in the DSL includes a description of the stencil neighborhood, and an optional kernel code. It distinguishes two type of stencil applications based on the abstractions it provides.

1) Weighted-average stencil applications: 
Many stencil applications are based on iteratively applying to each domain element an operation that consists on a weighted average of a neighborhood of elements. For this type of stencil applications the operations can be described in an abstract form by a pattern to determine the neighborhood, the weight to apply to each neighbor, and the factor to divide the sum and obtain the average. EPSILOD-DSL provides abstractions to express this stencil applications without the need to provide specific code.

2) Kernel-described stencil applications:
For more complex stencil applications EPSILOD-DSL provides a full abstract language to express a kernel code used to compute the next iteration of a domain element in terms of the values in its neighborhood.

EPSILOD-DSL is a Python-embedded DSL. It works with python3.12. A stencil application is described by creating an object of the Stencil class, provided by the "epsilod-dsl" module.

The Stencil class provides only two public methods:
	1) Contructor
	2) gen_stencil_files()

The constructor is used to provide the stencil application description. A call to the gen_stencil_files method generates the C-language-code files needed to use the EPSILOD framework to build the application. EPSILOD-DSL can be described by the Stencil class constructor parameters, and the content of the provided EPSILOD-DSL Python modules.


Stencil class contructor
--------------------------
- name: String, Name of the application

- basetype: String, Base type of the domain elements. A C-language native type or user-defined type available in a header file.

- Neighborhood description
	The two following parameters, shape and weights, determine the stencil neighborhood. They are used by EPSILOD to calculate the communications needed accross domain partitions.

  - shape: List of n-Tuples of two integers. shape = ( s_i = (r_i,r_i') : i in [0,n-1] )
A descripcion of the maximum radius of the stencil on each dimension and direction. The number of tuples determine the number of dimensions of the domain. 
Let x = (x_i : i in [0,n-1]) be the n-dimensional index of a domain element. 
Let y = (y_i : i in [0,n-1]) be the n-dimensional index of a neighbor element of x.
The elements of the index y_i are coordinates relative to each domain element x.
A neighbor of the domain element x is restricted to have an index with y_i in the range [x_i + r_i : x_i + r_i' ]. 

  - weights: It can have two different type of values.

    - List of numbers: A list of weights for each element in the neighborhood. The cardinality of the list is determined by the shape elements. 
	| weights | = \prod_{i=0}^{n-1} (r_i' - r_i)
	Each number represents the weight to apply to the neighbor in the corresponding relative position of the target domain element to be computed, in the case of an averaged sum stencil application. A zero value indicates that the the relative position is not included in the neighborhood. In the case of the second type of EPSILOD applications, any non-zero number indicate that the relative position is included in the neighborhood, but the exact weight has no special meaning.
    - Dictionary that maps neighbourhood relative indexes (y) to their weight. The default weight is 0.

 - weight_divisor: Optional. It specifies a divisor for the weighted mean. It defaults to the sum of weights. In the case of the second type of EPSILOD applications this is ignored.

 - ext_params: Optional. A list of strings. Each string is a  C-language declaration of a parameter. These parameters are included as extra information in the kernel of the stencil application. The user should provide specific values in the main code of the application.

 - defs: Optional. A list of strings. Each string contains two words. It is a list of named values that can be used to simplify the readability of the code introduced in the "kernel" parameter.

 - kernel: Optional. String. This string contains the kernel code for the second type of EPSILOD applications. The string can be literal, obtained from a file, etc.


Kernel code string
---------------------
The code in the kernel string is a subset of Python with added semantic restrictions.

The white-listed nodes of the abstract syntactic-tree allowed are:

- **Module**: Python base node
- **ImportFrom**: It is used to import the EPSILOD types module. This module contains type definitions including classes to represent C-language types, and unidimensional arrays (carrays class). Import of other modules is ignored.
- **AnnAssign**: Annotated assign. Restrictions: Only simple variable names.
	This node is used to declare variables with a compulsary type hint. Type hints are restricted
	to the C-types described in the EPSILOD types module. Variables of type carray require initizalization using
	the carray constructor with a literal number of elements or a 
	literal non-empty list.
- **Assign**: Restrictions: The assigned variable should have been previously declared. Chained assingments are not allowed. Tuple/list unpacking is not allowed. Assingment to carrays are not allowed but assingments to indexed/subscripted elements of carrays are allowed.
- **AugAssign**: Assingment to carrays are not allowed but assingments to indexed/subscripted elements of carrays are allowed.
- **Subscript**
- **If** 
- **IfExp**
- **While**: Restrictions: Orelse clause is not allowed.
- **For**: Restrictions: The loop index should be a simple variable. The iterator should be a range.
- **BinOP**
- **BoolOP**
- **Compare**
- **UnaryOP**
- **Name**
- **Constant**
- **Expr**
- **Call**
- **Attribute**
- **Break**
- **Continue**

### Predeclared symbols
- **neigh(y_0, y_1, ..., y_n-1)**: Reurns the value of a neighbor element in the previous iteration. Indexes are relative to the current domain element to compute.
- **old**: basetype variable. Shortcut for the value of the domain in the previous iteration.
- **old2**: basetype variable. Shortcut for the value of the domain two iterations ago.
- **new**: basetype variable. The user should assign to this variable the final value of the domain element in the current iteration. The default value is the "old" value.
- **global_idx**: carray[int]. The index of the domain element.
- **domain_size**: carray[int]. The sizes of the domain on each dimension.

#### Predeclared operations for LBM (Lattice Boltzman Methods)
- **lbm_load_neigh(Q, offsets)**: It returns a basetype value "z" containing Q values selected from the neighbors specified by "offsets" carray such that ( z_i : i in [0,n-1], z_i = neigh(offsets[i])_i ).The offsets list/array can be supplied as an ext_param declaration and its values provided in initizalition of the application.
- **lbm_bounce(z, Q, opposite)**: It retuns a basetype value z' containing the values of z reordered according to the "opposite" carray ( z'_i : i in [0,n-1], z'_i = z_{offsets[i]} ).
 
### EPSILOD types module

	module epsilod-ctpyes.pyi

	class carray(Generic[_T]):
		def __init__(self, init: int | list[_T]): ...
		def __getitem__(self, i) -> _T:	...
		def __setitem__(self, i, elem):	...


	class vec3f:
		def __init__(self, x: float, y: float, z: float): ...
		def __add__(self, other: Self) -> Self: ...
		def __mult__(self, scalar: float) -> Self: ...
		def __iadd__(self, other: Self) -> Self: ...
		def __imul__(self, scalar: float) -> Self: ...

	int
	float
	bool
	int8_t
	int16_t
	int32_t
	int64_t
	uint8_t
	uint16_t
	uint32_t
	uint64_t
	size_t
	long_t
	ulong_t
	float32
	float64
	double
	bool_t
