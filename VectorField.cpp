#include "VectorField.h"

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
    
    VectorField::VectorField(): m_vao(0), m_updated(false), m_initialized(false), m_gridMin(glm::vec2(0,0)), m_gridMax(glm::vec2(0,0)),
                                m_gridHeight(0), m_lengthRange(glm::vec2(FLT_MAX, -FLT_MAX)),
                                m_maxParticles(10000), m_lastUsedParticle(0), m_curOffset(0), m_pointScale(1.0) {
        
    }
    
    VectorField::~VectorField() {
        if(m_vao > 0) {
            glDeleteVertexArrays(1,&m_vao);
        }
        
        for(int i=0; i < 2; i++) {
            if(m_vbo[i])
                glDeleteBuffers(1,&m_vbo[i]);
        }
        
        if(m_material)
            delete m_material;
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
        
        m_particleContainer.resize(m_maxParticles);
        m_activeVertices.resize(m_maxParticles);
        m_activeColors.resize(m_maxParticles);
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
        Mesh* m = MeshUtils::cylinder(300, length);
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
                //cout << z << " " << x << " " << m_gridValues[z * m_gridSizeX + x][0] << " "
                //     << m_gridValues[z * m_gridSizeX + x][1] << endl;
                
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
        m_updated = true;
    }
    
    //particles
    int VectorField::findUnusedParticle() {
        
        for(int i = m_lastUsedParticle; i < m_maxParticles; i++){
            if ( m_particleContainer[i].state == STATE_DEAD ) {
                m_lastUsedParticle = i;
                return i;
            }
        }
        
        for(int i=0; i<m_lastUsedParticle; i++){
            if ( m_particleContainer[i].state == STATE_DEAD ){
                m_lastUsedParticle = i;
                return i;
            }
        }
        
        return 0;
    }
    
    void VectorField::simulateParticles() {
        
        if(!m_updated)
            return;
        
        // generate some new particles
        int newparticles = 0.016f*500.0; // ~ 1k a second
        
        float delta = 2000;
        
        float size_x = m_gridMax[0] - m_gridMin[0];
        float size_z = m_gridMax[1] - m_gridMin[1];
        
        for(int i=0; i < newparticles; i++) {
            
            float x, z;
            if(m_curOffset < size_x) {
                x = m_curOffset;
                z = m_gridMin[1];
            }
            else if (m_curOffset < size_x + size_z) {
                x = m_gridMax[0];
                z = m_curOffset - size_x;
            }
            else if (m_curOffset < 2*size_x + size_z) {
                x = m_curOffset - size_x - size_z;
                z = m_gridMax[1];
            }
            else if (m_curOffset < 2*size_x + 2*size_z) {
                x = m_gridMin[0];
                z = m_curOffset - 2*size_x - size_z;
            }
            else {
                m_curOffset = 0; //-= 2*size_x + 2*size_z - 2000;
            }
            m_curOffset += delta;
            
            int ind = findUnusedParticle();
            Particle& p = m_particleContainer[ind];
            p.pos = glm::vec3(x, m_gridHeight, z);
            p.state = STATE_ALIVE;
            p.opaque = 0;
        }
        
        
        // update all particles
        m_numActiveParticles = 0;
        for (int i=0; i  < m_maxParticles; i++) {
            Particle& p = m_particleContainer[i];
            if(p.state != STATE_DEAD) {
                
                int ix = int(p.pos[0] / m_cellSize);
                int iz = int(p.pos[2] / m_cellSize);
                ix = ix < 0 ? 0 : ix;
                ix = ix > m_gridSizeX - 1 ? m_gridSizeX - 1 : ix;
                iz = iz < 0 ? 0 : iz;
                iz = iz > m_gridSizeZ - 1 ? m_gridSizeZ - 1 : iz;
            
                glm::vec2 v = m_gridValues[iz * m_gridSizeX + ix];
                p.pos = p.pos + 3.0f*glm::vec3(v[0], 0, v[1]);
                
                if(p.pos[0] < m_gridMin[0] || p.pos[2] <  m_gridMin[1] ||
                   p.pos[0] > m_gridMax[0] || p.pos[2] >  m_gridMax[1] ) {
                    
                    p.state = STATE_DYING;
                    
                } else {
                    glm::vec4 color = calColor(glm::length(v));
                    m_activeVertices[m_numActiveParticles] = p.pos;
                    m_activeColors[m_numActiveParticles] = color;
                    m_numActiveParticles++;
                }
                
                if(p.state == STATE_DYING) {
                    p.opaque -= 0.001;
                    if(p.opaque < 0.0001)
                        p.state = STATE_DEAD;
                }
                else {
                    p.opaque += 0.001;
                    p.opaque = p.opaque > 1.0 ? 1.0 : p.opaque;
                }
                p.color *= p.opaque;
                
            }
        }
        
        //cout << "m_numActiveParticles: " << m_numActiveParticles << endl;
    }
    
    //display
    void VectorField::setup() {
        
        if (!m_updated)
            return;
        
        m_material = new PointMaterial();
        
        //glGenVertexArrays(1, &m_vao);
        //glBindVertexArray(m_vao);
        
        glGenBuffers(1, &m_vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_maxParticles, NULL, GL_STREAM_DRAW);
        
        glGenBuffers(1, &m_vbo[1]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m_maxParticles, NULL, GL_STREAM_DRAW);
        
        //glBindVertexArray(0);
        
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        
        m_initialized = true;
    }
    
    void VectorField::render(const float MV[16], const float P[16]) {
        
        if(!m_initialized)
            setup();
        
        for(int i=0; i < m_meshControlPoints.size(); i++)
            m_meshControlPoints[i]->render(MV, P);
        
        for(int i=0; i < m_meshTest.size(); i++)
            m_meshTest[i]->render(MV, P);
        
        // particles
        simulateParticles();
        
        glm::mat4 modelViewMatrix = glm::make_mat4(MV);
        glm::mat4 projMatrix = glm::make_mat4(P);
        glm::mat4 mvp = projMatrix * modelViewMatrix;
        
        GLSLProgram* shader = m_material->getShader();
        shader->bind();
        
        shader->setUniform("uMV", modelViewMatrix);
        shader->setUniform("uMVP", mvp);
        shader->setUniform("uScreenHeight", 800.0f);
        shader->setUniform("uPointScale", m_pointScale);
        
        //glBindVertexArray(m_vao);
        unsigned int val0, val1;
        
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        val0 = glGetAttribLocation(shader->getHandle(), "inPosition");
        glEnableVertexAttribArray(val0);
        glVertexAttribPointer( val0,  3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_numActiveParticles * sizeof(glm::vec3), &m_activeVertices[0]);
        
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
        val1 = glGetAttribLocation(shader->getHandle(), "inColor");
        glEnableVertexAttribArray(val1);
        glVertexAttribPointer( val1,  4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        glBufferSubData(GL_ARRAY_BUFFER, 0, m_numActiveParticles * sizeof(glm::vec4), &m_activeColors[0]);
        
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDrawArrays( GL_POINTS, 0, m_numActiveParticles );
        
        shader->unbind();
	
	glDisableVertexAttribArray(val0);
	glDisableVertexAttribArray(val1);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	shader->unbind();

        //glBindVertexArray(0);
    }
    
}; //namespace vectorfield
