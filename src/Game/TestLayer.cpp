#include "TestLayer.h"

#include "Camera.h"
#include "CameraController.h"
#include "Mesh.h"
#include "Terrain.h"
#include "Renderer.h"

#include "Spirv/SpirvReflection.h"
#include "vulkan/VulkanContext.h"
#include "vulkan/VulkanDescriptorPool.h"
#include "vulkan/VulkanSwapchain.h"
#include "vulkan/VulkanUtils.h"
#include "vulkan/VulkanShaderProgram.h"
#include "vulkan/VulkanImGuiRenderer.h"
#include "vulkan/VulkanTexture.h"
#include "vulkan/VulkanGraphicPipeline.h"

#include "AssimpImporter.h"

#include <Engine/Application.h>
#include <Engine/Event.h>
#include <Engine/Input.h>
#include <Engine/Layer.h>
#include <Engine/Log.h>
#include <Engine/SDL3/SDL3Window.h>
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>

#include <array>
#include <filesystem>

struct FrameData {
    VkCommandPool   commandPool;
    VkCommandBuffer commandBuffer;
};
FrameData        frameData{};
VulkanSwapchain* vulkanSwapchain{};
std::shared_ptr<VulkanShaderProgram> fullScreenShader;
VulkanGraphicPipelinePtr  pipelineFullScreen;

std::shared_ptr<Terrain> gTerrain;

TestLayer1::TestLayer1(const char* name) : Engine::Layer(name) {}
VulkanTexturePtr depthBuffer;
Engine::CameraController cameraController;
std::vector<Mesh>        meshs;
std::map<std::string,VulkanTexturePtr> gTextureCache;
entt::entity light1;
entt::entity light2;
entt::entity light3;
entt::entity flashLight;

bool gWalkCamMode = true;

TestLayer1::~TestLayer1() {}

void TestLayer1::onAttach() {
    VulkanContext::Initialize();
    Renderer::Init();
    mSceneRenderer = new SceneRenderer();

    gTerrain = std::make_shared<Terrain>();

    cameraController.setPosition({0, gTerrain->getHeight(0, 0), 0});

    // generate mipmap for normal and specular map ?
    gTextureCache["ab_crate_a"]        = VulkanTexture::Create("./data/ab_crate_a.png", true, true);
    gTextureCache["ab_crate_a_nm"]     = VulkanTexture::Create("./data/ab_crate_a_nm.png", false, true);
    gTextureCache["ab_crate_a_sm"]     = VulkanTexture::Create("./data/ab_crate_a_sm.png", false, true);
    gTextureCache["brick_wall2"]       = VulkanTexture::Create("./data/brick_wall2-diff-512.tga", true, true);
    gTextureCache["brick_wall2_sm"]    = VulkanTexture::Create("./data/brick_wall2-spec-512.tga", false, true);
    gTextureCache["brick_wall2_nm"]    = VulkanTexture::Create("./data/brick_wall2-nor-512.tga", false, true);
    gTextureCache["metal1"]            = VulkanTexture::Create("./data/metal1-dif-1024.tga", true, true);
    gTextureCache["metal1_sm"]         = VulkanTexture::Create("./data/metal1-spec-1024.tga", false, true);
    gTextureCache["metal1_nm"]         = VulkanTexture::Create("./data/metal1-nor-1024.tga", false, true);
    gTextureCache["FloorSandStone"]    = VulkanTexture::Create("./data/FloorDiffuse.png", true, true); // FloorAmbientOcclusion
    gTextureCache["FloorSandStone_nm"] = VulkanTexture::Create("./data/FloorNormal.png", false, true);
    gTextureCache["FloorSandStone_sm"] = VulkanTexture::Create("./data/FloorSpacular.png", false, true);

    gTextureCache["edf_soldier_a"]    = VulkanTexture::Create("./data/model/edf_soldier/edf_body_d.tga", true, true);
    gTextureCache["edf_soldier_a_nm"] = VulkanTexture::Create("./data/model/edf_soldier/edf_body_n.tga", false, true);
    gTextureCache["edf_soldier_a_sm"] = VulkanTexture::Create("./data/model/edf_soldier/edf_body_s.tga", false, true);

    gTextureCache["grass"]                 = VulkanTexture::Create("./data/textures/pattern_216/T_216_d.tga", true, true); // grass
    gTextureCache["grass_s"]               = VulkanTexture::Create("./data/textures/pattern_216/T_216_s.tga", true, true);
    gTextureCache["grass_n"]               = VulkanTexture::Create("./data/textures/pattern_216/T_216_n.tga", false, true);

    gTextureCache["coast_land_rocks_01"]   = VulkanTexture::Create("./data/textures/pattern_218/T_218_d.tga", true, true); // rock
    gTextureCache["coast_land_rocks_01_s"] = VulkanTexture::Create("./data/textures/pattern_218/T_218_s.tga", true, true);
    gTextureCache["coast_land_rocks_01_n"] = VulkanTexture::Create("./data/textures/pattern_218/T_218_n.tga", false, true);

    gTextureCache["coast_sand_rocks_02"]   = VulkanTexture::Create("./data/textures/pattern_216/T_216_d.tga", true, true); // grass
    gTextureCache["coast_sand_rocks_02_s"] = VulkanTexture::Create("./data/textures/pattern_216/T_216_s.tga", true, true);
    gTextureCache["coast_sand_rocks_02_n"] = VulkanTexture::Create("./data/textures/pattern_216/T_216_n.tga", false, true);

    gTextureCache["brown_mud_02"]          = VulkanTexture::Create("./data/textures/pattern_91/T_91_d.tga", true, true); // tiles
    gTextureCache["brown_mud_02_s"]        = VulkanTexture::Create("./data/textures/pattern_91/T_91_s.tga", true, true);
    gTextureCache["brown_mud_02_n"]        = VulkanTexture::Create("./data/textures/pattern_91/T_91_n.tga", false, true);

    gTextureCache["stones"]                = VulkanTexture::Create("./data/textures/pattern_215/T_215_d.tga", true, true); // grass with rock
    gTextureCache["stones_s"]              = VulkanTexture::Create("./data/textures/pattern_215/T_215_s.tga", true, true);
    gTextureCache["stones_n"]              = VulkanTexture::Create("./data/textures/pattern_215/T_215_n.tga", false, true);

    gTextureCache["TerrainBlendMap"] = VulkanTexture::Create("./data/terrain/blend.png", false, false);

    std::filesystem::path cubeMapPaths[6] = {
        "./data/skybox/sleepyhollow_ft.jpg", // right +z
        "./data/skybox/sleepyhollow_bk.jpg", // left  -x
        "./data/skybox/sleepyhollow_up.jpg", // up    +y
        "./data/skybox/sleepyhollow_dn.jpg", // down  -y
        "./data/skybox/sleepyhollow_rt.jpg", // right  +x
        "./data/skybox/sleepyhollow_lf.jpg", // left   -x
    };

    gTextureCache["SkyBox"] = VulkanTexture::CreateCubeMap(cubeMapPaths, true);
    gTextureCache["WhiteTexture"] = VulkanTexture::CreateWhiteTexture();
    gTextureCache["BlackTexture"] = VulkanTexture::CreateBlackTexture();
    gTextureCache["CheckBoard"]   = VulkanTexture::CreateCheckBoard();

    auto meshCube      = Mesh::CreateMeshCube(1.0f);
    auto meshGrid      = Mesh::CreateGrid(1.0f, 1.0f, 2, 2);
    auto meshCylinder  = Mesh::CreateCylinder(0.5, 0.3, 2, 100, 100);
    auto meshSphere    = Mesh::CreateSphere(0.5f, 100, 100);
    auto meshGeoSphere = Mesh::CreateGeoSphere(0.5f, 100);
    meshs.push_back(meshCube);
    meshs.push_back(meshGrid);
    meshs.push_back(meshCylinder);
    meshs.push_back(meshSphere);
    meshs.push_back(meshGeoSphere);

    AssimpImporter assimpImporter;
    auto importedMesh = assimpImporter.importMesh("./data/model/edf_soldier/edf_soldier_a.obj");
    meshs.push_back(importedMesh);
#if 1
    mRegistry.ctx().emplace<CSkyBox>().texture = gTextureCache["SkyBox"];

    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = meshGrid;
        CTransform& trans                = mRegistry.emplace<CTransform>(e);
        trans.position                   = {0, 0, 0};
        trans.rotation                   = {0, 0, 0};
        trans.scale                      = {30, 1, 30};
        CTerrain& terrain                = mRegistry.emplace<CTerrain>(e);
        terrain.terrain                  = gTerrain;
        terrain.diffuseMap0              = gTextureCache["grass"];
        terrain.specularMap0             = gTextureCache["grass_s"];
        terrain.normalMap0               = gTextureCache["grass_n"];
        terrain.diffuseMap1              = gTextureCache["stones"];
        terrain.specularMap1             = gTextureCache["stones_s"];
        terrain.normalMap1               = gTextureCache["stones_n"];
        terrain.diffuseMap2              = gTextureCache["coast_sand_rocks_02"];
        terrain.specularMap2             = gTextureCache["coast_sand_rocks_02_s"];
        terrain.normalMap2               = gTextureCache["coast_sand_rocks_02_n"];
        terrain.diffuseMap3              = gTextureCache["brown_mud_02"];
        terrain.specularMap3             = gTextureCache["brown_mud_02_s"];
        terrain.normalMap3               = gTextureCache["brown_mud_02_n"];
        terrain.diffuseMap4              = gTextureCache["coast_land_rocks_01"];
        terrain.specularMap4             = gTextureCache["coast_land_rocks_01_s"];
        terrain.normalMap4               = gTextureCache["coast_land_rocks_01_n"];
        terrain.blendMap                 = gTextureCache["TerrainBlendMap"];
    }

    // floor
    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = meshGrid;
        CTransform& trans                = mRegistry.emplace<CTransform>(e);
        trans.position                   = {-64, 0, 0};
        trans.rotation                   = {0, 0, 0};
        trans.scale                      = {1, 1, 1};
        CMaterial& mat                   = mRegistry.emplace<CMaterial>(e);
        mat.ambient                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                     = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.shininess                    = 32.0f;
        mat.diffuseMap                   = gTextureCache["metal1"];
        mat.normalMap                    = gTextureCache["metal1_nm"];
        mat.specularMap                  = gTextureCache["metal1_sm"];
        mat.texScale                     = glm::vec2(5.f, 5.0f);
    }
    // left wall (-x)
    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = meshGrid;
        CTransform& trans                = mRegistry.emplace<CTransform>(e);
        trans.position                   = {-15., 2.5f, 0.f};
        trans.rotation                   = {90.f, 90.f, 0.f};
        trans.scale                      = {30.f, 1.f, 5.f};
        CMaterial& mat                   = mRegistry.emplace<CMaterial>(e);
        mat.ambient                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                     = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                   = gTextureCache["brick_wall2"];
        mat.normalMap                    = gTextureCache["brick_wall2_nm"];
        mat.specularMap                  = gTextureCache["brick_wall2_sm"];
        mat.texScale                     = glm::vec2(10.f, 2.0f);
    }
    // right wall (+x)
    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = meshGrid;
        CTransform& trans                = mRegistry.emplace<CTransform>(e);
        trans.position                   = {15., 2.5f, 0.f};
        trans.rotation                   = {90.f, -90.f, 0.f};
        trans.scale                      = {30.f, 1.f, 5.f};
        CMaterial& mat                   = mRegistry.emplace<CMaterial>(e);
        mat.ambient                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                     = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                   = gTextureCache["brick_wall2"];
        mat.normalMap                    = gTextureCache["brick_wall2_nm"];
        mat.specularMap                  = gTextureCache["brick_wall2_sm"];
        mat.texScale                     = glm::vec2(10.f, 2.0f);
    }
    // back wall (-z)
    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = meshGrid;
        CTransform& trans                = mRegistry.emplace<CTransform>(e);
        trans.position                   = {0.f, 2.5f, -15.f};
        trans.rotation                   = {90.f, 0.f, 0.f};
        trans.scale                      = {30.f, 1.f, 5.f};
        CMaterial& mat                   = mRegistry.emplace<CMaterial>(e);
        mat.ambient                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                     = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                   = gTextureCache["brick_wall2"];
        mat.normalMap                    = gTextureCache["brick_wall2_nm"];
        mat.specularMap                  = gTextureCache["brick_wall2_sm"];
        mat.texScale                     = glm::vec2(10.f, 2.0f);
    }
    // front wall (+z)
    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = meshGrid;
        CTransform& trans                = mRegistry.emplace<CTransform>(e);
        trans.position                   = {0.f, 2.5f, 15.f};
        trans.rotation                   = {-90.f, 0.f, 0.f};
        trans.scale                      = {30.f, 1.f, 5.f};
        CMaterial& mat                   = mRegistry.emplace<CMaterial>(e);
        mat.ambient                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                     = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                   = gTextureCache["brick_wall2"];
        mat.normalMap                    = gTextureCache["brick_wall2_nm"];
        mat.specularMap                  = gTextureCache["brick_wall2_sm"];
        mat.texScale                     = glm::vec2(10.f, 2.0f);
    }

    // cubes
    {
        auto e                                    = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh          = meshCube;
        mRegistry.emplace<CTransform>(e).position = {0, 0.5f, 0};
        auto& mat                                 = mRegistry.emplace<CMaterial>(e);
        mat.ambient                               = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                               = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                              = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                            = gTextureCache["ab_crate_a"];
        mat.normalMap                             = gTextureCache["ab_crate_a_nm"];
        mat.specularMap                           = gTextureCache["ab_crate_a_sm"];
    }
    {
        auto e                                    = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh          = meshCube;
        mRegistry.emplace<CTransform>(e).position = {1, 0.5f, 0};
        auto& mat                                 = mRegistry.emplace<CMaterial>(e);
        mat.ambient                               = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                               = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                              = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                            = gTextureCache["ab_crate_a"];
        mat.normalMap                             = gTextureCache["ab_crate_a_nm"];
        mat.specularMap                           = gTextureCache["ab_crate_a_sm"];
    }
    {
        auto e                                    = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh          = meshCube;
        mRegistry.emplace<CTransform>(e).position = {-1, 0.5f, 0};
        auto& mat                                 = mRegistry.emplace<CMaterial>(e);
        mat.ambient                               = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                               = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                              = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                            = gTextureCache["ab_crate_a"];
        mat.normalMap                             = gTextureCache["ab_crate_a_nm"];
        mat.specularMap                           = gTextureCache["ab_crate_a_sm"];
    }
#endif
    {
        auto e                           = mRegistry.create();
        mRegistry.emplace<CMesh>(e).mesh = importedMesh;
        auto& transform                  = mRegistry.emplace<CTransform>(e);
        transform.position               = {-10, 0.0f, 10};
        transform.rotation               = {0.0f, 45.0f, 0.0f};
        auto& mat                        = mRegistry.emplace<CMaterial>(e);
        mat.ambient                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuse                      = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.specular                     = {1.0f, 1.0f, 1.0f, 1.0f};
        mat.diffuseMap                   = gTextureCache["edf_soldier_a"];
        mat.normalMap                    = gTextureCache["edf_soldier_a_nm"];
        mat.specularMap                  = gTextureCache["edf_soldier_a_sm"];
    }

auto createCynlinderAndSphere = [this, &meshCylinder, &meshGeoSphere](glm::vec3 position) {
        // cylinder
        {
            auto e                                    = mRegistry.create();
            mRegistry.emplace<CMesh>(e).mesh          = meshCylinder;
            mRegistry.emplace<CTransform>(e).position = position;
            auto& mat                                 = mRegistry.emplace<CMaterial>(e);
            mat.ambient                               = {1.0f, 1.0f, 1.0f, 1.0f};
            mat.diffuse                               = {1.0f, 1.0f, 1.0f, 1.0f};
            mat.specular                              = {1.0f, 1.0f, 1.0f, 1.0f};
            mat.diffuseMap                            = gTextureCache["FloorSandStone"];
            mat.normalMap                             = gTextureCache["FloorSandStone_nm"];
            mat.specularMap                           = gTextureCache["FloorSandStone_sm"];
        }
        // sphere
        {
            auto e                                    = mRegistry.create();
            mRegistry.emplace<CMesh>(e).mesh          = meshGeoSphere;
            mRegistry.emplace<CTransform>(e).position = {position.x, position.y + 1, position.z};
            auto& mat                                 = mRegistry.emplace<CMaterial>(e);
            mat.ambient                               = {1.0f, 1.0f, 1.0f, 1.0f};
            mat.diffuse                               = {1.0f, 1.0f, 1.0f, 1.0f};
            mat.specular                              = {1.0f, 1.0f, 1.0f, 1.0f};
            mat.diffuseMap                            = gTextureCache["FloorSandStone"];
            mat.normalMap                             = gTextureCache["FloorSandStone_nm"];
            mat.specularMap                           = gTextureCache["FloorSandStone_sm"];
        }
    };
#if 1
    createCynlinderAndSphere({9.0f, 1.0f, 5.0f});
    createCynlinderAndSphere({9.0f, 1.0f, 0.0f});
    createCynlinderAndSphere({9.0f, 1.0f, -5.0f});
    createCynlinderAndSphere({-9.0f, 1.0f, 5.0f});
    createCynlinderAndSphere({-9.0f, 1.0f, 0.0f});
    createCynlinderAndSphere({-9.0f, 1.0f, -5.0f});
    createCynlinderAndSphere({-3.0f, 1.0f, -7.0f});
    createCynlinderAndSphere({0.0f, 1.0f, -7.0f});
    createCynlinderAndSphere({3.0f, 1.0f, -7.0f});
#endif
    // lights
    auto createPointLight = [this, &meshSphere](const glm::vec3& position, float intensity, float range) -> entt::entity{
        auto e                                    = mRegistry.create();
        mRegistry.emplace<CTransform>(e).position = position;
        auto& light                               = mRegistry.emplace<CPointLight>(e);
        light.ambient                             = {0.2, 0.2, 0.2};
        light.diffuse                             = {1.0, 1.0, 1.0};
        light.specular                            = {1.0, 1.0, 1.0};
        light.constant                            = 1.0f;
        light.linear                              = 0.09f;
        light.quadratic                           = 0.0032f;
        light.intensity                           = intensity;
        light.range                               = range;
        mRegistry.emplace<CMesh>(e).mesh          = meshSphere;
        auto& mat                                 = mRegistry.emplace<CMaterial>(e);
        mat.ambient                               = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.diffuse                               = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.specular                              = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.diffuseMap                            = gTextureCache["ab_crate_a"];
        mat.specularMap                           = gTextureCache["ab_crate_a_nm"];
        mat.normalMap                             = gTextureCache["ab_crate_a_sm"];
        return e;
    };
    light1 = createPointLight({10, 8, -6}, 10, 10);
    light2 = createPointLight({10, 4,  0}, 20, 5);
    light3 = createPointLight({10, 8,  6}, 30, 10);

    auto createDirectionalLight = [this, &meshSphere](const glm::vec3& direction) -> entt::entity{
        auto e                                    = mRegistry.create();
        auto& light                               = mRegistry.emplace<CDirectionalLight>(e);
        light.color                               = {0.5, 0.5, 0.5};
        light.direction                           = direction;
        mRegistry.emplace<CMesh>(e).mesh          = meshSphere;
        auto& mat                                 = mRegistry.emplace<CMaterial>(e);
        mat.ambient                               = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.diffuse                               = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.specular                              = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.diffuseMap                            = gTextureCache["ab_crate_a"];
        mat.specularMap                           = gTextureCache["ab_crate_a_nm"];
        mat.normalMap                             = gTextureCache["ab_crate_a_sm"];
        return e;
    };
    createDirectionalLight({0.0, -1.0, 1.0});

    auto createSpotLight = [this, &meshSphere](const glm::vec3& position, const glm::vec3& direction) -> entt::entity{
        auto e                                    = mRegistry.create();
        mRegistry.emplace<CTransform>(e).position = position;
        auto& light                               = mRegistry.emplace<CSpotLight>(e);
        light.color                               = {0.5, 0.5, 0.5};
        light.direction                           = direction;
        light.cutOffAngle                         = 15.f;
        light.range                               = 10.0f;
        //mRegistry.emplace<CMesh>(e).mesh          = meshSphere;
        auto& mat                                 = mRegistry.emplace<CMaterial>(e);
        mat.ambient                               = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.diffuse                               = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.specular                              = {1.0f, 1.0f, 0.0f, 1.0f};
        mat.diffuseMap                            = gTextureCache["ab_crate_a"];
        mat.specularMap                           = gTextureCache["ab_crate_a_nm"];
        mat.normalMap                             = gTextureCache["ab_crate_a_sm"];
        return e;
    };
    flashLight = createSpotLight({0, 10, 0}, {1, -1, 0});

    auto sdlWindow   = Engine::Application::Get().GetWindow().getSDLWindow();
    auto win32Handle = SDL_GetPointerProperty(SDL_GetWindowProperties(sdlWindow),
                                              SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);

    VulkanImGuiRenderer::Init(Engine::Application::Get().GetWindow());

    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo{
        .sType     = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
        .pNext     = nullptr,
        .flags     = 0,
        .hinstance = nullptr,
        .hwnd      = (HWND)win32Handle};
    VkSurfaceKHR surface;
    if (VK_SUCCESS != vkCreateWin32SurfaceKHR(VulkanContext::getIntance(), &surfaceCreateInfo,
                                              nullptr, &surface)) {
        ENGINE_CORE_ERROR("vkCreateWin32SurfaceKHR fail");
    }
    vulkanSwapchain =
        new VulkanSwapchain(VulkanContext::getIntance(), VulkanContext::getPhycalDevice(),
                            VulkanContext::getDevice(), surface);
    vulkanSwapchain->build();
    // Create Command pool
    {
        VkCommandPoolCreateFlags flags{};
        // allows any command buffer allocated from a pool to be individually
        // reset to the initial state; either by calling vkResetCommandBuffer,
        // or via the implicit reset when calling vkBeginCommandBuffer.
        flags |= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        // specifies that command buffers allocated from the pool will be short-lived,
        // meaning that they will be reset or freed in a relatively short timeframe.
        flags |= VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;

        VkCommandPoolCreateInfo commandPoolCreateInfo = {
            .sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .pNext            = nullptr,
            .flags            = flags,
            .queueFamilyIndex = VulkanContext::getGraphicQueueFamilyIndex()};
        vkCreateCommandPool(VulkanContext::getDevice(), &commandPoolCreateInfo, nullptr,
                            &frameData.commandPool);
    }

    // Command buffer allocation
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool        = frameData.commandPool;
        allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(VulkanContext::getDevice(), &allocInfo, &frameData.commandBuffer);
    }

    fullScreenShader   = VulkanShaderProgram::CreateFromSpirv({"./shaders/fullscreen_vert.spv", "./shaders/fullscreen_frag.spv"});
    VulkanGraphicPipelineCreateInfo createInfo{};
    createInfo.shader = fullScreenShader;
    createInfo.enableDepthTest = false;
    createInfo.cullMode = VK_CULL_MODE_NONE;
    pipelineFullScreen = VulkanGraphicPipeline::Create(createInfo);

    depthBuffer = VulkanTexture::CreateDepthBuffer(vulkanSwapchain->getSize().width, vulkanSwapchain->getSize().height);
    //depthBuffer = VulkanContext::createTexture(
    //    vulkanSwapchain->getSize().width, vulkanSwapchain->getSize().height,
    //    VK_FORMAT_D24_UNORM_S8_UINT, 1, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void TestLayer1::onDetach() {
    // destroy the registry frist.
    // All component which hold shared ptr on vulkan ressource
    // will be released.
    mRegistry.clear();
    mRegistry.ctx().erase<CSkyBox>();

    vkDeviceWaitIdle(VulkanContext::getDevice());

    VulkanImGuiRenderer::Shutdown();

    meshs.clear();

    gTextureCache.clear();

    delete mSceneRenderer;
    vkDestroyCommandPool(VulkanContext::getDevice(), frameData.commandPool, nullptr);

    depthBuffer.reset();

    fullScreenShader.reset();
    pipelineFullScreen.reset();

    delete vulkanSwapchain;
    Renderer::Shutdown();
    VulkanContext::Shutdown();
}

void TestLayer1::onUpdate(float timeStep) {

    if(!ImGui::GetIO().WantCaptureKeyboard || !ImGui::GetIO().WantCaptureMouse) {
        cameraController.onUpdate(timeStep);
        if(gWalkCamMode) {
            const auto camPos = cameraController.getPosition();
            const float height = gTerrain->getHeight(camPos.x, camPos.z);
            cameraController.setPosition({camPos.x, height + 2.0f, camPos.z});
        }
    }

    auto& lightTrans = mRegistry.get<CTransform>(flashLight);
    auto& light = mRegistry.get<CSpotLight>(flashLight);
    lightTrans.position = cameraController.getPosition();
    light.direction     = cameraController.getDirection();

    // start command buffer
    {
        VkCommandBufferUsageFlags flags{};
        // The command buffer will be rerecorded right after executing it once.
        // flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        // This is a secondary command buffer that will be entirely within a single render pass.
        // flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
        // The command buffer can be resubmitted while it is also already pending execution.
        // flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags            = flags;   // Optional
        beginInfo.pInheritanceInfo = nullptr; // Optional

        // If the command buffer was already recorded once, then a call to vkBeginCommandBuffer will
        // implicitly reset it.
        vkBeginCommandBuffer(frameData.commandBuffer, &beginInfo);
    }

    // transition swapchain image layout
    {
        // Move the swapchain's back image layour from
        // VK_IMAGE_LAYOUT_UNDEFINED to VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        VulkanUtils::transitionImageLayout(
            frameData.commandBuffer,
            vulkanSwapchain->getImages()[vulkanSwapchain->getCurrentBackImageIndex()],
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT, 1);
    }

    // start render pass
    {
        VkRenderingAttachmentInfo colorAttachmentInfo[1]{};
        colorAttachmentInfo[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        colorAttachmentInfo[0].pNext = 0;
        colorAttachmentInfo[0].imageView =
            vulkanSwapchain->getImageViews()[vulkanSwapchain->getCurrentBackImageIndex()];
        colorAttachmentInfo[0].imageLayout        = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        colorAttachmentInfo[0].resolveMode        = VK_RESOLVE_MODE_NONE;
        colorAttachmentInfo[0].resolveImageView   = nullptr;
        colorAttachmentInfo[0].resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentInfo[0].loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentInfo[0].storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentInfo[0].clearValue.color   = {{1.0f, 0.0f, 1.0f, 1.0f}};

        VkRenderingAttachmentInfo depthAttachmentInfo{};
        depthAttachmentInfo.sType              = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        depthAttachmentInfo.pNext              = 0;
        depthAttachmentInfo.imageView          = depthBuffer->getImageView();
        depthAttachmentInfo.imageLayout        = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        depthAttachmentInfo.resolveMode        = VK_RESOLVE_MODE_NONE;
        depthAttachmentInfo.resolveImageView   = nullptr;
        depthAttachmentInfo.resolveImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentInfo.loadOp             = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentInfo.storeOp            = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentInfo.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo info{};
        info.sType                = VK_STRUCTURE_TYPE_RENDERING_INFO;
        info.pNext                = nullptr;
        info.flags                = 0;
        info.renderArea           = {0, 0, vulkanSwapchain->getSize().width,
                                     vulkanSwapchain->getSize().height};
        info.layerCount           = 1;
        info.viewMask             = 0;
        info.colorAttachmentCount = 1;
        info.pColorAttachments    = colorAttachmentInfo;
        info.pDepthAttachment     = &depthAttachmentInfo;
        info.pStencilAttachment   = nullptr;
        vkCmdBeginRendering(frameData.commandBuffer, &info);
    }

    // render stuff
    {
        VulkanContext::CmdBeginsLabel(frameData.commandBuffer, "Scene");

        VkViewport viewport;
        viewport.x        = 0;
        viewport.y        = 0;
        viewport.width    = vulkanSwapchain->getSize().width;
        viewport.height   = vulkanSwapchain->getSize().height;
        viewport.minDepth = 0;
        viewport.maxDepth = 1;
        vkCmdSetViewportWithCount(frameData.commandBuffer, 1, &viewport);

        VkRect2D rect;
        rect.offset.x      = 0;
        rect.offset.y      = 0;
        rect.extent.width  = vulkanSwapchain->getSize().width;
        rect.extent.height = vulkanSwapchain->getSize().height;
        vkCmdSetScissorWithCount(frameData.commandBuffer, 1, &rect);

        vkCmdSetPrimitiveTopology(frameData.commandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        // draw back ground
        {
            vkCmdBindPipeline(frameData.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                              pipelineFullScreen->getPipeline());
            vkCmdDraw(frameData.commandBuffer, 3, 1, 0, 0);
        }

        mSceneRenderer->render(&mRegistry, frameData.commandBuffer,
                               cameraController.getProjectonMatrix(),
                               cameraController.getViewMatrix(), cameraController.getPosition());

        VulkanContext::CmdEndLabel(frameData.commandBuffer);
    }


    VulkanImGuiRenderer::BeingFrame();
    onImGuiRender();
    VulkanImGuiRenderer::EndFrame(frameData.commandBuffer);

    // end render pass
    vkCmdEndRendering(frameData.commandBuffer);

    // transition swapchain imagelayout
    {
        // Move the swapchain's back image layour from
        // VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL to VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        VulkanUtils::transitionImageLayout(
            frameData.commandBuffer,
            vulkanSwapchain->getImages()[vulkanSwapchain->getCurrentBackImageIndex()],
            VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT, VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE, 1);
    }

    // end command buffer
    vkEndCommandBuffer(frameData.commandBuffer);

    // submit command buffer
    {
        VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
        commandBufferSubmitInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        commandBufferSubmitInfo.pNext         = nullptr;
        commandBufferSubmitInfo.commandBuffer = frameData.commandBuffer;
        commandBufferSubmitInfo.deviceMask    = 0;

        std::vector<VkSemaphoreSubmitInfo> waitSemaphoreSubmitInfo;
        {
            VkSemaphoreSubmitInfo info;
            info.sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.pNext       = nullptr;
            info.semaphore   = vulkanSwapchain->getImageAvailableSemaphores();
            info.value       = 0;
            info.stageMask   = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;
            info.deviceIndex = 0;
            waitSemaphoreSubmitInfo.push_back(info);
        }
        std::vector<VkSemaphoreSubmitInfo> signalSemaphoreSubmitInfo;
        {
            VkSemaphoreSubmitInfo info;
            info.sType       = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
            info.pNext       = nullptr;
            info.semaphore   = vulkanSwapchain->getRenderFinishSemaphores();
            info.value       = 0;
            info.stageMask   = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
            info.deviceIndex = 0;
            signalSemaphoreSubmitInfo.push_back(info);
        }

        // Move the command buffer to the Pending states
        VkSubmitInfo2 submitInfo2{};
        submitInfo2.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        submitInfo2.pNext                    = nullptr;
        submitInfo2.flags                    = 0;
        submitInfo2.waitSemaphoreInfoCount   = waitSemaphoreSubmitInfo.size();
        submitInfo2.pWaitSemaphoreInfos      = waitSemaphoreSubmitInfo.data();
        submitInfo2.commandBufferInfoCount   = 1;
        submitInfo2.pCommandBufferInfos      = &commandBufferSubmitInfo;
        submitInfo2.signalSemaphoreInfoCount = signalSemaphoreSubmitInfo.size();
        submitInfo2.pSignalSemaphoreInfos    = signalSemaphoreSubmitInfo.data();
        VK_CHECK(
            vkQueueSubmit2(VulkanContext::getGraphicQueue(), 1 /*submitCount*/, &submitInfo2, 0));
    }

    vulkanSwapchain->present(VulkanContext::getGraphicQueue(), VK_PRESENT_MODE_MAILBOX_KHR);

    // tempo
    vkDeviceWaitIdle(VulkanContext::getDevice());
}

void TestLayer1::onImGuiRender() {
    ImGuiStyle& style = ImGui::GetStyle();

    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
    ImGui::Begin("Debug info", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
    ImGui::Text("Position:  %.2f,%.2f,%.2f", cameraController.getPosition().x, cameraController.getPosition().y, cameraController.getPosition().z);
    ImGui::Text("Direction: %.2f,%.2f,%.2f", cameraController.getDirection().x, cameraController.getDirection().y, cameraController.getDirection().z);
    ImGui::End();

    ImGui::PopStyleVar(1);

    if(ImGui::Begin("Explorer")) {
        static bool useGamma = mSceneRenderer->isUseGammaCorrection();
        if(ImGui::Checkbox("Use Gamma correction", &useGamma)) {
            mSceneRenderer->setUseGammaCorrection(useGamma);
        }

        ImGui::SameLine();

        static float gammaValue = mSceneRenderer->getGammaCorrectionValue();
        if(ImGui::DragFloat("Gamma", &gammaValue, 1.0f, 0.0f, 10.0f)) {
            mSceneRenderer->setGammaCorrectionValue(gammaValue);
        }

        ImGui::Checkbox("WalkCamMode", &gWalkCamMode);

        static bool displayTerrain = true;
        if(ImGui::Checkbox("Display Terrain", &displayTerrain)) {
            mSceneRenderer->setTerrainVisible(displayTerrain);
        }

        static bool displayTerrainAABB = false;
        if(ImGui::Checkbox("Display Terrain AABB", &displayTerrainAABB)) {
            mSceneRenderer->setTerrainAABBVisible(displayTerrainAABB);
        }

        static auto ambientLight = mSceneRenderer->getAmbientLight();
        if(ImGui::ColorEdit3("Ambient Light", &ambientLight.x, ImGuiColorEditFlags_Float)) {
            mSceneRenderer->setAmbientLight(ambientLight);
        }

        for(const auto& entity : mRegistry.view<CDirectionalLight>()) {
            auto& light = mRegistry.get<CDirectionalLight>(entity);
            ImGui::TextUnformatted("Directional Light");
            ImGui::ColorEdit3("Color:", &light.color.x);
            ImGui::DragFloat3("Direction:", &light.direction.x, 1.0f, -1.0f, 1.0f);
        }
        unsigned int i = 0;
        for(const auto& entity : mRegistry.view<CTransform, CPointLight>()) {
            auto& trans = mRegistry.get<CTransform>(entity);
            auto& light = mRegistry.get<CPointLight>(entity);

            ImGui::Text("Point Lights %i", i);
            ImGui::PushID(i);
            ImGui::Checkbox("Enable",     &light.enable);
            ImGui::DragFloat3("Position", &trans.position.x);
            ImGui::ColorEdit3("Color",    &light.diffuse.x);
            ImGui::DragFloat("Range",     &light.range, 1.0f, 0.1f);
            ImGui::PopID();
            i++;
        }
        for(const auto& entity : mRegistry.view<CTransform, CSpotLight>()) {
            auto& trans = mRegistry.get<CTransform>(entity);
            auto& light = mRegistry.get<CSpotLight>(entity);

            ImGui::Text("Spot Lights %i", i);
            ImGui::PushID(i);
            ImGui::Checkbox("Enable",      &light.enable);
            ImGui::DragFloat3("Position",  &trans.position.x);
            ImGui::ColorEdit3("Color",     &light.color.x);
            ImGui::DragFloat("Range",      &light.range, 1.0f, 0.1f);
            ImGui::DragFloat("CutOffAngle",&light.cutOffAngle, 1.0f, 1.0f, 180.f);
            ImGui::PopID();
            i++;
        }

        VulkanImGuiRenderer::AddImage(gTextureCache["WhiteTexture"], ImVec2(100,100));
        VulkanImGuiRenderer::AddImage(gTextureCache["BlackTexture"], ImVec2(100,100));
        VulkanImGuiRenderer::AddImage(gTextureCache["CheckBoard"], ImVec2(100,100));
    }
    ImGui::End();
}

bool TestLayer1::onEvent(const Engine::Event& event) {
    if(ImGui::GetIO().WantCaptureKeyboard || ImGui::GetIO().WantCaptureMouse) {
        return false;
    }

    cameraController.onEvent(event);

    event.dispatch<Engine::WindowResizedEvent>([this](const auto& e) {
        // rebuild depth buffer.
        vkDeviceWaitIdle(VulkanContext::getDevice());
        vulkanSwapchain->build();
        depthBuffer = VulkanTexture::CreateDepthBuffer(vulkanSwapchain->getSize().width, vulkanSwapchain->getSize().height);
    });
    event.dispatch<Engine::KeyEvent>([this](const Engine::KeyEvent& e) {
        if (e.isPressed()) {
            if (e.getKey() == Engine::KeyCode::Escape) {
                Engine::Application::Get().close();
            }
            if (e.getKey() == Engine::KeyCode::Key1) {
                Engine::Application::Get().GetWindow().setFullScreen(true);
            }
            if (e.getKey() == Engine::KeyCode::Key2) {
                Engine::Application::Get().GetWindow().setFullScreen(false);
            }
            if (e.getKey() == Engine::KeyCode::KeyPad0) {
                Engine::Application::Get().GetWindow().toogleMouseGrab();
            }
            if (e.getKey() == Engine::KeyCode::KeyPadEnter) {
                Engine::Application::Get().GetWindow().toogleMouseRelativeMode();
            }
            if (e.getKey() == Engine::KeyCode::KeyPad1) {
                auto& light = mRegistry.get<CPointLight>(light1);
                light.enable = !light.enable;
            }
            if (e.getKey() == Engine::KeyCode::KeyPad2) {
                auto& light = mRegistry.get<CPointLight>(light2);
                light.enable = !light.enable;
            }
            if (e.getKey() == Engine::KeyCode::KeyPad3) {
                auto& light = mRegistry.get<CPointLight>(light3);
                light.enable = !light.enable;
            }
            if (e.getKey() == Engine::KeyCode::KeyPadMinus) {
                mSceneRenderer->toggleUseBlinnPhong();
            }
            if (e.getKey() == Engine::KeyCode::F) {
                auto& light = mRegistry.get<CSpotLight>(flashLight);
                light.enable = !light.enable;
            }
            if (e.getKey() == Engine::KeyCode::G) {
                mSceneRenderer->toggleGammaCorrection();
            }
        }
    });
    event.dispatch<Engine::MouseButtonEvent>([](const Engine::MouseButtonEvent& e) {
        // ENGINE_INFO("{}: {}", __FUNCTION__, e.toString());
    });
    event.dispatch<Engine::MouseMovedEvent>([](const Engine::MouseMovedEvent& e) {
        // ENGINE_INFO("{}: {}", __FUNCTION__, e.toString());
    });
    return false;
}
