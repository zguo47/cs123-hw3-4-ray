#include "raytracer.h"
#include "QtGui/qimage.h"
#include "raytracescene.h"
#include "intersect.h"
#include "illuminate.h"
#include "utils/rgba.h"
#include <iostream>

/**
 * @brief Stores the image specified from the input file in this class's
 * `std::vector<RGBA> m_image`.
 * @param file: file path to an image
 * @return True if successfully loads image, False otherwise.
 */
RGBA* RayTracer::loadTextureFromFile(const QString &file) {
    QImage myTexture;

    int width; int height;
    if (!myTexture.load(file)) {
        std::cout<<"Failed to load in image: " << file.toStdString() << std::endl;
        return nullptr;
    }
    myTexture = myTexture.convertToFormat(QImage::Format_RGBX8888);
    width = myTexture.width();
    height = myTexture.height();

    t_width = width;
    t_height = height;

    RGBA* texture = new RGBA[width*height];
    QByteArray arr = QByteArray::fromRawData((const char*) myTexture.bits(), myTexture.sizeInBytes());

    for (int i = 0; i < arr.size() / 4.f; i++){
        texture[i] = RGBA{(std::uint8_t) arr[4*i], (std::uint8_t) arr[4*i+1], (std::uint8_t) arr[4*i+2], (std::uint8_t) arr[4*i+3]};
    }

    return texture;
}

RayTracer::RayTracer(Config config) :
    m_config(config)
{}

void RayTracer::render(RGBA *imageData, const RayTraceScene &scene) {
    // Note that we're passing `data` as a pointer (to its first element)
    // Recall from Lab 1 that you can access its elements like this: `data[i]`

    // TODO: Implement the ray tracing algorithm. Good luck!

    int height = scene.height();
    int width = scene.width();
    int depth = 0;
    camera = scene.getCamera();
    metaData = scene.getMetaData();
    cameradata = metaData.cameraData;
    globalData = metaData.globalData;
    primiTypes = metaData.shapes;
    lights = metaData.lights;
    viewMatrix = camera.getViewMatrix(cameradata);
    Intersect intersect;
    Illuminate illuminate;
    for (int s = 0; s < primiTypes.size(); s++){
        if (primiTypes[s].primitive.material.textureMap.isUsed){
            QString filename = QString::fromStdString(primiTypes[s].primitive.material.textureMap.filename);
            RGBA* texture_map = loadTextureFromFile(filename);
            textures[primiTypes[s].primitive.type] = texture_map;
            t_widths[primiTypes[s].primitive.type] = t_width;
            t_heights[primiTypes[s].primitive.type] = t_height;
        }
    }
    for (int j = 0; j < height; j++){
        for (int i = 0; i < width; i++){
            float k = 1.0;
            float x = (i+0.5)/width - 0.5;
            float y = (height - 1 - j +0.5)/height -0.5;
            float V = 2 * k * tan(camera.getHeightAngle(cameradata)/2.0);
            float U = camera.getAspectRatio(width, height) * V;
            glm::vec4 uvk = glm::vec4(U*x, V*y, -k, 1.0);
            glm::vec4 eye = glm::vec4(0.0, 0.0, 0.0, 1.0);
            glm::vec4 d = uvk - eye;

            world_eye =  glm::inverse(viewMatrix) * eye;
            world_d = glm::inverse(viewMatrix) * d;

            imageData[j * width + i] = illuminate.toRGBA(rayTracer(world_eye, world_d, primiTypes, depth));

    }

}
}

glm::vec4 RayTracer::rayTracer(glm::vec4 world_eye, glm::vec4 world_d, std::vector<RenderShapeData> primiTypes, int depth){

    glm::vec4 color;

    Intersect intersect;
    Illuminate illuminate;

    float tMin = std::numeric_limits<float>::max();
    glm::vec4 texture(0.0, 0.0, 0.0, 0.0);
    float epsilon = 0.001f;
    glm::vec4 real_ray;
    glm::vec3 world_normal;
    SceneMaterial currsceneMaterial;
    glm::vec3 directionToCamera;
    bool inter_success = false;
    float t;
    glm::vec4 ray;
    for (int s = 0; s < primiTypes.size(); s++){

    RenderShapeData curr_shape = primiTypes[s];
    RGBA* texture_map = textures[curr_shape.primitive.type];
    int w = t_widths[curr_shape.primitive.type];
    int h = t_heights[curr_shape.primitive.type];
    glm::mat4 ctm = curr_shape.ctm;
    SceneMaterial sceneMaterial = curr_shape.primitive.material;
    glm::vec4 object_eye = glm::inverse(ctm) * world_eye;
    glm::vec4 object_d = glm::inverse(ctm) * world_d;


    directionToCamera = glm::vec3(-world_d.x, -world_d.y, -world_d.z);

    switch(curr_shape.primitive.type){
    case PrimitiveType::PRIMITIVE_CUBE:{
            bool success = intersect.intersect_cube(object_eye, object_d, t, ray);
            if (success){
                inter_success = true;
                if (t < tMin){
                    tMin = t;
                    real_ray = ray;
                    currsceneMaterial = sceneMaterial;
                    world_normal = intersect.normal_cube(ray, ctm);
                    if (curr_shape.primitive.material.textureMap.isUsed){
                        int repeatedU = curr_shape.primitive.material.textureMap.repeatU;
                        int repeatedV = curr_shape.primitive.material.textureMap.repeatV;
                        texture = illuminate.uv_cube(ray, texture_map, w, h, repeatedU, repeatedV);
                    }
                }
            }
            break;
    }
    case PrimitiveType::PRIMITIVE_CONE:{
            bool success = intersect.intersect_cone(object_eye, object_d, t, ray);
            if (success){
                inter_success = true;
                if (t < tMin){
                    tMin = t;
                    real_ray = ray;
                    currsceneMaterial = sceneMaterial;
                    world_normal = intersect.normal_cone(ray, ctm);
                    if (curr_shape.primitive.material.textureMap.isUsed){
                        int repeatedU = curr_shape.primitive.material.textureMap.repeatU;
                        int repeatedV = curr_shape.primitive.material.textureMap.repeatV;
                        texture = illuminate.uv_cone(ray, texture_map, w, h, repeatedU, repeatedV);
                    }
                }
            }
            break;
    }
    case PrimitiveType::PRIMITIVE_CYLINDER:{
            bool success = intersect.intersect_cylinder(object_eye, object_d, t, ray);
            if (success){
                inter_success = true;
                if (t < tMin){
                    tMin = t;
                    real_ray = ray;
                    currsceneMaterial = sceneMaterial;
                    world_normal = intersect.normal_cylinder(ray, ctm);
                    if (curr_shape.primitive.material.textureMap.isUsed){
                        int repeatedU = curr_shape.primitive.material.textureMap.repeatU;
                        int repeatedV = curr_shape.primitive.material.textureMap.repeatV;
                        texture = illuminate.uv_cylinder(ray, texture_map, w, h, repeatedU, repeatedV);
                    }
                }
            }
            break;
    }
    case PrimitiveType::PRIMITIVE_SPHERE:{
            bool success = intersect.intersect_sphere(object_eye, object_d, t, ray);
            if (success){
                inter_success = true;
                if (t < tMin){
                    tMin = t;
                    real_ray = ray;
                    currsceneMaterial = sceneMaterial;
                    world_normal = intersect.normal_sphere(ray, ctm);
                    if (curr_shape.primitive.material.textureMap.isUsed){
                        int repeatedU = curr_shape.primitive.material.textureMap.repeatU;
                        int repeatedV = curr_shape.primitive.material.textureMap.repeatV;
                        texture = illuminate.uv_sphere(ray, texture_map, w, h, repeatedU, repeatedV);
                    }
                }
            }
            break;
    }
    default:
            break;

    }
}

    if (inter_success){
        glm::vec4 intersect_position = world_eye + tMin * world_d;
        color = illuminate.phong(intersect_position, primiTypes, world_normal, directionToCamera, currsceneMaterial, texture, lights, globalData);
        glm::vec4 reflectiveness = currsceneMaterial.cReflective;
        if ((reflectiveness.x > 0.0f || reflectiveness.y > 0.0f || reflectiveness.z > 0.0f) && depth <= 4){
            glm::vec3 coming_ray = glm::vec3(world_d.x, world_d.y, world_d.z);
            world_eye = intersect_position + epsilon * glm::vec4(world_normal, 0.0f);
            world_d = glm::vec4(glm::reflect(coming_ray, world_normal), 0.0);
            color += globalData.ks * rayTracer(world_eye, world_d, primiTypes, depth + 1);
        }

        return color;
    }
    return glm::vec4(0.0, 0.0, 0.0, 255.0);

}


