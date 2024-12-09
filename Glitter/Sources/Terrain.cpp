#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/fwd.hpp"
#include "shader_m.h"
#include <iterator>
#include <queue>
#include <string>
#include <wctype.h>
#include <corecrt.h>
#include <cstdio>
#include <vector>
#include <Terrain.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <camera.h>


void Terrain::loadHeightMap(std::string path,std::string texture_path,std::string normal_path,std::string displace_path){
	int nrChannels;
	int textureWidth,textureHeight;
	int normalWidth,normalHeight;
	int displaceWidth,displaceHeight;
	_heightMapData  = stbi_load(path.c_str(), &_width, &_height, &nrChannels, 0);
	unsigned char *texture_data = stbi_load(texture_path.c_str(), &textureWidth, &textureHeight, &nrChannels, 0);
	unsigned char *normal_data = stbi_load(normal_path.c_str(), &normalWidth, &normalHeight, &nrChannels, 0);
	unsigned char *displace_data = stbi_load(displace_path.c_str(), &displaceWidth, &displaceHeight, &nrChannels, 0);
	std::cout<<nrChannels<<std::endl;
	if(texture_data){
		glGenTextures(1, &_terrainTestureID);
		glBindTexture(GL_TEXTURE_2D, _terrainTestureID);
        // 3. 生成纹理
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureWidth, textureHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);  // 水平重复
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  // 垂直重复
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);  // 使用 Mipmap
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);  // 放大时线性过滤
	}else{
		std::cout<<"Load Texture failed..."<<std::endl;
	}
	
		
	if(normal_data){
		glGenTextures(1, &_terrainNormalID);
		glBindTexture(GL_TEXTURE_2D, _terrainNormalID);
		// 设置纹理参数
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, normalWidth, normalHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, normal_data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}else{
		std::cout<<"Load Texture normal failed..."<<std::endl;
	}
	if (displace_data) {
    glGenTextures(1, &_terrainDisplaceTextureID);
    glBindTexture(GL_TEXTURE_2D, _terrainDisplaceTextureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, displaceWidth, displaceHeight, 0, GL_RED, GL_UNSIGNED_BYTE, displace_data);
    glGenerateMipmap(GL_TEXTURE_2D);

	}else{
		std::cout<<"Load Texture displace failed..."<<std::endl;
	}
    if (_heightMapData) {
		generateTerrainVertices(_terrainVertices, _heightMapData);
		generateTerrainIndices(_indices);
		calculateTerrainNormals(_terrainVertices, _indices);
		calculateTangents(_terrainVertices, _indices);
	    // 创建 VAO 和 VBO
	    glGenVertexArrays(1, &_vao);
	    glGenBuffers(1, &_vbo);
	    glBindVertexArray(_vao);
	
	    // 将顶点数据上传到 GPU
	    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
	    glBufferData(GL_ARRAY_BUFFER, _terrainVertices.size() * sizeof(float), _terrainVertices.data(), GL_STATIC_DRAW);
		unsigned int EBO;
		// 创建 EBO (索引缓冲对象)
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, _indices.size() * sizeof(unsigned int), _indices.data(), GL_STATIC_DRAW);

	    // 顶点属性：位置
	    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, _stride * sizeof(float), (void*)0);
	    glEnableVertexAttribArray(0);
	
	    // 法线属性
	    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, _stride * sizeof(float), (void*)(3 * sizeof(float)));
	    glEnableVertexAttribArray(1);

	    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, _stride * sizeof(float), (void*)(6 * sizeof(float)));
	    glEnableVertexAttribArray(2);

	    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, _stride * sizeof(float), (void*)(8 * sizeof(float)));
	    glEnableVertexAttribArray(3);

	    glBindVertexArray(0);
		
		std::cout<<"Load HeightMap success"<<"Width: "<<_width<<" Height: "<<_height<<std::endl;

    } else {
		std::cout<<"Load HeightMap error"<<std::endl;
    }
}
void Terrain::generateTerrainVertices(std::vector<float>& oVertices,unsigned char* data){
	//getchar();
    for (int z = 0; z < _height; ++z) {
        for (int x = 0; x < _width; ++x) {
            // 获取高度图纹理的像素值
            float heightValue = getHeightFromTexture(x, z,data);
            // 添加顶点数据：x, y (高度), z
            oVertices.push_back(float(x)/10);
            oVertices.push_back(float(heightValue)/10);
            oVertices.push_back(float(z)/10);

            // 添加法线（可以稍后计算，先简单放置）
            oVertices.push_back(0.0f); // 法线的 x
            oVertices.push_back(0.0f); // 法线的 y
            oVertices.push_back(0.0f); // 法线的 z

			oVertices.push_back(float(x)/(_width-1)*10);//u
			oVertices.push_back(float(z)/(_height-1)*10);//v

            oVertices.push_back(0.0f); // 法线的 x
            oVertices.push_back(0.0f); // 法线的 y
            oVertices.push_back(0.0f); // 法线的 z
        }
    }
}
float Terrain::getHeightFromTexture(int x,int z,unsigned char* data){
    return data[z*_width+x]/5.f* 1.0f-20; // 将灰度值转换为实际高度，可能需要调整
}

void Terrain::setUpTerrain() {
}
Terrain::Terrain(){
	loadHeightMap(std::string(PROJECT_SOURCE_DIR)+"/Glitter/Textures/terrain/heightmap.png",std::string(PROJECT_SOURCE_DIR)+"/Glitter/Textures/terrain/colors.jpg",std::string(PROJECT_SOURCE_DIR)+"/Glitter/Textures/terrain/normals.jpg",std::string(PROJECT_SOURCE_DIR)+"/Glitter/Textures/terrain/displace.jpg");
	setUpTerrain();
}
void Terrain::render(Shader* shaders){
	shaders->setInt("isTerrain", 1);

	shaders->setInt("texture_diffuse1", 1);
	shaders->setInt("normalMap",2);
	// 在渲染循环中绘制

	glm::mat4 model = glm::mat4(1.f);

	float terrainX = (float)(-_width)/20.f;
	float terrainY = (float)(-_height)/20.f;

	model = glm::translate(model, glm::vec3(terrainX,0.f,terrainY));
	//model = glm::translate(model,glm::vec3(0.f,0.f,0.f));
	shaders->setMat4("model", model);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,_terrainTestureID);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,_terrainNormalID);

	glBindVertexArray(_vao);
	glDrawElements(GL_TRIANGLES, _indices.size()*3, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Terrain::generateTerrainIndices(std::vector<unsigned int>&oIndices){
	for (int z = 0; z < _height - 1; ++z) {
	    for (int x = 0; x < _width - 1; ++x) {
	        int currentIndex = z * _width + x;  // 当前顶点的索引
	
	        // 三角形 1
	        oIndices.push_back(currentIndex);
	        oIndices.push_back(currentIndex + 1);
	        oIndices.push_back(currentIndex + _width);
	
	        // 三角形 2
	        oIndices.push_back(currentIndex + 1);
	        oIndices.push_back(currentIndex + _width + 1);
	        oIndices.push_back(currentIndex + _width);
	    }
	}
}
std::vector<float>& Terrain::getVertices(){
	return _terrainVertices;
}

void Terrain::calculateTerrainNormals(std::vector<float>&Vertices,std::vector<unsigned int>&Indices){
	for(int i = 0;i<Indices.size();i+=3){
		int idx0 = Indices[i]*_stride;
		int idx1 = Indices[i+1]*_stride;
		int idx2 = Indices[i+2]*_stride;
		glm::vec3 v0 = glm::vec3(Vertices[idx0],Vertices[idx0+1],Vertices[idx0+2]);
		glm::vec3 v1 = glm::vec3(Vertices[idx1],Vertices[idx1+1],Vertices[idx1+2]);
		glm::vec3 v2 = glm::vec3(Vertices[idx2],Vertices[idx2+1],Vertices[idx2+2]);
		glm::vec3 edge1 = v1-v0;
		glm::vec3 edge2 = v2-v0;

		glm::vec3 normal = glm::normalize(glm::cross(edge2, edge1));

		Vertices[idx0+3] += normal.x;
		Vertices[idx0+4] += normal.y;
		Vertices[idx0+5] += normal.z;

		Vertices[idx1+3] += normal.x;
		Vertices[idx1+4] += normal.y;
		Vertices[idx1+5] += normal.z;

		Vertices[idx2+3] += normal.x;
		Vertices[idx2+4] += normal.y;
		Vertices[idx2+5] += normal.z;

	}

}

void Terrain::calculateTangents(std::vector<float>&Vertices,std::vector<unsigned int>&Indices){
	for(int i = 0;i<Indices.size();i+=3){
		int idx0 = Indices[i]*_stride;
		int idx1 = Indices[i+1]*_stride;
		int idx2 = Indices[i+2]*_stride;
		glm::vec3 v0 = glm::vec3(Vertices[idx0],Vertices[idx0+1],Vertices[idx0+2]);
		glm::vec3 v1 = glm::vec3(Vertices[idx1],Vertices[idx1+1],Vertices[idx1+2]);
		glm::vec3 v2 = glm::vec3(Vertices[idx2],Vertices[idx2+1],Vertices[idx2+2]);
		glm::vec2 uv0 = glm::vec2(Vertices[idx0+6],Vertices[idx0+7]);
		glm::vec2 uv1 = glm::vec2(Vertices[idx1+6],Vertices[idx1+7]);
		glm::vec2 uv2 = glm::vec2(Vertices[idx2+6],Vertices[idx2+7]);
		glm::vec3 deltaPos1 = v1-v0;
		glm::vec3 deltaPos2 = v2-v0;
		glm::vec2 deltaUV1 = uv1-uv0;
		glm::vec2 deltaUV2 = uv2-uv0;

        float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
        glm::vec3 tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

		Vertices[idx0+8] = tangent.x;
		Vertices[idx0+9] = tangent.y;
		Vertices[idx0+10] = tangent.z;

		Vertices[idx1+8] = tangent.x;
		Vertices[idx1+9] = tangent.y;
		Vertices[idx1+10] = tangent.z;

		Vertices[idx2+8] = tangent.x;
		Vertices[idx2+9] = tangent.y;
		Vertices[idx2+10] = tangent.z;
	}
}

float Terrain::getHeight(float x,float y){
	int X = int(x*10-_bottom.x*10); 
	int Y = int(y*10-_bottom.y*10);

	float u = (x - _bottom.x) / (_top.x - _bottom.x) * (_width - 1);
	float v = (y - _bottom.y) / (_top.y - _bottom.y) * (_height - 1);
	int u0 = floor(u);
	int v0 = floor(v);
	int u1 = u0 + 1;
	int v1 = v0 + 1;	
	float height00 = getHeightFromTexture(u0, v0, _heightMapData);
	float height01 = getHeightFromTexture(u0, v1, _heightMapData);
	float height10 = getHeightFromTexture(u1, v0, _heightMapData);
	float height11 = getHeightFromTexture(u1, v1, _heightMapData);
	float height = (u1 - u) * (v1 - v) * height00 +
               (u1 - u) * (v - v0) * height01 +
               (u - u0) * (v1 - v) * height10 +
               (u - u0) * (v - v0) * height11;
	return height/10.f;
}
void Terrain::set2Zero(){
	float terrainX = (float)(-_width)/20.f;
	float terrainY = (float)(-_height)/20.f;

	_bottom.x = terrainX;
	_bottom.y = terrainY;

	_top.x = -terrainX;
	_top.y = -terrainY;
}

std::vector<glm::vec3> Terrain::getRandomPositions(){
	std::vector<glm::vec3> result;
	for(int i = 0;i<2000;i+=1){
		int x = rand()%_width;
		int y = rand()%_height;
		result.push_back(glm::vec3(float(x)/10+_bottom.x,getHeightFromTexture(x, y, _heightMapData)/10,(float)(y)/10+_bottom.y));
	}
	return result;
}
