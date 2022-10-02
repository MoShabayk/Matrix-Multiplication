#include <iostream>
#include <vector>
#include <algorithm>
#include <future>
#include <atomic>

#include <execution>
#include <chrono>

#include <random>
#include <ratio>
#include <numeric>
#include <functional>


#include <list>

#include "common.h"


using namespace std;

class Matrix {
	int* data;
	int rows;
	int columns;

public:
	Matrix(int r, int c) {
		rows = r;
		columns = c;
		data = new int[rows * columns];
		fill(data, data + rows * columns, 0);
	}

	void set_values(int i, int j, int value) {

		data[i * columns + j] = value;
	}

	void set_all(int value) {
		fill(data, data + columns * rows, value);
	}

	static void multiply(Matrix* x, Matrix* y, Matrix* results) {

		if (!(x->columns == y->rows) || !(x->rows == results->rows) && (y->columns == results->columns))
		{
			cout << "Error: Invalid dimensions of matrix for multiplication" << endl;

		}

		int r = results->rows * results->columns;

		for (size_t i = 0; i < r; i++) {

			for (size_t j = 0; j < x->columns; j++)
			{
				for (size_t j = 0; j < x->columns; j++)
				{
					results->data[i] += x->data[(i / results->columns) * x->columns + j] * y->data[i % results->rows + j * y->columns];
				}

			}
		}
	}

	static void parallel_multiply(Matrix* x, Matrix* y, Matrix* results) {

		struct process_data_chunk {
			void operator()(Matrix* x, Matrix* y, Matrix* results, int start_index, int end_index)
			{
				
				for (size_t i = start_index; i < end_index; i++) {

					for (size_t j = 0; j < x->columns; j++)
					{
						for (size_t j = 0; j < x->columns; j++)
						{
							results->data[i] += x->data[(i / results->columns) * x->columns + j] * y->data[i % results->rows + j * y->columns];
						}

					}
				}
			}
		};

		int length = results->rows * results->columns;

		if (!length)
			return;

		int min_per_thread = 10000;
		int max_threads = (length + min_per_thread + 1) / min_per_thread;
		int hardware_threads = thread::hardware_concurrency();
		int num_threads = min(hardware_threads != 0 ? hardware_threads : 2, max_threads);
		int block_size = length / num_threads;

		vector<thread> threads(num_threads - 1);
		int block_start = 0;
		int block_end = 0;

		{
			join_threads joiners(threads);

			for (unsigned i = 0; i < (num_threads - 1); i++) {
				block_start = i * block_size;
				block_end = block_start + block_size;
				threads[i] = thread(process_data_chunk(), results, x, y, block_start, block_end);
			}

			process_data_chunk()(results, x, y, block_end, length);

		}
	}



	void print()
	{
		if (rows < 50 && columns < 50)
		{
			for (size_t i = 0; i < rows; i++)
			{
				for (size_t j = 0; j < columns; j++)
				{
					std::cout << data[j + i * columns] << " ";
				}

				std::cout << "\n";
			}
			std::cout << std::endl;
		}
	}


};

int main() {

	Matrix A(200, 200);
	Matrix B(200, 200);
	Matrix results(200, 200);

	A.set_all(3);
	B.set_all(4);

	auto startTime = chrono::high_resolution_clock::now();
	Matrix::multiply(&A, &B, &results);
	auto endTime = chrono::high_resolution_clock::now();

	cout << "SEQUENTIAL MATRIX: " << chrono::duration_cast<chrono::microseconds>(endTime - startTime).count() << endl;


	Matrix C(200, 200);
	Matrix D(200, 200);
	Matrix results2(200, 200);

	C.set_all(5);
	D.set_all(6);

	chrono::steady_clock::time_point parStartTime = chrono::high_resolution_clock::now();
	Matrix::parallel_multiply(&C, &D, &results2);
	chrono::steady_clock::time_point parEndTime = chrono::high_resolution_clock::now();

	cout << "PARALLEL MATRIX: " << chrono::duration_cast<chrono::microseconds>(parEndTime - parStartTime).count() << endl;






}
