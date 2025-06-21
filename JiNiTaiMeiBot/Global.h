#pragma once

#include "Config.h"
#include "Logger.h"

#include <Windows.h>
#include <string>

class Config;
class Logger;
class OCREngine;

extern Config*    GConfig;
extern Logger*    GLogger;
extern OCREngine* GOCREngine;

extern HWND  GGtaHWnd;
extern DWORD GGtaPid;

bool pressKeyboard(byte key);
bool releaseKeyBoard(byte key);
bool clickKeyboard(byte key, unsigned long milliSecond = 90);

bool setClipboardText(const std::wstring& text);

std::string base64Encode(const void* pData, size_t inputLength);

inline std::string replaceAll(std::string str, std::string_view from, std::string_view to)
{
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
    return str;
}

inline std::vector<std::string> split(const std::string& str, const std::string& delimiter = " ")
{
    std::vector<std::string> tokens;
    size_t                   prev = 0, pos = 0;

    do {
        pos = str.find(delimiter, prev);
        if (pos == std::string::npos)
            pos = str.length();
        auto token = str.substr(prev, pos - prev);
        if (!token.empty())
            tokens.push_back(token);
        prev = pos + delimiter.length();
    } while (pos < str.length() && prev < str.length());

    return tokens;
}