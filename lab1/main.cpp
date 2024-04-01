#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>

using std::chrono::nanoseconds;
using std::chrono::duration_cast;
using std::chrono::high_resolution_clock;

void printMatrix(const std::vector<std::vector<int>>& matrix)
{
    for (const auto& row : matrix) {
        for (int value : row) {
            std::cout << value << " ";
        }
        std::printf("\n");
    }
    std::printf("\n");
}

void fillRandomMatrix(std::vector<std::vector<int>>& matrix, int max_value)
{
    int n = matrix.size();

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            matrix[i][j] = std::rand() % (max_value + 1);
        }
    }
}

void compute(const std::size_t thread_id, const int n, std::vector<std::vector<int>>& result,
             const std::vector<std::vector<int>>& matrix1, const std::vector<std::vector<int>>& matrix2,
             const std::size_t num_threads, std::vector<double>& execution_times)
{
    std::size_t start_row = thread_id * (n / num_threads);
    std::size_t end_row = (thread_id == num_threads - 1) ? n : (thread_id + 1) * (n / num_threads);

    auto payload_begin = high_resolution_clock::now();

    for (std::size_t i = start_row; i < end_row; ++i) {
        for (std::size_t j = 0; j < n; ++j) {
            result[i][j] = matrix1[i][j] - matrix2[i][j];
        }
    }

    auto payload_end = high_resolution_clock::now();
    auto elapsed = duration_cast<nanoseconds>(payload_end - payload_begin);
    double elapsed_seconds = elapsed.count() * 1e-9;
    execution_times[thread_id] = elapsed_seconds;
//    std::printf("Thread %zu: Payload Time: %.3f seconds.\n", thread_id, elapsed_seconds);
}

int main()
{
    std::srand(std::time(0));
    const int num_threads = 192;
    const int matrix_size = 9000;

    std::vector<std::vector<int>> result(matrix_size, std::vector<int>(matrix_size, 0));
    std::vector<std::vector<int>> matrix1(matrix_size, std::vector<int>(matrix_size, 0));
    std::vector<std::vector<int>> matrix2(matrix_size, std::vector<int>(matrix_size, 0));

    fillRandomMatrix(matrix1, 9);
    fillRandomMatrix(matrix2, 9);
//    printMatrix(matrix1);
//    printMatrix(matrix2);

    std::vector<std::thread> threads;
    std::vector<double> execution_times(num_threads, 0.0);

    auto work_begin = high_resolution_clock::now();
    auto creation_begin = high_resolution_clock::now();

    for (int i = 0; i < num_threads; i++) {
        threads.emplace_back(compute, i, matrix_size, std::ref(result), std::ref(matrix1), std::ref(matrix2), num_threads, std::ref(execution_times));
    }

    auto creation_end = high_resolution_clock::now();

    for (auto& thread : threads) {
        thread.join();
    }

    auto work_end = high_resolution_clock::now();

    double average_execution_time = 0.0;
    for (const auto& time : execution_times) {
        average_execution_time += time;
    }

    auto elapsed_creation = duration_cast<nanoseconds>(creation_end - creation_begin);
    double total_creation_time = elapsed_creation.count() * 1e-9;
    std::printf("Creation time: %.4f seconds.\n", total_creation_time);

    average_execution_time /= num_threads;
    std::printf("Average payload time for all threads: %.4f seconds.\n", average_execution_time);

    auto elapsed_work = duration_cast<nanoseconds>(work_end - work_begin);
    double total_work_time = elapsed_work.count() * 1e-9;
    std::printf("Work time: %.4f seconds.\n", total_work_time);

//    std::printf("Result matrix:\n");
//    printMatrix(result);

    return 0;
}
