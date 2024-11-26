#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include <Grass.h>
//#include <GrassGenerator.h>
#include <GrassGen.h>
#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
Grass::Grass(){
	loadVertices(); 
    glGenVertexArrays(1, &_VAO);
    glGenBuffers(1, &_VBO);
    glBindVertexArray(_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, _VBO);
    glBufferData(GL_ARRAY_BUFFER, _NumOfVertices*sizeof(vex), _Vertices, GL_STATIC_DRAW);
    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(vex), (void*)(7*sizeof(float)));
    glEnableVertexAttribArray(3);
}
void Grass::render(Shader* shaders,const glm::vec3& pos, float time){
		shaders->use();
		glm::mat4 model = glm::mat4(1.f);
		model = glm::translate(model,pos);
		shaders->setMat4("model", model);
		shaders->setFloat("time", time);
		glBindVertexArray(_VAO);
		glDrawArrays(GL_TRIANGLES, 0, _NumOfVertices);
}

void Grass::loadVertices(){
	std::vector<vex> raw_vertices;
	GrassGenerator g;
	g.GetVex(raw_vertices);
	//GetVex(raw_vertices);
	_NumOfVertices = raw_vertices.size();
	int vex_size = raw_vertices.size()*sizeof(vex)/4;
	_Vertices = new float[vex_size];
	for(int i = 0;i<raw_vertices.size();i+=1){
		int j = 10*i;
		_Vertices[j] = raw_vertices[i].position.x;
		_Vertices[j+1] = raw_vertices[i].position.y;
		_Vertices[j+2] = raw_vertices[i].position.z;
		_Vertices[j+3] = raw_vertices[i].color.x;
		_Vertices[j+4] = raw_vertices[i].color.y;
		_Vertices[j+5] = raw_vertices[i].color.z;
		_Vertices[j+6] = raw_vertices[i].height;
		_Vertices[j+7] = raw_vertices[i].center.x;
		_Vertices[j+8] = raw_vertices[i].center.y;
		_Vertices[j+9] = raw_vertices[i].center.z;
	}
}
