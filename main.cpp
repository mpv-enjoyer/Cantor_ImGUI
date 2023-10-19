#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>
#include <iostream>
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#define MAX_STEP     20
#define WARNING_STEP 15

void generate_plus_null(int horisontal_steps, int current_position, bool* output_function) //Recursive function to generate monotonous sections
{
    if (horisontal_steps == 1)
    {
        output_function[current_position] = false;
        return;
    }
    generate_plus_null(horisontal_steps / 3, current_position, output_function);
    for (int i = current_position + horisontal_steps / 3; i < current_position + 2 * horisontal_steps / 3; i++)
    {
        output_function[i] = true;
    }
    generate_plus_null(horisontal_steps / 3, current_position + 2 * horisontal_steps / 3, output_function);
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Generic setup stuff
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Cantor", nullptr, nullptr);
    if (window == nullptr) return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Setting up values
    float* function = (float*)IM_ALLOC(sizeof(float) * 4);
    function[0] = 0.0f; function[1] = 0.5f; function[2] = 0.5f; function[3] = 1.0f;
    int size = 3;
    int display_count = 1;
    bool warning_shown = false;
    bool warning_active = false;
    int pre_warning_value = 1;
    int post_warning_value = 1;
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Actual code
        {
            if (warning_active)
            {
                ImGui::OpenPopup("Warning!");
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
                if (ImGui::BeginPopupModal("Warning!", NULL, ImGuiWindowFlags_AlwaysAutoResize))
                {
                    ImGui::Text("Your computer may hang.");
                    if (ImGui::Button("OK."))
                    {
                        display_count = post_warning_value;
                        warning_active = false;
                        warning_shown = true;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel.")) 
                    {
                        display_count = pre_warning_value;
                        warning_active = false;
                    }
                    if (!warning_active) ImGui::CloseCurrentPopup();
                    ImGui::EndPopup();
                }
            }
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::Begin("Cantor", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
            int pre_step_value = display_count;
            if (ImGui::SliderInt("Step count", &display_count, 1, MAX_STEP, "%d", ImGuiSliderFlags_NoInput))
            {
                if (display_count > WARNING_STEP && !warning_shown)
                {
                    pre_warning_value = pre_step_value;
                    post_warning_value = display_count;
                    warning_active = true;
                }
                else
                {
                    free(function);
                    size = pow(3, display_count);
                    function = (float*)IM_ALLOC(sizeof(float) * pow(3, display_count) + 1);
                    bool* plus_null = (bool*)IM_ALLOC(sizeof(bool) * pow(3, display_count));
                    generate_plus_null(pow(3, display_count), 0, plus_null);
                    float current_value = 0;
                    function[0] = 0.0f; // We always store one extra null value so our plot doesn't start from nowhere
                    for (int i = 0; i < size; i++)
                    {
                        plus_null[i] ? current_value : current_value += (1.0f / pow(2, display_count));
                        function[i + 1] = current_value;
                    }
                    free(plus_null);
                }

            }
            ImGui::SameLine();
            ImGui::Text("| %.1f FPS.", io.Framerate);
            ImGui::PlotLines("##Lines", function, size + 1, 0, NULL, 0.0f, 1.0f, ImVec2(-FLT_MIN, -FLT_MIN));
            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}