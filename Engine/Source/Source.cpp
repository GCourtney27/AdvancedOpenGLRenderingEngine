
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#include "Objects/Geometry/Model.h"
#include "Objects/Camera/Camera.h"
#include "Objects/Lights/Lights.h"

#define ThrowError(x) throw std::runtime_error(x)

#define IMGUI_NEW_FRAME \
	ImGui_ImplOpenGL3_NewFrame();\
	ImGui_ImplGlfw_NewFrame();\
	ImGui::NewFrame();\

#define IMGUI_RENDER \
	ImGui::Render();\
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());\

// Window Params
//--------------
int g_windowWidth = 1600;
int g_windowHeight = 900;
int g_msaaSamples = 4;

// Verticies
//----------
float skyboxVertices[] = {
	// positions          

	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	-1.0f, -1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f
};

float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
};

float planeVertices[] = {
	// positions            // normals         // texcoords
	 25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,  25.0f,  0.0f,
	-25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,   0.0f,  0.0f,
	-25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,   0.0f, 25.0f,

	 25.0f, -0.5f,  25.0f,  0.0f, -1.0f, 0.0f,  25.0f,  0.0f,
	-25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,   0.0f, 25.0f,
	 25.0f, -0.5f, -25.0f,  0.0f, -1.0f, 0.0f,  25.0f, 25.0f
};

// Lights
//-------
PointLight pointLight;
SpotLight spotLight;
DirectionalLight dirLight;

// Objects
//--------
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
Model fileModel;

// Buffers and Textures
//---------------------
unsigned int floorDiffTextureGammaCorrected;
unsigned int floorSpecTextureGammaCorrected;
unsigned int floorNormTextureGammaCorrected;
unsigned int brickDiffTextureGammaCorrected;
unsigned int brickNormalTextureGammaCorrected;
unsigned int brickDepthTextureGammaCorrected;
unsigned int cubemapTexture;

// Mouse
//------
float lastX = g_windowWidth / 2.0f;
float lastY = g_windowHeight / 2.0f;
bool g_firstMouse = true;

// Time
//-----
float g_deltaTime = 0.0f; // Time between current frame and last frame
float g_lastFrame = 0.0f; // Time of last frame

// GLFW Callbacks
//---------------
void frame_buffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

// Method Signatures
//------------------
unsigned int loadTexture(char const * path, bool gammaCorrection);
unsigned int loadCubeMapTextures(std::vector<std::string> faces);
void InitSkybox(unsigned int & skyVAO, unsigned int & skyboxVBO);
void ProcessInput(GLFWwindow* pWindow);
void RenderScene(const Shader& shader);
void renderCube();
void renderFloorQuad();
void CleanUp();

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, g_msaaSamples);

	GLFWwindow* pWindow = glfwCreateWindow(g_windowWidth, g_windowHeight, "OpenGL Engine!", NULL, NULL);
	if (pWindow == nullptr)
	{
		ThrowError("Failed to initialize glfw window!");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(pWindow);

	// Register callback functions
	glfwSetFramebufferSizeCallback(pWindow, frame_buffer_size_callback);
	glfwSetCursorPosCallback(pWindow, mouse_callback);
	glfwSetScrollCallback(pWindow, scroll_callback);

	//glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // To disable cursor, uncomment this
	gladLoadGL();
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(pWindow, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ThrowError("Failed to initialize GLAD!");
		return -1;
	}
	glEnable(GL_MULTISAMPLE);

	glViewport(0, 0, g_windowWidth, g_windowHeight);

	// Skybox VAO/VBO
	unsigned int skyboxVAO, skyboxVBO;
	InitSkybox(skyboxVAO, skyboxVBO);

	// Screen quad VAO/VBO
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	// configure (floating point) framebuffers
	// ---------------------------------------
	unsigned int hdrFBO;
	glGenFramebuffers(1, &hdrFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	// create 2 floating point color buffers (1 for normal rendering, other for brightness treshold values)
	unsigned int colorBuffers[2];
	glGenTextures(2, colorBuffers);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_windowWidth, g_windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// attach texture to framebuffer
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
	}
	// create and attach depth buffer (renderbuffer)
	unsigned int rboDepth;
	glGenRenderbuffers(1, &rboDepth);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, g_windowWidth, g_windowHeight);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Ping-pong framebuffers
	unsigned int pingpongFBO[2];
	unsigned int pingpongBuffer[2];
	glGenFramebuffers(2, pingpongFBO);
	glGenTextures(2, pingpongBuffer);
	for (unsigned int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_windowWidth, g_windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
	}
	Shader screenShader("Shaders/ScreenQuadPostProcess.vert", "Shaders/ScreenQuadPostProcess.frag");
	screenShader.SetInt("screenTexture", 0);
	screenShader.SetInt("bloomTexture", 1);


	// Generate shadow map frame buffer
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int shadowMap;
	glGenTextures(1, &shadowMap);
	glBindTexture(GL_TEXTURE_2D, shadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	Shader lightingDepthShader("Shaders/lightDepthPass.vert", "Shaders/lightDepthPass.frag");

	// Create cube map
	std::vector<std::string> faces{
		"../Assets//Skyboxes/Lake/right.jpg",
		"../Assets//Skyboxes/Lake/left.jpg",
		"../Assets//Skyboxes/Lake/top.jpg",
		"../Assets//Skyboxes/Lake/bottom.jpg",
		"../Assets//Skyboxes/Lake/front.jpg",
		"../Assets//Skyboxes/Lake/back.jpg",
	};
	cubemapTexture = loadCubeMapTextures(faces);

	// Compile shaders
	Shader lightingShader("Shaders/VertexShader.vert", "Shaders/FragmentShader.frag");
	Shader lampShader("Shaders/lamp.vert", "Shaders/lamp.frag");
	Shader skyboxShader("Shaders/Skybox.vert", "Shaders/Skybox.frag");
	Shader blurShader("Shaders/ScreenQuadPostProcess.vert", "Shaders/GaussianBlur.frag");
	blurShader.Use();
	blurShader.SetInt("image", 0);
	
	unsigned int ubiLightingShader = glGetUniformBlockIndex(lightingShader.ProgramID, "Matrices");
	glUniformBlockBinding(lightingShader.ProgramID, ubiLightingShader, 0);
	unsigned int uboMatrices;
	glGenBuffers(1, &uboMatrices);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));
	glm::mat4 projectionMat = glm::perspective(glm::radians(camera.Zoom), (float)g_windowWidth / (float)g_windowHeight, 0.1f, 100.0f);
	glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projectionMat));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
	floorDiffTextureGammaCorrected = loadTexture("../Assets/Textures/Planks_Diff.png", true);
	floorNormTextureGammaCorrected = loadTexture("../Assets/Textures/Planks_Norm.png", false);
	floorSpecTextureGammaCorrected = loadTexture("../Assets/Textures/Planks_Spec.png", true);

	brickDiffTextureGammaCorrected = loadTexture("../Assets/Textures/bricks2_diff.jpg", true);
	brickNormalTextureGammaCorrected = loadTexture("../Assets/Textures/bricks2_normal.jpg", false);
	brickDepthTextureGammaCorrected = loadTexture("../Assets/Textures/bricks2_disp.jpg", false);

	fileModel.Init("../Assets/Models/Dandelion/Textured_Flower.obj");
	//fileModel.Init("../Assets/Models/Primatives/Cube.obj");
	//fileModel.Init("../Assets/Models/nanosuit/nanosuit.obj");
	//fileModel.Init("../Assets/Models/sponza/sponza.obj");
	//Model light("../Assets/Models/Primatives/Cube.obj");

	pointLight.ambient = glm::vec3(0.05f);
	pointLight.diffuse = glm::vec3(0.8f);
	pointLight.specular = glm::vec3(1.0f);
	pointLight.linear = 0.09f;
	pointLight.quadratic = 0.032f;

	bool postProcessEnabled = true;
	bool filmGrainEnabled = true;
	float filmgrainStrength = 16.0f;
	bool vignetteEnabled = true;
	float vignetteInnerRadius = 0.1f;
	float vignetteOuterRadius = 1.0f;
	float vignetteOpacity = 1.0f;

	float parallaxHeightScale = 0.1f;

	bool bloomEnabled = true;

	float near_plane = 1.0f, far_plane = 27.5f;
	camera.Position = glm::vec3(0.0f, 4.0f, 10.0f);
	glm::vec3 lightPos(-2.0f, 4.0f, -1.0f);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	while (!glfwWindowShouldClose(pWindow))
	{
		float currentTime = static_cast<float>(glfwGetTime());
		g_deltaTime = currentTime - g_lastFrame;
		g_lastFrame = currentTime;

		ProcessInput(pWindow);

		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		IMGUI_NEW_FRAME;
		
		ImGui::Begin("Light Camera");
		{
			ImGui::DragFloat3("Position", &lightPos.x, 0.1f, -100.0f, 100.0f);
			ImGui::DragFloat("Near Plane", &near_plane, 0.1f, -50.0f, 100.0f);
			ImGui::DragFloat("Far Plane", &far_plane, 0.1f, -50.0f, 100.0f);
		}
		ImGui::End();
		// Shadow Depth Pre-Pass
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		lightingDepthShader.Use();
		lightingDepthShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
			glDisable(GL_CULL_FACE);
			RenderScene(lightingDepthShader);
			glEnable(GL_CULL_FACE);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		ImGui::Begin("Shadow Depth Pass Result");
		{
			ImGui::GetWindowDrawList()->AddImage(
												(void *)shadowMap,
												ImVec2(ImGui::GetCursorScreenPos()),
												ImVec2(ImGui::GetCursorScreenPos().x + g_windowWidth / 2,
														ImGui::GetCursorScreenPos().y + g_windowHeight / 2), ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();

		// Color pass
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
		glViewport(0, 0, g_windowWidth, g_windowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// View transformations
		glm::mat4 viewMat = camera.GetViewMatrix();
		glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(viewMat));
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
			   
		lightingShader.Use();
		lightingShader.SetMat4("projection", projectionMat);
		lightingShader.SetMat4("view", viewMat);
		lightingShader.SetVec3("viewPos", camera.Position);

		spotLight.position = camera.Position;
		spotLight.direction = camera.Front;

		lightingShader.SetPointLight("pointLights[0]", pointLight);
		lightingShader.SetDirectionalLight("dirLight", dirLight);
		lightingShader.SetSpotLight("spotLight", spotLight);
		lightingShader.SetMat4("lightSpaceMatrix", lightSpaceMatrix);

		ImGui::Begin("Parallax Amount");
		{
			ImGui::DragFloat("Amount", &parallaxHeightScale, 0.1, -1.0f, 1.0f);
		}
		ImGui::End();
		lightingShader.SetFloat("height_scale", parallaxHeightScale);

		lightingShader.SetInt("material.texture_diffuse1", 0);
		lightingShader.SetInt("material.texture_specular1", 1);
		lightingShader.SetInt("material.texture_normal1", 2);
		lightingShader.SetInt("shadowMap", 3);
		lightingShader.SetInt("depthMap", 4);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, floorDiffTextureGammaCorrected);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, floorSpecTextureGammaCorrected);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, floorNormTextureGammaCorrected);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, shadowMap);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, brickDepthTextureGammaCorrected);
		lightingShader.SetFloat("material.shininess", 32.0f);

		RenderScene(lightingShader);
		
		// Draw skybox
		glDepthFunc(GL_LEQUAL);
		skyboxShader.Use();
		glm::mat4 skyBoxViewMat = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		skyboxShader.SetMat4("projection", projectionMat);
		skyboxShader.SetMat4("view", skyBoxViewMat);
		glBindVertexArray(skyboxVAO);
		glActiveTexture(GL_TEXTURE0);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Bloom two-pass gaussian blur
		bool horizontal = true, first_iteration = true;
		unsigned int bloomAmount = 10;
		blurShader.Use();
		for (unsigned int i = 0; i < bloomAmount; i++)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
			blurShader.SetInt("horizontal", horizontal);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffer[!horizontal]);
			glBindVertexArray(quadVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			horizontal = !horizontal;
			if (first_iteration)
				first_iteration = false;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		// PostProcess pass
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, colorBuffers[0]);
		glBlitFramebuffer(0, 0, g_windowWidth, g_windowHeight, 0, 0, g_windowWidth, g_windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screenShader.Use();
		screenShader.SetFloat("time", (float)glfwGetTime());
		ImGui::Begin("Post Processing");
		{
			ImGui::Text("Film Grain");
			ImGui::Checkbox("FG Enabled", &filmGrainEnabled);
			ImGui::DragFloat("Strength", &filmgrainStrength, 0.1f, 0.0f, 80.0f);


			ImGui::Text("Vignette");
			ImGui::Checkbox("VN Enabled", &vignetteEnabled);
			ImGui::DragFloat("Inner Radius", &vignetteInnerRadius, 0.1f, 0.0f, 1.0f);
			ImGui::DragFloat("Outer Radius", &vignetteOuterRadius, 0.1f, 0.0f, 10.0f);
			ImGui::DragFloat("Opacity", &vignetteOpacity, 0.1f, 0.0f, 10.0f);

			ImGui::Text("Camera");
			ImGui::DragFloat("Exposure", &camera.Exposure, 0.1, 0.1, 1.0);

			ImGui::Text("Bloom");
			ImGui::Checkbox("BLM Enabled", &bloomEnabled);
		}
		ImGui::End();
		screenShader.SetFloat("filmgrainEnabled", filmGrainEnabled);
		screenShader.SetFloat("grainStrength", filmgrainStrength);
		screenShader.SetFloat("vignetteEnabled", vignetteEnabled);
		screenShader.SetFloat("vignetteInnerRadius", vignetteInnerRadius);
		screenShader.SetFloat("vignetteOuterRadius", vignetteOuterRadius);
		screenShader.SetFloat("vignetteOpacity", vignetteOpacity);
		screenShader.SetFloat("near_plane", near_plane);
		screenShader.SetFloat("far_plane", far_plane);
		screenShader.SetFloat("exposure", camera.Exposure);
		screenShader.SetFloat("bloomEnabled", bloomEnabled);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, pingpongBuffer[horizontal]);
		
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		
		IMGUI_RENDER;

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteTextures(sizeof(floorSpecTextureGammaCorrected), &floorSpecTextureGammaCorrected);
	glDeleteTextures(sizeof(floorDiffTextureGammaCorrected), &floorDiffTextureGammaCorrected);

	CleanUp();

	return 0;
}

void CleanUp()
{
	fileModel.Destroy();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
}

float planeRot = -90;
void RenderScene(const Shader& shader)
{
	// floor
	glm::mat4 floorMat = glm::mat4(1.0f);
	floorMat = glm::translate(floorMat, glm::vec3(0.0f, -2.0f, 0.0f));
	floorMat = glm::scale(floorMat, glm::vec3(20.0f));
	//floorMat = glm::rotate(floorMat, glm::radians(planeRot) , glm::vec3(1.0, 0.0, 0.0));
	shader.SetMat4("model", floorMat);
	renderFloorQuad();

	// cubes
	/*glm::mat4 model = glm::mat4(1.0f);
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.SetMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	shader.SetMat4("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	shader.SetMat4("model", model);
	renderCube();*/

	// render the loaded model
	glm::mat4 modelMat = glm::mat4(1.0f);
	//modelMat = glm::scale(modelMat, glm::vec3(0.01f));
	modelMat = glm::scale(modelMat, glm::vec3(0.5f));
	modelMat = glm::translate(modelMat, glm::vec3(0.0f, -1.75f, -2.0f));
	shader.SetMat4("model", modelMat);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	fileModel.Draw(shader);

}

unsigned int floorQuadVAO = 0;
unsigned int floorQuadVBO = 0;
void renderFloorQuad()
{
	if (floorQuadVAO == 0)
	{
		// positions
		glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
		glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
		glm::vec3 pos3(1.0f, -1.0f, 0.0f);
		glm::vec3 pos4(1.0f, 1.0f, 0.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 0.0f, 1.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent1 = glm::normalize(tangent1);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent1 = glm::normalize(bitangent1);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent2 = glm::normalize(tangent2);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent2 = glm::normalize(bitangent2);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};

		// configure plane VAO
		glGenVertexArrays(1, &floorQuadVAO);
		glGenBuffers(1, &floorQuadVBO);
		glBindVertexArray(floorQuadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, floorQuadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
	}
	glBindVertexArray(floorQuadVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glBindVertexArray(0);
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			// bottom face
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			-1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			// top face
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			-1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			-1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

unsigned int loadTexture(char const * path, bool gammaCorrection)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum internalFormat;
		GLenum dataFormat;
		if (nrComponents == 1)
		{
			internalFormat = dataFormat = GL_RED;
		}
		else if (nrComponents == 3)
		{
			internalFormat = gammaCorrection ? GL_SRGB : GL_RGB;
			dataFormat = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			internalFormat = gammaCorrection ? GL_SRGB_ALPHA : GL_RGBA;
			dataFormat = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

unsigned int loadCubeMapTextures(std::vector<std::string> faces)
{
	unsigned int cubeMapTextureID;
	glGenTextures(1, &cubeMapTextureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);

	int width, height, nrChannels;
	for (int i = 0; i < faces.size(); i++)
	{
		unsigned char* data;
		data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Failed to load cube map texture at path: " << faces[i] << "\n";
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return cubeMapTextureID;
}

void InitSkybox(unsigned int & skyboxVAO, unsigned int & skyboxVBO)
{
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
}

void ProcessInput(GLFWwindow* pWindow)
{
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);

	float camSpeedScale = 1.0f;
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camSpeedScale = 7.0f;

	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_E) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_Q) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, g_deltaTime * camSpeedScale);
	if (glfwGetKey(pWindow, GLFW_KEY_C) == GLFW_PRESS)
		pointLight.position = camera.Position;

}

void frame_buffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow * pWindow, double xpos, double ypos)
{
	if (glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
		return;
	
	if (g_firstMouse)
	{
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		g_firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}