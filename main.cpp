#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h>
#include <math.h>
#include <vector>
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

std::vector<bool> generate_plus_null(int horisontal_steps) //Recursive function to generate monotonous sections
{
    std::vector<bool> output;
    if (horisontal_steps == 1)
    {
        output.push_back(false);
        return output;
    }
    std::vector<bool> left_side = generate_plus_null(horisontal_steps / 3);
    for (int i = 0; i < horisontal_steps / 3; i++) output.push_back(left_side.at(i));
    for (int i = 0; i < horisontal_steps / 3; i++) output.push_back(true);
    for (int i = 0; i < horisontal_steps / 3; i++) output.push_back(left_side.at(left_side.size() - i - 1));
    return output;
}

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    // Generic setup stuff
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Cantor", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // Actual code
        {
            static int display_count = 1;
            static std::vector<bool> plus_null = {false, true, false};
            static std::vector<float> current_integral_function = {0.5f, 0.5f, 1.0f};
            static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::Begin("Cantor", nullptr, flags);
            struct Funcs
            {
                static float KantorLines(void*, int i)
                {
                    if (i==0) return 0.0f;
                    i-=1;
                    return current_integral_function[i];
                }
            };
            if (ImGui::SliderInt("Step count", &display_count, 1, 15))
            {
                plus_null = generate_plus_null(pow(3, display_count));
                float current = 0;
                current_integral_function = std::vector<float>();
                for (int i = 0; i < plus_null.size(); i++)
                {
                    plus_null[i] ? current : current += 1.0f/pow(2,display_count);
                    current_integral_function.push_back(current);
                }
            }
            ImGui::SameLine();
            ImGui::Text(": %.1f FPS.", io.Framerate);
            float (*func)(void*, int) = Funcs::KantorLines;
            if (current_integral_function.size() == pow(3, display_count))
            {
                ImGui::PlotLines("##Lines", func, NULL, pow(3, display_count) + 1, 0, NULL, 0.0f, 1.0f, ImVec2(-FLT_MIN, -FLT_MIN));
            }
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
