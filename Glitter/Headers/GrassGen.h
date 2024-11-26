#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#define PI 3.1415926535
struct vex{
	glm::vec3 position;
	glm::vec3 center;
	glm::vec3 color;
	float height;
};
struct VertexData{
	glm::vec4 _Position;
    float _Height;
    float _Width;
	glm::vec4 _Color;
    float _Rotation;
    float _Tilt;
	glm::vec2 _Bend;
};

class GrassGenerator{
	private:
		static const int NB_VERT_HIGH_LOD = 15;
		static const int NB_VERT_LOW_LOD = 7;

		glm::mat4 view;
		glm::mat4 proj;
		int tileLOD = 2;
		//out
		glm::vec3 geomFragCol;
		glm::vec3 geomFragNormal;
		glm::vec3 geomFragPos;
		
		std::vector<vex> gl_Position;
		const int HIGH_LOD = 1;
		const int LOW_LOD = 2;
		const int NB_QUAD_HIGH_LOD = 6;
		const int NB_QUAD_LOW_LOD = 2;
		const glm::vec3 TIP_COLOR = glm::vec3(0.5f, 0.5f, 0.1f);
		const glm::vec4 red = glm::vec4(1.f, 0.f, 0.f, 1.f);

		VertexData vertexData;
		const float MAX_WIDTH = 0.006f;
		const float MIN_WIDTH = 0.008f;
		const float MAX_HEIGHT = 0.1f; 
		const float MIN_HEIGHT = 0.05f;
		const float MAX_GREEN = 1.5f;
		const float MIN_GREEN = 0.5f;
		float tileWidth = 10;
		float tileHeight = 10;
		int gridNbCols = 1;
		int gridNbLines = 1;
		glm::vec2 tilePos = glm::vec2(0.f,0.f);
		int tileID = 1;
		float rand(glm::vec2 co);
		float rand(glm::vec2 co,float mini,float maxi);
		glm::uint getGridCell(glm::vec2 position);
		bool cellIsFirstRow(glm::uint cellId);
		bool cellIsLastRow(glm::uint cellId);
		bool cellIsFirstCol(glm::uint cellId);
		bool cellIsLastCol(glm::uint cellId);
		glm::uint getClosestIntersections(glm::vec3 bladePosition, glm::uint resultIds[9]);
		glm::vec2 getIntersectionPosition(glm::uint cellId);
		void getVoronoiPositions(glm::uint nbIntersections, glm::uint voronoiCellIds[9], glm::vec2 voronoiPositions[9]);
		glm::uint findNearestNeighbour(glm::vec3 bladePosition, glm::uint nbIntersections, glm::vec2 voronoiPositions[9]);
		glm::vec4 getRandomPosition(int id);
		glm::vec4 getRandomPosition(glm::vec2 seed1, glm::vec2 seed2);
		glm::uint getClumpId(glm::vec3 bladePosition);
		glm::vec2 getBend(glm::uint id, glm::uint clumpId, float height, float tilt);
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
		glm::vec3 getModelPos(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], int vertex, float rotation);
		glm::vec4 getWorldPos(glm::vec3 center, glm::vec3 position, float rotation);
		glm::vec4 getWorldPos(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], int vertex, float rotation);
		glm::vec3 getRotatedNormals(glm::vec3 normal, float rotation);
		void createTriangle(glm::vec3 center, glm::vec3 positions[NB_VERT_HIGH_LOD], glm::vec3 normals[NB_VERT_HIGH_LOD], glm::vec3 colors[NB_VERT_HIGH_LOD],float rotation, float height,glm::vec3 color, int indices[3]);
		glm::vec3 getAvgNormal(float tilt, float height);
		float getRotation(float tilt, float height);
		void createTriangles(glm::vec3 center, float rotation, float height,glm::vec3 color,glm::vec3 positions[NB_VERT_HIGH_LOD],glm::vec3 normals[NB_VERT_HIGH_LOD],glm::vec3 colors[NB_VERT_HIGH_LOD]);
	public:
		void GetVex(std::vector<vex>&oArray);
};

