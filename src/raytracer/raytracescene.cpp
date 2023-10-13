#include <stdexcept>
#include "raytracescene.h"
#include "utils/sceneparser.h"

RayTraceScene::RayTraceScene(int width, int height, const RenderData &metaData) {
    // Optional TODO: implement this. Store whatever you feel is necessary.
}

const int& RayTraceScene::width() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}

const int& RayTraceScene::height() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}

const SceneGlobalData& RayTraceScene::getGlobalData() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}

const Camera& RayTraceScene::getCamera() const {
    // Optional TODO: implement the getter or make your own design
    throw std::runtime_error("not implemented");
}
