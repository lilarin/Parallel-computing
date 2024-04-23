#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <vector>

enum class Request {
    SendData,
    StartComputing,
    GetData,
    EndConnection
};

enum class Response {
    Processing,
    DataProcessed
};

#endif // PROTOCOL_H
