#pragma once
#include <windows.h>


bool ConvertToGUI(const char* exePath) {
    HANDLE hFile = CreateFileA(exePath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        CNativePatchMFCDlg::LogMessage(_T("Failed to open file:") + (CString)exePath);
        return false;
    }

    DWORD bytesRead;
    IMAGE_DOS_HEADER dosHeader;
    if (!ReadFile(hFile, &dosHeader, sizeof(dosHeader), &bytesRead, NULL) || bytesRead != sizeof(dosHeader)) {
        CNativePatchMFCDlg::LogMessage(_T("Failed to read DOS header "));
        CloseHandle(hFile);
        return false;
    }

    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        CNativePatchMFCDlg::LogMessage(_T("Invalid DOS signature."));
        CloseHandle(hFile);
        return false;
    }

    if (SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {

        CNativePatchMFCDlg::LogMessage(_T("Failed to set file pointer"));
        CloseHandle(hFile);
        return false;
    }

    IMAGE_NT_HEADERS ntHeaders;
    if (!ReadFile(hFile, &ntHeaders, sizeof(ntHeaders), &bytesRead, NULL) || bytesRead != sizeof(ntHeaders)) {
        CNativePatchMFCDlg::LogMessage(_T("Failed to read NT headers"));
        CloseHandle(hFile);
        return false;
    }

    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        CNativePatchMFCDlg::LogMessage(_T("Invalid NT signature."));
        CloseHandle(hFile);
        return false;
    }

    ntHeaders.OptionalHeader.Subsystem = IMAGE_SUBSYSTEM_WINDOWS_GUI;

    if (SetFilePointer(hFile, dosHeader.e_lfanew, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER) {
        CNativePatchMFCDlg::LogMessage(_T("Failed to set file pointer."));
        CloseHandle(hFile);
        return false;
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile, &ntHeaders, sizeof(ntHeaders), &bytesWritten, NULL) || bytesWritten != sizeof(ntHeaders)) {
        CNativePatchMFCDlg::LogMessage(_T("Failed to write NT headers."));
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);

    return true;
}