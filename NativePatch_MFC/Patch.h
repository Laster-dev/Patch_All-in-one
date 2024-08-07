#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
void ShowMessage(const std::string& message, const std::string& title) {
    MessageBoxA(NULL, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
}
void ShowError(const std::string& message) {
    MessageBoxA(NULL, message.c_str(), "错误", MB_OK | MB_ICONERROR);
}

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
        ShowError("无法打开文件: " + filePath);
        return false;
    }

    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        ShowError("无效的DOS签名");
        return false;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        ShowError("无效的NT签名");
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
        ShowMessage("无法找到对应的文件偏移，RVA 可能不在任何节区中。", "错误");
        return false;
    }

    std::ifstream textFile(textBinPath, std::ios::binary);
    std::vector<char> textData((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());

    if (!flag) {
        std::ofstream peFile(peFilePath, std::ios::binary | std::ios::in | std::ios::out);
        peFile.seekp(fileOffset);
        peFile.write(textData.data(), textData.size());
        ShowMessage(".text节区已成功覆盖在PE文件中。", "成功");
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

        std::string message = ".text节区已成功覆盖在PE文件中，fuzz文件保存在：" + newFilePath;
        ShowMessage(message, "成功");
    }
    return true;
}

bool extractTextSection(const std::string& pePath, const std::string& outputPath) {
    PEFile pe(pePath);
    if (!pe.isLoaded()) {
        ShowMessage("PE文件加载失败。", "错误");
        return false;
    }

    std::vector<char> textData = pe.getSectionData(".text");
    if (textData.empty()) {
        ShowMessage("没有找到.text节区", "错误");
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    outFile.write(textData.data(), textData.size());
    ShowMessage(".text节区已提取并保存。", "成功");
    return true;
}

bool isHex(const std::string& s) {
    return s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}

void execute(const std::string& modifyPeFilePath, const std::string& textOrPePath) {
    if (modifyPeFilePath.empty() || textOrPePath.empty()) {
        MessageBoxA(0, "输入不能为空。", 0, 0);
        return;
    }

    if (modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".exe" && modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".dll") {
        MessageBoxA(0, "待修改的PE文件必须是.exe或.dll格式。", 0, 0);
        return;
    }

    PEFile pe(modifyPeFilePath);
    if (!pe.isLoaded()) {
        MessageBoxA(0, "无法加载PE文件。", 0, 0);
        return;
    }

    DWORD va = pe.findEntryPointVA();
    std::cout << "找到的入口点VA: 0x" << std::hex << va << std::endl;
    char buffer[256];
    // 使用 sprintf 格式化字符串
    sprintf(buffer, "找到的入口点VA: 0x%08X", va);

    // 显示 MessageBox
    MessageBoxA(NULL, buffer, "信息", MB_OK | MB_ICONINFORMATION);


    std::string textBinPath = textOrPePath;
    if (textOrPePath.substr(textOrPePath.size() - 4) == ".exe") {
        if (!checkFileReadable(textOrPePath)) {
             MessageBoxA(0, "PE文件不可读或不存在。", 0, 0);
            return;
        }
        textBinPath += ".text";
        extractTextSection(textOrPePath, textBinPath);
    }
    else if (textOrPePath.substr(textOrPePath.size() - 5) == ".text") {
        if (!checkFileReadable(textOrPePath)) {
            MessageBoxA(0, ".text文件不可读或不存在。", 0, 0);
            return;
        }
    }
    else {
        MessageBoxA(0, "提供的文件必须是PE文件或.text文件。", 0, 0);
        return;
    }

    replaceTextSection(modifyPeFilePath, textBinPath, va, false);
}


