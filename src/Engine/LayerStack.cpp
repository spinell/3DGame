#include <Engine/LayerStack.h>

#include <algorithm>

namespace Engine {

LayerStack::~LayerStack() {
    clear();
}

void LayerStack::clear() {
    for (Layer* layer : mLayers) {
        layer->onDetach();
        delete layer;
    }
}

void LayerStack::pushLayer(Layer* layer) {
    mLayers.emplace(mLayers.begin() + mLayerInsertIndex, layer);
    mLayerInsertIndex++;
}

void LayerStack::pushOverlay(Layer* overlay) { mLayers.emplace_back(overlay); }

void LayerStack::popLayer(Layer* layer) {
    if (auto it = std::find(mLayers.begin(), mLayers.end(), layer); it != mLayers.end()) {
        mLayers.erase(it);
        --mLayerInsertIndex;
    }
}

void LayerStack::popOverlay(Layer* overlay) {
    if (auto it = std::find(mLayers.begin(), mLayers.end(), overlay); it != mLayers.end()) {
        mLayers.erase(it);
    }
}

} // namespace Engine
