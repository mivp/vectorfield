#include "Mesh.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <float.h>

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;
using namespace glm;

namespace vectorfield {
    
    /**
     Mesh Terrain
     */
    Mesh::Mesh(): m_vao(0), m_ibo(0),
    m_wireframe(false), m_material(0), m_initialized(false) {
        vertices.clear(); normals.clear(); uvs.clear();
        indices.clear();
        m_modelMatrix = glm::mat4(1.0);
        m_color = glm::vec4(1.0);
        m_vbo[0] = 0; m_vbo[1] = 0; m_vbo[2] = 0;
    }
    
    Mesh::~Mesh() {
        if(m_vao > 0)
            glDeleteVertexArrays(1,&m_vao);
        
        for(int i=0; i < 3; i++)
            if(m_vbo[i])
                glDeleteBuffers(1,&m_vbo[i]);
        
        glDeleteBuffers(1,&m_ibo);
        
        if(m_material)
            delete m_material;
        vertices.clear(); normals.clear(); uvs.clear();
        indices.clear();
    }
    
    void Mesh::setup(){
        
        if(vertices.size() < 1)
            return;
        
        if(!m_material)
            m_material = new MeshMaterial();
        
        m_material->shininess = 10.0f;
        m_material->Ka = glm::vec3(0.2, 0.2, 0.2);
        m_material->Kd = glm::vec3(1.0, 1.0, 1.0);
        m_material->Ks = glm::vec3(0.7, 0.7, 0.7);
        
        //glGenVertexArrays(1, &m_vao);
        //glBindVertexArray(m_vao);
        
        // create vbos
        // vertices
        glGenBuffers(1,&m_vbo[0]);
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
        // normals
        if(normals.size() > 0) {
            glGenBuffers(1,&m_vbo[1]);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*normals.size(), &normals[0], GL_STATIC_DRAW);
        }
        // uvs
	/*
        if(uvs.size() > 0) {
            glGenBuffers(1,&m_vbo[2]);
            glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);
            glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*uvs.size(), &uvs[0], GL_STATIC_DRAW);
        }
	cout << m_vbo[0] << " " << m_vbo[1] << " " << m_vbo[2] << endl;
        */

        // create ibo
        glGenBuffers(1,&m_ibo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int)*indices.size(), &indices[0], GL_STATIC_DRAW);
        
        //glBindVertexArray(0);
        
        m_initialized = true;
if(oglError) return;
        
    }
    
    void Mesh::moveTo(glm::vec3 pos) {
        m_modelMatrix = glm::translate(pos) * m_modelMatrix;
        //std::cout<< glm::to_string(m_modelMatrix)<<std::endl;
    }
    
    void Mesh::rotate(float angle, glm::vec3 axis ) {
        
        m_modelMatrix = glm::rotate( angle, axis ) * m_modelMatrix;
        //std::cout<< glm::to_string(m_modelMatrix)<<std::endl;
    }
    
    void Mesh::render(const float MV[16], const float P[16]) {
        
        if(!m_initialized)
            setup();
        
        //glEnable(GL_DEPTH_TEST);
        
        GLSLProgram* shader = m_material->getShader();
        shader->bind();
        if(oglError) return;

        glm::mat4 viewMatrix = glm::make_mat4(MV);
        glm::mat4 projMatrix = glm::make_mat4(P);
        glm::mat4 modelViewMatrix = viewMatrix * m_modelMatrix;
        glm::mat3 normalMatrix = glm::inverseTranspose(glm::mat3(modelViewMatrix));
        glm::mat4 mvp = projMatrix * modelViewMatrix;
        
        shader->setUniform("uModelView", modelViewMatrix);
        shader->setUniform("uMVP", mvp);
        shader->setUniform("uNormalMatrix", normalMatrix);
        
        // Set the lighting parameters
        glm::vec4 worldLightDirection( 0.50000019433, 0.86602529158, 0.0f, 0.0f ); //sin(30), cos(30)
        glm::mat4 worldToEyeNormal( normalMatrix );
        glm::vec4 lightDirection = worldToEyeNormal * worldLightDirection;
        shader->setUniform( "light.position", lightDirection );
        shader->setUniform( "light.intensity", glm::vec3( 1.0f, 1.0f, 1.0f ) );
        
        // Set the material properties
        shader->setUniform( "material.color",  m_color );
        shader->setUniform( "material.Ka",  m_material->Ka );
        shader->setUniform( "material.Kd",  m_material->Kd );
        shader->setUniform( "material.Ks",  m_material->Ks );
        shader->setUniform( "material.shininess", m_material->shininess );
        
        //if(m_wireframe)
        //    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        
        //glBindVertexArray(m_vao);
        unsigned int val0, val1, val2;
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
        val0 = glGetAttribLocation(shader->getHandle(), "inPosition");
        glEnableVertexAttribArray(val0);
        glVertexAttribPointer( val0,  3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
if(oglError) return;

        
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[1]);
        val1 = glGetAttribLocation(shader->getHandle(), "inNormal");
        glEnableVertexAttribArray(val1);
        glVertexAttribPointer( val1,  3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
if(oglError) return;
        /* 
        glBindBuffer(GL_ARRAY_BUFFER, m_vbo[2]);
        val2 = glGetAttribLocation(shader->getHandle(), "inTexCoord");
        glEnableVertexAttribArray(val2);
        glVertexAttribPointer( val2,  2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void*)0);
if(oglError) return;
        */
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ibo);
        
        glDrawElements( GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, (void*)0 );
if(oglError) return;
        
        //if(m_wireframe)
        //    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
	glDisableVertexAttribArray(val0);
	glDisableVertexAttribArray(val1);
	//glDisableVertexAttribArray(val2);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	shader->unbind();
        //glBindVertexArray(0);
    }
    
    
    Mesh* MeshUtils::cube(float size) {
        Mesh* cube = new Mesh();
        
        float size2 = size / 2.0f;
        
        float v[24*3] = {
            // Front
            -size2, -size2, size2,
            size2, -size2, size2,
            size2,  size2, size2,
            -size2,  size2, size2,
            // Right
            size2, -size2, size2,
            size2, -size2, -size2,
            size2,  size2, -size2,
            size2,  size2, size2,
            // Back
            -size2, -size2, -size2,
            -size2,  size2, -size2,
            size2,  size2, -size2,
            size2, -size2, -size2,
            // Left
            -size2, -size2, size2,
            -size2,  size2, size2,
            -size2,  size2, -size2,
            -size2, -size2, -size2,
            // Bottom
            -size2, -size2, size2,
            -size2, -size2, -size2,
            size2, -size2, -size2,
            size2, -size2, size2,
            // Top
            -size2,  size2, size2,
            size2,  size2, size2,
            size2,  size2, -size2,
            -size2,  size2, -size2
        };
        
        float n[24*3] = {
            // Front
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            // Right
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            // Back
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            0.0f, 0.0f, -1.0f,
            // Left
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            -1.0f, 0.0f, 0.0f,
            // Bottom
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            0.0f, -1.0f, 0.0f,
            // Top
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };
        
        float tex[24*2] = {
            // Front
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Right
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Back
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Left
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Bottom
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f,
            // Top
            0.0f, 0.0f,
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.0f, 1.0f
        };
        
        int el[] = {
            0,1,2,0,2,3,
            4,5,6,4,6,7,
            8,9,10,8,10,11,
            12,13,14,12,14,15,
            16,17,18,16,18,19,
            20,21,22,20,22,23
        };
        
        for(int i=0; i < 24; i++) {
            cube->vertices.push_back(glm::vec3(v[3*i], v[3*i+1], v[3*i+2]));
            cube->normals.push_back(glm::vec3(n[3*i], n[3*i+1], n[3*i+2]));
            cube->uvs.push_back(glm::vec2(tex[2*i], tex[2*i+1]));
        }
        
        cube->indices.assign(el, el + 36);
        
        return cube;
    }
    
    // SPHERE
    void generateSphereVerts(float * verts, float * norms, float * tex,
                             int * el, float radius, int slices, int stacks)
    {
        // Generate positions and normals
        GLfloat theta, phi;
        GLfloat thetaFac = (2.0 * PI) / slices;
        GLfloat phiFac = PI / stacks;
        GLfloat nx, ny, nz, s, t;
        GLuint idx = 0, tIdx = 0;
        for( int i = 0; i <= slices; i++ ) {
            theta = i * thetaFac;
            s = (GLfloat)i / slices;
            for( int j = 0; j <= stacks; j++ ) {
                phi = j * phiFac;
                t = (GLfloat)j / stacks;
                nx = sinf(phi) * cosf(theta);
                ny = sinf(phi) * sinf(theta);
                nz = cosf(phi);
                verts[idx] = radius * nx; verts[idx+1] = radius * ny; verts[idx+2] = radius * nz;
                norms[idx] = nx; norms[idx+1] = ny; norms[idx+2] = nz;
                idx += 3;
                
                tex[tIdx] = s;
                tex[tIdx+1] = t;
                tIdx += 2;
            }
        }
        
        // Generate the element list
        idx = 0;
        for( int i = 0; i < slices; i++ ) {
            GLuint stackStart = i * (stacks + 1);
            GLuint nextStackStart = (i+1) * (stacks+1);
            for( int j = 0; j < stacks; j++ ) {
                if( j == 0 ) {
                    el[idx] = stackStart;
                    el[idx+1] = stackStart + 1;
                    el[idx+2] = nextStackStart + 1;
                    idx += 3;
                } else if( j == stacks - 1) {
                    el[idx] = stackStart + j;
                    el[idx+1] = stackStart + j + 1;
                    el[idx+2] = nextStackStart + j;
                    idx += 3;
                } else {
                    el[idx] = stackStart + j;
                    el[idx+1] = stackStart + j + 1;
                    el[idx+2] = nextStackStart + j + 1;
                    el[idx+3] = nextStackStart + j;
                    el[idx+4] = stackStart + j;
                    el[idx+5] = nextStackStart + j + 1;
                    idx += 6;
                }
            }
        }
    }
    
    Mesh* MeshUtils::sphere(float radius, int slices, int stacks) {
        
        Mesh* sphere = new Mesh();
        
        int nVerts = (slices+1) * (stacks + 1);
        int elements = (slices * 2 * (stacks-1) ) * 3;
        
        // Verts
        float * v = new float[3 * nVerts];
        // Normals
        float * n = new float[3 * nVerts];
        // Tex coords
        float * tex = new float[2 * nVerts];
        // Elements
        int * el = new int[elements];
        
        // Generate the vertex data
        generateSphereVerts(v, n, tex, el, radius, slices, stacks);
        
        for(int i=0; i < nVerts; i++) {
            sphere->vertices.push_back(glm::vec3(v[3*i], v[3*i+1], v[3*i+2]));
            sphere->normals.push_back(glm::vec3(n[3*i], n[3*i+1], n[3*i+2]));
            sphere->uvs.push_back(glm::vec2(tex[2*i], tex[2*i+1]));
        }
        
        sphere->indices.assign(el, el + elements);
        
        return sphere;
    }
    
    
    //============= CYLINDER ===============
    void generateEndCapVertexData(  float radius, int slices,
                                    float y,
                                    int& index,
                                    int& elIndex,
                                    vector<float>& vertices,
                                    vector<float>& normals,
                                    vector<unsigned int>& indices ) {
        
        // Make a note of the vertex index for the center of the end cap
        int endCapStartIndex = index / 3;
        
        vertices[index]   = 0.0f;
        vertices[index+1] = 1.3*y;
        vertices[index+2] = 0.0f;
        
        normals[index]   = 0.0f;
        normals[index+1] = ( y >= 0.0f ) ? 1.0f : -1.0;
        normals[index+2] = 0.0f;
        
        index += 3;
        
        const float dTheta = PI2 / static_cast<float>( slices );
        for ( int slice = 0; slice < slices; ++slice )
        {
            const float theta = static_cast<float>( slice ) * dTheta;
            const float cosTheta = cosf( theta );
            const float sinTheta = sinf( theta );
            
            vertices[index]   = 1.3*radius * cosTheta;
            vertices[index+1] = y;
            vertices[index+2] = 1.3*radius * sinTheta;
            
            normals[index]   = -cosTheta;
            normals[index+1] = 0.5; //( y >= 0.0f ) ? 1.0f : -1.0;
            normals[index+2] = -sinTheta;
            
            index += 3;
        }
        
        for ( int i = 0; i < slices; ++i )
        {
            // Use center point and i+1, and i+2 vertices to create triangles
            if ( i != slices - 1 )
            {
                indices[elIndex]   = endCapStartIndex;
                indices[elIndex+1] = endCapStartIndex + i + 1;
                indices[elIndex+2] = endCapStartIndex + i + 2;
            }
            else
            {
                indices[elIndex]   = endCapStartIndex;
                indices[elIndex+1] = endCapStartIndex + i + 1;
                indices[elIndex+2] = endCapStartIndex + 1;
            }
            
            elIndex += 3;
        }
    }
    
    
    Mesh* MeshUtils::cylinder(float radius, float length, int rings, int slices ) {
        Mesh* cylinder = new Mesh();
        
        int faces = slices * rings;
        int nVerts  = ( slices + 1 ) * rings        // Sides
                        + 1 * ( slices + 1 );       // 1 x endcap
        
        vector<float> vertices;
        vector<float> normals;
        vector<unsigned int> indices;
        
        vertices.resize( 3 * nVerts );
        normals.resize( 3 * nVerts );
        indices.resize( 6 * faces + 1*slices );
        
        // The side of the cylinder
        const float dTheta = PI2 / static_cast<float>( slices );
        const float dy = length / static_cast<float>( rings - 1 );
        
        // Iterate over heights (rings)
        int index = 0;
        
        for ( int ring = 0; ring < rings; ++ring ) {
            
            const float y = -length / 2.0f + static_cast<float>( ring ) * dy;
            
            // Iterate over slices (segments in a ring)
            for ( int slice = 0; slice < slices + 1; ++slice ) {
                
                const float theta = static_cast<float>( slice ) * dTheta;
                const float cosTheta = cosf( theta );
                const float sinTheta = sinf( theta );
                
                vertices[index]   = radius * cosTheta;
                vertices[index+1] = y;
                vertices[index+2] = radius * sinTheta;
                
                normals[index]   = -cosTheta;
                normals[index+1] = 0.0f;
                normals[index+2] = -sinTheta;
                
                index += 3;
            }
        }
        
        int elIndex = 0;
        for ( int i = 0; i < rings - 1; ++i ) {
            
            const int ringStartIndex = i * ( slices + 1 );
            const int nextRingStartIndex = ( i + 1 ) * ( slices + 1 );
            
            for ( int j = 0; j < slices; ++j ) {
                
                // Split the quad into two triangles
                indices[elIndex]   = ringStartIndex + j;
                indices[elIndex+1] = ringStartIndex + j + 1;
                indices[elIndex+2] = nextRingStartIndex + j;
                indices[elIndex+3] = nextRingStartIndex + j;
                indices[elIndex+4] = ringStartIndex + j + 1;
                indices[elIndex+5] = nextRingStartIndex + j + 1;
                
                elIndex += 6;
            }
        }
        
        generateEndCapVertexData(radius, slices,
                                 length / 2.0f, index, elIndex,
                                 vertices, normals, indices );

        for(int i=0; i < nVerts; i++) {
            cylinder->vertices.push_back(glm::vec3(vertices[3*i], vertices[3*i+1], vertices[3*i+2]));
            cylinder->normals.push_back(glm::vec3(normals[3*i], normals[3*i+1], normals[3*i+2]));
            cylinder->uvs.push_back(glm::vec2(0, 0));
        }
        cylinder->indices = indices;
        
        return cylinder;
    }
    
    
}; //namespace vectorfield
