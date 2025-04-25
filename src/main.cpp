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
VkFormat imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
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

    //验证层(共一个)
    vector<const char *> layers;
    const char *layer = "VK_LAYER_KHRONOS_validation";
    layers.push_back(layer);
    //需启用的验证层数量
    const uint32_t layersCount{static_cast<uint32_t>(layers.size())};

    //填写VkInstanceCreateInfo结构
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
    LOGI<<"physicDeviceCount--"<<physicDeviceCount;
    for(auto physicDivice:physicDivices){
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
    //队列族（索引）
    vector<uint32_t> queueFamilies;
    queueFamilies.push_back(physicDeviceQueueFamiliesIndex);
    //队列族数量：1
    const uint32_t queueFamiliesCount = {static_cast<uint32_t>(queueFamilies.size())};

    //创建逻辑设备
    //填写deviceCreateInfo,
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount=queueFamiliesCount;
    //队列族Infos
    vector<VkDeviceQueueCreateInfo> queueFamilyInfos;
    //第一个队列族的Info(共一个)
    VkDeviceQueueCreateInfo queueFamilyInfo{};
    //填写队列族信息
    queueFamilyInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamilyInfo.queueFamilyIndex=physicDeviceQueueFamiliesIndex;//此队列族的索引
    queueFamilyInfo.queueCount=queueCount;
    //队列优先级（此队列族只开启一个队列）
    vector<float> queuePriorities(queueCount);
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
    const uint32_t deviceExtsCount = {static_cast<uint32_t>(deviceExts.size())};
    deviceCreateInfo.enabledExtensionCount=deviceExtsCount;
    deviceCreateInfo.ppEnabledExtensionNames=deviceExts.data();
    //创建
    VkDevice device;
    myVkResult = vkCreateDevice(selectedPhysicDevice,&deviceCreateInfo,nullptr,&device);
    if(VK_SUCCESS!= myVkResult){
        LOGE<<"vkCreateDevice--" << VkResultToString(myVkResult);
        return -1;
    }
    LOGI<<"vkCreateDevice";
    DeviceCleaner deviceCleaner{&device};

    vector<VkSurfaceFormatKHR> surfaceFormats;
    getPhysicalDeviceSurfaceFormat(&selectedPhysicDevice,&surface,surfaceFormats);

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
    swapchainCreateInfo.pNext=nullptr;
    swapchainCreateInfo.surface=surface;
    swapchainCreateInfo.minImageCount=bufferCount;
    swapchainCreateInfo.imageFormat=VK_FORMAT_B8G8R8A8_SRGB;
    swapchainCreateInfo.imageColorSpace=VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    //是固定分辨率
    if(0xFFFFFFFF!=surfaceCapability.currentExtent.width){
        swapchainCreateInfo.imageExtent=surfaceCapability.currentExtent;
    }
    else{
        LOGE<<"Extent not set!";
        return -1;
    }
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
    SwapchainCleaner swapchainCleaner(&device,&swapchain);

    return 0;
}