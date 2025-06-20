#pragma once

#include <Windows.h>
#include <vector>
#include <string>
#include <json/json.h>

#include "Global.h"

class OCREngine {
protected:
    HANDLE mHandleProcess = nullptr;
    HANDLE mHandleRead = nullptr;
    HANDLE mHandleWrite = nullptr;

    bool writePipe(const std::string& text) const;

public:
    OCREngine();

    ~OCREngine() {
        CloseHandle(mHandleRead);
        CloseHandle(mHandleWrite);
        TerminateProcess(mHandleProcess, 0);
        CloseHandle(mHandleProcess);
    }
    Json::Value ocrJson(HWND hWnd, float x = 0, float y = 0, float z = 0.5f, float w = 0.5f) const;
    std::vector<wchar_t> ocrUTF(HWND hWnd, float x = 0, float y = 0, float z = 0.5f, float w = 0.5f) const;
};
