#include<vulkan/vulkan.h>
#include"utilities.h"
#include"loguru.hpp"
using namespace std;

bool checkValidationLayarSupport(const string &layerName){
    uint32_t layerCount = 0;
    if(VK_SUCCESS != vkEnumerateInstanceLayerProperties(&layerCount, nullptr)){
        //LOG_S(ERROR) << "1st vkEnumerateInstanceLayerProperties";
    }
    //LOG_S(INFO) << "layerCount:" << layerCount;
    return true;

}