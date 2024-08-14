#pragma once

//
// @brief: PE�ļ���Դ��ȡ��
// @copyright: Copyright 2024 FlameCyclone
// @license: 
// @birth: Created by Visual Studio 2022 on 2024-01-27
// @version: V1.0.0
// @revision: last revised by FlameCyclone on 2024-01-27
//

#include <wtypesbase.h>
#include <windows.h>
#include <string>
#include <vector>

#ifdef _UNICODE
using _tstring = std::wstring;
#else
using _tstring = std::string;
#endif

//��زο��ĵ�
//ICO��ʽ�ٿ� https://en.wikipedia.org/wiki/ICO_(file_format)
//NEWHEADER �ṹ https://learn.microsoft.com/zh-cn/windows/win32/menurc/newheader
//RESDIR �ṹ https://learn.microsoft.com/zh-cn/windows/win32/menurc/resdir
//ICONRESDIR �ṹ https://learn.microsoft.com/zh-cn/windows/win32/menurc/iconresdir

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    WORD Reserved;                  //����;����Ϊ��
    WORD ResType;                   //��Դ���� 1: RES_ICON    2: RES_CURSOR
    WORD ResCount;                  //��Դ���е�ͼ����α������
} ICON_GROUP_HEADER, * LPICON_GROUP_HEADER;

typedef struct
{
    BYTE Width;                     //ͼ��Ŀ�ȣ�������Ϊ��λ���� �ɽ��ܵ�ֵΪ 16��32 �� 64
    BYTE Height;                    //ͼ��ĸ߶ȣ�������Ϊ��λ���� �ɽ��ܵ�ֵΪ 16��32 �� 64
    BYTE ColorCount;                //ͼ���е���ɫ���� �ɽ��ܵ�ֵΪ 2��8 �� 16��
    BYTE reserved;                  //����;��������Ϊ��ͼ���ļ���ͷ�б����ֶε�ֵ��ͬ��ֵ
    WORD Planes;                    //ͼ�����λͼ�е���ɫƽ����
    WORD BitCount;                  //ͼ�����λͼ��ÿ���ص�λ��
    DWORD BytesInRes;               //��Դ�Ĵ�С�����ֽ�Ϊ��λ��
    WORD IconId;                    //����Ψһ��ű�ʶ����ͼ�����
} ICON_ENTRY, * LPICON_ENTRY;

typedef struct {
    ICON_GROUP_HEADER Header;       //ͼ����ͷ��
    ICON_ENTRY    IconEntry[1];     //����ͼ����Ϣ
}ICON_GROUP_DIR, * LPICON_GROUP_DIR;

typedef struct
{
    BYTE Width;                     //ͼ��Ŀ�ȣ�������Ϊ��λ���� �ɽ��ܵ�ֵΪ 16��32 �� 64
    BYTE Height;                    //ͼ��ĸ߶ȣ�������Ϊ��λ���� �ɽ��ܵ�ֵΪ 16��32 �� 64
    BYTE ColorCount;                //ͼ���е���ɫ���� �ɽ��ܵ�ֵΪ 2��8 �� 16
    BYTE reserved;                  //����;��������Ϊ��ͼ���ļ���ͷ�б����ֶε�ֵ��ͬ��ֵ
    WORD Planes;                    //ͼ�����λͼ�е���ɫƽ����
    WORD BitCount;                  //ͼ�����λͼ��ÿ���ص�λ��
    DWORD BytesInRes;               //��Դ�Ĵ�С�����ֽ�Ϊ��λ��
    DWORD Offset;                   //ͼ���ļ�ƫ��
} ICON_FILE_ENTRY, * LPICON_FILE_ENTRY;

typedef struct {
    ICON_GROUP_HEADER Header;           //ͼ����ͷ��
    ICON_FILE_ENTRY    IconEntry[1];    //����ͼ����Ϣ
}ICON_FILE_DIR, * LPICON_FILE_DIR;

// IHDR����ͷ
// http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.Additional-chunk-types
typedef struct
{
    DWORD Width;                //ͼƬ���(��ͷ����)
    DWORD Height;               //ͼƬ�߶�(��ͷ����)
    BYTE BitDepth;              //ÿ���ص�λ��, ��ЧֵΪ 1��2��4��8 �� 16
    BYTE ColorType;             //��ɫ����
    //0: BitDepthΪ 1,2,4,8,16 ÿ�����ض���һ���Ҷ�����
    //2: BitDepthΪ 8,16 ÿ��������һ�� R��G��B ��Ԫ��
    //3: BitDepthΪ 1,2,4,8 ÿ������Ϊ��ɫ������, ������� PLTE ��
    //4: BitDepthΪ 8,16 ÿ�����ض���һ���Ҷ�����, ��� alpha ������
    //6: BitDepthΪ 8,16 ÿ��������һ�� R��G��B ��Ԫ��, ��� alpha ����

    BYTE CompressionMethod;     //ѹ������
    BYTE FilterMethod;          //�˲�������
    BYTE InterlaceMethod;       //����ɨ�跽��: 0���Ǹ���ɨ�� 1�� Adam7(��Adam M.Costello������7�����ɨ�跽��)
}IHDR, * LPIHDR;

// PNGͼ���ļ�ͷ��Ϣ
// https://en.wikipedia.org/wiki/PNG
typedef struct
{
    BYTE Signature[8];          //PNG�̶�ǩ����ʶ
    DWORD ChunkLength;          //���ݿ鳤��
    CHAR ChunkType[4];          //���ݿ�����, Ӧ��ΪIHDR
    IHDR Ihdr;                  //IHDRͼ��ͷ
    DWORD Crc32;                //������У����(�����������������
}PNG_HEADER, * LPPNG_HEADER;

// https://learn.microsoft.com/zh-cn/windows/win32/gdi/bitmap-header-types
typedef union
{
    BITMAPCOREHEADER BitmapCore;  //Windows 2.0 or later          12 �ֽ�
    BITMAPINFOHEADER BitmapInfo;  //Windows NT, 3.1x or later     40 �ֽ�
    BITMAPV4HEADER BitmapV4;      //Windows NT 4.0, 95 or later   108 �ֽ�
    BITMAPV5HEADER BitmapV5;      //Windows NT 5.0, 98 or later   124 �ֽ�
}BITMAPHEADER, * LPBITMAPHEADER;

// BMPλͼ�ļ�ͷ��Ϣ
// https://en.wikipedia.org/wiki/BMP_file_format
typedef struct
{
    WORD Signature[2];          //BMP�̶�ǩ����ʶ 0x4D42
    DWORD Size;                 //ͼ���ļ���С(�����ļ��Ĵ�С)
    WORD Reserved[2];           //�����ֶ�
    DWORD Offset;               //ͼ������ƫ��
    BITMAPHEADER BitmapInfo;    //λͼ��Ϣ
}BMP_HEADER, * LPBMP_HEADER;

#pragma pack(pop)

enum eIconFileFormat
{
    eFormatBmp,                 //λͼ��ʽ
    eFormatPng                  //PNGͼƬ
};

typedef struct
{
    WORD wID;                   //IDֵ
    DWORD Width;                //ͼƬ���
    DWORD Height;               //ͼƬ�߶�
    eIconFileFormat FileFormat; //�ļ���ʽ
}ICON_INFO;

typedef struct
{
    _tstring strIDName;         //ID�ַ���
    WORD wID;                   //IDֵ
    bool bIntResource;          //�Ƿ�Ϊ������ԴID, ����Ϊ�ַ�����ԴID
    std::vector<ICON_INFO> Icons; //ͼ����Ϣ�б�
}ICON_GROUP;

// ��Դ��ȡ������
class CResourceExtractor
{
public:
    CResourceExtractor();
    ~CResourceExtractor();

    bool Load(const _tstring& strPeFile);

    //
    // @brief: ��ȡͼ��������
    // @param: strModule            ģ��·��
    // @ret: size_t                 ͼ������
    size_t GetIconGroupsCount() const;

    //
    // @brief: ��ȡͼ������Ϣ�б�
    // @ret: std::vector<ICON_GROUP_INFO>      ͼ����Ϣ�б�
    std::vector<ICON_GROUP> GetIconGroups() const;

    //
    // @brief: ��ȡͼ��������С�ߴ��ͼ������
    // @param: nGroupIndex          ͼ��������
    // @param: nIconIndex           ͼ������
    // @param: nLimitSize           ͼ����С��С����(����ȡ��С��ָ����С��ͼ������, 0: ������)
    // @ret: bool                   �����Ƿ�ɹ�
    bool GetMinSizeIconIndex(WORD nGroupIndex, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: ��ȡͼ��������С�ߴ��ͼ������
    // @param: iconGroup            ͼ������Ϣ
    // @param: nIconIndex           ͼ������
    // @param: nLimitSize           ͼ����С��С����(����ȡ��С��ָ����С��ͼ������, 0: ������)
    // @ret: bool                   �����Ƿ�ɹ�
    bool GetMinSizeIconIndex(const ICON_GROUP& iconGroup, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: ��ȡͼ���������ߴ��ͼ������
    // @param: nGroupIndex          ͼ��������
    // @param: nIconIndex           ͼ������
    // @param: nLimitSize           ͼ������С����(����ȡ������ָ����С��ͼ������, 0: ������)
    // @ret: bool                   �����Ƿ�ɹ�
    bool GetMaxSizeIconIndex(WORD nGroupIndex, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: ��ȡͼ���������ߴ��ͼ������
    // @param: iconGroup            ͼ������Ϣ
    // @param: nIconIndex           ͼ������
    // @param: nLimitSize           ͼ������С����(����ȡ������ָ����С��ͼ������, 0: ������)
    // @ret: bool                   �����Ƿ�ɹ�
    bool GetMaxSizeIconIndex(const ICON_GROUP& iconGroup, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: ��ȡͼ���鱣�浽�ļ�
    // @param: nIconIndex           ͼ������
    // @param: strFile              �����ļ�·��
    // @ret: bool                   �����Ƿ�ɹ�
    bool ExtractIconGroupToFile(WORD nIconIndex, const _tstring& strOutFile);

    //
    // @brief: ��ȡͼ���鱣�浽�ļ�
    // @param: iconGroup            ͼ������Ϣ
    // @param: strFile              �����ļ�·��
    // @ret: bool                   �����Ƿ�ɹ�
    bool ExtractIconGroupToFile(const ICON_GROUP& iconGroup, const _tstring& strOutFile);

    //
    // @brief: ��ȡ����ͼ�걣�浽�ļ�
    // @param: nGroupIndex          ͼ��������
    // @param: nIconIndex           ͼ������
    // @param: strFile              �����ļ�·��
    // @ret: bool                   �����Ƿ�ɹ�
    bool ExtractIconToFile(WORD nGroupIndex, WORD nIconIndex, const _tstring& strOutFile);

    //
    // @brief: ��ȡ����ͼ�걣�浽�ļ�
    // @param: iconGroup            ͼ������Ϣ
    // @param: nIconIndex           ͼ������
    // @param: strFile              �����ļ�·��
    // @ret: bool                   �����Ƿ�ɹ�
    bool ExtractIconToFile(const ICON_GROUP& iconGroup, WORD nIconIndex, const _tstring& strOutFile);

private:

    LPICON_GROUP_DIR _GetGroupDir(const ICON_GROUP& iconGroupInfo);
    bool _WriteIconGroupToFile(HMODULE hModule, LPICON_GROUP_DIR lpIconGroupDir, const _tstring& strOutFile);
    bool _WriteIconToFile(HMODULE hModule, LPICON_GROUP_DIR lpIconGroupDir, WORD wID, const _tstring& strOutFile);
    static std::vector<ICON_INFO> _GetIconGroupInfo(HMODULE hModule, LPCTSTR lpIdName);
    static bool _IsPngSignature(LPVOID lpData);

private:

    HMODULE m_hModule;                          //PEģ����
    std::vector<ICON_GROUP> m_listIcons;   //ͼ��ID�б�
};