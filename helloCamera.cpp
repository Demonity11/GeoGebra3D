#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "stb_image.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void vertexSpecification();
void textureSpecification();
glm::mat4 customLookAtMatrix(glm::vec3 pos, glm::vec3 worldUp, glm::vec3 front);

unsigned int VAO{};
unsigned int VBO{};
//unsigned int EBO{};
unsigned int texture1{};
unsigned int texture2{};

// settings
constexpr unsigned int SCR_WIDTH{ 800 };
constexpr unsigned int SCR_HEIGHT{ 600 };

// camera
glm::vec3 cameraPos{ glm::vec3(0.0f, -2.0f,  3.0f) };
glm::vec3 cameraFront{ glm::vec3(0.0f, 0.0f, -1.0f) };
glm::vec3 cameraUp{ glm::vec3(0.0f, 1.0f,  0.0f) };

float deltaTime{ 0.0f };
float lastTime{ 0.0f };

float lastX{ 400.0f };
float lastY{ 300.0f };

float yaw{};
float pitch{};

bool firstMouse{ true };
bool fpsCamera{ true };

float fov{ 45.0f };

int main()
{
	// glfw initilization and configuration
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);

	if (window == NULL)
	{
		std::cerr << "Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glfw load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return -1;
	}

	// build and compile our shader program

	Shader ourShader{ "helloCamera.vert", "helloCamera.frag" };

	ourShader.use();
	glUniform1i(glGetUniformLocation(ourShader.ID, "texture1"), 0);
	ourShader.setInt("texture2", 1);

	std::vector<glm::vec3> cubePositions
	{
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f),
	};

	glEnable(GL_DEPTH_TEST); // enables depth testing, which means testing if a fragment is behind another
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	vertexSpecification();
	textureSpecification();

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// rendering commands here
		glClearColor(0.2f, 0.4f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();

		glActiveTexture(GL_TEXTURE0); // activate the texture unit first before binding the texture
		glBindTexture(GL_TEXTURE_2D, texture1);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);

		//glm::mat4 view{ glm::mat4(1.0f) };
		//view = glm::translate(view, glm::vec3(movX, 0.0f, movZ));
		//view = glm::rotate(view, glm::radians(rotationY), glm::vec3(0.0f, 1.0f, 0.0f));
		//view = glm::rotate(view, glm::radians(rotationX), glm::vec3(1.0f, 0.0f, 0.0f));

		// camera

		glm::mat4 view{};
		//view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		view = customLookAtMatrix(cameraPos, cameraFront, cameraUp);
		//view = customLookAtMatrix(glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		float currentFrame{ static_cast<float>(glfwGetTime()) };
		deltaTime = currentFrame - lastTime;
		lastTime = currentFrame;

		glm::mat4 projection{};
		projection = glm::perspective(glm::radians(fov), 800.0f / 600.0f, 0.1f, 100.0f);

		ourShader.setMat4("view", view);
		ourShader.setMat4("projection", projection);

		glBindVertexArray(VAO);
		for (int i{ 0 }; i < 10; ++i)
		{
			glm::mat4 model{ glm::mat4(1.0f) };
			model = glm::translate(model, cubePositions[i]);

			float angle{ (i + 1) * 20.0f };

			if (i % 3 == 0)
			{
				angle = static_cast<float>(glfwGetTime()) * 30.0f;

				model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			}

			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));

			ourShader.setMat4("model", model);

			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		// plane
		glm::mat4 model{ glm::mat4(1.0f) };
		model = glm::translate(model, glm::vec3(0.0f, -3.0f, 0.0f));
		model = glm::scale(model, glm::vec3(30.0f, 0.0f, 30.0f));
		ourShader.setMat4("model", model);

		glDrawArrays(GL_TRIANGLES, 36, 6);

		//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void vertexSpecification()
{
	//std::vector vertices
	//{      // positions        // colors     // texture coords
	//	-0.5f,  0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,  // top left
	//	 0.5f,  0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,  // top right
	//	 0.5f, -0.5f, 0.0f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,  // bottom right
	//	-0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 0.0f,  0.0f, 0.0f   // bottom left
	//};

	std::vector vertices
	{
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,

	// plane
	 1.0f, 0.0f,  1.0f, 1.0f, 1.0f,
	 1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, 0.0f,  1.0f, 0.0f, 1.0f,
	 1.0f, 0.0f, -1.0f, 1.0f, 0.0f,
	-1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
	-1.0f, 0.0f,  1.0f, 0.0f, 1.0f
	};

	// cubes
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	//glGenBuffers(1, &EBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

	int stride{ 5 * sizeof(float) };
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));

	//glEnableVertexAttribArray(2);
	//glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
}

void textureSpecification()
{
	// generating a texture obj just like all the others
	glGenTextures(1, &texture1);
	// binding = selecting the current target -> texture
	glBindTexture(GL_TEXTURE_2D, texture1);

	// the axes s,t,r are equivalent to x,y,z. so we explicity tell OpenGL what to do when we go "out of bounds" by axe
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// we can specify what filter OpenGL will use when downscaling/upscaling a object
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// loading the img into memory
	int width{};
	int height{};
	int nrChannels{};
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data{ stbi_load("container.jpg", &width, &height, &nrChannels, 0) };
	if (data)
	{
		// actually passing the data to binded obj
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		// automatically generating the mipmaps
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cerr << "Failed to load texture\n";
	}
	// freeing image memory
	stbi_image_free(data);

	// for the second texture
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	data = stbi_load("awesomeface.png", &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cerr << "Failed to load texture\n";
	}
	stbi_image_free(data);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	
	// free camera
	float cameraSpeed{ 3.5f * deltaTime };
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		cameraPos += cameraSpeed * cameraFront;
	}

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		cameraPos -= cameraSpeed * cameraFront;

	}

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}

	if (fpsCamera) // locks 'y' direction so that the user stays at a fixed height
	{
		cameraPos.y = -2.0f;
	}
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = ypos - lastY;

	lastX = xpos;
	lastY = ypos;

	const float sensitivity{ 0.1f };

	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw   += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch =  89.0f;

	if (pitch < -89.0f)
		pitch = -89.0f;	

	glm::vec3 direction{};
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = -sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	fov -= static_cast<float>(yoffset);

	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 45.0f)
		fov = 45.0f;
}

glm::mat4 customLookAtMatrix(glm::vec3 pos, glm::vec3 target, glm::vec3 worldUp)
{
	glm::mat4 translation{ glm::mat4(1.0f) };

	translation[3][0] = -pos.x;
	translation[3][1] = -pos.y;
	translation[3][2] = -pos.z;

	glm::vec3 direction{ glm::normalize(pos - target) };
	glm::vec3 right{ glm::normalize(glm::cross(glm::normalize(worldUp), direction)) };
	glm::vec3 up{ glm::cross(direction, right) };

	glm::mat4 rotation{ glm::mat4(1.0f) };

	rotation[0][0] = right.x;
	rotation[1][0] = right.y;
	rotation[2][0] = right.z;

	rotation[0][1] = up.x;
	rotation[1][1] = up.y;
	rotation[2][1] = up.z;

	rotation[0][2] = direction.x;
	rotation[1][2] = direction.y;
	rotation[2][2] = direction.z;

	return rotation * translation;
}
