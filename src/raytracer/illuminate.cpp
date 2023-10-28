#include "utils/rgba.h"
#include "raytracer/illuminate.h"
#include "raytracer/raytracer.h"
#include "utils/scenedata.h"
#include "intersect.h"
#include <iostream>

// Helper function to convert illumination to RGBA, applying some form of tone-mapping (e.g. clamping) in the process
RGBA Illuminate::toRGBA(const glm::vec4 &illumination) {
    // Task 1
    RGBA color;
    color.r = 255.0 * std::fmin(std::fmax(illumination.x, 0.0), 1.0);
    color.g = 255.0 * std::fmin(std::fmax(illumination.y, 0.0), 1.0);
    color.b = 255.0 * std::fmin(std::fmax(illumination.z, 0.0), 1.0);
    return color;
}

// Calculates the RGBA of a pixel from intersection infomation and globally-defined coefficients
glm::vec4 Illuminate::phong(glm::vec4  position,
                       std::vector<RenderShapeData> primiTypes,
                      glm::vec3  normal,
                      glm::vec3  directionToCamera,
                      SceneMaterial  &material,
                      std::vector<SceneLightData> &lights,
                      SceneGlobalData globalData) {
    // Normalizing directions
    normal            = glm::normalize(normal);
    directionToCamera = glm::normalize(directionToCamera);



    float ka = globalData.ka;
    float ks = globalData.ks;
    float kd = globalData.kd;

    // Output illumination (we can ignore opacity)
    glm::vec4 illumination(0, 0, 0, 1);


    glm::vec4 ambient = material.cAmbient * ka;
    illumination = illumination + ambient;

    for (const SceneLightData &light : lights) {
        switch(light.type){
        case LightType::LIGHT_DIRECTIONAL:{
            float distanceToLight = std::numeric_limits<float>::max();
            if (has_shadow(position, primiTypes, -light.dir, distanceToLight) == false){
            glm::vec3 new_light_pos = glm::vec3(-light.dir.x, -light.dir.y, -light.dir.z);
            glm::vec4 diffuse = material.cDiffuse * kd;
            glm::vec3 directionToLight = glm::normalize(new_light_pos);
            float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);
            illumination += light.color * diffuse * product;

            glm::vec4 specular = material.cSpecular * ks;
            float reflect = std::fmax(glm::dot(directionToLight, normal), 0.0);
            glm::vec3 directionReflection = glm::normalize(2.0f * reflect * normal - directionToLight);
            float r_product = std::fmin(std::fmax(glm::dot(directionReflection, directionToCamera), 0.0), 1.0);
            r_product = pow(r_product, material.shininess);
            illumination += light.color * specular * r_product;
            }
            break;
        }
        case LightType::LIGHT_SPOT:{

            glm::vec4 distance = light.pos - position;
            glm::vec3 new_distance = glm::vec3(distance.x, distance.y, distance.z);
            glm::vec3 directionToLight = glm::normalize(new_distance);

            float distanceToLight = glm::length(distance);

            if (has_shadow(position, primiTypes, distance, distanceToLight) == false){
            float real_distance = sqrt(pow(distance.x, 2) + pow(distance.y, 2) + pow(distance.z, 2));
            float att = std::fmin(1.0, 1.0f / (light.function.x + real_distance * light.function.y + pow(real_distance, 2) * light.function.z));

            float outer_angle = light.angle;
            float inner_angle = outer_angle - light.penumbra;
            glm::vec3 spot_light_dir = glm::normalize(glm::vec3(-light.dir.x, -light.dir.y, -light.dir.z));
            float x = acos(std::fmin(std::fmax(glm::dot(directionToLight, spot_light_dir), -1.0), 1.0));
            float falloff = -2 * pow(((abs(x - inner_angle)) / light.penumbra), 3) + 3 * pow((abs((x - inner_angle)) / light.penumbra), 2);

            if (x <= inner_angle){
                glm::vec4 diffuse = material.cDiffuse * kd;
                float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);
                illumination += light.color * att * diffuse * product;

                glm::vec4 specular = material.cSpecular * ks;
                float reflect = std::fmax(glm::dot(directionToLight, normal), 0.0);
                glm::vec3 directionReflection = glm::normalize(2.0f * reflect * normal - directionToLight);
                float r_product = std::fmin(std::fmax(glm::dot(directionReflection, directionToCamera), 0.0), 1.0);
                r_product = pow(r_product, material.shininess);
                illumination += light.color * att * specular * r_product;

            }else if (x > inner_angle && x <= outer_angle){
                glm::vec4 diffuse = material.cDiffuse * kd;
                float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);
                illumination += light.color * att * (1 - falloff) * diffuse * product;

                glm::vec4 specular = material.cSpecular * ks;
                float reflect = std::fmax(glm::dot(directionToLight, normal), 0.0);
                glm::vec3 directionReflection = glm::normalize(2.0f * reflect * normal - directionToLight);
                float r_product = std::fmin(std::fmax(glm::dot(directionReflection, directionToCamera), 0.0), 1.0);
                r_product = pow(r_product, material.shininess);
                illumination += light.color * att * (1 - falloff) *  specular * r_product;
            }
            }

            break;
        }
        case LightType::LIGHT_POINT:{
            glm::vec4 distance = light.pos - position;
            glm::vec3 new_distance = glm::vec3(distance.x, distance.y, distance.z);
            glm::vec3 directionToLight = glm::normalize(new_distance);

            float distanceToLight = glm::length(distance);

            if (has_shadow(position, primiTypes, distance, distanceToLight) == false){
            float real_distance = sqrt(pow(distance.x, 2) + pow(distance.y, 2) + pow(distance.z, 2));
            float att = std::fmin(1.0, 1.0f / (light.function.x + real_distance * light.function.y + pow(real_distance, 2) * light.function.z));

            glm::vec4 diffuse = material.cDiffuse * kd;

            float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);
            illumination += light.color * att * diffuse * product;

            glm::vec4 specular = material.cSpecular * ks;
            float reflect = std::fmax(glm::dot(directionToLight, normal), 0.0);
            glm::vec3 directionReflection = glm::normalize(2.0f * reflect * normal - directionToLight);
            float r_product = std::fmin(std::fmax(glm::dot(directionReflection, directionToCamera), 0.0), 1.0);
            r_product = pow(r_product, material.shininess);
            illumination += light.color * att * specular * r_product;
            }
            break;
        }
        default:
            break;
        }


    }

//    RGBA returnValue = toRGBA(illumination);
    return illumination;
}

bool Illuminate::has_shadow(glm::vec4 position, std::vector<RenderShapeData> primiTypes, glm::vec4 world_directionToLight, float distanceToLight){

    Intersect intersect;
    bool is_shadow = false;
    float tMin = std::numeric_limits<float>::max();

    float t;
    glm::vec4 ray;

    for (int s = 0; s < primiTypes.size(); s++){
        RenderShapeData curr_shape = primiTypes[s];
        glm::mat4 ctm = curr_shape.ctm;

        glm::vec4 transformedPosition = glm::inverse(ctm) * position;
        glm::vec4 directionToLight = glm::inverse(ctm) * world_directionToLight;

        float epsilon = 0.001f;
        glm::vec4 shadowRayStart = transformedPosition + epsilon * directionToLight;

    switch(curr_shape.primitive.type){
    case PrimitiveType::PRIMITIVE_CUBE:{
        bool success = intersect.intersect_cube(shadowRayStart, directionToLight, t, ray);
        if (success){
            if (t < distanceToLight) {
                is_shadow = true;
            }
        }
        break;
    }
    case PrimitiveType::PRIMITIVE_CONE:{
        bool success = intersect.intersect_cone(shadowRayStart, directionToLight, t, ray);
        if (success){
            if (t < distanceToLight) {
                is_shadow = true;
            }
        }
        break;
    }
    case PrimitiveType::PRIMITIVE_CYLINDER:{
        bool success = intersect.intersect_cylinder(shadowRayStart, directionToLight, t, ray);
        if (success){
            if (t < distanceToLight) {
                is_shadow = true;
            }
        }
        break;
    }
    case PrimitiveType::PRIMITIVE_SPHERE:{
        bool success = intersect.intersect_sphere(shadowRayStart, directionToLight, t, ray);
        if (success){
            if (t < distanceToLight) {
                is_shadow = true;
            }
        }
        break;
    }
    default:
        break;
    }
    if (is_shadow){
        break;
    }
    }

    return is_shadow;
}
