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

#ifndef TNT_FILAMENT_DESCRIPTORSETLAYOUT_H
#define TNT_FILAMENT_DESCRIPTORSETLAYOUT_H

#include <backend/DriverEnums.h>

#include <backend/DriverApiForward.h>
#include <backend/Handle.h>

#include <utils/bitset.h>

#include <stddef.h>
#include <stdint.h>

namespace filament {

class DescriptorSetLayout {
public:
    DescriptorSetLayout() noexcept;
    DescriptorSetLayout(backend::DriverApi& driver,
            backend::DescriptorSetLayout descriptorSetLayout) noexcept;

    DescriptorSetLayout(DescriptorSetLayout const&) = delete;
    DescriptorSetLayout(DescriptorSetLayout&& rhs) noexcept;
    DescriptorSetLayout& operator=(DescriptorSetLayout const&) = delete;
    DescriptorSetLayout& operator=(DescriptorSetLayout&& rhs) noexcept;

    void terminate(backend::DriverApi& driver) noexcept;

    backend::DescriptorSetLayoutHandle getHandle() const noexcept {
        return mDescriptorSetLayoutHandle;
    }

    backend::DescriptorSetLayout const& getDescriptorSetLayout() const noexcept {
        return mDescriptorSetLayout;
    }

    size_t getDynamicBufferCount() const noexcept {
        return mDynamicBufferCount;
    }

    size_t getBufferCount() const noexcept {
        return mBufferCount;
    }

    size_t getSamplerCount() const noexcept {
        return mSamplerCount;
    }

    size_t getDescriptorCount() const noexcept {
        return mDescriptorCount;
    }

    utils::bitset64 getDynamicBuffers() const noexcept {
        return mDynamicBuffers;
    }

private:
    backend::DescriptorSetLayoutHandle mDescriptorSetLayoutHandle;
    backend::DescriptorSetLayout mDescriptorSetLayout;
    utils::bitset64 mDynamicBuffers;
    uint8_t mDynamicBufferCount = 0;
    uint8_t mBufferCount = 0;
    uint8_t mSamplerCount = 0;
    uint8_t mDescriptorCount;
};


} // namespace filament

#endif //TNT_FILAMENT_DESCRIPTORSETLAYOUT_H
