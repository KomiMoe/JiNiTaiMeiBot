#pragma once

#include <iostream>
#include <vector>

class Logger
{
public:
    wchar_t Buffer[10240];

    Logger();

public:
    void Debug(const std::wstring& text) const;
    void Info(const std::wstring& text) const;
    void BufInfo() const;
    void Warn(const std::wstring& text) const;
    void Err(const std::wstring& text) const;
    void Prefix(const std::wstring& prefix, const std::wstring& text) const;
    void PtRaw(const std::wstring& text) const;
    void BufRaw() const;

};