//
// Created by koerriva on 2022/1/5.
//
#include <iostream>
#include "renderer.h"

Animator::Animator(struct object_t *_model):model(_model) {
}

void Animator::Play() {
    playing = true;
    playingAnimation = "";
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
    playingAnimation = "";
    std::cout << "Stop Animation ..." << std::endl;
    for (int i = 0; i < MAX_ANIMATION_COUNT; ++i) {
        currTime[i] = 0;//时间同步
        for (int j = 0; j < MAX_CHANNEL_COUNT; ++j) {

            prevFrame[i][j] = nullptr;
            nextFrame[i][j] = nullptr;

            if(i<model->animation_count){
                animation_t * animation = model->animations + i;
                if(j<animation->channel_count){
                    channel_t * channel = animation->channels + j;

                    channel->target->animated = false;
                }
            }
        }
    }
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
            float interp = (currTime[i] - prevFrame[i][j]->time)/(nextFrame[i][j]->time-prevFrame[i][j]->time);

            channel->target->animated_transform = channel->target->transform;

            if(channel->has_translation){
                vec3 translation = mix(prevFrame[i][j]->translation,nextFrame[i][j]->translation,interp);
                vec3 origin = channel->target->transform.position;
                channel->target->animated_transform.position = origin + translation;
                channel->target->animated = true;
            }

            if(channel->has_rotation){
                quat origin = channel->target->transform.rotation;

                quat prev = prevFrame[i][j]->rotation;
                quat next = nextFrame[i][j]->rotation;

                quat rotation = slerp(prev, next,interp);
                channel->target->animated_transform.rotation = origin * rotation;
                channel->target->animated = true;
            }

            if(channel->has_scale){
                vec3 scale = mix(prevFrame[i][j]->scale,nextFrame[i][j]->scale,interp);
                vec3 origin = channel->target->transform.scale;
                channel->target->animated_transform.scale = origin * scale;
                channel->target->animated = true;
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

        currTime[i] += 0.001;
    }
}

bool Animator::IsPlaying() const {
    return playing;
}