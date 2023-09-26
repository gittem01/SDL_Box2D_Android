#include <box2d/box2d.h>
#include <SDL.h>
#include <vector>
#include <math.h>

class Base
{
public:
	b2World* world;
	b2Body* groundBody;
	class DebugRenderer* debugRenderer;
	SDL_Window* window;
	SDL_Renderer* renderer;
    SDL_Sensor *accelerometer;
	std::vector<char> pressedFingers;
	char fingerPresses[20];
	int fingerPositions[20][2];
    std::vector<b2MouseJoint*> dragJoints;

	float deltaTime = 0.0f;
	uint32 renderFlags = 0x1F; // initially render everything
    int numFingers = 0;

	int width = 1280;
	int height = 720;
	int halfWidth = width / 2;
	int halfHeight = height / 2;

	bool shouldQuit = false;

	Base();
	~Base();

	void createTestBodies();
	void handleEvents();
	void loop();
};