#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

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
        std::cerr << "�޷����ļ�: " << filePath << std::endl;
        return false;
    }

    file.read(reinterpret_cast<char*>(&dosHeader), sizeof(dosHeader));
    if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
        std::cerr << "��Ч��DOSǩ��" << std::endl;
        return false;
    }

    file.seekg(dosHeader.e_lfanew, std::ios::beg);
    file.read(reinterpret_cast<char*>(&ntHeaders), sizeof(ntHeaders));
    if (ntHeaders.Signature != IMAGE_NT_SIGNATURE) {
        std::cerr << "��Ч��NTǩ��" << std::endl;
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
        std::cerr << "�޷��ҵ���Ӧ���ļ�ƫ�ƣ�RVA ���ܲ����κν����С�" << std::endl;
        return false;
    }

    std::ifstream textFile(textBinPath, std::ios::binary);
    std::vector<char> textData((std::istreambuf_iterator<char>(textFile)), std::istreambuf_iterator<char>());

    if (!flag) {
        std::ofstream peFile(peFilePath, std::ios::binary | std::ios::in | std::ios::out);
        peFile.seekp(fileOffset);
        peFile.write(textData.data(), textData.size());
        std::cout << ".text�����ѳɹ�������PE�ļ��С�" << std::endl;
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

        std::cout << ".text�����ѳɹ�������PE�ļ��У�fuzz�ļ������ڣ�" << newFilePath << std::endl;
    }
    return true;
}

bool extractTextSection(const std::string& pePath, const std::string& outputPath) {
    PEFile pe(pePath);
    if (!pe.isLoaded()) {
        std::cerr << "PE�ļ�����ʧ�ܡ�" << std::endl;
        return false;
    }

    std::vector<char> textData = pe.getSectionData(".text");
    if (textData.empty()) {
        std::cerr << "û���ҵ�.text����" << std::endl;
        return false;
    }

    std::ofstream outFile(outputPath, std::ios::binary);
    outFile.write(textData.data(), textData.size());
    std::cout << ".text��������ȡ�����档" << std::endl;
    return true;
}

bool isHex(const std::string& s) {
    return s.find_first_not_of("0123456789abcdefABCDEF") == std::string::npos;
}

void execute(const std::string& modifyPeFilePath, const std::string& textOrPePath) {
    if (modifyPeFilePath.empty() || textOrPePath.empty()) {
        std::cerr << "���벻��Ϊ�ա�" << std::endl;
        return;
    }

    if (modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".exe" && modifyPeFilePath.substr(modifyPeFilePath.size() - 4) != ".dll") {
        std::cerr << "���޸ĵ�PE�ļ�������.exe��.dll��ʽ��" << std::endl;
        return;
    }

    PEFile pe(modifyPeFilePath);
    if (!pe.isLoaded()) {
        std::cerr << "�޷�����PE�ļ���" << std::endl;
        return;
    }

    DWORD va = pe.findEntryPointVA();
    std::cout << "�ҵ�����ڵ�VA: 0x" << std::hex << va << std::endl;

    std::string textBinPath = textOrPePath;
    if (textOrPePath.substr(textOrPePath.size() - 4) == ".exe") {
        if (!checkFileReadable(textOrPePath)) {
            std::cerr << "PE�ļ����ɶ��򲻴��ڡ�" << std::endl;
            return;
        }
        textBinPath += ".text";
        extractTextSection(textOrPePath, textBinPath);
    }
    else if (textOrPePath.substr(textOrPePath.size() - 5) == ".text") {
        if (!checkFileReadable(textOrPePath)) {
            std::cerr << ".text�ļ����ɶ��򲻴��ڡ�" << std::endl;
            return;
        }
    }
    else {
        std::cerr << "�ṩ���ļ�������PE�ļ���.text�ļ���" << std::endl;
        return;
    }

    replaceTextSection(modifyPeFilePath, textBinPath, va, false);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <PE file path> <.text file or PE file path>" << std::endl;
        return 1;
    }

    std::string modifyPeFilePath = argv[1];
    std::string textOrPePath = argv[2];
    execute(modifyPeFilePath, textOrPePath);
    return 0;
}
