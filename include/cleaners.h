#ifndef _CLEANERS_H_
#define _CLEANERS_H_
#include<vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"
//需要的话可支持cleaner的移动
//需要的话可支持空指针检测
//可使用explicit关键字禁止隐式转换

struct GlfwCleaner{
    ~GlfwCleaner(){
        glfwTerminate();
        LOGI<<"glfwTerminate";
    }
private:
};

struct WindowCleaner{
    WindowCleaner(GLFWwindow *pwindow):_pwindow(pwindow){
    }
    ~WindowCleaner(){
        glfwDestroyWindow(_pwindow);
        LOGI<<"glfwDestroyWindow";
    }
private:
    GLFWwindow *_pwindow;
};

struct InstanceCleaner{
    InstanceCleaner(VkInstance *pinstance):_pinstance(pinstance){
    }
    ~InstanceCleaner(){
        vkDestroyInstance(*_pinstance,nullptr);
        LOGI<<"vkDestroyInstance";
    }
private:
    VkInstance *_pinstance;
};

struct SurfaceCleaner{
    SurfaceCleaner(VkInstance *pinstance,VkSurfaceKHR *psurface):_pinstance(pinstance),_psurface(psurface){
    }
    ~SurfaceCleaner(){
        vkDestroySurfaceKHR(*_pinstance,*_psurface,nullptr);
        LOGI<<"vkDestroySurfaceKHR";
    }
private:
    VkInstance *_pinstance;
    VkSurfaceKHR *_psurface;
};

struct DeviceCleaner{
    DeviceCleaner(VkDevice *pdevice):_pdevice(pdevice){
    }
    ~DeviceCleaner(){
        vkDestroyDevice(*_pdevice,nullptr);
        LOGI<<"vkDestroyDevice";
    }
private:
    VkDevice *_pdevice;
};

struct SwapchainCleaner{
    SwapchainCleaner(VkDevice *pdevice,VkSwapchainKHR *pswapchain):_pdevice(pdevice),_pswapchain(pswapchain){
    }
    ~SwapchainCleaner(){
        vkDestroySwapchainKHR(*_pdevice,*_pswapchain,nullptr);
        LOGI<<"vkDestroySwapchainKHR";
    }
private:
    VkDevice *_pdevice;
    VkSwapchainKHR *_pswapchain;
};

struct RenderpassCleaner{
    RenderpassCleaner(VkDevice *pdevice,VkRenderPass *prenderpass):_pdevice(pdevice),_prenderpass(prenderpass){
    }
    ~RenderpassCleaner(){
        vkDestroyRenderPass(*_pdevice,*_prenderpass,nullptr);
        LOGI<<"vkDestroyRenderPass";
    }
private:
    VkDevice *_pdevice;
    VkRenderPass *_prenderpass;
};
#endif