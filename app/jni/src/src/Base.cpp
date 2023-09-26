#include <Base.h>
#include <DebugRenderer.h>
#include <set>
#include <vulkan/vulkan.h>
#include <vk/ThinDrawer.h>
#include "vk/Camera.h"

void Base::createTestBodies()
{
    // box0
    b2BodyDef boxBodyDef;
    boxBodyDef.type = b2_dynamicBody;
    boxBodyDef.position.Set(0.0f, 0.0f);
    b2Body* boxBody = world->CreateBody(&boxBodyDef);

    b2PolygonShape boxShape;
    boxShape.SetAsBox(1.0f, 1.0f);
    boxBody->CreateFixture(&boxShape, 1.0f);

    boxBody->SetAngularVelocity(1.0);

    // box1
    boxBodyDef.position.Set(0.6f, 2.5f);
    boxBody = world->CreateBody(&boxBodyDef);

    boxShape.SetAsBox(1.0f, 1.0f);
    boxBody->CreateFixture(&boxShape, 1.0f);

    // second fixture for box1
    boxShape.SetAsBox(0.5f, 0.5f, b2Vec2(0.0f, 0.8f), b2_pi * 0.25f);
    boxBody->CreateFixture(&boxShape, 1.0f);

    // static box
    boxBodyDef.type = b2_staticBody;
    boxBodyDef.position.Set(0.0f, -10.0f);
    groundBody = world->CreateBody(&boxBodyDef);

    boxShape.SetAsBox(100.0f, 1.0f);
    groundBody->CreateFixture(&boxShape, 1.0f);

    // circle
    b2BodyDef circleBodyDef;
    circleBodyDef.type = b2_dynamicBody;
    circleBodyDef.position.Set(5.0f, 0.0f);
    b2Body* circleBody = world->CreateBody(&circleBodyDef);

    b2CircleShape circleShape;
    circleShape.m_radius = 1.0f;
    circleBody->CreateFixture(&circleShape, 1.0f);

    circleBody->SetAngularVelocity(8.0);

    b2MouseJointDef jDef;
    jDef.bodyA = boxBody;
    jDef.bodyB = circleBody;
    jDef.collideConnected = true;

    jDef.target = circleBody->GetPosition() - b2Vec2(1.0, 0.0);
    jDef.maxForce = circleBody->GetMass() * 20.0f;
    
    b2LinearStiffness(jDef.stiffness, jDef.damping, 1.0f, 0.0f, jDef.bodyA, jDef.bodyB);

    b2MouseJoint* mj = (b2MouseJoint*)world->CreateJoint(&jDef);

    b2Vec2 nextTarget = b2Vec2(0.0, -3.0);
    mj->SetTarget(nextTarget);
}

class QueryCallback : public b2QueryCallback
{
public:
    Base* base;
    b2Vec2 p;
    std::set<b2Body*> clickedBodies;

    QueryCallback(Base* base, b2Vec2 p)
    {
        this->base = base;
        this->p = p;
    }

    virtual bool ReportFixture(b2Fixture* fixture)
    {
        if (fixture->GetBody()->GetType() == b2_dynamicBody && fixture->TestPoint(p))
        {
            fixture->GetBody()->SetAwake(true);
            clickedBodies.insert(fixture->GetBody());
        }

        return true;
    }
};

Base::Base()
{
    //td = new ThinDrawer(nullptr);

    memset(fingerPresses, 0, sizeof(fingerPresses));
    memset(fingerPositions, 0, sizeof(fingerPositions));

    SDL_DisplayMode displayMode;
    bool r = SDL_GetDisplayMode(0, 0, &displayMode);

    int refreshRate;
    if (displayMode.refresh_rate > 0 && r == 0)
    {
        refreshRate = displayMode.refresh_rate;
    }
    else
    {
        refreshRate = 60;
    }
    deltaTime = 1.0f / (float)refreshRate;

    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN, &window, &renderer);

    //renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    b2Vec2 gravity = b2Vec2(0.0f, -10.0f);
    world = new b2World(gravity);
    world->SetAllowSleeping(false);

    debugRenderer = new DebugRenderer(this);
    debugRenderer->SetFlags(renderFlags);
    world->SetDebugDraw(debugRenderer);

    if (SDL_InitSubSystem(SDL_INIT_SENSOR) != 0) {
        SDL_Log("Unable to initialize SDL Sensor: %s", SDL_GetError());
    }

    accelerometer = SDL_SensorOpen(0);
    if (!accelerometer) {
        SDL_Log("SDL_SensorOpen failed: %s", SDL_GetError());
    }

    createTestBodies();
}

Base::~Base()
{
    delete world;
    delete debugRenderer;

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Base::handleEvents()
{
    for (char fingerId : pressedFingers)
    {
        if (fingerPresses[fingerId] == 2)
        {
            fingerPresses[fingerId] = 1;
        }
    }
    pressedFingers.clear();

    SDL_Event e;
    while (SDL_PollEvent(&e) != 0)
    {
        switch (e.type)
        {
        case SDL_QUIT:
            shouldQuit = true;
            break;
        case SDL_FINGERDOWN:
            SDL_Log("Finger: %d", (int)e.tfinger.fingerId);
            fingerPresses[e.tfinger.fingerId] = 2;
            fingerPositions[e.tfinger.fingerId][0] = (int)(e.tfinger.x * (float)width);
            fingerPositions[e.tfinger.fingerId][1] = (int)(e.tfinger.y * (float)height);
            pressedFingers.push_back(e.tfinger.fingerId);
            numFingers++;
            break;
        case SDL_FINGERUP:
            fingerPresses[e.tfinger.fingerId] = 0;
            numFingers--;
            break;
        case SDL_FINGERMOTION:
            if (numFingers == 2)
            {
                debugRenderer->camPos.x -= 0.5f * e.tfinger.dx * (float)width / debugRenderer->scaleFactor;
                debugRenderer->camPos.y += 0.5f * e.tfinger.dy * (float)height / debugRenderer->scaleFactor;

                //td->wh->cam->pos.x -= 0.01f * e.tfinger.dx * (float)width / pow(td->wh->cam->zoom, 2.0f);
                //td->wh->cam->pos.y -= 0.01f * e.tfinger.dy * (float)height / pow(td->wh->cam->zoom, 2.0f);
            }
            fingerPositions[e.tfinger.fingerId][0] = (int)(e.tfinger.x * (float)width);
            fingerPositions[e.tfinger.fingerId][1] = (int)(e.tfinger.y * (float)height);

            break;
        case SDL_MULTIGESTURE:
            if (numFingers == 2)
            {
                b2Vec2 p = b2Vec2(e.mgesture.x * (float)width, e.mgesture.y * (float)height);
                p.x -= (float)halfWidth; p.y = (float)halfHeight - p.y;
                b2Vec2 p0 = (1.0f / debugRenderer->scaleFactor) * p;
                debugRenderer->scaleFactor *= 1.0f + e.mgesture.dDist * 10.0f;
                b2Vec2 p1 = (1.0f / debugRenderer->scaleFactor) * p;

                debugRenderer->camPos += p0 - p1;

                //td->wh->cam->zoom += e.mgesture.dDist;
            }
            break;
        case SDL_WINDOWEVENT:
            switch (e.window.event)
            {
            case SDL_WINDOWEVENT_RESIZED:
            case SDL_WINDOWEVENT_SIZE_CHANGED:
                SDL_GetWindowSize(window, &width, &height);
                halfWidth = width / 2; halfHeight = height / 2;
                break;
            }
            break;
            case SDL_SENSORUPDATE:
            {
                if (e.sensor.which == 0) {
                    float x = e.sensor.data[0];
                    float y = e.sensor.data[1];
                    float z = e.sensor.data[2];

                    world->SetGravity(b2Vec2(-x, -y));
                }
                break;
            }

            default:
            break;
        }
    }

    if (numFingers == 1 && fingerPresses[0] == 2)
    {
        b2Vec2 wp = debugRenderer->translateToWorldCoords(
                b2Vec2((float)fingerPositions[0][0], (float)fingerPositions[0][1]));

        const float r = 0.001;
        b2Vec2 lowerBound = b2Vec2(wp.x - r, wp.y - r);
        b2Vec2 upperBound = b2Vec2(wp.x + r, wp.y + r);

        QueryCallback qb(this, wp);
        b2AABB aabb = { lowerBound, upperBound };
        world->QueryAABB(&qb, aabb);

        for (b2Body* body : qb.clickedBodies)
        {
            b2MouseJointDef jDef;
            jDef.bodyA = groundBody;
            jDef.bodyB = body;
            jDef.collideConnected = true;

            jDef.target = wp;
            jDef.maxForce = body->GetMass() * 20.0f;

            b2LinearStiffness(jDef.stiffness, jDef.damping, 5.0f, 1.0f, jDef.bodyA, jDef.bodyB);

            dragJoints.push_back((b2MouseJoint*)world->CreateJoint(&jDef));
        }
    }
    else if (numFingers != 1)
    {
        for (b2MouseJoint* mj : dragJoints)
        {
            world->DestroyJoint(mj);
        }

        dragJoints.clear();
    }

    for (b2MouseJoint* mj : dragJoints)
    {
        mj->SetTarget(debugRenderer->translateToWorldCoords(
                b2Vec2((float)fingerPositions[0][0], (float)fingerPositions[0][1])));
    }

    debugRenderer->scaleFactor = std::max(std::min(debugRenderer->scaleFactor, debugRenderer->maxScale), debugRenderer->minScale);

    debugRenderer->SetFlags(renderFlags);
}

void Base::loop()
{
    while (!shouldQuit)
    {
        //td->wh->cam->updateOrtho();

        //td->renderLoop();
        //td->gameTime += deltaTime;

        handleEvents();

        world->Step(deltaTime, 8, 3);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
        SDL_RenderClear(renderer);

        world->DebugDraw();

        SDL_RenderPresent(renderer);
    }
}