#include "utils/rgba.h"
#include "utils/scenedata.h"
#include "raytracer/raytracer.h"

class Illuminate{
public:

    RGBA toRGBA(const glm::vec4 &illumination);
    glm::vec4 phong(glm::vec4  position,
               std::vector<RenderShapeData> primiTypes,
           glm::vec3  normal,
           glm::vec3  directionToCamera,
           SceneMaterial  &material,
           std::vector<SceneLightData> &lights,
           SceneGlobalData globalData);
    bool has_shadow(glm::vec4 position, std::vector<RenderShapeData> primiTypes, glm::vec4 lightPosition, float distanceToLight);

};
