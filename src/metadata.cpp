#include "metadata.hpp"

MetaData::MetaData() : isValid_(false) {}

void MetaData::parse(const std::vector<unsigned char>& rawData) {
    try {
        std::string dataStr(rawData.begin(), rawData.end());
        metaData_ = nlohmann::json::parse(dataStr, nullptr, false);
        if (metaData_.is_discarded()) {
            Logger::getInstance().log("Error: Failed to parse metadata!");
            isValid_ = false;
        } else {
            isValid_ = true;
        }
    } catch (const std::exception& e) {
        Logger::getInstance().log(std::string("Error parsing metadata: ") + e.what());
        isValid_ = false;
    }
}

bool MetaData::isValid() const {
    return isValid_;
}

const nlohmann::json& MetaData::get() const {
    return metaData_;
}
