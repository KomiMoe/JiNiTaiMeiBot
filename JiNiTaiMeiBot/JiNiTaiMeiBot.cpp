#include <iostream>
#include <filesystem>

#include <Windows.h>
#include <atlimage.h>

#include <json/json.h>
#include <curl/curl.h>

#include "Config.h"
#include "Global.h"
#include "OCREngine.h"

HWND  GGtaHWnd = nullptr;
DWORD GGtaPid  = NULL;

#pragma comment(lib, "ws2_32")
#pragma comment(lib, "crypt32")

struct EnumWindowArg
{
    LPCWSTR windowName  = nullptr;
    LPCWSTR sendMsg     = nullptr;
    bool    isClassName = false;
};

bool switchFocus(HWND hWnd)
{
    // const auto oldWnd = GetForegroundWindow();
    // AttachThreadInput(GetWindowThreadProcessId(oldWnd, nullptr), GetCurrentThreadId(), TRUE);
    // SwitchToThisWindow(hWnd, false);
    // ShowWindow(hWnd, SW_SHOW);
    // SetFocus(hWnd);
    // SetForegroundWindow(hWnd);
    // AttachThreadInput(GetWindowThreadProcessId(GetForegroundWindow(), nullptr), GetCurrentThreadId(), FALSE);
    // AttachThreadInput(GetWindowThreadProcessId(oldWnd, nullptr), GetCurrentThreadId(), FALSE);
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

BOOL CALLBACK sendTextEnumWindowCallback(HWND hWnd, LPARAM customMsg)
{
    if (!customMsg || !hWnd) {
        return false;
    }
    const auto args = reinterpret_cast<EnumWindowArg*>(customMsg);
    if (!args->windowName || !args->sendMsg) {
        return false;
    }
    EnumChildWindows(hWnd, sendTextEnumWindowCallback, customMsg);
    constexpr auto bufferSize = 512;
    wchar_t buffer[bufferSize]{};

    if (!args->isClassName) {
        if (!GetWindowTextW(hWnd, buffer, bufferSize - 1)) {
            return true;
        }
    } else {
        if (!GetClassNameW(hWnd, buffer, bufferSize - 1)) {
            return true;
        }
    }

    if (wcsstr(buffer, args->windowName)) {
        if (GConfig->debug) {
            swprintf_s(GLogger->Buffer, L"Steam char window handle: %p", hWnd);
            GLogger->Debug(GLogger->Buffer);
        }

        if (!switchFocus(hWnd)) {
            return false;
        }
        //Sleep(300);

        RECT rcClient;
        GetClientRect(hWnd, &rcClient);
        const auto targetX = rcClient.right / 2;
        const auto targetY = rcClient.bottom - 20;

        POINT pt = { targetX, targetY };
        ClientToScreen(hWnd, &pt);
        SetCursorPos(pt.x, pt.y);

        INPUT input      = { 0 };
        input.type       = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &input, sizeof(INPUT));

        Sleep(100);
        ZeroMemory(&input, sizeof(INPUT));
        input.type       = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &input, sizeof(INPUT));

        setClipboardText(args->sendMsg);

        Sleep(200);
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

void postMessageToSteamChat(LPCWSTR msg)
{
    const auto oldFocus = GetForegroundWindow();

    constexpr auto steamChatWindowName = L"蠢人";
    EnumWindowArg  enumArg{};
    enumArg.sendMsg    = msg;
    enumArg.windowName = steamChatWindowName;
    EnumWindows(sendTextEnumWindowCallback, reinterpret_cast<LPARAM>(&enumArg));

    switchFocus(oldFocus);
}

bool findText(HWND hWnd, LPCWSTR text, float x, float y, float z, float w)
{
    const auto ocrResult = GOCREngine->ocrUTF(hWnd, x, y, z, w);
    return ocrResult.size() > 1 && wcsstr(ocrResult.data(), text);
}

bool isRespawned(HWND hWnd)
{
    return findText(hWnd, L"床", 0, 0, 0.5f, 0.5f);
}

bool isOnJobPanel(HWND hWnd)
{
    return findText(hWnd, L"别惹德瑞", 0, 0, 0.5f, 0.5f);
}

bool ocrFoundJob(HWND hWnd)
{
    return findText(hWnd, L"猎杀", 0, 0, 0.5f, 0.5f);
}


bool suspendProcess(DWORD pid, DWORD time)
{
    static DWORD (__stdcall*procZwSuspendProcess)(HANDLE hProc) = nullptr;
    static DWORD (__stdcall*procZwResumeProcess)(HANDLE hProc)  = nullptr;

    if (!pid) {
        return false;
    }
    HANDLE hProc = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProc) {
        GLogger->Err(L"Can not open GTA process");
        return false;
    }

    if (!procZwSuspendProcess || !procZwResumeProcess) {
        const auto hNtdll = LoadLibraryW(L"ntdll.dll");
        if (!hNtdll) {
            GLogger->Err(L"Can not found NTDLL");
            CloseHandle(hProc);
            return false;
        }

        if (!procZwSuspendProcess) {
            reinterpret_cast<FARPROC&>(procZwSuspendProcess) = GetProcAddress(hNtdll, "ZwSuspendProcess");
            if (!procZwSuspendProcess) {
                GLogger->Err(L"Can not found ZwSuspendProcess");
                CloseHandle(hProc);
                return false;
            }
        }

        if (!procZwResumeProcess) {
            reinterpret_cast<FARPROC&>(procZwResumeProcess) = GetProcAddress(hNtdll, "ZwResumeProcess");
            if (!procZwResumeProcess) {
                GLogger->Err(L"Can not found ZwResumeProcess");
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

void goDownstairs()
{
    // 走到柱子上卡住 - start
    pressKeyboard('A');
    pressKeyboard('W');
    Sleep(2000);
    releaseKeyBoard('W');
    releaseKeyBoard('A');
    // 走到柱子上卡住 - end

    // 走到楼梯口 - start
    pressKeyboard('D');
    Sleep(6000);
    for (const auto startTime = GetTickCount64(); GetTickCount64() - startTime < 2500;) {
        clickKeyboard('S', 150);
        clickKeyboard('W', 150);
    }
    Sleep(2000);
    releaseKeyBoard('D');

    // 走到楼梯口 - end

    // 走进楼梯门 - start
    pressKeyboard('W');
    Sleep(2000);
    releaseKeyBoard('W');
    // 走进楼梯门 - end

    // 走下楼梯 - start
    pressKeyboard('S');
    Sleep(4000);
    releaseKeyBoard('S');

    pressKeyboard('D');
    pressKeyboard('W');
    Sleep(2000);
    releaseKeyBoard('W');
    releaseKeyBoard('D');

    pressKeyboard('A');
    Sleep(5000);
    releaseKeyBoard('A');
    // 走下楼梯 - end
}


bool foundJob(HWND hWnd)
{
    auto startTickCount = GetTickCount64();

    // 走出门 - start
    while (GetTickCount64() - startTickCount < GConfig->goOutStairsTime) {
        clickKeyboard('S', GConfig->pressSTimeStairs);
        clickKeyboard('A', GConfig->pressATimeStairs);
    }
    // 走出门 - end

    // 走到任务附近 - start
    startTickCount = GetTickCount64();

    while (GetTickCount64() - startTickCount < GConfig->crossAisleTime) {
        clickKeyboard('S', GConfig->pressSTimeAisle);
        clickKeyboard('A', GConfig->pressATimeAisle);
    }
    // 走到任务附近 - end

    // OCR 走到黄圈 - start
    startTickCount  = GetTickCount64();
    bool isJobFound = false;
    while (GetTickCount64() - startTickCount < GConfig->waitFindJobTimeout) {
        clickKeyboard('S', GConfig->pressSTimeGoJob);
        //Sleep(300);
        if ((isJobFound = ocrFoundJob(hWnd))) {
            break;
        }
        clickKeyboard('A', GConfig->pressATimeGoJob);
        //Sleep(300);
        if ((isJobFound = ocrFoundJob(hWnd))) {
            break;
        }
    }
    // OCR 走到黄圈 - end
    return isJobFound;
}

bool newMatch(HWND hWnd)
{
    clickKeyboard(VK_ESCAPE);
    Sleep(2000);

    if (!findText(hWnd, L"地图", 0, 0, 0.5f, 0.5f)) {
        clickKeyboard(VK_ESCAPE);
        return false;
    }

    clickKeyboard('E');
    Sleep(2000);
    clickKeyboard(VK_RETURN);
    Sleep(1000);
    for (int i = 0; i < 5; ++i) {
        clickKeyboard('W', 100);
        Sleep(600);
    }
    clickKeyboard(VK_RETURN);
    Sleep(1000);

    if (!findText(hWnd, L"邀请", 0, 0, 0.5f, 0.5f)) {
        for (int i = 0; i < 3; ++i) {
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

long countText(const std::vector<wchar_t>& searchText, LPCWSTR text)
{
    long count = 0;

    if (searchText.size() <= 1) {
        return count;
    }

    auto searchPoint = searchText.data();

    while ((searchPoint = wcsstr(searchPoint, text))) {
        searchPoint++;
        count++;
    }

    return count;
}

constexpr auto chatMsgFormatBufferSize = 100;

bool waitTeam(HWND hWnd)
{
    const auto startWaitTeamTickCount = GetTickCount64();

    bool result = false;

    LONGLONG lastActiveTime        = 0;
    LONGLONG lastJoiningTime       = 0;
    long     lastJoiningCount      = 0;
    long     lastJoinedCount       = 0;
    long     lastActivePlayerCount = 1;
    postMessageToSteamChat(L"德瑞差事已启动，卡好CEO直接来");
    while (true) {
        Sleep(GConfig->checkLoopTime * 1000);
        const auto currentCheckTime = GetTickCount64();
        if (!isOnJobPanel(hWnd)) {
            break;
        }

        if (GetTickCount64() - startWaitTeamTickCount > GConfig->matchPanelTimeout * 1000 && !lastJoinedCount) {
            postMessageToSteamChat(L"启动任务超时，重新启动中");
            break;
        }

        if (GetTickCount64() - lastJoiningTime > GConfig->joiningPlayerKick * 1000 && lastJoiningCount) {
            postMessageToSteamChat(L"任务中含有卡B，重新启动中");
            break;
        }

        const auto ocrResult = GOCREngine->ocrUTF(hWnd, .5f, 0.f, 1.f, 1.f);

        const auto joiningCount = countText(ocrResult, L"正在");
        swprintf_s(GLogger->Buffer, L"Joining count: %d", joiningCount);
        GLogger->Info(GLogger->Buffer);
        const auto joinedCount = countText(ocrResult, L"已加");
        swprintf_s(GLogger->Buffer, L"Joined count: %d", joinedCount);
        GLogger->Info(GLogger->Buffer);

        if (joiningCount != lastJoiningCount || joinedCount != lastJoinedCount) {
            if (joiningCount != lastJoiningCount) {
                lastJoiningTime = GetTickCount64();
            }

            lastJoiningCount = joiningCount;
            lastJoinedCount  = joinedCount;
            lastActiveTime   = currentCheckTime;

            const auto numActivePlayer = joinedCount + joiningCount + 1;
            if (lastActivePlayerCount != numActivePlayer) {
                lastActivePlayerCount = numActivePlayer;
                if (numActivePlayer >= 4) {
                    postMessageToSteamChat(L"满了");
                }
            }

            continue;
        }

        if ((joinedCount == 3 && GConfig->startOnAllJoined) || (currentCheckTime - lastActiveTime > GConfig->startMatchDelay * 1000 && joinedCount > 0 && !joiningCount)) {
            clickKeyboard(VK_RETURN);
            Sleep(1000);
            if (!isOnJobPanel(hWnd)) {
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

static size_t curlWriteCallback(void* contents, size_t size, size_t nBlock, void* pUser) {
    static_cast<std::string*>(pUser)->append(static_cast<char*>(contents), size * nBlock);
    return size * nBlock;
}

void tryToJoinBot()
{
    const auto pCurl = curl_easy_init();
    if (!pCurl) {
        GLogger->Err(L"Can not init curl.");
        system("pause");
        return;
    }
    std::string quellBotList;
    curl_easy_setopt(pCurl, CURLOPT_URL, "http://quellgtacode.mageangela.cn:52014/botJvp/");
    curl_easy_setopt(pCurl, CURLOPT_WRITEFUNCTION, curlWriteCallback);
    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, &quellBotList);
    const auto curlResult = curl_easy_perform(pCurl);
    if (curlResult != CURLE_OK) {
        swprintf_s(GLogger->Buffer, L"Can not get bot list: %hs", curl_easy_strerror(curlResult));
        GLogger->Err(GLogger->Buffer);
        system("pause");
        return;
    }
    swprintf_s(GLogger->Buffer, L"%hs", quellBotList.data());
    GLogger->BufInfo();

    const auto botLines = split(replaceAll(quellBotList, "\r", ""), "\n");

    for (int i = 3; i < botLines.size(); ++i) {
        const auto botJvp = botLines[i];
        if (botJvp.find("|") == std::string::npos) {
            continue;
        }
        const auto name2Jvp = split(botJvp, "|");
        if (name2Jvp.size() < 2) {
            continue;
        }
        auto steamURL = "steam://rungame/3240220/76561199074735990/-steamjvp%3D" + name2Jvp[1];
        ShellExecuteA(nullptr, "open", steamURL.c_str(), nullptr, nullptr, SW_SHOW);
        Sleep(3000);
        const auto startJoinTime = GetTickCount64();
        while (GetTickCount64() - startJoinTime < 60 * 1000) {
            clickKeyboard(VK_RETURN);
            clickKeyboard('Z');
            Sleep(1000);
            if (findText(GGtaHWnd, L"在线模式", 0, 0, 0.5f, 0.5f)) {
                return;
            }
        }
    }
}

int main(int argc, const char** argv)
{
    SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);

    const auto hStdIn = GetStdHandle(STD_INPUT_HANDLE);
    DWORD      mode;
    GetConsoleMode(hStdIn, &mode);
    mode &= ~ENABLE_QUICK_EDIT_MODE;
    mode &= ~ENABLE_INSERT_MODE;
    mode &= ~ENABLE_MOUSE_INPUT;
    SetConsoleMode(hStdIn, mode);
    GLogger = new Logger();
    GLogger->Info(L"Console logger initialized.");

    std::string configFileName = "config.json";

    if (argc > 1) {
        configFileName = argv[1];
    }

    swprintf_s(GLogger->Buffer, L"Starting up with %hs ...", configFileName.data());
    GLogger->BufInfo();

    try {
        GConfig = new Config(configFileName);
    } catch (const std::runtime_error& err) {
        swprintf_s(GLogger->Buffer, L"Exception on read config:  %hs ...", err.what());
        GLogger->Err(GLogger->Buffer);
        return 0;
    }
    GLogger->Info(L"Config initialized.");
    GLogger->Info(L"Initializing OCR engine...");

    try {
        GOCREngine = new OCREngine();
    } catch (const std::runtime_error& err) {
        swprintf_s(GLogger->Buffer, L"Exception on start ocr engine:  %hs ...", err.what());
        GLogger->Err(GLogger->Buffer);
        return 0;
    }

    GGtaHWnd = FindWindowW(nullptr, L"Grand Theft Auto V");
    if (!GGtaHWnd) {
        GLogger->Err(L"Can not found GTA window");
        system("pause");
        return 0;
    }

    GetWindowThreadProcessId(GGtaHWnd, &GGtaPid);
    if (!GGtaPid) {
        GLogger->Err(L"Can not lookup GTA process id");
        system("pause");
        return 0;
    }

    HWND focus;
    while ((focus = GetForegroundWindow()) != GGtaHWnd) {
        swprintf_s(GLogger->Buffer, L"Waiting for switch to GTA window. Current window: %p", focus);
        GLogger->Info(GLogger->Buffer);
        Sleep(1000);
    }

    while (true) {
        Sleep(1000);

        int newMatchErrorCount = 0;
        while (!newMatch(GGtaHWnd)) {
            newMatchErrorCount++;
            if (newMatchErrorCount % 3 == 2) {
                GLogger->Warn(L"Try to back to normal status");
                for (int i = 0; i < 7; ++i) {
                    clickKeyboard(VK_ESCAPE);
                    Sleep(500);
                }
            }
            Sleep(5000);
            GLogger->Warn(L"Retry start a new match");
        }

        while (!isRespawned(GGtaHWnd)) {
            Sleep(300);
        }

        goDownstairs();

        const auto isJobFound = foundJob(GGtaHWnd);
        swprintf_s(GLogger->Buffer, L"Find job result: %x", isJobFound);
        GLogger->Info(GLogger->Buffer);

        if (!isJobFound) {
            continue;
        }

        clickKeyboard('E');
        while (!isOnJobPanel(GGtaHWnd)) {
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

        const auto waitTeamResult = waitTeam(GGtaHWnd);
        if (!waitTeamResult) {
            clickKeyboard(VK_ESCAPE);
            Sleep(1000);
            clickKeyboard(VK_ESCAPE);
            Sleep(1000);
            clickKeyboard(VK_RETURN);
            continue;
        }
        const auto matchStartTime = GetTickCount();

        while (isOnJobPanel(GGtaHWnd) && GetTickCount() - matchStartTime < static_cast<unsigned int>(GConfig->exitMatchTimeout) * 1000) {
            Sleep(1000);
        }

        if (GConfig->suspendAfterMatchStarted) {
            while (GetTickCount() - matchStartTime < static_cast<unsigned int>(GConfig->exitMatchTimeout) * 1000) {
                clickKeyboard('Z');
                Sleep(1000);
                const auto ocrResult = GOCREngine->ocrUTF(GGtaHWnd, 0, 0, .5f, 1.f);

                if (wcsstr(ocrResult.data(), L"德瑞") ||
                    wcsstr(ocrResult.data(), L"前往") ||
                    wcsstr(ocrResult.data(), L"困难") ||
                    wcsstr(ocrResult.data(), L"简单") ||
                    wcsstr(ocrResult.data(), L"普通") ||
                    wcsstr(ocrResult.data(), L"在线")
                ) {
                    break;
                }
            }
        }

        if (GetTickCount() - matchStartTime < static_cast<unsigned int>(GConfig->exitMatchTimeout) * 1000) {
            Sleep(GConfig->delaySuspendTime * 1000);
            GLogger->Info(L"Suspend GTA process");
            suspendProcess(GGtaPid, GConfig->suspendGTATime * 1000);
            GLogger->Info(L"Resume GTA process");
            continue;
        }

        while (GetTickCount() - matchStartTime < static_cast<unsigned int>(GConfig->exitMatchTimeout) * 1000) {
            clickKeyboard('Z');
            Sleep(1000);
            const auto ocrResult = GOCREngine->ocrUTF(GGtaHWnd, 0, 0, .5f, 1.f);

            if (wcsstr(ocrResult.data(), L"德瑞") ||
                wcsstr(ocrResult.data(), L"前往") ||
                wcsstr(ocrResult.data(), L"困难") ||
                wcsstr(ocrResult.data(), L"简单") ||
                wcsstr(ocrResult.data(), L"普通") ||
                wcsstr(ocrResult.data(), L"在线")
                ) {
                break;
            }
        }

        tryToJoinBot();
    }
}