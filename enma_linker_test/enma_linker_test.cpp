
#include "stdafx.h"


int main(int argc, const char **argv){

    ai_enma enma;
    std::vector<ai_enma_module *> modules;

    if (argc < 3) {
        printf("need more parameters !");
        system("PAUSE");
        return 0;
    }


    for (unsigned int arg_idx = 1; arg_idx < argc; arg_idx++) {

        //module=
        if (lstrlenA(argv[arg_idx]) > 7 && !strncmp(argv[arg_idx],"module=",7)) {
            pe_image image = pe_image(std::string(argv[arg_idx]+7));
            if (image.get_image_status() == pe_image_status::pe_image_status_ok) {
                modules.push_back(new ai_enma_module(image));
                continue;
            }
            printf("[%s] image error!\n", std::string(argv[arg_idx] + 7).c_str());
            system("PAUSE");
            return 0;
        }
        //extname=
        if (lstrlenA(argv[arg_idx]) > 8 && !strncmp(argv[arg_idx], "extname=", 8)) {
            if (modules.size()) {
                modules[modules.size() - 1]->add_ext_name(std::string(argv[arg_idx] + 8));
                continue;
            }
            printf("[%s] extname error!\n", std::string(argv[arg_idx] + 8).c_str());
            system("PAUSE");
            return 0;
        }
    }

    enma.set_main_module(modules[0]);
    for (unsigned int module_idx = 1; module_idx < modules.size(); module_idx++) {
        enma.add_extended_module(modules[module_idx]);
    }

    std::vector<BYTE> out_exe;
    printf("enma code %d\n", enma.exec_enma(out_exe));

    HANDLE hTargetFile = CreateFileA("enma_result.exe", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hTargetFile != INVALID_HANDLE_VALUE) {
        DWORD npd;
        WriteFile(hTargetFile, out_exe.data(), out_exe.size(), &npd, 0);
        CloseHandle(hTargetFile);
    }

    system("PAUSE");
    return 0;
}

