#ifndef __TERRAIN_H__
#define __TERRAIN_H__

#include <vector>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include <glm/gtx/rotate_vector.hpp>

using std::vector;
using std::string;

#include "Material.h"
#include "Mesh.h"

struct ControlPoint {
    glm::vec2 pos;
    glm::vec2 value;
};

namespace vectorfield {

/**
 Tessellation terrain
 */
class VectorField {
    
private:
    bool m_initialized;
    
    vector<ControlPoint> m_controlPoints;
    vector<Mesh*> m_meshControlPoints;
    vector<Mesh*> m_meshTest;
    
    //2d grid
    glm::vec2 m_gridMin, m_gridMax;
    float m_gridHeight;
    float m_cellSize;
    int m_gridSizeX, m_gridSizeZ;
    vector<glm::vec2> m_gridValues;
    
    //display
    glm::vec2 m_lengthRange;
    
private:
    glm::vec4 calColor(float value);
    glm::vec2 calGridValue(int x, int z, float power = 2, float smoothing = 0);
    
public:
    VectorField();
    ~VectorField();
    
    void init(float min_x, float min_z, float max_x, float max_z, float cell_size, float height = 0);
    void printInfo();
    
    void addControlPoint(const float px, const float pz,
                         const float vx, const float vz);
    
    void update();
    void setup();
    void render(const float MV[16], const float P[16]);
};

}; //namespace vectorfield

#endif
