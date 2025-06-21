#include "OCREngine.h"

#include "Global.h"
#include "Config.h"

#include <atlimage.h>
#include <filesystem>
#include <json/json.h>

OCREngine* GOCREngine = nullptr;

OCREngine::OCREngine()
{
    SECURITY_ATTRIBUTES sa;
    sa.nLength              = sizeof sa;
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = true;
    HANDLE hEngineWrite     = nullptr;
    if (!CreatePipe(&mHandleRead, &hEngineWrite, &sa, 0x1000)) {
        throw std::runtime_error("Can not create OCR read pipe.");
    }

    sa.nLength              = sizeof sa;
    sa.lpSecurityDescriptor = nullptr;
    sa.bInheritHandle       = true;
    HANDLE hEngineRead      = nullptr;
    if (!CreatePipe(&hEngineRead, &mHandleWrite, &sa, 0x1000)) {
        CloseHandle(hEngineWrite);
        CloseHandle(mHandleRead);
        throw std::runtime_error("Can not create OCR read pipe.");
    }

    STARTUPINFOA        si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof si;
    GetStartupInfoA(&si);
    si.hStdError   = hEngineWrite;
    si.hStdOutput  = hEngineWrite;
    si.hStdInput   = hEngineRead;
    si.wShowWindow = SW_HIDE;
    si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
    const std::filesystem::path executePath("RapidOCR-json.exe");

    std::string arg;
    arg.append("\"");
    arg.append(executePath.filename().string());
    arg.append("\"");
    arg.append(" ");
    arg.append(GConfig->ocrArgs);
    if (!CreateProcessA("RapidOCR-json.exe", arg.data(), nullptr, nullptr, true, NULL, nullptr, nullptr, &si, &pi)) {
        CloseHandle(hEngineWrite);
        CloseHandle(hEngineRead);
        CloseHandle(mHandleRead);
        CloseHandle(mHandleWrite);

        throw std::runtime_error("Can not create OCR process.");
    }
    CloseHandle(hEngineWrite);
    CloseHandle(hEngineRead);
    CloseHandle(pi.hThread);
    mHandleProcess = pi.hProcess;

    std::string startResult;
    char        buffer[0x1000]{};
    DWORD       nRead = 0;

    const auto currentTime = GetTickCount64();
    while (GetTickCount64() - currentTime < 5000) {
        if (!ReadFile(mHandleRead, buffer, sizeof(buffer) - 1, &nRead, nullptr)) {
            break;
        }
        startResult.append(buffer, nRead);
        nRead = 0;
        memset(buffer, 0, sizeof buffer);
        if (startResult.find("OCR init completed.") != std::string::npos) {
            return;
        }
    }
    swprintf_s(GLogger->Buffer, L"%hs", startResult.data());
    GLogger->Err(GLogger->Buffer);
    throw std::runtime_error("Can not start ocrUTF engine.");
}

bool OCREngine::writePipe(const std::string& text) const
{
    DWORD finalWrite = 0;
    DWORD numWrite   = 0;
    while (WriteFile(mHandleWrite, text.data() + finalWrite, static_cast<DWORD>(text.size()) - finalWrite, &numWrite, nullptr) && finalWrite < text.size()) {
        finalWrite += numWrite;
        numWrite = 0;
    }
    return finalWrite == text.size();
}

Json::Value OCREngine::ocrJson(HWND hWnd, float x, float y, float z, float w) const
{
    Json::Value resultJsonValue;

    const auto hdcDesktop = GetDC(nullptr);
    if (!hdcDesktop) {
        GLogger->Err(L"Can not get desktop HDC.");
        return resultJsonValue;
    }

    RECT rcClient;
    GetClientRect(hWnd, &rcClient);
    POINT ptWindow{};
    ClientToScreen(hWnd, &ptWindow);

    const int startX = static_cast<int>(ptWindow.x + x * rcClient.right);
    const int startY = static_cast<int>(ptWindow.y + y * rcClient.bottom);
    const int endX   = static_cast<int>(ptWindow.x + z * rcClient.right);
    const int endY   = static_cast<int>(ptWindow.y + w * rcClient.bottom);
    const int width  = endX - startX;
    const int height = endY - startY;

    CImage image;
    if (!image.Create(width, height, GetDeviceCaps(hdcDesktop, BITSPIXEL))) {
        std::cout << "Can not create image" << std::endl;
        return resultJsonValue;
    }

    StretchBlt(image.GetDC(), 0, 0, image.GetWidth(), image.GetHeight(), hdcDesktop, startX, startY, width, height, SRCCOPY);
    image.ReleaseDC();
    const auto pStream = SHCreateMemStream(nullptr, NULL);
    if (!pStream) {
        image.ReleaseGDIPlus();
        GLogger->Err(L"Can not create stream");
        return resultJsonValue;
    }

    if (FAILED(image.Save(pStream, Gdiplus::ImageFormatPNG))) {
        pStream->Release();
        image.ReleaseGDIPlus();
        GLogger->Err(L"Can not save image.");
        return resultJsonValue;
    }

    STATSTG stat{};
    if (FAILED(pStream->Stat(&stat, STATFLAG_NONAME))) {
        pStream->Release();
        image.ReleaseGDIPlus();
        GLogger->Err(L"Can not get buffer size.");
        return resultJsonValue;
    }

    LARGE_INTEGER li = {};
    if (FAILED(pStream->Seek(li, STREAM_SEEK_SET, nullptr))) {
        pStream->Release();
        image.ReleaseGDIPlus();
        GLogger->Err(L"Can not seek to buffer head.");
        return resultJsonValue;
    }

    const auto           size = static_cast<ULONG>(stat.cbSize.QuadPart);
    std::vector<uint8_t> imgBuffer;
    ULONG                bytesRead = 0;
    imgBuffer.resize(size);
    if (FAILED(pStream->Read(imgBuffer.data(), size, &bytesRead)) || bytesRead != size) {
        pStream->Release();
        image.ReleaseGDIPlus();
        GLogger->Err(L"Can not read buffer data.");
        return resultJsonValue;
    }

    pStream->Release();
    image.ReleaseGDIPlus();
    ReleaseDC(WindowFromDC(hdcDesktop), hdcDesktop);

    const auto base64Image = base64Encode(imgBuffer.data(), imgBuffer.size());
    if (!writePipe("{\"image_base64\": \"") ||
        !writePipe(base64Image) ||
        !writePipe("\"}\n")) {
        GLogger->Err(L"Can not write request to pipe.");
    }

    std::string       startResult;
    std::vector<char> buffer;
    buffer.resize(0x1000);
    memset(buffer.data(), 0, buffer.size());

    DWORD nRead = 0;

    Json::Reader reader;

    const auto currentTime = GetTickCount64();
    while (GetTickCount64() - currentTime < GConfig->ocrTimeout * 1000) {
        if (!ReadFile(mHandleRead, buffer.data(), static_cast<DWORD>(buffer.size() - 1), &nRead, nullptr)) {
            break;
        }
        startResult.append(buffer.data(), nRead);
        nRead = 0;
        memset(buffer.data(), 0, buffer.size());
        if (reader.parse(startResult, resultJsonValue)) {
            return resultJsonValue;
        }
    }
    GLogger->Err(L"OCR wait json time out.");
    return resultJsonValue;
}

std::vector<wchar_t> OCREngine::ocrUTF(HWND hWnd, float x, float y, float z, float w) const
{
    std::vector<wchar_t> resultWideChar;
    resultWideChar.resize(4);
    memset(resultWideChar.data(), 0, resultWideChar.size() * sizeof(wchar_t));

    const auto rootJsonValue = ocrJson(hWnd, x, y, z, w);
    if (!rootJsonValue["code"].isInt()) {
        GLogger->Err(L"Can not found code in result json");
        return resultWideChar;
    }

    if (rootJsonValue["code"] != 100) {
        if (!rootJsonValue["data"].isString()) {
            GLogger->Err(L"Can not found error data in result json");
            return resultWideChar;
        }
        swprintf_s(GLogger->Buffer, L"OCR error: %hs", rootJsonValue["data"].asString().data());
        GLogger->Err(GLogger->Buffer);
        return resultWideChar;
    }

    const auto data = rootJsonValue["data"];
    if (data.size() < 1) {
        if (GConfig->debug) {
            GLogger->Debug(L"ORC result size too small");
        }
        return resultWideChar;
    }
    std::string resultText;
    for (unsigned int i = 0; i < data.size(); ++i) {
        resultText.append(data[i]["text"].asString());
    }

    if (GConfig->debug) {
        swprintf_s(GLogger->Buffer, L"%hs", resultText.data());
        GLogger->Debug(GLogger->Buffer);
    }

    resultWideChar.resize((resultText.length() + 1) * 2);
    memset(resultWideChar.data(), 0, resultWideChar.size() * sizeof(wchar_t));

    MultiByteToWideChar(CP_UTF8, 0, resultText.data(), -1, resultWideChar.data(), static_cast<int>(resultWideChar.size() - 1));

    return resultWideChar;
}