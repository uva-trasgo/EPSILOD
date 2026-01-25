# wavesim kernel

y_g: int = global_idx[0] - 1
x_g: int = global_idx[1] - 1

sizex: int = domain_size[0] - 2
sizey: int = domain_size[1] - 2

py: int = 1 if y_g < sizey - 1 else 0
my: int = -1 if y_g > 0 else 0
px: int = 1 if x_g < sizex - 1 else 0
mx: int = -1 if x_g > 0 else 0

lap: float = (
    (dt / dy) * (dt / dy) * ((neigh(py, 0) - old) - (old - neigh(my, 0))) +
    (dt / dx) * (dt / dx) * ((neigh(0, px) - old) - (old - neigh(0, mx)))
) #fmt: off
new = 2 * old - old2 + lap
