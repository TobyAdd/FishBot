#define IMGUI_DEFINE_MATH_OPERATORS
#include "gui.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui-hook.hpp>
#include <shellapi.h>
#include <chrono>
#include "replayEngine.h"
#include "hooks.h"
#include "speedhackaudio.h"

bool gui::show = false;
bool gui::inited = false;

void gui::Render()
{
    if (!gui::show)
        return;

    static bool init1 = false;
    if (!init1) {
        init1 = true;
        ImGui::SetNextWindowSize({ 350, 525 });
        ImGui::SetNextWindowPos({ 10, 10 });
    }

    ImGui::Begin("zBot", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);

    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);

    ImGui::TextColored(ImColor(255, 189, 0), "FishBot");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(78, 76, 82), "v1.1");

    ImGui::PopFont();

    ImGui::Text("Current Replay Name:");
    ImGui::SameLine();

    bool empty = (replay.replay_name != NULL) && (replay.replay_name[0] == '\0');
    ImGui::TextColored(ImColor(0, 255, 255), "%s", empty ? "No Replay Loaded" : replay.replay_name);

    ImGui::Text("Replay FPS:");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(0, 255, 255), "%.3f", replay.fps_value);

    int mode = (int)replay.mode;

    if (ImGui::RadioButton("Disable", &mode, 0))
        replay.mode = (state)mode;
    ImGui::SameLine();
    if (ImGui::RadioButton("Record", &mode, 1))
    {
        if (gd::GameManager::sharedState()->getGameVariable("0027"))
        {
            gd::GameManager::sharedState()->setGameVariable("0027", false);
        }
        auto pl = gd::GameManager::sharedState()->getPlayLayer();
        if (pl)
            strcpy_s(replay.replay_name, pl->m_level->levelName.c_str());
        replay.mode = (state)mode;
        replay.clear();
    }
    ImGui::SameLine();
    if (ImGui::RadioButton("Playback", &mode, 2))
        replay.mode = (state)mode;

    ImGui::NewLine();

    ImGui::Text("Import Replay by name\n(must be in replays folder)");

    static char macro_name[128];

    ImGui::InputText("##import_name", macro_name, IM_ARRAYSIZE(macro_name));

    if (ImGui::Button("Import")) {
        static int count = 0;

        if (replay.load((string)macro_name)) {
            strcpy_s(replay.replay_name, macro_name);
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("Open Replays Folder")) {
        ShellExecuteA(0, "open", "replays", 0, 0, SW_SHOWNORMAL);
    }

    ImGui::NewLine();

    ImGui::Text("Override Recording Name");

    static char export_name[128];

    ImGui::InputText("##export_name", export_name, IM_ARRAYSIZE(export_name));

    if (ImGui::Button("Apply")) {
        strcpy_s(replay.replay_name, export_name);
        strcpy_s(export_name, "");
    }

    ImGui::SameLine();

    if (ImGui::Button("Manually Save to File")) {
        replay.save((string)replay.replay_name);
    }

    ImGui::NewLine();

    ImGui::Checkbox("Real Time", &replay.real_time);
    ImGui::Checkbox("Ignore user input on playback", &replay.ignore_input);

    ImGui::End();

    static bool init2 = false;
    if (!init2) {
        init2 = true;
        ImGui::SetNextWindowSize({ 200, 330 });
        ImGui::SetNextWindowPos({ 380, 10 });
    }

    ImGui::Begin("Utilities", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    static bool speedaudio = false;
    ImGui::Text("FPS:");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(0, 255, 255), "%.3f", replay.fps_value);

    ImGui::Text("Speed:");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(0, 255, 255), "%.3f", replay.speed_value);

    ImGui::Text("Frame:");
    ImGui::SameLine();
    ImGui::TextColored(ImColor(0, 255, 255), "%i", gd::GameManager::sharedState()->getPlayLayer() ? replay.get_frame() : 0);

    ImGui::Text("Set FPS:");

    static float fps = 60.f;
    ImGui::InputFloat("##fps_value", &fps);
    if (ImGui::Button("Apply FPS")) {
        replay.fps_value = fps;
    }

    ImGui::NewLine();

    ImGui::Text("Set Speed:");

    static float speed = 1.f;
    ImGui::InputFloat("##speed_value", &speed);
    if (ImGui::Button("Apply Speedhack")) {
        replay.speed_value = speed;
        if (speedaudio) {
            SpeedhackAudio::set(speed);
        }
    }

    ImGui::End();

    static bool init3 = false;
    if (!init3) {
        init3 = true;
        ImGui::SetNextWindowSize({ 200, 320 });
        ImGui::SetNextWindowPos({ 600, 10 });
    }

    ImGui::Begin("Hacks", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    
    static bool noclip = false;
    if (ImGui::Checkbox("Noclip", &noclip)) {
        if (noclip) {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A23C), "\xE9\x79\x06\x00\x00", 5, NULL);
        }
        else {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A23C), "\x6A\x14\x8B\xCB\xFF", 5, NULL);
        }  
    }

    static bool pmh = false;
    if (ImGui::Checkbox("Practice Music", &pmh)) {
        if (pmh) {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C925), "\x90\x90\x90\x90\x90\x90", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D143), "\x90\x90", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A563), "\x90\x90", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A595), "\x90\x90", 2, NULL);
        }
        else {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C925), "\x0F\x85\xF7\x00\x00\x00", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D143), "\x75\x41", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A563), "\x75\x3E", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A595), "\x75\x0C", 2, NULL);
        }
    }

    
    static bool fastrespawn = false;
    if (ImGui::Checkbox("Fast Respawn", &fastrespawn)) {
        if (fastrespawn) {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A677), "\xE8", 1, NULL);
        }
        else {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20A677), "\xD8", 1, NULL);
        }  
    }

    static bool dde = false;
    if (ImGui::Checkbox("Disable Death Effect", &dde)) {
        if (dde) {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1EFBA4), "\x90\x90\x90\x90\x90", 5, NULL);
        }
        else {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1EFBA4), "\xE8\x37\x00\x00\x00", 5, NULL);
        }  
    }

    static bool anticheat = false;
    if (ImGui::Checkbox("Anticheat Bypass", &anticheat)) {
        if (anticheat) {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x202AAA), "\xEB\x2E", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x15FC2E), "\xEB", 1, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D3B3), "\x90\x90\x90\x90\x90", 5, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FF7A2), "\x90\x90", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x18B2B4), "\xB0\x01", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C4E6), "\xE9\xD7\x00\x00\x00\x90", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD557), "\xEB\x0C", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD742), "\xC7\x87\xE0\x02\x00\x00\x01\x00\x00\x00\xC7\x87\xE4\x02\x00\x00\x00\x00\x00\x00\x90\x90\x90\x90\x90\x90", 26, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD756), "\x90\x90\x90\x90\x90\x90", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD79A), "\x90\x90\x90\x90\x90\x90", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD7AF), "\x90\x90\x90\x90\x90\x90", 6, NULL);
        }
        else {
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x202AAA), "\x74\x2E", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x15FC2E), "\x74", 1, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20D3B3), "\xE8\x58\x04\x00\x00", 5, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FF7A2), "\x74\x6E", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x18B2B4), "\x88\xD8", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x20C4E6), "\x0F\x85\xD6\x00\x00\x00", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD557), "\x74\x0C", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD742), "\x80\xBF\xDD\x02\x00\x00\x00\x0F\x85\x0A\xFE\xFF\xFF\x80\xBF\x34\x05\x00\x00\x00\x0F\x84\xFD\xFD\xFF\xFF", 26, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD557), "\x74\x0C", 2, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD756), "\x0F\x84\xFD\xFD\xFF\xFF", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD79A), "\x0F\x84\xB9\xFD\xFF\xFF", 6, NULL);
            WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(gd::base + 0x1FD7AF), "\x0F\x85\xA4\xFD\xFF\xFF", 6, NULL);
        }  
    }


    if (ImGui::Checkbox("Speedhack Audio", &speedaudio)) {
        if (speedaudio)
            SpeedhackAudio::set(speed);
        else
            SpeedhackAudio::set(1);
    }


    ImGui::Checkbox("Frame Advance", &frameAdvance.enabled);
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Current key is %c", char(key_frameadvance));
        change_key_frameadvance = true;
    }
    else {
        change_key_frameadvance = false;
    }

    ImGui::End();
}

void gui::Toggle()
{
    gui::show = !gui::show;
}
