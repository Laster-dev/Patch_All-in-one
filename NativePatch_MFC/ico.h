#include <windows.h>
#include <vector>
#include <string>
#include <fstream>
#include <tchar.h>

bool writeToFile(const std::string& filePath, const std::vector<char>& data) {
    std::ofstream outFile(filePath, std::ios::binary);
    if (!outFile.is_open()) {
        CNativePatchMFCDlg::LogMessage(_T("�޷����ļ���д�롣"));
        return false;
    }
    outFile.write(data.data(), data.size());
    outFile.close();
    return true;
}

bool readFromFile(const std::string& filePath, std::vector<char>& data) {
    std::ifstream inFile(filePath, std::ios::binary);
    if (!inFile.is_open()) {
        CNativePatchMFCDlg::LogMessage(_T("�޷����ļ��Զ�ȡ��"));
        return false;
    }
    data.assign((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
    inFile.close();
    return true;
}

HICON extractIconFromExe(const std::string& exePath) {
    // ��ȡEXE�еĵ�һ��ͼ��
    HICON hIcon = ExtractIconA(NULL, exePath.c_str(), 0);
    if (hIcon == NULL || hIcon == (HICON)1) { // ExtractIconA����1��ʾͼ�겻����
        CNativePatchMFCDlg::LogMessage(_T("�޷���EXE����ȡͼ�ꡣ"));
        return NULL;
    }
    return hIcon;
}

// ��std::stringת��Ϊstd::wstring
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
            // ���� .exe �ļ�
            HICON hIcon = ExtractIcon(AfxGetInstanceHandle(), lpszIco, 0);
            if (hIcon && hIcon != (HICON)1)
            {
                // ��ȡͼ����Ϣ
                ICONINFO iconInfo = { 0 };
                GetIconInfo(hIcon, &iconInfo);

                // ��ȡͼ������
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

                // ������Դ
                DestroyIcon(hIcon);
            }
            else
            {
                std::wcerr << L"Failed to extract icon from EXE file.\n";
            }
        }
        else
        {
            // ���� .ico �ļ�
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
        CNativePatchMFCDlg::LogMessage(_T("EXE�ļ�����Ϊ�ջ���Ч��"));
        return false;
    }
    // ������ʱ�ļ�·��
    std::string tempExePath = "temp_exe_file.exe";

    // ��EXE����д����ʱ�ļ�
    if (!writeToFile(tempExePath, *exeData)) {
        CNativePatchMFCDlg::LogMessage(_T("д����ʱEXE�ļ�ʧ�ܡ�"));
        return false;
    }
    ReplaceExeIco(StringToWString(tempExePath).c_str(), StringToWString(icoPath).c_str());
    // ����ʱ�ļ��ж�ȡ�޸ĺ��EXE����
    if (!readFromFile(tempExePath, *exeData)) {
        CNativePatchMFCDlg::LogMessage(_T("��ȡ�޸ĺ��EXE�ļ�ʧ�ܡ�"));
        DeleteFileA(tempExePath.c_str()); // ɾ����ʱ�ļ�
        return false;
    }

    // ɾ����ʱ�ļ�
    DeleteFileA(tempExePath.c_str());

    CNativePatchMFCDlg::LogMessage(_T("ͼ���ѳɹ����ġ�"));
    return true;
}
