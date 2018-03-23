#pragma once
#include "..\..\enma_pe\enma_pe\enma_pe.h" //https://github.com/jnastarot/enma_pe


class ai_enma_module;
class ai_enma_plugin;
class ai_enma_module_linker;
class ai_enma_module_builder;

enum enma_ltable_errors {
    enma_ok,
    enma_linker_error_bad_input,
    enma_linker_error_export_linking,
    enma_linker_error_import_linking,
    enma_linker_error_loadconfig_linking,
    enma_linker_error_relocation_linking,
    enma_linker_error_exceptions_linking,
    enma_linker_error_tls_linking,
};

#include "ai_enma_module.h"
#include "ai_enma_plugin.h"
#include "ai_enma_module_linker.h"
#include "ai_enma_module_builder.h"



class ai_enma {
    std::vector<ai_enma_plugin*> plugins;
    std::vector<ai_enma_module*> extended_modules;
    ai_enma_module* main_module;
public:
    ai_enma::ai_enma();
    ai_enma::~ai_enma();

    enma_ltable_errors ai_enma::exec_enma(std::vector<BYTE>& out_image);
public:
    void ai_enma::set_main_module(ai_enma_module* module);
    void ai_enma::add_extended_module(ai_enma_module* module);
    void ai_enma::add_plugin(ai_enma_plugin* plugin);
public:
    std::vector<ai_enma_plugin*>& ai_enma::get_plugins();
    std::vector<ai_enma_module*>& ai_enma::get_extended_modules();
    ai_enma_module* ai_enma::get_main_module();
};
