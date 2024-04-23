#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <vector>

enum class Request {
    SendMatrixSize,
    SendData,
    StartComputing,
    GetData,
    EndConnection
};

enum class Response {
    Processing,
    DataProcessed
};

struct MatrixSize {
    int size;
};

#endif // PROTOCOL_H
