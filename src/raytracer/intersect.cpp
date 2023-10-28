#include "intersect.h"
#include "utils/scenedata.h"

bool Intersect::intersect_cylinder(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection){
    float A = d.x * d.x + d.z * d.z;
    float B = 2.0f * eye.x * d.x + 2.0f * eye.z * d.z;
    float C = eye.x * eye.x + eye.z * eye.z - 0.25f;

    float discriminant = B * B - 4 * A * C;
    float sqrtDiscriminant = std::sqrt(discriminant);
    float tMin = std::numeric_limits<float>::max();

    if (discriminant >= 0.0f){
        float t1 = (-B + sqrtDiscriminant) / (2.0f * A);
        float t2 = (-B - sqrtDiscriminant) / (2.0f * A);

        glm::vec4 int1 = eye + t1 * d;
        glm::vec4 int2 = eye + t2 * d;

        if (int1.y >= -0.5 && int1.y <= 0.5 && t1 > 0) {
            tMin = std::fmin(tMin, t1);
        }

        if (int2.y >= -0.5 && int2.y <= 0.5 && t2 > 0) {
            tMin = std::fmin(tMin, t2);
        }
    }

    // Check intersections with the top and bottom caps
    glm::vec3 N_top(0, 1, 0);
    glm::vec3 N_bottom(0, -1, 0);
    glm::vec3 P_top(0, 0.5, 0);
    glm::vec3 P_bottom(0, -0.5, 0);

    float t_top, t_bot;
    glm::vec4 int_top, int_bot;

    if (intersect_plane(eye, d, P_top, N_top, t_top, int_top)) {
        if (int_top.x * int_top.x + int_top.z * int_top.z <= 0.25 && t_top > 0) {
            tMin = std::fmin(tMin, t_top);
        }
    }

    if (intersect_plane(eye, d, P_bottom, N_bottom, t_bot, int_bot)) {
        if (int_bot.x * int_bot.x + int_bot.z * int_bot.z <= 0.25 && t_bot > 0) {
            tMin = std::fmin(tMin, t_bot);
        }
    }

    if (tMin == std::numeric_limits<float>::max()) {
        return false; // No valid intersection
    }

    t = tMin;
    intersection = eye + t * d;
    return true;
}


bool Intersect::intersect_plane(glm::vec4 eye, glm::vec4 d, glm::vec3& P_plane, glm::vec3& N, float &t, glm::vec4& intersection){

    glm::vec3 new_d = {d.x, d.y, d.z};
    glm::vec3 new_eye = {eye.x, eye.y, eye.z};

    float denominator = glm::dot(new_d, N);
    if (std::abs(denominator) < 1e-6) {
        return false;
    }

    t = glm::dot(P_plane - new_eye, N) / denominator;
    if (t < 0){
        return false;
    }
    intersection = eye + t * d;
    return true;

}

bool Intersect::intersect_cube(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection){

    float tMin = std::numeric_limits<float>::infinity();
    glm::vec4 closestIntersection;
    bool hasIntersection = false;

    // Define all six planes for the cube
    glm::vec3 planesNormals[6] = {
        { 1, 0, 0},
        {-1, 0, 0},
        {0, 1, 0},
        {0, -1, 0},
        {0, 0, 1},
        {0, 0, -1}
    };
    float planeOffsets[6] = {-0.5f, -0.5f, -0.5f, -0.5f, -0.5f, -0.5f};

    glm::vec3 N1(1, 0, 0);
    glm::vec3 N2(0, 1, 0);
    glm::vec3 N3(0, 0, 1);


    // Check intersection with each plane
    for(int i = 0; i < 6; i++) {
        glm::vec3 P_plane(planeOffsets[i] * planesNormals[i].x,
                          planeOffsets[i] * planesNormals[i].y,
                          planeOffsets[i] * planesNormals[i].z);
        float temp_t;
        glm::vec4 temp_intersection;
        glm::vec3 N;
        if (i == 0 || i == 1){
            N = N1;
        }else if (i == 2 || i == 3){
            N = N2;
        }else if (i == 4 || i == 5){
            N = N3;
        }

        if(intersect_plane(eye, d, P_plane, N, temp_t, temp_intersection)) {
            // Check if the intersection is within the cube's boundaries
            if (std::abs(temp_intersection.x) <= 0.5f + 1e-5 &&
                std::abs(temp_intersection.y) <= 0.5f + 1e-5 &&
                std::abs(temp_intersection.z) <= 0.5f + 1e-5) {

                // Update if this is the closest intersection found so far
                if(temp_t < tMin) {
                    tMin = temp_t;
                    closestIntersection = temp_intersection;
                    hasIntersection = true;
                }
            }
        }
    }

    if(hasIntersection) {
        t = tMin;
        intersection = closestIntersection;
    }

    return hasIntersection;
}

bool Intersect::intersect_cone(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection){

    glm::vec3 apex(0, 0.5, 0);
    float radius = 0.5f;
    float slope = radius / 1.0f;

    float tMin = std::numeric_limits<float>::infinity();
    glm::vec4 closestIntersection;
    bool hasIntersection = false;

    float A = d.x * d.x + d.z * d.z - slope * slope * d.y * d.y;
    float B = 2 * (d.x * eye.x + d.z * eye.z - slope * slope * d.y * (eye.y - apex.y));
    float C = eye.x * eye.x + eye.z * eye.z - slope * slope * (eye.y - apex.y) * (eye.y - apex.y);

    float discriminant = B*B - 4*A*C;
    if (discriminant >= 0){
        float t1 = (-B - std::sqrt(discriminant)) / (2*A);
        float t2 = (-B + std::sqrt(discriminant)) / (2*A);

        glm::vec4 intersection1 = eye + t1 * d;
        if (t1 > 0 && intersection1.y >= -0.5 && intersection1.y <= apex.y){
            tMin = t1;
            closestIntersection = intersection1;
            hasIntersection = true;
        }

        glm::vec4 intersection2 = eye + t2 * d;
        if (t2 > 0 && intersection2.y >= -0.5 && intersection2.y <= apex.y && t2 < tMin){
            tMin = t2;
            closestIntersection = intersection2;
            hasIntersection = true;
        }
    }

    float t_base;
    glm::vec4 intersection_base;
    glm::vec3 P_plane(0, -0.5, 0);
    glm::vec3 N(0, 1, 0);
    if (intersect_plane(eye, d, P_plane, N, t_base, intersection_base) && intersection_base.x * intersection_base.x + intersection_base.z * intersection_base.z <= radius * radius && t_base < tMin){
        tMin = t_base;
        closestIntersection = intersection_base;
        hasIntersection = true;
    }


    if(hasIntersection) {
        t = tMin;
        intersection = closestIntersection;
    }

    return hasIntersection;
}

bool Intersect::intersect_sphere(glm::vec4 eye, glm::vec4 d, float &t, glm::vec4& intersection){

    glm::vec3 new_d = {d.x, d.y, d.z};
    glm::vec3 new_eye = {eye.x, eye.y, eye.z};

    float radius = 0.5f;


    float A = glm::dot(new_d, new_d);
    float B = 2.0f * glm::dot(new_eye, new_d);
    float C = glm::dot(new_eye, new_eye) - radius * radius;

    float discriminant = B*B - 4*A*C;

    if (discriminant < 0) {
        return false;
    }

    float t1 = (-B - std::sqrt(discriminant)) / (2*A);
    float t2 = (-B + std::sqrt(discriminant)) / (2*A);

    if (t1 > 0 && (t1 < t2 || t2 <= 0)) {
        t = t1;
    } else if (t2 > 0 && (t2 < t1 || t1 <= 0)) {
        t = t2;
    } else {
        return false;
    }

    intersection = eye + t * d;
    return true;

}

glm::vec3 Intersect::normal_cone(glm::vec4& intersection, glm::mat4& ctm){
    glm::vec3 result;
    const float epsilon = 1e-5f;
    if (std::abs(intersection.y - 0.5f) < epsilon) {
        result = glm::vec3(0, 1, 0);
    }else if (std::abs(intersection.y + 0.5f) < epsilon) {
        result = glm::vec3(0, -1, 0);
    }else{
        result = glm::vec3(2.0f * intersection.x, (0.5f - intersection.y) / 2.0f, 2.0f * intersection.z);
    }

    result = glm::normalize(result);
    glm::vec4 new_normal(result.x, result.y, result.z, 0);
    glm::vec4 world_normal_temp = glm::inverse(glm::transpose(ctm)) * new_normal;
    glm::vec3 world_normal(world_normal_temp.x, world_normal_temp.y, world_normal_temp.z);
    return glm::normalize(world_normal);
}

glm::vec3 Intersect::normal_cube(glm::vec4& intersection, glm::mat4& ctm){
    glm::vec3 result;
    const float epsilon = 1e-5f;
    if (std::abs(intersection.y - 0.5f) < epsilon) {
        result = glm::vec3(0, 1, 0);
    }else if (std::abs(intersection.y + 0.5f) < epsilon) {
        result = glm::vec3(0, -1, 0);
    }else if (std::abs(intersection.x - 0.5f) < epsilon) {
        result = glm::vec3(1, 0, 0);
    }else if (std::abs(intersection.x + 0.5f) < epsilon) {
        result = glm::vec3(-1, 0, 0);
    }else if (std::abs(intersection.z - 0.5f) < epsilon) {
        result = glm::vec3(0, 0, 1);
    }else if (std::abs(intersection.z + 0.5f) < epsilon) {
        result = glm::vec3(0, 0, -1);
    }

    result = glm::normalize(result);
    glm::vec4 new_normal(result.x, result.y, result.z, 0);
    glm::vec4 world_normal_temp = glm::inverse(glm::transpose(ctm)) * new_normal;
    glm::vec3 world_normal(world_normal_temp.x, world_normal_temp.y, world_normal_temp.z);
    return glm::normalize(world_normal);
}

glm::vec3 Intersect::normal_cylinder(glm::vec4& intersection, glm::mat4& ctm){
    glm::vec3 result;
    const float epsilon = 1e-5f;

    if (std::abs(intersection.y - 0.5f) < epsilon) {
        result = glm::vec3(0, 1, 0);
    }else if (std::abs(intersection.y + 0.5f) < epsilon) {
        result = glm::vec3(0, -1, 0);
    }else{
        result = glm::vec3(intersection.x, 0, intersection.z);
    }
    result = glm::normalize(result);
    glm::vec4 new_normal(result.x, result.y, result.z, 0);
    glm::vec4 world_normal_temp = glm::inverse(glm::transpose(ctm)) * new_normal;
    glm::vec3 world_normal(world_normal_temp.x, world_normal_temp.y, world_normal_temp.z);
    return glm::normalize(world_normal);
}

glm::vec3 Intersect::normal_sphere(glm::vec4& intersection, glm::mat4& ctm){
    glm::vec3 result;
    result = glm::vec3(2*intersection.x, 2*intersection.y, 2*intersection.z);
    result = glm::normalize(result);
    glm::vec4 new_normal(result.x, result.y, result.z, 0);
    glm::vec4 world_normal_temp = glm::inverse(glm::transpose(ctm)) * new_normal;
    glm::vec3 world_normal(world_normal_temp.x, world_normal_temp.y, world_normal_temp.z);
    return glm::normalize(world_normal);
}
