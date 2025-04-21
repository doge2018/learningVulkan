#ifndef _CLEANERS_H_
#define _CLEANERS_H_
#include<vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"

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
#endif