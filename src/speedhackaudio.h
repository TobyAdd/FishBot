#ifndef SPEEDHACK_AUDIO_H
#define SPEEDHACK_AUDIO_H

#include <Windows.h>
#include <MinHook.h>

namespace SpeedhackAudio {
    inline void* channel = nullptr;
    inline float speed = 1.0f;
    inline bool initialized = false;
    inline void* (__stdcall* setVolume)(void* t_channel, float volume) = nullptr;
    inline void* (__stdcall* setFrequency)(void* t_channel, float frequency) = nullptr;

    inline void* __stdcall setVolumeHook(void* t_channel, float volume) {
        channel = t_channel;
        if (speed != 1.0f && setFrequency != nullptr)
            setFrequency(channel, speed);
        return setVolume(channel, volume);
    }

    inline void init() {
        if (initialized)
            return;

        HMODULE fmodDll = GetModuleHandleA("fmod.dll");

        setFrequency = reinterpret_cast<decltype(setFrequency)>(GetProcAddress(fmodDll, "?setPitch@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));
        DWORD hkAddr = reinterpret_cast<DWORD>(GetProcAddress(fmodDll, "?setVolume@ChannelControl@FMOD@@QAG?AW4FMOD_RESULT@@M@Z"));

        if (setFrequency == nullptr || hkAddr == 0)
            return;

        MH_CreateHook(reinterpret_cast<PVOID>(hkAddr), setVolumeHook, reinterpret_cast<PVOID*>(&setVolume));

        speed = 1.0f;
        initialized = true;
    }

    inline void set(float frequency) {
        if (!initialized)
            init();
        
        if (channel == nullptr)
            return;

        speed = frequency;
        if (setFrequency != nullptr)
            setFrequency(channel, frequency);
    }
}

#endif // SPEEDHACK_AUDIO_H
