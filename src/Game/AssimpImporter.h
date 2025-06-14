#pragma once
#include "Mesh.h"

#include <filesystem>

class AssimpImporter {
public:
    AssimpImporter();

    /// @brief Import all mesh in a file into a single mesh
    /// @param path The path of the file to import.
    /// @return The imported mesh
    Mesh importMesh(const std::filesystem::path& path);

private:
};
