 /******* EntityInspector.cpp ***************************************************/ /**
 *
 * @file EntityInspector.cpp
 *
 * User interface entity inspector handler implementation
 *
 * @version 1
 * @author SpookyProDH-Coder
 * @date 21/06/2026
 ***************************************************************************/

#include "EntityInspector.h"
#include "Registry.h"
#include "../../topology/MetricSpace.h"

using namespace std;

EntityInspector::EntityInspector(Registry* registry) 
: m_registry(registry) {}

void EntityInspector::render(unsigned selected_entity_id)
{
    ImGui::Begin("Topological Inspector");

    Entity dummy_ent;
    if (!m_registry->entities.search(selected_entity_id, dummy_ent))
    {
        ImGui::TextDisabled("[!] Invalid entity selected.");
        ImGui::End();
        return;
    }

    ImGui::PushID(selected_entity_id);

    if (ImGui::CollapsingHeader("Affine Transformation", ImGuiTreeNodeFlags_DefaultOpen))
        drawTransformPanel(selected_entity_id);

    ParametricSurface *surface = m_registry->parametricSurfaces.getPointer(selected_entity_id);
    if (surface != nullptr)
    {
        if (ImGui::CollapsingHeader("Combinatorial Surface", ImGuiTreeNodeFlags_DefaultOpen))
        {
            int segments = surface->u_subdivisions;

            const char* surfaces[] = { "Torus (aba^-1b^-1)", "Klein Bottle (abab^-1)", "Möbius Strip (abcb)", "Projective Plane (abab)", "Sphere (abb^-1a^-1)" };
            int current_surf = static_cast<int>(surface->type);
            
            if (ImGui::Combo("Word Policy", &current_surf, surfaces, 5)) 
            {
                surface->type = static_cast<SurfaceType>(current_surf);
                surface->needs_rebuild = true;
            }

            ImGui::Text("Fundamental Polygon Grid");

            if (ImGui::SliderInt("Subdivisions", &segments, 5, 1000))
            {
                surface->u_subdivisions = segments;
                surface->v_subdivisions = segments;
            }

            if (ImGui::IsItemDeactivatedAfterEdit())
                surface->needs_rebuild = true;

            ImGui::Separator();
            ImGui::Text("Visualization Options");
            if (ImGui::Checkbox("Render Topological Heatmap (p-norm)", &surface->use_heatmap))
                surface->needs_rebuild = true;

            ImGui::Separator();
            ImGui::TextDisabled("Metric: Intrinsic Geodesic (Dijkstra L_p)");
            ImGui::TextDisabled("Embedding: Flat Quotient Projection");
        }
    }

    ImGui::PopID();
    ImGui::End();
}

void EntityInspector::drawTransformPanel(unsigned entity_id)
{
    Entity *ent = m_registry->entities.getPointer(entity_id);

    if (ent != nullptr)
    {
        bool changed = false;

        changed |= ImGui::DragFloat3("Position (x,y,z)", &ent->position.x, 0.05f);

        changed |= ImGui::DragFloat4("Rotation (x,y,z,w)", &ent->rotation.x, 0.05f);
        
        changed |= ImGui::DragFloat3("Scale (x,y,z)", &ent->scale.x, 0.05f);
    }
    else
        ImGui::TextDisabled("Entity transform data unavailable.");
}