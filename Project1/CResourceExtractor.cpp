
#include "CResourceExtractor.h"

#define BIG_LITTLE_SWAP32(_data) (  (((DWORD)_data & 0x000000FF) << 24) | \
                                    (((DWORD)_data & 0x0000FF00) << 8) | \
                                    (((DWORD)_data & 0x00FF0000) >> 8) | \
                                    (((DWORD)_data & 0xFF000000) >> 24) )

CResourceExtractor::CResourceExtractor()
    :
    m_hModule(nullptr)
{

}

CResourceExtractor::~CResourceExtractor()
{
    if (m_hModule)
    {
        ::FreeLibrary(m_hModule);
        m_hModule = nullptr;
    }
}

size_t CResourceExtractor::GetIconGroupsCount() const
{
    return m_listIcons.size();
}

bool CResourceExtractor::Load(const _tstring& strPeFile)
{
    if (m_hModule)
    {
        ::FreeLibrary(m_hModule);
    }

    m_listIcons.clear();

    // 以数据资源方式加载PE文件
    m_hModule = ::LoadLibraryEx(strPeFile.c_str(), 0, LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE);
    if (!m_hModule)
    {
        return false;
    }

    // 枚举指定索引的资源
    ::EnumResourceNames(m_hModule, RT_GROUP_ICON, [](
        _In_opt_ HMODULE hModule,
        _In_ LPCTSTR lpType,
        _In_ LPTSTR lpName,
        _In_ LONG_PTR lParam
        )->BOOL {

            UNREFERENCED_PARAMETER(hModule);
            UNREFERENCED_PARAMETER(lpType);

            std::vector<ICON_GROUP>* pInfos = reinterpret_cast<std::vector<ICON_GROUP>*>(lParam);
            ICON_GROUP info;
            info.bIntResource = IS_INTRESOURCE(lpName);

            if (info.bIntResource)
            {
                info.wID = reinterpret_cast<WORD>(lpName);
            }
            else
            {
                info.strIDName = lpName;
            }

            info.Icons = _GetIconGroupInfo(hModule, lpName);
            pInfos->push_back(info);

            return TRUE;
        }, reinterpret_cast<LONG_PTR>(&m_listIcons));

    return true;
}

std::vector<ICON_INFO> CResourceExtractor::_GetIconGroupInfo(HMODULE hModule, LPCTSTR lpIdName)
{
    std::vector<ICON_INFO> infos;

    HRSRC hResource = nullptr;
    HGLOBAL hGlobal = nullptr;
    LPICON_GROUP_DIR lpIconGroupDir = nullptr;

    do
    {
        //查找资源
        hResource = ::FindResource(hModule, lpIdName, RT_GROUP_ICON);
        if (!hResource)
        {
            break;
        }

        //加载资源数据
        hGlobal = ::LoadResource(hModule, hResource);
        if (!hGlobal)
        {
            break;
        }

        //检索指向内存中指定资源的指针
        lpIconGroupDir = (LPICON_GROUP_DIR)::LockResource(hGlobal);
        if (!lpIconGroupDir)
        {
            break;
        }

        //写入数据
        for (int i = 0; i < lpIconGroupDir->Header.ResCount; i++)
        {
            HRSRC hIcoResource = nullptr;
            HGLOBAL hIcoGlobal = nullptr;
            LPBYTE lpIconData = nullptr;

            //查找资源
            hIcoResource = ::FindResource(hModule, MAKEINTRESOURCE(lpIconGroupDir->IconEntry[i].IconId), RT_ICON);
            if (!hIcoResource)
            {
                break;
            }

            //检查数据大小
            if (lpIconGroupDir->IconEntry[i].BytesInRes != ::SizeofResource(hModule, hIcoResource))
            {
                break;
            }

            //加载资源数据
            hIcoGlobal = ::LoadResource(hModule, hIcoResource);
            if (!hIcoGlobal)
            {
                break;
            }

            //锁定数据
            lpIconData = (LPBYTE)::LockResource(hIcoGlobal);
            if (!lpIconData)
            {
                break;
            }

            LPPNG_HEADER lpPngHeader = (LPPNG_HEADER)lpIconData;
            //LPBITMAPHEADER lpBmpHeader = (LPBITMAPHEADER)lpIconData;

            ICON_INFO info = { 0 };
            info.wID = lpIconGroupDir->IconEntry[i].IconId;
            info.FileFormat = eIconFileFormat::eFormatBmp;

            if (_IsPngSignature(lpPngHeader))
            {
                info.FileFormat = eIconFileFormat::eFormatPng;
                info.Width = BIG_LITTLE_SWAP32(lpPngHeader->Ihdr.Width);
                info.Height = BIG_LITTLE_SWAP32(lpPngHeader->Ihdr.Height);
            }
            else
            {
                info.FileFormat = eIconFileFormat::eFormatBmp;
                info.Width = lpIconGroupDir->IconEntry[i].Width;
                info.Height = lpIconGroupDir->IconEntry[i].Height;
            }

            infos.push_back(info);
        }

    } while (false);

    return infos;
}

bool CResourceExtractor::_IsPngSignature(LPVOID lpData)
{
    const BYTE PngSignature[] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
    return 0 == memcmp(lpData, PngSignature, sizeof(PngSignature));
}

std::vector<ICON_GROUP> CResourceExtractor::GetIconGroups() const
{
    return m_listIcons;
}

bool CResourceExtractor::GetMinSizeIconIndex(WORD nGroupIndex, WORD& nIconIndex, DWORD nLimitSize/* = 0*/)
{
    if (m_listIcons.size() <= nGroupIndex)
    {
        return false;
    }

    return GetMinSizeIconIndex(m_listIcons[nGroupIndex], nIconIndex, nLimitSize);
}

bool CResourceExtractor::GetMinSizeIconIndex(const ICON_GROUP& iconGroup, WORD& nIconIndex, DWORD nLimitSize/* = 0*/)
{
    const std::vector<ICON_INFO>& icons = iconGroup.Icons;
    if (icons.empty())
    {
        return false;
    }

    DWORD dwMaxSize = 0;
    DWORD dwMinSize = UINT16_MAX;

    // 统计最大最小尺寸
    for (auto& icon : icons)
    {
        if (icon.Width >= dwMaxSize)
        {
            dwMaxSize = icon.Width;
        }

        if (icon.Width <= dwMinSize)
        {
            dwMinSize = icon.Width;
        }
    }

    // 限制大小
    if (nLimitSize > dwMaxSize)
    {
        nLimitSize = dwMaxSize;
    }

    if (nLimitSize < dwMinSize)
    {
        nLimitSize = dwMinSize;
    }

    DWORD dwTmpSize = UINT16_MAX;
    WORD nItemIndex = 0;
    nIconIndex = 0;
    for (auto& icon : icons)
    {
        if (0 == nLimitSize && icon.Width < dwTmpSize)
        {
            dwTmpSize = icon.Width;
            nIconIndex = nItemIndex;
        }
        else
        {
            if (icon.Width >= nLimitSize && icon.Width < dwTmpSize)
            {
                dwTmpSize = icon.Width;
                nIconIndex = nItemIndex;
            }
        }

        nItemIndex++;
    }

    return true;
}

bool CResourceExtractor::GetMaxSizeIconIndex(WORD nGroupIndex, WORD& nIconIndex, DWORD nLimitSize/* = 0*/)
{
    if (m_listIcons.size() <= nGroupIndex)
    {
        return false;
    }

    return GetMaxSizeIconIndex(m_listIcons[nGroupIndex], nIconIndex, nLimitSize);
}

bool CResourceExtractor::GetMaxSizeIconIndex(const ICON_GROUP& iconGroup, WORD& nIconIndex, DWORD nLimitSize/* = 0*/)
{
    const std::vector<ICON_INFO>& icons = iconGroup.Icons;
    if (icons.empty())
    {
        return false;
    }

    DWORD dwMaxSize = 0;
    DWORD dwMinSize = UINT16_MAX;

    // 统计最大最小尺寸
    for (auto& icon : icons)
    {
        if (icon.Width >= dwMaxSize)
        {
            dwMaxSize = icon.Width;
        }

        if (icon.Width <= dwMinSize)
        {
            dwMinSize = icon.Width;
        }
    }

    // 限制大小
    if (nLimitSize > dwMaxSize)
    {
        nLimitSize = dwMaxSize;
    }

    if (nLimitSize < dwMinSize)
    {
        nLimitSize = dwMinSize;
    }

    DWORD dwTmpSize = 0;
    WORD nItemIndex = 0;
    nIconIndex = 0;
    for (auto& icon : icons)
    {
        // 不限定最大尺寸
        if (0 == nLimitSize && icon.Width > dwTmpSize)
        {
            dwTmpSize = icon.Width;
            nIconIndex = nItemIndex;
        }
        else
        {
            // 限定最大尺寸
            if (icon.Width <= nLimitSize && icon.Width > dwTmpSize)
            {
                dwTmpSize = icon.Width;
                nIconIndex = nItemIndex;
            }
        }

        nItemIndex++;
    }

    return true;
}

bool CResourceExtractor::ExtractIconGroupToFile(WORD nIconIndex, const _tstring& strOutFile)
{
    if (nullptr == m_hModule)
    {
        return false;
    }

    if (m_listIcons.size() <= nIconIndex)
    {
        return false;
    }

    return _WriteIconGroupToFile(m_hModule, _GetGroupDir(m_listIcons[nIconIndex]), strOutFile);
}

bool CResourceExtractor::ExtractIconGroupToFile(const ICON_GROUP& iconGroup, const _tstring& strOutFile)
{
    if (nullptr == m_hModule)
    {
        return false;
    }

    return _WriteIconGroupToFile(m_hModule, _GetGroupDir(iconGroup), strOutFile);
}

bool CResourceExtractor::ExtractIconToFile(WORD nGroupIndex, WORD nIconIndex, const _tstring& strOutFile)
{
    if (nullptr == m_hModule)
    {
        return false;
    }

    if (nGroupIndex >= m_listIcons.size())
    {
        return false;
    }

    if (nIconIndex >= m_listIcons[nGroupIndex].Icons.size())
    {
        return false;
    }

    const ICON_GROUP& iconGroup = m_listIcons[nGroupIndex];
    return _WriteIconToFile(m_hModule, _GetGroupDir(iconGroup), iconGroup.Icons[nIconIndex].wID, strOutFile);
}

bool CResourceExtractor::ExtractIconToFile(const ICON_GROUP& iconGroup, WORD nIconIndex, const _tstring& strOutFile)
{
    if (nullptr == m_hModule)
    {
        return false;
    }

    if (nIconIndex >= iconGroup.Icons.size())
    {
        return false;
    }

    return _WriteIconToFile(m_hModule, _GetGroupDir(iconGroup), iconGroup.Icons[nIconIndex].wID, strOutFile);
}

LPICON_GROUP_DIR CResourceExtractor::_GetGroupDir(const ICON_GROUP& iconGroup)
{
    if (nullptr == m_hModule)
    {
        return nullptr;
    }

    HRSRC hResource = nullptr;
    HGLOBAL hGlobal = nullptr;
    LPICON_GROUP_DIR lpIconGroupDir = nullptr;
    bool fResult = false;

    do
    {
        //查找资源
        if (iconGroup.bIntResource)
        {
            hResource = ::FindResource(m_hModule, (LPCTSTR)iconGroup.wID, RT_GROUP_ICON);
        }
        else
        {
            hResource = ::FindResource(m_hModule, iconGroup.strIDName.c_str(), RT_GROUP_ICON);
        }

        if (!hResource)
        {
            break;
        }

        //加载资源数据
        hGlobal = ::LoadResource(m_hModule, hResource);
        if (!hGlobal)
        {
            break;
        }

        //检索指向内存中指定资源的指针
        lpIconGroupDir = (LPICON_GROUP_DIR)::LockResource(hGlobal);
        if (!lpIconGroupDir)
        {
            break;
        }

        //检查数据大小
        DWORD dwIconDirSize = sizeof(ICON_GROUP_HEADER) + ((lpIconGroupDir->Header.ResCount) * sizeof(ICON_ENTRY));
        if (dwIconDirSize != ::SizeofResource(m_hModule, hResource))
        {
            break;
        }

        fResult = true;

    } while (false);

    if (!fResult)
    {
        lpIconGroupDir = nullptr;
    }

    return lpIconGroupDir;
}

bool CResourceExtractor::_WriteIconGroupToFile(HMODULE hModule, LPICON_GROUP_DIR lpIconGroupDir, const _tstring& strOutFile)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwNumberOfBytesWritten = 0;
    bool fWriteResult = true;
    bool fResult = false;

    hFile = ::CreateFile(strOutFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    do
    {
        //写入文件头
        ::WriteFile(hFile, &lpIconGroupDir->Header, sizeof(ICON_GROUP_HEADER), &dwNumberOfBytesWritten, nullptr);
        if (sizeof(ICON_GROUP_HEADER) != dwNumberOfBytesWritten)
        {
            break;
        }

        //写入图标头信息
        DWORD dwOffset = sizeof(ICON_GROUP_HEADER) + (lpIconGroupDir->Header.ResCount * sizeof(ICON_FILE_ENTRY));
        fWriteResult = true;
        for (int i = 0; i < lpIconGroupDir->Header.ResCount; i++)
        {
            ICON_FILE_ENTRY iconFileEntry = { 0 };
            memcpy_s(&iconFileEntry, sizeof(ICON_ENTRY), &lpIconGroupDir->IconEntry[i], sizeof(ICON_ENTRY));
            iconFileEntry.Offset = dwOffset;

            //写入单个图标信息
            ::WriteFile(hFile, &iconFileEntry, sizeof(ICON_FILE_ENTRY), &dwNumberOfBytesWritten, nullptr);
            if (sizeof(ICON_FILE_ENTRY) != dwNumberOfBytesWritten)
            {
                fWriteResult = false;
                break;
            }

            dwOffset += lpIconGroupDir->IconEntry[i].BytesInRes;
        }

        if (!fWriteResult)
        {
            break;
        }

        //写入数据
        for (int i = 0; i < lpIconGroupDir->Header.ResCount; i++)
        {
            HRSRC hIcoResource = nullptr;
            HGLOBAL hIcoGlobal = nullptr;
            LPBYTE lpIconData = nullptr;

            fWriteResult = false;
            //查找资源
            hIcoResource = ::FindResource(hModule, MAKEINTRESOURCE(lpIconGroupDir->IconEntry[i].IconId), RT_ICON);
            if (!hIcoResource)
            {
                break;
            }

            //检查数据大小
            if (lpIconGroupDir->IconEntry[i].BytesInRes != ::SizeofResource(hModule, hIcoResource))
            {
                break;
            }

            //加载资源数据
            hIcoGlobal = ::LoadResource(hModule, hIcoResource);
            if (!hIcoGlobal)
            {
                break;
            }

            //锁定数据
            lpIconData = (LPBYTE)::LockResource(hIcoGlobal);
            if (!lpIconData)
            {
                break;
            }

            //写入图标数据
            ::WriteFile(hFile, lpIconData, lpIconGroupDir->IconEntry[i].BytesInRes, &dwNumberOfBytesWritten, nullptr);
            if (dwNumberOfBytesWritten != lpIconGroupDir->IconEntry[i].BytesInRes)
            {
                break;
            }
            fWriteResult = true;
        }

        if (!fWriteResult)
        {
            break;
        }

        fResult = true;

    } while (false);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return fResult;
}

bool CResourceExtractor::_WriteIconToFile(HMODULE hModule, LPICON_GROUP_DIR lpIconGroupDir, WORD wID, const _tstring& strOutFile)
{
    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwNumberOfBytesWritten = 0;
    bool fWriteResult = true;
    bool fResult = false;

    if (!lpIconGroupDir)
    {
        return false;
    }

    hFile = ::CreateFile(strOutFile.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (INVALID_HANDLE_VALUE == hFile)
    {
        return false;
    }

    do
    {
        //写入文件头
        ICON_GROUP_HEADER iconGroupHeader = lpIconGroupDir->Header;
        iconGroupHeader.ResCount = 1;
        ::WriteFile(hFile, &iconGroupHeader, sizeof(ICON_GROUP_HEADER), &dwNumberOfBytesWritten, nullptr);
        if (sizeof(ICON_GROUP_HEADER) != dwNumberOfBytesWritten)
        {
            break;
        }

        //写入图标头信息
        DWORD dwOffset = sizeof(ICON_GROUP_HEADER) + (iconGroupHeader.ResCount * sizeof(ICON_FILE_ENTRY));
        fWriteResult = false;
        for (int i = 0; i < lpIconGroupDir->Header.ResCount; i++)
        {
            if (wID != lpIconGroupDir->IconEntry[i].IconId)
            {
                continue;
            }

            ICON_FILE_ENTRY iconFileEntry = { 0 };
            memcpy_s(&iconFileEntry, sizeof(ICON_ENTRY), &lpIconGroupDir->IconEntry[i], sizeof(ICON_ENTRY));
            iconFileEntry.Offset = dwOffset;

            //写入单个图标信息
            ::WriteFile(hFile, &iconFileEntry, sizeof(ICON_FILE_ENTRY), &dwNumberOfBytesWritten, nullptr);
            if (sizeof(ICON_FILE_ENTRY) != dwNumberOfBytesWritten)
            {
                break;
            }

            fWriteResult = true;
        }

        if (!fWriteResult)
        {
            break;
        }

        //写入数据
        fWriteResult = false;
        for (int i = 0; i < lpIconGroupDir->Header.ResCount; i++)
        {
            if (wID != lpIconGroupDir->IconEntry[i].IconId)
            {
                continue;
            }

            HRSRC hIcoResource = nullptr;
            HGLOBAL hIcoGlobal = nullptr;
            LPBYTE lpIconData = nullptr;

            //查找资源
            hIcoResource = ::FindResource(hModule, MAKEINTRESOURCE(lpIconGroupDir->IconEntry[i].IconId), RT_ICON);
            if (!hIcoResource)
            {
                break;
            }

            //检查数据大小
            if (lpIconGroupDir->IconEntry[i].BytesInRes != ::SizeofResource(hModule, hIcoResource))
            {
                break;
            }

            //加载资源数据
            hIcoGlobal = ::LoadResource(hModule, hIcoResource);
            if (!hIcoGlobal)
            {
                break;
            }

            //锁定数据
            lpIconData = (LPBYTE)::LockResource(hIcoGlobal);
            if (!lpIconData)
            {
                break;
            }

            //写入图标数据
            ::WriteFile(hFile, lpIconData, lpIconGroupDir->IconEntry[i].BytesInRes, &dwNumberOfBytesWritten, nullptr);
            if (dwNumberOfBytesWritten != lpIconGroupDir->IconEntry[i].BytesInRes)
            {
                break;
            }

            fWriteResult = true;
        }

        if (!fWriteResult)
        {
            break;
        }

        fResult = true;

    } while (false);

    if (INVALID_HANDLE_VALUE != hFile)
    {
        ::CloseHandle(hFile);
    }

    return fResult;
}