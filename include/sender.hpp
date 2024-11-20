#ifndef SENDER_HPP
#define SENDER_HPP

#include <vector>
#include <string>

class Sender {
public:
    virtual ~Sender() = default;


    virtual void start() = 0;

    virtual void send(const std::vector<unsigned char>& data) = 0;
};

#endif // SENDER_HPP
