#include "Config.h"

#include <iostream>
#include <fstream>
#include <json/json.h>

Config* GConfig = nullptr;

Config::Config(std::string& configFileName)
{
    std::ifstream ifs(configFileName);
    Json::Value   configRoot;

    if (ifs.is_open()) {
        Json::Reader reader;
        const auto   paresResult = reader.parse(ifs, configRoot);
        ifs.close();
        if (!paresResult) {
            throw std::runtime_error(reader.getFormattedErrorMessages());
        }
        for (const auto& [pValue, callback] : readCallbacks) {
            callback(configRoot, pValue);
        }
    }

    for (const auto& [pValue, callback] : writeCallbacks) {
        callback(configRoot, pValue);
    }

    Json::StyledStreamWriter writer{};
    std::ofstream            ofs(configFileName);
    writer.write(ofs, configRoot);
    ofs.flush();
    ofs.close();
}