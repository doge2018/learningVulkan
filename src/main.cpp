#include<iostream>
#include<vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include"utilities.h"
#include"cleaners.h"
#include <glm/glm.hpp>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
#include<array>
//#define _PRINT_SOMETHING_FOR_LEARNING

using namespace std;
//可优化项：
//减少硬编码
//开启队列数
const uint32_t queueCount = 1;
//缓冲数
const uint32_t bufferCount = 2;
VkResult myVkResult;
//image格式srbg、非线性空间
const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
VkColorSpaceKHR imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
const int glfwWidth = 800;
const int glfwHeight = 800;

int main(){
    //日志初始化
    plog::init(plog::verbose, "C:\\onedrive\\project\\learningVulkan\\log.txt",51200,1);
    LOGI << "--enter main--";

    //init glfw
    if(!glfwInit()){
        LOGE<<"glfwInit";
    }
    LOGI<<"init glfw";
    GlfwCleaner glfwCleaner;
    //检查glfw vulkan支持
    if(!glfwVulkanSupported()){
        LOGE<<"glfwVulkanSupported";
        return -1;
    }

    //instance extentions
    vector<const char*> instanceExts;
    //glfw需要的instance extentions
    uint32_t glfwInstanceExtCount = 0;
    const char **glfwInstanceExts = glfwGetRequiredInstanceExtensions(&glfwInstanceExtCount);
    for(uint32_t i=0;i<glfwInstanceExtCount;++i){
        instanceExts.push_back(glfwInstanceExts[i]);
    }
    //需启用的instance extentions数量
    uint32_t instanceExtsCount{static_cast<uint32_t>(instanceExts.size())};

    //验证层
    vector<const char *> layers;
    //第1个验证层(共1个)
    const char *layer = "VK_LAYER_KHRONOS_validation";
    layers.push_back(layer);
    const uint32_t layersCount{static_cast<uint32_t>(layers.size())};

    //VkInstanceCreateInfo
    VkInstanceCreateInfo vkInstanceInfo{};
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    vkInstanceInfo.enabledExtensionCount = instanceExtsCount;
    vkInstanceInfo.ppEnabledExtensionNames = instanceExts.data();
    vkInstanceInfo.enabledLayerCount = layersCount;
    vkInstanceInfo.ppEnabledLayerNames = layers.data();
    
    //创建instance
    VkInstance instance;
    myVkResult = vkCreateInstance(&vkInstanceInfo,nullptr,&instance);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateInstance" << VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create instance";
    InstanceCleaner instanceCleaner{&instance};

    //创建glfw window
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    const char *glfwTitle = "my window";
    GLFWwindow *pwindow = glfwCreateWindow(glfwWidth,glfwHeight,glfwTitle,NULL,NULL);
    if(NULL==pwindow){
        LOGE<<"glfwCreateWindow";
        return -1;
    }
    LOGI<<"create window";
    WindowCleaner windowCleaner{pwindow};
    
    //创建surface
    VkSurfaceKHR surface;
    myVkResult = glfwCreateWindowSurface(instance,pwindow,nullptr,&surface);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"glfwCreateWindowSurface"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create surface";
    SurfaceCleaner surfaceCleaner{&instance,&surface};

    //选择物理设备
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    //枚举物理设备
    uint32_t physicDeviceCount = 0;
    myVkResult = vkEnumeratePhysicalDevices(instance,&physicDeviceCount,nullptr);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkEnumeratePhysicalDevices"<<VkResultToString(myVkResult);
        return -1;
    }
    vector<VkPhysicalDevice> physicDivices(physicDeviceCount);
    myVkResult = vkEnumeratePhysicalDevices(instance,&physicDeviceCount,physicDivices.data());
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkEnumeratePhysicalDevices" << VkResultToString(myVkResult);
        return -1;
    }
    for(auto &physicDivice:physicDivices){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicDivice,&properties);
        //选择独显
        if(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == properties.deviceType){
            physicalDevice = physicDivice;
            break;
        }
    }
    if(VK_NULL_HANDLE == physicalDevice){
        LOGE<<"selected null";
        return -1;
    }
    //检测selected physic device的队列族
    //获取队列族
    uint32_t physicDeviceQueueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&physicDeviceQueueFamiliesCount,nullptr);
    vector<VkQueueFamilyProperties> physicDeviceQueueFamilies(physicDeviceQueueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,&physicDeviceQueueFamiliesCount,physicDeviceQueueFamilies.data());
    //检测
    bool queueFamilyIsOK = false;//是否同时支持渲染和呈现
    VkBool32 surfaceIsSuppoted = VK_FALSE;//是否支持呈现
    //队列族索引
    uint32_t physicDeviceQueueFamiliesIndex = 0;
    while(physicDeviceQueueFamiliesIndex < physicDeviceQueueFamiliesCount){
        vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice,physicDeviceQueueFamiliesIndex,surface,&surfaceIsSuppoted);
        if(physicDeviceQueueFamilies[physicDeviceQueueFamiliesIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT && surfaceIsSuppoted){
            queueFamilyIsOK = true;
            break;
        }
        ++physicDeviceQueueFamiliesIndex;
    }
    if(!queueFamilyIsOK){
        //不支持
        LOGE<<"queueFamilyIsOK";
        return -1;
    }
    //队列族索引vector
    vector<uint32_t> queueFamiliesIndexes;
    //第1个队列族索引(共1个)
    queueFamiliesIndexes.push_back(physicDeviceQueueFamiliesIndex);
    const uint32_t queueFamiliesIndexCount{static_cast<uint32_t>(queueFamiliesIndexes.size())};

    //创建逻辑设备
    //填写deviceCreateInfo,
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount=queueFamiliesIndexCount;
    //队列族Infos
    vector<VkDeviceQueueCreateInfo> queueFamilyInfos;
    //第一个队列族Info(共一个)
    VkDeviceQueueCreateInfo queueFamilyInfo{};
    queueFamilyInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyInfo.queueFamilyIndex=physicDeviceQueueFamiliesIndex;//此队列族的索引
    queueFamilyInfo.queueCount=queueCount;
    //队列优先级vector
    vector<float> queuePriorities(queueCount);
    //第1个队列优先级(共1个)
    queuePriorities[0]= 1.0f;
    queueFamilyInfo.pQueuePriorities=queuePriorities.data();
    queueFamilyInfos.push_back(queueFamilyInfo);
    deviceCreateInfo.pQueueCreateInfos=queueFamilyInfos.data();
    deviceCreateInfo.enabledLayerCount=layersCount;
    deviceCreateInfo.ppEnabledLayerNames=layers.data();
    //device extentions
    vector<const char *> deviceExts;
    const char *deviceExt = "VK_KHR_swapchain";
    deviceExts.push_back(deviceExt);
    //device extentions count
    const uint32_t deviceExtsCount{static_cast<uint32_t>(deviceExts.size())};
    deviceCreateInfo.enabledExtensionCount=deviceExtsCount;
    deviceCreateInfo.ppEnabledExtensionNames=deviceExts.data();
    //创建
    VkDevice device;
    myVkResult = vkCreateDevice(physicalDevice,&deviceCreateInfo,nullptr,&device);
    if(VK_SUCCESS!= myVkResult){
        LOGE<<"vkCreateDevice--" << VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create device";
    DeviceCleaner deviceCleaner{&device};

    //查询surface capability
    VkSurfaceCapabilitiesKHR surfaceCapability;
    myVkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice,surface,&surfaceCapability);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkGetPhysicalDeviceSurfaceCapabilitiesKHR--" << VkResultToString(myVkResult);
        return -1;
    }
#ifdef _PRINT_SOMETHING_FOR_LEARNING
    //image数量
    LOGI<<"minImageCount:"<<surfaceCapability.minImageCount;
    LOGI<<"maxImageCount:"<<surfaceCapability.maxImageCount;
    //image 分辨率
    if(0xFFFFFFFF!=surfaceCapability.currentExtent.width){
        LOGI<<u8"固定分辨率:"<<surfaceCapability.currentExtent.width<<"*"<<surfaceCapability.currentExtent.height;
    }
    else{
        LOGI<<u8"分辨率可设定为--";
        LOGI<<u8"最大值："<<surfaceCapability.minImageExtent.width<<"*"<<surfaceCapability.minImageExtent.height;
        LOGI<<u8"最小值："<<surfaceCapability.maxImageExtent.width<<"*"<<surfaceCapability.maxImageExtent.height;
    }
#endif

    //创建swapchain
    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType=VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface=surface;
    swapchainCreateInfo.minImageCount=bufferCount;
    swapchainCreateInfo.imageFormat=imageFormat;
    swapchainCreateInfo.imageColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    if(0xFFFFFFFF==surfaceCapability.currentExtent.width){
        //分辨率不是固定
        LOGE<<"Extent is changeable";
        return -1;
    }
    //image width and height
    const VkExtent2D extent{surfaceCapability.currentExtent.width,surfaceCapability.currentExtent.height};
    swapchainCreateInfo.imageExtent=extent;
    //非多层贴图
    swapchainCreateInfo.imageArrayLayers=1;
    swapchainCreateInfo.imageUsage=VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.imageSharingMode=VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.queueFamilyIndexCount=queueFamiliesIndexCount;
    swapchainCreateInfo.pQueueFamilyIndices=queueFamiliesIndexes.data();
    swapchainCreateInfo.preTransform=VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    swapchainCreateInfo.compositeAlpha=VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.presentMode=VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.clipped=VK_TRUE;
    swapchainCreateInfo.oldSwapchain=VK_NULL_HANDLE;
    //创建
    VkSwapchainKHR swapchain;
    myVkResult =  vkCreateSwapchainKHR(device,&swapchainCreateInfo,nullptr,&swapchain);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateSwapchainKHR:" << VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create swapchain";
    SwapchainCleaner swapchainCleaner{&device,&swapchain};

    //创建render pass
    //render pass create info
    VkRenderPassCreateInfo renderPassCreateInfo{};
    //attachment
    vector<VkAttachmentDescription> attachmentsDescriptions;
    //第一个attachment(共一个)
    VkAttachmentDescription attachmentsDescription{};
    attachmentsDescription.format=imageFormat;
    attachmentsDescription.samples=VK_SAMPLE_COUNT_1_BIT;
    attachmentsDescription.loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentsDescription.storeOp=VK_ATTACHMENT_STORE_OP_STORE;
    attachmentsDescription.stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachmentsDescription.stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentsDescription.initialLayout=VK_IMAGE_LAYOUT_UNDEFINED;
    attachmentsDescription.finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachmentsDescriptions.push_back(attachmentsDescription);
    //subpass
    vector<VkSubpassDescription> subpasses;
    //第一个subpass(共1个)
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS;
    //attachmentReferences
    vector<VkAttachmentReference> attachmentReferences;
    //第一个attachmentReference(共1个)
    VkAttachmentReference attachmentReference{};
    attachmentReference.attachment=0;
    attachmentReference.layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    attachmentReferences.push_back(attachmentReference);
    subpass.pColorAttachments=attachmentReferences.data();
    subpass.colorAttachmentCount=static_cast<uint32_t>(attachmentReferences.size());
    subpasses.push_back(subpass);
    //VkRenderPassCreateInfo
    renderPassCreateInfo.sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassCreateInfo.pAttachments=attachmentsDescriptions.data();
    renderPassCreateInfo.attachmentCount=static_cast<uint32_t>(attachmentsDescriptions.size());
    renderPassCreateInfo.pSubpasses=subpasses.data();
    renderPassCreateInfo.subpassCount=static_cast<uint32_t>(subpasses.size());
    //subpass dependencies
    vector<VkSubpassDependency> subpassDependencies(1);
    //第1个subpass dependency(共1个)
    subpassDependencies[0].srcSubpass=VK_SUBPASS_EXTERNAL;
    subpassDependencies[0].dstSubpass=0;
    subpassDependencies[0].srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//上一阶段是颜色输出，这是期望的要求
    subpassDependencies[0].dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;//此阶段也是颜色输出
    subpassDependencies[0].srcAccessMask=0;//因为上一阶段是external，所以不需要特定的访问掩码
    subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;//此阶段需要写color
    subpassDependencies[0].dependencyFlags = 0;// 没有特殊依赖标志
    renderPassCreateInfo.pDependencies=subpassDependencies.data();
    renderPassCreateInfo.dependencyCount=static_cast<uint32_t>(subpassDependencies.size());
    //创建
    VkRenderPass renderpass;
    myVkResult=vkCreateRenderPass(device,&renderPassCreateInfo,nullptr,&renderpass);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateRenderPass--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create renderpass";
    RenderpassCleaner renderpassCleaner{&device,&renderpass};

    //获取image
    uint32_t imageCount=0;
    myVkResult = vkGetSwapchainImagesKHR(device,swapchain,&imageCount,nullptr);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"1st vkGetSwapchainImagesKHR--"<<VkResultToString(myVkResult);
        return -1;
    }
    vector<VkImage> images(imageCount);
    myVkResult = vkGetSwapchainImagesKHR(device,swapchain,&imageCount,images.data());
    if(VK_SUCCESS != myVkResult){
        LOGE<<"2nd vkGetSwapchainImagesKHR--"<<VkResultToString(myVkResult);
        return -1;
    }
    //所有imageview
    vector<VkImageView> imageviews;
    //所有framebuffer
    vector<VkFramebuffer> framebuffers;
    for(auto &image:images){
        //创建attachments(image views)
        vector<VkImageView> attachmentsForOneFreambuffer;
        //创建第1个image view(共1个)
        VkImageViewCreateInfo imageViewInfo{};
        imageViewInfo.sType=VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewInfo.image=image;
        imageViewInfo.viewType=VK_IMAGE_VIEW_TYPE_2D;
        imageViewInfo.format=imageFormat;
        VkComponentMapping componentMapping{};
        componentMapping.r=VK_COMPONENT_SWIZZLE_IDENTITY;
        componentMapping.g=VK_COMPONENT_SWIZZLE_IDENTITY;
        componentMapping.b=VK_COMPONENT_SWIZZLE_IDENTITY;
        componentMapping.a=VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewInfo.components=componentMapping;
        VkImageSubresourceRange subresourceRange{};
        subresourceRange.aspectMask=VK_IMAGE_ASPECT_COLOR_BIT;
        subresourceRange.baseMipLevel=0;
        subresourceRange.levelCount=1;
        subresourceRange.baseArrayLayer=0;
        subresourceRange.layerCount=1;
        imageViewInfo.subresourceRange=subresourceRange;
        //创建
        VkImageView imageView;
        myVkResult = vkCreateImageView(device,&imageViewInfo,nullptr,&imageView);
        if(VK_SUCCESS != myVkResult){
            LOGE<<"vkCreateImageView--"<<VkResultToString(myVkResult);
            return -1;
        }
        attachmentsForOneFreambuffer.push_back(imageView);
        imageviews.push_back(imageView);

        //创建framebuffer
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass=renderpass;
        framebufferInfo.pAttachments=attachmentsForOneFreambuffer.data();
        framebufferInfo.attachmentCount=static_cast<uint32_t>(attachmentsForOneFreambuffer.size());
        framebufferInfo.width=extent.width;
        framebufferInfo.height=extent.height;
        framebufferInfo.layers=1;
        //创建
        VkFramebuffer framebuffer;
        myVkResult = vkCreateFramebuffer(device,&framebufferInfo,nullptr,&framebuffer);
        if(VK_SUCCESS != myVkResult){
            LOGE<<"vkCreateFramebuffer--"<<VkResultToString(myVkResult);
            return -1;
        }
        framebuffers.push_back(framebuffer);
    }
    LOGI<<"create imageviews and framebuffers";
    //先创建imageviews cleaner，再创建framebuffers cleaner
    ImageviewsCleaner imageviewsCleaner{&device,imageviews};
    FramebuffersCleaner framebuffersCleaner{&device,framebuffers};

    //顶点数据
    struct Vertex {
        glm::vec2 position;
        glm::vec3 color;
    };
    std::vector<Vertex> vertexes = {
        {{0.0f, 0.5f}, {1.0f, 0.0f, 0.0f}},//上面的顶点，红色
        {{-0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}},//左下的顶点，蓝色
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},//右下的顶点，绿色
    };
    //数据大小
    auto VerticeDataSize = sizeof(vertexes[0]) * vertexes.size();

    //创建缓冲区
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size=static_cast<VkDeviceSize>(VerticeDataSize);
    bufferInfo.usage=VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    bufferInfo.sharingMode=VK_SHARING_MODE_EXCLUSIVE;
    bufferInfo.pQueueFamilyIndices=queueFamiliesIndexes.data();
    bufferInfo.queueFamilyIndexCount=queueFamiliesIndexCount;
    //创建
    VkBuffer buffer;
    myVkResult = vkCreateBuffer(device,&bufferInfo,nullptr,&buffer);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateBuffer--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create buffer";
    BufferCleaner bufferCleaner{&device,&buffer};

    //查询buffer内存需求
    VkMemoryRequirements memeryReq;
    vkGetBufferMemoryRequirements(device,buffer,&memeryReq);

    //查询物理设备内存属性
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice,&memoryProperties);
    //LOGI<< "memoryTypeCount--" <<memoryProperties.memoryTypeCount;
    //LOGI<< "memoryHeapCount--" <<memoryProperties.memoryHeapCount;

    //选择memory type，获得index
    uint32_t memoryTypeIndex = UINT32_MAX;
    VkMemoryPropertyFlags memoryPropertyFlag = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    for(uint32_t i=0;i<memoryProperties.memoryTypeCount;++i){
        if(memeryReq.memoryTypeBits & (1<<i) &&
        (memoryProperties.memoryTypes[i].propertyFlags & memoryPropertyFlag) == memoryPropertyFlag)
        {
               memoryTypeIndex = i;
                break;
        }
    }
    if(UINT32_MAX == memoryTypeIndex){
        LOGE<<"there is no proper memory type";
        return -1;
    }
    LOGI << "memoryTypeIndex==" << memoryTypeIndex;

    //申请内存
    VkMemoryAllocateInfo memoryInfo{};
    memoryInfo.sType=VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryInfo.allocationSize=memeryReq.size;
    memoryInfo.memoryTypeIndex=memoryTypeIndex;
    //申请
    VkDeviceMemory memory;
    myVkResult = vkAllocateMemory(device,&memoryInfo,nullptr,&memory);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkAllocateMemory--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI << "allocate memory";
    MemoryFreer memoryFreer{&device,&memory};

    //将memory与buffer绑定
    myVkResult=vkBindBufferMemory(device,buffer,memory,0);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkBindBufferMemory--"<<VkResultToString(myVkResult);
        return -1;
    }

    //将memory映射到内存空间
    void *data;
    myVkResult = vkMapMemory(device,memory,0,static_cast<VkDeviceSize>(VerticeDataSize),0,&data);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkMapMemory--"<<VkResultToString(myVkResult);
        return -1;
    }
    memcpy(data,vertexes.data(),static_cast<size_t>(VerticeDataSize));
    vkUnmapMemory(device,memory);

    //创建pipeline vertex绑定信息
    //binding信息
    vector<VkVertexInputBindingDescription> vertexBindingInfos;
    //第1个binding信息(共1个)
    VkVertexInputBindingDescription vertexBindingInfo{};
    vertexBindingInfo.binding=0;
    vertexBindingInfo.stride=static_cast<uint32_t>(sizeof(vertexes[0]));
    vertexBindingInfo.inputRate=VK_VERTEX_INPUT_RATE_VERTEX;
    vertexBindingInfos.push_back(vertexBindingInfo);
    const uint32_t vertexBindingInfocount{static_cast<uint32_t>(vertexBindingInfos.size())};
    //attribute信息
    vector<VkVertexInputAttributeDescription> vertexAttributeInfos;
    //第1个attribute信息(共2个)
    VkVertexInputAttributeDescription vertexAttributeInfo_0{};
    vertexAttributeInfo_0.location=0;
    vertexAttributeInfo_0.binding=0;
    vertexAttributeInfo_0.format=VK_FORMAT_R32G32_SFLOAT;
    vertexAttributeInfo_0.offset=offsetof(Vertex, position);
    vertexAttributeInfos.push_back(vertexAttributeInfo_0);
    //第2个attribute信息(共2个)
    VkVertexInputAttributeDescription vertexAttributeInfo_1{};
    vertexAttributeInfo_1.location=1;
    vertexAttributeInfo_1.binding=0;
    vertexAttributeInfo_1.format=VK_FORMAT_R32G32B32_SFLOAT;
    vertexAttributeInfo_1.offset=offsetof(Vertex, color);
    vertexAttributeInfos.push_back(vertexAttributeInfo_1);
    const uint32_t vertexAttributeInfocount{static_cast<uint32_t>(vertexAttributeInfos.size())};
    //VkPipelineVertexInputStateCreateInfo
    VkPipelineVertexInputStateCreateInfo pipelineVertexInfo{};
    pipelineVertexInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    pipelineVertexInfo.pVertexBindingDescriptions=vertexBindingInfos.data();
    pipelineVertexInfo.vertexBindingDescriptionCount=vertexBindingInfocount;
    pipelineVertexInfo.pVertexAttributeDescriptions=vertexAttributeInfos.data();
    pipelineVertexInfo.vertexAttributeDescriptionCount=vertexAttributeInfocount;

    //读取spv文件
    const string vshader = "C:\\onedrive\\project\\learningVulkan\\shaders\\shader.vert.spv";
    const string fshader = "C:\\onedrive\\project\\learningVulkan\\shaders\\shader.frag.spv";
    vector<char> vvshader=readSPVFile(vshader);
    vector<char> vfshader=readSPVFile(fshader);
    if(vvshader.empty()){
        LOGE<<"vvshader.empty";
        return -1;
    }
    if(vfshader.empty()){
        LOGE<<"vfshader.empty";
        return -1;
    }
    //create shader module
    //vertex shader module
    VkShaderModuleCreateInfo vshaderModuleInfo{};
    vshaderModuleInfo.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vshaderModuleInfo.codeSize=vvshader.size();
    vshaderModuleInfo.pCode=reinterpret_cast<const uint32_t*>(vvshader.data());
    //create
    VkShaderModule vshaderModule, fshaderModule;
    myVkResult=vkCreateShaderModule(device,&vshaderModuleInfo,nullptr,&vshaderModule);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateShaderModule vertex--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create vertex shader module";
    ShaderModuleCleaner vshaderModuleCleaner{&device,&vshaderModule};
    //fragment shader module
    VkShaderModuleCreateInfo fshaderModuleInfo{};
    fshaderModuleInfo.sType=VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fshaderModuleInfo.codeSize=vfshader.size();
    fshaderModuleInfo.pCode=reinterpret_cast<const uint32_t*>(vfshader.data());
    //create
    myVkResult=vkCreateShaderModule(device,&fshaderModuleInfo,nullptr,&fshaderModule);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateShaderModule fragment--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create fragment shader module";
    ShaderModuleCleaner fshaderModuleCleaner{&device,&fshaderModule};

    //VkPipelineShaderStageCreateInfo,当前含有2项(2个阶段)
    array<VkPipelineShaderStageCreateInfo,2> pipelineStageInfos{};
    const string enterFunction = "main";
    //vertex shader stage
    pipelineStageInfos[0].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineStageInfos[0].stage=VK_SHADER_STAGE_VERTEX_BIT;
    pipelineStageInfos[0].module=vshaderModule;
    pipelineStageInfos[0].pName=enterFunction.c_str();
    //fragment shader stage
    pipelineStageInfos[1].sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    pipelineStageInfos[1].stage=VK_SHADER_STAGE_FRAGMENT_BIT;
    pipelineStageInfos[1].module=fshaderModule;
    pipelineStageInfos[1].pName=enterFunction.c_str();
    uint32_t pipelineStageCount{static_cast<uint32_t>(pipelineStageInfos.size())};


    //create pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //create
    VkPipelineLayout pipelineLayout;
    myVkResult=vkCreatePipelineLayout(device,&pipelineLayoutInfo,nullptr,&pipelineLayout);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreatePipelineLayout--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create pipeline layout";
    PipelineLayoutCleaner pipelineLayoutCleaner{&device,&pipelineLayout};

    //create pipeline
    //VkPipelineInputAssemblyStateCreateInfo
    VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyInfo{};
    pipelineInputAssemblyInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    pipelineInputAssemblyInfo.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    pipelineInputAssemblyInfo.primitiveRestartEnable=VK_FALSE;
    //VkPipelineViewportStateCreateInfo
    //view port
    vector<VkViewport> viewports;
    //第1个view port(共1个)
    VkViewport viewport{};
    viewport.x=0.0f;
    viewport.y=0.0f;
    viewport.width=static_cast<float>(extent.width);
    viewport.height=static_cast<float>(extent.height);
    viewport.minDepth=0.0f;
    viewport.maxDepth=1.0f;
    viewports.push_back(viewport);
    const uint32_t viewportCount{static_cast<uint32_t>(viewports.size())};
    //裁剪区域
    vector<VkRect2D> scissors;
    //第1个裁剪区域(共1个)
    VkRect2D scissor{};
    scissor.offset={0,0};
    scissor.extent=extent;
    scissors.push_back(scissor);
    const uint32_t scissorsCount{static_cast<uint32_t>(scissors.size())};
    VkPipelineViewportStateCreateInfo viewportInfo{};
    viewportInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportInfo.pViewports=viewports.data();
    viewportInfo.viewportCount=viewportCount;
    viewportInfo.pScissors=scissors.data();
    viewportInfo.scissorCount=scissorsCount;
    //VkPipelineRasterizationStateCreateInfo
    VkPipelineRasterizationStateCreateInfo resterizationStateInfo{};
    resterizationStateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    resterizationStateInfo.depthClampEnable=VK_FALSE;
    resterizationStateInfo.rasterizerDiscardEnable=VK_FALSE;
    resterizationStateInfo.polygonMode=VK_POLYGON_MODE_FILL;
    resterizationStateInfo.cullMode=VK_CULL_MODE_BACK_BIT;
    resterizationStateInfo.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE;
    resterizationStateInfo.depthBiasEnable=VK_FALSE;
    resterizationStateInfo.lineWidth=1.0f;
    //VkPipelineMultisampleStateCreateInfo
    VkPipelineMultisampleStateCreateInfo multiSampleInfo{};
    multiSampleInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multiSampleInfo.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
    multiSampleInfo.sampleShadingEnable=VK_FALSE;
    multiSampleInfo.alphaToCoverageEnable=VK_FALSE;
    multiSampleInfo.alphaToOneEnable=VK_FALSE;
    multiSampleInfo.minSampleShading=1.0f;
    multiSampleInfo.pSampleMask=nullptr;
    //colorBlendAttachmentStates
    array<VkPipelineColorBlendAttachmentState,1> colorBlendAttachmentStates{};
    colorBlendAttachmentStates[0].blendEnable=VK_FALSE;
    colorBlendAttachmentStates[0].colorWriteMask=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT;
    //VkPipelineColorBlendStateCreateInfo
    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable=VK_FALSE;
    colorBlendInfo.logicOp=VK_LOGIC_OP_COPY;
    colorBlendInfo.pAttachments=colorBlendAttachmentStates.data();
    colorBlendInfo.attachmentCount=static_cast<uint32_t>(colorBlendAttachmentStates.size());

    //pipelineInfo
    vector<VkGraphicsPipelineCreateInfo> pipelineInfos;
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.pStages=pipelineStageInfos.data();
    pipelineInfo.stageCount=pipelineStageCount;
    pipelineInfo.pVertexInputState=&pipelineVertexInfo;
    pipelineInfo.pInputAssemblyState=&pipelineInputAssemblyInfo;
    pipelineInfo.pTessellationState=nullptr;
    pipelineInfo.pViewportState=&viewportInfo;
    pipelineInfo.pRasterizationState=&resterizationStateInfo;
    pipelineInfo.pMultisampleState=&multiSampleInfo;
    pipelineInfo.pDepthStencilState=nullptr;
    pipelineInfo.pColorBlendState=&colorBlendInfo;

    pipelineInfo.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.layout=pipelineLayout;
    pipelineInfo.renderPass=renderpass;
    pipelineInfo.subpass=0;//唯一的subpass索引
    pipelineInfo.basePipelineHandle=VK_NULL_HANDLE;
    pipelineInfo.basePipelineIndex=-1;
    pipelineInfos.push_back(pipelineInfo);
    //create
    VkPipeline pipeline;//只创建1个
    myVkResult=vkCreateGraphicsPipelines(device,VK_NULL_HANDLE,static_cast<uint32_t>(pipelineInfos.size()),pipelineInfos.data(),nullptr,&pipeline);
    if(VK_SUCCESS != myVkResult){
        LOGE<<"vkCreateGraphicsPipelines--"<<VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create pipeline";
    vkDestroyPipeline(device,pipeline,nullptr);
    return 0;
}