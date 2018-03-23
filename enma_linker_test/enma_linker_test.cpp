
#include "stdafx.h"


int main(){


    ai_enma enma;
    std::vector<BYTE> out_exe;
    ai_enma_module config_image(pe_image(std::string("..\\app for test\\loader.exe")));
    ai_enma_module added_image(pe_image(std::string("..\\app for test\\TestDll.dll")));
    added_image.add_ext_name("user32.dll");

    enma.set_main_module(&config_image);
    enma.add_extended_module(&added_image);

    printf("enma code %d\n", enma.exec_enma(out_exe));

    HANDLE hTargetFile = CreateFile(L"..\\app for test\\test_result.exe", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hTargetFile != INVALID_HANDLE_VALUE) {
        DWORD npd;
        WriteFile(hTargetFile, out_exe.data(), out_exe.size(), &npd, 0);
        CloseHandle(hTargetFile);
    }


    return 0;
}

