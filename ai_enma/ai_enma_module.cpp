#include "stdafx.h"
#include "ai_enma_module.h"


ai_enma_module::ai_enma_module(pe_image &image) {
    do_expanded_pe_image(this->image_expanded, image);
    this->module_code = enma_module_not_initiated;
    this->rva_of_first_section  = 0;
    this->address_offset        = 0;
}
ai_enma_module::ai_enma_module(pe_image_expanded &expanded_image) {
    this->image_expanded.exports = expanded_image.exports;
    this->image_expanded.imports = expanded_image.imports;
    this->image_expanded.resources  = expanded_image.resources;
    this->image_expanded.exceptions = expanded_image.exceptions;
    this->image_expanded.relocations = expanded_image.relocations;
    this->image_expanded.debug       = expanded_image.debug;
    this->image_expanded.tls         = expanded_image.tls;
    this->image_expanded.load_config = expanded_image.load_config;
    this->image_expanded.delay_imports = expanded_image.delay_imports;
    this->image_expanded.bound_imports = expanded_image.bound_imports;

    this->module_code = enma_module_not_initiated;
    this->rva_of_first_section  = 0;
    this->address_offset        = 0;
}

ai_enma_module::~ai_enma_module() {

}

ai_enma_module& ai_enma_module::operator=(const ai_enma_module& enma_module) {
    this->module_code       = enma_module.module_code;
    this->map_description   = enma_module.map_description;
    this->export_extended_names = enma_module.export_extended_names;
    this->extended_exports  = enma_module.extended_exports;
    this->module_entry_points = enma_module.module_entry_points;
    this->rva_of_first_section  = enma_module.rva_of_first_section;
    this->address_offset        = enma_module.address_offset;
    this->image_expanded;
    this->image_expanded.exports = enma_module.image_expanded.exports;
    this->image_expanded.imports = enma_module.image_expanded.imports;
    this->image_expanded.resources  = enma_module.image_expanded.resources;
    this->image_expanded.exceptions = enma_module.image_expanded.exceptions;
    this->image_expanded.relocations   = enma_module.image_expanded.relocations;
    this->image_expanded.debug         = enma_module.image_expanded.debug;
    this->image_expanded.tls           = enma_module.image_expanded.tls;
    this->image_expanded.load_config   = enma_module.image_expanded.load_config;
    this->image_expanded.delay_imports = enma_module.image_expanded.delay_imports;
    this->image_expanded.bound_imports = enma_module.image_expanded.bound_imports;

    return *this;
}

ai_enma_module& ai_enma_module::add_ext_name(std::string name) {
    this->export_extended_names.push_back(name);
    return *this;
}
void ai_enma_module::set_map_description(map_root& map) {
    this->map_description = map.dirs;
}
void ai_enma_module::set_rva_of_first_section(DWORD rva) {
    this->rva_of_first_section = rva;
}
void ai_enma_module::set_address_offset(DWORD rva) {
    this->address_offset = rva;
}
void ai_enma_module::set_module_code(enma_module_code code) {
    this->module_code = code;
}

pe_image_expanded&      ai_enma_module::get_image_expanded() {
    return this->image_expanded;
}
pe_image&               ai_enma_module::get_image() {
    return this->image_expanded.image;
}
export_table&		    ai_enma_module::get_image_exports() {
    return this->image_expanded.exports;
}
import_table&		    ai_enma_module::get_image_imports() {
    return this->image_expanded.imports;
}
resource_directory&	    ai_enma_module::get_image_resources() {
    return this->image_expanded.resources;
}
exceptions_table&	    ai_enma_module::get_image_exceptions() {
    return this->image_expanded.exceptions;
}
relocation_table&	    ai_enma_module::get_image_relocations() {
    return this->image_expanded.relocations;
}
debug_table&	        ai_enma_module::get_image_debug() {
    return this->image_expanded.debug;
}
tls_table&			    ai_enma_module::get_image_tls() {
    return this->image_expanded.tls;
}
load_config_table&	    ai_enma_module::get_image_load_config() {
    return this->image_expanded.load_config;
}
delay_import_table&     ai_enma_module::get_image_delay_imports() {
    return this->image_expanded.delay_imports;
}
bound_import_table&     ai_enma_module::get_image_bound_imports() {
    return this->image_expanded.bound_imports;
}
std::vector<map_dir>&     ai_enma_module::get_map_description() {
    return this->map_description;
}
std::vector<std::string>& ai_enma_module::get_extended_names() {
    return this->export_extended_names;
}
std::vector<export_extended>& ai_enma_module::get_extended_exports() {
    return this->extended_exports;
}
std::vector<enma_module_entry_point>&  ai_enma_module::get_module_entry_points() {
    return this->module_entry_points;
}
DWORD ai_enma_module::get_rva_of_first_section() const {
    return this->rva_of_first_section;
}
DWORD ai_enma_module::get_address_offset() const {
    return this->address_offset;
}
enma_module_code ai_enma_module::get_module_code() const {
    return this->module_code;
}
