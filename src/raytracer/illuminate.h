#include "utils/rgba.h"
#include "utils/scenedata.h"
#include "raytracer/raytracer.h"

class Illuminate{
public:

    RGBA toRGBA(const glm::vec4 &illumination);
    glm::vec4 toVec4(const RGBA &pixel);
    glm::vec4 phong(glm::vec4  position,
               std::vector<RenderShapeData> primiTypes,
           glm::vec3  normal,
           glm::vec3  directionToCamera,
           SceneMaterial  &material,
                    glm::vec4 texture,
           std::vector<SceneLightData> &lights,
           SceneGlobalData globalData);
    bool has_shadow(glm::vec4 position, std::vector<RenderShapeData> primiTypes, glm::vec4 lightPosition, float distanceToLight);

    glm::vec4 uv_cylinder(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV);
    glm::vec4 uv_sphere(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV);
    glm::vec4 uv_cone(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV);
    glm::vec4 uv_cube(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV);

};
