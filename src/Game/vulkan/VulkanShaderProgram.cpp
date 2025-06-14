#include "VulkanShaderProgram.h"

#include "VulkanContext.h"

#include <Engine/Log.h>
#include <shaderc/shaderc.hpp>
#include <spirv-reflect/spirv_reflect.h>
#include <vulkan/vk_enum_string_helper.h>

#include <cassert>
#include <fstream>

bool ReadTextFile(std::filesystem::path path, std::string& pOutFileContent) {
    std::ifstream stream(path.string());
    if (!stream) return false;
    std::string str((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    pOutFileContent = str;
    return true;
}

/// @brief File includer for shaderc.
class GlslIncluder : public shaderc::CompileOptions::IncluderInterface {
public:
    explicit GlslIncluder()  = default;
    ~GlslIncluder() override = default;

    void addSearchPath(std::filesystem::path path) {
        mSearchIncludePath.emplace_back(path);
    }

    shaderc_include_result* GetInclude(const char*          requestedPath,
                                       shaderc_include_type type,
                                       const char*          requestingPath,
                                       size_t               includeDepth) override {
        ENGINE_CORE_TRACE(" GetInclude.. File {} include {} (Depth: {})", requestingPath,
                          requestedPath, includeDepth);

        // resolve the path for the file to include.
        // return the absolute path.
        std::filesystem::path filePath = [this, type, requestedPath,
                                          requestingPath]() -> std::filesystem::path {
            if (type == shaderc_include_type_relative) {
                const std::filesystem::path path(requestingPath);
                const std::filesystem::path parentPath = path.parent_path();
                const std::filesystem::path fileToFind = parentPath / requestedPath;
                if (std::filesystem::exists(fileToFind)) {
                    return fileToFind;
                }
            } else {
                for (const auto& systemIncludePath : mSearchIncludePath) {
                    auto absolutePath = systemIncludePath / requestedPath;
                    if (std::filesystem::exists(absolutePath)) {
                        return absolutePath;
                    };
                }
            }
            return {};
        }();

        if (filePath.empty()) {
            auto* const data  = new shaderc_include_result();
            data->source_name = "";
            return data;
        }

        std::string content;
        auto        it = mIncludeFile.emplace(filePath.string(), content);
        if (it.second) {
            ReadTextFile(filePath, content);
            it.first->second = content;
        }

        auto* const data         = new shaderc_include_result();
        data->source_name        = it.first->first.c_str();
        data->source_name_length = it.first->first.size();
        data->content            = it.first->second.c_str();
        data->content_length     = it.first->second.size();
        return data;
    }

    void ReleaseInclude(shaderc_include_result* data) override { delete data; }

private:
    /// @brief Include file cache.
    ///        Key   : absolute path
    ///        Value : file content;
    std::map<std::string, std::string> mIncludeFile;

    /// @brief List of search path used for #include <...>
    std::vector<std::filesystem::path> mSearchIncludePath;
};

std::shared_ptr<VulkanShaderProgram> VulkanShaderProgram::CreateFromSpirv(
    std::initializer_list<std::filesystem::path> paths) {
    std::vector<std::vector<uint32_t>> spirv;
    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            ENGINE_CORE_ERROR("Fail {} does't exists.", path.string());
            continue;
        }

        std::ifstream ifs(path.string(), std::ios::binary);
        if (!ifs) {
            ENGINE_CORE_ERROR("Fail to open file: {}", path.string());
            continue;
        }

        std::vector<uint32_t> fileData;
        const auto            fileSize = std::filesystem::file_size(path);
        fileData.resize(fileSize / 4);
        ifs.read((char*)fileData.data(), fileSize);

        spirv.emplace_back(std::move(fileData));
    }

    return CreateFromSpirv(spirv);
}

std::shared_ptr<VulkanShaderProgram> VulkanShaderProgram::CreateFromSpirv(
    std::initializer_list<std::span<const uint32_t>> spirv) {
    std::vector<std::vector<uint32_t>> datas;
    for (const auto& s : spirv) {
        auto& data = datas.emplace_back();
        data.resize(s.size());
        std::memcpy(data.data(), s.data(), s.size_bytes());
    }

    return CreateFromSpirv(datas);
}

std::shared_ptr<VulkanShaderProgram> VulkanShaderProgram::CreateFromSpirv(
    std::vector<std::vector<uint32_t>> spirv) {
    auto program = std::make_shared<VulkanShaderProgram>();

    struct SetData {
        std::vector<VkDescriptorSetLayoutBinding> vkBinding;
    };
    std::map<uint32_t, SetData> setInfo;
    // std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>>
    // uniformBuffers; std::unordered_map<uint32_t, std::unordered_map<uint32_t,
    // VkDescriptorSetLayoutBinding>> combinedImageSampler;

    for (const auto& data : spirv) {
        spv_reflect::ShaderModule reflectShaderModule(data, SPV_REFLECT_MODULE_FLAG_NONE);
        assert(reflectShaderModule.GetResult() == SPV_REFLECT_RESULT_SUCCESS);

        VkShaderStageFlagBits shaderStage =
            (VkShaderStageFlagBits)reflectShaderModule.GetShaderStage();
        if (program->hasShaderStage(shaderStage)) {
            ENGINE_CORE_WARNING("VulkanShaderProgram already has the stage: ",
                                string_VkShaderStageFlagBits(shaderStage));
            continue;
        }

        program->mStages |= reflectShaderModule.GetShaderStage();

        //
        // Create the shader module
        //
        VkShaderModuleCreateInfo pCreateInfo{};
        pCreateInfo.sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        pCreateInfo.pNext    = nullptr;
        pCreateInfo.flags    = 0;
        pCreateInfo.codeSize = data.size() * 4;
        pCreateInfo.pCode    = data.data();

        VkShaderModule shaderModule{};
        vkCreateShaderModule(VulkanContext::getDevice(), &pCreateInfo, nullptr, &shaderModule);

        VulkanContext::setDebugObjectName((uint64_t)shaderModule, VK_OBJECT_TYPE_SHADER_MODULE,
                                          string_VkShaderStageFlags(program->mStages));

        //
        // Fill the structure used for creating a pipeline
        //
        VkPipelineShaderStageCreateInfo& createInfo =
            program->mPipelineShaderStageCreateInfos.emplace_back();
        createInfo.sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        createInfo.pNext               = nullptr;
        createInfo.flags               = 0;
        createInfo.stage               = shaderStage;
        createInfo.module              = shaderModule;
        createInfo.pName               = "main";
        createInfo.pSpecializationInfo = nullptr;

        //
        // Start reflection
        //
        {
            uint32_t count = 0;
            reflectShaderModule.EnumeratePushConstantBlocks(&count, nullptr);
            for (uint32_t i = 0; i < count; i++) {
                const SpvReflectBlockVariable* block =
                    reflectShaderModule.GetPushConstantBlock(i, nullptr);

                VkPushConstantRange& range = program->mPushConstantRanges.emplace_back();
                range.stageFlags           = shaderStage;
                range.offset               = block->offset;
                range.size                 = block->size;
            }
        }

        {
            uint32_t count = 0;
            reflectShaderModule.EnumerateDescriptorSets(&count, nullptr);
            std::vector<SpvReflectDescriptorSet*> descriptorSet(count);
            reflectShaderModule.EnumerateDescriptorSets(&count, descriptorSet.data());
            for (auto* reflectDescriptorSet: descriptorSet) {

                for (uint32_t bindingIdx = 0; bindingIdx < reflectDescriptorSet->binding_count;
                     bindingIdx++) {
                    const SpvReflectDescriptorBinding* reflectBinding =
                        reflectDescriptorSet->bindings[bindingIdx];
                    if (!reflectBinding) {
                        continue;
                    }

                    switch (reflectBinding->descriptor_type) {
                        case SPV_REFLECT_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER: {
                            auto& set               = setInfo[reflectBinding->set];
                            auto it = std::find_if(set.vkBinding.begin(), set.vkBinding.end(), [&reflectBinding](const VkDescriptorSetLayoutBinding& b){ return b.binding == reflectBinding->binding; });
                            if(it == set.vkBinding.end()) {
                                auto& binding           = set.vkBinding.emplace_back();
                                binding.binding         = reflectBinding->binding;
                                binding.descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                                binding.descriptorCount = reflectBinding->count;
                                binding.stageFlags      = shaderStage;
                            } else {
                                it->stageFlags |= shaderStage;
                            }
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_INPUT_ATTACHMENT: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLED_IMAGE: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_SAMPLER: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_IMAGE: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER: {
                            auto& set = setInfo[reflectBinding->set];
                            if (reflectBinding->binding == set.vkBinding.size()) {
                                set.vkBinding.emplace_back();
                            }
                            auto& binding           = set.vkBinding[reflectBinding->binding];
                            binding.binding         = reflectBinding->binding;
                            binding.descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                            binding.descriptorCount = 1;
                            binding.stageFlags |= shaderStage;
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC: {
                            break;
                        }
                        case SPV_REFLECT_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER: {
                            break;
                        }
                    }
                }
            }
        }
    }

    for (const auto& [setIdx, info] : setInfo) {
        VkDescriptorSetLayoutCreateInfo descriptorLayout = {};
        descriptorLayout.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorLayout.pNext        = nullptr;
        descriptorLayout.flags        = 0;
        descriptorLayout.bindingCount = info.vkBinding.size();
        descriptorLayout.pBindings    = info.vkBinding.data();
        VkDescriptorSetLayout a{};
        vkCreateDescriptorSetLayout(VulkanContext::getDevice(), &descriptorLayout, nullptr, &a);
        program->mDescriptorSetLayout.push_back(a);
    }

    //
    // Create pipeline layout
    //
    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType          = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext          = nullptr;
    pPipelineLayoutCreateInfo.setLayoutCount = program->mDescriptorSetLayout.size();
    pPipelineLayoutCreateInfo.pSetLayouts    = program->mDescriptorSetLayout.data();
    pPipelineLayoutCreateInfo.pushConstantRangeCount =
        (uint32_t)program->mPushConstantRanges.size();
    pPipelineLayoutCreateInfo.pPushConstantRanges = program->mPushConstantRanges.data();

    vkCreatePipelineLayout(VulkanContext::getDevice(), &pPipelineLayoutCreateInfo, nullptr,
                           &program->mPipelineLayout);
    return program;
}

std::shared_ptr<VulkanShaderProgram> VulkanShaderProgram::CreateFromFile(
    std::vector<std::filesystem::path> paths) {
    std::vector<std::vector<uint32_t>> allSpirv;

    for (const auto& path : paths) {
        if (!std::filesystem::exists(path)) {
            ENGINE_CORE_ERROR("File {} does't exists.", path.string());
            return nullptr;
        }

        std::ifstream ifs(path.string(), std::ios::in);
        if (!ifs) {
            ENGINE_CORE_ERROR("Fail to open file: {}", path.string());
            return nullptr;
        }

        std::string source;
        if (!ReadTextFile(path, source)) {
            ENGINE_CORE_ERROR("Fail to read file: {}", path.string());
            return nullptr;
        }

        ENGINE_CORE_INFO("Compiling shader: {}", path.string());

        auto includer = std::make_unique<GlslIncluder>();

        shaderc::Compiler       compiler;
        shaderc::CompileOptions options;
        options.SetWarningsAsErrors();
        options.SetOptimizationLevel(shaderc_optimization_level_performance);
        options.SetIncluder(std::move(includer));
        shaderc::CompilationResult result = compiler.CompileGlslToSpv(
            source, shaderc_glsl_infer_from_source, path.string().c_str(), options);
        if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
            ENGINE_CORE_ERROR("Fail to compile shader file: {}", result.GetErrorMessage());
            return nullptr;
        }
        std::vector<uint32_t> spirv{result.cbegin(), result.cend()};
        allSpirv.push_back(spirv);
    }

    return CreateFromSpirv(allSpirv);
}

VulkanShaderProgram::~VulkanShaderProgram() {
    // delete shader module
    for (auto& info : mPipelineShaderStageCreateInfos) {
        vkDestroyShaderModule(VulkanContext::getDevice(), info.module, nullptr);
    }

    // delete pipline layout
    vkDestroyPipelineLayout(VulkanContext::getDevice(), mPipelineLayout, nullptr);

    // delete descriptor set layout
    for (auto setLayout : mDescriptorSetLayout) {
        vkDestroyDescriptorSetLayout(VulkanContext::getDevice(), setLayout, nullptr);
    }
}
