////////////////////////////////////////////////////////////////////////////
// ExeIconReplacer.h -- replace icon of exe file
// Copyright (C) 2015 Katayama Hirofumi MZ.  All rights reserved.
////////////////////////////////////////////////////////////////////////////

#ifndef EXEICONREPLACER_H_
#define EXEICONREPLACER_H_ 1

#ifndef _INC_WINDOWS
#include <windows.h>
#endif

#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif

////////////////////////////////////////////////////////////////////////////

typedef struct
{
    WORD           idReserved;  // Reserved (must be 0)
    WORD           idType;      // Resource Type (1 for icons)
    WORD           idCount;     // How many images?
} ICONDIR, * LPICONDIR;

typedef struct
{
    BYTE        bWidth;         // Width, in pixels, of the image
    BYTE        bHeight;        // Height, in pixels, of the image
    BYTE        bColorCount;    // Number of colors in image (0 if >=8bpp)
    BYTE        bReserved;      // Reserved ( must be 0)
    WORD        wPlanes;        // Color Planes
    WORD        wBitCount;      // Bits per pixel
    DWORD       dwBytesInRes;   // How many bytes in this resource?
    DWORD       dwImageOffset;  // Where in the file is this image?
} ICONDIRENTRY, * LPICONDIRENTRY;

#include <pshpack2.h>
typedef struct
{
    BYTE   bWidth;               // Width, in pixels, of the image
    BYTE   bHeight;              // Height, in pixels, of the image
    BYTE   bColorCount;          // Number of colors in image (0 if >=8bpp)
    BYTE   bReserved;            // Reserved
    WORD   wPlanes;              // Color Planes
    WORD   wBitCount;            // Bits per pixel
    DWORD   dwBytesInRes;        // how many bytes in this resource?
    WORD   nID;                  // the ID
} GRPICONDIRENTRY, * LPGRPICONDIRENTRY;
#include <poppack.h>

////////////////////////////////////////////////////////////////////////////

// function ReplaceIconOfExeFile
#ifdef __cplusplus
extern "C"
#endif
BOOL ReplaceIconOfExeFile(LPCTSTR pszExeFile, LPCTSTR pszIconFile,
    UINT nIconGroupID, UINT nIconBaseID);

////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
    // class ExeIconReplacer
class ExeIconReplacer {
public:
    ExeIconReplacer() {
        ZeroMemory(&m_dir, sizeof(m_dir));
        m_entry = NULL;
        m_image = NULL;
        m_group = NULL;
    }

    ~ExeIconReplacer() {
        if (m_image) {
            for (int i = 0; i < m_dir.idCount; i++) {
                delete m_image[i];
            }
        }
        delete m_image;
        delete m_entry;
        delete m_group;
    }

    bool LoadIconFile(LPCTSTR pszFileName);

    LPBYTE CreateIconGroupData(int nBaseID);

    int GetImageCount() const {
        return m_dir.idCount;
    }

    LPBYTE GetImageData(int index) const {
        assert(0 <= index && index < GetImageCount());
        assert(m_image);
        return m_image[index];
    }

    DWORD GetImageSize(int index) const {
        assert(0 <= index && index < GetImageCount());
        assert(m_entry);
        return m_entry[index].dwBytesInRes;
    }

    BOOL IsIconDirOK() const {
        return m_dir.idReserved == 0 &&
            m_dir.idType == 1 && m_dir.idCount > 0;
    }

    int SizeOfIconGroupData() const {
        return sizeof(ICONDIR) +
            sizeof(GRPICONDIRENTRY) * GetImageCount();
    }

protected:
    ICONDIR         m_dir;
    ICONDIRENTRY* m_entry;
    LPBYTE* m_image;
    LPBYTE          m_group;
}; // class ExeIconReplacer
#endif  // def __cplusplus

////////////////////////////////////////////////////////////////////////////

#endif  // ndef EXEICONREPLACER_H_
