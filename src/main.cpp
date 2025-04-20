#include<iostream>
#include<vulkan/vulkan.h>
#include"utilities.h"
#include"loguru.hpp"

int main(){
    //设置日志文件
    loguru::add_file("C:\\onedrive\\project\\learningVulkan\\mylog", loguru::Append, loguru::Verbosity_MAX);
    LOG_F(INFO, "Hello log file!");
    //添加验证层
    

    //api version 1.1.95
    /* VkApplicationInfo appinfo{};
    appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appinfo.apiVersion = VK_API_VERSION_1_1;
    */
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    checkValidationLayarSupport("");


    std::cout << "hello world!" << std::endl;
    return 0; 
}