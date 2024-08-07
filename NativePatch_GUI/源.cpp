#include <windows.h>
#include <commdlg.h>
#include <shlobj.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <CommCtrl.h>

#pragma comment(lib, "comctl32.lib")

class PEFile {
public:
    PEFile(const std::string& filePath);
    ~PEFile();

    bool isLoaded() const;
    DWORD getImageBase() const;
    DWORD rvaToOffset(DWORD rva) const;
    std::vector<char> getSectionData(const std::string& sectionName) const;

    DWORD findEntryPointVA() const;

private:
    std::string filePath;
    bool loaded;
    IMAGE_DOS_HEADER dosHeader;
    IMAGE_NT_HEADERS ntHeaders;
    std::vector<IMAGE_SECTION_HEADER> sectionHeaders;

    bool loadPEFile();
    DWORD getSectionOffset(DWORD rva) const;
};

PEFile::PEFile(const std::string& filePath) : filePath(filePath), loaded(false) {
    loaded = loadPEFile();
}

PEFile::~PEFile() {
    // Cleanup if necessary
}

bool PEFile::isLoaded() const {
    return loaded;
}

DWORD PEFile::getImageBase() const {
    return ntHeaders.OptionalHeader.ImageBase;
}

DWORD PEFile::rvaToOffset(DWORD rva) const {
    return getSectionOffset(rva);
}

std::vector<char> PEFile::getSectionData(const std::string& sectionName) const {
    std::vector<char> sectionData;
    for (const auto& section : sectionHeaders) {
        if (strncmp(reinterpret_cast<const char*>(section.Name), sectionName.c_str(), sectionName.size()) == 0) {
            sectionData.resize(section.SizeOfRawData);
            std::ifstream file(filePath, std::ios::binary);
            if (file) {
                file.seekg(section.PointerToRawData);
                file.read(sectionData.data(), section.SizeOfRawData);
            }
            break;
        }
    }
    return sectionData;
}

bool PEFile::loadPEFile() {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "无法打开文件: " << filePath << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "无效的DOS签名" << std::endl;
        return false;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "无效的NT签名" << std::endl;
        return false;
    }

    sectionHeaders.resize(ntHeaders.FileHeader.NumberOfSections);
    file.read(reinterpret_cast<char*>(sectionHeaders.data()), sectionHeaders.size() * sizeof(IMAGE_SECTION_HEADER));

    return true;
}

DWORD PEFile::getSectionOffset(DWORD rva) const {
    for (const auto& section : sectionHeaders) {
        if (rva >= section.VirtualAddress && rva < section.VirtualAddress + section.SizeOfRawData) {
            return rva - section.VirtualAddress + section.PointerToRawData;
        }
    }
    return -1;
}

DWORD PEFile::findEntryPointVA() const {
    return ntHeaders.OptionalHeader.AddressOfEntryPoint + ntHeaders.OptionalHeader.ImageBase;
}

bool checkFileReadable(const std::string& filePath) {
    std::ifstream file(filePath);
    return file.good();
}

DWORD vaToRva(PEFile& pe, DWORD va) {
    return va - pe.getImageBase();
}

bool replaceTextSection(const std::string& peFilePath, const std::string& textBinPath, DWORD va, bool flag) {
    PEFile pe(peFilePath);
    DWORD rva = vaToRva(pe, va);
    DWORD fileOffset = pe.rvaToOffset(rva);

    if (fileOffset == -1) {
        std::cerr << "无法找到对应的文件偏移，RVA 可能不在任何节区中。" << std::endl;
        return false;
    }

    std::ifstream textFile(textBinPath, std::ios::binary);
    std::vector<char> textData((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());

    if (!flag) {
        std::ofstream peFile(peFilePath, std::ios::binary | std::ios::in | std::ios::out);
        peFile.seekp(fileOffset);
        peFile.write(textData.data(), textData.size());
        std::cout << ".text节区已成功覆盖在PE文件中。" << std::endl;
    }
    else {
        int counter = 0;
        std::string newFilePath;
        do {
            newFilePath = peFilePath + "_fuzz_" + std::to_string(counter) + ".exe";
            counter++;
        } while (std::ifstream(newFilePath).good());

        std::ifstream src(peFilePath, std::ios::binary);
        std::ofstream dst(newFilePath, std::ios::binary);

        dst << src.rdbuf();
        dst.seekp(fileOffset);
        dst.write(textData.data(), textData.size());

        std::cout << ".text节区已成功覆盖在PE文件中，fuzz文件保存在：" << newFilePath << std::endl;
    }
    return true;
}

bool extractTextSection(const std::string& pePath, const std::string& outputPath) {
    PEFile pe(pePath);
    if (!pe.isLoaded()) {
        std::cerr << "PE文件加载失败。" << std::endl;
        return false;
    }

    std::vector<char> textData = pe.getSectionData(".text");
    if (textData.empty()) {
        std::cerr << "没有找到.text节区" << std::endl;
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    outFile.write(textData.data(), textData.size());
    std::cout << ".text节区已提取并保存。" << std::endl;
    return true;
}

bool isHex(const std::string& s) {
    return s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}

void execute(const std::string& modifyPeFilePath, const std::string& textOrPePath) {
    if (modifyPeFilePath.empty() || textOrPePath.empty()) {
        std::cerr << "输入不能为空。" << std::endl;
        return;
    }

    if (modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".exe" && modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".dll") {
        std::cerr << "待修改的PE文件必须是.exe或.dll格式。" << std::endl;
        return;
    }

    PEFile pe(modifyPeFilePath);
    if (!pe.isLoaded()) {
        std::cerr << "无法加载PE文件。" << std::endl;
        return;
    }

    DWORD va = pe.findEntryPointVA();
    std::cout << "找到的入口点VA: 0x" << std::hex << va << std::endl;

    std::string textBinPath = textOrPePath;
    if (textOrPePath.substr(textOrPePath.size() - 4) == ".exe") {
        if (!checkFileReadable(textOrPePath)) {
            std::cerr << "PE文件不可读或不存在。" << std::endl;
            return;
        }
        textBinPath += ".text";
        extractTextSection(textOrPePath, textBinPath);
    }
    else if (textOrPePath.substr(textOrPePath.size() - 5) == ".text") {
        if (!checkFileReadable(textOrPePath)) {
            std::cerr << ".text文件不可读或不存在。" << std::endl;
            return;
        }
    }
    else {
        std::cerr << "提供的文件必须是PE文件或.text文件。" << std::endl;
        return;
    }

    replaceTextSection(modifyPeFilePath, textBinPath, va, false);
}

HWND hPeFileEdit, hTextFileEdit;
HFONT hFont;

void OnDropFiles(HWND hwnd, HDROP hDrop) {
    char szFileName[MAX_PATH];
    UINT fileCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

    if (fileCount != 2) {
        MessageBox(hwnd, "请拖放两个文件：一个PE文件和一个.text文件或PE文件。", "错误", MB_OK | MB_ICONERROR);
        DragFinish(hDrop);
        return;
    }

    DragQueryFile(hDrop, 0, szFileName, MAX_PATH);
    SetWindowText(hPeFileEdit, szFileName);

    DragQueryFile(hDrop, 1, szFileName, MAX_PATH);
    SetWindowText(hTextFileEdit, szFileName);

    DragFinish(hDrop);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE: {
        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(icex);
        icex.dwICC = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);
        hFont = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, TEXT("Segoe UI"));

        CreateWindow("STATIC", "PE文件路径:", WS_VISIBLE | WS_CHILD, 10, 10, 100, 20, hwnd, NULL, NULL, NULL);
        hPeFileEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 120, 10, 400, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hPeFileEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        CreateWindow("STATIC", "文本文件路径:", WS_VISIBLE | WS_CHILD, 10, 40, 100, 20, hwnd, NULL, NULL, NULL);
        hTextFileEdit = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOHSCROLL, 120, 40, 400, 20, hwnd, NULL, NULL, NULL);
        SendMessage(hTextFileEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

        HWND hExecuteButton = CreateWindow("BUTTON", "执行", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 120, 70, 80, 30, hwnd, (HMENU)1, NULL, NULL);
        SendMessage(hExecuteButton, WM_SETFONT, (WPARAM)hFont, TRUE);

        DragAcceptFiles(hwnd, TRUE);
        break;
    }
    case WM_DROPFILES:
        OnDropFiles(hwnd, (HDROP)wParam);
        break;
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            char peFilePath[MAX_PATH], textFilePath[MAX_PATH];
            GetWindowText(hPeFileEdit, peFilePath, MAX_PATH);
            GetWindowText(hTextFileEdit, textFilePath, MAX_PATH);

            execute(peFilePath, textFilePath);
        }
        break;
    case WM_DESTROY:
        DeleteObject(hFont);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "PE Editor";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "PE Editor",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 600, 150,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
