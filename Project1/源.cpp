#include <iostream>
#include "CResourceExtractor.h"
#include <tchar.h>

int main()
{
    CResourceExtractor obj;
    obj.Load(_T(R"(1.exe)"));
    std::vector<ICON_GROUP> vInfos = obj.GetIconGroups();
    obj.ExtractIconGroupToFile(0, _T("2.ico"));
    obj.ExtractIconToFile(0, 0, _T("wps_0.ico"));
    obj.ExtractIconToFile(0, 1, _T("wps_1.ico"));
    obj.ExtractIconToFile(0, 2, _T("wps_2.ico"));
    obj.ExtractIconToFile(0, 3, _T("wps_3.ico"));
    obj.ExtractIconToFile(0, 4, _T("wps_4.ico"));
    obj.ExtractIconToFile(0, 5, _T("wps_5.ico"));
    obj.ExtractIconToFile(0, 6, _T("wps_6.ico"));
    obj.ExtractIconToFile(0, 7, _T("wps_7.ico"));
    obj.ExtractIconToFile(0, 8, _T("wps_8.ico"));
    obj.ExtractIconToFile(0, 9, _T("wps_9.ico"));
    obj.ExtractIconToFile(0, 10, _T("wps_10.ico"));

    return 0;
}