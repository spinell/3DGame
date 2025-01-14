#pragma once

#include <Engine/Event.h>
#include <Engine/Layer.h>

class EditorLayer : public Engine::Layer {
public:
    EditorLayer();

    ~EditorLayer();

    void onAttach() override;

    void onDetach() override;

    void onUpdate(float timeStep) override;

    void onImGuiRender() override;

    bool onEvent(const Engine::Event& event) override;
};
