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
    Json::StreamWriterBuilder builder;
    builder["emitUTF8"] = true;

    std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    std::ofstream            ofs(configFileName);
    writer->write(configRoot, &ofs);
    ofs.flush();
    ofs.close();
}