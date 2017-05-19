#ifndef __MESH_H__
#define __MESH_H__

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/rotate_vector.hpp>

using std::vector;
using std::string;

#include "Material.h"

#ifndef PI
#define PI 3.14159265359
#define PI2 2*PI
#endif
#define degreesToRadians(angleDegrees) (angleDegrees * PI / 180.0)

#define GLM_FORCE_RADIANS

namespace vectorfield {
    
    /**
     Create full mesh from heighmap
     */
    class Mesh {
        
    private:
        unsigned int m_vbo[3];
        unsigned int m_ibo;
        unsigned int m_vao;
        bool m_initialized;
        
        glm::mat4 m_modelMatrix;
        
        glm::vec4 m_color;
        
    public:
        // mesh data
        vector<glm::vec3> vertices;
        vector<glm::vec3> normals;
        vector<glm::vec2> uvs;
        vector<unsigned int> indices;
        
        // display
        bool m_wireframe;
        Material* m_material;
        
    private:
        void setup();
        
    public:
        Mesh();
        ~Mesh();
        
        void moveTo(glm::vec3 pos);
        void rotate(float angle, glm::vec3 axis);
        void setColor(glm::vec4 color) { m_color = color; };
        void render(const float MV[16], const float P[16]);
    };
    
    /**
     Create meshes
     */
    class MeshUtils {
        
    public:
        static Mesh* cube(float size);
        static Mesh* sphere(float radius, int slices, int stacks);
        static Mesh* cylinder(float radius = 1.0f, float length = 2.0f, int rings = 5, int slices = 15 );
    };
    
}; //namespace vectorfield

#endif
