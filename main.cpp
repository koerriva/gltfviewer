
#include <iostream>
#include <window.h>
#include <renderer.h>

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
    Renderer::LoadModel("data/Snake.gltf",&snake);

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
    }

    delete window;
    return 0;
}
