#include <stdexcept>
#include "raytracescene.h"
#include "utils/sceneparser.h"

RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData) {
    // Optional TODO: implement this. Store whatever you feel is necessary.
    t_width = width;
    t_height = height;
    t_metaData = metaData;
}

const int& RayTraceScene::width() const {
    // Optional TODO: implement the getter or make your own design
    return t_width;
}

const int& RayTraceScene::height() const {
    // Optional TODO: implement the getter or make your own design
    return t_height;
}

const SceneGlobalData& RayTraceScene::getGlobalData() const {
    // Optional TODO: implement the getter or make your own design
    return t_metaData.globalData;
}

const Camera& RayTraceScene::getCamera() const {
    // Optional TODO: implement the getter or make your own design
    return camera;
}

const RenderData& RayTraceScene::getMetaData() const {
    return t_metaData;
}

const glm::vec3& RayTraceScene::getNormal() const {
    return normal;
}
