/*
* Copyright 2014 Gunnar Lilleaasen
*
* This file is part of CascLib.
*
* CascLib is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* CascLib is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CascLib.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <bitset>
#include <stdint.h>
#include <limits>

namespace Casc
{
    namespace Shared
    {
        namespace DataTypes
        {
            using namespace Casc::Shared::Functions::Endian;

            /// Data structure that contains information about a
            /// block of memory in the data files.
            class MemoryInfo
            {
                /// The file number.
                /// Max value of this field is 2^10 (10 bit).
                size_t file_ = 0;

                /// The offset into the file. This is where the memory block starts.
                /// Max value of this field is 2^30 (30 bit).
                size_t offset_ = 0;

                /// The amount bytes in the memory block.
                /// Max value of this field is 2^30 (30 bit).
                size_t size_ = 0;

            public:
                enum BytesType
                {
                    Count,
                    Offset
                };

                /// Default constructor.
                MemoryInfo()
                {

                }

                /// Constructor.
                ///
                /// Allows for a shift parameter which shifts file to the left
                /// and moves bits from the end of offset into the start of file.
                ///
                /// @param file the file number.
                /// @param offset the offset into the file.
                /// @param amount the number of bytes.
                MemoryInfo(uint8_t file, uint32_t offset, uint32_t length, bool Shifted = true)
                    : size_(length)
                {
                    if (Shifted)
                    {
                        std::bitset<sizeof(uint8_t) * 8> fileBits(file);
                        std::bitset<sizeof(uint32_t) * 8> offsetBits(offset);

                        fileBits <<= 2;

                        fileBits[0] = offsetBits[30];
                        fileBits[1] = offsetBits[31];

                        offsetBits[30] = false;
                        offsetBits[31] = false;

                        this->file_ = fileBits.to_ulong();
                        this->offset_ = offsetBits.to_ulong();
                    }
                    else
                    {
                        this->file_ = file;
                        this->offset_ = offset;
                    }
                }

                /// Gets the file number containing the block.
                /// @return the file number.
                int file() const
                {
                    return file_;
                }

                /// Gets the offset where the writeable area start.
                /// @return the offset in the file given in bytes.
                size_t offset() const
                {
                    return offset_;
                }

                /// Gets the size of the block.
                /// @return the amount of bytes in the block.
                size_t size() const
                {
                    return size_;
                }

                /// Gets a byte representation for serializing.
                /// @return the byte representatio.
                template <BytesType Type>
                std::array<char, 5> bytes() const
                {
                    std::array<char, 5> bytes{};
                    std::array<char, 1> first{};
                    std::array<char, 4> second{};

                    switch (Type)
                    {
                    case BytesType::Count:
                        second = writeBE<uint32_t>(size_);
                        break;

                    case BytesType::Offset:
                        std::bitset<sizeof(uint32_t) * 8> offsetBits(offset_);
                        std::bitset<sizeof(uint8_t) * 8> fileBits(file_);

                        offsetBits[30] = fileBits[0];
                        offsetBits[31] = fileBits[1];

                        fileBits[0] = false;
                        fileBits[1] = false;

                        fileBits >>= 2;

                        second = writeBE(static_cast<uint32_t>(offsetBits.to_ulong()));
                        first = writeBE(static_cast<uint8_t>(fileBits.to_ulong()));

                        break;
                    }

                    std::copy(first.begin(), first.end(), bytes.begin());
                    std::copy(second.begin(), second.end(), bytes.begin() + 1);

                    return std::move(bytes);
                }
            };
        }
    }
}