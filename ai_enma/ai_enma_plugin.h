#pragma once
class ai_enma_plugin{

public:
    virtual ai_enma_plugin::~ai_enma_plugin() {};

    virtual void ai_enma_plugin::on_before_explore_module(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_explore_module(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_sections_merge(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_sections_merge(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_merge_relocations(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_merge_relocations(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_process_import(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_process_import(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_merge_export(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_merge_export(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_merge_tls(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_merge_tls(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_merge_loadconfig(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_merge_loadconfig(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_merge_exceptions(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_merge_exceptions(ai_enma_module_linker * linker) {};

    virtual void ai_enma_plugin::on_before_linking(ai_enma_module_linker * linker) {};
    virtual void ai_enma_plugin::on_after_linking(ai_enma_module_linker * linker) {};
};

