#include <GrassGen.h>
#include "glm/common.hpp"
#include "glm/fwd.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include <cstdlib>
#include <iostream>
#include <vector>

float GrassGenerator::rand(glm::vec2 co){
    return glm::fract(sin(dot(co ,glm::vec2(12.9898f,78.233f))) * 43758.5453f);
}
float GrassGenerator::rand(glm::vec2 co,float mini,float maxi){
	float random = rand(co);
	return random*(maxi-mini)+mini;
}
glm::uint GrassGenerator::getGridCell(glm::vec2 position){
    float cellWidth = (1.f*tileWidth) / gridNbCols;
    float cellHeight = (1.f*tileHeight) / gridNbLines;

	glm::uint cellX = glm::uint(position.x / cellWidth);
	glm::uint cellY = glm::uint(position.y / cellHeight);

    return cellX + cellY * gridNbCols;
}

bool GrassGenerator::cellIsFirstRow(glm::uint cellId){
    return cellId < gridNbCols;
}

bool GrassGenerator::cellIsLastRow(glm::uint cellId){
    return cellId > gridNbCols*(gridNbLines-1);
}

bool GrassGenerator::cellIsFirstCol(glm::uint cellId){
    return (cellId % gridNbLines) == 0;
}

bool GrassGenerator::cellIsLastCol(glm::uint cellId){
    return (cellId % gridNbLines) == (gridNbCols-1);
}

// return the ids of the closest intersections to the blade position and return the number of neighbors to check
glm::uint GrassGenerator::getClosestIntersections(glm::vec3 bladePosition, glm::uint resultIds[9]){
	glm::vec2 positionTmp = glm::vec2(bladePosition.x,bladePosition.z);
	glm::uint cellId = getGridCell(positionTmp);

    // center cell
    resultIds[0] = cellId;
	glm::uint curIdx = 1;
    // up cell
    if(!cellIsFirstRow(cellId)){
        resultIds[curIdx] = cellId - gridNbCols; 
        curIdx++; 
    }
    // down cell
    if(!cellIsLastRow(cellId)){
        resultIds[curIdx] = cellId + gridNbCols; 
        curIdx++; 
    }
    // left cell
    if(!cellIsFirstCol(cellId)){
        resultIds[curIdx] = cellId - 1; 
        curIdx++;
    }
    // right cell
    if(!cellIsLastCol(cellId)){
        resultIds[curIdx] = cellId + 1; 
        curIdx++;
    }
    // up left
    if(!cellIsFirstRow(cellId) && !cellIsFirstCol(cellId)){
        resultIds[curIdx] = cellId - gridNbCols - 1;
        curIdx++;
    }
    // up right
    if(!cellIsFirstRow(cellId) && !cellIsLastCol(cellId)){
        resultIds[curIdx] = cellId - gridNbCols + 1;
        curIdx++;
    }
    // down left
    if(!cellIsLastRow(cellId) && !cellIsFirstCol(cellId)){
        resultIds[curIdx] = cellId + gridNbCols - 1;
        curIdx++;
    }
    // down right
    if(!cellIsLastRow(cellId) && !cellIsLastCol(cellId)){
        resultIds[curIdx] = cellId + gridNbCols + 1;
        curIdx++;
    }

    return curIdx;
}

glm::vec2 GrassGenerator::getIntersectionPosition(glm::uint cellId){
    float randX = rand(glm::vec2(cellId, 0.f));
    float randZ = rand(glm::vec2(0.f, cellId));

    float cellX = cellId % gridNbCols;
    float cellZ = cellId / gridNbLines;

    return glm::vec2(cellX+randX, cellZ+randZ);
}

// return the jittered positions of the closest intersections to the blade position
void GrassGenerator::getVoronoiPositions(glm::uint nbIntersections, glm::uint voronoiCellIds[9], glm::vec2 voronoiPositions[9]){
    for(glm::uint i=0; i<nbIntersections; i++){
        voronoiPositions[i] = getIntersectionPosition(voronoiCellIds[i]);
    }
}

// return the id of the nearest neighbour
glm::uint GrassGenerator::findNearestNeighbour(glm::vec3 bladePosition, glm::uint nbIntersections, glm::vec2 voronoiPositions[9]){
	glm::vec2 positionTmp = glm::vec2(bladePosition.x,bladePosition.z);
    float minDist = distance(positionTmp, voronoiPositions[0]);
	glm::uint bestId = 0;

    for(glm::uint i=1; i<nbIntersections; i++){
        float curDist = distance(positionTmp, voronoiPositions[i]);
        if(curDist < minDist){
            minDist = curDist;
            bestId = i;
        }
    }

    return bestId;
}

glm::vec4 GrassGenerator::getRandomPosition(int id){
	glm::vec4 newPos = glm::vec4(0.f, 0.f, 0.f, 1.f);

    float randX = rand(glm::vec2(id, tileID));
    float randZ = rand(glm::vec2(tileID, id));

    newPos.x = randX * float(tileWidth);
    newPos.z = randZ * float(tileHeight);

	glm::vec4 curTilePos = glm::vec4(tilePos.x, 0.f, tilePos.y, 1.f);

    return newPos + curTilePos;
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

    newPos.x = randX * float(tileWidth);
    newPos.z = randZ * float(tileHeight);

	glm::vec4 curTilePos = glm::vec4(tilePos.x, 0.f, tilePos.y, 1.f);

    return newPos + curTilePos;
}

// find the clump in which the grass blade is
glm::uint GrassGenerator::getClumpId(glm::vec3 bladePosition){
    // find the nearest intersections ids
	glm::uint intersections[9];
	glm::uint nbIntersections = getClosestIntersections(bladePosition, intersections);
    // get their jittered positions
	glm::vec2 voronoiPositions[9];
    getVoronoiPositions(nbIntersections, intersections, voronoiPositions);
    // find nearest of them
	glm::uint nearestId = findNearestNeighbour(bladePosition, nbIntersections, voronoiPositions);
    // return the clump id
    return intersections[nearestId];
}

glm::vec2 GrassGenerator::getBend(glm::uint id, glm::uint clumpId, float height, float tilt){
    float maxX = tilt;
    float minX = tilt / 3.f;
    float randX = rand(glm::vec2(id*tileID, clumpId+tileID), minX, maxX);

    float maxY = height;
    float minY = (height / tilt) * randX + height / 4.f;
    float randY = rand(glm::vec2(clumpId*tileID, id+tileID), minY, maxY);

    return glm::vec2(randX, randY);
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
    return glm::radians(rand(glm::vec2(id*tileID, id+tileID)) * 360.f);
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
    int nbVert = tileLOD == HIGH_LOD ? NB_VERT_HIGH_LOD : NB_VERT_LOW_LOD;

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

glm::vec3 GrassGenerator::getModelPos(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], int vertex, float rotation){
    return getModelPos(center, positions[vertex], rotation);
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
		glm::vec3 modelPos = getModelPos(center, positions, idx, rotation);
		glm::vec4 worldPos = getWorldPos(center, positions, idx, rotation);
		glm::vec3 normal = getRotatedNormals(normals[idx], rotation);
        geomFragCol = colors[idx];
        geomFragNormal = normal;
        geomFragPos = modelPos;
	//	std::cout<<worldPos.x<<" "<<worldPos.y<<" "<<worldPos.z<<std::endl;
		vex o;
		o.position = worldPos;
		o.color = geomFragCol;
		o.height = height;
		o.center = center;
        gl_Position.push_back(o); ;
		
    }
}
glm::vec3 GrassGenerator::getAvgNormal(float tilt, float height){
	glm::vec3 P0 = glm::vec3(0.f);
	glm::vec3 P1 = glm::vec3(tilt, height, 0.f);
	glm::vec3 widthTangent = glm::vec3(0.f, 0.f, 1.f);
	glm::vec3 heightTangent = normalize(P1-P0);
    return cross(heightTangent, widthTangent);
}

float GrassGenerator::getRotation(float tilt, float height){
    float rotation = vertexData._Rotation;
	glm::vec3 normal = getAvgNormal(tilt, height);
	glm::vec4 temp = (view*glm::vec4(getRotatedNormals(normal, rotation), 1.f));
    normal = {temp.x,temp.y,temp.z};

	glm::vec3 camDir = glm::vec3(0.f, 0.f, -1.f);

    float dotProduct = dot(normalize(camDir), normalize(normal));
    rotation += acos(dotProduct) / 3.f;

    return rotation;
}


void GrassGenerator::createTriangles(glm::vec3 center, float rotation, float height,glm::vec3 color,
    glm::vec3 positions[NB_VERT_HIGH_LOD], 
    glm::vec3 normals[NB_VERT_HIGH_LOD],
    glm::vec3 colors[NB_VERT_HIGH_LOD]
    ){
    int nbQuads = tileLOD == HIGH_LOD ? NB_QUAD_HIGH_LOD : NB_QUAD_LOW_LOD;
    int nbVert = tileLOD == HIGH_LOD ? NB_VERT_HIGH_LOD : NB_VERT_LOW_LOD;
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

void GrassGenerator::GetVex(std::vector<vex>&oArray){
	tileWidth = 0.1;
	view = glm::mat4(1.f);
	tileHeight = 0.1;
	gridNbCols = 1;
	gridNbLines = 1;
	tilePos = glm::vec2(0.f,0.f);
	tileID = 1;
	const int num_of_blades = 64;
	glm::vec4 positions[num_of_blades];
	float heights[num_of_blades];
	float widths[num_of_blades];
	glm::vec4 colors[num_of_blades];
	float rotations[num_of_blades];
	float tilts[num_of_blades];
	glm::vec2 bends[num_of_blades];


	int _Width = 800;
	int _Height = 600;

	for(int i = 0;i<num_of_blades;i+=1){
		int instanceIndex = i;
    	// Store data in buffers
	//	glm::vec4 position = getRandomPosition(instanceIndex);

		glm::vec4 position = getRandomPosition(glm::vec2(instanceIndex, tileID), glm::vec2(tileID, instanceIndex));
		glm::uint clumpId = getClumpId(glm::vec3(position.x,position.y,position.z));
    	float height = rand({position.x,position.z}, MIN_HEIGHT, MAX_HEIGHT);
    	float width = rand({position.x,position.z}, MIN_WIDTH, MAX_WIDTH);
		glm::vec4 color = getColor(clumpId);
    	float rotation = getRotation(instanceIndex);
    	float tilt = getTilt(clumpId, height);
//		glm::vec2 bend = getBend(instanceIndex, clumpId, height, tilt);

		glm::vec2 bend = getBend(glm::vec2(clumpId, position.x), glm::vec2(position.z, clumpId), height, tilt);
		positions[instanceIndex] = position;
	    heights[instanceIndex] = height;
	    widths[instanceIndex] = width;
	    colors[instanceIndex] = color;
	    rotations[instanceIndex] = rotation;
	    tilts[instanceIndex] = tilt;
	    bends[instanceIndex] = bend;
		//std::cout<<position.x<<" "<<position.y<<" "<<position.z<<std::endl;
		//std::cout<<width<<std::endl;
		//std::cout<<tilt<<std::endl;
		//std::cout<<std::endl;
	}
	for(int i = 0;i<num_of_blades;i+=1){
		int gl_VertexID = i;
		vertexData._Position = positions[gl_VertexID];
    	vertexData._Height = heights[gl_VertexID];
    	vertexData._Width = widths[gl_VertexID];
    	vertexData._Color = colors[gl_VertexID];
    	vertexData._Rotation = rotations[gl_VertexID];
    	vertexData._Tilt = tilts[gl_VertexID];
    	vertexData._Bend = bends[gl_VertexID];
		glm::vec3 pos = glm::vec3(vertexData._Position.x,vertexData._Position.y,vertexData._Position.z);
 	    float height = vertexData._Height;
 	    float width = vertexData._Width;
		glm::vec3 color = glm::vec3(vertexData._Color.x,vertexData._Color.y,vertexData._Color.z);
 	    float tilt = vertexData._Tilt;
		glm::vec2 bend = vertexData._Bend;
 	    float rotation = getRotation(tilt, height);

		glm::vec3 positions[NB_VERT_HIGH_LOD];
		glm::vec3 normals[NB_VERT_HIGH_LOD];
		glm::vec3 colors[NB_VERT_HIGH_LOD];

		getVerticesPositionsAndNormals(pos, width, height, tilt, bend, color, positions, normals, colors);
		createTriangles(pos, rotation,height, color, positions, normals, colors);
	}	
	oArray.assign(gl_Position.begin(), gl_Position.end());
}

