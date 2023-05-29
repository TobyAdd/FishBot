#include "imgui-hook.hpp"
#include "imgui_theme.hpp"
#include "font.hpp"
#include <functional>
#include <chrono>

WNDPROC originalWndProc;
bool isInitialized = false;
bool ImGuiHook::blockMetaInput = true;
std::function<void()> drawFunction = []() {};
std::function<void()> toggleFunction = []() {};
HWND hWnd;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void(__thiscall* CCEGLView_swapBuffers)(CCEGLView*);
void __fastcall CCEGLView_swapBuffers_H(CCEGLView* self)
{
    if (!isInitialized)
    {
        isInitialized = true;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_data, sizeof(segoeui_size), 20.f);
        io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_data, sizeof(segoeui_size), 34.f);
        //io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_data, sizeof(segoeui_size), 32.f);        
        io.IniFilename = nullptr;
        ApplyStyle();
        hWnd = WindowFromDC(*reinterpret_cast<HDC*>(reinterpret_cast<uintptr_t>(self->getWindow()) + 0x244));
        originalWndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplOpenGL3_Init();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    drawFunction();

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glFlush();

    CCEGLView_swapBuffers(self);
}

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

    if (msg == WM_KEYDOWN && wParam == VK_SHIFT && !ImGui::GetIO().WantCaptureKeyboard)
    {
        toggleFunction();
    }

    if (ImGui::GetIO().WantCaptureMouse && ImGuiHook::blockMetaInput)
    {
        if (msg == WM_MOUSEMOVE || msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_MOUSEWHEEL)
            return 0;
    }

    if (ImGui::GetIO().WantCaptureKeyboard)
    {
        if (msg == WM_KEYDOWN || msg == WM_KEYUP || msg == WM_SYSKEYDOWN || msg == WM_SYSKEYUP ||
            msg == WM_HOTKEY || msg == WM_KILLFOCUS || msg == WM_SETFOCUS)
            return 0;
    }

    return CallWindowProc(originalWndProc, hWnd, msg, wParam, lParam);
}

void(__thiscall* CCEGLView_toggleFullScreen)(void*, bool);
void __fastcall CCEGLView_toggleFullScreen_H(void* self, void*, bool toggle)
{
    ImGuiHook::Unload();
    CCEGLView_toggleFullScreen(self, toggle);
}

void ImGuiHook::Load(std::function<void(void*, void*, void**)> hookFunc)
{
    auto cocosBase = GetModuleHandleA("libcocos2d.dll");
    hookFunc(GetProcAddress(cocosBase, "?swapBuffers@CCEGLView@cocos2d@@UAEXXZ"), CCEGLView_swapBuffers_H, reinterpret_cast<void**>(&CCEGLView_swapBuffers));
    hookFunc(GetProcAddress(cocosBase, "?toggleFullScreen@CCEGLView@cocos2d@@QAEX_N@Z"), CCEGLView_toggleFullScreen_H, reinterpret_cast<void**>(&CCEGLView_toggleFullScreen));
}

void ImGuiHook::Unload()
{
    isInitialized = false;
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();    
}


void ImGuiHook::setRenderFunction(std::function<void()> func)
{
    drawFunction = func;
}

void ImGuiHook::setToggleFunction(std::function<void()> func)
{
    toggleFunction = func;
}
