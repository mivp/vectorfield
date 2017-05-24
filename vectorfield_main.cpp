#include <omega.h>
#include <omegaGl.h>
#include <iostream>
#include <vector>

#include "VectorField.h"

using namespace std;
using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class VectorFieldRenderModule : public EngineModule
{
public:
    VectorFieldRenderModule() :
        EngineModule("VectorFieldRenderModule"), visible(true)
    {
        vectorfield = new vectorfield::VectorField();
    }

    virtual void initializeRenderer(Renderer* r);

    virtual void update(const UpdateContext& context)
    {
        // After a frame all render passes had a chance to update their
        // textures. reset the raster update flag.       
    }
    
    virtual void dispose()
    {
        if(vectorfield)
            delete vectorfield;
    }

    void init(  const float min_x, const float min_z, 
                const float max_x, const float max_z, 
                const float cell_size, const float height )
    {
        if(vectorfield)
            vectorfield->init(min_x, min_z, max_x, max_z, cell_size, height);
    } 

    void addControlPoint(const float px, const float pz,
                         const float vx, const float vz)
    {
        if(vectorfield)
            vectorfield->addControlPoint(px, pz, vx, vz);
    }

    void updateVectorField()
    {
        if(vectorfield)
            vectorfield->update();
    }

    void setVisible(bool v)
    {
        visible = v;
    }

    void toggleVisible() 
    {
        visible = !visible;
    }

    void setPointScale(float ps)
    {
        if(vectorfield)
            vectorfield->setPointScale(ps);
    }

    void setArrowScale(float as)
    {
        if(vectorfield)
            vectorfield->setArrowScale(as);
    }

    void nextParticleType()
    {
        if(vectorfield->getParticleType() == TYPE_POINT)
            vectorfield->setParticleType(TYPE_ARROW);
        else
            vectorfield->setParticleType(TYPE_POINT);
    }

    vectorfield::VectorField* vectorfield;
    bool visible;
};

///////////////////////////////////////////////////////////////////////////////
class VectorFieldRenderPass : public RenderPass
{
public:
    VectorFieldRenderPass(Renderer* client, VectorFieldRenderModule* prm) : 
        RenderPass(client, "VectorFieldRenderPass"), 
        module(prm) {}
    
    virtual void initialize()
    {
        RenderPass::initialize();
    }

    virtual void render(Renderer* client, const DrawContext& context)
    {
    	if(context.task == DrawContext::SceneDrawTask)
        {
            glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT | GL_CURRENT_BIT);
            client->getRenderer()->beginDraw3D(context);
	
    	    if(module->visible)
    	    { 
                float* MV = context.modelview.cast<float>().data();
                float* P = context.projection.cast<float>().data();

                module->vectorfield->render(MV, P);

                if(oglError) return;
    	    }
            
            client->getRenderer()->endDraw();
            glPopAttrib();
        }
        
    }

private:
    VectorFieldRenderModule* module;

};

///////////////////////////////////////////////////////////////////////////////
void VectorFieldRenderModule::initializeRenderer(Renderer* r)
{
    r->addRenderPass(new VectorFieldRenderPass(r, this));
}

///////////////////////////////////////////////////////////////////////////////
VectorFieldRenderModule* initialize()
{
    VectorFieldRenderModule* prm = new VectorFieldRenderModule();
    ModuleServices::addModule(prm);
    prm->doInitialize(Engine::instance());
    return prm;
}

///////////////////////////////////////////////////////////////////////////////
// Python API
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(vectorfield)
{
    //
    PYAPI_REF_BASE_CLASS(VectorFieldRenderModule)
    PYAPI_METHOD(VectorFieldRenderModule, init)
    PYAPI_METHOD(VectorFieldRenderModule, addControlPoint)
    PYAPI_METHOD(VectorFieldRenderModule, updateVectorField)
    PYAPI_METHOD(VectorFieldRenderModule, setVisible)
    PYAPI_METHOD(VectorFieldRenderModule, toggleVisible)
    PYAPI_METHOD(VectorFieldRenderModule, setPointScale)
    PYAPI_METHOD(VectorFieldRenderModule, setArrowScale)
    PYAPI_METHOD(VectorFieldRenderModule, nextParticleType)
    ;

    def("initialize", initialize, PYAPI_RETURN_REF);
}
