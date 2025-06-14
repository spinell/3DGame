#pragma once
#include <Engine/Layer.h>

#include <cassert>
#include <vector>

namespace Engine {

/// @brief
class LayerStack {
public:
    LayerStack() = default;
    ~LayerStack();

    LayerStack(const LayerStack&) = delete;
    LayerStack(LayerStack&&) = delete;

    LayerStack& operator=(const LayerStack&) = delete;
    LayerStack& operator=(LayerStack&&) = delete;

    void clear();

    /// @brief
    /// @param index
    /// @return
    Layer* operator[](size_t index) {
        assert(index >= 0 && index < mLayers.size());
        return mLayers[index];
    }

    /// @brief
    /// @param index
    /// @return
    const Layer* operator[](size_t index) const {
        assert(index >= 0 && index < mLayers.size());
        return mLayers[index];
    }

    /// @brief
    /// @return
    size_t size() const { return mLayers.size(); }

    /// @brief
    /// @param layer
    void pushLayer(Layer* layer);

    /// @brief
    /// @param overlay
    void pushOverlay(Layer* overlay);

    /// @brief
    /// @param layer
    void popLayer(Layer* layer);

    /// @brief
    /// @param overlay
    void popOverlay(Layer* overlay);

    /// @brief
    /// @return
    std::vector<Layer*>::iterator         begin() { return mLayers.begin(); }
    std::vector<Layer*>::reverse_iterator rbegin() { return mLayers.rbegin(); }

    /// @brief
    /// @return
    std::vector<Layer*>::iterator         end() { return mLayers.end(); }
    std::vector<Layer*>::reverse_iterator rend() { return mLayers.rend(); }

private:
    std::vector<Layer*> mLayers;
    unsigned int        mLayerInsertIndex{};
};

} // namespace Engine
