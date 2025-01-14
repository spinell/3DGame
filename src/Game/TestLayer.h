#pragma once

#include <Engine/Event.h>
#include <Engine/Layer.h>

class TestLayer1 : public Engine::Layer {
public:
    TestLayer1(const char* name);

    ~TestLayer1();

    void onAttach() override;

    void onDetach() override;

    void onUpdate(float timeStep) override;

    void onImGuiRender() override;

    bool onEvent(const Engine::Event& event) override;
};
