#include <glm/glm.hpp>
#include "shader_m.h"

class Grass{
private:
	float* _Vertices;
	unsigned int _VBO, _VAO;
	int _NumOfVertices;
	void loadVertices();
public:
	Grass();
	void render(Shader* shaders,const glm::vec3& pos, float time);
};
