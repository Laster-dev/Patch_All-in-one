
// NativePatch_MFCDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "NativePatch_MFC.h"
#include "NativePatch_MFCDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#include "Patch.h"
using namespace std;


// CNativePatchMFCDlg 对话框



CNativePatchMFCDlg::CNativePatchMFCDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_NATIVEPATCH_MFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNativePatchMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, myfile2);
}

BEGIN_MESSAGE_MAP(CNativePatchMFCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CNativePatchMFCDlg::OnBnClickedOk)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CNativePatchMFCDlg 消息处理程序

BOOL CNativePatchMFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNativePatchMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNativePatchMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}
string CStringTostring(CString cstr)
{
	// 确定目标代码页，比如CP_ACP代表当前系统的默认ANSI代码页
	int codePage = CP_ACP;
	// 计算转换后需要的缓冲区大小
	int requiredSize = WideCharToMultiByte(codePage, 0, (LPCWSTR)cstr, -1, nullptr, 0, nullptr, nullptr);
	// 分配足够的内存来存储转换后的字符串
	std::vector<char> buffer(requiredSize);
	// 执行转换
	WideCharToMultiByte(codePage, 0, (LPCWSTR)cstr, -1, &buffer[0], requiredSize, nullptr, nullptr);
	// 构造std::string对象，注意不要包括终止的null字符
	std::string str(&buffer[0], &buffer[0] + requiredSize - 1);
	return str;
}



void CNativePatchMFCDlg::OnBnClickedOk()
{
	CString modifyPeFilePath(_T(""));
	GetDlgItemText(IDC_EDIT1, modifyPeFilePath);//获取白路径

	CString textOrPePath(_T(""));
	GetDlgItemText(IDC_EDIT2, textOrPePath);//获取黑路径
	execute(CStringTostring(modifyPeFilePath), CStringTostring(textOrPePath));
}


void CNativePatchMFCDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//获取文件路径
	TCHAR szPath[MAX_PATH] = { 0 };
	DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	//显示到控件上
	SetDlgItemText(IDC_EDIT1, szPath);
	UpdateData(TRUE);
	CDialogEx::OnDropFiles(hDropInfo);
}
