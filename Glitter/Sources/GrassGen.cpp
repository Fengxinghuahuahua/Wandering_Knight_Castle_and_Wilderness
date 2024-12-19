#include <GrassGen.h>
#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cstdlib>
#include <stdlib.h>
#include <iostream>
#include <vector>

float GrassGenerator::rand(glm::vec2 co){
    return glm::fract(glm::sin(glm::dot(co ,glm::vec2(12.9898f,78.233f))) * 43758.5453f);
}
float GrassGenerator::rand(glm::vec2 co,float mini,float maxi){
	float random = rand(co);
	return random*(maxi-mini)+mini;
}


glm::vec4 GrassGenerator::getRandomPosition(glm::vec2 seed1, glm::vec2 seed2){
	glm::vec4 newPos = glm::vec4(0.f, 0.f, 0.f, 1.f);

    float epsilon = 1e-4;
    
    float randX = 0.f;
    float factor = 1.f;
    while(randX < epsilon){
        randX = rand(seed1*factor);
        factor += 1.f;
    }
    float randZ = 0.f;
    factor = 1.f;
    while(randZ < epsilon){
        randZ = rand(seed2*factor);
        factor += 1.f;
    }

    newPos.x = randX * float(_width);
    newPos.z = randZ * float(_height);

	glm::vec4 curTilePos = glm::vec4(_tile_pos.x, 0.f, _tile_pos.y, 1.f);

    return newPos + curTilePos;
}



glm::vec2 GrassGenerator::getBend(glm::vec2 seed1, glm::vec2 seed2, float height, float tilt){
    float maxX = tilt;
    float minX = tilt / 3.f;
    float randX = rand(seed1, minX, maxX);

    float maxY = height;
    float minY = (height / tilt) * randX + height / 3.f;
    float randY = rand(seed2, minY, maxY);

    return glm::vec2(randX, randY);
}

glm::vec4 GrassGenerator::getColor(glm::uint clumpId){
    float green = rand(glm::vec2(clumpId, clumpId), MIN_GREEN, MAX_GREEN);

    return glm::vec4(
        0.05f,
        0.2f * green,
        0.01f,
        1.f
    );
}

float GrassGenerator::getRotation(glm::uint id){
    return glm::radians(rand(glm::vec2(id*_tile_id, id+_tile_id)) * 360.f);
}

float GrassGenerator::getTilt(glm::uint id, float height){
    return rand(glm::vec2(id, 0.f), height / 2.f, height);
}
glm::mat3 GrassGenerator::getRotationMatrix(float rotation){
    return glm::mat3(glm::cos(rotation), 0.f, glm::sin(rotation),
                0.f, 1.f, 0.f,
                -glm::sin(rotation), 0.f, glm::cos(rotation));
}


glm::vec2 GrassGenerator::quadraticBezierCurve(float t, glm::vec2 P0, glm::vec2 P1, glm::vec2 P2){
    return (1.f-t)*(1.f-t)*P0 + 2.f*(1.f-t)*t*P1 + t*t*P2;
}

glm::vec2 GrassGenerator::quadraticBezierCurveDerivative(float t, glm::vec2 P0, glm::vec2 P1, glm::vec2 P2){
    return -2.f*(1.f-t)*P0 + 2.f*P1*(1.f-2.f*t) + 2.f*t*P2;
}

glm::vec3 GrassGenerator::getColor(glm::vec3 color, float maxHeight, float curHeight){
    return mix(color, TIP_COLOR, (curHeight / maxHeight));
}

void GrassGenerator::getVerticesPositionsAndNormals(glm::vec3 pos, float width, float height, float tilt, glm::vec2 bend, glm::vec3 color,
     glm::vec3 positions[NB_VERT_HIGH_LOD],
     glm::vec3 normals[NB_VERT_HIGH_LOD],
     glm::vec3 colors[NB_VERT_HIGH_LOD]
    ){
    int nbVert = _lod == HIGH_LOD ? NB_VERT_HIGH_LOD : NB_VERT_LOW_LOD;

    float minWidth = width / 5.f;
    float widthDelta = minWidth / nbVert;
    float curWidth = width / 2.f;

	glm::vec2 P0 = glm::vec2(0.f);
	glm::vec2 P1 = bend;
	glm::vec2 P2 = glm::vec2(tilt, height);

	glm::vec3 widthTangent = glm::vec3(0.f, 0.f, 1.f);

    for(int i=0; i<nbVert-1; i+=2){
        float t = i / (1.f * nbVert);
		glm::vec2 bendAndTilt = quadraticBezierCurve(t, P0, P1, P2);
        positions[i] = pos + glm::vec3(bendAndTilt.x, bendAndTilt.y, -curWidth);
        positions[i+1] = pos + glm::vec3(bendAndTilt.x, bendAndTilt.y, curWidth);
        colors[i] = getColor(color, height, bendAndTilt.y);
        colors[i+1] = getColor(color, height, bendAndTilt.y);
        curWidth -= widthDelta;

		glm::vec2 bezierDerivative = quadraticBezierCurveDerivative(t, P0, P1, P2);
		glm::vec3 bezierNormal = normalize(glm::vec3(bezierDerivative.x, bezierDerivative.y, 0.f));
		glm::vec3 normal = cross(bezierNormal, widthTangent);
        // rotate the normals a bit
        // normals[i] = normal;
        // normals[i+1] = normal;
        normals[i] = normalize(getRotationMatrix(PI * 0.3f) * normal);
        normals[i+1] = normalize(getRotationMatrix(PI * (-0.3f)) * normal);
    }

    positions[nbVert-1] = pos + glm::vec3(P2, 0.f);
	glm::vec2 bezierDerivative = quadraticBezierCurveDerivative(1.f, P0, P1, P2);
	glm::vec3 bezierNormal = normalize(glm::vec3(bezierDerivative.x, bezierDerivative.y, 0.f));
    normals[nbVert-1] = cross(bezierNormal, widthTangent);
    colors[nbVert-1] = TIP_COLOR;
}

glm::vec3 GrassGenerator::getModelPos(glm::vec3 center, glm::vec3 position, float rotation){
    return getRotationMatrix(rotation) * (position - center) + center;
}


glm::vec4 GrassGenerator::getWorldPos(glm::vec3 center, glm::vec3 position, float rotation){
	glm::vec3 curPosition = getModelPos(center, position, rotation);
	glm::vec4 worldPosition = glm::vec4(curPosition, 1.f);
    return worldPosition;
}

glm::vec4 GrassGenerator::getWorldPos(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], int vertex, float rotation){
    return getWorldPos(center, positions[vertex], rotation);
}

glm::vec3 GrassGenerator::getRotatedNormals(glm::vec3 normal, float rotation){
    return normalize(getRotationMatrix(rotation) * normal);
}

void GrassGenerator::createTriangle(glm::vec3 center, 
    glm::vec3 positions[NB_VERT_HIGH_LOD], 
    glm::vec3 normals[NB_VERT_HIGH_LOD], 
    glm::vec3 colors[NB_VERT_HIGH_LOD],
    float rotation, float height,glm::vec3 color, int indices[3]){
    int idx = 0;
    for(int i=0; i<3; i++){
        idx = indices[i];
		glm::vec4 worldPos = getWorldPos(center, positions, idx, rotation);
		glm::vec3 normal = getRotatedNormals(normals[idx], rotation);
	//	std::cout<<worldPos.x<<" "<<worldPos.y<<" "<<worldPos.z<<std::endl;
		vex o;
		o.position = worldPos;
		o.color = colors[idx];
		o.height = height;
		o.center = center;
        o.normal = normal;
        _positions.push_back(o); ;
		
    }
}
glm::vec3 GrassGenerator::getAvgNormal(float tilt, float height){
	glm::vec3 P0 = glm::vec3(0.f);
	glm::vec3 P1 = glm::vec3(tilt, height, 0.f);
	glm::vec3 widthTangent = glm::vec3(0.f, 0.f, 1.f);
	glm::vec3 heightTangent = normalize(P1-P0);
    return cross(heightTangent, widthTangent);
}


void GrassGenerator::createTriangles(glm::vec3 center, float rotation, float height,glm::vec3 color,
    glm::vec3 positions[NB_VERT_HIGH_LOD], 
    glm::vec3 normals[NB_VERT_HIGH_LOD],
    glm::vec3 colors[NB_VERT_HIGH_LOD]
    ){
    int nbQuads = _lod == HIGH_LOD ? NB_QUAD_HIGH_LOD : NB_QUAD_LOW_LOD;
    int nbVert = _lod == HIGH_LOD ? NB_VERT_HIGH_LOD : NB_VERT_LOW_LOD;
    int vertCounter = 0;
    int indices[3];
    // draw the rectangles
    for(int i=0; i<nbQuads; i++){
        // first triangle
        indices[0] = vertCounter;
        indices[1] = vertCounter+1;
        indices[2] = vertCounter+2;
        createTriangle(center, positions, normals, colors, rotation,height, color, indices);

        // second triangle
        indices[0] = vertCounter+2;
        indices[1] = vertCounter+1;
        indices[2] = vertCounter+3;
        createTriangle(center, positions, normals, colors, rotation,height, color, indices);

        vertCounter += 2;
    }
    // last triangle
    indices[0] = nbVert-3;
    indices[1] = nbVert-2;
    indices[2] = nbVert-1;
    createTriangle(center, positions, normals, colors, rotation,height, color, indices);
}

void GrassGenerator::GetVex(std::vector<vex>&oArray,int LOD,int numOfBlades){
    _positions.clear();
	_width = 0.1;
	_height = 0.1;
	_tile_pos = glm::vec2(0.f,0.f);
	_tile_id = 1;
    _lod = LOD;
    int num = numOfBlades;
	for(int i = 0;i<num;i+=1){
		int instanceIndex = i;

		glm::vec4 position = getRandomPosition(glm::vec2(instanceIndex, _tile_id), glm::vec2(_tile_id, instanceIndex));
		//glm::uint clumpId = getClumpId(glm::vec3(position.x,position.y,position.z));
		glm::uint clumpId = 0;
    	float height = rand({position.x,position.z}, MIN_HEIGHT, MAX_HEIGHT);
    	float width = rand({position.x,position.z}, MIN_WIDTH, MAX_WIDTH);
		glm::vec4 color = getColor(clumpId);
    	float rotation = getRotation(instanceIndex);
    	float tilt = getTilt(clumpId, height);
		glm::vec2 bend = getBend(glm::vec2(clumpId, position.x), glm::vec2(position.z, clumpId), height, tilt);

		glm::vec3 pos = position;

		glm::vec3 positions[NB_VERT_HIGH_LOD];
		glm::vec3 normals[NB_VERT_HIGH_LOD];
		glm::vec3 colors[NB_VERT_HIGH_LOD];

		getVerticesPositionsAndNormals(pos, width, height, tilt, bend, color, positions, normals, colors);
		createTriangles(pos, rotation,height, color, positions, normals, colors);

	}
	oArray.assign(_positions.begin(), _positions.end());
}

