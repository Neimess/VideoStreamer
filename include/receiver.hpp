#ifndef RECEIVER_HPP
#define RECEIVER_HPP
#include <vector>
class Receiver {
public:
    virtual ~Receiver() = default;

    virtual void start() = 0;
    virtual std::vector<unsigned char> receive() = 0;
};

#endif // RECEIVER_HPP
