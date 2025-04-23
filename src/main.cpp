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

    //需要开启的扩展
    //获取glfw所需扩展
    uint32_t glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);
    //需要开启的所有扩展的名字
    vector<const char*> extNames(glfwExts, glfwExts+glfwExtCount);
    //需要开启的扩展数量
    uint32_t extCount{static_cast<uint32_t>(extNames.size())};

    //填写VkInstanceCreateInfo结构
    VkInstanceCreateInfo vkInstanceInfo{};
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    //填写所需扩展
    vkInstanceInfo.enabledExtensionCount = extCount;
    vkInstanceInfo.ppEnabledExtensionNames = extNames.data();
    //填写所需验证层
    //要启用的所有验证层的名称
    vector<const char *> layerNames;
    const char *layerName = "VK_LAYER_KHRONOS_validation";
    layerNames.push_back(layerName);
    //验证层数量
    const uint32_t layerCount{static_cast<uint32_t>(layerNames.size())};
    vkInstanceInfo.enabledLayerCount = layerCount;
    vkInstanceInfo.ppEnabledLayerNames = layerNames.data();
    
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
    VkPhysicalDevice selectedPhysicDevice = VK_NULL_HANDLE;
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
    //检测物理设备的队列族是否可同时支持渲染、以及呈现到surface
    //获取队列族
    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicDevice,&queueFamiliesCount,nullptr);
    vector<VkQueueFamilyProperties> vqueueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(selectedPhysicDevice,&queueFamiliesCount,vqueueFamilies.data());
    //检测
    bool queueFamilyIsOK = false;//队列族是否同时支持渲染和呈现
    VkBool32 surfaceIsSuppoted = VK_FALSE;//是否支持呈现
    //队列族索引
    uint32_t queueFamiliesIndex = 0;
    while(queueFamiliesIndex < queueFamiliesCount){
        vkGetPhysicalDeviceSurfaceSupportKHR(selectedPhysicDevice,queueFamiliesIndex,surface,&surfaceIsSuppoted);
        if(vqueueFamilies[queueFamiliesIndex].queueFlags & VK_QUEUE_GRAPHICS_BIT && surfaceIsSuppoted){
            queueFamilyIsOK = true;
            break;
        }
        ++queueFamiliesIndex;
    }
    if(!queueFamilyIsOK){
        LOGE<<"queueFamilyIsOK";
        return -1;
    }

    //查询物理设备支持的扩展
    uint32_t extPropertiesCount = 0;
    if(VK_SUCCESS != vkEnumerateDeviceExtensionProperties(selectedPhysicDevice,nullptr,&extPropertiesCount,nullptr)){
        LOGE<<"1st vkEnumerateDeviceExtensionProperties";
        return -1;
    }
    vector<VkExtensionProperties> extProperties(extPropertiesCount);
    if(VK_SUCCESS != vkEnumerateDeviceExtensionProperties(selectedPhysicDevice,nullptr,&extPropertiesCount,extProperties.data())){
        LOGE<<"2nd vkEnumerateDeviceExtensionProperties";
        return -1;
    }
    for(const auto& extProperty:extProperties){
        LOGI<<extProperty.extensionName;
    }


    //创建逻辑设备
    //队列族信息
    vector<VkDeviceQueueCreateInfo> deviceQueueCreateInfos;
    //队列优先级
    vector<float> queuePriorities(queueCount);
    queuePriorities[0]= 1.0f;
    VkDeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceQueueCreateInfo.queueFamilyIndex=queueFamiliesIndex;
    deviceQueueCreateInfo.queueCount=queueCount;
    deviceQueueCreateInfo.pQueuePriorities=queuePriorities.data();
    deviceQueueCreateInfos.push_back(deviceQueueCreateInfo);
    //队列族数量
    const uint32_t queueFamilyCount = {static_cast<uint32_t>(deviceQueueCreateInfos.size())};
    //填写deviceCreateInfo,
    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType=VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.queueCreateInfoCount=queueFamilyCount;
    deviceCreateInfo.pQueueCreateInfos=deviceQueueCreateInfos.data();
    deviceCreateInfo.enabledLayerCount=layerCount;
    deviceCreateInfo.ppEnabledLayerNames=layerNames.data();
    deviceCreateInfo.enabledExtensionCount=extCount;
    deviceCreateInfo.ppEnabledExtensionNames=extNames.data();
    //创建逻辑设备
    VkDevice device;
    vkResult = vkCreateDevice(selectedPhysicDevice,&deviceCreateInfo,nullptr,&device);
    if(VK_SUCCESS!= vkResult){
        LOGE<<"vkCreateDevice--" << VkResultToString(vkResult);
        LOGE<<"extCount--"<<extCount;
        for(const auto &name:extNames){
            LOGE<<name;
        }

        return -1;
    }
    LOGI<<"vkCreateDevice";
    DeviceCleaner deviceCleaner{&device};

    return 0;
}