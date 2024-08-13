#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <tchar.h>

bool writeToFile(const std::string& filePath, const std::vector<char>& data) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        CNativePatchMFCDlg::LogMessage(_T("无法打开文件以写入。"));
        return false;
    }
    outFile.write(data.data(), data.size());
    outFile.close();
    return true;
}

bool readFromFile(const std::string& filePath, std::vector<char>& data) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        CNativePatchMFCDlg::LogMessage(_T("无法打开文件以读取。"));
        return false;
    }
    data.assign((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    return true;
}

HICON extractIconFromExe(const std::string& exePath) {
    // 提取EXE中的第一个图标
    HICON hIcon = ExtractIconA(NULL, exePath.c_str(), 0);
    if (hIcon == NULL || hIcon == (HICON)1) { // ExtractIconA返回1表示图标不可用
        CNativePatchMFCDlg::LogMessage(_T("无法从EXE中提取图标。"));
        return NULL;
    }
    return hIcon;
}

// 将std::string转换为std::wstring
std::wstring StringToWString(const std::string& str) {
    int len;
    int slength = static_cast<int>(str.length()) + 1;
    len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, &r[0], len);
    return r;
}


void ReplaceExeIco(LPCTSTR lpszApp, LPCTSTR lpszIco)
{
    try
    {
        CString strIcoPath(lpszIco);
        CString strExeExtension = _T(".exe");

        if (strIcoPath.Right(4).CompareNoCase(strExeExtension) == 0)
        {
            // 处理 .exe 文件
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), lpszIco, 0);
            if (hIcon && hIcon != (HICON)1)
            {
                // 获取图标信息
                ICONINFO iconInfo = { 0 };
                GetIconInfo(hIcon, &iconInfo);

                // 获取图标数据
                std::vector<BYTE> iconData;
                HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, 0);
                if (hGlobal)
                {
                    HDC hdc = CreateCompatibleDC(NULL);
                    ICONINFOEXW iconInfoEx = { sizeof(ICONINFOEXW) };
                    if (GetIconInfoExW(hIcon, &iconInfoEx))
                    {
                        BITMAP bmpColor;
                        GetObject(iconInfoEx.hbmColor, sizeof(BITMAP), &bmpColor);

                        DWORD iconSize = bmpColor.bmWidthBytes * bmpColor.bmHeight * bmpColor.bmPlanes;
                        iconData.resize(iconSize);

                        // Get the icon data into the buffer
                        GetBitmapBits(iconInfoEx.hbmColor, iconSize, iconData.data());

                        HANDLE hUpdateRes = BeginUpdateResource(lpszApp, FALSE);
                        if (hUpdateRes)
                        {
                            UpdateResource(hUpdateRes, RT_ICON, MAKEINTRESOURCE(1), 0, iconData.data(), iconSize);
                            EndUpdateResource(hUpdateRes, FALSE);
                        }

                        DeleteObject(iconInfoEx.hbmColor);
                        DeleteObject(iconInfoEx.hbmMask);
                    }
                    DeleteDC(hdc);
                    GlobalFree(hGlobal);
                }

                // 清理资源
                DestroyIcon(hIcon);
            }
            else
            {
                std::wcerr << L"Failed to extract icon from EXE file.\n";
            }
        }
        else
        {
            // 处理 .ico 文件
            CFile file(lpszIco, CFile::modeRead);
            LONG lOffset = 0;
            DWORD dwSize = 0;
            CONST BYTE bGroupIcoSize = 0x18;
            std::vector<BYTE> lpGroupIco(bGroupIcoSize);
            file.Read(lpGroupIco.data(), 0x06);
            file.Read(lpGroupIco.data() + 0x08, 0x0C);
            memcpy(&dwSize, lpGroupIco.data() + 0x10, sizeof(DWORD));
            file.Read(&lOffset, sizeof(lOffset));

            std::vector<BYTE> lpRes(dwSize);
            file.Seek(lOffset, CFile::begin);
            file.Read(lpRes.data(), dwSize);

            HANDLE hApp = BeginUpdateResource(lpszApp, FALSE);
            UpdateResource(hApp, RT_GROUP_ICON, MAKEINTRESOURCE(1), 0, lpGroupIco.data(), bGroupIcoSize);
            UpdateResource(hApp, RT_ICON, MAKEINTRESOURCE(1), 0, lpRes.data(), dwSize);
            EndUpdateResource(hApp, FALSE);

            file.Close();
        }
    }
    catch (CException* e)
    {
        e->ReportError();
        e->Delete();
    }
}

bool changeIcon(std::vector<char>* exeData, const std::string& icoPath) {
    if (exeData == nullptr || exeData->empty()) {
        CNativePatchMFCDlg::LogMessage(_T("EXE文件数据为空或无效。"));
        return false;
    }
    // 创建临时文件路径
    std::string tempExePath = "temp_exe_file.exe";

    // 将EXE数据写入临时文件
    if (!writeToFile(tempExePath, *exeData)) {
        CNativePatchMFCDlg::LogMessage(_T("写入临时EXE文件失败。"));
        return false;
    }
    ReplaceExeIco(StringToWString(tempExePath).c_str(), StringToWString(icoPath).c_str());
    // 从临时文件中读取修改后的EXE数据
    if (!readFromFile(tempExePath, *exeData)) {
        CNativePatchMFCDlg::LogMessage(_T("读取修改后的EXE文件失败。"));
        DeleteFileA(tempExePath.c_str()); // 删除临时文件
        return false;
    }

    // 删除临时文件
    DeleteFileA(tempExePath.c_str());

    CNativePatchMFCDlg::LogMessage(_T("图标已成功更改。"));
    return true;
}
