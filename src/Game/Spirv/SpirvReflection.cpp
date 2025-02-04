#include "SpirvReflection.h"

#include <spirv_reflect.h>

#include <cassert>

void SpirvReflection::reflect(std::span<const uint32_t> spirv) {
    // Generate reflection data for a shader
    SpvReflectShaderModule module;
    SpvReflectResult       result =
        spvReflectCreateShaderModule(spirv.size_bytes(), spirv.data(), &module);
    assert(result == SPV_REFLECT_RESULT_SUCCESS);

    mShaderStage = static_cast<VkShaderStageFlagBits>(module.shader_stage);

    std::vector<SpvReflectBlockVariable*> reflectBlockVariable;
    {
        uint32_t count = 0;
        result         = spvReflectEnumeratePushConstantBlocks(&module, &count, NULL);
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        reflectBlockVariable.resize(count);
        result = spvReflectEnumeratePushConstantBlocks(&module, &count, reflectBlockVariable.data());
        assert(result == SPV_REFLECT_RESULT_SUCCESS);

        assert(reflectBlockVariable.size() <= 1);
        for (auto& pushConstant : reflectBlockVariable) {
            mPushConstantRange.stageFlags = module.shader_stage;
            mPushConstantRange.offset     = pushConstant->offset;
            mPushConstantRange.size       = pushConstant->size;
        }
    }

    // Destroy the reflection data when no longer required.
    spvReflectDestroyShaderModule(&module);
}
