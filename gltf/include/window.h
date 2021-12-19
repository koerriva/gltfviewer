//
// Created by koerriva on 2021/12/19.
//

#ifndef GLTFVIEWER_WINDOW_H
#define GLTFVIEWER_WINDOW_H

#include <glad/glad.h>
#include "GLFW/glfw3.h"

typedef void(*window_callback)(int signal);

enum key {
    KEY_F1=290,KEY_F2,KEY_F3,KEY_F4,KEY_F5,
    KEY_A=65,
    KEY_B,KEY_C,KEY_D,KEY_E,KEY_F,KEY_G,KEY_H,KEY_I,KEY_J,KEY_K,KEY_L,KEY_M,KEY_N,KEY_O,KEY_P,KEY_Q,KEY_R,KEY_S,KEY_T,
    KEY_U,KEY_V,KEY_W,KEY_X,KEY_Y,KEY_Z
};

class Window{
public:
    Window(int width,int height,char *title);
    ~Window();

    void Update();

    void OnClose(bool* signal);

    bool GetKeyPressed(key key);
    bool GetKeyDown(key key);
    bool GetKeyUp(key key);

private:
    GLFWwindow* m_handle;
    bool* _on_exit;

    int key_last_state[GLFW_KEY_LAST+1] = {0};
    int key_curr_state[GLFW_KEY_LAST+1] = {0};
};
#endif //GLTFVIEWER_WINDOW_H
