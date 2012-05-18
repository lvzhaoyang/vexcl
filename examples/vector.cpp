#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <oclutil/oclutil.hpp>

using namespace clu;

int main() {
    srand(time(0));

    try {
	cl::Context context;
	std::vector<cl::CommandQueue> queue;

	// Select every GPU supporting double precision.
	std::tie(context, queue) = queue_list(
		Filter::Type(CL_DEVICE_TYPE_GPU) && Filter::DoublePrecision(),
		true
		);

	if (queue.empty()) {
	    std::cerr << "No OpenCL devices found." << std::endl;
	    return 1;
	}

	// Allocate host vector, fill it with random values.
	const uint N = 1024 * 1024;
	std::vector<double> host_vec(N);
	std::generate(host_vec.begin(), host_vec.end(), []() {
		return double(rand()) / RAND_MAX;
		});

	// Allocate device vector initialized with host vector data.
	// Device vector will be partitioned between selected devices.
	clu::vector<double> x(queue, CL_MEM_READ_WRITE, host_vec);

	// Allocate uninitialized device vectors.
	clu::vector<double> y(queue, CL_MEM_READ_WRITE, N);
	clu::vector<double> z(queue, CL_MEM_READ_WRITE, N);

	// Appropriate kernels are compiled (once) and called automagically.
	// To see generated kernels uncomment the following line:
	//clu::vector<double>::show_kernels = true;

	// Fill device vector with constant value...
	y = Const(42);
	// ... or with some expression:
	z = sqrt(Const(2) * x) + cos(y);

	std::cout << "y * y = " << inner_product(y, y) << std::endl;

	// Check results at random location.
	uint pos = rand() % N;

	// You can read or write device vector elements with [] notation (very
	// ineffective but sometimes convenient).
	std::cout << y[pos] << std::endl;
	std::cout << "res = " << z[pos] - sqrt(2 * x[pos]) - cos(y[pos]) << std::endl;

	// Or you can read the entire vector to host:
	copy(z, host_vec);
    } catch (const cl::Error &e) {
	std::cout << "OpenCL error: " << e << std::endl;
    }
}
