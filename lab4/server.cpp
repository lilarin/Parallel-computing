#include <iostream>
#include <winsock2.h>
#include <thread>
#include "utils.h"
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8000;
const int MATRIX_SIZE = 3;
const int NUM_THREADS = 3;

void closeConnection (SOCKET client_socket) {
    std::cout << "Connection ended by server." << std::endl << std::endl;
    closesocket(client_socket);
}

void receiveData(SOCKET client_socket, std::vector<int>& matrix1, std::vector<int>& matrix2) {
    std::cout << "Receiving matrix 1 from client..." << std::endl;
    int bytesReceived = recv(client_socket, reinterpret_cast<char*>(matrix1.data()), sizeof(int) * MATRIX_SIZE * MATRIX_SIZE, 0);
    if (bytesReceived != sizeof(int) * MATRIX_SIZE * MATRIX_SIZE) {
        closeConnection(client_socket);
        return;
    }
    std::cout << "Matrix 1 received." << std::endl;
    Utility::printMatrix(matrix1, MATRIX_SIZE);

    std::cout << "Receiving matrix 2 from client..." << std::endl;
    recv(client_socket, reinterpret_cast<char*>(matrix2.data()), sizeof(int) * MATRIX_SIZE * MATRIX_SIZE, 0);
    std::cout << "Matrix 2 received." << std::endl;
    Utility::printMatrix(matrix2, MATRIX_SIZE);
}

void computeData(SOCKET client_socket, std::vector<int>& result,
                 const std::vector<int>& matrix1, const std::vector<int>& matrix2) {
    std::vector<std::thread> threads;

    Response response = Response::Processing;
    send(client_socket, reinterpret_cast<const char*>(&response), sizeof(Response), 0);

    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(Utility::compute, i * MATRIX_SIZE / NUM_THREADS, (i + 1) * MATRIX_SIZE / NUM_THREADS,
                             MATRIX_SIZE, std::ref(result), std::cref(matrix1), std::cref(matrix2));
    }
    for (auto& thread : threads) {
        thread.join();
    }

    std::cout << "Data processed." << std::endl;
    Utility::printMatrix(result, MATRIX_SIZE);

    response = Response::DataProcessed;
    send(client_socket, reinterpret_cast<const char*>(&response), sizeof(Response), 0);
}

void returnData (SOCKET client_socket, std::vector<int>& result) {
    std::cout << "Sending result to client..." << std::endl;
    send(client_socket, reinterpret_cast<const char*>(result.data()), sizeof(int) * MATRIX_SIZE * MATRIX_SIZE, 0);
    std::cout << "Result sent." << std::endl;
}

void handleClient(SOCKET client_socket) {
    std::cout << "Connection with client established." << std::endl;
    std::vector<int> matrix1(MATRIX_SIZE * MATRIX_SIZE);
    std::vector<int> matrix2(MATRIX_SIZE * MATRIX_SIZE);
    std::vector<int> result(MATRIX_SIZE * MATRIX_SIZE);
    while (true) {
        Request request;
        int bytesReceived = recv(client_socket, reinterpret_cast<char*>(&request), sizeof(Request), 0);

        if (bytesReceived == SOCKET_ERROR || bytesReceived == 0) {
            if (bytesReceived == SOCKET_ERROR) {
                std::cout << "Error receiving data from client. Error code: " << WSAGetLastError() << std::endl;
            } else {
                std::cout << "Connection with client terminated." << std::endl;
            }
            closeConnection(client_socket);
            return;
        }

        switch (request) {
            case Request::SendData: {
                receiveData(client_socket, matrix1, matrix2);
                break;
            }
            case Request::StartComputing:
                computeData(client_socket, result, matrix1, matrix2);
                break;

            case Request::GetData:
                returnData(client_socket, result);
                break;

            case Request::EndConnection:
                closeConnection(client_socket);
                return;
        }
    }
}

int main() {
    WSADATA wsaData;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    int client_address_len = sizeof(client_address);

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize socket" << std::endl;
        return 1;
    }

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET) {
        std::cout << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_ANY);
    server_address.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::cout << "Failed to bind socket" << std::endl;
        closeConnection(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, 5) == SOCKET_ERROR) {
        std::cout << "Failed to listen on socket" << std::endl;
        closeConnection(server_socket);
        WSACleanup();
        return 1;
    }

    try {
        std::cout << "Listening for client to connect..." << std::endl << std::endl;
        while (true) {
            client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
            if (client_socket == INVALID_SOCKET) {
                std::cout << "Failed to accept client connection" << std::endl;
                continue;
            }

            std::thread client_thread(handleClient, client_socket);
            client_thread.detach();
        }
    }
    catch (...) {
        std::cout << "Exception caught" << std::endl;
    }

    closeConnection(server_socket);
    WSACleanup();

    return 0;
}