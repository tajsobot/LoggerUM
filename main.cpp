#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>

#include <cstdio>
#include <iostream>
#include <string>

#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_sdl3.h"

#include <cstdio>
#include <windows.h>
#include "GoIO_DLL_interface.h"

//GoIO ------------------------------------------------------------------------------------------------
bool testGoIO() {
 GoIO_Init();

 // Find the first available Go!Motion.
 char deviceName[GOIO_MAX_SIZE_DEVICE_NAME];
 int numFound = GoIO_UpdateListOfAvailableDevices(VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID);
 if (numFound <= 0) {
   printf("No Go!Motion found.\n");
   GoIO_Uninit();
   return 1;
 }
 GoIO_GetNthAvailableDeviceName(deviceName, GOIO_MAX_SIZE_DEVICE_NAME,
                                VERNIER_DEFAULT_VENDOR_ID, CYCLOPS_DEFAULT_PRODUCT_ID, 0);

 GOIO_SENSOR_HANDLE hDevice = GoIO_Sensor_Open(deviceName, VERNIER_DEFAULT_VENDOR_ID,
                                                CYCLOPS_DEFAULT_PRODUCT_ID, 0);
 if (hDevice == nullptr) {
   printf("Failed to open Go!Motion.\n");
   GoIO_Uninit();
   return 1;
 }

 // Start the sensor sampling at 40ms/measurement.
 GoIO_Sensor_SetMeasurementPeriod(hDevice, 0.10, SKIP_TIMEOUT_MS_DEFAULT);
 GoIO_Sensor_SendCmdAndGetResponse(hDevice, SKIP_CMD_ID_START_MEASUREMENTS,
                                    NULL, 0, NULL, NULL, SKIP_TIMEOUT_MS_DEFAULT);

  Sleep(300);
 gtype_int32 raw = GoIO_Sensor_GetLatestRawMeasurement(hDevice);
 double volts  = GoIO_Sensor_ConvertToVoltage(hDevice, raw);   // for Go!Motion this IS meters already
 double meters = GoIO_Sensor_CalibrateData(hDevice, volts);   // pass-through for this sensor

 printf("Distance: %.3f m\n", meters);

 GoIO_Sensor_Close(hDevice);
 GoIO_Uninit();
 return 0;
}


//UI ------------------------------------------------------------------------------------------------
bool CenteredButton(const char* label) {
  ImGuiStyle& style = ImGui::GetStyle();
  float buttonWidth = ImGui::CalcTextSize(label).x + style.FramePadding.x * 2.0f;
  float avail = ImGui::GetContentRegionAvail().x;
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - buttonWidth) * 0.5f);
  return ImGui::Button(label);
}

void CenteredText(const char* text) {
  float avail = ImGui::GetContentRegionAvail().x;
  float textWidth = ImGui::CalcTextSize(text).x;
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (avail - textWidth) * 0.5f);
  ImGui::Text("%s", text);
}

void SetGlobalScaleAndStyle(float scale, bool lightMode) {
  static ImGuiStyle baseStyle = ImGui::GetStyle(); // captured once, at 1.0x sizes
  ImGuiStyle& style = ImGui::GetStyle();
  style = baseStyle;
  style.ScaleAllSizes(scale);

  if (lightMode)
    ImGui::StyleColorsLight();
  else
    ImGui::StyleColorsDark();

  ImGui::GetIO().FontGlobalScale = scale;
}

int main() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
      printf("SDL_Init failed: %s\n", SDL_GetError());
      return 1;
  }

  // baseline that ImGui's OpenGL3 backend targets by default.
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  static bool isLightMode = true;
  static float globalScale = 2.0f;

  static int wHeight = 300*2 + 30;
  static int wWidth = 600*2 + 30;

  SDL_Window* window = SDL_CreateWindow(
    "LoggerUM",
    wWidth, wHeight,
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

  SetGlobalScaleAndStyle(globalScale, true);

  ImGui_ImplSDL3_InitForOpenGL(window, glContext);
  ImGui_ImplOpenGL3_Init("#version 150");

  static bool isMeasurementRunning = false;

  static std::string outputFilename = "data";
  static std::string outputDelimiter = ";";
  static std::string outputSeperator = ".";

  static double measuringTime = 10.0; //s
  static double measuringRate = 10.0; //Hz


  enum class OutputFormat { TXT, CSV };
  static OutputFormat outputFormat = OutputFormat::TXT;

  bool isMainLoopRunning = true;
  //main loop
  while (isMainLoopRunning) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) isMainLoopRunning = false;
    }

    /// ------------------------------------------------------------------------------------------------------------
    /// GUI \/ \/ \/ \/
    /// ------------------------------------------------------------------------------------------------------------

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL3_NewFrame();

    ImGui::NewFrame();

    // --- Bare-window content: just enough to confirm it's alive ---
    static char textBuf[128] = "";
    static int clickCount = 0;

    //SETTINGS -------------------------------------------------------------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Always);
    ImGui::Begin("SETTINGS", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Color Mode:  ");
    ImGui::SameLine();
    if (ImGui::Button("Light")) {
      isLightMode = true;
    }
    ImGui::SameLine();
    if (ImGui::Button("Dark")) {
      isLightMode = false;
    }

    ImGui::Text("Scale:  ");
    ImGui::SameLine();
    if (ImGui::Button("-0.25x")) {
      globalScale -= 0.25;
    }
    ImGui::SameLine();
    if (ImGui::Button("Reset scale")) {
      globalScale = 2.0f;
    }
    ImGui::SameLine();
    if (ImGui::Button("+0.25x")) {
      globalScale += 0.25;
    }
    SetGlobalScaleAndStyle(globalScale, isLightMode);
    ImGui::End();


    //INPUT CONFIG -------------------------------------------------------------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(620, 10), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Once);
    ImGui::Begin("INPUT CONFIG", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    //sensor dropdown
    const char* deviceOptions[] = { "Go!Motion", "Go!Temperature" };
    static int deviceIndex = 0; // 0 = txt, 1 = csv -- kept in sync with outputFormat below
    ImGui::Text("Device:");
    if (ImGui::Combo("##Output format", &deviceIndex, deviceOptions, IM_ARRAYSIZE(deviceOptions))) {
    //todo
    }
    if (ImGui::Button("Test")) {
      testGoIO();
    }
    ImGui::Text("Time [s]:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);
    static char measurementTimeBuff[128] = "";
    if (ImGui::InputText("##MeasurementTime", measurementTimeBuff, sizeof(measurementTimeBuff))) {
      try {
        measuringTime = std::stoi(measurementTimeBuff);
      }
      catch (const std::invalid_argument&) {
        std::cout << "NaN" << std::endl;
      }
      catch (const std::out_of_range&) {
        std::cout << "range" << std::endl;
      }
    }

    ImGui::Text("Rate [Hz]:");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(100.0f);

    static char measurementRateBuff[128] = "";
    if (ImGui::InputText("##MeasurementRate", measurementRateBuff, sizeof(measurementRateBuff))) {
      try {
        measuringRate = std::stoi(measurementRateBuff);
        std::cout << measuringRate << std::endl;
      }
      catch (const std::invalid_argument&) {
        std::cout << "NaN" << std::endl;
      }
      catch (const std::out_of_range&) {
        std::cout << "range" << std::endl;
      }
    }

    ImGui::End();

    //OUTPUT CONFIG -------------------------------------------------------------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(620, 320), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Once);
    ImGui::Begin("OUTPUT CONFIG", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar);
    //output filename
    ImGui::Text("Output filename:");
    static char outputFilenameBuf[128] = "";
    static bool seeded = false;
    if (!seeded) {
      snprintf(outputFilenameBuf, sizeof(outputFilenameBuf), "%s", outputFilename.c_str());
      seeded = true;
    }
    if (ImGui::InputText("##OutputFilename", outputFilenameBuf, sizeof(outputFilenameBuf))) {
      outputFilename = outputFilenameBuf;
    }

    //output format
    const char* formatOptions[] = { "txt", "csv" };
    static int formatIndex = 0; // 0 = txt, 1 = csv -- kept in sync with outputFormat below
    ImGui::Text("Output format:");
    if (ImGui::Combo("##Output format", &formatIndex, formatOptions, IM_ARRAYSIZE(formatOptions))) {
      outputFormat = (formatIndex == 0) ? OutputFormat::TXT : OutputFormat::CSV;
    }

    //output delimiter inputbox
    static char outputDelimiterBuf[128] = "";
    ImGui::Text("Delimiter:");
    ImGui::SameLine();
    seeded = false;
    if (!seeded) {
      snprintf(outputDelimiterBuf, sizeof(outputDelimiterBuf), "%s", outputDelimiter.c_str());
      seeded = true;
    }
    ImGui::SetNextItemWidth(30.0f);
    if (ImGui::InputText("##OutputDelimiter", outputDelimiterBuf, sizeof(outputDelimiterBuf))) {
      outputDelimiter = outputDelimiterBuf;
    }

    //output sepetrator
    static char outputSeperatorBuf[128] = "";
    ImGui::SameLine();
    ImGui::Text("Seperator:");
    ImGui::SameLine();
    seeded = false;
    if (!seeded) {
      snprintf(outputSeperatorBuf, sizeof(outputSeperatorBuf), "%s", outputSeperator.c_str());
      seeded = true;
    }
    ImGui::SetNextItemWidth(30.0f);
    if (ImGui::InputText("##OutputSeperator", outputSeperatorBuf, sizeof(outputSeperatorBuf))) {
      outputSeperator = outputSeperatorBuf;
    }

    std::string finalFilenameString = "Filename: " + outputFilename + "." + formatOptions[formatIndex];
    ImGui::Text(finalFilenameString.c_str());
    ImGui::End();


    //MEASUREMENT -------------------------------------------------------------------------------------------------------------
    ImGui::SetNextWindowPos(ImVec2(10, 320), ImGuiCond_Once);
    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_Once);
    ImGui::Begin("MEASUREMENT", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize| ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);
    std::string measuremetStatus = "idle";
    if (isMeasurementRunning) {
      measuremetStatus = "running";
    }

    ImGui::Text(("Status: " + measuremetStatus).c_str());

    if (ImGui::Button("START")) {
      isMeasurementRunning = true; //todo
    }
    if (isMeasurementRunning) {
    ImGui::SameLine();
      if (ImGui::Button("STOP")) {
        isMeasurementRunning = false;
      }
    }
    ImGui::End();

    /// ------------------------------------------------------------------------------------------------------------
    /// GUI /\ /\ /\ /\
    /// ------------------------------------------------------------------------------------------------------------

    ImGui::Render();
    glViewport(0, 0, (int)ImGui::GetIO().DisplaySize.x, (int)ImGui::GetIO().DisplaySize.y);

    //background light mode
    if (isLightMode) {
      glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    }else glClearColor(0.1f, 0.1f, 0.12f, 1.0f);

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