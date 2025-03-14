//
// Created by estartce on 11.03.2025
//
#include "ELFParser.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>

using namespace MyELF;

void ELFParser::load()
{
    if (!mStatus.loaded)
    {
        std::ifstream file(mFilename, std::ios::binary | std::ios::ate); // Open file at the end
        if (!file.is_open())
        {
            std::cout << "Can't open file: " << mFilename << std::endl;
            return;
        }
        std::streamsize file_size = file.tellg(); // Get file size
        file.seekg(0, std::ios::beg);             // Move back to the beginning

        if (file_size > 0)
        {
            mBuffer.resize(static_cast<size_t>(file_size));
            file.read(reinterpret_cast<char *>(mBuffer.data()), file_size);
        }
        mStatus.loaded = true;
    }
}

void ELFParser::readData()
{
    if (mStatus.loaded && !mStatus.header_read)
    {
        readHeader();
        readProgramHeader();
        readSectionTable();
        readSymbolTable();
    }
}

void ELFParser::readHeader()
{

    if (mBuffer.size() < sizeof(ELFHeader64))
    {
        std::stringstream ss;
        ss << "Only " << mBuffer.size() << " bytes in buffer!";
        throw std::runtime_error(ss.str());
    }
    std::memcpy(&mHeader, mBuffer.data(), sizeof(ELFHeader64));
    if (mHandlers.header_handler)
    {
        mHandlers.header_handler(mHeader);
    }
    mStatus.header_read = true;
}

void ELFParser::readSectionTable()
{
    if (mStatus.header_read && !mStatus.section_table_read)
    {
        if (mHeader.e_shoff > 0)
        {
            // Read section table
            // Read all sections first to get access to section header table
            std::vector<std::unique_ptr<ELFSection64>> sect_vector; // tmp vector to store read section
            sect_vector.reserve(mHeader.e_shnum);
            // Raw data pointer
            ELFSection64 *sections = reinterpret_cast<ELFSection64 *>(mBuffer.data() + mHeader.e_shoff);
            // Read all section first to access section header table
            auto sectGen = genReadStruct(sections, mHeader.e_shnum);
            while (auto sectopt = sectGen())
            {
                auto &section = *sectopt;
                sect_vector.push_back(std::move(section));
            }
            auto &shstrtab = sect_vector[mHeader.e_shstrndx]; // Section header table
            // Read all section names and store them in a map
            for (size_t i = 0; i < mHeader.e_shnum; i++)
            {
                auto &section = sect_vector[i];
                // Sections names are null-terminated
                auto section_name = reinterpret_cast<char *>(mBuffer.data() + shstrtab->sh_offset + section->sh_name);
                mSections[std::move(section_name)] = std::move(section);
            }
            mStatus.section_table_read = true;
        }
    }
    if (mStatus.section_table_read)
    {
        if (mHandlers.section_handler)
        {
            for (const auto &[name, section] : mSections)
            {
                mHandlers.section_handler(*section, name);
            }
        }
    }
}

void MyELF::ELFParser::readSymbolTable()
{
    if (mStatus.header_read && mStatus.section_table_read)
    {
        if (!mSections.contains(".symtab"))
        {
            throw std::runtime_error("Cannot find symbol table!");
        }
        if (!mSections.contains(".strtab"))
        {
            throw std::runtime_error("Cannot find string table!");
        }
        auto &symtab = mSections[".symtab"];
        size_t num_symbols = symtab->sh_size / symtab->sh_entsize;
        // Raw data pointer
        const ELFSymbol64 *symbols = reinterpret_cast<const ELFSymbol64 *>(getSectionPtr(".symtab"));
        auto symGen = genReadStruct(symbols, num_symbols);
        while (auto symopt = symGen())
        {
            auto &symbol = *symopt;
            auto symbol_name = reinterpret_cast<const char *>(getSectionPtr(".strtab") + symbol->st_name);
            mSymbols[std::string(symbol_name)] = std::move(symbol);
        }
        mStatus.symbol_table_read = true;
    }
    if (mStatus.symbol_table_read)
    {
        if (mHandlers.symbol_handler)
        {
            for (const auto &[name, symbol] : mSymbols)
            {
                mHandlers.symbol_handler(*symbol, name);
            }
        }
    }
}

void MyELF::ELFParser::readProgramHeader()
{
    if (mStatus.header_read && !mStatus.program_header_read)
    {
        if (mHeader.e_phoff > 0)
        {
            // Read program header
            // Raw data pointer
            mProgramHeaders.reserve(mHeader.e_phnum);
            ELFProgramHeader64 *program_headers = reinterpret_cast<ELFProgramHeader64 *>(mBuffer.data() + mHeader.e_phoff);
            auto phGen = genReadStruct(program_headers, mHeader.e_phnum);
            while (auto phopt = phGen())
            {
                auto &program_header = *phopt;
                mProgramHeaders.push_back(std::move(program_header));
            }
            mStatus.program_header_read = true;
        }
    }
    if (mStatus.program_header_read)
    {
        if (mHandlers.program_header_handler)
        {
            for (const auto &program_header : mProgramHeaders)
            {
                mHandlers.program_header_handler(*program_header);
            }
        }
    }
}
