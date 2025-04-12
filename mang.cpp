#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_sdl3.h"
#include "include/imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL3/SDL.h>

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <include/SDL3/SDL_opengles2.h>
#else

#include <SDL3/SDL_opengl.h>

#endif

//paneb asjad keskele
void AlignForWidth(float width, float alignment = 0.5f) {
    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - width) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}

#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

// Rakenduse vaated: menüü või mäng
enum class Vaade {
    MENYY,
    MANG
};
// Aktiivne vaade (alguses: menüü)
Vaade aktiivneVaade = Vaade::MENYY;
//Mängulaua suurus mida kasutaja saab valida
static int gridSize = 3;


int main() {
    // SDL käivitamine
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        printf("Error: SDL_Init(): %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#elif defined(__APPLE__)
    // GL 3.2 Core + GLSL 150
    const char* glsl_version = "#version 150";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // Always required on Mac
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
#else
    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // SDL akna loomine
    SDL_Window *window = SDL_CreateWindow("Mang", 1280, 720,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    // Sean akna ekraani keskele
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);

    // Akna loomise kontroll
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (gl_context == nullptr) {
        printf("Error: SDL_GL_CreateContext(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync
    SDL_ShowWindow(window);

    // Imgui loomine
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // Akna taustavärv
    bool mangKaib = true;

    // Mainloop
    while (mangKaib) {

        // Sündmuste handlimine
        SDL_Event sundmus;
        while (SDL_PollEvent(&sundmus)) {
            // Imgui akna sündmused
            ImGui_ImplSDL3_ProcessEvent(&sundmus);

            // Mängu lõpetamise (sulgemise) sündmus
            if (sundmus.type == SDL_EVENT_QUIT) {
                mangKaib = false;
            }
        }

        // Ma ei tea mis see on aga see oli näitekoodis
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED) {
            SDL_Delay(10);
            continue;
        }

        // Imgui akna alustamine
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();


        // Imgui akna definitsioon
        ImVec2 fixedSize(512, 288);
        ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);
        ImGui::Begin("Numbripusle", nullptr, ImGuiWindowFlags_NoResize);

        if (aktiivneVaade == Vaade::MENYY) {
        // Tekst on keskel
        float textWidth = ImGui::CalcTextSize("Tere tulemast numbripusle mängu!").x;
        AlignForWidth(textWidth);
        ImGui::Text("Tere tulemast numbripusle mängu!");
        ImGui::NewLine();
        ImGui::Text("Kas suudad kõik numbrid õigesse järjekorda panna?\n");
        ImGui::Text("Iga käik loeb! Püüa lahendada pusle võimalikult väheste käikudega!\n");
        ImGui::NewLine();
        ImGui::Text("Vali sobiv ruudustiku suurus ja asu väljakutsele!\n");
        ImGui::Text("Head mänguõnne!\n");
        ImGui::NewLine();

        // Paigutame slideri keskele
        float sliderWidth = 300.0f;
        AlignForWidth(sliderWidth);
        ImGui::SliderInt(" ", &gridSize, 3, 8);
        ImGui::NewLine();

        // Et nupud oleks keskel
        ImGui::SetCursorPosX(fixedSize.x * 0.5f - 100);
        if (ImGui::Button("Alusta mängu")) {
            aktiivneVaade = Vaade::MANG;
        }
        ImGui::SameLine();
        if (ImGui::Button("Välju mängust")) {
            mangKaib = false;
        }

        // 10 pikslit servast
        float bottomY = ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 10;
        ImGui::SetCursorPosY(bottomY);

        ImGui::Text("Mängu tegid: Annabel & Egert\n");

        }
        else if (aktiivneVaade == Vaade::MANG) {
            ImGui::Begin("Mäng", nullptr, ImGuiWindowFlags_NoResize);

            ImGui::Text("Siin tuleb mängulaud (nt %dx%d grid) ", gridSize, gridSize);
            ImGui::NewLine();

            if (ImGui::Button("Tagasi menüüsse")) {
                aktiivneVaade = Vaade::MENYY;
            }
            ImGui::End();
        }

        //END
        ImGui::End();


        // Renderimine
        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                     clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Programmi lõpetamine peale mainloopi
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DestroyContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}