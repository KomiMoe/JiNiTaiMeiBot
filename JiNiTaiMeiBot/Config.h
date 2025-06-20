#pragma once

#include <string>
#include <Windows.h>

class Config {
public:
    bool debug = true;

    int ocrTimeout = 3;
    std::string ocrArgs = "--models=\".\\models\" "
            "--det=ch_PP-OCRv4_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx "
            "--rec=rec_ch_PP-OCRv4_infer.onnx  --keys=dict_chinese.txt --padding=60 "
            "--maxSideLen=1024 --boxScoreThresh=0.5 --boxThresh=0.3 --unClipRatio=1.6 --doAngle=0 "
            "--mostAngle=0 --numThread=1";

    int  suspendGTATime = 15;
    int  delaySuspendTime = 5;
    int  checkLoopTime = 1;
    int  matchPanelTimeout = 300;
    int  joiningPlayerKick = 120;
    int  startMatchDelay = 15;
    bool startOnAllJoined = true;
    bool suspendAfterMatchStarted = true;
    int exitMatchTimeout = 120;

    int pressSTimeStairs = 400;
    int pressATimeStairs = 400;
    int goOutStairsTime = 3500;

    int intoWallTime = 1500;

    int pressSTimeAisle = 360;
    int pressATimeAisle = 500;
    int crossAisleTime = 4000;

    int pressSTimeGoJob = 340;
    int pressATimeGoJob = 480;
    int waitFindJobTimeout = 15000;

    explicit Config(std::string& configFileName);
};
