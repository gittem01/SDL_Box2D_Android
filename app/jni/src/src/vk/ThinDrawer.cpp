#include <vk/ThinDrawer.h>
#include <vk/SwapChain.h>
#include <vk/Camera.h>
#include <vk/Shader.h>
#include <vk/vkInit.h>
#include <vector>
#include <array>

ThinDrawer* ThinDrawer::object = nullptr;

ThinDrawer::ThinDrawer(PortalWorld* pWorld)
{
    ThinDrawer::object = this;

    this->pWorld = pWorld;

    initBase();
    initExtra();
}

void ThinDrawer::surfaceRecreate()
{
    vkDeviceWaitIdle(logicalDevice);

    for (ShaderBase* shader : shaders)
    {
        vkDestroyPipeline(logicalDevice, shader->pipeline, VK_NULL_HANDLE);
    }

    swapChain->destroy();
    
    swapChain->creationLoop();
    createRenderPass();
    swapChain->createFrameBuffers();
    preparePipelines();

   for (int i = 0; i < frames.size(); i++)
   {
       vkResetCommandBuffer(frames[i].commandBuffer, 0);
   }

    vkDeviceWaitIdle(logicalDevice);
}

void ThinDrawer::initExtra()
{
    setupDescriptorPool();

    ShaderBase::init(this);

    gunEffect = new GunEffect(this); shaders.push_back(gunEffect);

    for (ShaderBase* shader : shaders)
    {
        shader->init();
    }
}

void ThinDrawer::uniformFiller()
{
    s_sharedUniformData* sharedData;
    CHECK_RESULT_VK(vkMapMemory(logicalDevice, ShaderBase::sharedOrthoUniform->memory, ShaderBase::alignedSharedSize * lastSwapChainImageIndex,
        ShaderBase::alignedSharedSize, 0, (void**)&sharedData));
    
    sharedData->orthoMatrix = wh->cam->camMatrix;
    sharedData->cameraPos = glm::vec4(
        wh->cam->pos.x, wh->cam->pos.y,
        wh->cam->xSides.y - wh->cam->xSides.x, wh->cam->ySides.y - wh->cam->ySides.x);
    sharedData->data.x = wh->cam->zoom;
    sharedData->data.y = gameTime;

    glm::vec2 worldMouse = wh->cam->getMouseCoords();
    sharedData->data.z = worldMouse.x;
    sharedData->data.w = worldMouse.y;

    
    vkUnmapMemory(logicalDevice, ShaderBase::sharedOrthoUniform->memory);
}

void ThinDrawer::renderLoop()
{
    int imNum = frameNumber % swapChain->imageCount;

    s_frameData currentFrame = frames[imNum];

    CHECK_RESULT_VK(vkWaitForFences(logicalDevice, 1, &currentFrame.renderFence, true, UINT64_MAX));
    CHECK_RESULT_VK(vkResetFences(logicalDevice, 1, &currentFrame.renderFence));

    VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain->swapChain, UINT64_MAX,
                          currentFrame.presentSemaphore, VK_NULL_HANDLE, &lastSwapChainImageIndex);

    for (auto& f : memMapFuncs)
    {
        f.first(f.second);
    }

    uniformFiller();

    vkResetCommandBuffer(frames[lastSwapChainImageIndex].commandBuffer, 0);
    buildCommandBuffers();

    VkSubmitInfo submit = { };
    submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    submit.pWaitDstStageMask = &waitStage;

    submit.waitSemaphoreCount = 1;
    submit.pWaitSemaphores = &currentFrame.presentSemaphore;

    submit.signalSemaphoreCount = 1;
    submit.pSignalSemaphores = &currentFrame.renderSemaphore;

    submit.commandBufferCount = 1;
    submit.pCommandBuffers = &frames[lastSwapChainImageIndex].commandBuffer;

    CHECK_RESULT_VK(vkQueueSubmit(queues.graphicsQueue, 1, &submit, currentFrame.renderFence));
    VkPresentInfoKHR presentInfo = { };
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pSwapchains = &swapChain->swapChain;
    presentInfo.swapchainCount = 1;

    presentInfo.pWaitSemaphores = &currentFrame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;

    presentInfo.pImageIndices = &lastSwapChainImageIndex;

    result = vkQueuePresentKHR(queues.graphicsQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
    {
        surfaceRecreate();
    }

    frameNumber++;
}

void ThinDrawer::preparePipelines()
{
    for (ShaderBase* shader : shaders)
    {
        shader->preparePipeline();
    }
}

void ThinDrawer::setupDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> sizes =
    {
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    };

    VkDescriptorPoolCreateInfo descriptorPoolInfo = { };
    descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptorPoolInfo.poolSizeCount = sizes.size();
    descriptorPoolInfo.pPoolSizes = sizes.data();
    descriptorPoolInfo.maxSets = 10000;

    CHECK_RESULT_VK(vkCreateDescriptorPool(logicalDevice, &descriptorPoolInfo, VK_NULL_HANDLE, &descriptorPool));
}

void ThinDrawer::renderCommandBeginner(VkCommandBuffer& cmdBuff)
{
    VkCommandBufferBeginInfo cmdBufInfo = vkinit::commandBufferBeginInfo();

    VkClearValue clearValue;
    clearValue.color = { 0.0f, 0.0f, 0.1f, 1.0f };

    VkClearValue clearValues[] = { clearValue };

    VkRenderPassBeginInfo renderPassBeginInfo = vkinit::renderPassBeginInfo(renderPass, swapChain->extent.width, swapChain->extent.height, clearValues);
    renderPassBeginInfo.clearValueCount = std::size(clearValues);

    renderPassBeginInfo.framebuffer = swapChain->frameBuffers[lastSwapChainImageIndex];

    CHECK_RESULT_VK(vkBeginCommandBuffer(cmdBuff, &cmdBufInfo));

    vkCmdBeginRenderPass(cmdBuff, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport = { };
    viewport.height = (float)swapChain->extent.height;
    viewport.width = (float)swapChain->extent.width;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    vkCmdSetViewport(cmdBuff, 0, 1, &viewport);
}

void ThinDrawer::buildCommandBuffers()
{
    const uint32_t camOffset = ShaderBase::alignedSharedSize * lastSwapChainImageIndex;

    VkCommandBuffer cmdBuff = frames[lastSwapChainImageIndex].commandBuffer;

    renderCommandBeginner(cmdBuff);

    VkDeviceSize offsets = 0;

    for (auto& f : earlyDrawFuncs)
    {
        f.first(cmdBuff, f.second);
    }

    for (auto& f : drawFuncs)
    {
        f.first(cmdBuff, f.second);
    }

    if (pWorld)
    {

    }

    vkCmdBindPipeline(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS, gunEffect->pipeline);

    vkCmdBindVertexBuffers(cmdBuff, 0, 1, &gunEffect->buffers[0]->buffer, &offsets);

    vkCmdBindDescriptorSets(cmdBuff, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            gunEffect->pipelineLayout, 0, 1, &ShaderBase::sharedSet, 1, &camOffset);

    s_gunData gunData = { { 0.0f, -55.0f, 0.0f, 15.0f },   { 1.0f, 1.0f, 1.0f, 1.0f },
                         { NAN, 0.0f, 0.0f, 0.0f },     { 2.5f, 0.0f, 0.0f, 0.0f } };
    vkCmdPushConstants(cmdBuff, gunEffect->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(s_gunData), (void*)&gunData);

    vkCmdDraw(cmdBuff, gunEffect->buffers[0]->count, 1, 0, 0);

    for (auto& f : lateDrawFuncs)
    {
        f.first(cmdBuff, f.second);
    }

    vkCmdEndRenderPass(cmdBuff);

    CHECK_RESULT_VK(vkEndCommandBuffer(cmdBuff));
}