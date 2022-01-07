//
// Created by koerriva on 2022/1/5.
//
#include <iostream>
#include "renderer.h"

Animator::Animator(struct object_t *_model):model(_model) {
}

void Animator::Play() {
    Play("");
}

void Animator::Play(const char* name) {
    playing = true;
    playingAnimation = name;
    std::cout << "Play Animation : " << name << std::endl;

    for (int i = 0; i < model->animation_count; ++i) {
        animation_t* animation = model->animations+i;
        for (int j = 0; j < animation->channel_count; ++j) {
            channel_t * channel = animation->channels+j;
            channel->target->animated = 1;
            channel->target->animated_transform = channel->target->transform;
        }
    }
}

void Animator::Pause() {
    playing = false;
    std::cout << "Pause Animation ..." << std::endl;
}

void Animator::Stop() {
    playing = false;
    playingAnimation = "";
    std::cout << "Stop Animation ..." << std::endl;

    for (int i = 0; i < model->animation_count; ++i) {
        currTime[i] = 0;//时间同步
        animation_t* animation = model->animations+i;
        for (int j = 0; j < animation->channel_count; ++j) {
            prevFrame[i][j] = nullptr;
            nextFrame[i][j] = nullptr;

            channel_t * channel = animation->channels+j;
            channel->target->animated = 0;
            channel->target->animated_transform = channel->target->transform;
        }
    }
}

vec3 lerp(vec3 prev,vec3 next,float interp){
    return prev+interp*(next-prev);
}

void Animator::Update(float delta) {
    if(!playing)return;
    for (int i = 0; i < model->animation_count; ++i) {
        animation_t * animation = model->animations + i;

        bool isPlayThisAnimation;
        isPlayThisAnimation = strcmp(playingAnimation,animation->name)==0;
        isPlayThisAnimation = isPlayThisAnimation || strcmp(playingAnimation, "") == 0 && i == 0;

        if(!isPlayThisAnimation){
            continue;
        }

        int finished_channel = 0;

        for (int j = 0; j < animation->channel_count; ++j) {
            channel_t* channel = animation->channels+j;

            keyframe_t* base_keyframe = channel->keyframe;
            int keyframe_count = channel->keyframe_count;

            for (int k = 0; k < keyframe_count; ++k) {
                keyframe_t* keyframe = base_keyframe+k;
                if(keyframe->time<=currTime[i]){
                    prevFrame[i][j] = keyframe;
                }
            }

            if(prevFrame[i][j] == nullptr){
                prevFrame[i][j] = new keyframe_t ;
            }

            bool lastKeyframe = true;
            for (int k = 0; k < keyframe_count; ++k) {
                keyframe_t* keyframe = base_keyframe+k;
                if(keyframe->time>currTime[i]){
                    nextFrame[i][j] = keyframe;
                    lastKeyframe = false;
                    break;
                }
            }

            if(lastKeyframe){
                finished_channel++;
                continue;
            }

            //linear
            float interp = (currTime[i] - prevFrame[i][j]->time)/(nextFrame[i][j]->time - prevFrame[i][j]->time);

            if(channel->has_translation){
                vec3 prev = prevFrame[i][j]->translation;
                vec3 next = nextFrame[i][j]->translation;
                vec3 translation = lerp(prev,next,interp);//lerp
                channel->target->animated_transform.position = translation;

                if(channel->target->jointed==0){
                    channel->target->animated_transform.position = channel->target->transform.position + translation;
                }
            }

            if(channel->has_rotation){
                quat prev = prevFrame[i][j]->rotation;
                quat next = nextFrame[i][j]->rotation;
                quat rotation = slerp(prev, next,interp);
                channel->target->animated_transform.rotation = rotation;

                if(channel->target->jointed==0){
                    channel->target->animated_transform.rotation = channel->target->transform.rotation * rotation;
                }
            }

            if(channel->has_scale){
                vec3 prev = prevFrame[i][j]->scale;
                vec3 next = nextFrame[i][j]->scale;
                vec3 scale = lerp(prev,next,interp);//lerp
                channel->target->animated_transform.scale = scale;

                if(channel->target->jointed==0){
                    channel->target->animated_transform.scale = channel->target->transform.scale * scale;
                }
            }
        }

        if(finished_channel>=animation->channel_count){
            currTime[i] = 0.0;
            for (int j = 0; j < animation->channel_count; ++j){
                prevFrame[i][j] = nullptr;
                nextFrame[i][j] = nullptr;
            }
            continue;
        }

        currTime[i] += delta;
    }
}

bool Animator::IsPlaying() const {
    return playing;
}