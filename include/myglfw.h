#ifndef _MYGLFW_H_
#define _MYGLFW_H_
#include <GLFW/glfw3.h>
#include <plog/Log.h>
#include "plog/Initializers/RollingFileInitializer.h"

class MyGLFW
{
public:
    MyGLFW(){
    }
    ~MyGLFW(){
        if(glfwNeedTerminate){
            glfwTerminate();
            LOGI<< "glfwTerminate";
        }
    }
    void setglfwNeedTerminate(){
        glfwNeedTerminate = true;
    }
    bool init(){
        if(!glfwInit()){
            LOGE<< "glfwInit";
            return false;
        }
        setglfwNeedTerminate();
        return true;
    }
private:
    bool glfwNeedTerminate = false;
};

#endif