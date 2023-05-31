#include <functional>
#include "imgui-hook.hpp"
#include "imgui_theme.hpp"
#include "font.hpp"

SwapBuffersType originalSwapBuffers = nullptr;
WNDPROC originalWndProc = nullptr;
HGLRC originalContext = 0, newContext = 0;

bool isInitialized = false;
std::function<void()> drawFunc = []() {};
std::function<void()> toggleFunction = []() {};
HWND hWnd;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

BOOL WINAPI HookedSwapBuffers(HDC hdc)
{
    if (!isInitialized)
    {
        originalContext = wglGetCurrentContext();
        newContext = wglCreateContext(hdc);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_data, sizeof(segoeui_size), 20.f);
        io.Fonts->AddFontFromMemoryCompressedTTF(segoeui_data, sizeof(segoeui_size), 34.f);       
        io.IniFilename = nullptr;
        ApplyStyle();
        hWnd = WindowFromDC(hdc);
        originalWndProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)HookedWndProc);
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplOpenGL2_Init();

        isInitialized = true;
    }

    wglMakeCurrent(hdc, newContext);

    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    drawFunc();

    ImGui::Render();
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());

    glFlush();

    wglMakeCurrent(hdc, originalContext);

    return originalSwapBuffers(hdc);
}

void ImGuiHook::Unload()
{
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)originalWndProc);
    isInitialized = false;
}

LRESULT CALLBACK HookedWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);

    if (msg == WM_KEYDOWN && wParam == VK_SHIFT && !ImGui::GetIO().WantCaptureKeyboard)
    {
        toggleFunction();
    }

    if (ImGui::GetIO().WantCaptureMouse)
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

void (__thiscall* CCEGLView_toggleFullScreen)(void*, bool);
void __fastcall CCEGLView_toggleFullScreen_H(void* self, void*, bool toggle)
{
    ImGuiHook::Unload();
    CCEGLView_toggleFullScreen(self, toggle);
}


void ImGuiHook::Load(std::function<void(void*, void*, void**)> hookFunc)
{
    hookFunc(
        GetProcAddress(LoadLibraryA("opengl32.dll"), "wglSwapBuffers"),
        HookedSwapBuffers,
        reinterpret_cast<void**>(&originalSwapBuffers)
    );

    hookFunc(
        GetProcAddress(GetModuleHandleA("libcocos2d.dll"), "?toggleFullScreen@CCEGLView@cocos2d@@QAEX_N@Z"),
        CCEGLView_toggleFullScreen_H,
        reinterpret_cast<void**>(&CCEGLView_toggleFullScreen)
    );
}

void ImGuiHook::setRenderFunction(std::function<void()> func)
{
    drawFunc = func;
}

void ImGuiHook::setToggleFunction(std::function<void()> func)
{
    toggleFunction = func;
}
