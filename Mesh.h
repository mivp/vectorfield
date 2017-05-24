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
        int m_numInstances;
        unsigned int m_vbo[7];
        unsigned int m_ibo;
        unsigned int m_vao;
        bool m_initialized;
        
        vector<glm::vec4> m_colors;
        vector<glm::mat4> m_modelMatrixes;
        
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
        
        void setNumInstances(int num);
        
        
        void reset(int instance = 0);
        void scale(glm::vec3 s) { scale(0, s); }
        void rotate(float angle, glm::vec3 axis) { rotate(0, angle, axis); }
        void moveTo(glm::vec3 pos) { moveTo(0, pos); }
        
        void scale(int instance, glm::vec3 s);
        void rotate(int instance, float angle, glm::vec3 axis);
        void moveTo(int instance, glm::vec3 pos);
        
        void setColor(glm::vec4 color) { setColor(0, color); }
        void setColor(int instance, glm::vec4 color) { m_colors[instance] = color; };
        
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
