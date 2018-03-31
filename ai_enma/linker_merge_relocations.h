#pragma once

bool ai_enma_module_linker::process_relocations(pe_image_expanded& expanded_image) {

    expanded_image.relocations.sort();
    std::vector<relocation_item>& items = expanded_image.relocations.get_items();
    pe_section * item_section = 0;
    for (unsigned int item_idx = 0, item_rva_low = 0, item_rva_high = 0; item_idx < expanded_image.relocations.size(); item_idx++) {
        if (!item_rva_low ||!item_section ||
            items[item_idx].relative_virtual_address < item_rva_low ||
            items[item_idx].relative_virtual_address >= item_rva_high
            ) {

            item_section = expanded_image.image.get_section_by_rva(items[item_idx].relative_virtual_address);
            if (!item_section) { return false; }

            item_rva_low = item_section->get_virtual_address();
            item_rva_high = ALIGN_UP(item_section->get_virtual_address() + item_section->get_virtual_size(), 0x1000);
        }


        if (expanded_image.image.is_x32_image()) {
            items[item_idx].data = expanded_image.image.va_to_rva(*(DWORD*)&item_section->get_section_data().data()[
                items[item_idx].relative_virtual_address - item_section->get_virtual_address()
            ]);
        }
        else {
            items[item_idx].data = expanded_image.image.va_to_rva(*(DWORD64*)&item_section->get_section_data().data()[
                items[item_idx].relative_virtual_address - item_section->get_virtual_address()
            ]);
        }
    }

    for (auto& item : expanded_image.relocations.get_items()) {

        DWORD relocation_dst_rva = (DWORD)item.data;


        for (unsigned int lib_idx = 0; lib_idx < expanded_image.imports.get_libs().size(); lib_idx++) {//find rva in imports

            if (expanded_image.image.is_x32_image()) {
                if (relocation_dst_rva >= expanded_image.imports.get_libs()[lib_idx].get_rva_iat() &&
                    relocation_dst_rva < expanded_image.imports.get_libs()[lib_idx].get_items().size() * sizeof(DWORD) + expanded_image.imports.get_libs()[lib_idx].get_rva_iat()) {

                    item.relocation_id = SET_RELOCATION_ID_IAT(lib_idx, ((relocation_dst_rva - expanded_image.imports.get_libs()[lib_idx].get_rva_iat()) / sizeof(DWORD)));
                    goto go_next_item_;
                }
            }
            else {
                if (relocation_dst_rva >= expanded_image.imports.get_libs()[lib_idx].get_rva_iat() &&
                    relocation_dst_rva < expanded_image.imports.get_libs()[lib_idx].get_items().size() * sizeof(DWORD64) + expanded_image.imports.get_libs()[lib_idx].get_rva_iat()) {

                    item.relocation_id = SET_RELOCATION_ID_IAT(lib_idx, ((relocation_dst_rva - expanded_image.imports.get_libs()[lib_idx].get_rva_iat()) / sizeof(DWORD64)));
                    goto go_next_item_;
                }
            }

        }

        if (expanded_image.tls.get_address_of_index()) {           //find rva in tls
            if (relocation_dst_rva == expanded_image.tls.get_address_of_index()) {
                item.relocation_id = e_relocation_tls_index;
                continue;
            }
        }


        item.relocation_id = e_relocation_default;
    go_next_item_:;
    }


    return true;
}


bool ai_enma_module_linker::merge_relocations() {
    for (auto& plugin : plugins) { plugin->on_before_merge_relocations(this); }

    for (auto& module : extended_modules) {

        for (auto& item : module->get_image_relocations().get_items()) { //handle image relocations

            switch (item.relocation_id) {

            case e_relocation_default: {

                pe_section * target_section = main_module->get_image().get_section_by_rva(item.relative_virtual_address + module->get_address_offset());

                if (!target_section) {
                    return false;
                }

                if (main_module->get_image().is_x32_image()) {
                    (*(DWORD*)&target_section->get_section_data()[
                        (item.relative_virtual_address + module->get_address_offset()) - target_section->get_virtual_address()
                    ]) += DWORD(main_module->get_image().get_image_base() - module->get_image().get_image_base()) + module->get_address_offset();
                }
                else {
                    (*(DWORD64*)&target_section->get_section_data()[
                        (item.relative_virtual_address + module->get_address_offset()) - target_section->get_virtual_address()
                    ]) += (main_module->get_image().get_image_base() - module->get_image().get_image_base()) + module->get_address_offset();
                }

                main_module->get_image_relocations().add_item(item.relative_virtual_address + module->get_address_offset(), item.relocation_id);
                break;
            }
            case e_relocation_tls_index: {
                //todo
                main_module->get_image_relocations().add_item(item.relative_virtual_address + module->get_address_offset(), item.relocation_id);
                break;
            }
            case e_relocation_security_cookie: {
                //todo
                main_module->get_image_relocations().add_item(item.relative_virtual_address + module->get_address_offset(), item.relocation_id);
                break;
            }
            default: {

                if (item.relocation_id&e_relocation_iat_address) {

                    unsigned int src_lib_idx = GET_HI_NUMBER(item.relocation_id);
                    unsigned int src_func_idx = GET_LO_NUMBER(item.relocation_id);

                    imported_library& src_library = module->get_image_imports().get_libs()[src_lib_idx];
                    imported_func&    src_func = src_library.get_items()[src_func_idx];

                    unsigned int dst_lib_idx;
                    unsigned int dst_func_idx;

                    if (src_func.is_import_by_name()) {
                        if (get_import_func_index(main_module->get_image_imports(),
                            src_library.get_library_name(), src_func.get_func_name(),
                            dst_lib_idx, dst_func_idx)) {

                            main_module->get_image_relocations().add_item(
                                item.relative_virtual_address + module->get_address_offset(),
                                SET_RELOCATION_ID_IAT(dst_lib_idx, dst_func_idx)
                            );

                            break;
                        }
                    }
                    else {
                        if (get_import_func_index(main_module->get_image_imports(),
                            src_library.get_library_name(), src_func.get_ordinal(),
                            dst_lib_idx, dst_func_idx)) {

                            main_module->get_image_relocations().add_item(
                                item.relative_virtual_address + module->get_address_offset(),
                                SET_RELOCATION_ID_IAT(dst_lib_idx, dst_func_idx)
                            );

                            break;
                        }
                    }
                }
                return false;
            }
            }

        }
    }

    for (auto& plugin : plugins) { plugin->on_after_merge_relocations(this); }
	return true;
}


