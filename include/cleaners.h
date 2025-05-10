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

struct FramebuffersCleaner{
    FramebuffersCleaner(VkDevice *pdevice, std::vector<VkFramebuffer> &framebuffers):_pdevice(pdevice),_framebuffers(framebuffers){
    }
    ~FramebuffersCleaner(){
        for(auto &framebuffer:_framebuffers){
            vkDestroyFramebuffer(*_pdevice,framebuffer,nullptr);
        }
        _framebuffers.clear();
        LOGI<<"vkDestroyFramebuffer";
    }
private:
    VkDevice *_pdevice;
    std::vector<VkFramebuffer> _framebuffers;
};

struct ImageviewsCleaner{
    ImageviewsCleaner(VkDevice *pdevice, std::vector<VkImageView> &imageviews):_pdevice(pdevice),_imageviews(imageviews){
    }
    ~ImageviewsCleaner(){
        for(auto &imageview:_imageviews){
            vkDestroyImageView(*_pdevice,imageview,nullptr);
        }
        _imageviews.clear();
        LOGI<<"vkDestroyImageView";
    }
private:
    VkDevice *_pdevice;
    std::vector<VkImageView> _imageviews;
};

struct BufferCleaner{
    BufferCleaner(VkDevice *pdevice,VkBuffer *pbuffer):_pdevice(pdevice),_pbuffer(pbuffer){
    }
    ~BufferCleaner(){
        vkDestroyBuffer(*_pdevice,*_pbuffer,nullptr);
        LOGI<<"vkDestroyBuffer";
    }
private:
    VkDevice *_pdevice;
    VkBuffer *_pbuffer;
};

struct MemoryFreer{
    MemoryFreer(VkDevice *pdevice,VkDeviceMemory *pmemory):_pdevice(pdevice),_pmemory(pmemory){
    }
    ~MemoryFreer(){
        vkFreeMemory(*_pdevice,*_pmemory,nullptr);
        LOGI<<"vkFreeMemory";
    }
private:
    VkDevice *_pdevice;
    VkDeviceMemory *_pmemory;
};

struct ShaderModuleCleaner{
    ShaderModuleCleaner(VkDevice *pdevice,VkShaderModule *pshaderModule):_pdevice(pdevice),_pshaderModule(pshaderModule){
    }
    ~ShaderModuleCleaner(){
        vkDestroyShaderModule(*_pdevice,*_pshaderModule,nullptr);
        LOGI<<"vkDestroyShaderModule";
    }
private:
    VkDevice *_pdevice;
    VkShaderModule *_pshaderModule;
};

struct PipelineLayoutCleaner{
    PipelineLayoutCleaner(VkDevice *pdevice,VkPipelineLayout *ppipelineLayout):_pdevice(pdevice),_ppipelineLayout(ppipelineLayout){
    }
    ~PipelineLayoutCleaner(){
        vkDestroyPipelineLayout(*_pdevice,*_ppipelineLayout,nullptr);
        LOGI<<"vkDestroyPipelineLayout";
    }
private:
    VkDevice *_pdevice;
    VkPipelineLayout *_ppipelineLayout;
};

struct PipelineCleaner{
    PipelineCleaner(VkDevice *pdevice,VkPipeline *ppipeline):_pdevice(pdevice),_ppipeline(ppipeline){
    }
    ~PipelineCleaner(){
        vkDestroyPipeline(*_pdevice,*_ppipeline,nullptr);
        LOGI<<"vkDestroyPipeline";
    }
private:
    VkDevice *_pdevice;
    VkPipeline *_ppipeline;
};

struct CommandPoolCleaner{
    CommandPoolCleaner(VkDevice *pdevice,VkCommandPool *pcommandPool):_pdevice(pdevice),_pcommandPool(pcommandPool){
    }
    ~CommandPoolCleaner(){
        vkDestroyCommandPool(*_pdevice,*_pcommandPool,nullptr);
        LOGI<<"vkDestroyCommandPool";
    }
private:
    VkDevice *_pdevice;
    VkCommandPool *_pcommandPool;
};
struct FenceCleaner{
    FenceCleaner(VkDevice *pdevice,VkFence *pfence):_pdevice(pdevice),_pfence(pfence){
    }
    ~FenceCleaner(){
        vkDestroyFence(*_pdevice,*_pfence,nullptr);
        LOGI<<"vkDestroyFence";
    }
private:
    VkDevice *_pdevice;
    VkFence *_pfence;
};

struct SemaphoreCleaner{
    SemaphoreCleaner(VkDevice *pdevice,VkSemaphore *psemaphore):_pdevice(pdevice),_psemaphore(psemaphore){
    }
    ~SemaphoreCleaner(){
        vkDestroySemaphore(*_pdevice,*_psemaphore,nullptr);
        LOGI<<"vkDestroySemaphore";
    }
private:
    VkDevice *_pdevice;
    VkSemaphore *_psemaphore;
};




#endif