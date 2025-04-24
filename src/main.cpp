#include<iostream>
#include<vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include"utilities.h"
#include"cleaners.h"
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
using namespace std;

const uint32_t queueCount = 1;
VkResult vkResult;

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
    if(VK_SUCCESS != vkCreateInstance(&vkInstanceInfo,nullptr,&instance)){
        LOGE<<"vkCreateInstance";
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
    vkResult = glfwCreateWindowSurface(instance,pwindow,nullptr,&surface);
    if(VK_SUCCESS != vkResult){
        LOGE<<"glfwCreateWindowSurface";
        switch (vkResult) {
            case VK_ERROR_OUT_OF_HOST_MEMORY: LOGE << "VK_ERROR_OUT_OF_HOST_MEMORY";break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: LOGE << "VK_ERROR_OUT_OF_DEVICE_MEMORY";break;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: LOGE << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";break;
            case VK_ERROR_SURFACE_LOST_KHR: LOGE << "VK_ERROR_SURFACE_LOST_KHR";break;
            default: LOGE << "Unknown VkResult error";break;
        }    
        return -1;
    }
    LOGI<<"create surface";
    SurfaceCleaner surfaceCleaner{&instance,&surface};

    //选择物理设备
    VkPhysicalDevice selectedPhysicDevice = VK_NULL_HANDLE;
    //枚举物理设备
    uint32_t physicDeviceCount = 0;
    if(VK_SUCCESS != vkEnumeratePhysicalDevices(instance,&physicDeviceCount,nullptr)){
        LOGE<<"vkEnumeratePhysicalDevices";
        return -1;
    }
    vector<VkPhysicalDevice> physicDivices(physicDeviceCount);
    if(VK_SUCCESS != vkEnumeratePhysicalDevices(instance,&physicDeviceCount,physicDivices.data())){
        LOGE<<"vkEnumeratePhysicalDevices";
        return -1;
    }
    LOGI<<"physicDeviceCount--"<<physicDeviceCount;
    for(auto physicDivice:physicDivices){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicDivice,&properties);
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

    //创建逻辑设备
    //队列族信息，一个队列族对应一个VkDeviceQueueCreateInfo结构
    vector<VkDeviceQueueCreateInfo> queueFamilies;
    //第一个队列族(共一个)
    VkDeviceQueueCreateInfo queueFamily{};
    //队列优先级，此队列族开启一个队列
    vector<float> queuePriorities(queueCount);
    queuePriorities[0]= 1.0f;
    //填写队列族信息
    queueFamily.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueFamily.queueFamilyIndex=physicDeviceQueueFamiliesIndex;//此队列族的索引
    queueFamily.queueCount=queueCount;
    queueFamily.pQueuePriorities=queuePriorities.data();
    queueFamilies.push_back(queueFamily);
    //队列族数量
    const uint32_t queueFamiliesCount = {static_cast<uint32_t>(queueFamilies.size())};
    //device extentions
    vector<const char *> deviceExts;
    const char *deviceExt = "VK_KHR_swapchain";
    deviceExts.push_back(deviceExt);
    //device extentions count
    const uint32_t deviceExtsCount = {static_cast<uint32_t>(deviceExts.size())};
    //填写deviceCreateInfo,
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount=queueFamiliesCount;
    deviceCreateInfo.pQueueCreateInfos=queueFamilies.data();
    deviceCreateInfo.enabledLayerCount=layersCount;
    deviceCreateInfo.ppEnabledLayerNames=layers.data();
    deviceCreateInfo.enabledExtensionCount=deviceExtsCount;
    deviceCreateInfo.ppEnabledExtensionNames=deviceExts.data();
    //创建逻辑设备
    VkDevice device;
    vkResult = vkCreateDevice(selectedPhysicDevice,&deviceCreateInfo,nullptr,&device);
    if(VK_SUCCESS!= vkResult){
        LOGE<<"vkCreateDevice--" << VkResultToString(vkResult);
        return -1;
    }
    LOGI<<"vkCreateDevice";
    DeviceCleaner deviceCleaner{&device};

    return 0;
}