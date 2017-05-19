#include "Material.h"
#include <iostream>

namespace vectorfield {
    
    Material::Material(): m_shader(0) {
        m_shaderDir = SHADER_DIR;
        //std::cout << "Shader dir: " << SHADER_DIR << std::endl;
        
        Ka = glm::vec3( 0.1f, 0.1f, 0.1f );
        Kd = glm::vec3( 1.0f, 1.0f, 1.0f );
        Ks = glm::vec3( 0.3f, 0.3f, 0.3f );
        shininess = 1.0;
        fogEnabled = true;
    }
    
    Material::~Material() {
        if(m_shader)
            delete m_shader;
    }
    
    /**
     */
    MeshMaterial::MeshMaterial(): Material() {
        m_shader = new GLSLProgram();
        string filename;
        filename = m_shaderDir; filename.append("mesh.vert");
        m_shader->compileShader(filename.c_str());
        filename = m_shaderDir; filename.append("mesh.frag");
        m_shader->compileShader(filename.c_str());
        m_shader->link();
    }
    
    MeshMaterial::~MeshMaterial() {
        
    }
    
    /**
     */
    PointMaterial::PointMaterial(): Material() {
        m_shader = new GLSLProgram();
        string filename;
        filename = m_shaderDir; filename.append("point.vert");
        m_shader->compileShader(filename.c_str());
        filename = m_shaderDir; filename.append("point.frag");
        m_shader->compileShader(filename.c_str());
        m_shader->link();
    }
    
    PointMaterial::~PointMaterial() {
        
    }
    
}; //namespace vectorfield
