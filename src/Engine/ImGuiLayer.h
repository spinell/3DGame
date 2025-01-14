#pragma once
#include <Engine/Layer.h>

namespace Engine {

class ImGuiLayer : public Engine::Layer {
public:
    ImGuiLayer();

    ~ImGuiLayer() {}

    void begin();
    void end();

    void onAttach() override;

    void onDetach() override;
};

} // namespace Engine
