
#include <iostream>
#include <window.h>
#include <renderer.h>
#include <thread>

struct GameState{
    bool running = true;
};

int main(int argc,char** argv) {
    GameState state;

    auto* window = new Window(1280,720,"GLTF2.0 Viewer");
    window->OnClose(&state.running);

    auto* renderer = new Renderer();

    auto* camera = new Camera();
    model snake;
    Renderer::LoadAnimateModel("data/animate_triangle.gltf",&snake);

    render_mode mode;
    while (state.running){
        if(window->GetKeyPressed(KEY_F1)){
            std::cout << "Change Render Mode" << std::endl;
            if (mode==shader){
                mode = wireframe;
            } else{
                mode = shader;
            }
            renderer->SetRenderMode(mode);
        }
        renderer->Render(camera,&snake);
        window->Update();

//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(16ms);
    }

    delete window;
    return 0;
}
