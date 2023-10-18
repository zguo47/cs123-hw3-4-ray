#pragma once

#include <glm/glm.hpp>
#include "utils/rgba.h"
#include "utils/scenedata.h"

class Intersect{
public:

    RGBA toRGBA(const glm::vec4 &illumination);
    RGBA phong(glm::vec3  normal,
               glm::vec3  directionToCamera,
               SceneMaterial  &material,
               std::vector<SceneLightData> &lights,
               SceneGlobalData globalData);

    bool intersect_cone(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection);
    bool intersect_plane(glm::vec4 eye, glm::vec4 d, glm::vec3& P_plane, glm::vec3& N, float &t, glm::vec4& intersection);
    bool intersect_cube(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection);
    bool intersect_cylinder(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection);
    bool intersect_sphere(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection);

    glm::vec3 normal_cone(glm::vec4& intersection, glm::mat4& ctm);
    glm::vec3 normal_cube(glm::vec4& intersection, glm::mat4& ctm);
    glm::vec3 normal_cylinder(glm::vec4& intersection, glm::mat4& ctm);
    glm::vec3 normal_sphere(glm::vec4& intersection, glm::mat4& ctm);
};
