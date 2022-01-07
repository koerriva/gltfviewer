
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

    std::cout << "Load AnimateShader ..." << std::endl;
    uint32_t animateShader = Shader::LoadAnimateShader();
    auto* scene = new scene_t;
    scene->shader = animateShader;
    scene->camera = new Camera();

    object_t * tri = Assets::LoadAnimateModel("data/animate_triangle.gltf",scene);
    tri->transform.scale = vec3(2);
    tri->transform.position += vec3(-1,1,-2);

    object_t * cube = Assets::LoadAnimateModel("data/AnimatedCube/AnimatedCube.gltf",scene);
    cube->transform.position = vec3(3,2,1);
    cube->transform.scale = vec3(0.5);
    cube->transform.rotation *= quat(vec3(radians(10.0f),0.0f,radians(-10.0f)));

    object_t* box = Assets::LoadAnimateModel("data/BoxAnimated.gltf",scene);
    box->transform.position = vec3(0,2,0);
    box->transform.scale = vec3(0.5);
    box->transform.rotation *= quat(vec3(0.0f,0.0f, radians(30.0f)));

    object_t* snake = Assets::LoadAnimateModel("data/Snake.gltf",scene);
    snake->transform.position = vec3(0,-1,0);
    snake->transform.scale = vec3(0.8);

    object_t* fox = Assets::LoadAnimateModel("data/Fox/Fox.gltf",scene);
    fox->transform.position = vec3(4,-1,1);
    fox->transform.scale = vec3(0.02);

    object_t* arm = Assets::LoadAnimateModel("data/arm_skin.gltf",scene);
    arm->transform.position += vec3(-3.0f,-1.0f,0.0f);

    render_mode mode;
    bool play_animate = false;
    float time = 0.0f;
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
            for (int i = 0; i < scene->root_count; ++i) {
                object_t* root = scene->roots[i];
                if(play_animate){
                    root->animator->Play();
                }else{
                    root->animator->Pause();
                }
            }
        }
        if(window->GetKeyPressed(KEY_S)){
            play_animate = false;
            for (int i = 0; i < scene->root_count; ++i) {
                object_t* root = scene->roots[i];
                root->animator->Stop();
            }
        }

        renderer->Render(scene);
        window->Update();

        snake->transform.position.z = cos(time)*2;

        time+=1.0f/60;
//        using namespace std::chrono_literals;
//        std::this_thread::sleep_for(16ms);
    }

    delete scene;
    delete window;
    return 0;
}
