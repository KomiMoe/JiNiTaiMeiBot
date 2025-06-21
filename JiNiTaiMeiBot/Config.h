#pragma once

#include <string>
#include <Windows.h>
#include <json/json.h>
#include <map>

#define REG_INT_VALUE(name, def, comment) \
    int name = [&]() -> auto { \
        readCallbacks[&name] = [](const Json::Value& configRoot, void* pValue) { \
            if(configRoot[#name].isInt()) { \
                *static_cast<int*>(pValue) = configRoot[#name].asInt(); \
            } \
        }; \
        writeCallbacks[&name] = [](Json::Value& configRoot, const void* pValue) { \
            configRoot[#name] = *static_cast<const int*>(pValue); \
            configRoot[#name].setComment(std::string("// "##comment), Json::commentAfterOnSameLine); \
        }; \
        return def; \
    }();

#define REG_BOOL_VALUE(name, def, comment) \
    bool name = [&]() -> auto { \
        readCallbacks[&name] = [](const Json::Value& configRoot, void* pValue) { \
            if(configRoot[#name].isBool()) { \
                *static_cast<bool*>(pValue) = configRoot[#name].asBool(); \
            } \
        }; \
        writeCallbacks[&name] = [](Json::Value& configRoot, const void* pValue) { \
            configRoot[#name] = *static_cast<const bool*>(pValue); \
            configRoot[#name].setComment(std::string("// "##comment), Json::commentAfterOnSameLine); \
        }; \
        return def; \
    }();

#define REG_STRING_VALUE(name, def, comment) \
    std::string name = [&]() -> auto { \
        readCallbacks[&name] = [](const Json::Value& configRoot, void* pValue) { \
            if(configRoot[#name].isString()) { \
                static_cast<std::string*>(pValue)->clear(); \
                static_cast<std::string*>(pValue)->append(configRoot[#name].asString());\
            } \
        }; \
        writeCallbacks[&name] = [](Json::Value& configRoot, const void* pValue) { \
            configRoot[#name] = *static_cast<const std::string*>(pValue); \
            configRoot[#name].setComment(std::string("// "##comment), Json::commentAfterOnSameLine); \
        }; \
        return def; \
    }();


class Config
{
protected:
    std::map<void*, void (*)(const Json::Value& configRoot, void* pValue)> readCallbacks;
    std::map<void*, void (*)(Json::Value& configRoot, const void* pValue)> writeCallbacks;

public:
    REG_BOOL_VALUE(debug, true, "开启调试模式");

    REG_INT_VALUE(ocrTimeout, 3, "等待OCR结果超时时间(秒)");
    REG_STRING_VALUE(ocrArgs, "--models=\".\\models\" "
                     "--det=ch_PP-OCRv4_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx "
                     "--rec=rec_ch_PP-OCRv4_infer.onnx  --keys=dict_chinese.txt --padding=70 "
                     "--maxSideLen=1024 --boxScoreThresh=0.5 --boxThresh=0.3 --unClipRatio=1.6 --doAngle=0 "
                     "--mostAngle=0 --numThread=1", "OCR参数");

    REG_INT_VALUE(suspendGTATime, 15, "卡单持续时间 (秒)");

    REG_INT_VALUE(delaySuspendTime, 5, "卡单延迟时间 (秒)(在面板消失xx秒后卡单/在落地xx秒后卡单)");
    REG_INT_VALUE(checkLoopTime, 1, "检测间隔时间 (秒)");
    REG_INT_VALUE(matchPanelTimeout, 300, "面板无人加入时重开时间 (秒)");
    REG_INT_VALUE(joiningPlayerKick, 120, "等待正在加入玩家超时重开时间 (秒)");
    REG_INT_VALUE(startMatchDelay, 15, "开始差事等待延迟 (秒)");
    REG_BOOL_VALUE(startOnAllJoined, true, "全部玩家已加入时立即开始差事而不等待 (绕过 \"startMatchDelay\" 时间)");
    REG_BOOL_VALUE(suspendAfterMatchStarted, false, "true: 等待落地后延迟再卡单; false: 面板消失后延迟再卡单");
    REG_INT_VALUE(exitMatchTimeout, 120, "等待差事启动落地超时时间 (秒)(防止卡在启动战局中)");

    REG_INT_VALUE(pressSTimeStairs, 400, "差事层楼梯口进行\"走出门\"动作时 每轮按S的持续时间 (毫秒)");
    REG_INT_VALUE(pressATimeStairs, 400, "差事层楼梯口进行\"走出门\"动作时 每轮按A的持续时间 (毫秒)");
    REG_INT_VALUE(goOutStairsTime, 1000, "差事层楼梯口进行\"走出门\"动作持续时间 (毫秒)");

    REG_INT_VALUE(pressSTimeAisle, 360, "差事层进行\"穿过走道\"动作时 每轮按S的持续时间 (毫秒)");
    REG_INT_VALUE(pressATimeAisle, 500, "差事层进行\"穿过走道\"动作时 每轮按A的持续时间 (毫秒)");
    REG_INT_VALUE(crossAisleTime, 3000, "差事层进行\"穿过走道\"动作持续时间 (毫秒)");

    REG_INT_VALUE(pressSTimeGoJob, 340, "差事层进行\"寻找差事黄圈\"动作时 每轮按S的持续时间 (毫秒)");
    REG_INT_VALUE(pressATimeGoJob, 480, "差事层进行\"寻找差事黄圈\"动作时 每轮按S的持续时间 (毫秒)");
    REG_INT_VALUE(waitFindJobTimeout, 15000, "差事层进行\"寻找差事黄圈\"动作超时时间 (毫秒)");

    REG_STRING_VALUE(msgOpenJobPanel, "德瑞差事已启动，卡好CEO直接来", "开好面板时发的消息 (设置为空字符串则不发这条消息)");
    REG_STRING_VALUE(msgWaitPlayerTimeout, "启动任务超时，重新启动中", "没人加入超时重开时发的消息 (设置为空字符串则不发这条消息)");
    REG_STRING_VALUE(msgJoiningPlayerKick, "任务中含有卡B，重新启动中", "有人卡在正在加入超时重开时发的消息 (设置为空字符串则不发这条消息)");
    REG_STRING_VALUE(msgTeamFull, "满了", "满人时发的消息 (设置为空字符串则不发这条消息)");
    REG_STRING_VALUE(msgJobStarted, "开了，请等待下一辆车", "差事开始时发的消息 (设置为空字符串则不发这条消息)");

    explicit Config(std::string& configFileName);
};