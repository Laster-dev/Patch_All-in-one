#pragma once
//#include <dbghelp.h>
#include <imagehlp.h>
#pragma comment(lib, "Imagehlp.lib")

DWORD RemoveAuthenticodeSignature(LPCWSTR filePath)

{
    DWORD lastError = 0;
    std::unique_ptr<char[]> ansiFilePath;

    CString filePathStr(filePath); // 将 LPCWSTR 转换为 CString

    CNativePatchMFCDlg::LogMessage(_T("[+] Stripping file: ") + filePathStr);

    // 打开文件，使用读写权限
    HANDLE fileHandle = CreateFileW(
        filePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        0,
        nullptr
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        lastError = GetLastError();
        CNativePatchMFCDlg::LogMessage(_T("[-] CreateFile failed with error 0x") + CString(std::to_wstring(lastError).c_str()));
        return lastError;
    }

    // 尝试移除 Authenticode 证书
    if (!ImageRemoveCertificate(fileHandle, 0))
    {
        lastError = GetLastError();
        CNativePatchMFCDlg::LogMessage(_T("[-] ImageRemoveCertificate failed with error 0x") + CString(std::to_wstring(lastError).c_str()));

        if (lastError == ERROR_INVALID_PARAMETER)
        {
            CNativePatchMFCDlg::LogMessage(_T("[-] This happens when there's a listing in IMAGE_DIRECTORY_SECURITY\n")
                _T("in the PE's header, but the actual Authenticode signature has been stripped.\n")
                _T("Let's fix that ...\n"));

            // 关闭文件句柄并重新映射文件进行修改
            CloseHandle(fileHandle);
            fileHandle = nullptr;

            size_t ansiFilePathLength = wcslen(filePath) + 1;
            ansiFilePath = std::make_unique<char[]>(ansiFilePathLength);

            if (sprintf_s(ansiFilePath.get(), ansiFilePathLength, "%S", filePath) == -1)
            {
                lastError = GetLastError();
                CNativePatchMFCDlg::LogMessage(_T("[-] Failed to copy file path to ANSI string. GLE == 0x") + CString(std::to_wstring(lastError).c_str()));
                return lastError;
            }

            _LOADED_IMAGE loadedImage;
            if (MapAndLoad(ansiFilePath.get(), nullptr, &loadedImage, FALSE, FALSE))
            {
                CNativePatchMFCDlg::LogMessage(_T("[+] certificates->Size == 0x") + CString(std::to_wstring(loadedImage.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size).c_str()));
                CNativePatchMFCDlg::LogMessage(_T("[+] certificates->VA == 0x") + CString(std::to_wstring(loadedImage.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress).c_str()));

                // 设置大小和虚拟地址为0，以去除证书信息
                loadedImage.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].Size = 0;
                loadedImage.FileHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_SECURITY].VirtualAddress = 0;

                if (!UnMapAndLoad(&loadedImage))
                {
                    lastError = GetLastError();
                    CNativePatchMFCDlg::LogMessage(_T("[-] Failed to UnMapAndLoad. GLE == 0x") + CString(std::to_wstring(lastError).c_str()));
                    return lastError;
                }
            }
            else
            {
                lastError = GetLastError();
                CNativePatchMFCDlg::LogMessage(_T("[-] MapAndLoad failed. GLE == 0x") + CString(std::to_wstring(lastError).c_str()));
                return lastError;
            }
        }
    }

    if (fileHandle)
        CloseHandle(fileHandle);

    if (lastError == 0)
        CNativePatchMFCDlg::LogMessage(_T("[+] Succeeded."));

    return lastError;
}
void HandleError(DWORD errorCode, HANDLE& fileHandle, std::unique_ptr<char[]>& ansiFilePath)
{
    if (fileHandle != nullptr && fileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle(fileHandle);
    }

    if (ansiFilePath)
    {
        ansiFilePath.reset();
    }

    CNativePatchMFCDlg::LogMessage(_T("[-] Error occurred: GLE == 0x") + CString(std::to_wstring(errorCode).c_str()));
}
