// gui_main.cpp
//
// Bare ImGui + SDL3 + OpenGL3 window. Deliberately has zero sensor code --
// the point of this file is to prove the GUI stack builds and runs on its
// own, so if something breaks later you know which half (GUI vs. sensor)
// is at fault.

#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>
#include <cstdio>

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
      printf("SDL_Init failed: %s\n", SDL_GetError());
      return 1;
  }

  // baseline that ImGui's OpenGL3 backend targets by default.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_Window* window = SDL_CreateWindow(
    "LoggerUM",
    1280, 720,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

  if (window == nullptr) {
    printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
    SDL_Quit();
    return 1;
  }

  SDL_GLContext glContext = SDL_GL_CreateContext(window);
  if (glContext == nullptr) {
    printf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 1;
  }

  SDL_GL_MakeCurrent(window, glContext);
  SDL_GL_SetSwapInterval(1); // vsync on

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::StyleColorsDark();

  ImGui_ImplSDL3_InitForOpenGL(window, glContext);
  ImGui_ImplOpenGL3_Init("#version 150");

  bool running = true;
  //main loop
  while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) running = false;
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    // --- Bare-window content: just enough to confirm it's alive ---
    static char textBuf[128] = "";
    static int clickCount = 0;

    ImGui::Begin("LoggerUM");
    if (ImGui::Button("Click me")) {
      clickCount++;
      printf("Button clicked! Count = %d\n", clickCount);
    }
    ImGui::SameLine();
    ImGui::Text("Clicked %d times", clickCount);

    ImGui::InputText("Label", textBuf, sizeof(textBuf));
    if (ImGui::Button("Print text"))
    {
      printf("Text box contains: %s\n", textBuf);
    }

    ImGui::End();
    // ----------------------------------------------------------------
    ImGui::Render();
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);
    glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    SDL_GL_SwapWindow(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplSDL3_Shutdown();
  ImGui::DestroyContext();

  SDL_GL_DestroyContext(glContext);
  SDL_DestroyWindow(window);
  SDL_Quit();
  return 0;
}