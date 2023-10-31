#include "utils/rgba.h"
#include "raytracer/illuminate.h"
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

glm::vec4 Illuminate::toVec4(const RGBA &pixel) {
    glm::vec4 color;
    color.x = pixel.r / 255.0;
    color.y = pixel.g / 255.0;
    color.z = pixel.b / 255.0;
    return color;
}

// Calculates the RGBA of a pixel from intersection infomation and globally-defined coefficients
glm::vec4 Illuminate::phong(glm::vec4  position,
                       std::vector<RenderShapeData> primiTypes,
                      glm::vec3  normal,
                      glm::vec3  directionToCamera,
                      SceneMaterial  &material,
                      glm::vec4 texture,
                      std::vector<SceneLightData> &lights,
                      SceneGlobalData globalData) {
    // Normalizing directions
    normal            = glm::normalize(normal);
    directionToCamera = glm::normalize(directionToCamera);

    float blend = material.blend;
    float ka = globalData.ka;
    float ks = globalData.ks;
    float kd = globalData.kd;

    // Output illumination (we can ignore opacity)
    glm::vec4 illumination(0, 0, 0, 1);


    glm::vec4 ambient = material.cAmbient * ka;
    glm::vec4 diffuse = material.cDiffuse * kd;
    glm::vec4 specular = material.cSpecular * ks;
    illumination = illumination + ambient;

    for (const SceneLightData &light : lights) {
        switch(light.type){
        case LightType::LIGHT_DIRECTIONAL:{
            float distanceToLight = std::numeric_limits<float>::max();

            if (has_shadow(position, primiTypes, -light.dir, distanceToLight) == false){

                glm::vec3 new_light_pos = glm::vec3(-light.dir.x, -light.dir.y, -light.dir.z);
                glm::vec3 directionToLight = glm::normalize(new_light_pos);
                float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);
                illumination += light.color * (blend * texture + (1.0f - blend) * diffuse) * product;

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

                float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);

                float reflect = std::fmax(glm::dot(directionToLight, normal), 0.0);
                glm::vec3 directionReflection = glm::normalize(2.0f * reflect * normal - directionToLight);
                float r_product = std::fmin(std::fmax(glm::dot(directionReflection, directionToCamera), 0.0), 1.0);
                r_product = pow(r_product, material.shininess);

                if (x <= inner_angle){
                    illumination += light.color * att * (blend * texture + (1.0f - blend) * diffuse) * product;
                    illumination += light.color * att * specular * r_product;

                }else if (x > inner_angle && x <= outer_angle){
                    illumination += light.color * att * (1 - falloff) * (blend * texture + (1.0f - blend) * diffuse) * product;
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


                float product = std::fmin(std::fmax(glm::dot(normal, directionToLight), 0.0), 1.0);
                illumination += light.color * att * (blend * texture + (1.0f - blend) * diffuse) * product;

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

glm::vec4 Illuminate::uv_cylinder(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV){
    float width = (float) t_width;
    float height = (float) t_height;
    float epsilon = 0.001f;
    float u;
    float v;
    int c;
    int r;
    if (abs(intersect_position.y - 0.5) <= epsilon) {
        u = intersect_position.x + 0.5 + epsilon;
        v = 1.0f - (intersect_position.z + 0.5) + epsilon;
    }else if (abs(intersect_position.y + 0.5) <= epsilon){
        u = intersect_position.x + 0.5 + epsilon;
        v = intersect_position.z + 0.5 + epsilon;
    }else{
        float theta = atan2(intersect_position.x, intersect_position.z) - (M_PI / 2.0f);
        u = theta / (2.0f * M_PI) + epsilon;
        v = intersect_position.y + 0.5 + epsilon;
    }
    c = static_cast<int>(floor(u * repeatedU * width)) % t_width;
    r = static_cast<int>(floor((1 - v) * repeatedV * height)) % t_height;
    if (c == t_width){
        c -= 1;
    }
    if (r == t_height){
        r -= 1;
    }
    return toVec4(texture[r * t_width + c]);

}

glm::vec4 Illuminate::uv_cone(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV){
    float width = (float) t_width;
    float height = (float) t_height;
    float epsilon = 0.001f;
    float u;
    float v;
    int c;
    int r;
    if (abs(intersect_position.y + 0.5) <= epsilon){
        u = intersect_position.x + 0.5 + epsilon;
        v = intersect_position.z + 0.5 + epsilon;

    }else{
        float theta = atan2(intersect_position.x, intersect_position.z) - (M_PI / 2.0f);
        u = theta / (2.0f * M_PI) + epsilon;
        v = intersect_position.y + 0.5 + epsilon;
    }
    c = static_cast<int>(floor(u * repeatedU * width)) % t_width;
    r = static_cast<int>(floor((1 - v) * repeatedV * height)) % t_height;
    if (c == t_width){
        c -= 1;
    }
    if (r == t_height){
        r -= 1;
    }
    return toVec4(texture[r * t_width + c]);

}

glm::vec4 Illuminate::uv_cube(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV){
    float width = (float) t_width;
    float height = (float) t_height;
    float epsilon = 0.001f;
    float u;
    float v;
    int c;
    int r;
    if (abs(intersect_position.y - 0.5) <= epsilon) {
        u = intersect_position.x + 0.5 + epsilon;
        v = 1.0f - (intersect_position.z + 0.5) + epsilon;
    }else if (abs(intersect_position.y + 0.5) <= epsilon){
        u = intersect_position.x + 0.5 + epsilon;
        v = intersect_position.z + 0.5 + epsilon;
    }else if (abs(intersect_position.x - 0.5) <= epsilon) {
        u = 1.0f - (intersect_position.z + 0.5) + epsilon;
        v = intersect_position.y + 0.5 + epsilon;
    }else if (abs(intersect_position.x + 0.5) <= epsilon){
        u = intersect_position.z + 0.5 + epsilon;
        v = intersect_position.y + 0.5 + epsilon;
    }else if (abs(intersect_position.z - 0.5) <= epsilon) {
        u = intersect_position.x + 0.5 + epsilon;
        v = intersect_position.y + 0.5 + epsilon;
    }else if (abs(intersect_position.z + 0.5) <= epsilon){
        u = 1.0f - (intersect_position.x + 0.5) + epsilon;
        v = intersect_position.y + 0.5 + epsilon;
    }
    c = static_cast<int>(floor(u * repeatedU * width)) % t_width;
    r = static_cast<int>(floor((1 - v) * repeatedV * height)) % t_height;

    if (c == t_width){
        c = c - 1;
    }
    if (r == t_height){
        r = r - 1;
    }
    return toVec4(texture[r * t_width + c]);

}

glm::vec4 Illuminate::uv_sphere(glm::vec4 intersect_position, RGBA* texture, int t_width, int t_height, int repeatedU, int repeatedV){
    float width = (float) t_width;
    float height = (float) t_height;
    float epsilon = 0.001f;

    float theta = atan2(intersect_position.x, intersect_position.z) - (M_PI / 2.0f);
    float phi = asin(intersect_position.y / 0.5f);
    float u = theta / (2.0f * M_PI) + epsilon;
    float v = phi / M_PI + 0.5f + epsilon;
    int c = static_cast<int>(floor(u * repeatedU * width)) % t_width;
    int r = static_cast<int>(floor((1 - v) * repeatedV * height)) % t_height;

    if (c == t_width){
        c -= 1;
    }
    if (r == t_height){
        r -= 1;
    }
    return toVec4(texture[r * t_width + c]);

}

