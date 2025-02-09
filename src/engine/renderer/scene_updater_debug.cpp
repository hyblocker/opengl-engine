#include "engine/core.hpp"
#include "scene_updater.hpp"
#include "engine/log.hpp"

#include <fmt/format.h>
#include <imgui.h>
#include "imgui_extensions.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "light.hpp"

namespace render {

    size_t s_sceneTreeCounter = 0;

    void SceneUpdater::drawDebugSceneGraphEntity(const std::string& sceneName, const std::shared_ptr<Entity> entity, void** pSelectedEntity) {
        ASSERT(pSelectedEntity != nullptr);
        ImGuiTreeNodeFlags entityFlags = ImGuiTreeNodeFlags_DefaultOpen;
        if (entity->children.empty()) {
            entityFlags = entityFlags | ImGuiTreeNodeFlags_Leaf;
        }
        if (entity.get() == *pSelectedEntity) {
            entityFlags = entityFlags | ImGuiTreeNodeFlags_Selected;
        }
        ImGui::PushID(fmt::format("SceneHierarchy_{}_{}", sceneName, s_sceneTreeCounter).c_str());
        std::string displayName = entity->name;
        if (entity->name.empty()) {
            displayName = fmt::format("<Unnamed entity {}>", s_sceneTreeCounter);
        }
        if (ImGui::TreeNodeEx(displayName.c_str(), entityFlags)) {

            if (ImGui::IsItemClicked()) {
                *pSelectedEntity = const_cast<Entity*>(entity.get());
            }

            for (const std::shared_ptr<render::Entity> childEntity : entity->children) {
                s_sceneTreeCounter++;
                drawDebugSceneGraphEntity(sceneName, childEntity, pSelectedEntity);
            }

            ImGui::TreePop();
        }
        ImGui::PopID();
    }

    void SceneUpdater::drawDebugSceneGraph(const Scene& scene, void** pSelectedEntity) {
        ASSERT(pSelectedEntity != nullptr);

        ImGuiTreeNodeFlags sceneFlags = ImGuiTreeNodeFlags_DefaultOpen;
        if (scene.root.children.empty()) {
            sceneFlags = sceneFlags | ImGuiTreeNodeFlags_Leaf;
        }
        if (&scene == *pSelectedEntity) {
            sceneFlags = sceneFlags | ImGuiTreeNodeFlags_Selected;
        }
        const char* displayName = scene.sceneName.c_str();
        if (scene.sceneName.empty()) {
            displayName = "<Unnamed scene>";
        }
        ImGui::PushID(fmt::format("SceneHierarchy_{}_{}", scene.sceneName, s_sceneTreeCounter).c_str());
        if (ImGui::TreeNodeEx(displayName, sceneFlags)) {

            if (ImGui::IsItemClicked()) {
                *pSelectedEntity = const_cast<Scene*>(&scene);
            }

            for (const std::shared_ptr<render::Entity> entity : scene.root.children) {
                s_sceneTreeCounter++;
                drawDebugSceneGraphEntity(scene.sceneName, entity, pSelectedEntity);
            }

            ImGui::TreePop();
        }

        // Reset counter
        s_sceneTreeCounter = 0;
        ImGui::PopID();
    }

    // Current state of the inspector GUI
    struct InspectorState_t {
        const char* currentLightDropDown;
    };

    void SceneUpdater::drawDebugInspector(const Scene& scene, void** pSelectedEntity) {
        ASSERT(pSelectedEntity != nullptr);

        if (*pSelectedEntity == nullptr) {
            // Nothing is selected
            return;
        }

        float groupWidth = ImGui::GetWindowSize().x - ImGui::GetStyle().ItemSpacing.x * 2;

        if (&scene == *pSelectedEntity) {
            // is scene
            Scene* pScene = (Scene*) (*pSelectedEntity);
            
            // Name
            char textBuffer[256] = {};
            memcpy(textBuffer, pScene->sceneName.data(), pScene->sceneName.size());
            ImGui::InputTextWithHint("Name", "Scene name", textBuffer, 256);
            pScene->sceneName = textBuffer;

            // Skybox props
            ImGui::BeginGroupPanel("Skybox", ImVec2(groupWidth, 0));

            ImGui::EndGroupPanel();

        } else {
            // is entity
            Entity* pEntity = (Entity*) (*pSelectedEntity);
            
            ImGui::Checkbox("Enabled", &pEntity->enabled);

            // Name
            char textBuffer[256] = {};
            memcpy(textBuffer, pEntity->name.data(), pEntity->name.size());
            ImGui::InputTextWithHint("Name", "Entity name", textBuffer, 256);
            pEntity->name = textBuffer;

            // Draw transform component
            ImGui::BeginGroupPanel("Transform", ImVec2(groupWidth, 0));
            bool markDirty = ImGui::DragFloat3("Position", pEntity->transform.m_position.f32, 0.05f);
            markDirty |= ImGui::DragFloat4("Rotation", pEntity->transform.m_rotation.f32, 0.05f);
            markDirty |= ImGui::DragFloat3("Scale", pEntity->transform.m_scale.f32, 0.05f);
            pEntity->transform.m_isDirty = pEntity->transform.m_isDirty || markDirty;
            ImGui::EndGroupPanel();

            // Draw components
            size_t component_iter = 0;
            for (const std::shared_ptr<IComponent>& component : pEntity->components) {

                ComponentType componentType = component->getComponentType();

                ImGui::PushID(fmt::format("{}_{}_{}", pEntity->name, (int)component->getComponentType(), component_iter).c_str());
                switch (componentType) {
                case ComponentType::MeshRenderer:
                {
                    ImGui::BeginGroupPanel("Mesh Renderer", ImVec2(groupWidth, 0));
                    break;
                }
                case ComponentType::Light:
                {
                    ImGui::BeginGroupPanel("Light", ImVec2(groupWidth, 0));
                    break;
                }
                case ComponentType::ParticleSystem:
                {
                    ImGui::BeginGroupPanel("Particle System", ImVec2(groupWidth, 0));
                    break;
                }
                case ComponentType::Camera:
                {
                    ImGui::BeginGroupPanel("Camera", ImVec2(groupWidth, 0));
                    break;
                }
                case ComponentType::UserBehaviour:
                {
                    ImGui::BeginGroupPanel("User Behaviour", ImVec2(groupWidth, 0));
                    break;
                }
                case ComponentType::UICanvas:
                {
                    ImGui::BeginGroupPanel("UI Canvas", ImVec2(groupWidth, 0));
                    break;
                }
                case ComponentType::UIElement:
                {
                    ImGui::BeginGroupPanel("UI Element", ImVec2(groupWidth, 0));
                    break;
                }
                default:
                    ImGui::BeginGroupPanel("<UNKNOWN-TYPE>", ImVec2(groupWidth, 0));
                    break;
                }
                ImGui::Checkbox("Enabled", const_cast<bool*>(&component->enabled));

                switch (componentType) {
                case ComponentType::MeshRenderer:
                {
                    MeshRenderer* pMeshRenderer = (MeshRenderer*)component.get();
                    if (pMeshRenderer->mesh.vertexBuffer) {
                        ImGui::Text(fmt::format("Vertex buffer: {}", pMeshRenderer->mesh.vertexBuffer->getDesc().debugName).c_str());
                    }
                    if (pMeshRenderer->mesh.indexBuffer) {
                        ImGui::Text(fmt::format("Vertex Index buffer: {}", pMeshRenderer->mesh.indexBuffer->getDesc().debugName).c_str());
                    }
                    ImGui::Text(fmt::format("{} Triangles", pMeshRenderer->mesh.triangleCount).c_str());
                    ImGui::BeginGroupPanel("Material", ImVec2(groupWidth - 2 * ImGui::GetStyle().ItemSpacing.x, 0));
                    ImGui::ColorEdit3("Ambient", pMeshRenderer->material.ambient.f32);
                    ImGui::ColorEdit3("Diffuse", pMeshRenderer->material.diffuse.f32);
                    if (pMeshRenderer->material.diffuseTex) {
                        ImGui::Image((ImTextureID)(intptr_t)pMeshRenderer->material.diffuseTex->getNativeObject(), ImVec2(64, 64));
                    }
                    ImGui::ColorEdit3("Specular", pMeshRenderer->material.specular.f32);
                    if (pMeshRenderer->material.specularTex) {
                        ImGui::Image((ImTextureID)(intptr_t)pMeshRenderer->material.specularTex->getNativeObject(), ImVec2(64, 64));
                    }
                    ImGui::ColorEdit3("Emission", pMeshRenderer->material.emissionColour.f32);
                    if (pMeshRenderer->material.emissionTex) {
                        ImGui::Image((ImTextureID)(intptr_t)pMeshRenderer->material.emissionTex->getNativeObject(), ImVec2(64, 64));
                    }
                    ImGui::DragFloat("Intensity", &pMeshRenderer->material.emissionIntensity, 0.1f, 0, 10.0f);
                    ImGui::EndGroupPanel();
                    break;
                }
                case ComponentType::Light:
                {
                    Light* pLight = (Light*)component.get();

                    const char* lightTypeNames[] = { "Directional", "Point", "Spot" };
                    const char* lightAttenuationTypeNames[] = { "None", "Linear", "Quad" };

                    ImGui::ComboboxEx("Type", (int*)&pLight->type, lightTypeNames, IM_ARRAYSIZE(lightTypeNames));
                    ImGui::ComboboxEx("Attenuation", (int*)&pLight->attenuation, lightAttenuationTypeNames, IM_ARRAYSIZE(lightAttenuationTypeNames));

                    ImGui::DragFloat3("Position", pLight->position.f32);
                    ImGui::DragFloat3("Direction", pLight->direction.f32);
                    
                    ImGui::ColorEdit3("Colour", pLight->colour.f32);
                    ImGui::DragFloat("Intensity", &pLight->intensity);
                    ImGui::DragFloat("Radius", &pLight->radius);

                    break;
                }
                case ComponentType::Camera:
                {
                    Camera* pCamera = (Camera*)component.get();

                    const char* cameraProjectionTypes[] = { "Perspective", "Orthographic" };
                    auto oldProj = pCamera->m_projection;
                    ImGui::ComboboxEx("Projection", (int*)&pCamera->m_projection, cameraProjectionTypes, IM_ARRAYSIZE(cameraProjectionTypes));
                    if (pCamera->m_projection != oldProj) {
                        pCamera->m_isPerspectiveDirty = true;
                    }

                    bool perspectiveDirty = ImGui::DragFloat("FOV", &pCamera->m_fov, 0.01f, 20.0f, 179.9f);
                    perspectiveDirty |= ImGui::DragFloat("Near plane", &pCamera->m_nearPlane, 0.01f, 0.00000001f, 999999.9f);
                    if (pCamera->m_infiniteFar) {
                        ImGui::BeginDisabled();
                    }
                    perspectiveDirty |= ImGui::DragFloat("Far plane", &pCamera->m_farPlane, 0.01f, 0.00000001f, 999999.9f);
                    if (pCamera->m_infiniteFar) {
                        ImGui::EndDisabled();
                    }
                    perspectiveDirty |= ImGui::Checkbox("Use infinite far plane", &pCamera->m_infiniteFar);
                    perspectiveDirty |= ImGui::DragFloat("Aspect ratio", &pCamera->m_aspect);

                    pCamera->m_isPerspectiveDirty = pCamera->m_isPerspectiveDirty || perspectiveDirty;
                    pCamera->m_isViewDirty = pCamera->m_isViewDirty || markDirty;
                    }
                    break;
                }
                default:
                    break;
                }

                ImGui::EndGroupPanel();
                ImGui::PopID();

                component_iter++;
            }

        }
    }
}