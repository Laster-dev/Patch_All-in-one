
// MFC_Patch.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CMFC_PatchApp:
// �йش����ʵ�֣������ MFC_Patch.cpp
//

class CMFC_PatchApp : public CWinApp
{
public:
	CMFC_PatchApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CMFC_PatchApp theApp;