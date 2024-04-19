/*
 * Copyright (C) 2024 The Android Open Source Project
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

#include "DescriptorSet.h"

#include "DescriptorSetLayout.h"

#include "details/Engine.h"

#include <private/filament/EngineEnums.h>

#include <backend/DriverEnums.h>
#include <backend/Handle.h>

#include <utils/compiler.h>
#include <utils/debug.h>
#include <utils/FixedCapacityVector.h>

#include <utility>

#include <stdint.h>

namespace filament {

DescriptorSet::DescriptorSet() noexcept = default;

DescriptorSet::~DescriptorSet() noexcept = default;

DescriptorSet::DescriptorSet(DescriptorSetLayout const& descriptorSetLayout) noexcept
        : mDescriptors(descriptorSetLayout.getDescriptorCount()), mDirty(0xFFFFFFFF) {
}

DescriptorSet::DescriptorSet(DescriptorSet&& rhs) noexcept = default;

DescriptorSet& DescriptorSet::operator=(DescriptorSet&& rhs) noexcept = default;

void DescriptorSet::terminate(FEngine::DriverApi& driver) noexcept {
    if (mDescriptorSetHandle) {
        driver.destroyDescriptorSet(mDescriptorSetHandle);
    }
}

void DescriptorSet::commitSlow(DescriptorSetLayout const& layout,
        FEngine::DriverApi& driver) noexcept {
    mDirty.clear();
    // if we have a dirty descriptor set,
    // we need to allocate a new one and reset all the descriptors
    if (UTILS_LIKELY(mDescriptorSetHandle)) {
        driver.destroyDescriptorSet(mDescriptorSetHandle);
    }
    mDescriptorSetHandle = driver.createDescriptorSet(layout.getHandle());
    for (auto const& entry: layout.getDescriptorSetLayout().bindings) {
        if (entry.type == backend::DescriptorType::SAMPLER) {
            driver.updateDescriptorSetTexture(mDescriptorSetHandle, entry.binding,
                    mDescriptors[entry.binding].texture.th,
                    mDescriptors[entry.binding].texture.params);
        } else {
            driver.updateDescriptorSetBuffer(mDescriptorSetHandle, entry.binding,
                    mDescriptors[entry.binding].buffer.boh,
                    mDescriptors[entry.binding].buffer.offset,
                    mDescriptors[entry.binding].buffer.size);
        }
    }
}

void DescriptorSet::bind(FEngine::DriverApi& driver, DescriptorSetBindingPoints set) const noexcept {
    bind(driver, set, {});
}

void DescriptorSet::bind(FEngine::DriverApi& driver, DescriptorSetBindingPoints set,
        utils::FixedCapacityVector<uint32_t> dynamicOffsets) const noexcept {
    // TODO: on debug check that dynamicOffsets is large enough
    assert_invariant(mDirty.none());
    assert_invariant(mDescriptorSetHandle);
    driver.bindDescriptorSet(mDescriptorSetHandle, +set, std::move(dynamicOffsets));
}

void DescriptorSet::setBuffer(
        backend::descriptor_binding_t binding,
        backend::Handle<backend::HwBufferObject> boh, uint32_t offset, uint32_t size) noexcept {
    // TODO: validate it's the right kind of descriptor
    if (mDescriptors[binding].buffer.boh != boh || mDescriptors[binding].buffer.size != size) {
        // we don't set the dirty bit if only offset changes
        mDirty.set(binding);
    }
    mDescriptors[binding].buffer = { boh, offset, size };
}

void DescriptorSet::setSampler(
        backend::descriptor_binding_t binding,
        backend::Handle<backend::HwTexture> th, backend::SamplerParams params) noexcept {
    // TODO: validate it's the right kind of descriptor
    mDescriptors[binding].texture = { th, params };
    mDirty.set(binding);
}

} // namespace filament
