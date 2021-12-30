
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

    int size = sizeof(model_t);
    model_t* snake = (model_t*)calloc(1,size);
    Assets::LoadAnimateModel("data/animate_triangle.gltf",snake);
    Animator* animator = new Animator(snake);

    render_mode mode;
    bool play_animate = false;
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
        if(window->GetKeyPressed(KEY_P)){
            play_animate = !play_animate;
            if(play_animate){
                animator->Play();
            } else{
                animator->Stop();
            }
        }

        animator->Update(1.0f/60.f);
        renderer->Render(camera,snake);
        window->Update();

//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(16ms);
    }

    free(snake);
    delete window;
    return 0;
}
