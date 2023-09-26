#include <vk/ShaderBase.h>
#include <vk/ThinDrawer.h>
#include <vk/SwapChain.h>
#include <vk/vkInit.h>

#define ALLOCATION_COUNT_ALL 20000
#define ALLOCATION_COUNT_POLY 20000

ShaderBase::ShaderBase(ThinDrawer* thinDrawer)
{
    this->thinDrawer = thinDrawer;
    this->logicalDevice = thinDrawer->logicalDevice;
}

void ShaderBase::init(ThinDrawer* thinDrawer)
{
    setSharedLayout(thinDrawer);
    writeSharedDescriptors(thinDrawer);
}

void ShaderBase::setSharedLayout(ThinDrawer* thinDrawer)
{
    const int s = 32;
    alignedSharedSize = thinDrawer->pad_uniform_buffer_size(sizeof(s_sharedUniformData));
    sharedOrthoUniform = (s_uniformBuffer*)malloc(sizeof(s_uniformBuffer));
    thinDrawer->uniformHelper(alignedSharedSize * s, sharedOrthoUniform, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, s);

    VkDescriptorSetLayoutBinding layoutBinding0 =
            vkinit::descriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 1, 0);

    VkDescriptorSetLayoutBinding bindings[] = { layoutBinding0 };

    VkDescriptorSetLayoutCreateInfo descriptorLayout = vkinit::descriptorSetLayoutCreateInfo(bindings, sizeof(bindings) / sizeof(bindings[0]));
    CHECK_RESULT_VK(vkCreateDescriptorSetLayout(thinDrawer->logicalDevice, &descriptorLayout, VK_NULL_HANDLE, &sharedLayout));
}

void ShaderBase::writeSharedDescriptors(ThinDrawer* thinDrawer)
{
    VkDescriptorSetAllocateInfo allocInfo = vkinit::descriptorSetAllocateInfo(thinDrawer->descriptorPool, &sharedLayout, 1);
    CHECK_RESULT_VK(vkAllocateDescriptorSets(thinDrawer->logicalDevice, &allocInfo, &sharedSet));

    VkWriteDescriptorSet writeDescriptorSet = { };

    writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet = sharedSet;
    writeDescriptorSet.descriptorCount = 1;
    writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    writeDescriptorSet.pBufferInfo = &ShaderBase::sharedOrthoUniform->descriptor;
    writeDescriptorSet.dstBinding = 0;
    vkUpdateDescriptorSets(thinDrawer->logicalDevice, 1, &writeDescriptorSet, 0, VK_NULL_HANDLE);
}

void ShaderBase::init()
{
    prepareVertexData();
    prepareUniforms();
    setupDescriptorSetLayout();
    preparePipeline();
    setupDescriptorSet();
}

VkDescriptorSetLayout ShaderBase::sharedLayout;
VkDescriptorSet ShaderBase::sharedSet;
int ShaderBase::alignedSharedSize;
s_uniformBuffer* ShaderBase::sharedOrthoUniform;