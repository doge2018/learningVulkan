#include<iostream>
#include<vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include"utilities.h"
#include"cleaners.h"
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
using namespace std;

//标示资源的宏


int main(){
    //日志初始化
    plog::init(plog::verbose, "C:\\onedrive\\project\\learningVulkan\\log.txt",51200,1);
    LOGI << "--enter main--";

    //init glfw
    if(!glfwInit()){
        LOGE<<"glfwInit";
    }
    GlfwCleaner glfwCleaner;
    //检查glfw vulkan支持
    if(!glfwVulkanSupported()){
        LOGE<<"glfwVulkanSupported";
        return -1;
    }
    //获取glfw所需扩展
    uint32_t glfwExtCount = 0;
    const char **glfwExts = glfwGetRequiredInstanceExtensions(&glfwExtCount);

    //填写VkInstanceCreateInfo结构
    VkInstanceCreateInfo vkInstanceInfo{};
    vkInstanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    //填写所需扩展层
    vkInstanceInfo.enabledExtensionCount = glfwExtCount;
    vkInstanceInfo.ppEnabledExtensionNames = glfwExts;
    //填写所需验证层
    vector<const char *> vValidations;
    const char *layerName = "VK_LAYER_KHRONOS_validation";
    vValidations.push_back(layerName);
    //验证层数量
    const uint32_t layerCount{static_cast<uint32_t>(vValidations.size())};
    vkInstanceInfo.enabledLayerCount = layerCount;
    vkInstanceInfo.ppEnabledLayerNames = vValidations.data();
    
    //日志记录
    LOGI<<"ppEnabledExtensionNames:";
    for(uint32_t i = 0;i<glfwExtCount;++i){
        LOGI<<glfwExts[i];
    }
    LOGI<<"ppEnabledLayerNames:";
    for(auto layer:vValidations){
        LOGI<<layer;
    }

    //创建instance
    VkInstance instance;
    if(VK_SUCCESS != vkCreateInstance(&vkInstanceInfo,nullptr,&instance)){
        LOGE<<"vkCreateInstance";
        return -1;
    }
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
    WindowCleaner windowCleaner{pwindow};
    
    //创建surface
    VkSurfaceKHR surface;
    auto result = glfwCreateWindowSurface(instance,pwindow,nullptr,&surface);
    if(VK_SUCCESS != result){
        LOGE<<"glfwCreateWindowSurface";
        switch (result) {
            case VK_ERROR_OUT_OF_HOST_MEMORY: LOGE << "VK_ERROR_OUT_OF_HOST_MEMORY";break;
            case VK_ERROR_OUT_OF_DEVICE_MEMORY: LOGE << "VK_ERROR_OUT_OF_DEVICE_MEMORY";break;
            case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: LOGE << "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";break;
            case VK_ERROR_SURFACE_LOST_KHR: LOGE << "VK_ERROR_SURFACE_LOST_KHR";break;
            default: LOGE << "Unknown VkResult error";break;
        }    
        return -1;
    }
    SurfaceCleaner surfaceCleaner{&instance,&surface};

    //选择物理设备
    //枚举物理设备
    uint32_t physicDeviceCount = 0;
    if(VK_SUCCESS != vkEnumeratePhysicalDevices(instance,&physicDeviceCount,nullptr)){
        LOGE<<"vkEnumeratePhysicalDevices";
        return -1;
    }
    vector<VkPhysicalDevice> vphysicDivices(physicDeviceCount);
    if(VK_SUCCESS != vkEnumeratePhysicalDevices(instance,&physicDeviceCount,vphysicDivices.data())){
        LOGE<<"vkEnumeratePhysicalDevices";
        return -1;
    }
    VkPhysicalDevice selected = VK_NULL_HANDLE;
    for(auto physicDivice:vphysicDivices){
        VkPhysicalDeviceProperties properties;
        vkGetPhysicalDeviceProperties(physicDivice,&properties);
        if(VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU == properties.deviceType){
            selected = physicDivice;
        }
    }
    if(VK_NULL_HANDLE == selected){
        LOGE<<"selected null";
    }
    //检测物理设备的队列族是否支持渲染、以及呈现到surface
    //获取队列族
    uint32_t queueFamiliesCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(selected,&queueFamiliesCount,nullptr);
    vector<VkQueueFamilyProperties> vqueueFamilies(queueFamiliesCount);
    vkGetPhysicalDeviceQueueFamilyProperties(selected,&queueFamiliesCount,vqueueFamilies.data());
    //检测
    VkBool32 surfaceIsSuppoted = VK_FALSE;
    bool queueFamilyIsOK = false;
    //队列族索引
    uint32_t queueFamiliesIndex = 0;
    while(queueFamiliesIndex < queueFamiliesCount){
        vkGetPhysicalDeviceSurfaceSupportKHR(selected,queueFamiliesIndex,surface,&surfaceIsSuppoted);
        //存在一个队列族同时支持渲染和呈现到给定surface
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

    //创建逻辑设备


    return 0;
}