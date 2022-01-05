//
// Created by koerriva on 2022/1/5.
//
#include <iostream>
#include "renderer.h"

Animator::Animator(struct model_t *_model):model(_model) {
    for (int i = 0; i < MAX_ANIMATION_COUNT; ++i) {
        for (int j = 0; j < MAX_CHANNEL_COUNT; ++j) {
            currTime[i][j] = 0;
            prevFrame[i][j] = nullptr;
            nextFrame[i][j] = nullptr;

            if(i<model->animation_count){
                animation_t * animation = model->animations + i;
                if(j<animation->channel_count){
                    channel_t * channel = animation->channels + j;

                    origin_transform[i][j] = *channel->transform;
                }
            }
        }
    }
}

void Animator::Play() {
    playing = true;
    playingAnimation = "default";
    std::cout << "Play Animation [Default]" << std::endl;
}

void Animator::Play(const char* name) {
    playing = true;
    playingAnimation = name;
    std::cout << "Play Animation : " << name << std::endl;
}

void Animator::Pause() {
    playing = false;
    std::cout << "Pause Animation ..." << std::endl;
}

void Animator::Stop() {
    playing = false;
    playingAnimation = "default";
    std::cout << "Stop Animation ..." << std::endl;
    for (int i = 0; i < MAX_ANIMATION_COUNT; ++i) {
        for (int j = 0; j < MAX_CHANNEL_COUNT; ++j) {
            currTime[i][j] = 0;
            prevFrame[i][j] = nullptr;
            nextFrame[i][j] = nullptr;

            if(i<model->animation_count){
                animation_t * animation = model->animations + i;
                if(j<animation->channel_count){
                    channel_t * channel = animation->channels + j;

                    *channel->transform = origin_transform[i][j];
                }
            }
        }
    }
}

void Animator::Update(float delta) {
    if(!playing)return;
    for (int i = 0; i < model->animation_count; ++i) {
        animation_t * animation = model->animations + i;
        if(strcmp(playingAnimation,animation->name)!=0)continue;

        for (int j = 0; j < animation->channel_count; ++j) {
            channel_t* channel = animation->channels+j;
            keyframe_t* base_keyframe = channel->keyframe;
            int keyframe_count = channel->keyframe_count;

            for (int k = 0; k < keyframe_count; ++k) {
                keyframe_t* keyframe = base_keyframe+k;
                if(keyframe->time<=currTime[i][j]){
                    prevFrame[i][j] = keyframe;
                }
            }

            bool lastKeyframe = true;
            for (int k = 0; k < keyframe_count; ++k) {
                keyframe_t* keyframe = base_keyframe+k;
                if(keyframe->time>currTime[i][j]){
                    nextFrame[i][j] = keyframe;
                    lastKeyframe = false;
                    break;
                }
            }

            if(lastKeyframe){
                currTime[i][j] = 0.0;
                nextFrame[i][j] = base_keyframe;
            }

            //linear
            float interp = (currTime[i][j] - prevFrame[i][j]->time)/(nextFrame[i][j]->time-prevFrame[i][j]->time);

            if(channel->has_translation){
                vec3 translation = mix(prevFrame[i][j]->translation,nextFrame[i][j]->translation,interp);
                channel->transform->position = origin_transform[i][j].position + translation;
            }

            if(channel->has_rotation){
                quat origin = origin_transform[i][j].rotation;

                quat prev = prevFrame[i][j]->rotation;
                quat next = nextFrame[i][j]->rotation;

                quat rotation = slerp(prev, next,interp);
                channel->transform->rotation = origin * rotation;
            }

            if(channel->has_scale){
                vec3 scale = mix(prevFrame[i][j]->scale,nextFrame[i][j]->scale,interp);
                channel->transform->scale = origin_transform[i][j].scale * scale;
            }

            currTime[i][j] += delta;
        }
    }
}

bool Animator::IsPlaying() const {
    return playing;
}