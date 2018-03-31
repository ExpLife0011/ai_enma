#include "stdafx.h"
#include "ai_enma_module_linker.h"

inline bool nocase_cmp(const std::string& str1, const std::string& str2);
inline bool get_import_func_index(import_table& imports,
    std::string lib_name, std::string funcname,
    unsigned int & lib_idx, unsigned int & func_idx);
inline bool get_import_func_index(import_table& imports,
    std::string lib_name, WORD func_ordinal,
    unsigned int & lib_idx, unsigned int & func_idx);


#include "linker_merge_module.h"
#include "linker_merge_exceptions.h"
#include "linker_merge_relocations.h"
#include "linker_merge_import.h"
#include "linker_merge_export.h"
#include "linker_merge_tls.h"
#include "linker_merge_loadconfig.h"




ai_enma_module_linker::ai_enma_module_linker(){}
ai_enma_module_linker::~ai_enma_module_linker(){}


bool ai_enma_module_linker::explore_module(ai_enma_module * module) {
    if (module->get_module_code() == enma_module_code::enma_module_explored) { return true; }

    for (auto& plugin : plugins) { plugin->on_before_explore_module(this); }

    std::vector<erased_zone> zones;
    
    erase_directories_pe_image(module->get_image_expanded().image, zones, &module->get_image_relocations(), true);

    
    if (module->get_image_delay_imports().get_libs().size()) {//merge delay import
        for (auto &item : module->get_image_delay_imports().get_libs()) {
            module->get_image_imports().add_lib(item.convert_to_imported_library());         
        }
    }

	if (!process_relocations(module->get_image_expanded()))  { return false; }
    if (!process_import(module->get_image_expanded()))       { return false; }


    for (auto& section_ : module->get_image_expanded().image.get_sections()) {
        if (section_->get_virtual_size() < section_->get_size_of_raw_data()) {
            section_->set_virtual_size(section_->get_size_of_raw_data());
        }
    }

    module->set_module_code(enma_module_explored);

    for (auto& plugin : plugins) { plugin->on_after_explore_module(this); }
	return true;
}


enma_ltable_errors ai_enma_module_linker::link_modules(
    std::vector<ai_enma_plugin*> plugins,
    std::vector<ai_enma_module*> extended_modules,
    ai_enma_module* main_module) {

    this->plugins = plugins;
    this->extended_modules = extended_modules;
    this->main_module = main_module;

    if (!main_module) { return enma_linker_error_bad_input; } else { explore_module(main_module); }
    for (auto  &module : extended_modules) { 
        if (main_module->get_image().is_x32_image() != module->get_image().is_x32_image()) {
            return enma_linker_error_bad_input;
        }
        explore_module(module);
    }

    for (auto& plugin : plugins) { plugin->on_before_linking(this); }

    merge_sections(); 

    if (!merge_import()) {
        return enma_linker_error_import_linking;
    }
    if (!merge_relocations()) {
        return enma_linker_error_relocation_linking;
    }
	if (!merge_export()) {
		return enma_linker_error_export_linking;
	}
    if (!merge_tls()) {
        return enma_linker_error_tls_linking;
    }
    if (!merge_loadconfig()) {
        return enma_linker_error_loadconfig_linking;
    }
    if (!merge_exceptions()) {
        return enma_linker_error_exceptions_linking;
    }
   
    merge_module_data();

    for (auto& plugin : plugins) { plugin->on_after_linking(this); }
	return enma_ok;
}


std::vector<ai_enma_plugin*>& ai_enma_module_linker::get_plugins() {
    return plugins;
}
std::vector<ai_enma_module*>& ai_enma_module_linker::get_extended_modules() {
    return extended_modules;
}
ai_enma_module* ai_enma_module_linker::get_main_module() {
    return main_module;
}


inline bool nocase_cmp(const std::string& str1, const std::string& str2) {
    unsigned int sz = str1.size();
    if (str2.size() != sz) {
        return false;
    }
    for (unsigned int i = 0; i < sz; ++i) {
        if (tolower(str1[i]) != tolower(str2[i])) {
            return false;
        }
    }
    return true;
}


inline bool get_import_func_index(import_table& imports,
    std::string lib_name, std::string funcname,
    unsigned int & lib_idx, unsigned int & func_idx) {


    for (unsigned int current_lib_idx = 0; current_lib_idx < imports.get_libs().size(); current_lib_idx++) {
        imported_library & current_library = imports.get_libs()[current_lib_idx];

        if (nocase_cmp(current_library.get_library_name(), lib_name)) {

            for (unsigned int current_func_idx = 0; current_func_idx < current_library.get_items().size(); current_func_idx++) {
                imported_func& current_func = current_library.get_items()[current_func_idx];

                if (current_func.is_import_by_name() && current_func.get_func_name() == funcname) {

                    lib_idx = current_lib_idx;
                    func_idx = current_func_idx;
                    return true;
                }
            }
        }

    }

    return false;
}


inline bool get_import_func_index(import_table& imports,
    std::string lib_name, WORD func_ordinal,
    unsigned int & lib_idx, unsigned int & func_idx) {

    for (unsigned int current_lib_idx = 0; current_lib_idx < imports.get_libs().size(); current_lib_idx++) {
        imported_library & current_library = imports.get_libs()[current_lib_idx];

        if (nocase_cmp(current_library.get_library_name(), lib_name)) {

            for (unsigned int current_func_idx = 0; current_func_idx < current_library.get_items().size(); current_func_idx++) {
                imported_func& current_func = current_library.get_items()[current_func_idx];

                if (!current_func.is_import_by_name() && current_func.get_ordinal() == func_ordinal) {

                    lib_idx = current_lib_idx;
                    func_idx = current_func_idx;
                    return true;
                }
            }
        }

    }

    return false;
}
