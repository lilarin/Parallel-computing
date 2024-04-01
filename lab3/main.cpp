#include <iostream>
#include <chrono>
#include <atomic>
#include <random>
#include <thread>
#include <vector>

constexpr int arraySize = 500000;
constexpr int numThreads = 12;
int array[arraySize];


static int getRandomValue(int lower_bound, int upper_bound) {
    std::random_device rd;

    std::uniform_int_distribution<int> dist(lower_bound, upper_bound);

    return dist(rd);
}

void fillArray() {
    for (int & i : array) {
        i = getRandomValue(0, 100);
    }
}

void task1(int& sum, int& smallest) {
    int localSum = 0;
    int localSmallest = INT_MAX;

    for (int i : array) {
        if (i % 2 == 0 && i != 0) {
            localSum += i;
            if (i < localSmallest) {
                localSmallest = i;
            }
        }
    }
    sum += localSum;
    if (localSmallest < smallest) {
        smallest = localSmallest;
    }
}

void task2(int& sum, int& smallest, int start, int end, std::mutex& sumMutex, std::mutex& smallestMutex) {
    int localSum = 0;
    int localSmallest = INT_MAX;

    for (int i = start; i < end; ++i) {
        if (array[i] % 2 == 0 && array[i] != 0) {
            localSum += array[i];
            if (array[i] < localSmallest) {
                localSmallest = array[i];
            }
        }
    }
    sumMutex.lock();
    sum += localSum;
    sumMutex.unlock();

    smallestMutex.lock();
    if (localSmallest < smallest) {
        smallest = localSmallest;
    }
    smallestMutex.unlock();
}

void task3(std::atomic<int>& sum, std::atomic<int>& smallest, int start, int end) {
    int localSum = 0;
    int localSmallest = INT_MAX;

    for (int i = start; i < end; ++i) {
        if (array[i] % 2 == 0 && array[i] != 0) {
            localSum += array[i];
            if (array[i] < localSmallest) {
                localSmallest = array[i];
            }
        }
    }
    int expectedSum = sum.load();
    while (!sum.compare_exchange_weak(expectedSum, expectedSum + localSum)) {}

    int expectedSmallest = smallest;
    while (localSmallest < expectedSmallest) {
        if (smallest.compare_exchange_weak(expectedSmallest, localSmallest)) {
            break;
        }
    }
}


int main() {
    fillArray();

    int sum_task_1 = 0;
    int smallest_task_1 = INT_MAX;
    auto start_task_1 = std::chrono::high_resolution_clock::now();
    task1(sum_task_1, smallest_task_1);
    auto end_task_1 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration_task_1 = end_task_1 - start_task_1;
    std::cout << "Task 1, without parallelization " << std::endl;
    std::cout << "Time taken: " << duration_task_1.count() << " seconds" << std::endl;
    std::cout << "Sum of even elements: " << sum_task_1 << std::endl;
    std::cout << "Smallest even number: " << smallest_task_1 << std::endl << std::endl;


    int sum_task_2 = 0;
    int smallest_task_2 = INT_MAX;
    std::mutex sumMutex;
    std::mutex smallestMutex;
    std::vector<std::thread> threads_task_2;
    threads_task_2.reserve(numThreads);
    int chunkSize = arraySize / numThreads;
    int remaining = arraySize % numThreads;
    int startIdx_task2 = 0;
    int endIdx_task2 = 0;

    auto start_task_2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        startIdx_task2 = endIdx_task2;
        int extra = (i == numThreads - 1) ? remaining : 0;
        endIdx_task2 = startIdx_task2 + chunkSize + extra;
        threads_task_2.emplace_back(task2, std::ref(sum_task_2), std::ref(smallest_task_2), startIdx_task2, endIdx_task2, std::ref(sumMutex), std::ref(smallestMutex));
    }

    for (std::thread & t : threads_task_2) {
        t.join();
    }
    auto end_task_2 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration_task_2 = end_task_2 - start_task_2;
    std::cout << "Task 2, with sync primitives" << std::endl;
    std::cout << "Time taken: " << duration_task_2.count() << " seconds" << std::endl;
    std::cout << "Sum of even elements: " << sum_task_2 << std::endl;
    std::cout << "Smallest even number: " << smallest_task_2 << std::endl << std::endl;


    std::vector<std::thread> threads_task_3;
    threads_task_3.reserve(numThreads);
    std::atomic<int> sum_task_3(0);
    std::atomic<int> smallest_task_3(INT_MAX);
    int startIdx_task3 = 0;
    int endIdx_task3 = 0;

    auto start_task_3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < numThreads; ++i) {
        startIdx_task3 = endIdx_task3;
        endIdx_task3 = startIdx_task3 + chunkSize + (i < remaining ? 1 : 0);
        threads_task_3.emplace_back(task3, std::ref(sum_task_3), std::ref(smallest_task_3), startIdx_task3, endIdx_task3);
    }

    for (std::thread & t : threads_task_3) {
        t.join();
    }
    auto end_task_3 = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> duration_task_3 = end_task_3 - start_task_3;
    std::cout << "Task 3, with non-blocking synchronization" << std::endl;
    std::cout << "Time taken: " << duration_task_3.count() << " seconds" << std::endl;
    std::cout << "Sum of even elements: " << sum_task_3 << std::endl;
    std::cout << "Smallest even number: " << smallest_task_3 << std::endl;


    return 0;
}
