#pragma once

#include "SceneRenderer.h"

#include <Engine/Event.h>
#include <Engine/Layer.h>
#include <entt/entt.hpp>

class TestLayer1 : public Engine::Layer {
public:
    TestLayer1(const char* name);

    ~TestLayer1();

    void onAttach() override;

    void onDetach() override;

    void onUpdate(float timeStep) override;

    void onImGuiRender() override;

    bool onEvent(const Engine::Event& event) override;

private:
    entt::registry mRegistry;
    SceneRenderer*  mSceneRenderer{};
};
