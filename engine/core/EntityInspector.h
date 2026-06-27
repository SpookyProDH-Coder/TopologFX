 /******* EntityInspector.h ***************************************************/ /**
 *
 * @file EntityInspector.h
 *
 * User interface entity inspector handler
 *  
 * @version 1
 * @author SpookyProDH-Coder
 * @date 21/06/2026
 ***************************************************************************/

#ifndef _H_ENTITY_INSPECTOR
#define _H_ENTITY_INSPECTOR

#include <imgui.h>
#include "Registry.h"
#include "../../topology/MetricSpace.h"

class Registry;
enum class MetricSpaceType : unsigned;

class EntityInspector 
{
    public:
        EntityInspector(Registry*);
        ~EntityInspector() = default;

        /**
         * @brief Draw the main inspector window
         * @param selected_entity_id The selected entity id
         */
        void render(unsigned);

    private:
        Registry* m_registry;

        /** Modular pannels */
        void drawTransformPanel(unsigned);
};

#endif