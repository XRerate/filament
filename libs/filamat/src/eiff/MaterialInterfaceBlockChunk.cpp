/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "MaterialInterfaceBlockChunk.h"

#include "filament/MaterialChunkType.h"

#include "../SamplerBindingMap.h"
#include <private/filament/SamplerInterfaceBlock.h>
#include <private/filament/BufferInterfaceBlock.h>
#include <private/filament/SubpassInfo.h>
#include <private/filament/ConstantInfo.h>

#include <backend/DriverEnums.h>

#include <utility>

using namespace filament;

namespace filamat {

MaterialUniformInterfaceBlockChunk::MaterialUniformInterfaceBlockChunk(
        BufferInterfaceBlock const& uib) :
        Chunk(ChunkType::MaterialUib),
        mUib(uib) {
}

void MaterialUniformInterfaceBlockChunk::flatten(Flattener& f) {
    f.writeString(mUib.getName());
    auto uibFields = mUib.getFieldInfoList();
    f.writeUint64(uibFields.size());
    for (auto uInfo: uibFields) {
        f.writeString(uInfo.name.c_str());
        f.writeUint64(uInfo.size);
        f.writeUint8(static_cast<uint8_t>(uInfo.type));
        f.writeUint8(static_cast<uint8_t>(uInfo.precision));
    }
}

// ------------------------------------------------------------------------------------------------

MaterialSamplerInterfaceBlockChunk::MaterialSamplerInterfaceBlockChunk(
        SamplerInterfaceBlock const& sib) :
        Chunk(ChunkType::MaterialSib),
        mSib(sib) {
}

void MaterialSamplerInterfaceBlockChunk::flatten(Flattener& f) {
    f.writeString(mSib.getName().c_str());
    auto sibFields = mSib.getSamplerInfoList();
    f.writeUint64(sibFields.size());
    for (auto sInfo: sibFields) {
        f.writeString(sInfo.name.c_str());
        f.writeUint8(static_cast<uint8_t>(sInfo.type));
        f.writeUint8(static_cast<uint8_t>(sInfo.format));
        f.writeUint8(static_cast<uint8_t>(sInfo.precision));
        f.writeBool(sInfo.multisample);
    }
}

// ------------------------------------------------------------------------------------------------

MaterialSubpassInterfaceBlockChunk::MaterialSubpassInterfaceBlockChunk(SubpassInfo const& subpass) :
        Chunk(ChunkType::MaterialSubpass),
        mSubpass(subpass) {
}

void MaterialSubpassInterfaceBlockChunk::flatten(Flattener& f) {
    f.writeString(mSubpass.block.c_str());
    f.writeUint64(mSubpass.isValid ? 1 : 0);   // only ever a single subpass for now
    if (mSubpass.isValid) {
        f.writeString(mSubpass.name.c_str());
        f.writeUint8(static_cast<uint8_t>(mSubpass.type));
        f.writeUint8(static_cast<uint8_t>(mSubpass.format));
        f.writeUint8(static_cast<uint8_t>(mSubpass.precision));
        f.writeUint8(static_cast<uint8_t>(mSubpass.attachmentIndex));
        f.writeUint8(static_cast<uint8_t>(mSubpass.binding));
    }
}

// ------------------------------------------------------------------------------------------------

MaterialConstantParametersChunk::MaterialConstantParametersChunk(
        utils::FixedCapacityVector<MaterialConstant> constants)
    : Chunk(ChunkType::MaterialConstants), mConstants(std::move(constants)) {}

void MaterialConstantParametersChunk::flatten(Flattener& f) {
    f.writeUint64(mConstants.size());
    for (const auto& constant : mConstants) {
        f.writeString(constant.name.c_str());
        f.writeUint8(static_cast<uint8_t>(constant.type));
    }
}

// ------------------------------------------------------------------------------------------------

MaterialBindingUniformInfoChunk::MaterialBindingUniformInfoChunk(Container list) noexcept
        : Chunk(ChunkType::MaterialBindingUniformInfo),
          mBindingUniformInfo(std::move(list)) {
}

void MaterialBindingUniformInfoChunk::flatten(Flattener& f) {
    f.writeUint8(mBindingUniformInfo.size());
    for (auto const& [index, name, uniforms] : mBindingUniformInfo) {
        f.writeUint8(uint8_t(index));
        f.writeString({ name.data(), name.size() });
        f.writeUint8(uint8_t(uniforms.size()));
        for (auto const& uniform: uniforms) {
            f.writeString({ uniform.name.data(), uniform.name.size() });
            f.writeUint16(uniform.offset);
            f.writeUint8(uniform.size);
            f.writeUint8(uint8_t(uniform.type));
        }
    }
}

// ------------------------------------------------------------------------------------------------

MaterialAttributesInfoChunk::MaterialAttributesInfoChunk(Container list) noexcept
        : Chunk(ChunkType::MaterialAttributeInfo),
          mAttributeInfo(std::move(list))
{
}

void MaterialAttributesInfoChunk::flatten(Flattener& f) {
    f.writeUint8(mAttributeInfo.size());
    for (auto const& [attribute, location]: mAttributeInfo) {
        f.writeString({ attribute.data(), attribute.size() });
        f.writeUint8(location);
    }
}

// ------------------------------------------------------------------------------------------------

MaterialDescriptorBindingsChuck::MaterialDescriptorBindingsChuck(Container list) noexcept
        : Chunk(ChunkType::MaterialDescriptorBindingsInfo),
          mProgramDescriptorBindings(std::move(list)) {
}

void MaterialDescriptorBindingsChuck::flatten(Flattener& f) {
    assert_invariant(sizeof(backend::descriptor_set_t) == sizeof(uint8_t));
    assert_invariant(sizeof(backend::descriptor_binding_t) == sizeof(uint8_t));
    auto const& bindings = mProgramDescriptorBindings;
    f.writeUint8(bindings.size());
    for (auto&& entry: bindings) {
        f.writeString({ entry.name.data(), entry.name.size() });
        f.writeUint8(uint8_t(entry.type));
        f.writeUint8(entry.binding);
    }
}

// ------------------------------------------------------------------------------------------------

MaterialDescriptorSetLayoutChunk::MaterialDescriptorSetLayoutChunk(Container list) noexcept
        : Chunk(ChunkType::MaterialDescriptorSetLayoutInfo),
          mDescriptorSetLayout(std::move(list)) {
}

void MaterialDescriptorSetLayoutChunk::flatten(Flattener& f) {
    assert_invariant(sizeof(backend::descriptor_set_t) == sizeof(uint8_t));
    assert_invariant(sizeof(backend::descriptor_binding_t) == sizeof(uint8_t));
    auto const& bindings = mDescriptorSetLayout.bindings;
    f.writeUint8(bindings.size());
    for (auto&& entry: bindings) {
        f.writeUint8(uint8_t(entry.type));
        f.writeUint8(uint8_t(entry.stageFlags));
        f.writeUint8(entry.binding);
        f.writeUint8(uint8_t(entry.flags));
        f.writeUint16(entry.count);
    }
}

} // namespace filamat
