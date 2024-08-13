#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>
#include "NativePatch_MFCDlg.h"

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
        CNativePatchMFCDlg::LogMessage(_T("无法打开文件: ") + CString(filePath.c_str()));
        return false;
    }

    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        CNativePatchMFCDlg::LogMessage(_T("无效的DOS签名"));
        return false;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        CNativePatchMFCDlg::LogMessage(_T("无效的NT签名"));
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

std::vector<char> replaceTextSection(const std::string& peFilePath, const std::string& textBinPath, DWORD va) {
    std::vector<char> result;

    PEFile pe(peFilePath);
    DWORD rva = vaToRva(pe, va);
    DWORD fileOffset = pe.rvaToOffset(rva);

    if (fileOffset == -1) {
        CNativePatchMFCDlg::LogMessage(_T("无法找到对应的文件偏移，RVA 可能不在任何节区中。"));
        return result;  // 返回空的vector
    }

    std::ifstream textFile(textBinPath, std::ios::binary);
    std::vector<char> textData((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());

    // 读取原PE文件内容
    std::ifstream peFile(peFilePath, std::ios::binary);
    std::vector<char> peContent((std::istreambuf_iterator<char>(peFile)), std::istreambuf_iterator<char>());

    // 在内存中修改.text节区
    if (fileOffset + textData.size() <= peContent.size()) {
        std::copy(textData.begin(), textData.end(), peContent.begin() + fileOffset);
        CNativePatchMFCDlg::LogMessage(_T(".text节区已成功覆盖在PE文件中。"));
    }
    else {
        CNativePatchMFCDlg::LogMessage(_T(".text数据超出PE文件大小，无法覆盖。"));
        return result;  // 返回空的vector
    }

    result = peContent;
    CNativePatchMFCDlg::LogMessage(_T("文件大小："));
    return result;
}

bool extractTextSection(const std::string& pePath, const std::string& outputPath) {
    PEFile pe(pePath);
    if (!pe.isLoaded()) {
        CNativePatchMFCDlg::LogMessage(_T("PE文件加载失败。"));
        return false;
    }

    std::vector<char> textData = pe.getSectionData(".text");
    if (textData.empty()) {
        CNativePatchMFCDlg::LogMessage(_T("没有找到.text节区"));
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    outFile.write(textData.data(), textData.size());
    CNativePatchMFCDlg::LogMessage(_T(".text节区已提取并保存。"));
    return true;
}

bool isHex(const std::string& s) {
    return s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}
void LogVectorData(const std::vector<char>& data) {
    CString hexString;

    for (char byte : data) {
        // 每个字节转换为十六进制并追加到 CString 中
        CString temp;
        temp.Format(_T("%02X"), (unsigned char)byte);
        hexString += temp;
    }

    // 使用 LogMessage 输出
    CNativePatchMFCDlg::LogMessage(hexString);
}

std::vector<char> execute(const std::string& modifyPeFilePath, const std::string& textOrPePath) {
    if (modifyPeFilePath.empty() || textOrPePath.empty()) {
        CNativePatchMFCDlg::LogMessage(_T("输入不能为空。"));
        return {};
    }

    if (modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".exe" && modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".dll") {
        CNativePatchMFCDlg::LogMessage(_T("待修改的PE文件必须是.exe或.dll格式。"));
        return {};
    }

    PEFile pe(modifyPeFilePath);
    if (!pe.isLoaded()) {
        CNativePatchMFCDlg::LogMessage(_T("无法加载PE文件。"));
        return {};
    }

    DWORD va = pe.findEntryPointVA();
    CString buffer;
    buffer.Format(_T("找到的入口点VA: 0x%08X"), va);
    CNativePatchMFCDlg::LogMessage(buffer);

    std::string textBinPath = textOrPePath;
    if (textOrPePath.substr(textOrPePath.size() - 4) == ".exe") {
        if (!checkFileReadable(textOrPePath)) {
            CNativePatchMFCDlg::LogMessage(_T("PE文件不可读或不存在。"));
            return {};
        }
        textBinPath += ".text";
        extractTextSection(textOrPePath, textBinPath);
    }
    else if (textOrPePath.substr(textOrPePath.size() - 5) == ".text") {
        if (!checkFileReadable(textOrPePath)) {
            CNativePatchMFCDlg::CNativePatchMFCDlg::LogMessage(_T(".text文件不可读或不存在。"));
            return {};
        }
    }
    else {
        CNativePatchMFCDlg::LogMessage(_T("提供的文件必须是PE文件或.text文件。"));
        return {};
    }
    std::vector<char> bytes = replaceTextSection(modifyPeFilePath, textBinPath, va);
    //LogVectorData(bytes);
    return bytes;
}
