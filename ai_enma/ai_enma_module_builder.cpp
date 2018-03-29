#include "stdafx.h"
#include "ai_enma_module_builder.h"



ai_enma_module_builder::ai_enma_module_builder(ai_enma_module& module, bool build_relocations, std::vector<BYTE>& out_image) {

    align_sections(module.get_image_expanded());
    out_image = get_dos_header(module.get_image_expanded());
    
    dos_stub_size        = out_image.size();
    nt_stub_size         = (module.get_image_expanded().image.is_x32_image() ? sizeof(IMAGE_NT_HEADERS32) : sizeof(IMAGE_NT_HEADERS64));
   
    build_directories(module.get_image_expanded(), build_relocations);
    first_section_offset = align_sections(module.get_image_expanded());

    out_image.resize(first_section_offset);


    if (module.get_image_expanded().image.is_x32_image()) {
        IMAGE_NT_HEADERS32 header;
        get_nt_header32(module.get_image_expanded(), header);   
        memcpy(&out_image.data()[dos_stub_size], &header, sizeof(IMAGE_NT_HEADERS32));
    }
    else {
        IMAGE_NT_HEADERS64 header;
        get_nt_header64(module.get_image_expanded(), header);
        memcpy(&out_image.data()[dos_stub_size], &header, sizeof(IMAGE_NT_HEADERS64));
    }

    for (unsigned int section_idx = 0; section_idx < module.get_image_expanded().image.get_sections_number(); section_idx++) {
        IMAGE_SECTION_HEADER section_hdr = { 0 };
        
        memcpy(section_hdr.Name, module.get_image_expanded().image.get_section_by_idx(section_idx)->get_name().c_str(), 
            min(module.get_image_expanded().image.get_section_by_idx(section_idx)->get_name().length(), 8));

        section_hdr.Misc.VirtualSize = module.get_image_expanded().image.get_section_by_idx(section_idx)->get_virtual_size();
        section_hdr.VirtualAddress = module.get_image_expanded().image.get_section_by_idx(section_idx)->get_virtual_address();
        section_hdr.SizeOfRawData = module.get_image_expanded().image.get_section_by_idx(section_idx)->get_size_of_raw_data();
        section_hdr.PointerToRawData = module.get_image_expanded().image.get_section_by_idx(section_idx)->get_pointer_to_raw();
        section_hdr.Characteristics = module.get_image_expanded().image.get_section_by_idx(section_idx)->get_characteristics();
        
        memcpy(&out_image.data()[dos_stub_size + nt_stub_size + (sizeof(IMAGE_SECTION_HEADER) * section_idx)],
            &section_hdr, sizeof(IMAGE_SECTION_HEADER));
    } 


    out_image.resize(first_section_offset +
        module.get_image_expanded().image.get_section_by_idx(module.get_image_expanded().image.get_sections_number() - 1)->get_pointer_to_raw() +
        ALIGN_UP(
            module.get_image_expanded().image.get_section_by_idx(module.get_image_expanded().image.get_sections_number() - 1)->get_size_of_raw_data()
            , 0x200));
    
    for (auto& section_ : module.get_image_expanded().image.get_sections()) {
        memcpy(&out_image.data()[section_->get_pointer_to_raw()],
            section_->get_section_data().data(), section_->get_size_of_raw_data());
    }

    *(DWORD*)&out_image.data()[dos_stub_size + offsetof(IMAGE_NT_HEADERS32, OptionalHeader.CheckSum)] =
        calculate_checksum(out_image);
}


ai_enma_module_builder::~ai_enma_module_builder() {



}



void ai_enma_module_builder::build_directories(pe_image_expanded& expanded_image, bool build_relocations) {

    if (expanded_image.imports.get_libs().size() ||
        expanded_image.exports.get_number_of_functions() ||
        expanded_image.relocations.size() ||
        expanded_image.tls.get_address_of_index() ||
        (!expanded_image.image.is_x32_image() && expanded_image.exceptions.get_items().size()) ||
        (!expanded_image.load_config.is_empty())
        ) {

        bool was_build = false;

        pe_section dir_section;
        dir_section.set_name(std::string(".rdata"));
        dir_section.set_readable(true).set_writeable(true).set_executable(false);
        dir_section.set_pointer_to_raw(
            ALIGN_UP(
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_pointer_to_raw() +
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_size_of_raw_data()
                , 0x200)
        );
        dir_section.set_virtual_address(
            ALIGN_UP(
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_address() +
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_size()
                , 0x1000)
        );

        if ((!expanded_image.image.is_x32_image() && expanded_image.exceptions.get_items().size())) {
            build_exceptions_table(expanded_image.image, dir_section, expanded_image.exceptions);  //build exceptions
            was_build = true;
        }
        if (expanded_image.exports.get_number_of_functions()) {                                    //build export
            build_export_table(expanded_image.image, dir_section, expanded_image.exports);
            was_build = true;
        }
        if (expanded_image.imports.get_libs().size()) {                                            //build import
            build_import_table(expanded_image.image, dir_section, expanded_image.imports);
            was_build = true;
        }
        if (expanded_image.tls.get_address_of_index()) {                                           //build tls
            build_tls_table(expanded_image.image, dir_section, expanded_image.tls, expanded_image.relocations);
            was_build = true;
        }
        if (!expanded_image.load_config.is_empty()) {                                              //build load config
            build_load_config_table(expanded_image.image, dir_section, expanded_image.load_config, expanded_image.relocations);
            was_build = true;
        }
        if (expanded_image.relocations.size()) {                                                   //build relocations
            process_relocations(expanded_image);

            if (build_relocations) {
                build_relocation_table(expanded_image.image, dir_section, expanded_image.relocations);
                was_build = true;
            }
        }

        if (was_build) {
            dir_section.set_virtual_size(ALIGN_UP(dir_section.get_size_of_raw_data(), 0x1000));
            expanded_image.image.add_section(dir_section);
        }
    }


    
    if (expanded_image.resources.get_entry_list().size()) {                                        //build resources
        pe_section rsrc_section;
        rsrc_section.set_name(std::string(".rsrc"));
        rsrc_section.set_readable(true).set_writeable(false).set_executable(false);
        rsrc_section.set_pointer_to_raw(
            ALIGN_UP(
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_pointer_to_raw() +
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_size_of_raw_data()
                , 0x200)
        );
        rsrc_section.set_virtual_address(
            ALIGN_UP(
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_address() +
                expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_size()
                , 0x1000)
        );


        build_resources_table(expanded_image.image, rsrc_section, expanded_image.resources);

        rsrc_section.set_virtual_size(rsrc_section.get_size_of_raw_data());
        expanded_image.image.add_section(rsrc_section);
    }
}

void ai_enma_module_builder::process_relocations(pe_image_expanded& expanded_image) {

    for (auto& reloc_item : expanded_image.relocations.get_items()) {

        if (reloc_item.relocation_id & e_relocation_iat_address) {
            unsigned int reloc_lib_idx =  GET_HI_NUMBER(reloc_item.relocation_id);
            unsigned int reloc_func_idx = GET_LO_NUMBER(reloc_item.relocation_id);

            imported_library& reloc_ref_lib = expanded_image.imports.get_libs()[reloc_lib_idx];
            imported_func& reloc_ref_func = reloc_ref_lib.get_items()[reloc_func_idx];

            pe_section * target_section = expanded_image.image.get_section_by_rva(reloc_item.relative_virtual_address);

            if (!target_section) { continue; }

            if (expanded_image.image.is_x32_image()) {
                (*(DWORD*)&target_section->get_section_data()[
                    (reloc_item.relative_virtual_address) - target_section->get_virtual_address()
                ]) = (DWORD)expanded_image.image.rva_to_va(reloc_ref_func.get_iat_rva());
            }
            else {
                (*(DWORD64*)&target_section->get_section_data()[
                    (reloc_item.relative_virtual_address) - target_section->get_virtual_address()
                ]) = expanded_image.image.rva_to_va(reloc_ref_func.get_iat_rva());
            }

        }

    }
}


DWORD ai_enma_module_builder::align_sections(pe_image_expanded& expanded_image) {

    DWORD first_section_raw = ALIGN_UP((dos_stub_size + nt_stub_size +
        (sizeof(IMAGE_SECTION_HEADER) * expanded_image.image.get_sections_number())), 0x200);

    DWORD current_section_raw = first_section_raw;
    for (auto & section_ : expanded_image.image.get_sections()) {
        if (section_->get_size_of_raw_data() > section_->get_virtual_size()) {
            section_->set_virtual_size(section_->get_size_of_raw_data());
        }
        section_->set_pointer_to_raw(current_section_raw);

        current_section_raw += ALIGN_UP(section_->get_size_of_raw_data(), 0x200);
    }

    return first_section_raw;
}




#define GET_RICH_HASH(x,i) (((x) << (i)) | ((x) >> (32 - (i))))

std::vector<BYTE> ai_enma_module_builder::get_dos_header(pe_image_expanded& expanded_image) {

    std::vector<BYTE> header;
    std::vector<BYTE> dos_stub;

    if (expanded_image.image.get_dos_stub().get_dos_stub().size()) {
        dos_stub = expanded_image.image.get_dos_stub().get_dos_stub();
    }
    else {
        IMAGE_DOS_HEADER dos_header;
        ZeroMemory(&dos_header, sizeof(dos_header));
        dos_header.e_magic = IMAGE_DOS_SIGNATURE;
        dos_stub.resize(sizeof(IMAGE_DOS_HEADER));
        memcpy(dos_stub.data(), &dos_header, sizeof(IMAGE_DOS_HEADER));
        //todo 
    }

    header = dos_stub;

    
    if (expanded_image.image.get_rich_data().size()) {
        std::vector<DWORD> rich_stub;

        rich_stub.resize(4 + 
            (expanded_image.image.get_rich_data().size()*2) +
            4
        );

        DWORD * rich_dw = rich_stub.data();
        
        rich_dw[0] = 0x536E6144; //DanS
        for (unsigned int item_idx = 0; item_idx < expanded_image.image.get_rich_data().size(); item_idx++) {
            rich_dw[4 + (item_idx * 2)] = (expanded_image.image.get_rich_data()[item_idx].get_compiler_build() & 0xFFFF) |
                ((expanded_image.image.get_rich_data()[item_idx].get_type() & 0xFFFF) << 16);
            rich_dw[4 + (item_idx * 2) + 1] = expanded_image.image.get_rich_data()[item_idx].get_count();
        }
        rich_dw[4 + (expanded_image.image.get_rich_data().size() * 2)] = 0x68636952;//Rich

        DWORD rich_hash = dos_stub.size();

        for (unsigned int i = 0; i < dos_stub.size(); i++) { //dos header + stub
            if (i >= 0x3C && i < 0x40) { continue; }//skip e_lfanew

            rich_hash += GET_RICH_HASH((DWORD)header.data()[i],i);
        }
        for (unsigned int i = 0; i < expanded_image.image.get_rich_data().size(); i++){ //Rich struct
            rich_hash += GET_RICH_HASH(rich_dw[4 + (i * 2)], rich_dw[4 + (i * 2) + 1]);
        }

        for (unsigned int i = 0; i < 4 + (expanded_image.image.get_rich_data().size() * 2);i++) {
            rich_dw[i] ^= rich_hash;
        }
        rich_dw[4 + (expanded_image.image.get_rich_data().size() * 2) + 1] = rich_hash;//Rich hash
        

        header.resize(header.size() + ((expanded_image.image.get_rich_data().size() * 2) + 8)*sizeof(DWORD));
        memcpy(&header.data()[dos_stub.size()], rich_stub.data(), rich_stub.size() * sizeof(DWORD));
    }
   
    PIMAGE_DOS_HEADER(header.data())->e_lfanew = header.size();

    return header;
}


void ai_enma_module_builder::get_nt_header32(pe_image_expanded& expanded_image, IMAGE_NT_HEADERS32 &header) {

    ZeroMemory(&header,sizeof(IMAGE_NT_HEADERS32));

    header.Signature = IMAGE_NT_SIGNATURE;

 
    header.FileHeader.Machine                   = expanded_image.image.get_machine();
    header.FileHeader.NumberOfSections          = expanded_image.image.get_sections_number();
    header.FileHeader.TimeDateStamp             = 0;
    header.FileHeader.PointerToSymbolTable      = 0;
    header.FileHeader.NumberOfSymbols           = 0;
    header.FileHeader.SizeOfOptionalHeader      = sizeof(IMAGE_OPTIONAL_HEADER32);
    header.FileHeader.Characteristics           =  ~(~expanded_image.image.get_characteristics() |
        (IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_LINE_NUMS_STRIPPED | IMAGE_FILE_LOCAL_SYMS_STRIPPED | IMAGE_FILE_DEBUG_STRIPPED));

    header.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;

    header.OptionalHeader.MajorLinkerVersion = expanded_image.image.get_major_linker();
    header.OptionalHeader.MinorLinkerVersion = expanded_image.image.get_minor_linker();

    header.OptionalHeader.SizeOfCode                = 0;
    header.OptionalHeader.SizeOfInitializedData     = 0;
    header.OptionalHeader.SizeOfUninitializedData   = 0;

    header.OptionalHeader.AddressOfEntryPoint       = expanded_image.image.get_entry_point();

    header.OptionalHeader.BaseOfCode                = 0;
    header.OptionalHeader.BaseOfData                = 0;

    header.OptionalHeader.ImageBase                 = expanded_image.image.get_image_base();

    header.OptionalHeader.SectionAlignment  = expanded_image.image.get_section_align(); 
    header.OptionalHeader.FileAlignment     = expanded_image.image.get_file_align();

    header.OptionalHeader.MajorOperatingSystemVersion  = 5;//all windows versions from XP
    header.OptionalHeader.MinorOperatingSystemVersion  = 1;
    header.OptionalHeader.MajorImageVersion            = expanded_image.image.get_image_ver_major();
    header.OptionalHeader.MinorImageVersion            = expanded_image.image.get_image_ver_minor();
    header.OptionalHeader.MajorSubsystemVersion        = 5;
    header.OptionalHeader.MinorSubsystemVersion        = 1;

    header.OptionalHeader.Win32VersionValue = 0;
    
    header.OptionalHeader.SizeOfImage = ALIGN_UP(
        (expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_address() +
            expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_size()), 0x1000);

    header.OptionalHeader.SizeOfHeaders = first_section_offset;
      

    header.OptionalHeader.CheckSum = 0;

    header.OptionalHeader.Subsystem          = expanded_image.image.get_sub_system();
    header.OptionalHeader.DllCharacteristics = expanded_image.image.get_characteristics_dll();


    header.OptionalHeader.SizeOfStackReserve    = (DWORD)expanded_image.image.get_stack_reserve_size();
    header.OptionalHeader.SizeOfStackCommit     = (DWORD)expanded_image.image.get_stack_commit_size();
    header.OptionalHeader.SizeOfHeapReserve     = (DWORD)expanded_image.image.get_heap_reserve_size();
    header.OptionalHeader.SizeOfHeapCommit      = (DWORD)expanded_image.image.get_heap_commit_size();

    header.OptionalHeader.LoaderFlags = 0;
    header.OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

    for (unsigned int dir_idx = 0; dir_idx < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; dir_idx++) {
        header.OptionalHeader.DataDirectory[dir_idx].VirtualAddress = expanded_image.image.get_directory_virtual_address(dir_idx);
        header.OptionalHeader.DataDirectory[dir_idx].Size = expanded_image.image.get_directory_virtual_size(dir_idx);
    }

}

void ai_enma_module_builder::get_nt_header64(pe_image_expanded& expanded_image, IMAGE_NT_HEADERS64 &header) {
    ZeroMemory(&header, sizeof(IMAGE_NT_HEADERS64));

    header.Signature = IMAGE_NT_SIGNATURE;


    header.FileHeader.Machine               = expanded_image.image.get_machine();
    header.FileHeader.NumberOfSections      = expanded_image.image.get_sections_number();
    header.FileHeader.TimeDateStamp         = 0;
    header.FileHeader.PointerToSymbolTable  = 0;
    header.FileHeader.NumberOfSymbols       = 0;
    header.FileHeader.SizeOfOptionalHeader  = sizeof(IMAGE_OPTIONAL_HEADER64);
    header.FileHeader.Characteristics       = ~(~expanded_image.image.get_characteristics() |
        (IMAGE_FILE_RELOCS_STRIPPED | IMAGE_FILE_LINE_NUMS_STRIPPED | IMAGE_FILE_LOCAL_SYMS_STRIPPED | IMAGE_FILE_DEBUG_STRIPPED));

    header.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;

    header.OptionalHeader.MajorLinkerVersion = expanded_image.image.get_major_linker();
    header.OptionalHeader.MinorLinkerVersion = expanded_image.image.get_minor_linker();

    header.OptionalHeader.SizeOfCode = 0;
    header.OptionalHeader.SizeOfInitializedData     = 0;
    header.OptionalHeader.SizeOfUninitializedData   = 0;

    header.OptionalHeader.AddressOfEntryPoint = expanded_image.image.get_entry_point();

    header.OptionalHeader.BaseOfCode = 0;

    header.OptionalHeader.ImageBase = expanded_image.image.get_image_base();

    header.OptionalHeader.SectionAlignment = expanded_image.image.get_section_align();
    header.OptionalHeader.FileAlignment = expanded_image.image.get_file_align();

    header.OptionalHeader.MajorOperatingSystemVersion = 5;//all windows versions from XP
    header.OptionalHeader.MinorOperatingSystemVersion = 1;
    header.OptionalHeader.MajorImageVersion = expanded_image.image.get_image_ver_major();
    header.OptionalHeader.MinorImageVersion = expanded_image.image.get_image_ver_minor();
    header.OptionalHeader.MajorSubsystemVersion = 5;
    header.OptionalHeader.MinorSubsystemVersion = 1;

    header.OptionalHeader.Win32VersionValue = 0;

    header.OptionalHeader.SizeOfImage = ALIGN_UP(
        (expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_address() +
            expanded_image.image.get_sections()[expanded_image.image.get_sections_number() - 1]->get_virtual_size()), 0x1000);

    header.OptionalHeader.SizeOfHeaders = first_section_offset;


    header.OptionalHeader.CheckSum = 0;

    header.OptionalHeader.Subsystem = expanded_image.image.get_sub_system();
    header.OptionalHeader.DllCharacteristics = expanded_image.image.get_characteristics_dll();


    header.OptionalHeader.SizeOfStackReserve    = expanded_image.image.get_stack_reserve_size();
    header.OptionalHeader.SizeOfStackCommit     = expanded_image.image.get_stack_commit_size();
    header.OptionalHeader.SizeOfHeapReserve     = expanded_image.image.get_heap_reserve_size();
    header.OptionalHeader.SizeOfHeapCommit      = expanded_image.image.get_heap_commit_size();

    header.OptionalHeader.LoaderFlags = 0;
    header.OptionalHeader.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;

    for (unsigned int dir_idx = 0; dir_idx < IMAGE_NUMBEROF_DIRECTORY_ENTRIES; dir_idx++) {
        header.OptionalHeader.DataDirectory[dir_idx].VirtualAddress = expanded_image.image.get_directory_virtual_address(dir_idx);
        header.OptionalHeader.DataDirectory[dir_idx].Size = expanded_image.image.get_directory_virtual_size(dir_idx);
    }

}