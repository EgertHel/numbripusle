#include "include/imgui/imgui.h"
#include "include/imgui/imgui_impl_sdl3.h"
#include "include/imgui/imgui_impl_opengl3.h"
#include <stdio.h>
#include <string>
#include "include/SDL3/SDL.h"
#include "grid.h"

#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <include/SDL3/SDL_opengles2.h>
#else

#include <SDL3/SDL_opengl.h>
#include <chrono>

#endif

//paneb asjad keskele
void AlignForWidth(float width, float alignment = 0.5f) {
    float avail = ImGui::GetContentRegionAvail().x;
    float off = (avail - width) * alignment;
    if (off > 0.0f)
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + off);
}
std::string VormindaAeg(std::chrono::steady_clock::duration aeg) {
    auto sekundid = std::chrono::duration_cast<std::chrono::seconds>(aeg).count();
    int min = sekundid / 60;
    int sec = sekundid % 60;

    return std::to_string(min) + " min " + std::to_string(sec) + " sek";
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
std::chrono::steady_clock::time_point mänguAlgus;
std::chrono::steady_clock::duration möödunudAeg;
std::chrono::steady_clock::duration loppAeg;
bool kasTaimerKäib = false;

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
    SDL_Window *window = SDL_CreateWindow("Mang", 1024, 576,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    // Sean akna ekraani keskele
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    SDL_SetWindowResizable(window, false);

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

    // Enda fondi seadmine
    io.FontDefault = io.Fonts->AddFontFromFileTTF("Leedsuni-23p8.ttf", 22.0f);

    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // Akna taustavärv
    bool mangKaib = true; // mainloopi muutuja
    bool manguakenAvatud = false; // Algselt avaneb menüü mitte mäng
    std::vector<bool> liigutuseAnimeerimine;
    ImVec2 animeerimisHetkePos;
    ImVec2 animeerimisLoppPos;
    ImVec2 animeerimisSamm;
    int emptyIndex;

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
            ImVec2 fixedSize(1024, 576);
            ImGui::SetNextWindowSize(fixedSize, ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Numbripusle", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            // Tekst on keskel
            float textWidth = ImGui::CalcTextSize("Tere tulemast numbripusle mängu!").x;
            AlignForWidth(textWidth);
            ImGui::Text("Tere tulemast numbripusle mängu!");
            ImGui::NewLine();

            textWidth = ImGui::CalcTextSize("Kas suudad kõik numbrid õigesse järjekorda panna?").x;
            AlignForWidth(textWidth);
            ImGui::Text("Kas suudad kõik numbrid õigesse järjekorda panna?");

            textWidth = ImGui::CalcTextSize("Iga käik loeb! Püüa lahendada pusle võimalikult väheste käikudega!").x;
            AlignForWidth(textWidth);
            ImGui::Text("Iga käik loeb! Püüa lahendada pusle võimalikult väheste käikudega!");
            ImGui::NewLine();

            textWidth = ImGui::CalcTextSize("Vali sobiv ruudustiku suurus ja asu väljakutsele!").x;
            AlignForWidth(textWidth);
            ImGui::Text("Vali sobiv ruudustiku suurus ja asu väljakutsele!");

            textWidth = ImGui::CalcTextSize("Head mänguõnne!").x;
            AlignForWidth(textWidth);
            ImGui::Text("Head mänguõnne!");
            ImGui::NewLine();

            // Paigutame slideri keskele
            float sliderWidth = 653.0f;
            AlignForWidth(sliderWidth);
            ImGui::SliderInt(" ", &gridSize, 3, 8);
            ImGui::NewLine();

            // Et nupud oleks keskel
            ImGui::SetCursorPosX(fixedSize.x * 0.5f - 200);
            if (ImGui::Button("Alusta mängu", ImVec2(200, 40))) {
                aktiivneVaade = Vaade::MANG;
                manguakenAvatud = true;
                kaigud = 0;
                kasOnGenereeritud = false;
                liigutuseAnimeerimine = std::vector<bool>(gridSize * gridSize, false);
                SDL_SetWindowSize(window, static_cast<float>(gridSize * 70) + 100,
                                static_cast<float>(gridSize * 70) + 220);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                //taimer
                mänguAlgus = std::chrono::steady_clock::now();
                kasTaimerKäib = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Välju mängust", ImVec2(200, 40))) {
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
            ImVec2 manguAknaSuurus{static_cast<float>(gridSize * 70) + 100,
                                   static_cast<float>(gridSize * 70) + 220};
            ImGui::SetNextWindowSize(manguAknaSuurus, ImGuiCond_Always);
            ImGui::SetNextWindowPos(ImVec2(0, 0));
            ImGui::Begin("Mäng", &manguakenAvatud, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

            std::string kaigudStr = "Käike tehtud: " + std::to_string(kaigud);
            AlignForWidth(ImGui::CalcTextSize(kaigudStr.c_str()).x);
            ImGui::Text("%s", kaigudStr.c_str());

            if (kasTaimerKäib) {
                möödunudAeg = std::chrono::steady_clock::now() - mänguAlgus;

                std::string taimeriTekst = "Aeg: " + VormindaAeg(möödunudAeg);

                AlignForWidth(ImGui::CalcTextSize(taimeriTekst.c_str()).x);
                ImGui::Text("%s", taimeriTekst.c_str());
            }

            // Veidi vahet
            ImGui::Dummy(ImVec2(0.0f, 20.0f));

            // Genereerime lahendatava ruudustiku
            if (!kasOnGenereeritud) {
                nuppudeVäärtused = GenerateSolvablePuzzle(gridSize);
                kasOnGenereeritud = true;
            }


            // Mängulaua loomine
            float animatsioonX;
            float animatsioonY;
            for (int rida = 0; rida < gridSize; rida++) {
                for (int veerg = 0; veerg < gridSize; veerg++) {
                    animatsioonX = 0;
                    animatsioonY = 0;
                    // Arvutame indeksi vektoris
                    int index = rida * gridSize + veerg;
                    int väärtus = nuppudeVäärtused[index];

                    if (liigutuseAnimeerimine[index]) {
                        animeerimisHetkePos.x += animeerimisSamm.x;
                        animeerimisHetkePos.y += animeerimisSamm.y;
                        animatsioonX = animeerimisHetkePos.x;
                        animatsioonY = animeerimisHetkePos.y;

                        if (animeerimisHetkePos.x == animeerimisLoppPos.x &&
                            animeerimisHetkePos.y == animeerimisLoppPos.y) {
                            liigutuseAnimeerimine[index] = false;
                            std::swap(nuppudeVäärtused[index], nuppudeVäärtused[emptyIndex]);
                            // Võidukontroll
                            if (IsPuzzleSolved(nuppudeVäärtused, gridSize)) {
                                ImGui::OpenPopup("Võit!");
                            }
                        }
                    }

                    ImGui::SetCursorPosX(50 + (veerg * 70) + animatsioonX);
                    ImGui::SetCursorPosY(70 + (rida * 70) + animatsioonY);

                    // Kui väärtus on 0, siis jätame tühja koha
                    if (väärtus == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(39.0f / 255.0f, 73.0f / 255.0f, 114.0f / 255.0f, 0.4f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(39.0f / 255.0f, 73.0f / 255.0f, 114.0f / 255.0f, 0.4f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(39.0f / 255.0f, 73.0f / 255.0f, 114.0f / 255.0f, 0.4f));
                        ImGui::Button(" ", ImVec2(70, 70));
                        ImGui::PopStyleColor(3);
                    } else {
                        // Muidu loome nupu vastava väärtusega
                        if (ImGui::Button(std::to_string(väärtus).c_str(), ImVec2(70, 70))) {
                            // Leia tühi koht
                            emptyIndex = -1;
                            for (size_t i = 0; i < nuppudeVäärtused.size(); i++) {
                                if (nuppudeVäärtused[i] == 0) {
                                    emptyIndex = i;
                                    break;
                                }
                            }

                            int row = index / gridSize;
                            int col = index % gridSize;
                            int emptyRow = emptyIndex / gridSize;
                            int emptyCol = emptyIndex % gridSize;

                            // Tühja koha kontroll
                            bool isAdjacent =
                                    (row == emptyRow && abs(col - emptyCol) == 1) ||  // vasak/parem
                                    (col == emptyCol && abs(row - emptyRow) == 1);   // üleval/all

                            if (isAdjacent) {
                                liigutuseAnimeerimine[index] = true;
                                animeerimisHetkePos = ImVec2(0, 0);
                                animeerimisLoppPos = ImVec2(((emptyCol - col) * 70), ((emptyRow - row) * 70));
                                animeerimisSamm = ImVec2(((emptyCol - col) * 10), ((emptyRow - row) * 10));
                                kaigud++;

                                // Võidukontroll
                                if (IsPuzzleSolved(nuppudeVäärtused, gridSize)) {
                                    kasTaimerKäib = false;  // Miks ei peata?
                                    ImGui::OpenPopup("Võit!");
                                }
                            }

                        }
                    }
                }
            }

            
            ImGui::SetCursorPos(ImVec2(50, (70 + (gridSize * 70) + 70)));

            if (ImGui::Button("Tagasi menüüsse", ImVec2(200, 40))) {
                aktiivneVaade = Vaade::MENYY;
                manguakenAvatud = false;
                SDL_SetWindowSize(window, 1024, 576);
                SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
            }// Kontrollime, kas "Võit!" pop-up aken on avatud
            if (ImGui::BeginPopupModal("Võit!", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::Text("Palju õnne!");
                ImGui::Text("Sa lahendasid numbripusle!");
                ImGui::Text("Käike kulus: %d", kaigud);
                ImGui::Text("Aega kulus: %s", VormindaAeg(möödunudAeg).c_str());
                ImGui::Dummy(ImVec2(0.0f, 10.0f));
                if (ImGui::Button("Tagasi menüüsse")) {
                    aktiivneVaade = Vaade::MENYY;
                    manguakenAvatud = false;
                    ImGui::CloseCurrentPopup();
                    SDL_SetWindowSize(window, 1024, 576);
                    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
                }
                ImGui::EndPopup();
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