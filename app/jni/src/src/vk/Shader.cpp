#include <vk/Shader.h>
#include <vk/definitions.h>
#include "SDL.h"

Shader::Shader(VkDevice device, std::vector<std::string> fileNames)
{
    this->device = device;
    this->shaderStages.resize(fileNames.size());

    VkShaderStageFlagBits bits[3] = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT, VK_SHADER_STAGE_GEOMETRY_BIT };

    for (int i = 0; i < fileNames.size(); i++)
    {
        VkShaderModule shaderModule = load_shader_module(device, fileNames[i].c_str());
        shaderModules.push_back(shaderModule);
        VkPipelineShaderStageCreateInfo info = { };
        info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;

        info.stage = bits[i];
        info.module = shaderModule;
        info.pName = "main";
        shaderStages[i] = info;
    }
}

Shader::~Shader()
{
    for (int i = 0; i < shaderModules.size(); i++) {
        vkDestroyShaderModule(device, shaderModules.at(i), NULL);
    }
    shaderModules.clear();
    shaderStages.clear();
}

VkShaderModule Shader::load_shader_module(VkDevice device, const char* filePath)
{
    std::string finalPath = filePath;
    SDL_RWops* sdlFile = SDL_RWFromFile(finalPath.c_str(), "rb");

    if (!sdlFile)
    {
        SDL_Log("Could not find shader file: %s -- SDLError: %s\n", finalPath.c_str(), SDL_GetError());
        exit(1);
    }

    Sint64 size = SDL_RWsize(sdlFile);
    std::vector<char> buffer; buffer.resize(size);
    Sint64 bytesRead = SDL_RWread(sdlFile, buffer.data(), 1, size);
    SDL_RWclose(sdlFile);

    VkShaderModuleCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = buffer.size();
    createInfo.pCode = (uint32_t*)buffer.data();

    VkShaderModule shaderModule;
    CHECK_RESULT_VK(vkCreateShaderModule(device, &createInfo, NULL, &shaderModule));

    return shaderModule;
}