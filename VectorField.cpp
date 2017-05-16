#include "VectorField.h"
#include "INIReader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h> 
#include <float.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;
using namespace glm;

namespace vectorfield {
    
    VectorField::VectorField(): m_initialized(false), m_gridMin(glm::vec2(0,0)), m_gridMax(glm::vec2(0,0)),
                                m_gridHeight(0), m_lengthRange(glm::vec2(FLT_MAX, -FLT_MAX)) {
        
    }
    
    VectorField::~VectorField() {

    }
    
    void VectorField::init(float min_x, float min_z, float max_x, float max_z, float cell_size, float height) {
        m_gridHeight = height;
        m_gridMin = glm::vec2(min_x, min_z);
        m_gridMax = glm::vec2(max_x, max_z);
        m_gridSizeX = int( (max_x - min_x) / cell_size );
        m_gridSizeZ = int( (max_z - min_z) / cell_size );
        m_cellSize = cell_size;
        
        int numPoints = m_gridSizeX * m_gridSizeZ;
        m_gridValues.resize(numPoints);
    }
    
    float angleWithXAxis(glm::vec2 vec2) {
        glm::vec2 vec1 = glm::vec2(1, 0);
        float theta = acos( glm::dot(vec1, vec2) / (glm::length(vec1)*glm::length(vec2)));
        if(vec2[1] > 0)
            theta = -theta;
        return theta;
    }
    
    void VectorField::addControlPoint(const float px, const float pz,
                                      const float vx, const float vz) {
        
        ControlPoint p;
        p.pos = glm::vec2(px, pz);
        p.value = glm::vec2(vx, vz);
        m_controlPoints.push_back(p);
        
        glm::vec2 dir = glm::vec2(vx, vz);
        float length = 150.0f * glm::length(dir);
        Mesh* m = MeshUtils::cylinder(150, length);
        m->rotate(degreesToRadians(-90), glm::vec3(0, 0, 1));
        m->rotate(angleWithXAxis(dir), glm::vec3(0, 1, 0));
        m->moveTo(glm::vec3(px, m_gridHeight, pz));
        m_meshControlPoints.push_back(m);
        
        if (m_lengthRange[0] > glm::length(dir))
            m_lengthRange[0] = glm::length(dir);
        
        if (m_lengthRange[1] < glm::length(dir))
            m_lengthRange[1] = glm::length(dir);
    
    }
    
    void VectorField::printInfo() {
        
        cout << "m_lengthRange: " << m_lengthRange[0] << " " << m_lengthRange[1] << endl;
        
    }
    
    glm::vec4 VectorField::calColor(float v) {
        
        float vmin = m_lengthRange[0];
        float vmax = m_lengthRange[1];
        
        
        glm::vec4 c = glm::vec4(1.0); // white
        
        float dv;
            
        if (v < vmin)
            v = vmin;
        if (v > vmax)
            v = vmax;
        dv = vmax - vmin;
        
        if (v < (vmin + 0.25 * dv)) {
            c[0] = 0;
            c[1] = 4 * (v - vmin) / dv;
        } else if (v < (vmin + 0.5 * dv)) {
            c[0] = 0;
            c[2] = 1 + 4 * (vmin + 0.25 * dv - v) / dv;
        } else if (v < (vmin + 0.75 * dv)) {
            c[0] = 4 * (v - vmin - 0.5 * dv) / dv;
            c[2] = 0;
        } else {
            c[1] = 1 + 4 * (vmin + 0.75 * dv - v) / dv;
            c[2] = 0;
        }
            
        return c;
    }
    
    
    glm::vec2 VectorField::calGridValue(int xi, int zi, float power, float smoothing) {
        
        glm::vec2 value;
        
        glm::vec2 nominator = glm::vec2(0);
        glm::vec2 denominator = glm::vec2(0);
        
        float x = m_gridMin[0] + xi * m_cellSize;
        float z = m_gridMin[1] + zi * m_cellSize;
        
        
        for (int i = 0; i < m_controlPoints.size(); i++) {
            
            float px = m_controlPoints[i].pos[0];
            float pz = m_controlPoints[i].pos[1];
            float v0 = m_controlPoints[i].value[0];
            float v1 = m_controlPoints[i].value[1];
            
            float dist = sqrt( (x-px)*(x-px) + (z-pz)*(z-pz) + smoothing*smoothing );
            
            if(dist < 0.0000000001)
                value = m_controlPoints[i].value;
            
            nominator[0] = nominator[0] + (v0 / pow(dist, power));
            nominator[1] = nominator[1] + (v1 / pow(dist, power));
            
            denominator[0] = denominator[0] + (1 / pow(dist,power));
            denominator[1] = denominator[1] + (1 / pow(dist,power));
            
            for(int j=0; j < 2; j++) {
                if ( denominator[j] > 0 )
                    value[j] = nominator[j] / denominator[j];
                else
                    value[j] = -FLT_MAX;
            }
            
        }
        
        return value;
    }
    
    void VectorField::update(){
        
        // recalculate colors
        for(int i=0; i < m_meshControlPoints.size(); i++) {
            glm::vec4 c = calColor(glm::length(glm::vec2(m_controlPoints[i].value)));
            m_meshControlPoints[i]->setColor(c);
        }
        
        // calculate grid values
        for (int z = 0; z < m_gridSizeZ; z++) {
            for (int x = 0; x < m_gridSizeX; x++) {
                m_gridValues[z * m_gridSizeX + x] = calGridValue(x, z);
                cout << z << " " << x << " " << m_gridValues[z * m_gridSizeX + x][0] << " "
                     << m_gridValues[z * m_gridSizeX + x][1] << endl;
                
                // testing
                /*
                glm::vec2 dir = m_gridValues[z * m_gridSizeX + x];
                float length = 150.0f * glm::length(dir);
                Mesh* m = MeshUtils::cylinder(150, length);
                m->rotate(degreesToRadians(-90), glm::vec3(0, 0, 1));
                m->rotate(angleWithXAxis(dir), glm::vec3(0, 1, 0));
                m->moveTo(glm::vec3(m_gridMin[0] + x * m_cellSize, m_gridHeight, m_gridMin[1] + z * m_cellSize));
                m_meshTest.push_back(m);
                */
                
            }
        }
    }
    
    void VectorField::setup(){
        
        m_initialized = true;
    }
    
    void VectorField::render(const float MV[16], const float P[16]) {
        
        if(!m_initialized)
            setup();
        
        for(int i=0; i < m_meshControlPoints.size(); i++)
            m_meshControlPoints[i]->render(MV, P);
        
        for(int i=0; i < m_meshTest.size(); i++)
            m_meshTest[i]->render(MV, P);
        
    }
    
}; //namespace vectorfield
