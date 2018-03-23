#pragma once
class ai_enma_module_builder {
    DWORD ai_enma_module_builder::align_sections(pe_image_expanded& expanded_image);

    void ai_enma_module_builder::get_nt_header32(pe_image_expanded& expanded_image,IMAGE_NT_HEADERS32 &header);
    void ai_enma_module_builder::get_nt_header64(pe_image_expanded& expanded_image,IMAGE_NT_HEADERS64 &header);

    void ai_enma_module_builder::build_directories(pe_image_expanded& expanded_image, bool build_relocations);

    void ai_enma_module_builder::process_relocations(pe_image_expanded& expanded_image);
public:
    ai_enma_module_builder::ai_enma_module_builder(ai_enma_module& module,bool build_relocations, std::vector<BYTE>& out_image);
    ai_enma_module_builder::~ai_enma_module_builder();
};

