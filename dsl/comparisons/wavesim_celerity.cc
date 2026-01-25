void foo() {
	struct update_config {
		static constexpr float a = 1.f;
		static constexpr float b = 1.f;
		static constexpr float c = 1.f;
	};

	template <typename T, typename Config, typename KernelName>
	void step(celerity::queue & queue, celerity::buffer<T, 2> up, celerity::buffer<T, 2> u, float dt, sycl::float2 delta) {
		queue.submit([&](celerity::handler &cgh) {
			celerity::accessor rw_up{up, cgh, celerity::access::one_to_one{}, celerity::read_write};
			celerity::accessor r_u{u, cgh, celerity::access::neighborhood{{1, 1}, celerity::neighborhood_shape::along_axes}, celerity::read_only};

			const auto size = up.get_range();
			cgh.parallel_for<KernelName>(size, [=](celerity::item<2> item) {
				const size_t py = item[0] < size[0] - 1 ? item[0] + 1 : item[0];
				const size_t my = item[0] > 0 ? item[0] - 1 : item[0];
				const size_t px = item[1] < size[1] - 1 ? item[1] + 1 : item[1];
				const size_t mx = item[1] > 0 ? item[1] - 1 : item[1];

				const float lap = (dt / delta.y()) * (dt / delta.y()) * ((r_u[{py, item[1]}] - r_u[item]) - (r_u[item] - r_u[{my, item[1]}])) +
								  (dt / delta.x()) * (dt / delta.x()) * ((r_u[{item[0], px}] - r_u[item]) - (r_u[item] - r_u[{item[0], mx}]));
				rw_up[item] = Config::a * 2 * r_u[item] - Config::b * rw_up[item] + Config::c * lap;
				rw_up[item] = Config::a * 2 * r_u[item] - Config::b * rw_up[item];
			});
		});
	}

	void update(celerity::queue & queue, celerity::buffer<float, 2> up, celerity::buffer<float, 2> u, float dt, sycl::float2 delta) {
		step<float, update_config, class update>(queue, up, u, dt, delta);
	}

	int main(int argc, char *argv[]) {

		celerity::queue queue;

		int node_count;
		MPI_Comm_size(MPI_COMM_WORLD, &node_count);
		celerity::range<1> node_range{static_cast<size_t>(node_count)};

		celerity::buffer<float, 2> up{celerity::range<2>(cfg.Y, cfg.X)}; // next
		celerity::buffer<float, 2> u{celerity::range<2>(cfg.Y, cfg.X)};  // current

		// initialiaztion

		// Compute
		auto   t = 0.0;
		size_t i = 0;
		while (t < cfg.T) {
			update(queue, up, u, cfg.dt, {cfg.dx, cfg.dy});
			std::swap(u, up);
			t += cfg.dt;
		}

		return EXIT_SUCCESS;
	}
}