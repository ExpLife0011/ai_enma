#include "stdafx.h"
#include "ai_enma.h"


ai_enma::ai_enma() {}
ai_enma::~ai_enma(){}

enma_ltable_errors ai_enma::exec_enma(std::vector<BYTE>& out_image) {

    bool main_has_relocations = main_module->get_image_relocations().size();
    unsigned int linker_ret = ai_enma_module_linker().link_modules(plugins, extended_modules, main_module);

    if (linker_ret) {
        return (enma_ltable_errors)linker_ret;
    }


    ai_enma_module_builder(*main_module, main_has_relocations, out_image);

    return enma_ltable_errors::enma_ok;
}

void ai_enma::set_main_module(ai_enma_module* module) {
    if (module) {
        main_module = module;
    }
}

void ai_enma::add_extended_module(ai_enma_module* module) {
    if (module) {
        extended_modules.push_back(module);
    }
}

void ai_enma::add_plugin(ai_enma_plugin* plugin) {
    if (plugin) {
        plugins.push_back(plugin);
    }
}

std::vector<ai_enma_plugin*>& ai_enma::get_plugins() {
    return plugins;
}

std::vector<ai_enma_module*>& ai_enma::get_extended_modules() {
    return extended_modules;
}

ai_enma_module* ai_enma::get_main_module() {
    return main_module;
}