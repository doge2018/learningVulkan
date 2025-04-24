#ifndef _UTILITIES_H_
#define _UTILITIES_H_
#include<string>
#include<vulkan/vulkan.h>

bool checkValidationLayarSupport(const std::string &desiredLayerName);
bool enumerateExtentionsSurppoted();
std::string VkResultToString(VkResult result);


#endif