#pragma once


bool ai_enma_module_linker::merge_exceptions() {
    if (main_module->get_image().is_x32_image()) { return true; }

    for (auto& plugin : plugins) { plugin->on_before_merge_exceptions(this); }

    for (auto& module : extended_modules) {

        for (auto& item : module->get_image_expanded().exceptions.get_items()) {

            main_module->get_image_exceptions().add_item(
                item.get_begin_address() + module->get_address_offset(),
                item.get_end_address() + module->get_address_offset(),
                item.get_unwind_data_address() + module->get_address_offset()
            );
        }
    }

    for (auto& plugin : plugins) { plugin->on_after_merge_exceptions(this); }
    return true;
}