#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_sdl3.h"
#include "include/imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <string>
#include <SDL3/SDL.h>
#include "grid.h"

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
//Mitu käiku on kasutaja käinud
int kaigud = 0;
//Nuppude väärtuste massiiv
static std::vector<int> nuppudeVäärtused;
//Kas nuppude väärtused on juba genetud?
static bool kasOnGenereeritud = false;


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
    bool mangKaib = true; // mainloopi muutuja
    bool manguakenAvatud = false; // Algselt avaneb menüü mitte mäng

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


        // Kui mänguaken suleti, kuid vaade on ikka MANG, siis tuleks see ära muuta,
        // et menüü uuesti avaneks
        if (!manguakenAvatud && aktiivneVaade == Vaade::MANG) {
            aktiivneVaade = Vaade::MENYY;
        }

        if (aktiivneVaade == Vaade::MENYY) {
            // Imgui akna definitsioon
            ImVec2 fixedSize(512, 288);
            ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);
            ImGui::Begin("Numbripusle", nullptr, ImGuiWindowFlags_NoResize);

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
                manguakenAvatud = true;
                kaigud = 0;
                kasOnGenereeritud = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Välju mängust")) {
                mangKaib = false;
            }

            // 10 pikslit servast
            float bottomY = ImGui::GetWindowHeight() - ImGui::GetTextLineHeightWithSpacing() - 10;
            ImGui::SetCursorPosY(bottomY);

            ImGui::Text("Mängu tegid: Annabel & Egert\n");

            ImGui::End();

        } else if (aktiivneVaade == Vaade::MANG) {
            // Tuleb kontrollida, kas "X" nuppu on vajutatud
            if (!manguakenAvatud) {
                break;
            }

            // Akna suurus on nuppude arv * (nupu laius + vahe laius nuppude vahel) + 100
            // ehk kummalgi pool ekraani peaks 50 ühikut vaba ruumi olema
            ImVec2 manguAknaSuurus{static_cast<float>(gridSize) * (40 + ImGui::GetStyle().ItemSpacing.x) + 100,
                                   static_cast<float>(gridSize * 40) + 150};
            ImGui::SetNextWindowSize(manguAknaSuurus, ImGuiCond_Always);
            ImGui::Begin("Mäng", &manguakenAvatud, ImGuiWindowFlags_NoResize);

            std::string kaigudStr = "Käike tehtud: " + std::to_string(kaigud);
            AlignForWidth(ImGui::CalcTextSize(kaigudStr.c_str()).x);
            ImGui::Text("%s", kaigudStr.c_str());


            // Veidi vahet
            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            // Genereerime lahendatava ruudustiku
            if (!kasOnGenereeritud) {
                nuppudeVäärtused = GenerateSolvablePuzzle(gridSize);
                kasOnGenereeritud = true;
            }


            // Mängulaua loomine
            ImGui::NewLine();
            for (int rida = 0; rida < gridSize; rida++) {
                for (int veerg = 0; veerg < gridSize; veerg++) {
                    ImGui::SameLine();

                    // Arvutame indeksi vektoris
                    int index = rida * gridSize + veerg;
                    int väärtus = nuppudeVäärtused[index];

                    // Keskele joondamine
                    if (veerg == 0) ImGui::SetCursorPosX(50);

                    // Kui väärtus on 0, siis jätame tühja koha (see on tühi ruut)
                    if (väärtus == 0) {
                        ImGui::Button(" ", ImVec2(40, 40));
                    } else {
                        // Muidu loome nupu vastava väärtusega
                        if (ImGui::Button(std::to_string(väärtus).c_str(), ImVec2(40, 40))) {
                            kaigud++;
                            // TODO mis juhtub kui nuppu vajutatakse?
                        }
                    }
                }
                ImGui::NewLine();
            }


            ImGui::NewLine();

            if (ImGui::Button("Tagasi menüüsse")) {
                aktiivneVaade = Vaade::MENYY;
                manguakenAvatud = false;
            }
            ImGui::End();

        }




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