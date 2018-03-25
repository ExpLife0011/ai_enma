#pragma once


bool ai_enma_module_linker::get_export_references(std::vector<export_references>& export_refs) {
    import_table imports = main_module->get_image_imports();
    std::vector<ai_enma_module*> modules_pool;

    modules_pool.push_back(main_module);
    for (auto& module_ : extended_modules) { modules_pool.push_back(module_); }


    for (unsigned int module_idx = 0; module_idx < modules_pool.size(); module_idx++) {
        export_table& module_export = modules_pool[module_idx]->get_image_exports();

        if (module_export.get_items().size() || 
            modules_pool[module_idx]->get_extended_exports().size()) {

            for (unsigned int import_lib_idx = 0; import_lib_idx < imports.get_libs().size(); import_lib_idx++) {
                imported_library & import_lib = imports.get_libs()[import_lib_idx];
                std::string current_export_lib_name;

                bool is_name_match = false;

                //check for main export
                if (nocase_cmp(module_export.get_name(), import_lib.get_name())) { //check for original name
                    current_export_lib_name = module_export.get_name();
                    is_name_match = true;
                }
                else {
                    for (auto& ext_name : modules_pool[module_idx]->get_extended_names()) {//check for ext names
                        if (nocase_cmp(ext_name, import_lib.get_name())) {
                            current_export_lib_name = ext_name;
                            is_name_match = true;
                            break;
                        }
                    }
                }

                if (!is_name_match) {
                    for (auto& ext_export : modules_pool[module_idx]->get_extended_exports()) {//check for ext modules

                        for (auto& ext_name : ext_export.extended_names) {
                            if (nocase_cmp(ext_name, import_lib.get_name())) {
                                current_export_lib_name = ext_name;
                                is_name_match = true;
                                break;
                            }
                        }


                        if (is_name_match) {       
                            for (unsigned int import_func_idx = 0; import_func_idx < import_lib.get_items().size(); import_func_idx++) {
                                imported_func& import_func = import_lib.get_items()[import_func_idx];

                                for (auto & export_item : module_export.get_items()) {
                                    if (import_func.is_import_by_name() == export_item.has_name()) {
                                        if (import_func.is_import_by_name()) {
                                            if (import_func.get_name() == export_item.get_name()) {
                                                export_refs.push_back({
                                                    0,
                                                    current_export_lib_name,
                                                    export_item,
                                                    module_idx - 1
                                                });

                                                import_lib.get_items().erase(import_lib.get_items().begin() + import_func_idx);
                                                import_func_idx--;
                                                goto next_main_import_item_ext_export;
                                            }
                                        }
                                        else {
                                            if (import_func.get_ordinal() == export_item.get_name_ordinal()) {
                                                export_refs.push_back({
                                                    0,
                                                    current_export_lib_name,
                                                    export_item,
                                                    module_idx - 1
                                                });

                                                import_lib.get_items().erase(import_lib.get_items().begin() + import_func_idx);
                                                import_func_idx--;
                                                goto next_main_import_item_ext_export;
                                            }
                                        }
                                    }
                                }
                            next_main_import_item_ext_export:;
                            }
                            if (!import_lib.get_items().size()) { //delete import lib if its empty
                                imports.get_libs().erase(imports.get_libs().begin() + import_lib_idx);
                                import_lib_idx--;
                            }
                        }
                    }
                }else{
                    for (unsigned int import_func_idx = 0; import_func_idx < import_lib.get_items().size(); import_func_idx++) {
                        imported_func& import_func = import_lib.get_items()[import_func_idx];

                        for (auto & export_item : module_export.get_items()) { 
                            if (import_func.is_import_by_name() == export_item.has_name()) {
                                if (import_func.is_import_by_name()) {
                                    if (import_func.get_name() == export_item.get_name()) {
                                        export_refs.push_back({
                                            0,
                                            current_export_lib_name,
                                            export_item,
                                            module_idx-1
                                        });

                                        import_lib.get_items().erase(import_lib.get_items().begin() + import_func_idx);
                                        import_func_idx--;
                                        goto next_main_import_item;
                                    }
                                }
                                else {
                                    if (import_func.get_ordinal() == export_item.get_name_ordinal()) {
                                        export_refs.push_back({
                                            0,
                                            current_export_lib_name,
                                            export_item,
                                            module_idx - 1
                                        });

                                        import_lib.get_items().erase(import_lib.get_items().begin() + import_func_idx);
                                        import_func_idx--;
                                        goto next_main_import_item;
                                    }
                                }
                            }
                        }
                    next_main_import_item:;
                    }
                    if (!import_lib.get_items().size()) { //delete import lib if its empty
                        imports.get_libs().erase(imports.get_libs().begin() + import_lib_idx);
                        import_lib_idx--;
                    }
                }
            }
        }
    }

    return export_refs.size() != 0;
}



void ai_enma_module_linker::initialize_export_redirect_table(std::vector<export_references>& export_refs) {


	pe_section *table_section = main_module->get_image().get_section_by_idx(
        main_module->get_image().get_sections_number() - 1
    );//get last section

	DWORD redirect_table_rva = table_section->get_virtual_address() + table_section->get_virtual_size();

    for (unsigned int i = 0; i < export_refs.size(); i++) { //initialize redirect table

        if (main_module->get_image().is_x32_image()) {
            export_refs[i].wrapper_rva = redirect_table_rva + sizeof(DWORD) * i;

            DWORD reloc = (DWORD)main_module->get_image().rva_to_va(export_refs[i].export_item.get_rva());

            if (export_refs[i].module_export_owner != -1) {
                reloc += extended_modules[export_refs[i].module_export_owner]->get_address_offset();
            }

            main_module->get_image().set_data_by_rva(table_section, export_refs[i].wrapper_rva, &reloc, sizeof(reloc));
        }
        else {
            export_refs[i].wrapper_rva = redirect_table_rva + sizeof(DWORD64) * i;

            DWORD64 reloc = main_module->get_image().rva_to_va(export_refs[i].export_item.get_rva());

            if (export_refs[i].module_export_owner != -1) {
                reloc += extended_modules[export_refs[i].module_export_owner]->get_address_offset();
            }

            main_module->get_image().set_data_by_rva(table_section, export_refs[i].wrapper_rva, &reloc, sizeof(reloc));
        }

        main_module->get_image_relocations().add_item(export_refs[i].wrapper_rva, e_relocation_default);
    }
}


bool ai_enma_module_linker::merge_export() {
    

	std::vector<export_references> export_refs;
    if (!get_export_references(export_refs)) { return true; }

    for (auto& plugin : plugins) { plugin->on_before_merge_export(this); }

	initialize_export_redirect_table(export_refs);       //create table in last section for export redirect

	
    for (auto& ref : export_refs) {                      //fix relocations for redirected import 
        unsigned int ref_lib_idx;
        unsigned int ref_func_idx;
       
        if (ref.export_item.has_name()){
            if (!get_import_func_index(main_module->get_image_imports(),
                ref.lib_name, ref.export_item.get_name(),
                ref_lib_idx, ref_func_idx)) {
                
                return false; // =\ why?
            }
        }
        else {
            if (!get_import_func_index(main_module->get_image_imports(),
                ref.lib_name, ref.export_item.get_ordinal(),
                ref_lib_idx, ref_func_idx)) {

                return false; // =\ why?
            }
        }

        std::vector<relocation_item*> found_relocs;
        main_module->get_image_relocations().get_items_by_relocation_id(found_relocs, SET_RELOCATION_ID_IAT(ref_lib_idx, ref_func_idx));

        for (auto & found_item : found_relocs) {

            if (main_module->get_image().is_x32_image()) {
                DWORD new_rel = (DWORD)main_module->get_image_expanded().image.rva_to_va(ref.wrapper_rva);
                main_module->get_image_expanded().image.set_data_by_rva(found_item->relative_virtual_address, &new_rel, sizeof(new_rel));
            }
            else {
                DWORD64 new_rel = main_module->get_image_expanded().image.rva_to_va(ref.wrapper_rva);
                main_module->get_image_expanded().image.set_data_by_rva(found_item->relative_virtual_address, &new_rel, sizeof(new_rel));
            }

            found_item->relocation_id = e_relocation_default;
        }
    }


    import_table new_import_table = main_module->get_image_imports();
    for (auto& ref : export_refs) {                             //erase items from import by export lib name and export item 

        for (unsigned int import_lib_idx = 0, original_import_lib_idx = 0; 
            import_lib_idx < new_import_table.get_libs().size();
            import_lib_idx++, original_import_lib_idx++) {

            auto & import_lib = new_import_table.get_libs()[import_lib_idx];

            if (nocase_cmp(ref.lib_name , import_lib.get_name())) {

                for (unsigned int import_func_idx = 0, original_import_func_idx = 0;
                    import_func_idx < import_lib.get_items().size();
                    import_func_idx++, original_import_func_idx++) {

                    auto & import_func = import_lib.get_items()[import_func_idx];

                    if (ref.export_item.has_name()) {
                        if (import_func.is_import_by_name() && import_func.get_name() == ref.export_item.get_name()) {
                            import_lib.get_items().erase(import_lib.get_items().begin() + import_func_idx);
                            import_func_idx--;
                        }
                        else {
                            continue;
                        }
                    }
                    else {
                        if (!import_func.is_import_by_name() && import_func.get_ordinal() == ref.export_item.get_ordinal()) {
                            import_lib.get_items().erase(import_lib.get_items().begin() + import_func_idx);
                            import_func_idx--;
                        }
                        else {
                            continue;
                        }
                    }
                }

                if (!import_lib.get_items().size()) {                    //delete if lib empty
                    new_import_table.get_libs().erase(new_import_table.get_libs().begin() + import_lib_idx);
                    import_lib_idx--;
                }
            }
        }
    }

    switch_import_refs(main_module->get_image_expanded(), new_import_table);

    for (auto& plugin : plugins) { plugin->on_after_merge_export(this); }
	return true;
}