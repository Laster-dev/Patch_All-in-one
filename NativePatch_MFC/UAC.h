#pragma once

#include <regex>
// 查找并替换字符串
bool FindAndReplace(std::string& source, const std::string& from, const std::string& to) {
    size_t pos = source.find(from);
    if (pos == std::string::npos) {
        return false;
    }
    source.replace(pos, from.length(), to);
    return true;
}

bool ModifyManifestResource(const std::wstring& exePath) {
    // 打开资源
    std::wcout << L"[+] : " << exePath << std::endl;
    HMODULE hModule = LoadLibraryExW(exePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule == NULL) {
        std::wcerr << L"[-] 无法加载可执行文件资源: " << exePath << std::endl;
        return false;
    }

    // 查找清单资源
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(1), RT_MANIFEST);
    if (hRes == NULL) {
        std::wcerr << L"[-] 无法找到清单资源: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // 加载资源
    HGLOBAL hResData = LoadResource(hModule, hRes);
    if (hResData == NULL) {
        std::wcerr << L"[-] 无法加载清单资源: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // 锁定资源以获取指针
    DWORD resSize = SizeofResource(hModule, hRes);
    const char* pResData = static_cast<const char*>(LockResource(hResData));
    if (pResData == NULL || resSize == 0) {
        std::wcerr << L"[-] 无法锁定清单资源: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // 将资源数据转换为 std::string 以便修改
    std::string manifestStr(pResData, resSize);

    // 修改清单内容
    bool modified = false;
    modified |= FindAndReplace(manifestStr, R"(level="requireAdministrator")", R"(level="asInvoker")");
    modified |= FindAndReplace(manifestStr, R"(level="highestAvailable")", R"(level="asInvoker")");

    if (!modified) {
        std::wcerr << L"[*] 清单资源没有需要修改的内容。" << std::endl;
        FreeLibrary(hModule);
        return true;  // 如果不需要修改，仍然视为成功
    }

    // 准备更新资源
    HANDLE hUpdate = BeginUpdateResourceW(exePath.c_str(), FALSE);
    if (hUpdate == NULL) {
        std::wcerr << L"[-] 无法打开资源进行更新: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // 更新清单资源
    if (!UpdateResourceW(hUpdate, RT_MANIFEST, MAKEINTRESOURCEW(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPVOID)manifestStr.c_str(), manifestStr.size())) {
        std::wcerr << L"[-] 无法更新清单资源: " << exePath << std::endl;
        EndUpdateResourceW(hUpdate, TRUE);  // 取消更新
        FreeLibrary(hModule);
        return false;
    }

    // 提交资源更新
    if (!EndUpdateResourceW(hUpdate, FALSE)) {
        std::wcerr << L"[-] 无法提交资源更新: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    std::wcout << L"[+] 清单资源已成功修改: " << exePath << std::endl;
    FreeLibrary(hModule);
    return true;
}

