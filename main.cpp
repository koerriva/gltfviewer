
#include <iostream>
#include <window.h>
#include <renderer.h>
//#include <thread>

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
    auto* models = (model_t*)calloc(4,size);

//    Assets::LoadAnimateModel("data/animate_triangle.gltf",models);
//    model_t* tri = models;
//    tri->transform.scale = vec3(2);
//    tri->transform.rotation = vec3(radians(0.f),radians(0.f),radians(0.f));
//    tri->transform.position += vec3(-1,0,-2);
//    tri->animator = new Animator(tri);
//
//    Assets::LoadAnimateModel("data/AnimatedCube/AnimatedCube.gltf",models+1);
//    model_t* cube = models+1;
//    cube->transform.position = vec3(2);
//    cube->transform.rotation = cube->transform.rotation * quat(vec3(radians(10.0f),0.0f,radians(10.0f)));
//    cube->animator = new Animator(cube);
//
    Assets::LoadAnimateModel("data/BoxAnimated.gltf",models+1);
    model_t* arm = models+1;
    arm->transform.position += vec3(-1,1,0);
    arm->transform.scale = vec3(0.5);
    arm->animator = new Animator(arm);

    Assets::LoadAnimateModel("data/Snake.gltf",models);
    model_t* snake = models;
    snake->transform.position += vec3(1,-1,0);
    snake->transform.scale = vec3(0.5);
    snake->animator = new Animator(snake);

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
//                if(cube->animator){
//                    ((Animator*)cube->animator)->Play();
//                }
//                if(tri->animator){
//                    ((Animator*)tri->animator)->Play();
//                }
                if(arm->animator){
                    ((Animator*)arm->animator)->Play();
                }
                if(snake->animator){
                    ((Animator*)snake->animator)->Play("Snake_Idle");
                }
            } else{
//                if(cube->animator){
//                    ((Animator*)cube->animator)->Pause();
//                }
//                if(tri->animator){
//                    ((Animator*)tri->animator)->Pause();
//                }
                if(arm->animator){
                    ((Animator*)arm->animator)->Pause();
                }
                if(snake->animator){
                    ((Animator*)snake->animator)->Pause();
                }
            }
        }
        if(window->GetKeyPressed(KEY_S)){
            play_animate = false;
//            if(cube->animator){
//                ((Animator*)cube->animator)->Stop();
//            }
//            if(tri->animator){
//                ((Animator*)tri->animator)->Stop();
//            }
            if(arm->animator){
                ((Animator*)arm->animator)->Stop();
            }
            if(snake->animator){
                ((Animator*)snake->animator)->Stop();
            }
        }

        renderer->Render(camera,models,2);
        window->Update();

//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(16ms);
    }

    free(models);
    delete window;
    return 0;
}
