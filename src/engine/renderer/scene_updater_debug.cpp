#include "engine/core.hpp"
#include "scene_updater.hpp"
#include "engine/log.hpp"

#include <fmt/format.h>
#include <imgui.h>
#include "imgui_extensions.hpp"

#include "mesh.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "engine/physics/physics_components.hpp"
#include "ui_components.hpp"

namespace render {

    using namespace ::physics;

    size_t s_sceneTreeCounter = 0;

    void ImGui_DrawTextureDebug(gpu::ITexture* tex) {
        if (tex) {
            bool isMipmapEnabled = tex->getDesc().generateMipmaps;
            gpu::TextureType textureType = tex->getDesc().type;
            ImGui::PushID(tex->getNativeObject());
            ImGui::Text(fmt::format("{} x {}", tex->getDesc().width, tex->getDesc().height).c_str());
            ImGui::Image((ImTextureID)(intptr_t)tex->getNativeObject(), ImVec2(64, 64), ImVec2(0, 1), ImVec2(1, 0));
            ImGui::BeginDisabled();
            ImGui::Checkbox("Mipmaps", &isMipmapEnabled);
            const char* textureTypeNames[] = { "Texture2D", "Texture3D", "TextureArray2D", "TextureCubeMap" };
            ImGui::ComboboxEx("Texture Type", (int*)&textureType, textureTypeNames, IM_ARRAYSIZE(textureTypeNames));
            ImGui::EndDisabled();
            ImGui::PopID();
        }
    }

    void SceneUpdater::drawDebugSceneGraphEntity(const std::string& sceneName, const std::shared_ptr<Entity> entity, void** pSelectedEntity) {
#if _DEBUG
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
#endif
    }

    void SceneUpdater::drawDebugSceneGraph(const Scene& scene, void** pSelectedEntity) {
#if _DEBUG
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
#endif
    }

    // Current state of the inspector GUI
    struct InspectorState_t {
        const char* currentLightDropDown;
    };

    void SceneUpdater::drawDebugInspector(const Scene& scene, void** pSelectedEntity) {
#if _DEBUG
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
            bool markDirty = ImGui::DragFloat3("Position", pEntity->transform.m_position.f32, 0.01f);
            markDirty |= ImGui::DragFloat4("Rotation", pEntity->transform.m_rotation.f32, 0.01f);
            markDirty |= ImGui::DragFloat3("Scale", pEntity->transform.m_scale.f32, 0.01f);
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
                case ComponentType::Physics:
                {
                    ImGui::BeginGroupPanel("Physics Component", ImVec2(groupWidth, 0));
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
                    ImGui::BeginGroupPanel(fmt::format("Material - {}", pMeshRenderer->material.name).c_str(), ImVec2(groupWidth - 2 * ImGui::GetStyle().ItemSpacing.x, 0));

                    {
                        // Graphics / rasteriser state (depth + cull state)
                        ImGui::BeginGroupPanel("Graphics State", ImVec2(groupWidth - 4 * ImGui::GetStyle().ItemSpacing.x, 0));
                        ImGui::BeginDisabled();

                        gpu::GraphicsState shaderState = pMeshRenderer->material.shader->getDesc().graphicsState;

                        const char* compareFuncNames[] = { "Never", "Less", "Equal", "LessOrEqual", "Greater", "NotEqual", "GreaterOrEqual", "Always" };
                        const char* faceCullModeNames[] = { "Back", "Front", "Both", "Never" };
                        const char* windingOrderNames[] = { "Clockwise", "CounterClockwise" };
                        ImGui::ComboboxEx("Depth State", (int*)&shaderState.depthState, compareFuncNames, IM_ARRAYSIZE(compareFuncNames));
                        ImGui::Checkbox("Depth Write", &shaderState.depthWrite);
                        ImGui::Checkbox("Depth Test", &shaderState.depthTest);
                        ImGui::ComboboxEx("Cull Mode", (int*)&shaderState.faceCullingMode, faceCullModeNames, IM_ARRAYSIZE(faceCullModeNames));
                        ImGui::ComboboxEx("Winding Order", (int*)&shaderState.faceWindingOrder, windingOrderNames, IM_ARRAYSIZE(windingOrderNames));

                        ImGui::EndDisabled();
                        ImGui::EndGroupPanel();
                        ImGui::NewLine();
                    }

                    ImGui::Text("Shader %s", pMeshRenderer->material.shader->getDesc().debugName.c_str());
                    ImGui::ColorEdit3("Ambient", pMeshRenderer->material.ambient.f32);
                    ImGui::ColorEdit3("Diffuse", pMeshRenderer->material.diffuse.f32);
                    ImGui_DrawTextureDebug(pMeshRenderer->material.diffuseTex);
                    ImGui::ColorEdit3("Specular", pMeshRenderer->material.specular.f32);
                    ImGui::ColorEdit3("Emission", pMeshRenderer->material.emissionColour.f32);
                    ImGui_DrawTextureDebug(pMeshRenderer->material.emissionTex);
                    ImGui::DragFloat("Intensity", &pMeshRenderer->material.emissionIntensity, 0.1f, 0, 10.0f);
                    ImGui::DragFloat("Metallic", &pMeshRenderer->material.metallic, 0.01f, 0, 1.0f);
                    ImGui::DragFloat("Roughness", &pMeshRenderer->material.roughness, 0.01f, 0, 1.0f);
                    ImGui::Text("Meta (R: metal G: rough)");
                    ImGui_DrawTextureDebug(pMeshRenderer->material.metaTex);
                    ImGui::Text("Matcap");
                    ImGui_DrawTextureDebug(pMeshRenderer->material.matcapTex);
                    ImGui::Text("BRDF LUT");
                    ImGui_DrawTextureDebug(pMeshRenderer->material.brdfLutTex);
                    ImGui::EndGroupPanel();

                    break;

                }
                case ComponentType::Light:
                {
                    Light* pLight = (Light*)component.get();

                    const char* lightTypeNames[] = { "Directional", "Point", "Spot" };
                    ImGui::ComboboxEx("Type", (int*)&pLight->type, lightTypeNames, IM_ARRAYSIZE(lightTypeNames));

                    ImGui::BeginDisabled();
                    hlslpp::float3 lightTmp = pLight->getPosition();
                    ImGui::DragFloat3("Position", lightTmp.f32);
                    lightTmp = pLight->getDirection();
                    ImGui::DragFloat3("Direction", lightTmp.f32);
                    ImGui::EndDisabled();
                    
                    ImGui::ColorEdit3("Colour", pLight->colour.f32);
                    ImGui::DragFloat("Intensity", &pLight->intensity, 0.01f, 0);
                    ImGui::DragFloat("Inner Radius", &pLight->innerRadius, 0.01f, 0);
                    ImGui::DragFloat("Outer Radius", &pLight->outerRadius, 0.01f, 0);

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

                    bool perspectiveDirty = ImGui::DragFloat("FOV", &pCamera->m_fov, 0.01f, 0.1f, 179.9f);
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
                    break;
                }
                case ComponentType::Physics:
                {
                    PhysicsComponent* pPhysics = (PhysicsComponent*)component.get();

                    const char* physicsBodyTypeNames[] = { "Static", "Kinematic", "Rigidbody" };
                    const char* physicsShapeNames[] = { "Box", "Circle", "Capsule" };

                    ImGui::DragFloat("Density", &pPhysics->density, 0.01f, 0);
                    ImGui::DragFloat("Friction", &pPhysics->friction, 0.01f, 0);
                    ImGui::DragFloat("Bounciness", &pPhysics->bounciness, 0.01f, 0);
                    ImGui::DragFloat("Gravity Scale", &pPhysics->gravityScale, 0.01f);
                    ImGui::ComboboxEx("Body Type", (int*)&pPhysics->bodyType, physicsBodyTypeNames, IM_ARRAYSIZE(physicsBodyTypeNames));

                    ImGui::ComboboxEx("Collider Shape", (int*)&pPhysics->shape.shape, physicsShapeNames, IM_ARRAYSIZE(physicsShapeNames));
                    switch (pPhysics->shape.shape) {
                    case PhysicsShape::Box: {
                        ImGui::DragFloat2("Size", pPhysics->shape.box.size.f32, 0.01f, 0);
                        break;
                    }
                    case PhysicsShape::Circle: {
                        ImGui::DragFloat2("Centre", pPhysics->shape.circle.centre.f32, 0.01f, 0);
                        ImGui::DragFloat("Radius", &pPhysics->shape.circle.radius, 0.01f, 0);
                        
                        break;
                    }
                    case PhysicsShape::Capsule: {
                        ImGui::DragFloat2("Point 1", pPhysics->shape.capsule.p1.f32, 0.01f, 0);
                        ImGui::DragFloat2("Point 2", pPhysics->shape.capsule.p2.f32, 0.01f, 0);
                        ImGui::DragFloat("Radius", &pPhysics->shape.capsule.radius, 0.01f, 0);
                        break;
                    }
                    }

                    b2Vec2 velocity = b2Body_GetLinearVelocity(pPhysics->m_physicsId);
                    ImGui::BeginDisabled();
                    ImGui::DragFloat2("Velocity", &velocity.x);
                    ImGui::EndDisabled();

                    break;
                }
                case ComponentType::UIElement:
                {
                    UIElement* pUiElement = (UIElement*)component.get();

                    const char* uiTypeNames[] = { "Text", "Sprite" };
                    ImGui::ComboboxEx("Type", (int*)&pUiElement->uiType, uiTypeNames, IM_ARRAYSIZE(uiTypeNames));

                    ImGui::DragFloat2("Position", &pUiElement->posX, 0.01f);
                    
                    switch (pUiElement->uiType) {
                    case UIElementType::Text: {

                        pUiElement->text.resize(pUiElement->text.length() + 200);
                        ImGui::InputTextMultiline("Text", pUiElement->text.data(), pUiElement->text.size());
                        ImGui::DragFloat("Text Scale", &pUiElement->textScale, 0.01f, 0);
                        ImGui::ColorEdit4("Text Colour", pUiElement->textColour.f32);
                        ImGui::DragFloat("Outline Width", &pUiElement->outlineWidth, 0.01f, 0, 1.0f);
                        if (pUiElement->outlineWidth == 0) {
                            ImGui::BeginDisabled();
                        }
                        ImGui::ColorEdit4("Outline Colour", pUiElement->outlineColour.f32);
                        if (pUiElement->outlineWidth == 0) {
                            ImGui::EndDisabled();
                        }
                        break;
                    }
                    case UIElementType::Sprite: {
                        ImGui::DragFloat2("Size", &pUiElement->sizeX, 0.01f);
                        ImGui::ColorEdit4("Texture Tint", pUiElement->textureTint.f32);
                        ImGui::Text("Texture");
                        ImGui_DrawTextureDebug(pUiElement->texture);
                        break;
                    }
                    default:
                        ImGui::BeginDisabled();
                        ImGui::Text("Unknown UI Type");
                        ImGui::EndDisabled();
                        break;
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
#endif
    }

    void SceneUpdater::drawPhysicsDebug(const Scene& scene) {
#if _DEBUG
        if (scene.physicsParams.m_initialised) {
            /*
            IComponent* camera = scene.root.findComponent(ComponentType::Camera);
            g_camera.m_center.x = camera->getEntity()->transform.getPosition().x;
            g_camera.m_center.y = camera->getEntity()->transform.getPosition().y;
            g_camera.m_zoom = 1;
            b2World_Draw(scene.physicsParams.m_box2Dworld, &g_draw.m_debugDraw);
            g_draw.Flush();
            */
        }
#endif
    }
}