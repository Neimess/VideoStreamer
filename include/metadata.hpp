#ifndef METADATA_HPP
#define METADATA_HPP

#include <nlohmann/json.hpp>
#include <vector>
#include <string>
#include "logger.hpp"

class MetaData {
public:
    MetaData();


    template <typename T>
    void create(const T& data) {
        try {
            metaData_ = data;
            isValid_ = true;
        } catch (const std::exception& e) {
            Logger::getInstance().log(std::string("Error creating metadata: ") + e.what());
            isValid_ = false;
        }
    }

    void parse(const std::vector<unsigned char>& rawData);
    bool isValid() const;

    const nlohmann::json& get() const;

private:
    nlohmann::json metaData_;
    bool isValid_;
};

#endif // METADATA_HPP
