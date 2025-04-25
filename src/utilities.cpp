#include"utilities.h"
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

string VkResultToString(VkResult result){
    switch (result) {
        case VK_SUCCESS:
            return "VK_SUCCESS";
        case VK_NOT_READY:
            return "VK_NOT_READY";
        case VK_TIMEOUT:
            return "VK_TIMEOUT";
        case VK_EVENT_SET:
            return "VK_EVENT_SET";
        case VK_EVENT_RESET:
            return "VK_EVENT_RESET";
        case VK_INCOMPLETE:
            return "VK_INCOMPLETE";
        case VK_ERROR_OUT_OF_HOST_MEMORY:
            return "VK_ERROR_OUT_OF_HOST_MEMORY";
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
            return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
        case VK_ERROR_INITIALIZATION_FAILED:
            return "VK_ERROR_INITIALIZATION_FAILED";
        case VK_ERROR_DEVICE_LOST:
            return "VK_ERROR_DEVICE_LOST";
        case VK_ERROR_MEMORY_MAP_FAILED:
            return "VK_ERROR_MEMORY_MAP_FAILED";
        case VK_ERROR_LAYER_NOT_PRESENT:
            return "VK_ERROR_LAYER_NOT_PRESENT";
        case VK_ERROR_EXTENSION_NOT_PRESENT:
            return "VK_ERROR_EXTENSION_NOT_PRESENT";
        case VK_ERROR_FEATURE_NOT_PRESENT:
            return "VK_ERROR_FEATURE_NOT_PRESENT";
        case VK_ERROR_INCOMPATIBLE_DRIVER:
            return "VK_ERROR_INCOMPATIBLE_DRIVER";
        case VK_ERROR_TOO_MANY_OBJECTS:
            return "VK_ERROR_TOO_MANY_OBJECTS";
        case VK_ERROR_FORMAT_NOT_SUPPORTED:
            return "VK_ERROR_FORMAT_NOT_SUPPORTED";
        case VK_ERROR_FRAGMENTED_POOL:
            return "VK_ERROR_FRAGMENTED_POOL";
        case VK_ERROR_UNKNOWN:
            return "VK_ERROR_UNKNOWN";
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            return "VK_ERROR_OUT_OF_POOL_MEMORY";
        case VK_ERROR_INVALID_EXTERNAL_HANDLE:
            return "VK_ERROR_INVALID_EXTERNAL_HANDLE";
        case VK_ERROR_FRAGMENTATION:
            return "VK_ERROR_FRAGMENTATION";
        case VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:
            return "VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS";
        case VK_ERROR_SURFACE_LOST_KHR:
            return "VK_ERROR_SURFACE_LOST_KHR";
        case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
            return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
        case VK_SUBOPTIMAL_KHR:
            return "VK_SUBOPTIMAL_KHR";
        case VK_ERROR_OUT_OF_DATE_KHR:
            return "VK_ERROR_OUT_OF_DATE_KHR";
        case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
            return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
        case VK_ERROR_VALIDATION_FAILED_EXT:
            return "VK_ERROR_VALIDATION_FAILED_EXT";
        case VK_ERROR_INVALID_SHADER_NV:
            return "VK_ERROR_INVALID_SHADER_NV";
        case VK_ERROR_NOT_PERMITTED_KHR:
            return "VK_ERROR_NOT_PERMITTED_KHR";
        case VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:
            return "VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT";
        case VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:
            return "VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT";
        case VK_ERROR_COMPRESSION_EXHAUSTED_EXT:
            return "VK_ERROR_COMPRESSION_EXHAUSTED_EXT";
        default:
            return "UNKNOWN_VK_RESULT";
    }
}

bool getPhysicalDeviceSurfaceFormat(VkPhysicalDevice *physicalDevice,VkSurfaceKHR *surface,std::vector<VkSurfaceFormatKHR> &formats){
    uint32_t formatsCount = 0;
    auto res = vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice,*surface,&formatsCount,nullptr);
    if(VK_SUCCESS != res){
        LOGE<<"1st vkGetPhysicalDeviceSurfaceFormatsKHR--"<<VkResultToString(res);
        return false; 
    }
    formats.resize(formatsCount);
    res = vkGetPhysicalDeviceSurfaceFormatsKHR(*physicalDevice,*surface,&formatsCount,formats.data());
    if(VK_SUCCESS != res){
        LOGE<<"2nd vkGetPhysicalDeviceSurfaceFormatsKHR--"<<VkResultToString(res);
        return false; 
    }
    for(auto format:formats){
        LOGI<<"format--"<<format.format<< "color space--"<<format.colorSpace;
    }
    return true;
}





