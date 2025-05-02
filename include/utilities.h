#ifndef _UTILITIES_H_
#define _UTILITIES_H_
#include<vector>
#include<string>
#include<vulkan/vulkan.h>

std::string VkResultToString(VkResult result);
bool checkValidationLayarSupport(const std::string &desiredLayerName);
bool enumerateExtentionsSurppoted();
//获取物理设备支持的image formats
bool getPhysicalDeviceSurfaceFormat(
    VkPhysicalDevice *physicalDevice,
    VkSurfaceKHR *surface,
    std::vector<VkSurfaceFormatKHR> &formats);
//read a spv file
std::vector<char> readSPVFile(const std::string &filename);

#endif