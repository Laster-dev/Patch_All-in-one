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
        CNativePatchMFCDlg::LogMessage(_T("�޷����ļ�: ") + CString(filePath.c_str()));
        return false;
    }

    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        CNativePatchMFCDlg::LogMessage(_T("��Ч��DOSǩ��"));
        return false;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        CNativePatchMFCDlg::LogMessage(_T("��Ч��NTǩ��"));
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
        CNativePatchMFCDlg::LogMessage(_T("�޷��ҵ���Ӧ���ļ�ƫ�ƣ�RVA ���ܲ����κν����С�"));
        return result;  // ���ؿյ�vector
    }

    std::ifstream textFile(textBinPath, std::ios::binary);
    std::vector<char> textData((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());

    // ��ȡԭPE�ļ�����
    std::ifstream peFile(peFilePath, std::ios::binary);
    std::vector<char> peContent((std::istreambuf_iterator<char>(peFile)), std::istreambuf_iterator<char>());

    // ���ڴ����޸�.text����
    if (fileOffset + textData.size() <= peContent.size()) {
        std::copy(textData.begin(), textData.end(), peContent.begin() + fileOffset);
        CNativePatchMFCDlg::LogMessage(_T(".text�����ѳɹ�������PE�ļ��С�"));
    }
    else {
        CNativePatchMFCDlg::LogMessage(_T(".text���ݳ���PE�ļ���С���޷����ǡ�"));
        return result;  // ���ؿյ�vector
    }

    result = peContent;
    CNativePatchMFCDlg::LogMessage(_T("�ļ���С��"));
    return result;
}

bool extractTextSection(const std::string& pePath, const std::string& outputPath) {
    PEFile pe(pePath);
    if (!pe.isLoaded()) {
        CNativePatchMFCDlg::LogMessage(_T("PE�ļ�����ʧ�ܡ�"));
        return false;
    }

    std::vector<char> textData = pe.getSectionData(".text");
    if (textData.empty()) {
        CNativePatchMFCDlg::LogMessage(_T("û���ҵ�.text����"));
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    outFile.write(textData.data(), textData.size());
    CNativePatchMFCDlg::LogMessage(_T(".text��������ȡ�����档"));
    return true;
}

bool isHex(const std::string& s) {
    return s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}
void LogVectorData(const std::vector<char>& data) {
    CString hexString;

    for (char byte : data) {
        // ÿ���ֽ�ת��Ϊʮ�����Ʋ�׷�ӵ� CString ��
        CString temp;
        temp.Format(_T("%02X"), (unsigned char)byte);
        hexString += temp;
    }

    // ʹ�� LogMessage ���
    CNativePatchMFCDlg::LogMessage(hexString);
}

std::vector<char> execute(const std::string& modifyPeFilePath, const std::string& textOrPePath) {
    if (modifyPeFilePath.empty() || textOrPePath.empty()) {
        CNativePatchMFCDlg::LogMessage(_T("���벻��Ϊ�ա�"));
        return {};
    }

    if (modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".exe" && modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".dll") {
        CNativePatchMFCDlg::LogMessage(_T("���޸ĵ�PE�ļ�������.exe��.dll��ʽ��"));
        return {};
    }

    PEFile pe(modifyPeFilePath);
    if (!pe.isLoaded()) {
        CNativePatchMFCDlg::LogMessage(_T("�޷�����PE�ļ���"));
        return {};
    }

    DWORD va = pe.findEntryPointVA();
    CString buffer;
    buffer.Format(_T("�ҵ�����ڵ�VA: 0x%08X"), va);
    CNativePatchMFCDlg::LogMessage(buffer);

    std::string textBinPath = textOrPePath;
    if (textOrPePath.substr(textOrPePath.size() - 4) == ".exe") {
        if (!checkFileReadable(textOrPePath)) {
            CNativePatchMFCDlg::LogMessage(_T("PE�ļ����ɶ��򲻴��ڡ�"));
            return {};
        }
        textBinPath += ".text";
        extractTextSection(textOrPePath, textBinPath);
    }
    else if (textOrPePath.substr(textOrPePath.size() - 5) == ".text") {
        if (!checkFileReadable(textOrPePath)) {
            CNativePatchMFCDlg::CNativePatchMFCDlg::LogMessage(_T(".text�ļ����ɶ��򲻴��ڡ�"));
            return {};
        }
    }
    else {
        CNativePatchMFCDlg::LogMessage(_T("�ṩ���ļ�������PE�ļ���.text�ļ���"));
        return {};
    }
    std::vector<char> bytes = replaceTextSection(modifyPeFilePath, textBinPath, va);
    //LogVectorData(bytes);
    return bytes;
}
