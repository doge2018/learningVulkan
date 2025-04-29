#include<iostream>
#include<vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include"utilities.h"
#include"cleaners.h"
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
//#define _PRINT_SOMETHING_FOR_LEARNING

using namespace std;

//开启队列数
const uint32_t queueCount = 1;
//缓冲数
const uint32_t bufferCount = 2;
VkResult myVkResult;
//image格式srbg、非线性空间
const VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
VkColorSpaceKHR imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

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
    const int glfwWidth = 800;
    const int glfwHeight = 800;
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
    VkPhysicalDevice selectedPhysicDevice = VK_NULL_HANDLE;
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
            selectedPhysicDevice = physicDivice;
        }
    }
    if(VK_NULL_HANDLE == selectedPhysicDevice){
        LOGE<<"selected null";
        return -1;
    }
    //检测selected physic device的队列族
    //获取队列族
    uint32_t physicDeviceQueueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicDevice,&physicDeviceQueueFamiliesCount,nullptr);
    vector<VkQueueFamilyProperties> physicDeviceQueueFamilies(physicDeviceQueueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicDevice,&physicDeviceQueueFamiliesCount,physicDeviceQueueFamilies.data());
    //检测
    bool queueFamilyIsOK = false;//是否同时支持渲染和呈现
    VkBool32 surfaceIsSuppoted = VK_FALSE;//是否支持呈现
    //队列族索引
    uint32_t physicDeviceQueueFamiliesIndex = 0;
    while(physicDeviceQueueFamiliesIndex < physicDeviceQueueFamiliesCount){
        vkGetPhysicalDeviceSurfaceSupportKHR(selectedPhysicDevice,physicDeviceQueueFamiliesIndex,surface,&surfaceIsSuppoted);
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
    vector<uint32_t> queueFamilies;
    //第1个队列族索引(共1个)
    queueFamilies.push_back(physicDeviceQueueFamiliesIndex);
    const uint32_t queueFamiliesCount{static_cast<uint32_t>(queueFamilies.size())};

    //创建逻辑设备
    //填写deviceCreateInfo,
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount=queueFamiliesCount;
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
    myVkResult = vkCreateDevice(selectedPhysicDevice,&deviceCreateInfo,nullptr,&device);
    if(VK_SUCCESS!= myVkResult){
        LOGE<<"vkCreateDevice--" << VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"create device";
    DeviceCleaner deviceCleaner{&device};

    //查询surface capability
    VkSurfaceCapabilitiesKHR surfaceCapability;
    myVkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(selectedPhysicDevice,surface,&surfaceCapability);
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
    swapchainCreateInfo.queueFamilyIndexCount=queueFamiliesCount;
    swapchainCreateInfo.pQueueFamilyIndices=queueFamilies.data();
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
    

    return 0;
}