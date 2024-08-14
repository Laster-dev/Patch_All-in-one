#pragma once

//
// @brief: PE文件资源提取器
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

//相关参考文档
//ICO格式百科 https://en.wikipedia.org/wiki/ICO_(file_format)
//NEWHEADER 结构 https://learn.microsoft.com/zh-cn/windows/win32/menurc/newheader
//RESDIR 结构 https://learn.microsoft.com/zh-cn/windows/win32/menurc/resdir
//ICONRESDIR 结构 https://learn.microsoft.com/zh-cn/windows/win32/menurc/iconresdir

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    WORD Reserved;                  //保留;必须为零
    WORD ResType;                   //资源类型 1: RES_ICON    2: RES_CURSOR
    WORD ResCount;                  //资源组中的图标或游标组件数
} ICON_GROUP_HEADER, * LPICON_GROUP_HEADER;

typedef struct
{
    BYTE Width;                     //图标的宽度（以像素为单位）。 可接受的值为 16、32 和 64
    BYTE Height;                    //图标的高度（以像素为单位）。 可接受的值为 16、32 和 64
    BYTE ColorCount;                //图标中的颜色数。 可接受的值为 2、8 和 16。
    BYTE reserved;                  //保留;必须设置为与图标文件标头中保留字段的值相同的值
    WORD Planes;                    //图标或光标位图中的颜色平面数
    WORD BitCount;                  //图标或光标位图中每像素的位数
    DWORD BytesInRes;               //资源的大小（以字节为单位）
    WORD IconId;                    //具有唯一序号标识符的图标或光标
} ICON_ENTRY, * LPICON_ENTRY;

typedef struct {
    ICON_GROUP_HEADER Header;       //图标组头部
    ICON_ENTRY    IconEntry[1];     //单个图标信息
}ICON_GROUP_DIR, * LPICON_GROUP_DIR;

typedef struct
{
    BYTE Width;                     //图标的宽度（以像素为单位）。 可接受的值为 16、32 和 64
    BYTE Height;                    //图标的高度（以像素为单位）。 可接受的值为 16、32 和 64
    BYTE ColorCount;                //图标中的颜色数。 可接受的值为 2、8 和 16
    BYTE reserved;                  //保留;必须设置为与图标文件标头中保留字段的值相同的值
    WORD Planes;                    //图标或光标位图中的颜色平面数
    WORD BitCount;                  //图标或光标位图中每像素的位数
    DWORD BytesInRes;               //资源的大小（以字节为单位）
    DWORD Offset;                   //图标文件偏移
} ICON_FILE_ENTRY, * LPICON_FILE_ENTRY;

typedef struct {
    ICON_GROUP_HEADER Header;           //图标组头部
    ICON_FILE_ENTRY    IconEntry[1];    //单个图标信息
}ICON_FILE_DIR, * LPICON_FILE_DIR;

// IHDR数据头
// http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html#C.Additional-chunk-types
typedef struct
{
    DWORD Width;                //图片宽度(大头序存放)
    DWORD Height;               //图片高度(大头序存放)
    BYTE BitDepth;              //每像素的位数, 有效值为 1、2、4、8 和 16
    BYTE ColorType;             //颜色类型
    //0: BitDepth为 1,2,4,8,16 每个像素都是一个灰度样本
    //2: BitDepth为 8,16 每个像素是一个 R、G、B 三元组
    //3: BitDepth为 1,2,4,8 每个像素为调色板索引, 必须出现 PLTE 块
    //4: BitDepth为 8,16 每个像素都是一个灰度样本, 后跟 alpha 样本。
    //6: BitDepth为 8,16 每个像素是一个 R、G、B 三元组, 后跟 alpha 样本

    BYTE CompressionMethod;     //压缩方法
    BYTE FilterMethod;          //滤波器方法
    BYTE InterlaceMethod;       //隔行扫描方法: 0：非隔行扫描 1： Adam7(由Adam M.Costello开发的7遍隔行扫描方法)
}IHDR, * LPIHDR;

// PNG图像文件头信息
// https://en.wikipedia.org/wiki/PNG
typedef struct
{
    BYTE Signature[8];          //PNG固定签名标识
    DWORD ChunkLength;          //数据块长度
    CHAR ChunkType[4];          //数据块类型, 应该为IHDR
    IHDR Ihdr;                  //IHDR图像头
    DWORD Crc32;                //块数据校验码(包括块类型与块数据
}PNG_HEADER, * LPPNG_HEADER;

// https://learn.microsoft.com/zh-cn/windows/win32/gdi/bitmap-header-types
typedef union
{
    BITMAPCOREHEADER BitmapCore;  //Windows 2.0 or later          12 字节
    BITMAPINFOHEADER BitmapInfo;  //Windows NT, 3.1x or later     40 字节
    BITMAPV4HEADER BitmapV4;      //Windows NT 4.0, 95 or later   108 字节
    BITMAPV5HEADER BitmapV5;      //Windows NT 5.0, 98 or later   124 字节
}BITMAPHEADER, * LPBITMAPHEADER;

// BMP位图文件头信息
// https://en.wikipedia.org/wiki/BMP_file_format
typedef struct
{
    WORD Signature[2];          //BMP固定签名标识 0x4D42
    DWORD Size;                 //图像文件大小(整个文件的大小)
    WORD Reserved[2];           //保留字段
    DWORD Offset;               //图像数据偏移
    BITMAPHEADER BitmapInfo;    //位图信息
}BMP_HEADER, * LPBMP_HEADER;

#pragma pack(pop)

enum eIconFileFormat
{
    eFormatBmp,                 //位图格式
    eFormatPng                  //PNG图片
};

typedef struct
{
    WORD wID;                   //ID值
    DWORD Width;                //图片宽度
    DWORD Height;               //图片高度
    eIconFileFormat FileFormat; //文件格式
}ICON_INFO;

typedef struct
{
    _tstring strIDName;         //ID字符串
    WORD wID;                   //ID值
    bool bIntResource;          //是否为整数资源ID, 否则为字符串资源ID
    std::vector<ICON_INFO> Icons; //图标信息列表
}ICON_GROUP;

// 资源提取工具类
class CResourceExtractor
{
public:
    CResourceExtractor();
    ~CResourceExtractor();

    bool Load(const _tstring& strPeFile);

    //
    // @brief: 获取图标组总数
    // @param: strModule            模块路径
    // @ret: size_t                 图标数量
    size_t GetIconGroupsCount() const;

    //
    // @brief: 获取图标组信息列表
    // @ret: std::vector<ICON_GROUP_INFO>      图标信息列表
    std::vector<ICON_GROUP> GetIconGroups() const;

    //
    // @brief: 获取图标组中最小尺寸的图标索引
    // @param: nGroupIndex          图标组索引
    // @param: nIconIndex           图标索引
    // @param: nLimitSize           图标最小大小限制(仅获取不小于指定大小的图标索引, 0: 不限制)
    // @ret: bool                   操作是否成功
    bool GetMinSizeIconIndex(WORD nGroupIndex, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: 获取图标组中最小尺寸的图标索引
    // @param: iconGroup            图标组信息
    // @param: nIconIndex           图标索引
    // @param: nLimitSize           图标最小大小限制(仅获取不小于指定大小的图标索引, 0: 不限制)
    // @ret: bool                   操作是否成功
    bool GetMinSizeIconIndex(const ICON_GROUP& iconGroup, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: 获取图标组中最大尺寸的图标索引
    // @param: nGroupIndex          图标组索引
    // @param: nIconIndex           图标索引
    // @param: nLimitSize           图标最大大小限制(仅获取不大于指定大小的图标索引, 0: 不限制)
    // @ret: bool                   操作是否成功
    bool GetMaxSizeIconIndex(WORD nGroupIndex, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: 获取图标组中最大尺寸的图标索引
    // @param: iconGroup            图标组信息
    // @param: nIconIndex           图标索引
    // @param: nLimitSize           图标最大大小限制(仅获取不大于指定大小的图标索引, 0: 不限制)
    // @ret: bool                   操作是否成功
    bool GetMaxSizeIconIndex(const ICON_GROUP& iconGroup, WORD& nIconIndex, DWORD nLimitSize = 0);

    //
    // @brief: 提取图标组保存到文件
    // @param: nIconIndex           图标索引
    // @param: strFile              保存文件路径
    // @ret: bool                   操作是否成功
    bool ExtractIconGroupToFile(WORD nIconIndex, const _tstring& strOutFile);

    //
    // @brief: 提取图标组保存到文件
    // @param: iconGroup            图标组信息
    // @param: strFile              保存文件路径
    // @ret: bool                   操作是否成功
    bool ExtractIconGroupToFile(const ICON_GROUP& iconGroup, const _tstring& strOutFile);

    //
    // @brief: 提取单个图标保存到文件
    // @param: nGroupIndex          图标组索引
    // @param: nIconIndex           图标索引
    // @param: strFile              保存文件路径
    // @ret: bool                   操作是否成功
    bool ExtractIconToFile(WORD nGroupIndex, WORD nIconIndex, const _tstring& strOutFile);

    //
    // @brief: 提取单个图标保存到文件
    // @param: iconGroup            图标组信息
    // @param: nIconIndex           图标索引
    // @param: strFile              保存文件路径
    // @ret: bool                   操作是否成功
    bool ExtractIconToFile(const ICON_GROUP& iconGroup, WORD nIconIndex, const _tstring& strOutFile);

private:

    LPICON_GROUP_DIR _GetGroupDir(const ICON_GROUP& iconGroupInfo);
    bool _WriteIconGroupToFile(HMODULE hModule, LPICON_GROUP_DIR lpIconGroupDir, const _tstring& strOutFile);
    bool _WriteIconToFile(HMODULE hModule, LPICON_GROUP_DIR lpIconGroupDir, WORD wID, const _tstring& strOutFile);
    static std::vector<ICON_INFO> _GetIconGroupInfo(HMODULE hModule, LPCTSTR lpIdName);
    static bool _IsPngSignature(LPVOID lpData);

private:

    HMODULE m_hModule;                          //PE模块句柄
    std::vector<ICON_GROUP> m_listIcons;   //图标ID列表
};