#pragma once


struct export_references {
    DWORD wrapper_rva;
    std::string lib_name;
    export_table_item export_item;
    unsigned int module_export_owner;//-1 = main module other in bound modules
};


class ai_enma_module_linker {
    std::vector<ai_enma_plugin*> plugins;
    std::vector<ai_enma_module*> extended_modules;
    ai_enma_module* main_module;

	bool ai_enma_module_linker::explore_module(ai_enma_module * module);
    bool ai_enma_module_linker::process_import(pe_image_expanded& expanded_image);
    bool ai_enma_module_linker::process_relocations(pe_image_expanded& expanded_image);

    bool ai_enma_module_linker::switch_import_refs(pe_image_expanded& expanded_image, import_table& new_import_table);

    bool ai_enma_module_linker::get_export_references(std::vector<export_references>& export_refs);
    void ai_enma_module_linker::initialize_export_redirect_table(std::vector<export_references>& export_refs);

    void ai_enma_module_linker::merge_sections();
	bool ai_enma_module_linker::merge_relocations();
	bool ai_enma_module_linker::merge_import();
	bool ai_enma_module_linker::merge_export();
    bool ai_enma_module_linker::merge_tls();
    bool ai_enma_module_linker::merge_loadconfig();
    bool ai_enma_module_linker::merge_exceptions();
    void ai_enma_module_linker::merge_module_data();
public:
	ai_enma_module_linker::ai_enma_module_linker();
	ai_enma_module_linker::~ai_enma_module_linker();


public:
    enma_ltable_errors ai_enma_module_linker::link_modules(
        std::vector<ai_enma_plugin*> plugins,
        std::vector<ai_enma_module*> extended_modules,
        ai_enma_module* main_module
    );

public:
    std::vector<ai_enma_plugin*>& ai_enma_module_linker::get_plugins();
    std::vector<ai_enma_module*>& ai_enma_module_linker::get_extended_modules();
    ai_enma_module* ai_enma_module_linker::get_main_module();

};

