#include "SpirvReflection.h"

#include <Engine/Log.h>
#include <spirv_reflect.h>
#include <vulkan/vk_enum_string_helper.h>

#include <cassert>

void SpirvReflection::reflect(std::span<const uint32_t> spirv) {
    // Generate reflection data for a shader
    SpvReflectShaderModule module;
    SpvReflectResult       result =
        spvReflectCreateShaderModule(spirv.size_bytes(), spirv.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    mShaderStage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

    // Push constant
    std::vector<SpvReflectBlockVariable*> reflectBlockVariable;
    {
        uint32_t count = 0;
        result         = spvReflectEnumeratePushConstantBlocks(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        reflectBlockVariable.resize(count);
        result =
            spvReflectEnumeratePushConstantBlocks(&module, &count, reflectBlockVariable.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        assert(reflectBlockVariable.size() <= 1);
        for (auto& pushConstant : reflectBlockVariable) {
            mPushConstantRange.stageFlags = module.shader_stage;
            mPushConstantRange.offset     = pushConstant->offset;
            mPushConstantRange.size       = pushConstant->size;
        }
    }

    // set reflection
    std::vector<SpvReflectDescriptorSet*> reflectDescriptorSet;
    {
        uint32_t count = 0;
        result         = spvReflectEnumerateDescriptorSets(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        reflectDescriptorSet.resize(count);
        result = spvReflectEnumerateDescriptorSets(&module, &count, reflectDescriptorSet.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        for (auto& descriptorSet : reflectDescriptorSet) {
            ENGINE_CORE_DEBUG("set index : {}", descriptorSet->set);
            ENGINE_CORE_DEBUG("nb binding: {}", descriptorSet->binding_count);

            for (uint32_t i = 0; i < descriptorSet->binding_count; i++) {
                SpvReflectDescriptorBinding* binding = descriptorSet->bindings[i];
                ENGINE_CORE_DEBUG(" Name: {} set({}) binding({}) {}", binding->name, binding->set, binding->binding, string_VkDescriptorType((VkDescriptorType)binding->descriptor_type));

                const SpvReflectBlockVariable& block = binding->block;
                ENGINE_CORE_DEBUG(" block name            : {}", block.name);
                ENGINE_CORE_DEBUG(" block offset          : {}", block.offset);
                ENGINE_CORE_DEBUG(" block absolute_offset : {}", block.absolute_offset);
                ENGINE_CORE_DEBUG(" block size            : {}", block.size);
                ENGINE_CORE_DEBUG(" block padded_size     : {}", block.padded_size);
                ENGINE_CORE_DEBUG(" block flags           : {}", block.flags);
                ENGINE_CORE_DEBUG(" block decoration_flags: {}", block.decoration_flags);

                for (uint32_t memberIdx = 0; memberIdx < block.member_count; memberIdx++) {
                    const SpvReflectBlockVariable& var = block.members[memberIdx];
                    ENGINE_CORE_DEBUG(" -Name: {} offset: {} size: {} flags: {}", var.name, var.offset, var.size, var.flags);
                }

                //spvReflectBlockVariableTypeName()
            }
        }
    }

    std::vector<SpvReflectDescriptorBinding*> reflectDescriptorBinding;
    {
        uint32_t count = 0;
        result         = spvReflectEnumerateDescriptorBindings(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        reflectDescriptorBinding.resize(count);
        result =
            spvReflectEnumerateDescriptorBindings(&module, &count, reflectDescriptorBinding.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        for (auto& binding : reflectDescriptorBinding) {
            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
            descriptorSetLayoutBinding.binding         = binding->binding;
            descriptorSetLayoutBinding.descriptorType  = (VkDescriptorType)binding->descriptor_type;
            descriptorSetLayoutBinding.descriptorCount = binding->count;
            descriptorSetLayoutBinding.stageFlags      = module.shader_stage;
            descriptorSetLayoutBinding.pImmutableSamplers;
            mDescriptorSetLayoutBinding.push_back(descriptorSetLayoutBinding);
        }
    }


    // Destroy the reflection data when no longer required.
    spvReflectDestroyShaderModule(&module);
}
