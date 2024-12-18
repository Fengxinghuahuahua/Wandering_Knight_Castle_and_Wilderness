#include <glm/glm.hpp>
#include "camera.h"
#include "glm/ext/vector_float3.hpp"
#include "shader_m.h"
#include <vector>
#include <GrassTile.h>
class Grass{
private:
	float* _high_vertices;
    float* _low_vertices;
    std::vector<glm::mat4> highLODModel;
    std::vector<glm::mat4> lowLODModel;
	unsigned int _high_vbo, _high_vao;
	unsigned int _low_vbo, _low_vao;
	int _high_num_vertices;
	int _low_num_vertices;
	void loadVertices();
    bool isInFrustum(const Frustum& frustum, const glm::vec3 &pos);
    bool isInFrustum(const Frustum& frustum, const GrassTile& tile);
    bool isInFrontOfPlane(const Plane& plane, const glm::vec3& pos);
public:
	Grass();
    void setTiles(std::vector<GrassTile&>tiles);
    void render(Shader* shaders, std::vector<GrassTile>& tiles, Frustum& frustum,float time);
    void instanceRender(Shader* shaders, std::vector<GrassTile>& tiles, Frustum& frustum,float time);
	void render(Shader* shaders,Camera* camera, Frustum &frustum,const glm::vec3& pos,float rotator, float time); };
