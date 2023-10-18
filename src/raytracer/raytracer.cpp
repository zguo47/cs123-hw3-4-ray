#include "raytracer.h"
#include "raytracescene.h"
#include "intersect.h"
#include <iostream>


RayTracer::RayTracer(Config config) :
    m_config(config)
{}

void RayTracer::render(RGBA *imageData, const RayTraceScene &scene) {
    // Note that we're passing `data` as a pointer (to its first element)
    // Recall from Lab 1 that you can access its elements like this: `data[i]`

    // TODO: Implement the ray tracing algorithm. Good luck!

    int height = scene.height();
    int width = scene.width();
    Camera camera = scene.getCamera();
    RenderData metaData = scene.getMetaData();
    SceneCameraData cameradata = metaData.cameraData;
    SceneGlobalData globalData = metaData.globalData;
    std::vector<RenderShapeData> primiTypes = metaData.shapes;
    std::vector<SceneLightData> lights = metaData.lights;
    glm::mat4 viewMatrix = camera.getViewMatrix(cameradata);
    for (int j = 0; j < height; j++){
        for (int i = 0; i < width; i++){
            float k = 1.0;
            float x = (i+0.5)/width - 0.5;
            float y = (height - 1 - j +0.5)/height -0.5;
            float U = 2 * k * tan((camera.getHeightAngle(cameradata)*camera.getAspectRatio(width, height))/2.0);
            float V = 2 * k * tan(camera.getHeightAngle(cameradata)/2.0);
            glm::vec4 uvk = glm::vec4(U*x, V*y, -k, 1.0);
            glm::vec4 eye = glm::vec4(0.0, 0.0, 0.0, 1.0);
            glm::vec4 d = uvk - eye;

            float tMin = std::numeric_limits<float>::max();
            glm::vec3 world_normal;
            Intersect intersect;
            SceneMaterial currsceneMaterial;
            glm::vec3 directionToCamera;
            bool inter_success = false;
            for (int s = 0; s < primiTypes.size(); s++){

                RenderShapeData curr_shape = primiTypes[s];
                glm::mat4 ctm = curr_shape.ctm;
                SceneMaterial sceneMaterial = curr_shape.primitive.material;
                glm::vec4 world_eye =  glm::inverse(viewMatrix) * eye;
                glm::vec4 object_eye = glm::inverse(ctm) * world_eye;

                glm::vec4 world_d = glm::inverse(viewMatrix) * d;
                glm::vec4 object_d = glm::inverse(ctm) * world_d;

                float t;
                glm::vec4 ray;
                directionToCamera = glm::vec3(-world_d.x, -world_d.y, -world_d.z);

                switch(curr_shape.primitive.type){
                case PrimitiveType::PRIMITIVE_CUBE:{
                    bool success = intersect.intersect_cube(object_eye, object_d, t, ray);
                    if (success){
                        inter_success = true;
                        if (t < tMin){
                            tMin = t;
                            currsceneMaterial = sceneMaterial;
                            world_normal = intersect.normal_cube(ray, ctm);
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
                            currsceneMaterial = sceneMaterial;
                            world_normal = intersect.normal_cone(ray, ctm);
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
                            currsceneMaterial = sceneMaterial;
                            world_normal = intersect.normal_cylinder(ray, ctm);
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
                            currsceneMaterial = sceneMaterial;
                            world_normal = intersect.normal_sphere(ray, ctm);
                        }
                    }
                    break;
                }
                default:
                    break;

                }
            }

            if (inter_success){
                imageData[j * width + i] = intersect.phong(world_normal, directionToCamera, currsceneMaterial, lights, globalData);
            }

    }

}
}

