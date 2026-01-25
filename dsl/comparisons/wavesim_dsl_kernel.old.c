const float dt = ext_params.dt;
const float dx = ext_params.dx;
const float dy = ext_params.dy;

int radius = 1;

const int y_g = thr_i + global_coords.offset[0] - radius;
const int x_g = thr_j + global_coords.offset[1] - radius;

const int sizex = global_coords.size[1] - 2 * radius;
const int sizey = global_coords.size[0] - 2 * radius;

const int py = y_g < sizey - 1 ? 1 : 0;
const int my = y_g > 0 ? -1 : 0;
const int px = x_g < sizex - 1 ? 1 : 0;
const int mx = x_g > 0 ? -1 : 0;

const float lap =
	(dt / dy) * (dt / dy) * ((neigh(py, 0) - old) - (old - neigh(my, 0))) +
	(dt / dx) * (dt / dx) * ((neigh(0, px) - old) - (old - neigh(0, mx)));

new = 2 * old - hit(matrix, thr_i, thr_j) + lap;