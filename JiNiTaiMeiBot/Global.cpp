#include "Global.h"

#include <stdint.h>

bool pressKeyboard(byte key) {
    keybd_event(key, MapVirtualKeyW(key, 0), 0, 0);
    return true;
    // return SendInput(1, &input, sizeof(input));
}

bool releaseKeyBoard(byte key) {
    keybd_event(key, MapVirtualKeyW(key, 0), KEYEVENTF_KEYUP, 0);
    return true;
    // return SendInput(1, &input, sizeof(input));
}

bool clickKeyboard(byte key, unsigned long milliSecond) {
    // auto flag = 
    pressKeyboard(key);
    // if (!flag) {
    //     printf("An error when press key bord, error code: %d", GetLastError());
    //     return false;
    // }
    Sleep(milliSecond);
    // flag = 
    releaseKeyBoard(key);
    // if (!flag) {
    //     printf("An error when release key bord, error code: %d", GetLastError());
    //     return false;
    // }
    return true;
}

bool setClipboardText(const std::wstring& text) {
    if(!text.size()) {
        return false;
    }
    if(!OpenClipboard(nullptr)) {
        return false;
    }

    const auto msgLength = text.size();
    const int  bufferSize = static_cast<int>((msgLength + 4) * sizeof(wchar_t));
    const auto hMemHandle = GlobalAlloc(GPTR, bufferSize);
    if(!hMemHandle) {
        CloseClipboard();
        return false;
    }
    const auto pMemData = static_cast<wchar_t*>(GlobalLock(hMemHandle));
    if(!pMemData) {
        GlobalFree(hMemHandle);
        CloseClipboard();
        return false;
    }

    wcscpy_s(pMemData, bufferSize, text.data());

    GlobalUnlock(hMemHandle);

    EmptyClipboard();
    const auto result = SetClipboardData(CF_UNICODETEXT, hMemHandle);

    CloseClipboard();
    return !!result;
}
