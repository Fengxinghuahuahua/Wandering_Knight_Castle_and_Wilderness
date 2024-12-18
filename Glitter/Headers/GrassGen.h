#include "glm/ext/vector_float3.hpp"
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#define PI 3.1415926535
struct vex{
	glm::vec3 position;
	glm::vec3 center;
	glm::vec3 color;
	float height;
    glm::vec3 normal;
};

class GrassGenerator{
	private:
		static const int NB_VERT_HIGH_LOD = 15;
		static const int NB_VERT_LOW_LOD = 7;
		const int HIGH_LOD = 1;
		const int LOW_LOD = 2;
		const int NB_QUAD_HIGH_LOD = 6;
		const int NB_QUAD_LOW_LOD = 2;
		const glm::vec3 TIP_COLOR = glm::vec3(0.5f, 0.5f, 0.1f);
		const float MAX_WIDTH = 0.006f;
		const float MIN_WIDTH = 0.008f;
		const float MAX_HEIGHT = 0.1f; 
		const float MIN_HEIGHT = 0.07f;
		const float MAX_GREEN = 1.5f;
		const float MIN_GREEN = 0.5f;
        
		int _lod;
		std::vector<vex> _positions;
		float _width;
		float _height;
		glm::vec2 _tile_pos = glm::vec2(0.f,0.f);
		int _tile_id;
		float rand(glm::vec2 co);
		float rand(glm::vec2 co,float mini,float maxi);
		glm::vec4 getRandomPosition(glm::vec2 seed1, glm::vec2 seed2);
		glm::vec2 getBend(glm::vec2 seed1, glm::vec2 seed2, float height, float tilt);
		glm::vec4 getColor(glm::uint clumpId);
		float getRotation(glm::uint id);
		float getTilt(glm::uint id, float height);
		glm::mat3 getRotationMatrix(float rotation);
		glm::vec2 quadraticBezierCurve(float t, glm::vec2 P0, glm::vec2 P1, glm::vec2 P2);
		glm::vec2 quadraticBezierCurveDerivative(float t, glm::vec2 P0, glm::vec2 P1, glm::vec2 P2);
		glm::vec3 getColor(glm::vec3 color, float maxHeight, float curHeight);
		void getVerticesPositionsAndNormals(glm::vec3 pos, float width, float height, float tilt, glm::vec2 bend, glm::vec3 color,glm::vec3 positions[NB_VERT_HIGH_LOD],glm::vec3 normals[NB_VERT_HIGH_LOD],glm::vec3 colors[NB_VERT_HIGH_LOD]);
		glm::vec3 getModelPos(glm::vec3 center, glm::vec3 position, float rotation);
		glm::vec4 getWorldPos(glm::vec3 center, glm::vec3 position, float rotation);
		glm::vec4 getWorldPos(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], int vertex, float rotation);
		glm::vec3 getRotatedNormals(glm::vec3 normal, float rotation);
		void createTriangle(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], glm::vec3 normals[NB_VERT_HIGH_LOD], glm::vec3 colors[NB_VERT_HIGH_LOD],float rotation, float height,glm::vec3 color, int indices[3]);
		glm::vec3 getAvgNormal(float tilt, float height);
		void createTriangles(glm::vec3 center, float rotation, float height,glm::vec3 color,glm::vec3 positions[NB_VERT_HIGH_LOD],glm::vec3 normals[NB_VERT_HIGH_LOD],glm::vec3 colors[NB_VERT_HIGH_LOD]);
	public:
		void GetVex(std::vector<vex>&oArray,int lod,int numOfBlades);
};

