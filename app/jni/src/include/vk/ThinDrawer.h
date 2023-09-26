#pragma once

#include "vk/WindowHandler.h"
#include "vk/definitions.h"
#include <vulkan/vulkan.h>
#include "vk/ShaderBase.h"
#include "ShaderIncludes/GunEffect.h"

#define PRINT_INFO_MESSAGES 1

static const char* ENABLE_EXTENSIONS[] =
{
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static const int NUM_ENABLE_EXTENSIONS = sizeof(ENABLE_EXTENSIONS) / sizeof(ENABLE_EXTENSIONS[0]);

typedef struct
{
    VkCommandPool commandPool;
    VkCommandBuffer commandBuffer;
    VkFence renderFence;
    VkSemaphore presentSemaphore;
    VkSemaphore renderSemaphore;
} s_frameData;

typedef struct
{
    VkQueue graphicsQueue;
    uint32_t graphicsQueueFamilyIndex;
} s_queueData;

typedef struct
{
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures features;
    VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
} s_vulkanInfo;

typedef struct
{
    VkDeviceMemory memory;
    VkBuffer buffer;
} s_stagingBuffer;

typedef struct
{
    s_stagingBuffer vertices;
    s_stagingBuffer indices;
} s_stagingBuffers;

typedef struct TextureData
{
    unsigned char* data;
    int width;
    int height;
    int numChannels;
} s_textureData;

class SwapChain;
class PortalBody;
class Portal;
class PortalWorld;

class ThinDrawer
{
public:
    static ThinDrawer* object;

    PortalWorld* pWorld;
    VkDescriptorPool imguiPool;

    ThinDrawer(PortalWorld* pWorld);

    bool drawReleases = false;
    bool drawDebugBodies = false;
    bool drawDebugEdges = false;
    bool drawDebugPortals = false;

    // Base var start

    uint32_t frameNumber = 0;
    uint32_t lastSwapChainImageIndex = 0;
    VkSampleCountFlagBits samples;
    VkSampleCountFlagBits desiredSamples = VK_SAMPLE_COUNT_1_BIT;

    s_vulkanInfo* vulkanInfo;
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice logicalDevice;
    s_queueData queues;
    SwapChain* swapChain;
    VkRenderPass renderPass;
    VkCommandPool uploadPool;
    VkFence uploadFence;

    WindowHandler* wh;

    std::vector<s_frameData> frames;

    VkDescriptorPool descriptorPool;

    std::vector<std::pair<void (*) (void*), void*>> memMapFuncs;
    
    std::vector<std::pair<void (*) (VkCommandBuffer, void*), void*>> earlyDrawFuncs;
    std::vector<std::pair<void (*) (VkCommandBuffer, void*), void*>> midDrawFuncs;
    std::vector<std::pair<void (*) (VkCommandBuffer, void*), void*>> drawFuncs;
    std::vector<std::pair<void (*) (VkCommandBuffer, void*), void*>> lateDrawFuncs;

    // Diff var start


    double gameTime = 0.0;
    double frameDT = 0.0;
    double upUpAnim = 0.0;
    double upDownAnim = 0.0;

    GunEffect* gunEffect;
    std::vector<ShaderBase*> shaders;

    // var end

    void surfaceRecreate();

    void createWindow();
    void initBase();
    void initExtra();

    void createInstance();
    void createCommands();
    void createSyncThings();
    void createRenderPass();
    void createLogicalDevice();
    void selectPhysicalDevice();
    void preparePipelines();
    void setupDescriptorPool();

    void renderCommandBeginner(VkCommandBuffer& cmdBuff);
    void buildCommandBuffers();

    void beforeCreate();
    void afterCreate();
    
    void uniformFiller();
    void renderLoop();
    void animate();

    void bufferStage(void* bufferData, uint32_t dataSize, VkBufferUsageFlags flags, s_buffers* fillBuffer);
    VkGraphicsPipelineCreateInfo* getPipelineInfoBase();
    static void freePipelineData(VkGraphicsPipelineCreateInfo* pipelineCreateInfo);
    void uniformHelper( int size, s_uniformBuffer* uniformBuffer,
                        VkBufferUsageFlagBits usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, int numInstances = 1);

    int pad_uniform_buffer_size(int originalSize);
    int pad_storage_buffer_size(int originalSize);

    VkCommandBuffer getCommandBuffer(bool begin);
    void flushCommandBuffer(VkCommandBuffer commandBuffer);
    uint32_t getMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties);
    void setSamples();

    VkCommandBuffer createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin);
    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    void loadTexture(char* fileName, s_texture* texture, bool enableAnisotropy = true,
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkFilter filter = VK_FILTER_LINEAR, float lodBias=0.0);
    void loadTextureFromMemory(s_textureData* texData, s_texture* texture, bool enableAnisotropy = true,
        VkSamplerAddressMode addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT, VkFilter filter = VK_FILTER_LINEAR, int numMipLevels=0);
    void updateImageDescriptors(s_texture* tex, VkDescriptorSetLayout& setLayout);
    void createImage(uint32_t width, uint32_t p_height, uint32_t mipLevels,
                     VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage,
                     VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory, uint32_t depth=1, uint32_t arrayLayers=1);

    static bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
};