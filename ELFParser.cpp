//
// Created by estartce on 11.03.2025
//
#include "ELFParser.h"
#include <fstream>
#include <cstring>
#include <iostream>
#include <memory>

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
    }
}

void ELFParser::readHeader()
{

    if (mBuffer.size() < sizeof(ELFHeader64))
    {
        std::cout << "Only " << mBuffer.size() << " bytes in buffer!" << std::endl;
        return;
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
            mSections.resize(mHeader.e_shnum);
            std::memcpy(mSections.data(), mBuffer.data() + mHeader.e_shoff, mHeader.e_shnum * sizeof(ELFSection64));
            mStatus.section_table_read = true;
            // Read section names
            readSectionNames();
        }
    }
    if(mStatus.section_table_read)
    {
        for (size_t i = 0; i < mHeader.e_shnum; i++)
        {
            if (mHandlers.section_handler)
            {
                mHandlers.section_handler(mSections[i], mSectionNames[i]);
            }
        }
    }
}

void ELFParser::readSectionNames()
{
    if (mStatus.section_table_read && !mStatus.section_names_read)
    {
        // Read section names
        auto &shstrtab = mSections[mHeader.e_shstrndx];
        mSectionNames.resize(mHeader.e_shnum);
        for (size_t i = 0; i < mHeader.e_shnum; i++)
        {
            auto &section = mSections[i];
            mSectionNames[i] = std::string(reinterpret_cast<char *>(mBuffer.data() + shstrtab.sh_offset + section.sh_name));
        }
        mStatus.section_names_read = true;
    }
}
