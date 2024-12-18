#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_float3.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <vector>
#include <shader_m.h>
#include <GrassTile.h>
struct TransForm{
    glm::vec3 trans;
    float rotate;
};
class Terrain{
	private:
		glm::vec2 _bottom;
		glm::vec2 _top;
		GLuint _vao,_vbo;
		int _stride = 11;
		GLuint _terrainTestureID;
		GLuint _terrainNormalID;
		GLuint _terrainDisplaceTextureID;
		int _width;
		int _height;
		std::vector<unsigned int>_indices;
		std::vector<float>_terrainVertices;
        std::vector<glm::vec3> _randGrassPositions;
		unsigned char* _heightMapData;
		void loadHeightMap(std::string path,std::string texture_path,std::string normal_path,std::string displace_path);
		void generateTerrainVertices(std::vector<float>& oVertices,unsigned char* data);
		void generateTerrainIndices(std::vector<unsigned int>&oIndices);
		void calculateTerrainNormals(std::vector<float>&oVertices,std::vector<unsigned int>&oIndices);
		void calculateTangents(std::vector<float>&Vertices,std::vector<unsigned int>&Indices);
		float getHeightFromTexture(int,int,unsigned char*);
		void setUpTerrain();
			
	public:
		Terrain();
		float getHeight(float x,float y);
		void render(Shader* shaders);
		std::vector<float>& getVertices();
		void set2Zero();
		std::vector<TransForm> getRandomPositions();
        std::vector<GrassTile> GetGrassTiles(float areaWidth,float minDistance, int k);
};
