//
// Created by estartce on 11.03.2025
//
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <functional>
#include <memory>

#include <unordered_map>

namespace MyELF
{

    /*
     * ELF definitions common to all 64-bit architectures.
     */

    typedef uint64_t Elf64_Addr;
    typedef uint16_t Elf64_Half;
    typedef uint64_t Elf64_Off;
    typedef int32_t Elf64_Sword;
    typedef int64_t Elf64_Sxword;
    typedef uint32_t Elf64_Word;
    typedef uint64_t Elf64_Lword;
    typedef uint64_t Elf64_Xword;

    struct ELFIdent
    {
        uint8_t e_mag[4];
        uint8_t e_class;
        uint8_t e_data;
        uint8_t e_iversion;
        uint8_t padding[9];
    };

    struct ELFHeader64
    {
        ELFIdent e_ident;
        Elf64_Half e_type;      /* File type. */
        Elf64_Half e_machine;   /* Machine architecture. */
        Elf64_Word e_version;   /* ELF format version. */
        Elf64_Addr e_entry;     /* Entry point. */
        Elf64_Off e_phoff;      /* Program header file offset. */
        Elf64_Off e_shoff;      /* Section header file offset. */
        Elf64_Word e_flags;     /* Architecture-specific flags. */
        Elf64_Half e_ehsize;    /* Size of ELF header in bytes. */
        Elf64_Half e_phentsize; /* Size of program header entry. */
        Elf64_Half e_phnum;     /* Number of program header entries. */
        Elf64_Half e_shentsize; /* Size of section header entry. */
        Elf64_Half e_shnum;     /* Number of section header entries. */
        Elf64_Half e_shstrndx;  /* Section name strings section. */
    };

    struct ELFSection64
    {
        uint32_t sh_name;
        uint32_t sh_type;
        uint64_t sh_flags;
        Elf64_Addr sh_addr;
        Elf64_Off sh_offset;
        uint64_t sh_size;
        uint32_t sh_link;
        uint32_t sh_info;
        uint64_t sh_addralign;
        uint64_t sh_entsize;
    };

    using ELFHeaderHandler = std::function<void(const ELFHeader64 &)>;
    using ELFSectionHandler = std::function<void(const ELFSection64 &, const std::string &)>;

    struct ELFReadStatus
    {
        bool loaded;
        bool header_read;
        bool section_table_read;
    };

    struct ELFSectionHandlers
    {
        ELFHeaderHandler header_handler;
        ELFSectionHandler section_handler;
    };

    class ELFParser
    {
    private:
        ELFReadStatus mStatus;
        std::string mFilename;
        std::vector<uint8_t> mBuffer;
        ELFSectionHandlers mHandlers;
        ELFHeader64 mHeader;
        std::unordered_map<std::string, std::unique_ptr<ELFSection64>> mSections;

    public:
        ELFParser() = delete;
        ELFParser(const std::string &filename) : mStatus{}, mFilename(filename) {};
        ~ELFParser() {};
        void load();
        void readData();
        void readHeader();
        void readSectionTable();
        void setHeaderHandler(ELFHeaderHandler h) { mHandlers.header_handler = h; };
        void setSectionHandler(ELFSectionHandler h) { mHandlers.section_handler = h; };
    };
} // namespace MyELF