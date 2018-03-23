#pragma once

#define SET_HI_NUMBER(x,num) ((x&0xF000FFFF)|((num&0xFFF)<<16))
#define SET_LO_NUMBER(x,num) ((x&0xFFFF0000)| (num&0xFFFF))

#define GET_HI_NUMBER(x) ((x&0x0FFF0000)>>16)
#define GET_LO_NUMBER(x) ((x&0x0000FFFF))

#define SET_RELOCATION_ID_IAT(lib_num,func_num) (((lib_num&0xFFF)<<16)|(func_num&0xFFFF)|e_relocation_iat_address)

enum e_relocation_index_description {
    e_relocation_default         = 1,
    e_relocation_tls_index       = 2,
    e_relocation_security_cookie = 3,

    e_relocation_iat_address = 0xF0000000,//((reloc_id&0x0FFF0000)>>16) module id (reloc_id&0x0000FFFF) function id
};

enum e_enma_module_entry_point {
    e_enma_module_entry_point_exe = 1,
    e_enma_module_entry_point_dll = 2
};

enum enma_module_code {
    enma_module_not_initiated,
    enma_module_explored,
};


struct export_extended {
    std::vector<std::string> extended_names;
    std::vector<export_table_item> export_items;
    DWORD address_offset;
};

struct enma_module_entry_point {
    e_enma_module_entry_point type;
    DWORD entry_point_rva;
};

class ai_enma_module{
    enma_module_code module_code;                  //initiation stage
    pe_image_expanded image_expanded;
    std::vector<map_dir> map_description;          //where first section is started in main module
    std::vector<std::string> export_extended_names;//ext names of image
    
    //explored info
    DWORD rva_of_first_section;                     //where first section is started in main module 
    DWORD address_offset;                           //relation between position of the first section in original module and main module 
    std::vector<export_extended> extended_exports;            //ext exports of image
    std::vector<enma_module_entry_point> module_entry_points; //ext ep of image
public:
    ai_enma_module::ai_enma_module(pe_image &image);
    ai_enma_module::ai_enma_module(pe_image_expanded &expanded_image);
    
    ai_enma_module::~ai_enma_module();

    ai_enma_module& ai_enma_module::operator=(const ai_enma_module& enma_module);
public:
    ai_enma_module& ai_enma_module::add_ext_name(std::string name);

    void ai_enma_module::set_map_description(map_root& map);
    void ai_enma_module::set_rva_of_first_section(DWORD rva);
    void ai_enma_module::set_address_offset(DWORD rva);
    void ai_enma_module::set_module_code(enma_module_code code);
public:
    pe_image_expanded&      ai_enma_module::get_image_expanded();
    pe_image&               ai_enma_module::get_image();
    export_table&		    ai_enma_module::get_image_exports();
    import_table&		    ai_enma_module::get_image_imports();
    resource_directory&	    ai_enma_module::get_image_resources();
    exceptions_table&	    ai_enma_module::get_image_exceptions();
    relocation_table&	    ai_enma_module::get_image_relocations();
    debug_table&	        ai_enma_module::get_image_debug();
    tls_table&			    ai_enma_module::get_image_tls();
    load_config_table&	    ai_enma_module::get_image_load_config();
    delay_import_table&     ai_enma_module::get_image_delay_imports();
    bound_import_table&     ai_enma_module::get_image_bound_imports();

    std::vector<map_dir>&         ai_enma_module::get_map_description();
    std::vector<std::string>&     ai_enma_module::get_extended_names();
    std::vector<export_extended>& ai_enma_module::get_extended_exports();
    std::vector<enma_module_entry_point>&  ai_enma_module::get_module_entry_points();

    DWORD ai_enma_module::get_rva_of_first_section() const;
    DWORD ai_enma_module::get_address_offset() const;
    enma_module_code ai_enma_module::get_module_code() const;
};

