#include "Logger.h"

#include "Global.h"
#include "conmanip.h"

#include <sstream>

Logger* GLogger;

using namespace conmanip;

conmanip::console_out_context* ctxOut = nullptr;
conmanip::console_out*         conOut = nullptr;
conmanip::console_in_context*  ctxIn  = nullptr;
conmanip::console_in*          conIn  = nullptr;

void Logger::PtEndLine() const
{
    conOut->setpos(0, conOut->getpos().Y);
    conOut->settextcolor(console_text_colors::light_white);
    std::wcout << L"\033[2K" << EndLineStr;
    conOut->resetcolors();
}

Logger::Logger()
{
    EndLineStr = L"";
    conOut     = new conmanip::console_out(*(ctxOut = new conmanip::console_out_context()));
    conOut->settitle(L"");
    Info(L"Logger started up");
    conIn = new conmanip::console_in(*(ctxIn = new conmanip::console_in_context));
    Info(L"I/O started up");
}

void Logger::SetEndLineStr(const std::wstring& endLineStr)
{
    EndLineStr.clear();
    EndLineStr.append(endLineStr);
    PtEndLine();
}

void Logger::Debug(const std::wstring& text) const
{
    conOut->settextcolor(console_text_colors::white);
    Prefix(L"[DEB ]", text);
    conOut->resetcolors();
}

void Logger::Info(const std::wstring& text) const
{
    conOut->settextcolor(console_text_colors::light_cyan);
    Prefix(L"[INFO]", text);
    conOut->resetcolors();
}


void Logger::BufInfo() const
{
    conOut->settextcolor(console_text_colors::light_cyan);
    Prefix(L"[INFO]", Buffer);
    conOut->resetcolors();
}

void Logger::Warn(const std::wstring& text) const
{
    conOut->settextcolor(console_text_colors::light_yellow);
    Prefix(L"[WARN]", text);
    conOut->resetcolors();
}

void Logger::Err(const std::wstring& text) const
{
    conOut->settextcolor(console_text_colors::light_red);
    Prefix(L"[ERR ]", text);
    conOut->resetcolors();
}

void Logger::Prefix(const std::wstring& prefix, const std::wstring& text) const
{
    std::wstring buffer;
    buffer.append(prefix);
    buffer.append(L" ");
    buffer.append(text);
    PtRaw(buffer);
}

void Logger::PtRaw(const std::wstring& text) const
{
    conOut->setpos(0, conOut->getpos().Y);
    std::wcout << L"\033[2K" << text << std::endl;
    PtEndLine();
}

void Logger::BufRaw() const
{
    conOut->resetcolors();
    PtRaw(Buffer);
}

std::vector<std::wstring> Logger::RdCommand() const
{
    const auto                line = RdRaw();
    std::vector<std::wstring> result;
    std::wstringstream        wss(line);
    std::wstring              tempItem;
    while (std::getline(wss, tempItem, L' ')) {
        result.emplace_back(tempItem);
    }
    return result;
}

std::wstring Logger::RdRaw() const
{
    std::wstring result;
    std::getline(std::wcin, result);
    PtEndLine();
    return result;
}