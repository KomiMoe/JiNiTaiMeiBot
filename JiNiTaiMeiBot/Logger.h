#pragma once

#include <iostream>
#include <vector>

class Logger
{
private:
    std::wstring EndLineStr;

    void PtEndLine() const;

public:
    wchar_t Buffer[10240];

    Logger();
    void SetEndLineStr(const std::wstring& endLineStr);

public:
    void Debug(const std::wstring& text) const;
    void Info(const std::wstring& text) const;
    void BufInfo() const;
    void Warn(const std::wstring& text) const;
    void Err(const std::wstring& text) const;
    void Prefix(const std::wstring& prefix, const std::wstring& text) const;
    void PtRaw(const std::wstring& text) const;
    void BufRaw() const;

public:
    std::vector<std::wstring> RdCommand() const;
    std::wstring              RdRaw() const;

};