#pragma once


bool ai_enma_module_linker::merge_tls() {
    for (auto& plugin : plugins) { plugin->on_before_merge_tls(this); }

    for (auto& module : extended_modules) {

        if (module->get_image_tls().get_callbacks().size()) {

            for (auto& item : module->get_image_tls().get_callbacks()) {
                main_module->get_image_tls().get_callbacks().push_back({ module->get_address_offset() + item.rva_callback ,item.use_relocation });
            }
        }
    }

    for (auto& plugin : plugins) { plugin->on_after_merge_tls(this); }
    return true;
}