#pragma once

bool ai_enma_module_linker::merge_loadconfig() {
    for (auto& plugin : plugins) { plugin->on_before_merge_loadconfig(this); }
    
    for (auto & module : extended_modules) {
        for (auto& se_handler : module->get_image_load_config().get_se_handlers()) {
            main_module->get_image_load_config().get_se_handlers().push_back(se_handler + module->get_address_offset());
        }
        for (auto& lock_prefix_rva : module->get_image_load_config().get_lock_prefixes()) {
            main_module->get_image_load_config().get_lock_prefixes().push_back(lock_prefix_rva + module->get_address_offset());
        }
        for (auto& guard_cf_func_rva : module->get_image_load_config().get_guard_cf_functions()) {
            main_module->get_image_load_config().get_guard_cf_functions().push_back(guard_cf_func_rva + module->get_address_offset());
        }
    }  

    for (auto& plugin : plugins) { plugin->on_after_merge_loadconfig(this); }
    return true;
}