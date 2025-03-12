//
// Created by estartce on 11.03.2025
//
#include "ELFParser.h"
#include <fstream>
#include <cstring>
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
            mBuffer.resize(file_size);
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
            for (size_t i = 0; i < mHeader.e_shnum; i++)
            {
                std::unique_ptr<ELFSection64> section = std::make_unique<ELFSection64>();
                std::memcpy(section.get(), &(sections[i]), sizeof(ELFSection64));
                sect_vector.push_back(std::move(section));
            }
            auto &shstrtab = sect_vector[mHeader.e_shstrndx]; // Section header table
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
        auto &strtab = mSections[".strtab"];
        size_t num_symbols = symtab->sh_size / symtab->sh_entsize;
        // Raw data pointer
        ELFSymbol64 *symbols = reinterpret_cast<ELFSymbol64 *>(mBuffer.data() + symtab->sh_offset);
        for (size_t i = 0; i < num_symbols; i++)
        {
            std::unique_ptr<ELFSymbol64> symbol = std::make_unique<ELFSymbol64>();
            std::memcpy(symbol.get(), &(symbols[i]), sizeof(ELFSymbol64));
            auto symbol_name = reinterpret_cast<char *>(mBuffer.data() + strtab->sh_offset + symbol->st_name);
            mSymbols[std::move(symbol_name)] = std::move(symbol);
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
