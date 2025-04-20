#include"utilities.h"
#include<vulkan/vulkan.h>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
using namespace std;
//检查是否支持给定的验证层
bool checkValidationLayarSupport(const string &desiredLayerName){
    uint32_t count = 0;
    if(VK_SUCCESS != vkEnumerateInstanceLayerProperties(&count, nullptr)){
        LOGE << "1st vkEnumerateInstanceLayerProperties";
        return false;
    }
    LOGI << "count:" << count;
    std::vector<VkLayerProperties> layers(count);
    if(VK_SUCCESS != vkEnumerateInstanceLayerProperties(&count, layers.data())){
        LOGE << "2st vkEnumerateInstanceLayerProperties";
        return false;
    }
    for(const auto &layer:layers){
        if(0 == strcmp(layer.layerName,desiredLayerName.c_str())){
            return true;
        }
    }
    return false;
}
//列出支持的所有扩展层
bool enumerateExtentionsSurppoted(){
    uint32_t count = 0;
    if(VK_SUCCESS != vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr)){
        LOGE << "1st vkEnumerateInstanceExtensionProperties";
        return false;
    }
    LOGI << "count:" << count;
    std::vector<VkExtensionProperties> extensions(count);
    if(VK_SUCCESS != vkEnumerateInstanceExtensionProperties(nullptr, &count, extensions.data())){
        LOGE << "2st vkEnumerateInstanceExtensionProperties";
        return false;
    }
    for(auto &ext:extensions){
        LOGI << ext.extensionName;
    }
    return true;
}