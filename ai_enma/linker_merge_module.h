#pragma once


void ai_enma_module_linker::merge_sections() {
    for (auto& plugin : plugins) { plugin->on_before_sections_merge(this); }

    for (auto& module_ : extended_modules) {

        module_->set_rva_of_first_section(ALIGN_UP(main_module->get_image().get_sections()[
            main_module->get_image().get_sections_number() - 1
        ]->get_virtual_address() + main_module->get_image().get_sections()[
            main_module->get_image().get_sections_number() - 1]->get_virtual_size(), 0x1000));


        module_->set_address_offset(
            module_->get_rva_of_first_section() - module_->get_image().get_sections()[0]->get_virtual_address()
        );

        DWORD section_top_rva = module_->get_rva_of_first_section();
        DWORD section_top_raw = ALIGN_UP(main_module->get_image().get_sections()[
            main_module->get_image().get_sections_number() - 1
        ]->get_pointer_to_raw() + main_module->get_image().get_sections()[
            main_module->get_image().get_sections_number() - 1]->get_size_of_raw_data(), 0x200);

        for (auto& section_ : module_->get_image().get_sections()) { //add sections
            pe_section pre_section = *section_;
            pre_section.set_virtual_address(section_top_rva);
            pre_section.set_pointer_to_raw(section_top_raw);
            main_module->get_image().add_section(pre_section);

            section_top_raw += ALIGN_UP(pre_section.get_pointer_to_raw() + pre_section.get_size_of_raw_data(), 0x200);
            section_top_rva += ALIGN_UP(pre_section.get_virtual_size(), 0x1000);
        }
    }
    for (auto& plugin : plugins) { plugin->on_after_sections_merge(this); }
}



void ai_enma_module_linker::merge_module_data() {

    for (auto& module_ : extended_modules) {
        if (module_->get_image_exports().get_items().size()) {
            main_module->get_extended_exports().push_back({
                module_->get_extended_names(),
                module_->get_image_exports().get_items(),
                module_->get_address_offset()
            });
        }

        if (module_->get_image_expanded().image.get_characteristics()&IMAGE_FILE_DLL) {
            main_module->get_module_entry_points().push_back({ e_enma_module_entry_point_dll,
                module_->get_address_offset() + module_->get_image().get_entry_point()
            });

            //fix it
            main_module->get_image_tls().set_address_of_index(1);
            main_module->get_image_tls().get_callbacks().push_back({ module_->get_image().get_entry_point() + module_->get_address_offset(), true });
        }
        else {
            main_module->get_module_entry_points().push_back({ e_enma_module_entry_point_exe,
                module_->get_address_offset() + module_->get_image().get_entry_point()
            });
        }
    }
}