#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <vector>
bool changeIcon(const std::string& exePath, const std::string& icoPath) {
    // ��Ŀ��EXE�ļ����޸���Դ
    HANDLE hUpdate = BeginUpdateResourceA(exePath.c_str(), FALSE);
    if (hUpdate == NULL) {
        std::cerr << "�޷����ļ���Դ: " << GetLastError() << std::endl;
        return false;
    }

    // ����ͼ���ļ�
    HANDLE hIcon = LoadImageA(NULL, icoPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (hIcon == NULL) {
        std::cerr << "����ͼ��ʧ��: " << GetLastError() << std::endl;
        EndUpdateResource(hUpdate, TRUE);
        return false;
    }

    // ��ͼ����ת��Ϊͼ��Ŀ¼�ṹ
    ICONINFO iconInfo;
    GetIconInfo((HICON)hIcon, &iconInfo);
    BITMAP bmpColor, bmpMask;
    GetObject(iconInfo.hbmColor, sizeof(BITMAP), &bmpColor);
    GetObject(iconInfo.hbmMask, sizeof(BITMAP), &bmpMask);

    DWORD dwBytesPerLine = ((bmpColor.bmWidth * bmpColor.bmBitsPixel + 31) / 32) * 4;
    DWORD dwIconImageSize = dwBytesPerLine * bmpColor.bmHeight + dwBytesPerLine * bmpMask.bmHeight;

    std::vector<BYTE> iconData(dwIconImageSize);
    GetBitmapBits(iconInfo.hbmColor, dwIconImageSize / 2, &iconData[0]);
    GetBitmapBits(iconInfo.hbmMask, dwIconImageSize / 2, &iconData[dwIconImageSize / 2]);

    // ������Դ
    if (!UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), iconData.data(), dwIconImageSize)) {
        std::cerr << "����ͼ����Դʧ��: " << GetLastError() << std::endl;
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        EndUpdateResource(hUpdate, TRUE);
        return false;
    }

    // ���������Դ����
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    if (!EndUpdateResource(hUpdate, FALSE)) {
        std::cerr << "�����Դ����ʧ��: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}
