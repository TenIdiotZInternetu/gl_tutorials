#include <iostream>
#include <cassert>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <array>

#include "ogl_resource.hpp"
#include "error_handling.hpp"
#include "window.hpp"
#include "shader.hpp"


#include "scene_definition.hpp"
#include "renderer.hpp"

#include "ogl_geometry_factory.hpp"
#include "ogl_material_factory.hpp"

#include <glm/gtx/string_cast.hpp>

const float MOVEMENT_STEP = 2.0f;
constexpr glm::vec3 CAMERA_POS = {0.0f, 5.0f, 20.0f};

void toggle(const std::string &aToggleName, bool &aToggleValue) {

	aToggleValue = !aToggleValue;
	std::cout << aToggleName << ": " << (aToggleValue ? "ON\n" : "OFF\n");
}

void defineControls(Window& window, Camera& camera, MouseTracking& mouseTracking ) {
	window.onCheckInput([&camera, &mouseTracking](GLFWwindow *aWin) {
		mouseTracking.update(aWin);
		if (glfwGetMouseButton(aWin, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			camera.orbit(-0.4f * mouseTracking.offset(), glm::vec3());
		}
	});

	window.setKeyCallback([&camera](GLFWwindow *aWin, int key, int scancode, int action, int mods) {
		if (action != GLFW_PRESS) return;

		switch (key) {
			case GLFW_KEY_ENTER:
				camera.setPosition(CAMERA_POS);
				camera.lookAt(glm::vec3());
				break;
			case GLFW_KEY_W:
				camera.move(-camera.getForwardVector() * MOVEMENT_STEP);
				break;
			case GLFW_KEY_S:
				camera.move(camera.getForwardVector() * MOVEMENT_STEP);
				break;
			case GLFW_KEY_A:
				camera.move(-camera.getRightVector() * MOVEMENT_STEP);
				break;
			case GLFW_KEY_D:
				camera.move(camera.getRightVector() * MOVEMENT_STEP);
				break;
		}
	});
}

struct Config {
	int currentSceneIdx = 0;
	bool showSolid = true;
	bool showWireframe = false;
	bool useZOffset = false;
};

int main() {
	// Initialize GLFW
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW" << std::endl;
		return -1;
	}

	try {
		auto window = Window();
		Config config;
		Camera camera(window.aspectRatio());
		camera.setPosition(CAMERA_POS);
		camera.lookAt(glm::vec3());
		SpotLight light;
		light.setPosition(glm::vec3(25.0f, 40.0f, 30.0f));
		light.lookAt(glm::vec3());

		MouseTracking mouseTracking;
		defineControls(window, camera, mouseTracking);

		OGLMaterialFactory materialFactory;
		materialFactory.loadShadersFromDir("./shaders/");
		materialFactory.loadTexturesFromDir("./data/textures/");

		OGLGeometryFactory geometryFactory;


		std::array<SimpleScene, 1> scenes {
			createCottageScene(materialFactory, geometryFactory),
		};

		Renderer renderer(materialFactory);
		renderer.applySsao(ssao{});

		window.onResize([&camera, &window, &renderer](int width, int height) {
				camera.setAspectRatio(window.aspectRatio());
				renderer.initialize(width, height);
			});


		renderer.initialize(window.size()[0], window.size()[1]);
		window.runLoop([&] {
			renderer.shadowMapPass(scenes[config.currentSceneIdx], light);
			// renderer.shadowMapPass(scenes[config.currentSceneIdx], camera);

			renderer.clear();
			renderer.geometryPass(scenes[config.currentSceneIdx], camera, RenderOptions{"solid"});
			renderer.compositingPass(light, camera);
		});
	} catch (ShaderCompilationError &exc) {
		std::cerr
			<< "Shader compilation error!\n"
			<< "Shader type: " << exc.shaderTypeName()
			<<"\nError: " << exc.what() << "\n";
		return -3;
	} catch (OpenGLError &exc) {
		std::cerr << "OpenGL error: " << exc.what() << "\n";
		return -2;
	} catch (std::exception &exc) {
		std::cerr << "Error: " << exc.what() << "\n";
		return -1;
	}

	glfwTerminate();
	return 0;
}
