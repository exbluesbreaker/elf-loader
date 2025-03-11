#include <iostream>
#include "ELFParser.h"

using namespace MyELF;

inline std::string elf_class(uint8_t e_data)
{
    switch (e_data)
    {
    case 0:
        return "Invalid data enconding";
    case 1:
        return "LSB";
    case 2:
        return "MSB";
    default:
        return "Unexpected";
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <elf file>" << std::endl;
        return 1;
    }
    ELFParser parser{argv[1]};
    parser.load();
    ELFHeaderHandler header_printer = [](const ELFHeader64 &header)
    {
        std::string indent = "\t";
        std::cout << "Elf header: " << std::endl;
        std::cout << indent << "Elf identification: " << std::endl;
        indent += "\t";
        auto &e_data = header.e_ident.e_data;
        std::cout << indent << "Magic: 0x" << std::hex << static_cast<uint16_t>(header.e_ident.e_mag[0]) << " \""
                  << header.e_ident.e_mag[1] << header.e_ident.e_mag[2] << header.e_ident.e_mag[3] << "\"" << std::endl;
        std::cout << indent << "Class: 0x" << std::hex << static_cast<uint16_t>(header.e_ident.e_class) << std::endl;
        std::cout << indent << "Data: 0x" << std::hex << static_cast<uint16_t>(e_data) << " ("
                  << elf_class(e_data) << ")" << std::endl;
        std::cout << indent << "Version: 0x" << std::hex << static_cast<uint16_t>(header.e_ident.e_iversion) << std::endl;
        indent = "\t";
        std::cout << indent << "Type: 0x" << std::hex << header.e_type << std::endl;
        std::cout << indent << "Machine: 0x" << std::hex << header.e_machine << std::endl;
        std::cout << indent << "Entry: 0x" << std::hex << header.e_entry << std::endl;
        std::cout << indent << "Phoff: 0x" << std::hex << header.e_phoff << std::endl;
        std::cout << indent << "Shoff: 0x" << std::hex << header.e_shoff << std::endl;
        std::cout << indent << "Flags: 0x" << std::hex << header.e_flags << std::endl;
        std::cout << indent << "Ehsize: " << std::dec << header.e_ehsize << std::endl;
        std::cout << indent << "Phentsize: " << std::dec << header.e_phentsize << std::endl;
        std::cout << indent << "Phnum: " << std::dec << header.e_phnum << std::endl;
        std::cout << indent << "Shentsize: " << std::dec << header.e_shentsize << std::endl;
        std::cout << indent << "Shnum: " << std::dec << header.e_shnum << std::endl;
        std::cout << indent << "Shstrndx: " << std::dec << header.e_shstrndx << std::endl;
    };
    ELFSectionHandler section_printer = [count = 0](const ELFSection64 &section, const std::string &name) mutable
    {
        std::string indent = "\t";
        std::cout << "Section # " << ++count << std::endl;
        std::cout << indent << "Name: " << name << std::endl;
        std::cout << indent << "Type: 0x" << std::hex << section.sh_type << std::endl;
        std::cout << indent << "Flags: 0x" << std::hex << section.sh_flags << std::endl;
        std::cout << indent << "Addr: 0x" << std::hex << section.sh_addr << std::endl;
        std::cout << indent << "Offset: 0x" << std::hex << section.sh_offset << std::endl;
        std::cout << indent << "Size: 0x" << std::hex << section.sh_size << std::endl;
        std::cout << indent << "Link: 0x" << std::hex << section.sh_link << std::endl;
        std::cout << indent << "Info: 0x" << std::hex << section.sh_info << std::endl;
        std::cout << indent << "Addralign: 0x" << std::hex << section.sh_addralign << std::endl;
        std::cout << indent << "Entsize: 0x" << std::hex << section.sh_entsize << std::endl;
    };
    parser.setSectionHandler(section_printer);
    parser.setHeaderHandler(header_printer);
    parser.readData();
}