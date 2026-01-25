void foo() {
	const int FLAG_OBSTACLE      = 1 << 0;
	const int FLAG_KEEP_VELOCITY = 1 << 1;

	typedef struct {
		unsigned int mantissa : 23;
		unsigned int exponent : 8;
		unsigned int sign : 1;
	} floatparts;

	const size_t            Q = 19;
	typedef array<float, Q> cell_t;
	typedef vec3<float>     vec3f;

	namespace msl::gassimulation {

		MSL_MANAGED vec3<int> size;

		MSL_MANAGED float deltaT = 1.f;

		MSL_MANAGED float tau       = 0.65;
		MSL_MANAGED float cellwidth = 1.0f;

		MSL_CONSTANT const array<vec3f, Q> offsets;

		MSL_CONSTANT const array<unsigned char, Q> opposite;

		MSL_CONSTANT const array<float, Q> wis;

		MSL_USERFUNC cell_t update(const PLCube<cell_t> &plCube, int x, int y, int z) {
			cell_t cell = plCube(x, y, z);

			auto *parts = (floatparts *)&cell[0];

			if (parts->exponent == 255 && parts->mantissa & FLAG_KEEP_VELOCITY) {
				return cell;
			}

			// Streaming.
			for (int i = 1; i < Q; i++) {
				int sx  = x + (int)offsets[i].x;
				int sy  = y + (int)offsets[i].y;
				int sz  = z + (int)offsets[i].z;
				cell[i] = plCube(sx, sy, sz)[i];
			}

			// Collision.
			if (parts->exponent == 255 && parts->mantissa & FLAG_OBSTACLE) {
				if (parts->mantissa & FLAG_OBSTACLE) {
					cell_t cell2 = cell;
					for (size_t i = 1; i < Q; i++) {
						cell[i] = cell2[opposite[i]];
					}
				}
				return cell;
			}
			float p = 0;
			vec3f vp{0, 0, 0};
			for (size_t i = 0; i < Q; i++) {
				p += cell[i];
				vp += offsets[i] * cellwidth * cell[i];
			}
			vec3f v = p == 0 ? vp : vp * (1 / p);
			for (size_t i = 0; i < Q; i++) {
				cell[i] = cell[i] + deltaT / tau * (feq(i, p, v) - cell[i]);
			}
			return cell;
		}

		class Initialize : public Functor4<int, int, int, cell_t, cell_t> {

		  public:
			Initialize(int sizex, int sizey, int sizez) : Functor4() {
				this->sizex = sizex;
				this->sizey = sizey;
				this->sizez = sizez;
			}

			MSL_USERFUNC cell_t operator()(int x, int y, int z, cell_t c) const override {
				for (int i = 0; i < Q; i++) {
					float wi  = wis[i];
					float cw  = cellwidth;
					vec3f v   = {.1f, 0, 0};
					float dot = offsets[i] * cw * v;
					c[i]      = wi * 1.f * (1 + (1 / (cw * cw)) * (3 * dot + (9 / (2 * cw * cw)) * dot * dot - (3.f / 2) * (v * v)));
				}

				if (x <= 1 || y <= 1 || z <= 1 || x >= sizex - 2 || y >= sizey - 2 || z >= sizez - 2 || POW(x - 50, 2) + POW(y - 50, 2) + POW(z - 8, 2) <= 225) {

					auto *parts     = (floatparts *)&c[0];
					parts->sign     = 0;
					parts->exponent = 255;
					if (x <= 1 || x >= sizex - 1 || y <= 1 || y >= sizey - 1 || z <= 1 || z >= sizez - 1) {
						parts->mantissa = 1 << 22 | FLAG_KEEP_VELOCITY;
					} else {
						parts->mantissa = 1 << 22 | FLAG_OBSTACLE;
					}
				}
				return c;
			}

		  private:
			int       sizex, sizey, sizez;
			const int FLAG_OBSTACLE      = 1 << 0;
			const int FLAG_KEEP_VELOCITY = 1 << 1;
		};

		void gassimulation_test(vec3<int> dimension, int iterations, const std::string &importFile, const std::string &exportFile, const std::string &runtimeFile) {
			size = dimension;
			DC<cell_t> dc(size.x, size.y, size.z);
			DC<cell_t> dc2(size.x, size.y, size.z);
			// DA<cell_t> da(size.x, {});

			// Pointers for swapping.
			DC<cell_t> *dcp1 = &dc;
			DC<cell_t> *dcp2 = &dc2;
			// Update update;
			for (int i = 0; i < iterations; i++) {
				dcp1->mapStencil<update>(*dcp2, 1, {});
				dcp2->mapStencil<update>(*dcp1, 1, {});
			}

			dcp1->updateHost();
		}
	} // namespace msl::gassimulation
}