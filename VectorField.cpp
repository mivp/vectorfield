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
                                m_maxParticles(10000), m_lastUsedParticle(0), m_curOffset(0), m_pointScale(1.0), m_skip(0),
                                m_particleType(TYPE_ARROW), m_numActiveParticles(0), m_meshControlPoints(0), m_arrows(0),
                                m_arrowScale(1.0), m_elevationScale(1.0) {
        
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
        
        if(m_meshControlPoints)
            delete m_meshControlPoints;
        
        if(m_arrows)
            delete m_arrows;
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
        m_activeVectors.resize(m_maxParticles);
        
        m_meshControlPoints = MeshUtils::cylinder(200, 150);
        m_arrows = MeshUtils::cylinder(80, 100, 4, 6);
        
        m_elevationMap.clear();
        m_elevationMapSize = glm::vec2(0);
    }
    
    float angleWithXAxis(glm::vec2 vec2) {
        glm::vec2 vec1 = glm::vec2(1, 0);
        float theta = acos( glm::dot(vec1, vec2) / (glm::length(vec1)*glm::length(vec2)));
        if(vec2[1] > 0)
            theta = -theta;
        return theta;
    }
    
    void VectorField::loadElevationFromFile(string txtfile) {
        ifstream myfile;
        myfile.open(txtfile.c_str(), std::ifstream::in);
        
        if (!myfile.is_open()) {
            cout << "Unable to open file " << txtfile << endl;
            return;
        }
        
        //first line contains nrows, ncols
        string line, token;
        if (getline(myfile, line)) {
            stringstream ss;
            ss << line;
            ss >> token; m_elevationMapSize[0] = atof(token.c_str());
            ss >> token; m_elevationMapSize[1] = atof(token.c_str());
            cout << "nrows: " << m_elevationMapSize[0] << "ncols: " << m_elevationMapSize[1] << endl;
        }
        else {
            cout << "Unable to get elevation map size " << endl;
            return;
        }
        
        int numval = m_elevationMapSize[0] * m_elevationMapSize[1];
        m_elevationMap.resize(numval);
        
        int row = 0;
        while ( getline (myfile,line) ) {
            stringstream ss;
            ss << line;
            for (int col=0; col < m_elevationMapSize[1]; col++) {
                ss >> token;
                m_elevationMap[row*m_elevationMapSize[1] + col] = atof(token.c_str());
            }
            row++;
        }
    
        myfile.close();
    }
    
    void VectorField::addControlPoint(const float px, const float pz,
                                      const float vx, const float vz) {
        
        ControlPoint p;
        p.pos = glm::vec2(px, pz);
        p.value = glm::vec2(vx, vz);
        m_controlPoints.push_back(p);
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
    
    float VectorField::calElevation(float posx, float posz) {
        
        if(m_elevationMap.size() < 1)
            return 0;
        
        int indx = round((posx - m_gridMin[0]) / (m_gridMax[0] - m_gridMin[0]) * m_elevationMapSize[1]);
        int indz = round((posz - m_gridMin[1]) / (m_gridMax[1] - m_gridMin[1]) * m_elevationMapSize[0]);
        
        indx = indx < 0 ? 0 : indx;
        indx = indx > m_elevationMapSize[1] - 1 ? m_elevationMapSize[1] - 1 : indx;
        indz = indz < 0 ? 0 : indz;
        indz = indz > m_elevationMapSize[0] - 1 ? m_elevationMapSize[0] - 1 : indz;
        
        return m_elevationScale * m_elevationMap[indz * m_elevationMapSize[1] + indx];
    }
    
    void VectorField::update(){
        
        int numControlPoints = m_controlPoints.size();
        
        m_meshControlPoints->setNumInstances(numControlPoints);
        
        for(int i=0; i < numControlPoints; i++) {
            
            glm::vec2 dir = m_controlPoints[i].value;
            float length = glm::length(dir);
           
            m_meshControlPoints->scale(i, glm::vec3(1, length, 1));
            m_meshControlPoints->rotate(i, degreesToRadians(-90), glm::vec3(0, 0, 1));
            m_meshControlPoints->rotate(i, angleWithXAxis(dir), glm::vec3(0, 1, 0));
            float height = m_gridHeight + calElevation(m_controlPoints[i].pos[0], m_controlPoints[i].pos[1]);
            m_meshControlPoints->moveTo(i, glm::vec3(m_controlPoints[i].pos[0], height, m_controlPoints[i].pos[1]));
            
            if (m_lengthRange[0] > glm::length(dir))
                m_lengthRange[0] = glm::length(dir);
            
            if (m_lengthRange[1] < glm::length(dir))
                m_lengthRange[1] = glm::length(dir);
            
        }
        
        
        // recalculate colors
        for(int i=0; i < numControlPoints; i++) {
            glm::vec4 c = calColor(glm::length(glm::vec2(m_controlPoints[i].value)));
            m_meshControlPoints->setColor(i, c);
        }
        
        // calculate grid values
        for (int z = 0; z < m_gridSizeZ; z++) {
            for (int x = 0; x < m_gridSizeX; x++) {
                m_gridValues[z * m_gridSizeX + x] = calGridValue(x, z);
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
        
        m_skip++;
        if(m_skip % 2 != 0)
            return;
    
        float SPEED = 8.0f;
        
        m_numActiveParticles = 0;
        
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
                p.pos = p.pos + SPEED*glm::vec3(v[0], 0, v[1]);
                p.pos[1] = m_gridHeight + calElevation(p.pos[0], p.pos[2]);
                
                if(p.pos[0] < m_gridMin[0] || p.pos[2] <  m_gridMin[1] ||
                   p.pos[0] > m_gridMax[0] || p.pos[2] >  m_gridMax[1] ) {
                    
                    p.state = STATE_DYING;
                    
                } else {
                    glm::vec4 color = calColor(glm::length(v));
                    m_activeVertices[m_numActiveParticles] = p.pos;
                    m_activeColors[m_numActiveParticles] = color;
                    m_activeVectors[m_numActiveParticles] = v;
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
        
        glGenVertexArrays(1, &m_vao);
        glBindVertexArray(m_vao);
        
        glGenBuffers(1, &m_vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * m_maxParticles, NULL, GL_STREAM_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer( 0,  3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
        
        glGenBuffers(1, &m_vbo[1]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * m_maxParticles, NULL, GL_STREAM_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer( 1,  4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void*)0);
        
        glBindVertexArray(0);
        
        //glEnable(GL_POINT_SPRITE);
        //glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        
        m_initialized = true;
    }
    
    void VectorField::render(const float MV[16], const float P[16]) {
        
        if(!m_initialized)
            setup();
        
        if(m_meshControlPoints)
            m_meshControlPoints->render(MV, P);
        
        // particles
        simulateParticles();
        if(m_numActiveParticles <= 0)
            return;
        
        if(m_particleType == TYPE_ARROW) {
            m_arrows->setNumInstances(m_numActiveParticles);
            for(int i=0; i < m_numActiveParticles; i++) {
                m_arrows->reset(i);
                float length = glm::length(m_activeVectors[i]);
                m_arrows->scale(i, glm::vec3(m_arrowScale, length, m_arrowScale));
                m_arrows->rotate(i, degreesToRadians(-90), glm::vec3(0, 0, 1));
                m_arrows->rotate(i, angleWithXAxis(m_activeVectors[i]), glm::vec3(0, 1, 0));
                m_arrows->moveTo(i, m_activeVertices[i]);
                m_arrows->setColor(i, m_activeColors[i]);
            }
            
            m_arrows->render(MV, P);
        }
        else {
            glm::mat4 modelViewMatrix = glm::make_mat4(MV);
            glm::mat4 projMatrix = glm::make_mat4(P);
            glm::mat4 mvp = projMatrix * modelViewMatrix;
            
            GLSLProgram* shader = m_material->getShader();
            shader->bind();
            
            shader->setUniform("uMV", modelViewMatrix);
            shader->setUniform("uMVP", mvp);
            shader->setUniform("uScreenHeight", 800.0f);
            shader->setUniform("uPointScale", m_pointScale);
            
            glBindVertexArray(m_vao);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_numActiveParticles * sizeof(glm::vec3), &m_activeVertices[0]);
            
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
            glBufferSubData(GL_ARRAY_BUFFER, 0, m_numActiveParticles * sizeof(glm::vec4), &m_activeColors[0]);
            
	    glEnable(GL_POINT_SPRITE);
            glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

            glDrawArrays( GL_POINTS, 0, m_numActiveParticles );
             
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            shader->unbind();
            glBindVertexArray(0);
        }
    }
    
}; //namespace vectorfield
