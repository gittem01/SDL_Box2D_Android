#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include "glm/glm.hpp"
#include <SDL.h>
#include <SDL_vulkan.h>

class Camera;

class WindowHandler
{
public:
	int* keyData = (int*)calloc(512, sizeof(int));
	double* mouseData = (double*)calloc(7, sizeof(double));
	double* trackpadData = (double*)calloc(2, sizeof(int));
	int* lastMousePos = (int*)calloc(2, sizeof(int));
	glm::vec2 windowSizes = glm::vec2(1280, 720);
	bool releaseQueue[3] = { false, false, false };
	std::vector<int> newPressIndices;
	SDL_Window* window;
	Camera* cam;

	uint32_t dpiScaling = 1;

	WindowHandler(Camera* cam);

	void massInit();
	bool looper();
	void handleMouseData();
	void handleKeyData();
};