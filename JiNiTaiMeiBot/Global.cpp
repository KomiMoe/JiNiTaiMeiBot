#include "Global.h"

#include <stdint.h>

bool pressKeyboard(byte key)
{
    keybd_event(key, MapVirtualKeyW(key, 0), 0, 0);
    return true;
    // return SendInput(1, &input, sizeof(input));
}

bool releaseKeyBoard(byte key)
{
    keybd_event(key, MapVirtualKeyW(key, 0), KEYEVENTF_KEYUP, 0);
    return true;
    // return SendInput(1, &input, sizeof(input));
}

bool clickKeyboard(byte key, unsigned long milliSecond)
{
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

bool setClipboardText(const std::wstring& text)
{
    if (!text.size()) {
        return false;
    }
    if (!OpenClipboard(nullptr)) {
        return false;
    }

    const auto msgLength  = text.size();
    const int  bufferSize = static_cast<int>((msgLength + 4) * sizeof(wchar_t));
    const auto hMemHandle = GlobalAlloc(GPTR, bufferSize);
    if (!hMemHandle) {
        CloseClipboard();
        return false;
    }
    const auto pMemData = static_cast<wchar_t*>(GlobalLock(hMemHandle));
    if (!pMemData) {
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

std::string base64Encode(const void* pData, size_t inputLength)
{
    // clang-format off
    static char encodingTable[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
        'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
        'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',
        '4', '5', '6', '7', '8', '9', '+', '/' };
    // clang-format on

    static int modTable[] = { 0, 2, 1 };

    const auto   data         = static_cast<const uint8_t*>(pData);
    const size_t outputLength = 4 * ((inputLength + 2) / 3);
    std::string  result;
    result.resize(outputLength);

    const auto encodedData = result.data();

    for (int i = 0, j = 0; i < inputLength;) {
        const uint32_t octetA = i < inputLength ? static_cast<uint32_t>(data[i++]) : 0;
        const uint32_t octetB = i < inputLength ? static_cast<uint32_t>(data[i++]) : 0;
        const uint32_t octetC = i < inputLength ? static_cast<uint32_t>(data[i++]) : 0;

        const uint32_t triple = (octetA << 0x10) + (octetB << 0x08) + octetC;

        encodedData[j++] = encodingTable[(triple >> 3 * 6) & 0x3F];
        encodedData[j++] = encodingTable[(triple >> 2 * 6) & 0x3F];
        encodedData[j++] = encodingTable[(triple >> 1 * 6) & 0x3F];
        encodedData[j++] = encodingTable[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < modTable[inputLength % 3]; i++) {
        encodedData[outputLength - 1 - i] = '=';
    }

    return encodedData;
}