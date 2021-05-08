// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// (GL3W is a helper library to access OpenGL functions since there is no standard header to access modern OpenGL functions easily. Alternatives are GLEW, Glad, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#include <SDL.h>



#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif



//my stuff
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "tree.cpp"

#include <cstdio>
#include <iostream>
#include <fstream>
#include <string>

void drawGeometry(int shaderProgram, int model1_start, int model1_numVerts, int model2_start, int model2_numVerts);

using namespace std;

// Shader sources
const GLchar* vertexSource =
    "#version 150 core\n"
    "in vec3 position;"
    "in vec3 inColor;"
    "out vec3 Color;"
    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 proj;"
    "void main() {"
    "   Color = inColor;"
    "   gl_Position = proj * view * model * vec4(position,1.0);"
    "}";

const GLchar* fragmentSource =
    "#version 150 core\n"
    "in vec3 Color;"
    "out vec4 outColor;"
    "void main() {"
    "   outColor = vec4(Color, 1.0);"
    "}";


int screenWidth = 1500;
int screenHeight = 800;
float timePast = 0;

//SJG: Store the object coordinates
//You should have a representation for the state of each object
float objx=0, objy=0, objz=0;
float colR=1, colG=1, colB=1;


//camera
glm::vec3 cameraPos = glm::vec3(0.0f, 1.8f, -0.8f);
glm::vec3 cameraFront = glm::vec3(0.0f, -0.8f, 0.8f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.0f, 1.0f);

//glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); 

//glm::mat4 view = glm::lookAt(
      //glm::vec3(0.0f, 1.8f, -0.8f),  //Cam Position
      //glm::vec3(0.0f, 1.0f, 0.0f),  //Look at point
      //glm::vec3(0.0f, 0.0f, 1.0f)); //Up




float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool DEBUG_ON = true;
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName);
bool fullscreen = false;
void Win2PPM(int width, int height);

vector<tree> trees;

//srand(time(NULL));
float rand01(){
	return rand()/(float)RAND_MAX;
}

// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
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
    const char* glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
#endif

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL3 example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("Version:  %s\n\n", glGetString(GL_VERSION));

		////////////////////////////////////////////////////////////////// MATT'S PORTION /////////////////////////////////////////////////////////////////////////
		//Load the vertex Shader
	  GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	  glShaderSource(vertexShader, 1, &vertexSource, NULL);
	  glCompileShader(vertexShader);

	  //Let's double check the shader compiled
	  GLint status;
	  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
	  if (!status){
	    char buffer[512];
	    glGetShaderInfoLog(vertexShader, 512, NULL, buffer);
	    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
	                        "Compilation Error",
	                        "Failed to Compile: Check Consol Output.",
	                        NULL);
	    printf("Vertex Shader Compile Failed. Info:\n\n%s\n",buffer);
	  }

	  GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	  glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	  glCompileShader(fragmentShader);

	  //Double check the shader compiled
	  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
	  if (!status){
	    char buffer[512];
	    glGetShaderInfoLog(fragmentShader, 512, NULL, buffer);
	    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,
	                        "Compilation Error",
	                        "Failed to Compile: Check Consol Output.",
	                        NULL);
	    printf("Fragment Shader Compile Failed. Info:\n\n%s\n",buffer);
	  }

		GLuint shaderProgram = glCreateProgram();
	  glAttachShader(shaderProgram, vertexShader);
	  glAttachShader(shaderProgram, fragmentShader);
	  glBindFragDataLocation(shaderProgram, 0, "outColor"); // set output
	  glLinkProgram(shaderProgram); //run the linker

	  GLint posAttrib = glGetAttribLocation(shaderProgram, "position");
	  GLint colAttrib = glGetAttribLocation(shaderProgram, "inColor");

	  map<char, string> rules = { {'F', "F[+F]F[-F]F"} };
	  map<char, string> rules1 = { {'F', "F[-F][+F]"} };
	  map<char, string> rules2 = { {'F', "FF[-F][-F][+F]F"} };
	  map<char, string> rules3 = { {'F', "F[+F]F[-F][F]"} };
	  vector<pair<char, float>> start;
	  start.push_back(make_pair('F', 0.0f));
	  trees.push_back(tree(start, rules1, glm::vec3(0, 0, 0), 10, 0.1));


    //Build a Vertex Array Object (VAO) to store mapping of shader attributse to VBO
	/*GLuint vao;
	glGenVertexArrays(1, &vao); //Create a VAO
	glBindVertexArray(vao); //Bind the above created VAO to the current context

	//Allocate memory on the graphics card to store geometry (vertex buffer object)
	GLuint vbo[1];
	glGenBuffers(1, vbo);  //Create 1 buffer called vbo
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]); //Set the vbo as the active array buffer (Only one buffer can be active at a time)
	glBufferData(GL_ARRAY_BUFFER, totalNumVerts*8*sizeof(float), modelData, GL_STATIC_DRAW); //upload vertices to vbo
	//GL_STATIC_DRAW means we won't change the geometry, GL_DYNAMIC_DRAW = geometry changes infrequently
	//GL_STREAM_DRAW = geom. changes frequently.  This effects which types of GPU memory is used

	int texturedShader = InitShader("textured-Vertex.glsl", "textured-Fragment.glsl");*/

	//Tell OpenGL how to set fragment shader input
	/*GLint posAttrib = glGetAttribLocation(texturedShader, "position");
	glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), 0);
	  //Attribute, vals/attrib., type, isNormalized, stride, offset
	glEnableVertexAttribArray(posAttrib);

	//GLint colAttrib = glGetAttribLocation(phongShader, "inColor");
	//glVertexAttribPointer(colAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
	//glEnableVertexAttribArray(colAttrib);

	GLint normAttrib = glGetAttribLocation(texturedShader, "inNormal");
	glVertexAttribPointer(normAttrib, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(5*sizeof(float)));
	glEnableVertexAttribArray(normAttrib);

	GLint texAttrib = glGetAttribLocation(texturedShader, "inTexcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));

	GLint uniView = glGetUniformLocation(texturedShader, "view");
	GLint uniProj = glGetUniformLocation(texturedShader, "proj");

	glBindVertexArray(0); //Unbind the VAO in case we want to create a new one*/


	glEnable(GL_DEPTH_TEST);




    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    bool show_demo_window = false;
    //bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	//color stuff
	float col1[3] = { 0.5f, 0.27f, 0.07f };

	//growth stuff
	 static float growthv = 0.4f;
	 static float lastgrowthv = 0.4f;


    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;

        //timePast = SDL_GetTicks()/1000.f;
		float currentFrame = SDL_GetTicks()/1000.f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        while (SDL_PollEvent(&event))
        {
			float cameraSpeed = 2.0f * deltaTime;
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;

            //player movement and camera movement (implemented on keyboard only)
			if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_w){ //If "w" is pressed
				cameraPos += cameraSpeed * cameraFront;
			}
			if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_s){ //If "s" is pressed
				cameraPos -= cameraSpeed * cameraFront;
			}
			if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_a){ //If "a" is pressed
				if (event.key.keysym.mod & KMOD_SHIFT) cameraFront = cameraFront + glm::vec3(-0.05f,0.0f,0.0f); //Is shift pressed?
				else cameraPos -= glm::normalize(glm::cross(cameraFront,cameraUp)) * cameraSpeed;
			}
			if (event.type == SDL_KEYUP && event.key.keysym.sym == SDLK_d){ //If "d" is pressed
				if (event.key.keysym.mod & KMOD_SHIFT) cameraFront = cameraFront + glm::vec3(0.05f,0.0f,0.0f); //Is shift pressed?
				else cameraPos += glm::normalize(glm::cross(cameraFront,cameraUp)) * cameraSpeed;
			}
        }

				///////////////////////////////////////////////////////////////// MATT'S PORTION AGAIN //////////////////////////////////////////////////////////////////////////

				/*glm::mat4 view = glm::lookAt(
			      glm::vec3(0.0f, 1.8f, -0.8f),  //Cam Position
			      glm::vec3(0.0f, 1.0f, 0.0f),  //Look at point
			      glm::vec3(0.0f, 0.0f, 1.0f)); //Up
			    GLint uniView = glGetUniformLocation(shaderProgram, "view");
			    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

			    glm::mat4 proj = glm::perspective(3.14f/4, 800.0f / 600.0f, 0.5f, 10.0f); //FOV, aspect, near, far
			    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
			    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

				GLint uniModel = glGetUniformLocation(shaderProgram, "model");

				for (int i = 0; i < trees.size(); i++) {
					trees[i].render(shaderProgram, uniModel, view, proj);
				}*/

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {


            //parameters for gui widgets

            static float sizepar = 0.0f;
            static ImVec4 color = ImVec4(114.0f / 255.0f, 144.0f / 255.0f, 154.0f / 255.0f, 200.0f / 255.0f);
            //static int counter = 0;

            ImGui::Begin("Procedural Tree Generation GUI");

            ImGui::Text("here are some tools you can use!");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("imgui library demo", &show_demo_window);      // Edit bools storing our window open/close state

            ImGui::SliderFloat("tree growth variance", &growthv, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f



            /*ImGui::Text("Color widget:");
			ImGui::SameLine(); HelpMarker(
            "Click on the color square to open a color picker.\n"
            "CTRL+click on individual component to input value.\n");
			ImGui::ColorEdit3("MyColor##1", (float*)&color, misc_flags);
			*/

			ImGui::Text("Add or Delete Tree: ");
			ImGui::SameLine();
			if (ImGui::Button(" + ")) {
                float choose = rand()%100 + 1;
                float lrbound = rand()%10;
                float negchoose = rand()%100;
                if(negchoose > 50){
					lrbound = lrbound * -1;
				}
				float bbound = rand()%33;
                if(choose >= 1 && choose <= 25){
					trees.push_back(tree(start, rules, glm::vec3(lrbound/10, 0, bbound/10), 10, 0.1));
				}
				if(choose >= 26 && choose <= 50){
					trees.push_back(tree(start, rules1, glm::vec3(lrbound/10, 0, bbound/10), 10, 0.1));
				}
				if(choose >= 51 && choose <= 75){
					trees.push_back(tree(start, rules2, glm::vec3(lrbound/10, 0, bbound/10), 10, 0.1));
				}
				if(choose >= 76 && choose <= 100){
					trees.push_back(tree(start, rules3, glm::vec3(lrbound, 0, bbound), 10, 0.1));
				}
            }
            ImGui::SameLine();
            if (ImGui::Button(" - ")) {
                trees.pop_back();
            }
            ImGui::ColorEdit3("color 1", col1);


            if (ImGui::Button("close application")) {
                done = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("reset environment")) {
				trees.clear();
			}
			ImGui::SameLine();
            if (ImGui::Button("generate environment")) {
				if(trees.empty()){
				trees.push_back(tree(start, rules1, glm::vec3(0, 0, 0), 10, 0.1));
				}

			}
			ImGui::SameLine();
			if (ImGui::Button("pause growth")) {
				lastgrowthv = growthv;
				growthv = 0;
			}
			ImGui::SameLine();
			if (ImGui::Button("continue growth")) {
				growthv = lastgrowthv;
				if(lastgrowthv == 0){
					growthv = 0.4;
				}
			}


            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }



        // 3. Show another simple window.
        //if (show_another_window)
        //{
            //ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            //ImGui::Text("Hello from another window!");
            //if (ImGui::Button("Close Me"))
                //show_another_window = false;
            //ImGui::End();
        //}

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				// Clear the screen to default color
    //glClearColor(.2f, 0.4f, 0.8f, 1.0f);
  //  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float time = SDL_GetTicks()/1000.f;
    /*glm::mat4 model = glm::mat4(1);
    model = glm::rotate(model,time * 3.14f/2,glm::vec3(0.0f, 1.0f, 1.0f));
    model = glm::rotate(model,time * 3.14f/4,glm::vec3(1.0f, 0.0f, 0.0f));
    GLint uniModel = glGetUniformLocation(shaderProgram, "model");
    glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));*/

    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); 
    GLint uniView = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

    glm::mat4 proj = glm::perspective(3.14f/4, 800.0f / 600.0f, 0.5f, 10.0f); //FOV, aspect, near, far
    GLint uniProj = glGetUniformLocation(shaderProgram, "proj");
    glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));

	GLint uniModel = glGetUniformLocation(shaderProgram, "model");
	for (int i = 0; i < trees.size(); i++) {
		trees[i].color[0] = col1[0];
		trees[i].color[1] = col1[1];
		trees[i].color[2] = col1[2];
		trees[i].growthSpeed = growthv;
		trees[i].render(shaderProgram, uniModel, view, proj);
	}

    //glBindVertexArray(vao);  //Bind the VAO for the shader(s) we are using
    //glDrawArrays(GL_TRIANGLES, 0, 36); //Number of vertices
    //Win2PPM(screen_width,screen_height); //Will save an image

    //SDL_GL_SwapWindow(window); //Double buffering

    //float t_end = SDL_GetTicks();
		char update_title[100];
		/*float time_per_frame = t_end-t_start;
		avg_render_time = .98*avg_render_time + .02*time_per_frame; //Weighted average for smoothing*/
		//sprintf_s(update_title,"%s [Update: %3.0f ms]\n",window_title,avg_render_time);
		SDL_SetWindowTitle(window, update_title);







        //my stuff
		/*glUseProgram(texturedShader);




		//camera
		glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

		/*glm::mat4 view = glm::lookAt(
		glm::vec3(3.f, 0.f, 0.f),  //Cam Position
		glm::vec3(0.0f, 0.0f, 0.0f),  //Look at point
		glm::vec3(0.0f, 0.0f, 1.0f)); //Up

		glUniformMatrix4fv(uniView, 1, GL_FALSE, glm::value_ptr(view));

		glm::mat4 proj = glm::perspective(3.14f/4, screenWidth / (float) screenHeight, 1.0f, 10.0f); //FOV, aspect, near, far
		glUniformMatrix4fv(uniProj, 1, GL_FALSE, glm::value_ptr(proj));


		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex0);
		glUniform1i(glGetUniformLocation(texturedShader, "tex0"), 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, tex1);
		glUniform1i(glGetUniformLocation(texturedShader, "tex1"), 1);

		glBindVertexArray(vao);
		//drawGeometry(texturedShader, startVertTeapot, numVertsTeapot, startVertKnot, numVertsKnot);*/
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		SDL_GL_SwapWindow(window); //Double buffering





    }

    // Cleanup
		glDeleteProgram(shaderProgram);
	  glDeleteShader(fragmentShader);
	  glDeleteShader(vertexShader);

	  for (int i = 0; i < trees.size(); i++) {
		  glDeleteBuffers(1, &trees.at(i).vao);
		  glDeleteBuffers(1, &trees.at(i).vbo);
	  }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}




void drawGeometry(int shaderProgram, int model1_start, int model1_numVerts, int model2_start, int model2_numVerts){

	GLint uniColor = glGetUniformLocation(shaderProgram, "inColor");
	glm::vec3 colVec(colR,colG,colB);
	glUniform3fv(uniColor, 1, glm::value_ptr(colVec));

  GLint uniTexID = glGetUniformLocation(shaderProgram, "texID");

	//************
	//Draw model #1 the first time
	//This model is stored in the VBO starting a offest model1_start and with model1_numVerts num of verticies
	//*************

	//Rotate model (matrix) based on how much time has past
	glm::mat4 model = glm::mat4(1);
	model = glm::rotate(model,timePast * 3.14f/2,glm::vec3(0.0f, 1.0f, 1.0f));
	model = glm::rotate(model,timePast * 3.14f/4,glm::vec3(1.0f, 0.0f, 0.0f));
	//model = glm::scale(model,glm::vec3(.2f,.2f,.2f)); //An example of scale
	GLint uniModel = glGetUniformLocation(shaderProgram, "model");
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model)); //pass model matrix to shader

	//Set which texture to use (-1 = no texture)
	glUniform1i(uniTexID, -1);

	//Draw an instance of the model (at the position & orientation specified by the model matrix above)
	glDrawArrays(GL_TRIANGLES, model1_start, model1_numVerts); //(Primitive Type, Start Vertex, Num Verticies)


	//************
	//Draw model #1 the second time
	//This model is stored in the VBO starting a offest model1_start and with model1_numVerts num. of verticies
	//*************

	//Translate the model (matrix) left and back
	model = glm::mat4(1); //Load intentity
	model = glm::translate(model,glm::vec3(-2,1,-.4));
	//model = glm::scale(model,2.f*glm::vec3(1.f,1.f,0.5f)); //scale example
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	//Set which texture to use (0 = wood texture ... bound to GL_TEXTURE0)
	glUniform1i(uniTexID, 0);

  //Draw an instance of the model (at the position & orientation specified by the model matrix above)
	glDrawArrays(GL_TRIANGLES, model1_start, model1_numVerts); //(Primitive Type, Start Vertex, Num Verticies)

	//************
	//Draw model #2 once
	//This model is stored in the VBO starting a offest model2_start and with model2_numVerts num of verticies
	//*************

	//Translate the model (matrix) based on where objx/y/z is
	// ... these variables are set when the user presses the arrow keys
	model = glm::mat4(1);
	model = glm::scale(model,glm::vec3(100.f,1.f,100.f)); //scale this model
	model = glm::translate(model,glm::vec3(0.0f,-1.5f,0.0f));

	//Set which texture to use (1 = brick texture ... bound to GL_TEXTURE1)
	glUniform1i(uniTexID, 0);
	glUniformMatrix4fv(uniModel, 1, GL_FALSE, glm::value_ptr(model));

	//Draw an instance of the model (at the position & orientation specified by the model matrix above)
	glDrawArrays(GL_TRIANGLES, model2_start, model2_numVerts); //(Primitive Type, Start Vertex, Num Verticies)
}

// Create a NULL-terminated string by reading the provided file
static char* readShaderSource(const char* shaderFile){
	FILE *fp;
	long length;
	char *buffer;

	// open the file containing the text of the shader code
	fp = fopen(shaderFile, "r");

	// check for errors in opening the file
	if (fp == NULL) {
		printf("can't open shader source file %s\n", shaderFile);
		return NULL;
	}

	// determine the file size
	fseek(fp, 0, SEEK_END); // move position indicator to the end of the file;
	length = ftell(fp);  // return the value of the current position

	// allocate a buffer with the indicated number of bytes, plus one
	buffer = new char[length + 1];

	// read the appropriate number of bytes from the file
	fseek(fp, 0, SEEK_SET);  // move position indicator to the start of the file
	fread(buffer, 1, length, fp); // read all of the bytes

	// append a NULL character to indicate the end of the string
	buffer[length] = '\0';

	// close the file
	fclose(fp);

	// return the string
	return buffer;
}

// Create a GLSL program object from vertex and fragment shader files
GLuint InitShader(const char* vShaderFileName, const char* fShaderFileName){
	GLuint vertex_shader, fragment_shader;
	GLchar *vs_text, *fs_text;
	GLuint program;

	// check GLSL version
	printf("GLSL version: %s\n\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

	// Create shader handlers
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

	// Read source code from shader files
	vs_text = readShaderSource(vShaderFileName);
	fs_text = readShaderSource(fShaderFileName);

	// error check
	if (vs_text == NULL) {
		printf("Failed to read from vertex shader file %s\n", vShaderFileName);
		exit(1);
	} else if (DEBUG_ON) {
		printf("Vertex Shader:\n=====================\n");
		printf("%s\n", vs_text);
		printf("=====================\n\n");
	}
	if (fs_text == NULL) {
		printf("Failed to read from fragent shader file %s\n", fShaderFileName);
		exit(1);
	} else if (DEBUG_ON) {
		printf("\nFragment Shader:\n=====================\n");
		printf("%s\n", fs_text);
		printf("=====================\n\n");
	}

	// Load Vertex Shader
	const char *vv = vs_text;
	glShaderSource(vertex_shader, 1, &vv, NULL);  //Read source
	glCompileShader(vertex_shader); // Compile shaders

	// Check for errors
	GLint  compiled;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		printf("Vertex shader failed to compile:\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(vertex_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(vertex_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Load Fragment Shader
	const char *ff = fs_text;
	glShaderSource(fragment_shader, 1, &ff, NULL);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);

	//Check for Errors
	if (!compiled) {
		printf("Fragment shader failed to compile\n");
		if (DEBUG_ON) {
			GLint logMaxSize, logLength;
			glGetShaderiv(fragment_shader, GL_INFO_LOG_LENGTH, &logMaxSize);
			printf("printing error message of %d bytes\n", logMaxSize);
			char* logMsg = new char[logMaxSize];
			glGetShaderInfoLog(fragment_shader, logMaxSize, &logLength, logMsg);
			printf("%d bytes retrieved\n", logLength);
			printf("error message: %s\n", logMsg);
			delete[] logMsg;
		}
		exit(1);
	}

	// Create the program
	program = glCreateProgram();

	// Attach shaders to program
	glAttachShader(program, vertex_shader);
	glAttachShader(program, fragment_shader);

	// Link and set program to use
	glLinkProgram(program);

	return program;
}
