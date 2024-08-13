#pragma once

#include <regex>
// ���Ҳ��滻�ַ���
bool FindAndReplace(std::string& source, const std::string& from, const std::string& to) {
    size_t pos = source.find(from);
    if (pos == std::string::npos) {
        return false;
    }
    source.replace(pos, from.length(), to);
    return true;
}

bool ModifyManifestResource(const std::wstring& exePath) {
    // ����Դ
    std::wcout << L"[+] : " << exePath << std::endl;
    HMODULE hModule = LoadLibraryExW(exePath.c_str(), NULL, LOAD_LIBRARY_AS_DATAFILE);
    if (hModule == NULL) {
        std::wcerr << L"[-] �޷����ؿ�ִ���ļ���Դ: " << exePath << std::endl;
        return false;
    }

    // �����嵥��Դ
    HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(1), RT_MANIFEST);
    if (hRes == NULL) {
        std::wcerr << L"[-] �޷��ҵ��嵥��Դ: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // ������Դ
    HGLOBAL hResData = LoadResource(hModule, hRes);
    if (hResData == NULL) {
        std::wcerr << L"[-] �޷������嵥��Դ: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // ������Դ�Ի�ȡָ��
    DWORD resSize = SizeofResource(hModule, hRes);
    const char* pResData = static_cast<const char*>(LockResource(hResData));
    if (pResData == NULL || resSize == 0) {
        std::wcerr << L"[-] �޷������嵥��Դ: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // ����Դ����ת��Ϊ std::string �Ա��޸�
    std::string manifestStr(pResData, resSize);

    // �޸��嵥����
    bool modified = false;
    modified |= FindAndReplace(manifestStr, R"(level="requireAdministrator")", R"(level="asInvoker")");
    modified |= FindAndReplace(manifestStr, R"(level="highestAvailable")", R"(level="asInvoker")");

    if (!modified) {
        std::wcerr << L"[*] �嵥��Դû����Ҫ�޸ĵ����ݡ�" << std::endl;
        FreeLibrary(hModule);
        return true;  // �������Ҫ�޸ģ���Ȼ��Ϊ�ɹ�
    }

    // ׼��������Դ
    HANDLE hUpdate = BeginUpdateResourceW(exePath.c_str(), FALSE);
    if (hUpdate == NULL) {
        std::wcerr << L"[-] �޷�����Դ���и���: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    // �����嵥��Դ
    if (!UpdateResourceW(hUpdate, RT_MANIFEST, MAKEINTRESOURCEW(1), MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPVOID)manifestStr.c_str(), manifestStr.size())) {
        std::wcerr << L"[-] �޷������嵥��Դ: " << exePath << std::endl;
        EndUpdateResourceW(hUpdate, TRUE);  // ȡ������
        FreeLibrary(hModule);
        return false;
    }

    // �ύ��Դ����
    if (!EndUpdateResourceW(hUpdate, FALSE)) {
        std::wcerr << L"[-] �޷��ύ��Դ����: " << exePath << std::endl;
        FreeLibrary(hModule);
        return false;
    }

    std::wcout << L"[+] �嵥��Դ�ѳɹ��޸�: " << exePath << std::endl;
    FreeLibrary(hModule);
    return true;
}

