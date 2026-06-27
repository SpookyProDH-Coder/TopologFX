 /******* SceneManager.h ***************************************************/ /**
 *
 * @file SceneManager.h
 *
 * Scene manager class
 * 
 * @version 1
 * @author SpookyProDH-Coder
 * @date 21/06/2026
 ***************************************************************************/

#ifndef _H_SCENE_MANAGER
#define _H_SCENE_MANAGER

#include "Registry.h"
#include "../render/Renderer.h"
#include <bgfx/bgfx.h>
#include <string>
#include <vector>
#include <memory>
#include <future>
#include <chrono>

using namespace std;

class SceneManager 
{
    public:
        SceneManager(Renderer*);
        ~SceneManager();
        
        /* Scene manager functions */
        void loadScene();
        void initializeTopologicScene();
        void clear();

        /* Explicit getter */
        const Registry& getRegistry() const;
        
        unsigned addEntity(const Entity&);
        void renderEntity(unsigned);

        /* Setters */
        void setEntityPosition(unsigned, const bx::Vec3&);
        void setEntityScale(unsigned, const bx::Vec3&);
        void setEntityRotation(unsigned, const bx::Quaternion&);
        void setMetricParamP(unsigned, float);

        /* Tactical auxiliar functions */
        void updateTransforms();
        void updateTopology();

    private:
        void mxSRT_Update(unsigned, Entity&);
        bgfx::ShaderHandle loadShader(const char*);
        bgfx::ProgramHandle loadProgram(const char*, const char*);

        Renderer* m_renderer;
        
        /* Dynamic entity controller */
        vector<unsigned> m_activeEntities;
        unsigned m_nextEntityId = 0;
};

#endif