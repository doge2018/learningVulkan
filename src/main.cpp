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
    LOGI << "enter main";

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
    vkInstanceInfo.enabledLayerCount = static_cast<uint32_t>(vValidations.size());
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

    return 0;
}