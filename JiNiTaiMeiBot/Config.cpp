#include "Config.h"

#include <iostream>
#include <fstream>
#include <json/json.h>

Config* GConfig = nullptr;

#define READ_VALUE(key,ty) \
    this->##key = [&]() ->auto { \
        if(configRoot[#key].is##ty()) { \
            return configRoot[#key].as##ty(); \
        } \
        return this->##key; \
    }();

#define READ_INT(key) READ_VALUE(key, Int);
#define READ_STRING(key) READ_VALUE(key, String);
#define READ_BOOL(key) READ_VALUE(key, Bool);

#define WRITE_VALUE(key) \
    configRoot[#key] = key;

Config::Config(std::string& configFileName) {
    Json::Reader  reader;
    std::ifstream ifs(configFileName);
    Json::Value   configRoot;

    if(!ifs.is_open()) {
#ifdef _DEBUG
        WRITE_VALUE(debug);

        WRITE_VALUE(ocrTimeout);
        WRITE_VALUE(ocrArgs);

        WRITE_VALUE(suspendGTATime);
        WRITE_VALUE(delaySuspendTime);
        WRITE_VALUE(checkLoopTime);
        WRITE_VALUE(matchPanelTimeout);
        WRITE_VALUE(joiningPlayerKick);
        WRITE_VALUE(startMatchDelay);
        WRITE_VALUE(startOnAllJoined);
        WRITE_VALUE(suspendAfterMatchStarted);

        WRITE_VALUE(pressSTimeStairs);
        WRITE_VALUE(pressATimeStairs);
        WRITE_VALUE(goOutStairsTime);

        WRITE_VALUE(intoWallTime);

        WRITE_VALUE(pressSTimeAisle);
        WRITE_VALUE(pressATimeAisle);
        WRITE_VALUE(crossAisleTime);

        WRITE_VALUE(pressSTimeGoJob);
        WRITE_VALUE(pressATimeGoJob);
        WRITE_VALUE(waitFindJobTimeout);

        Json::StyledStreamWriter writer{};
        std::ofstream ofs(configFileName);
        writer.write(ofs, configRoot);
        ofs.flush();
        ofs.close();
        return;
#endif
        throw std::runtime_error("Can not open file");
    }
    const auto paresResult = reader.parse(ifs, configRoot);
    ifs.close();
    if(!paresResult) {
        throw std::runtime_error(reader.getFormattedErrorMessages());
    }

    READ_BOOL(debug);

    READ_INT(ocrTimeout);
    READ_STRING(ocrArgs);

    READ_INT(suspendGTATime);
    READ_INT(delaySuspendTime);
    READ_INT(checkLoopTime);
    READ_INT(matchPanelTimeout);
    READ_INT(joiningPlayerKick);
    READ_INT(startMatchDelay);
    READ_BOOL(startOnAllJoined);
    READ_BOOL(suspendAfterMatchStarted);

    READ_INT(pressSTimeStairs);
    READ_INT(pressATimeStairs);
    READ_INT(goOutStairsTime);

    READ_INT(intoWallTime);

    READ_INT(pressSTimeAisle);
    READ_INT(pressATimeAisle);
    READ_INT(crossAisleTime);

    READ_INT(pressSTimeGoJob);
    READ_INT(pressATimeGoJob);
    READ_INT(waitFindJobTimeout);

}
