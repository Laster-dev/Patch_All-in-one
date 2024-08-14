////////////////////////////////////////////////////////////////////////////
// ExeIconReplacer.cpp -- replace icon of exe file
// Copyright (C) 2015 Katayama Hirofumi MZ.  All rights reserved.
////////////////////////////////////////////////////////////////////////////
#include "pch.h"
#include "ico.h"

////////////////////////////////////////////////////////////////////////////
// ExeIconReplacer

bool ExeIconReplacer::LoadIconFile(LPCTSTR pszFileName) {
    bool bOK = false;

    HANDLE hFile = ::CreateFile(pszFileName, GENERIC_READ,
        0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    assert(hFile != INVALID_HANDLE_VALUE);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD dwRead;
    if (::ReadFile(hFile, &m_dir, sizeof(ICONDIR), &dwRead, NULL)) {
        assert(IsIconDirOK());
        if (IsIconDirOK()) {
            m_entry = new ICONDIRENTRY[m_dir.idCount];
            bOK = true;
            for (int i = 0; i < m_dir.idCount; i++) {
                if (!::ReadFile(hFile, &m_entry[i],
                    sizeof(ICONDIRENTRY), &dwRead, NULL))
                {
                    bOK = false;
                    assert(0);
                    break;
                }
            }
            if (bOK) {
                m_image = new LPBYTE[m_dir.idCount];
                bOK = true;
                for (int i = 0; i < m_dir.idCount; i++) {
                    ::SetFilePointer(hFile, m_entry[i].dwImageOffset,
                        NULL, FILE_BEGIN);
                    m_image[i] = new BYTE[m_entry[i].dwBytesInRes];
                    if (!::ReadFile(hFile, m_image[i],
                        m_entry[i].dwBytesInRes,
                        &dwRead, NULL))
                    {
                        for (int j = i; j >= 0; --j) {
                            delete[] m_image[i];
                            m_image[i] = NULL;
                        }
                        bOK = false;
                        assert(0);
                        break;
                    }
                }
                if (!bOK) {
                    delete[] m_image;
                    m_image = NULL;
                }
            }
            if (!bOK) {
                delete m_entry;
                m_entry = NULL;
            }
        }
    }
    else {
        assert(0);
    }
    ::CloseHandle(hFile);
    return bOK;
} // ExeIconReplacer::LoadIconFile

LPBYTE ExeIconReplacer::CreateIconGroupData(int nBaseID) {
    delete m_group;
    m_group = new BYTE[SizeOfIconGroupData()];
    CopyMemory(m_group, &m_dir, sizeof(ICONDIR));

    int offset = sizeof(ICONDIR);
    for (int i = 0; i < GetImageCount(); i++) {
        BITMAPINFOHEADER    bmih;
        CopyMemory(&bmih, GetImageData(i),
            sizeof(BITMAPINFOHEADER));

        GRPICONDIRENTRY grpEntry;
        grpEntry.bWidth = m_entry[i].bWidth;
        grpEntry.bHeight = m_entry[i].bHeight;
        grpEntry.bColorCount = m_entry[i].bColorCount;
        grpEntry.bReserved = m_entry[i].bReserved;
        grpEntry.wPlanes = bmih.biPlanes;
        grpEntry.wBitCount = bmih.biBitCount;
        grpEntry.dwBytesInRes = m_entry[i].dwBytesInRes;
        grpEntry.nID = nBaseID + i;

        CopyMemory(m_group + offset, &grpEntry,
            sizeof(GRPICONDIRENTRY));

        offset += sizeof(GRPICONDIRENTRY);
    }

    return m_group;
} // ExeIconReplacer::CreateIconGroupData

////////////////////////////////////////////////////////////////////////////

// function ReplaceIconOfExeFile
#ifdef __cplusplus
extern "C"
#endif
BOOL ReplaceIconOfExeFile(LPCTSTR pszExeFile, LPCTSTR pszIconFile,
    UINT nIconGroupID, UINT nIconBaseID)
{
    ExeIconReplacer replacer;
    if (replacer.LoadIconFile(pszIconFile)) {
        HANDLE hUpdate = ::BeginUpdateResource(pszExeFile, FALSE);
        if (hUpdate) {
            // RT_GROUP_ICON
            BOOL bOK = ::UpdateResource(hUpdate, RT_GROUP_ICON,
                MAKEINTRESOURCE(nIconGroupID), 0,
                replacer.CreateIconGroupData(nIconBaseID),
                replacer.SizeOfIconGroupData()
            );

            // RT_ICON
            if (bOK) {
                for (int i = 0; i < replacer.GetImageCount(); i++) {
                    bOK = ::UpdateResource(hUpdate, RT_ICON,
                        MAKEINTRESOURCE(nIconBaseID + i), 0,
                        replacer.GetImageData(i),
                        replacer.GetImageSize(i)
                    );
                    if (!bOK) {
                        break;
                    }
                }
            }

            // finish
            if (bOK) {
                return ::EndUpdateResource(hUpdate, FALSE);
            }
            ::EndUpdateResource(hUpdate, TRUE);
        }
    }
    return FALSE;
} // ReplaceIconOfExeFile

////////////////////////////////////////////////////////////////////////////

#include <string>
#include <iostream>
#include <tchar.h>

#ifdef _UNICODE
typedef std::wstring tstring;
#define tcout std::wcout
#else
typedef std::string tstring;
#define tcout std::cout
#endif

//////////////////////////////////////////////////////////////////////////////
//
//// NOTE: define NO_ICONREPLACER_PROGRAM if you want to use as a library.
////#define NO_ICONREPLACER_PROGRAM
//
//#ifndef NO_ICONREPLACER_PROGRAM
//extern "C"
//int _tmain(int argc, _TCHAR** targv) {
//#if 0
//    // test only
//    if (ReplaceIconOfExeFile(TEXT("hello.exe"), TEXT("excel.ico"), 1, 1)) {
//        std::cout << "success" << std::endl;
//        return 0;
//    }
//    std::cout << "failure" << std::endl;
//    return 1;
//#endif
//
//    if (argc != 3) {
//        // show usage
//        tstring progname = targv[0];
//        size_t i = progname.find_last_of(TEXT("\\/"));
//        if (i != std::string::npos) {
//            progname = progname.substr(i + 1);
//        }
//        tcout << TEXT("Usage: ") << progname <<
//            TEXT(" exefile.exe iconfile.ico") << std::endl;
//        return 0;
//    }
//
//    // replace icon
//    if (ReplaceIconOfExeFile(targv[1], targv[2], 1, 1)) {
//        std::cout << "success" << std::endl;
//        return 0;
//    }
//    std::cout << "failure" << std::endl;
//    return 1;
//} // _tmain
//#endif  // ndef NO_ICONREPLACER_PROGRAM
//
//////////////////////////////////////////////////////////////////////////////
