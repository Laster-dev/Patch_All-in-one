#pragma once
#include <string>
#include <Windows.h>
#include <iostream>
#include <vector>
bool changeIcon(const std::string& exePath, const std::string& icoPath) {
    // 打开目标EXE文件以修改资源
    HANDLE hUpdate = BeginUpdateResourceA(exePath.c_str(), FALSE);
    if (hUpdate == NULL) {
        std::cerr << "无法打开文件资源: " << GetLastError() << std::endl;
        return false;
    }

    // 加载图标文件
    HANDLE hIcon = LoadImageA(NULL, icoPath.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE);
    if (hIcon == NULL) {
        std::cerr << "加载图标失败: " << GetLastError() << std::endl;
        EndUpdateResource(hUpdate, TRUE);
        return false;
    }

    // 将图标句柄转换为图标目录结构
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

    // 更新资源
    if (!UpdateResource(hUpdate, RT_ICON, MAKEINTRESOURCE(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), iconData.data(), dwIconImageSize)) {
        std::cerr << "更新图标资源失败: " << GetLastError() << std::endl;
        DeleteObject(iconInfo.hbmColor);
        DeleteObject(iconInfo.hbmMask);
        EndUpdateResource(hUpdate, TRUE);
        return false;
    }

    // 清理并完成资源更新
    DeleteObject(iconInfo.hbmColor);
    DeleteObject(iconInfo.hbmMask);
    if (!EndUpdateResource(hUpdate, FALSE)) {
        std::cerr << "完成资源更新失败: " << GetLastError() << std::endl;
        return false;
    }

    return true;
}
