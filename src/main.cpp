#include<iostream>
#include<vulkan/vulkan.h>
#include"utilities.h"
#include"myglfw.h"
#include <GLFW/glfw3.h>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
using namespace std;

int main(){
    //日志初始化
    plog::init(plog::verbose, "C:\\onedrive\\project\\learningVulkan\\log.txt",5120,1);
    LOGI << "enter main";

    //glfw初始化
    MyGLFW myglfw;
    if(!myglfw.init()){
        LOGE<<"myglfw.init";
    }
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

    //创建instance
    VkInstance myVkInstance;
    if(VK_SUCCESS != vkCreateInstance(&vkInstanceInfo,nullptr,&myVkInstance)){
        LOGE<<"vkCreateInstance";
    }
    //销毁instance
    vkDestroyInstance(myVkInstance,nullptr);
    LOGI<<"main return";
    return 0;
}