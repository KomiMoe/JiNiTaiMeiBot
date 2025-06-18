#include <iostream>
#include <filesystem>

#include <Windows.h>

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

bool clickKeyboard(byte key, unsigned long milliSecond = 60) {
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
    if(!CreatePipe(&hRead, &hWrite, &sa, NULL)) {
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

// Code from Microsoft
int CaptureAnImage(HWND hWnd, float x, float y, float z, float w) {
    HBITMAP hbmScreen = nullptr;
    BITMAP  bmpScreen;
    DWORD   dwBytesWritten = 0;
    DWORD   dwSizeofDIB = 0;
    HANDLE  hFile = nullptr;
    char*   lpBitmap = nullptr;
    HANDLE  hDIB = nullptr;
    DWORD   dwBmpSize = 0;

    // Retrieve the handle to a display device context for the client 
    // area of the window. 
    const auto hdcScreen = GetDC(nullptr);
    const auto hdcWindow = GetDC(hWnd);


    // Get the client area for size calculation.
    RECT rcClient;
    GetClientRect(hWnd, &rcClient);

    LONG startX = static_cast<LONG>(static_cast<float>(rcClient.right) * x);
    LONG startY = static_cast<LONG>(static_cast<float>(rcClient.bottom) * y);
    LONG endX = static_cast<LONG>(static_cast<float>(rcClient.right) * z);
    LONG endY = static_cast<LONG>(static_cast<float>(rcClient.bottom) * w);

    // Create a compatible DC, which is used in a BitBlt from the window DC.
    const auto hdcMemDC = CreateCompatibleDC(hdcWindow);

    if(!hdcMemDC) {
        MessageBox(hWnd, L"CreateCompatibleDC has failed", L"Failed", MB_OK);
        goto done;
    }

    // This is the best stretch mode.
    SetStretchBltMode(hdcWindow, HALFTONE);

    // The source DC is the entire screen, and the destination DC is the current window (HWND).
    if(!StretchBlt(hdcWindow,
                   0,
                   0,
                   rcClient.right,
                   rcClient.bottom,
                   hdcScreen,
                   0,
                   0,
                   GetSystemMetrics(SM_CXSCREEN),
                   GetSystemMetrics(SM_CYSCREEN),
                   SRCCOPY)) {
        MessageBox(hWnd, L"StretchBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Create a compatible bitmap from the Window DC.
    hbmScreen = CreateCompatibleBitmap(hdcWindow, endX - startX, endY - startY);

    if(!hbmScreen) {
        MessageBox(hWnd, L"CreateCompatibleBitmap Failed", L"Failed", MB_OK);
        goto done;
    }

    // Select the compatible bitmap into the compatible memory DC.
    SelectObject(hdcMemDC, hbmScreen);

    // Bit block transfer into our compatible memory DC.
    if(!BitBlt(hdcMemDC,
               0,
               0,
               endX - startX,
               endY - startY,
               hdcWindow,
               startX,
               startY,
               SRCCOPY)) {
        MessageBox(hWnd, L"BitBlt has failed", L"Failed", MB_OK);
        goto done;
    }

    // Get the BITMAP from the HBITMAP.
    GetObject(hbmScreen, sizeof(BITMAP), &bmpScreen);

    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bmpScreen.bmWidth;
    bi.biHeight = bmpScreen.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    dwBmpSize = ((bmpScreen.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.
    hDIB = GlobalAlloc(GHND, dwBmpSize);
    lpBitmap = (char*)GlobalLock(hDIB);

    // Gets the "bits" from the bitmap, and copies them into a buffer 
    // that's pointed to by lpbitmap.
    GetDIBits(hdcWindow,
              hbmScreen,
              0,
              (UINT)bmpScreen.bmHeight,
              lpBitmap,
              (BITMAPINFO*)&bi,
              DIB_RGB_COLORS);

    // A file is created, this is where we will save the screen capture.
    hFile = CreateFile(L"captureqwsx.bmp",
                       GENERIC_WRITE,
                       0,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);

    // Add the size of the headers to the size of the bitmap to get the total file size.
    dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpBitmap, dwBmpSize, &dwBytesWritten, NULL);

    // Unlock and Free the DIB from the heap.
    GlobalUnlock(hDIB);
    GlobalFree(hDIB);

    // Close the handle for the file that was created.
    CloseHandle(hFile);

    // Clean up.
done:
    DeleteObject(hbmScreen);
    DeleteObject(hdcMemDC);
    ReleaseDC(NULL, hdcScreen);
    ReleaseDC(hWnd, hdcWindow);

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

static std::vector<wchar_t> captureGTA(HWND hWnd, float x = 0, float y = 0, float z = 0.5f, float w = 0.5f) {
    std::vector<wchar_t> resultWideChar;

    CaptureAnImage(hWnd, x, y, z, w);
    auto hRead = startOCR("RapidOCR-json.exe",
                          "--models=\".\\models\" "
                          "--det=ch_PP-OCRv4_det_infer.onnx --cls=ch_ppocr_mobile_v2.0_cls_infer.onnx "
                          "--rec=rec_ch_PP-OCRv4_infer.onnx  --keys=dict_chinese.txt --padding=60 "
                          "--maxSideLen=1024 --boxScoreThresh=0.5 --boxThresh=0.3 --unClipRatio=1.6 --doAngle=0 "
                          "--mostAngle=0 --numThread=1 --image_path=captureqwsx.bmp");
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

    static DWORD(__stdcall* procZwSuspendProcess)(HANDLE hProc) = nullptr;
    static DWORD(__stdcall* procZwResumeProcess)(HANDLE hProc) = nullptr;

    if(!pid) {
        return false;
    }
    HANDLE hProc = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pid);
    if (!hProc) {
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
    Sleep(1500);
    releaseKeyBoard('W');
    releaseKeyBoard('A');
    // 走到柱子上卡住 - end

    // 走到楼梯口 - start
    pressKeyboard('D');
    Sleep(10000);
    releaseKeyBoard('D');
    // 走到楼梯口 - end

    // 走进楼梯门 - start
    pressKeyboard('W');
    Sleep(2000);
    releaseKeyBoard('W');
    // 走进楼梯门 - end

    // 走下楼梯 - start
    pressKeyboard('S');
    Sleep(3000);
    releaseKeyBoard('S');

    pressKeyboard('D');
    Sleep(1000);
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

    while(GetTickCount64() - startTickCount < 6000) {
        clickKeyboard('S', 350);
        clickKeyboard('A', 500);
    }
    // 走到任务附近 - end

    // OCR 走到黄圈 - start
    startTickCount = GetTickCount64();
    bool isJobFound = false;
    while(GetTickCount64() - startTickCount < 10000) {
        clickKeyboard('S', 350);
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
    Sleep(500);

    auto ocrResult = captureGTA(hWnd);
    if(!findText(hWnd, L"地图", 0, 0, 0.5f, 0.5f)) {
        clickKeyboard(VK_ESCAPE);
        return false;
    }

    clickKeyboard('E');
    Sleep(500);
    clickKeyboard(VK_RETURN);
    Sleep(200);
    for(int i = 0; i < 5; ++i) {
        clickKeyboard('W');
        Sleep(200);
    }
    clickKeyboard(VK_RETURN);
    Sleep(500);

    if(!findText(hWnd, L"邀请", 0, 0, 0.5f, 0.5f)) {
        clickKeyboard(VK_ESCAPE);
        return false;
    }

    clickKeyboard('S');
    Sleep(200);
    clickKeyboard(VK_RETURN);
    Sleep(300);
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

bool waitTeam(HWND hWnd) {
    constexpr auto exitMatchTime = 5 * 60 * 1000;
    constexpr auto waitTime = 10 * 1000;
    const auto startWaitTeamTickCount = GetTickCount64();

    bool result = false;

    LONGLONG lastActiveTime = 0;
    long lastJoiningCount = 0;
    long lastJoinedCount = 0;
    while(true) {
        Sleep(1000);
        const auto currentCheckTime = GetTickCount64();
        if(!isOnJobPanel(hWnd)) {
            break;
        }

        if(GetTickCount64() - startWaitTeamTickCount > exitMatchTime) {
            break;
        }

        const auto ocrResult = captureGTA(hWnd, 0.25, 0, 0.75, 0.5);
        const auto hostCount = countText(ocrResult, L"主持");
        std::cout << "Host count: " << hostCount << std::endl;

        if(hostCount < 1) {
            break;
        }

        const auto joiningCount = countText(ocrResult, L"正在");
        std::cout << "Joining count: " << joiningCount << std::endl;
        const auto joinedCount = countText(ocrResult, L"已加");
        std::cout << "Joined count: " << joinedCount << std::endl;

        if(joiningCount != lastJoiningCount || joinedCount != lastJoinedCount) {
            lastJoiningCount = joiningCount;
            lastJoinedCount = joinedCount;

            lastActiveTime = currentCheckTime;
            continue;
        }

        if(joinedCount == 3 || (currentCheckTime - lastActiveTime > waitTime && joinedCount > 0 && !joiningCount)) {
            clickKeyboard(VK_RETURN);
            Sleep(1000);
            if(!isOnJobPanel(hWnd)) {
                clickKeyboard(VK_RETURN);
                Sleep(1000);
                continue;
            }
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
    if (!gtaPid) {
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

        while(!newMatch(hWnd)) {
            Sleep(1000);
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
        Sleep(200);
        clickKeyboard(VK_RETURN);
        Sleep(500);
        clickKeyboard('A');
        Sleep(200);
        clickKeyboard('W');

        const auto waitTeamResult = waitTeam(hWnd);
        if(!waitTeamResult) {
            clickKeyboard(VK_ESCAPE);
            Sleep(500);
            clickKeyboard(VK_ESCAPE);
            Sleep(500);
            clickKeyboard(VK_RETURN);
            continue;
        }

        while(isOnJobPanel(hWnd)) {
            Sleep(1000);
        }
        Sleep(5000);
        suspendProcess(gtaPid, 10 * 1000);
    }
}
