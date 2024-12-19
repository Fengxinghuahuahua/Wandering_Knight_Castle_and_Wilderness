#include "GrassTile.h"
#include "camera.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include <Grass.h>
//#include <GrassGenerator.h>
#include <GrassGen.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
Grass::Grass(){
	loadVertices(); 
    glGenVertexArrays(1, &_high_vao);
    glGenBuffers(1, &_high_vbo);
    glBindVertexArray(_high_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _high_vbo);
    glBufferData(GL_ARRAY_BUFFER, _high_num_vertices*sizeof(vex), _high_vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(7*sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(10*sizeof(float)));
    glEnableVertexAttribArray(4);

    glGenVertexArrays(1, &_low_vao);
    glGenBuffers(1, &_low_vbo);
    glBindVertexArray(_low_vao);

    glBindBuffer(GL_ARRAY_BUFFER, _low_vbo);
    glBufferData(GL_ARRAY_BUFFER, _low_num_vertices*sizeof(vex), _low_vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(7*sizeof(float)));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(10*sizeof(float)));
    glEnableVertexAttribArray(4);

    glGenBuffers(1, &_instance_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, _instance_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * 30000 , nullptr, GL_STATIC_DRAW);
}
bool Grass::isInFrontOfPlane(const Plane& plane, const glm::vec3& pos){
    float result = plane.getSignedDistanceToPlane(pos);
    return result >= 0;

}
bool Grass::isInFrustum(const Frustum& frustum, const GrassTile& tile){
    for(auto &each:tile.corners){
        if(isInFrustum(frustum, each)){
            return true;
        }
    }
    return false;
}
bool Grass::isInFrustum(const Frustum& frustum, const glm::vec3 &l){
    //return !(!isInFrontOfPlane(frustum._LeftFace, l)
    //    &&isInFrontOfPlane(frustum._RightFace, l))
    //    &&isInFrontOfPlane(frustum._FarFace, l)
    //    &&isInFrontOfPlane(frustum._NearFace, l);
    return isInFrontOfPlane(frustum._LeftFace, l)
        &&isInFrontOfPlane(frustum._RightFace, l)
        &&isInFrontOfPlane(frustum._FarFace, l)
        &&isInFrontOfPlane(frustum._NearFace, l);
}
void Grass::render(Shader* shaders,Camera* camera, Frustum& frustum, const glm::vec3& pos, float rotate, float time){
        if(!isInFrustum(frustum,pos)){
            return;
        }
		shaders->use();
		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model,pos);
        model = glm::rotate(model, rotate, glm::vec3(0.f,1.f,0.f));
		shaders->setMat4("model", model);
		shaders->setFloat("time", time);
		glBindVertexArray(_low_vao);
		glDrawArrays(GL_TRIANGLES, 0, _low_num_vertices);
		//glBindVertexArray(_high_vao);
		//glDrawArrays(GL_TRIANGLES, 0, _high_num_vertices);
}

void Grass::loadVertices(){
	std::vector<vex> raw_vertices;
	GrassGenerator g;
	g.GetVex(raw_vertices,1,48);
	_high_num_vertices = raw_vertices.size();
	int vex_size = raw_vertices.size()*sizeof(vex)/4;
	_high_vertices = new float[vex_size];
	for(int i = 0;i<raw_vertices.size();i+=1){
		int j = 13*i;
		_high_vertices[j] = raw_vertices[i].position.x;
		_high_vertices[j+1] = raw_vertices[i].position.y;
		_high_vertices[j+2] = raw_vertices[i].position.z;
		_high_vertices[j+3] = raw_vertices[i].color.x;
		_high_vertices[j+4] = raw_vertices[i].color.y;
		_high_vertices[j+5] = raw_vertices[i].color.z;
		_high_vertices[j+6] = raw_vertices[i].height;
		_high_vertices[j+7] = raw_vertices[i].center.x;
		_high_vertices[j+8] = raw_vertices[i].center.y;
		_high_vertices[j+9] = raw_vertices[i].center.z;
        _high_vertices[j+10] = raw_vertices[i].normal.x;
        _high_vertices[j+11] = raw_vertices[i].normal.y;
        _high_vertices[j+12] = raw_vertices[i].normal.z;
	}
    raw_vertices.clear();
    raw_vertices.shrink_to_fit();
    g.GetVex(raw_vertices,2,16);

    _low_num_vertices = raw_vertices.size();
	vex_size = raw_vertices.size()*sizeof(vex)/4;
    _low_vertices = new float[vex_size]; 
	for(int i = 0;i<raw_vertices.size();i+=1){
		int j = 13*i;
		_low_vertices[j] = raw_vertices[i].position.x;
		_low_vertices[j+1] = raw_vertices[i].position.y;
		_low_vertices[j+2] = raw_vertices[i].position.z;
		_low_vertices[j+3] = raw_vertices[i].color.x;
		_low_vertices[j+4] = raw_vertices[i].color.y;
		_low_vertices[j+5] = raw_vertices[i].color.z;
		_low_vertices[j+6] = raw_vertices[i].height;
		_low_vertices[j+7] = raw_vertices[i].center.x;
		_low_vertices[j+8] = raw_vertices[i].center.y;
		_low_vertices[j+9] = raw_vertices[i].center.z;
        _low_vertices[j+10] = raw_vertices[i].normal.x;
        _low_vertices[j+11] = raw_vertices[i].normal.y;
        _low_vertices[j+12] = raw_vertices[i].normal.z;
	}
}

void Grass::instanceRender(Shader* shaders, std::vector<GrassTile>& tiles, Frustum& frustum,float time){
    shaders->use();
    shaders->setFloat("time", time);
    shaders->setInt("isInstance", 1);

    highLODModel.clear();
    lowLODModel.clear();
    for (auto& tile : tiles) {
        //if (!isInFrustum(frustum, tile.center)) {
        //    continue;
        //}
        if(!isInFrustum(frustum, tile)){
            continue;
        }
        float d = frustum._NearFace.getSignedDistanceToPlane(tile.center);
        for (auto& grass : tile.positions) {
            if(d>10.f){
                lowLODModel.push_back(grass.model);
            }
            else
                highLODModel.push_back(grass.model);
        }
    }

    //std::cout<<highLODModel.size()<<" "<<lowLODModel.size()<<std::endl;

    if (highLODModel.size() > 30000) { 
        glBindBuffer(GL_ARRAY_BUFFER, _instance_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * highLODModel.size(), &highLODModel[0], GL_STATIC_DRAW);
    }
    else if (!highLODModel.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, _instance_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * highLODModel.size(), &highLODModel[0]);
    }

    glBindVertexArray(_high_vao); 
    for (GLuint i = 0; i < 4; ++i) {
        glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(5 + i);
        glVertexAttribDivisor(5 + i, 1);
    }
    glBindVertexArray(_high_vao);  
    glDrawArraysInstanced(GL_TRIANGLES, 0, _high_num_vertices, highLODModel.size());

    if (lowLODModel.size() > 30000) { 
        glBindBuffer(GL_ARRAY_BUFFER, _instance_buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4) * lowLODModel.size(), &lowLODModel[0], GL_STATIC_DRAW);
    }
    else if (!lowLODModel.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, _instance_buffer);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(glm::mat4) * lowLODModel.size(), &lowLODModel[0]);
    }
    glBindVertexArray(_low_vao);  
    for (GLuint i = 0; i < 4; ++i) {
        glVertexAttribPointer(5 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glEnableVertexAttribArray(5 + i);
        glVertexAttribDivisor(5 + i, 1);
    }
    glBindVertexArray(_low_vao); 
    glDrawArraysInstanced(GL_TRIANGLES, 0, _low_num_vertices, lowLODModel.size());

}
void Grass::render(Shader* shaders, std::vector<GrassTile>& tiles, Frustum& frustum,float time){
    shaders->use();
    shaders->setFloat("time", time);
    shaders->setInt("isInstance", 0);
    for(auto& tile:tiles){
        if(!isInFrustum(frustum, tile.center)){
            continue;
        }
        float d = frustum._NearFace.getSignedDistanceToPlane(tile.center);
        for(auto& grass:tile.positions){
		    glm::mat4 model = glm::mat4(1.f);
		    model = glm::translate(model,grass.trans);
            model = glm::rotate(model, grass.rotate, glm::vec3(0.f,1.f,0.f));
		    shaders->setMat4("model", model);
            if(d>10.f){
                glBindVertexArray(_low_vao);
                glDrawArrays(GL_TRIANGLES, 0, _low_num_vertices);
            }else{
                glBindVertexArray(_high_vao);
                glDrawArrays(GL_TRIANGLES, 0, _high_num_vertices);

            }
        }
    }
}

