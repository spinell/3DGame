#pragma once

#include "EditorLayer.h"

#include <Engine/Application.h>
#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <imgui.h>

EditorLayer::EditorLayer() : Engine::Layer("EditorLayer") {}

EditorLayer::~EditorLayer() {}

void EditorLayer::onAttach() {}

void EditorLayer::onDetach() {}

void EditorLayer::onUpdate(float timeStep) {}

void EditorLayer::onImGuiRender() {}

bool EditorLayer::onEvent(const Engine::Event& event) { return false; }
