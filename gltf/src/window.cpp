//
// Created by koerriva on 2021/12/19.
//

#include <iostream>
#include "window.h"

Window::Window(int width, int height, char *title) {
    glfwInit();
    glfwWindowHint(GLFW_SCALE_TO_MONITOR,GLFW_TRUE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES,4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    m_handle = glfwCreateWindow(width,height,title, nullptr, nullptr);
    glfwSwapInterval(1);
    glfwMakeContextCurrent(m_handle);

    int r = gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    if(!r){
        std::cerr << "glad loader fail" << std::endl;
    }
}

Window::~Window() {
    std::cout << "Destroy Window" << std::endl;
    glfwDestroyWindow(m_handle);
}

void Window::Update() {
    glfwPollEvents();

    for (int i = 0; i < GLFW_KEY_LAST + 1; ++i) {
        key_last_state[i] = key_curr_state[i];
        key_curr_state[i] = glfwGetKey(m_handle,i);
    }

    glfwSwapBuffers(m_handle);

    if(glfwWindowShouldClose(m_handle)){
        *_on_exit = false;
    }
}

void Window::OnClose(bool* signal) {
    _on_exit = signal;
}

bool Window::GetKeyDown(key key) {
    int last_state = key_last_state[key];
    int curr_state = key_curr_state[key];
    return last_state==curr_state&&curr_state==GLFW_PRESS;
}

bool Window::GetKeyUp(key key) {
    int last_state = key_last_state[key];
    int curr_state = key_curr_state[key];
    return last_state==curr_state&&curr_state==GLFW_RELEASE;
}

bool Window::GetKeyPressed(key key) {
    int last_state = key_last_state[key];
    int curr_state = glfwGetKey(m_handle,key);
    return last_state!=curr_state&&curr_state==GLFW_PRESS;
}

