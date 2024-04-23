#ifndef LAB4_UTILS_H
#define LAB4_UTILS_H

#include <random>
#include <iostream>
#include <vector>

class Utility {
public:
    static void printMatrix(const std::vector<int>& matrix, int matrix_size)
    {
        for (int i = 0; i < matrix_size; ++i) {
            for (int j = 0; j < matrix_size; ++j) {
                std::cout << matrix[i * matrix_size + j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    static void fillRandomMatrix(std::vector<int>& matrix, int matrix_size, int max_value)
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int> distribution(0, max_value);

        for (int i = 0; i < matrix_size * matrix_size; ++i) {
            matrix[i] = distribution(gen);
        }
    }

    static void compute(const std::size_t start_row, const std::size_t end_row, const int matrix_size,
                        std::vector<int>& result,
                        const std::vector<int>& matrix1, const std::vector<int>& matrix2)
    {
        for (std::size_t i = start_row; i < end_row; ++i) {
            for (std::size_t j = 0; j < matrix_size; ++j) {
                result[i * matrix_size + j] = matrix1[i * matrix_size + j] - matrix2[i * matrix_size + j];
            }
        }
    }
};

#endif //LAB4_UTILS_H
