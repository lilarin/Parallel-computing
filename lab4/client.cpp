#include <iostream>
#include <winsock2.h>
#include "utils.h"
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

const int PORT = 8000;
const char* SERVER_IP = "127.0.0.1";
const int MATRIX_SIZE = 3;

void sendData(SOCKET socket_fd) {
    std::vector<int> matrix1(MATRIX_SIZE * MATRIX_SIZE);
    std::vector<int> matrix2(MATRIX_SIZE * MATRIX_SIZE);
    Utility::fillRandomMatrix(matrix1, MATRIX_SIZE, 10);
    Utility::fillRandomMatrix(matrix2, MATRIX_SIZE, 10);

    Request request = Request::SendData;
    send(socket_fd, reinterpret_cast<const char*>(&request), sizeof(Request), 0);

    std::cout << "Sending matrix 1 to the server..." << std::endl;
    send(socket_fd, reinterpret_cast<const char*>(matrix1.data()), sizeof(int) * MATRIX_SIZE * MATRIX_SIZE, 0);
    std::cout << "Matrix 1 sent:" << std::endl;
    Utility::printMatrix(matrix1, MATRIX_SIZE);

    std::cout << "Sending matrix 2 to the server..." << std::endl;
    send(socket_fd, reinterpret_cast<const char*>(matrix2.data()), sizeof(int) * MATRIX_SIZE * MATRIX_SIZE, 0);
    std::cout << "Matrix 2 sent:" << std::endl;
    Utility::printMatrix(matrix2, MATRIX_SIZE);
}

void startCloudComputing(SOCKET socket_fd) {
    Request request = Request::StartComputing;
    send(socket_fd, reinterpret_cast<const char*>(&request), sizeof(Request), 0);

    Response response;
    recv(socket_fd, reinterpret_cast<char*>(&response), sizeof(Response), 0);

    if (response == Response::StartedProcessing) {
        std::cout << "Server has started computing data." << std::endl;
    }
}

void getData(SOCKET socket_fd) {
    Request request = Request::GetData;
    send(socket_fd, reinterpret_cast<const char*>(&request), sizeof(Request), 0);
    Response response;

    while (true) {
        recv(socket_fd, reinterpret_cast<char*>(&response), sizeof(Response), 0);

        if (response == Response::Processing) {
            std::cout << "Server has started computation." << std::endl;
        }
        else if (response == Response::Processed) {
            std::vector<int> result(MATRIX_SIZE * MATRIX_SIZE);
            std::cout << "Receiving result from the server..." << std::endl;
            recv(socket_fd, reinterpret_cast<char *>(result.data()), sizeof(int) * MATRIX_SIZE * MATRIX_SIZE, 0);
            std::cout << "Result received:" << std::endl;
            Utility::printMatrix(result, MATRIX_SIZE);
            break;
        } else {
            std::cout << "Data not yet processed. Waiting for server..." << std::endl;
            Sleep(1000);
        }
    }
}

void endConnection(SOCKET socket_fd) {
    Request request = Request::EndConnection;
    send(socket_fd, reinterpret_cast<const char*>(&request), sizeof(Request), 0);
    std::cout << "Connection ended by client." << std::endl;
    closesocket(socket_fd);
}

int main() {
    WSADATA wsaData;
    SOCKET socket_fd;
    struct sockaddr_in server_address{};

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cout << "Failed to initialize WinSock lib" << std::endl;
        return 1;
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_fd == INVALID_SOCKET) {
        std::cout << "Failed to create socket" << std::endl;
        WSACleanup();
        return 1;
    }

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr(SERVER_IP);
    if (server_address.sin_addr.s_addr == INADDR_NONE) {
        std::cout << "Invalid IP address" << std::endl;
        closesocket(socket_fd);
        WSACleanup();
        return 1;
    }

    if (connect(socket_fd, (struct sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::cout << "Failed to connect to server" << std::endl;
        closesocket(socket_fd);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected to the server!" << std::endl;

    sendData(socket_fd);
    startCloudComputing(socket_fd);
    getData(socket_fd);
    endConnection(socket_fd);

    closesocket(socket_fd);
    WSACleanup();

    return 0;
}