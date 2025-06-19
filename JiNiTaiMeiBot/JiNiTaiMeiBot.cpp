#include <iostream>
#include <filesystem>

#include <Windows.h>
#include <atlimage.h>

#include <json/json.h>


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

bool clickKeyboard(byte key, unsigned long milliSecond = 90) {
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

HANDLE startOCR(const std::string& executeFileName, const std::string& args) {
    HANDLE              hRead, hWrite;
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof sa;
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle = true;
    if(!CreatePipe(&hRead, &hWrite, &sa, 0x1000)) {
        std::cout << "Can not create pipe" << std::endl;
        return nullptr;
    }
    STARTUPINFOA        si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof si;
    GetStartupInfoA(&si);
    si.hStdError = hWrite;
    si.hStdOutput = hWrite;
    si.wShowWindow = SW_HIDE;
    si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    const std::filesystem::path executePath(executeFileName);
    std::string                 arg;
    arg.append("\"");
    arg.append(executePath.filename().string());
    arg.append("\"");
    arg.append(" ");
    arg.append(args);
    if(!CreateProcessA(executeFileName.c_str(), arg.data(), nullptr, nullptr, true, NULL, nullptr, nullptr, &si, &pi)) {
        return nullptr;
    }
    CloseHandle(hWrite);
    CloseHandle(pi.hThread);
    CloseHandle(pi.hProcess);
    return hRead;
}

std::string readPipeAndClose(HANDLE hRead) {
    std::string result;
    char        buffer[4096]{};
    DWORD       nRead = 0;
    while(ReadFile(hRead, buffer, sizeof(buffer) - 1, &nRead, nullptr)) {
        result.append(buffer);
        memset(buffer, 0, sizeof buffer);
    }
    CloseHandle(hRead);
    return result;
}

int CaptureAnImage(HWND hWnd, float x, float y, float z, float w) {
    const auto hdcDesktop = GetDC(nullptr);
    if(!hdcDesktop) {
        std::cout << "Can not get desktop HDC" << std::endl;
        return -1;
    }

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    RECT rcWindow;
    GetWindowRect(hWnd, &rcWindow);

    const int startX = static_cast<int>(rcWindow.left + (x * rcClient.right));
    const int startY = static_cast<int>(rcWindow.top + (y * rcClient.bottom));
    const int width = static_cast<int>(rcClient.right * z);
    const int height = static_cast<int>(rcClient.bottom * w);

    CImage image;
    if(!image.Create(width, height, GetDeviceCaps(hdcDesktop, BITSPIXEL))) {
        std::cout << "Can not create image" << std::endl;
        return -1;
    }
    StretchBlt(image.GetDC(), 0, 0, image.GetWidth(), image.GetHeight(), hdcDesktop, startX, startY, width, height, SRCCOPY);
    if(!SUCCEEDED(image.Save(L"temp.png", Gdiplus::ImageFormatPNG))) {
        std::cout << "Can not save image" << std::endl;
        return -1;
    }
    image.ReleaseDC();
    ReleaseDC(WindowFromDC(hdcDesktop), hdcDesktop);
    return 0;
}

struct Vector2i {
    int x;
    int y;
    int z;
    int w;
};

static bool rectAaBb(const Vector2i& rect1, const Vector2i& rect2) {
    return rect1.x < rect2.x + rect2.z &&
            rect1.x + rect1.z > rect2.x &&
            rect1.y < rect2.y + rect2.w &&
            rect1.w + rect1.y > rect2.y;
}

struct EnumWindowArg {
    LPCWSTR windowName = nullptr;
    LPCWSTR sendMsg = nullptr;
    bool    isClassName = false;
};

bool switchFocus(HWND hWnd) {
    // AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), nullptr), GetCurrentThreadId(), TRUE);
    // SetForegroundWindow(hWnd);
    // SetFocus(hWnd);
    // AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), nullptr), GetCurrentThreadId(), FALSE);
    // return GetForegroundWindow() == hWnd;
    pressKeyboard(VK_LMENU);
    Sleep(100);
    pressKeyboard(VK_TAB);
    Sleep(200);
    releaseKeyBoard(VK_TAB);
    Sleep(50);
    releaseKeyBoard(VK_LMENU);
    Sleep(500);
    return true;
}

BOOL CALLBACK sendTextEnumWindowCallback(HWND hWnd, LPARAM customMsg) {
    if(!customMsg || !hWnd) {
        return false;
    }
    const auto args = reinterpret_cast<EnumWindowArg*>(customMsg);
    if(!args->windowName || !args->sendMsg) {
        return false;
    }
    EnumChildWindows(hWnd, sendTextEnumWindowCallback, customMsg);
    const auto pBuffer = new wchar_t[0x1000]{};
    if(!pBuffer) {
        return false;
    }
    if(!args->isClassName) {
        if(!GetWindowTextW(hWnd, pBuffer, 0x1000)) {
            return true;
        }
    } else {
        if(!GetClassNameW(hWnd, pBuffer, 0x1000)) {
            return true;
        }
    }

    if(wcsstr(pBuffer, args->windowName)) {
        std::cout << "Steam chat wnd: " << hWnd << std::endl;

        if(!switchFocus(hWnd)) {
            return false;
        }

        // Sleep(1000);

        const auto msgLength = wcslen(args->sendMsg);
        const int  bufferSize = (static_cast<int>(msgLength) + 2) * 2;
        const auto hMemHandle = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT, bufferSize);
        const auto pMemData = static_cast<wchar_t*>(GlobalLock(hMemHandle));
        wcscpy_s(pMemData, bufferSize, args->sendMsg);

        GlobalUnlock(hMemHandle);

        OpenClipboard(nullptr);
        EmptyClipboard();
        SetClipboardData(CF_UNICODETEXT, hMemHandle);

        CloseClipboard();

        Sleep(500);
        pressKeyboard(VK_LCONTROL);
        pressKeyboard('V');
        Sleep(80);
        releaseKeyBoard('V');
        releaseKeyBoard(VK_LCONTROL);
        Sleep(500);
        clickKeyboard(VK_RETURN);
        Sleep(500);
        return false;
    }

    return true;
}

void postMessageToSteamChat(LPCWSTR msg) {
    const auto oldFocus = GetForegroundWindow();

    constexpr auto steamChatWindowName = L"蠢人";
    EnumWindowArg  enumArg{};
    enumArg.sendMsg = msg;
    enumArg.windowName = steamChatWindowName;
    EnumWindows(sendTextEnumWindowCallback, reinterpret_cast<LPARAM>(&enumArg));

    switchFocus(oldFocus);
}

static std::vector<wchar_t> captureGTA(HWND hWnd, float x = 0, float y = 0, float z = 0.5f, float w = 0.5f) {
    std::vector<wchar_t> resultWideChar;

    if(CaptureAnImage(hWnd, x, y, z, w)) {
        std::cout << "Error in OCR -1." << std::endl;
        return resultWideChar;
    }
    auto hRead = startOCR("RapidOCR-json.exe",
                          "--models=\".\\models\" "
                          "--det=ch_PP-OCRv4_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx "
                          "--rec=rec_ch_PP-OCRv4_infer.onnx  --keys=dict_chinese.txt --padding=60 "
                          "--maxSideLen=1024 --boxScoreThresh=0.5 --boxThresh=0.3 --unClipRatio=1.6 --doAngle=0 "
                          "--mostAngle=0 --numThread=1 --image_path=temp.png");
    auto ocrResult = readPipeAndClose(hRead);
    // std::cout<< ocrResult << std::endl;
    const auto completedPos = ocrResult.find("completed.");
    if(completedPos == std::string::npos) {
        std::cout << "Error in OCR 0." << std::endl;
        return resultWideChar;
    }

    ocrResult = ocrResult.substr(completedPos + strlen("completed."));
    Json::Reader reader;
    Json::Value  rootValue;
    if(!reader.parse(ocrResult, rootValue)) {
        std::cout << "Error in OCR 1." << std::endl;
        return resultWideChar;
    }
    if(rootValue["code"] != 100) {
        std::cout << "ORC code error" << std::endl;
        return resultWideChar;
    }
    const auto data = rootValue["data"];
    if(data.size() < 1) {
        std::cout << "ORC result size too small";
        return resultWideChar;
    }
    std::string resultText;
    for(unsigned int i = 0; i < data.size(); ++i) {
        resultText.append(data[i]["text"].asString());
    }
    std::cout << resultText << std::endl << std::endl;

    resultWideChar.resize((resultText.length() + 1) * 4);
    memset(resultWideChar.data(), 0, resultWideChar.size());

    MultiByteToWideChar(CP_UTF8, 0, resultText.data(), -1, resultWideChar.data(), static_cast<int>(resultWideChar.size()));
    return resultWideChar;
}

bool findText(HWND hWnd, LPCWSTR text, float x, float y, float z, float w) {
    const auto ocrResult = captureGTA(hWnd, x, y, z, w);
    return ocrResult.size() > 1 && wcsstr(ocrResult.data(), text);
}

bool isRespawned(HWND hWnd) {
    return findText(hWnd, L"床", 0, 0, 0.5f, 0.5f);
}

bool isOnJobPanel(HWND hWnd) {
    return findText(hWnd, L"别惹德瑞", 0, 0, 0.5f, 0.5f);
}

bool ocrFoundJob(HWND hWnd) {
    return findText(hWnd, L"猎杀", 0, 0, 0.5f, 0.5f);
}


bool suspendProcess(DWORD pid, DWORD time) {
    static DWORD (__stdcall*procZwSuspendProcess)(HANDLE hProc) = nullptr;
    static DWORD (__stdcall*procZwResumeProcess)(HANDLE hProc) = nullptr;

    if(!pid) {
        return false;
    }
    HANDLE hProc = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if(!hProc) {
        std::cout << "Can not open GTA process" << std::endl;
        return false;
    }

    if(!procZwSuspendProcess || !procZwResumeProcess) {
        const auto hNtdll = LoadLibraryW(L"ntdll.dll");
        if(!hNtdll) {
            std::cout << "Can not found NTDLL" << std::endl;
            CloseHandle(hProc);
            return false;
        }

        if(!procZwSuspendProcess) {
            reinterpret_cast<FARPROC&>(procZwSuspendProcess) = GetProcAddress(hNtdll, "ZwSuspendProcess");
            if(!procZwSuspendProcess) {
                std::cout << "Can not found ZwSuspendProcess" << std::endl;
                CloseHandle(hProc);
                return false;
            }
        }

        if(!procZwResumeProcess) {
            reinterpret_cast<FARPROC&>(procZwResumeProcess) = GetProcAddress(hNtdll, "ZwResumeProcess");
            if(!procZwResumeProcess) {
                std::cout << "Can not found ZwResumeProcess" << std::endl;
                CloseHandle(hProc);
                return false;
            }
        }
    }

    procZwSuspendProcess(hProc);
    Sleep(time);
    procZwResumeProcess(hProc);
    CloseHandle(hProc);
    return true;
}

void goDownstairs() {
    // 走到柱子上卡住 - start
    pressKeyboard('A');
    pressKeyboard('W');
    Sleep(2000);
    releaseKeyBoard('W');
    releaseKeyBoard('A');
    // 走到柱子上卡住 - end

    // 走到楼梯口 - start
    pressKeyboard('D');
    Sleep(8000);
    for(const auto startTime = GetTickCount64(); GetTickCount64() - startTime < 2500;) {
        clickKeyboard('S', 150);
    }
    Sleep(1500);
    releaseKeyBoard('D');

    // 走到楼梯口 - end

    // 走进楼梯门 - start
    pressKeyboard('W');
    Sleep(3000);
    releaseKeyBoard('W');
    // 走进楼梯门 - end

    // 走下楼梯 - start
    pressKeyboard('S');
    Sleep(4000);
    releaseKeyBoard('S');

    pressKeyboard('D');
    Sleep(2000);
    releaseKeyBoard('D');

    pressKeyboard('W');
    Sleep(3000);
    releaseKeyBoard('W');
    // 走下楼梯 - end
}


bool foundJob(HWND hWnd) {
    auto startTickCount = GetTickCount64();

    // 走出门 - start
    while(GetTickCount64() - startTickCount < 3500) {
        clickKeyboard('S', 500);
        clickKeyboard('A', 500);
    }
    // 走出门 - end

    // 卡在墙上 - start
    pressKeyboard('W');
    pressKeyboard('A');
    Sleep(1500);
    releaseKeyBoard('A');
    releaseKeyBoard('W');
    // 卡在墙上 - end

    // 走到任务附近 - start
    startTickCount = GetTickCount64();

    while(GetTickCount64() - startTickCount < 4000) {
        clickKeyboard('S', 360);
        clickKeyboard('A', 500);
    }
    // 走到任务附近 - end

    // OCR 走到黄圈 - start
    startTickCount = GetTickCount64();
    bool isJobFound = false;
    while(GetTickCount64() - startTickCount < 10000) {
        clickKeyboard('S', 320);
        Sleep(1000);
        if((isJobFound = ocrFoundJob(hWnd))) {
            break;
        }
        clickKeyboard('A', 500);
        Sleep(1000);
        if((isJobFound = ocrFoundJob(hWnd))) {
            break;
        }
    }
    // OCR 走到黄圈 - end
    return isJobFound;
}

bool newMatch(HWND hWnd) {
    clickKeyboard(VK_ESCAPE);
    Sleep(2000);

    auto ocrResult = captureGTA(hWnd);
    if(!findText(hWnd, L"地图", 0, 0, 0.5f, 0.5f)) {
        clickKeyboard(VK_ESCAPE);
        return false;
    }

    clickKeyboard('E');
    Sleep(2000);
    clickKeyboard(VK_RETURN);
    Sleep(1000);
    for(int i = 0; i < 5; ++i) {
        clickKeyboard('W', 100);
        Sleep(600);
    }
    clickKeyboard(VK_RETURN);
    Sleep(1000);

    if(!findText(hWnd, L"邀请", 0, 0, 0.5f, 0.5f)) {
        for(int i = 0; i < 3; ++i) {
            clickKeyboard(VK_ESCAPE);
            Sleep(1000);
        }
        return false;
    }

    clickKeyboard('S');
    Sleep(600);
    clickKeyboard(VK_RETURN);
    Sleep(2000);
    clickKeyboard(VK_RETURN);
    return true;
}

long countText(const std::vector<wchar_t>& searchText, LPCWSTR text) {
    long count = 0;

    if(searchText.size() <= 1) {
        return count;
    }

    auto searchPoint = searchText.data();

    while((searchPoint = wcsstr(searchPoint, text))) {
        searchPoint++;
        count++;
    }

    return count;
}

constexpr auto chatMsgFormatBufferSize = 100;

bool waitTeam(HWND hWnd) {
    constexpr auto exitMatchTime = 5 * 60 * 1000;
    constexpr auto joiningWaitTime = 2 * 60 * 1000;
    constexpr auto waitTime = 15 * 1000;
    const auto     startWaitTeamTickCount = GetTickCount64();

    bool result = false;

    LONGLONG lastActiveTime = 0;
    LONGLONG lastJoiningTime = 0;
    long     lastJoiningCount = 0;
    long     lastJoinedCount = 0;
    long     lastActivePlayerCount = 1;
    postMessageToSteamChat(L"新德瑞差事已启动，卡好CEO直接来");
    while(true) {
        Sleep(1000);
        const auto currentCheckTime = GetTickCount64();
        if(!isOnJobPanel(hWnd)) {
            break;
        }

        if(GetTickCount64() - startWaitTeamTickCount > exitMatchTime && !lastJoinedCount) {
            postMessageToSteamChat(L"启动任务超时，重新启动中");
            break;
        }

        if(GetTickCount64() - lastJoiningTime > joiningWaitTime && lastJoiningCount) {
            postMessageToSteamChat(L"任务中含有卡B，重新启动中");
            break;
        }

        const auto ocrResult = captureGTA(hWnd, .5f, 0.f, 1.f, 1.f);

        const auto joiningCount = countText(ocrResult, L"正在");
        std::cout << "Joining count: " << joiningCount << std::endl;
        const auto joinedCount = countText(ocrResult, L"已加");
        std::cout << "Joined count: " << joinedCount << std::endl;

        if(joiningCount != lastJoiningCount || joinedCount != lastJoinedCount) {
            if(joiningCount != lastJoiningCount) {
                lastJoiningTime = GetTickCount64();
            }

            lastJoiningCount = joiningCount;
            lastJoinedCount = joinedCount;
            lastActiveTime = currentCheckTime;

            const auto numActivePlayer = joinedCount + joiningCount + 1;
            if(lastActivePlayerCount != numActivePlayer) {
                lastActivePlayerCount = numActivePlayer;
                const auto chatMsg = new wchar_t[chatMsgFormatBufferSize]{};
                if(numActivePlayer < 4) {
                    swprintf_s(chatMsg, chatMsgFormatBufferSize, L"德瑞 %d=%d，卡好CEO直接来", numActivePlayer, 4 - numActivePlayer);
                } else {
                    swprintf_s(chatMsg, chatMsgFormatBufferSize, L"满了");
                }
                postMessageToSteamChat(chatMsg);
                delete[] chatMsg;
            }

            continue;
        }

        if(joinedCount == 3 || (currentCheckTime - lastActiveTime > waitTime && joinedCount > 0 && !joiningCount)) {
            clickKeyboard(VK_RETURN);
            if(!isOnJobPanel(hWnd)) {
                clickKeyboard(VK_RETURN);
                Sleep(1000);
                continue;
            }
            postMessageToSteamChat(L"开了，请等待下一辆车");
            result = true;
            break;
        }
    }
    return result;
}


int main() {
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    auto hWnd = FindWindowW(nullptr, L"Grand Theft Auto V");
    if(!hWnd) {
        std::cout << "Can not found GTA window" << std::endl;
        system("pause");
        return 0;
    }

    DWORD gtaPid = 0;
    GetWindowThreadProcessId(hWnd, &gtaPid);
    if(!gtaPid) {
        std::cout << "Can not lookup GTA process id" << std::endl;
        system("pause");
        return 0;
    }

    HWND focus;
    while((focus = GetForegroundWindow()) != hWnd) {
        std::cout << focus << std::endl;
        Sleep(1000);
    }

    while(true) {
        Sleep(1000);

        int newMatchErrorCount = 0;
        while(!newMatch(hWnd)) {
            newMatchErrorCount++;
            if(newMatchErrorCount % 3 == 2) {
                for(int i = 0; i < 7; ++i) {
                    clickKeyboard(VK_ESCAPE);
                    Sleep(500);
                }
            }
            Sleep(5000);
            std::cout << "Retry new match" << std::endl;
        }

        while(!isRespawned(hWnd)) {
            Sleep(300);
        }

        goDownstairs();

        const auto isJobFound = foundJob(hWnd);
        std::cout << "Found Job: " << isJobFound << std::endl;

        if(!isJobFound) {
            continue;
        }

        clickKeyboard('E');
        while(!isOnJobPanel(hWnd)) {
            Sleep(1000);
        }

        clickKeyboard('W');
        Sleep(800);
        clickKeyboard(VK_RETURN);
        Sleep(2000);
        clickKeyboard('A');
        Sleep(800);
        clickKeyboard('W');
        Sleep(500);

        const auto waitTeamResult = waitTeam(hWnd);
        if(!waitTeamResult) {
            clickKeyboard(VK_ESCAPE);
            Sleep(1000);
            clickKeyboard(VK_ESCAPE);
            Sleep(1000);
            clickKeyboard(VK_RETURN);
            continue;
        }

        while(isOnJobPanel(hWnd)) {
            Sleep(1000);
        }
        Sleep(10000);
        std::cout << "Suspend GTA process" << std::endl;
        suspendProcess(gtaPid, 10 * 1000);
        std::cout << "Resume GTA process" << std::endl;
    }
}
