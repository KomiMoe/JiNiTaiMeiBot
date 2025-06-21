#pragma once

#include <string>
#include <Windows.h>
#include <json/json.h>
#include <map>

#define REG_INT_VALUE(name, def) \
    int name = [&]() -> auto { \
        readCallbacks[&name] = [](const Json::Value& configRoot, void* pValue) { \
            if(configRoot[#name].isInt()) { \
                *static_cast<int*>(pValue) = configRoot[#name].asInt(); \
            } \
        }; \
        writeCallbacks[&name] = [](Json::Value& configRoot, const void* pValue) { \
            configRoot[#name] = *static_cast<const int*>(pValue); \
        }; \
        return def; \
    }();

#define REG_BOOL_VALUE(name, def) \
    bool name = [&]() -> auto { \
        readCallbacks[&name] = [](const Json::Value& configRoot, void* pValue) { \
            if(configRoot[#name].isBool()) { \
                *static_cast<bool*>(pValue) = configRoot[#name].asBool(); \
            } \
        }; \
        writeCallbacks[&name] = [](Json::Value& configRoot, const void* pValue) { \
            configRoot[#name] = *static_cast<const bool*>(pValue); \
        }; \
        return def; \
    }();

#define REG_STRING_VALUE(name, def) \
    std::string name = [&]() -> auto { \
        readCallbacks[&name] = [](const Json::Value& configRoot, void* pValue) { \
            if(configRoot[#name].isString()) { \
                static_cast<std::string*>(pValue)->clear(); \
                static_cast<std::string*>(pValue)->append(configRoot[#name].asString());\
            } \
        }; \
        writeCallbacks[&name] = [](Json::Value& configRoot, const void* pValue) { \
            configRoot[#name] = *static_cast<const std::string*>(pValue); \
        }; \
        return def; \
    }();


class Config
{
protected:
    std::map<void*, void (*)(const Json::Value& configRoot, void* pValue)> readCallbacks;
    std::map<void*, void (*)(Json::Value& configRoot, const void* pValue)> writeCallbacks;

public:
    REG_BOOL_VALUE(debug, true);

    REG_INT_VALUE(ocrTimeout, 3);
    REG_STRING_VALUE(ocrArgs, "--models=\".\\models\" "
                     "--det=ch_PP-OCRv4_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx "
                     "--rec=rec_ch_PP-OCRv4_infer.onnx  --keys=dict_chinese.txt --padding=70 "
                     "--maxSideLen=1024 --boxScoreThresh=0.5 --boxThresh=0.3 --unClipRatio=1.6 --doAngle=0 "
                     "--mostAngle=0 --numThread=1");

    REG_INT_VALUE(suspendGTATime, 15);

    REG_INT_VALUE(delaySuspendTime, 5);
    REG_INT_VALUE(checkLoopTime, 1);
    REG_INT_VALUE(matchPanelTimeout, 300);
    REG_INT_VALUE(joiningPlayerKick, 120);
    REG_INT_VALUE(startMatchDelay, 15);
    REG_BOOL_VALUE(startOnAllJoined, true);
    REG_BOOL_VALUE(suspendAfterMatchStarted, true);
    REG_INT_VALUE(exitMatchTimeout, 120);

    REG_INT_VALUE(pressSTimeStairs, 400);
    REG_INT_VALUE(pressATimeStairs, 400);
    REG_INT_VALUE(goOutStairsTime, 1000);

    REG_INT_VALUE(pressSTimeAisle, 360);
    REG_INT_VALUE(pressATimeAisle, 500);
    REG_INT_VALUE(crossAisleTime, 3000);

    REG_INT_VALUE(pressSTimeGoJob, 340);
    REG_INT_VALUE(pressATimeGoJob, 480);
    REG_INT_VALUE(waitFindJobTimeout, 15000);

    explicit Config(std::string& configFileName);
};