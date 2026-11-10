#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <array>

int main()
{
	static float masterGain = 0.1f;

	// Initialize the GLFW library.
	// Must be called before using any other GLFW functions.
	glfwInit();

	// Create a window with a size of 1280x720 pixels.
	// "DSP Playground" will appear in the window title bar.
	// nullptr indicates windowed mode (not fullscreen)
	// and no shared OpenGL context.
	GLFWwindow* window = glfwCreateWindow(
		1280, 720, "DSP Playground", nullptr, nullptr
	);

	// Make the window's OpenGL context current.
	// All subsequent OpenGL calls will affect this window
	glfwMakeContextCurrent(window);

	// Verify that the compiled ImGui version matches
	// the version of the linked ImGui library.
	IMGUI_CHECKVERSION();

	// Create the main ImGui context.
	// This must be done before using any ImGui features.
	ImGui::CreateContext();

	// Initialize the GLFW backend for ImGui.
	// The second parameter enables automatic installation
	// of keyboard and mouse input callbacks.
	ImGui_ImplGlfw_InitForOpenGL(window, true);

	// Initialize the OpenGL 3 renderer backend for ImGui.
	// "#version 130" specifies the GLSL shader version to use.
	ImGui_ImplOpenGL3_Init("#version 130");


    std::array<float, 512> waveform{};

	while (!glfwWindowShouldClose(window))
	{
		// Process pending window, keyboard, and mouse events.
		glfwPollEvents();

		// Start a new frame for the OpenGL renderer backend.
		ImGui_ImplOpenGL3_NewFrame();

		// Start a new frame for the GLFW platform backend.
		ImGui_ImplGlfw_NewFrame();

		// Begin a new ImGui frame.
		// All UI widgets for this frame are created after this call.
		ImGui::NewFrame();

        // Set content window size.
        ImGui::SetNextWindowSize(
            ImVec2(800.0f, 600.0f),
            ImGuiCond_FirstUseEver
        );

		// Begin a new ImGui window named "DSP Playground".
		ImGui::Begin("DSP Playground");
        
        // Waveform visualization.
        ImGui::PlotLines(
            "Waveform",
            waveform.data(),
            static_cast<int>(waveform.size()),
            0,
            nullptr,
            -1.0f,
            1.0f,
            ImVec2(0.0f, 120.0f)
        );

        // Play Audio.
		ImGui::Button("Play");

        // Pause Audio.
        ImGui::Button("Pause");

        // Stop Audio.
        ImGui::Button("Stop");

        // Manipulate Master Gain.
		ImGui::SliderFloat(
			"Master Gain", &masterGain, 0.0f, 1.0f
		);

        // Export to WAV file.
        ImGui::Button("Export to WAV file.");

        // Summary of Google Benchmark.
        ImGui::Text("Summary of Google Benchmark.");

		// Finish building the ImGui window.
		ImGui::End();

		// Generate the final draw data for all ImGui widgets.
		ImGui::Render();

		// Get the current framebuffer size.
		// This may differ from the window size on HiDPI displays.
		int width;
		int height;

		glfwGetFramebufferSize(
			window, &width, &height
		);

		// Set the OpenGL viewport to cover the entire framebuffer.
		glViewport(
			0, 0, width, height
		);

		// Clear the screen using the current clear color.
		glClear(GL_COLOR_BUFFER_BIT);

		// Render all ImGui draw commands using OpenGL.
		ImGui_ImplOpenGL3_RenderDrawData(
			ImGui::GetDrawData()
		);

		// Present the rendered frame to the screen.
		// This swaps the back buffer and front buffer.
		glfwSwapBuffers(window);
	}

	// Shut down the ImGui OpenGL 3 renderer backend.
	// Releases OpenGL-related resources used by ImGui.
	ImGui_ImplOpenGL3_Shutdown();

	// Shut down the ImGui GLFW platform backend.
	// Cleans up input handling and GLFW-related resources.
	ImGui_ImplGlfw_Shutdown();

	// Destroy the main ImGui context.
	// Frees all memory allocated by ImGui.
	ImGui::DestroyContext();

	// Destroy the GLFW window and its associated OpenGL context.
	glfwDestroyWindow(window);

	// Terminate the GLFW library.
	// Must be called before the application exits.
	glfwTerminate();
}
