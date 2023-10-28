#pragma once

#include <glm/glm.hpp>
#include "utils/rgba.h"
#include "raytracer.h"
#include "raytracescene.h"

// A forward declaration for the RaytraceScene class

class RayTraceScene;

// A class representing a ray-tracer

class RayTracer
{
public:
    struct Config {
        bool enableShadow        = false;
        bool enableReflection    = false;
        bool enableRefraction    = false;
        bool enableTextureMap    = false;
        bool enableTextureFilter = false;
        bool enableParallelism   = false;
        bool enableSuperSample   = false;
        bool enableAcceleration  = false;
        bool enableDepthOfField  = false;
        int maxRecursiveDepth    = 4;
        bool onlyRenderNormals   = false;
    };

    Camera camera;
    RenderData metaData;
    SceneCameraData cameradata;
    SceneGlobalData globalData;
    std::vector<RenderShapeData> primiTypes;
    std::vector<SceneLightData> lights;
    glm::mat4 viewMatrix;

    glm::vec4 world_eye;
    glm::vec4 world_d;

public:
    RayTracer(Config config);

    // Renders the scene synchronously.
    // The ray-tracer will render the scene and fill imageData in-place.
    // @param imageData The pointer to the imageData to be filled.
    // @param scene The scene to be rendered.
    void render(RGBA *imageData, const RayTraceScene &scene);
    glm::vec4 rayTracer(glm::vec4 world_eye, glm::vec4 world_d, std::vector<RenderShapeData> primiTypes, int depth);

private:
    const Config m_config;
};

