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
