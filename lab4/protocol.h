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
    StartedProcessing,
    Processing,
    Processed
};

#endif // PROTOCOL_H
