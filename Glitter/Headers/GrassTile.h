#ifndef _GRASS_TILE_H
#define _GRASS_TILE_H
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <vector>
struct GrassTransForm{
    glm::vec3 trans;
    float rotate;
    glm::mat4 model;
    GrassTransForm(glm::vec3 t, float r){
        trans = t;
        rotate = r;
        model = glm::mat4(1.f);
        model = glm::translate(model, t);
        model = glm::rotate(model, r, glm::vec3(0.f,1.f,0.f));

    }
};
struct GrassTile{
    glm::vec3 center;
    std::vector<glm::vec3> corners;
    std::vector<GrassTransForm> positions;
};
#endif
