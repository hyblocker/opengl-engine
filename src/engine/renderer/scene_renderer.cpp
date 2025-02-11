#include "scene_renderer.hpp"
#include "engine/log.hpp"
#include "engine/renderer/light.hpp"
#include "engine/renderer/mesh.hpp"
#include "engine/renderer/camera.hpp"
#include "engine/core.hpp"

namespace render {

    void SceneRenderer::init(gpu::IDevice* pDevice, managers::AssetManager* pAssetManager) {
        ASSERT(pDevice != nullptr);
        ASSERT(pAssetManager != nullptr);
        m_pDevice = pDevice;
        m_pAssetManager = pAssetManager;
        // prepare renderer state

        // 1. load skybox

        // 2. create global shared resources states, passed to the draw functions

        // Allocate and reserve size for cbuffers
        m_geometryCbuffer = m_pDevice->makeBuffer({ .type = gpu::BufferType::ConstantBuffer, .usage = gpu::Usage::Dynamic, .debugName = "GeometryCbuffer" });
        m_pDevice->writeBuffer(m_geometryCbuffer, sizeof(GeometryCBuffer), nullptr);

        m_materialCbuffer = m_pDevice->makeBuffer({ .type = gpu::BufferType::ConstantBuffer, .usage = gpu::Usage::Dynamic, .debugName = "MaterialCbuffer" });
        m_pDevice->writeBuffer(m_materialCbuffer, sizeof(MaterialCBuffer), nullptr);

        m_lightsCbuffer = m_pDevice->makeBuffer({ .type = gpu::BufferType::ConstantBuffer, .usage = gpu::Usage::Dynamic, .debugName = "LightsCbuffer" });
        m_pDevice->writeBuffer(m_lightsCbuffer, sizeof(LightsCbuffer), nullptr);

        m_trillinearAniso16ClampSampler = m_pDevice->makeTextureSampler({ /* default (linear, wrap, 16x-aniso) */ });
        
        m_skyboxTexShader = m_pAssetManager->fetchShader({
        .graphicsState = {
                .depthState = gpu::CompareFunc::GreaterOrEqual,
                .depthWrite = false,
                .depthTest = true,
            },
            .vertShader = "skybox_tex_vert.glsl",
            .fragShader = "skybox_tex_frag.glsl",
            .debugName = "SkyboxTexture"
        });
        
        m_skyboxProceduralShader = m_pAssetManager->fetchShader({
        .graphicsState = {
                .depthState = gpu::CompareFunc::GreaterOrEqual,
                .depthWrite = true,
                .depthTest = true,
                .faceCullingMode = gpu::FaceCullMode::Never,
            },
            .vertShader = "skybox_procedural_vert.glsl",
            .fragShader = "skybox_procedural_frag.glsl",
            .debugName = "SkyboxProcedural"
        });

        // m_skyboxQuad = m_pAssetManager->fetchMesh("skybox_quad.obj");
        m_skyboxQuad = m_pAssetManager->fetchMesh("skybox_sphere.obj");

        // We need a pair of blend states, one for opaque rendering, and one for alpha blending
        m_opaque_BlendState = m_pDevice->makeBlendState({
            .blendEnable        = false,
        });
        
        m_alphaBlend_BlendState = m_pDevice->makeBlendState({
            .blendEnable        = true,
            .srcFactor          = gpu::BlendFactor::SrcAlpha,
            .dstFactor          = gpu::BlendFactor::OneMinusSrcColour,
            .blendOp            = gpu::BlendOp::Add,
        });
    }

    void SceneRenderer::drawSkybox(Scene& scene, Camera* cameraComponent, Light* sunLight) {
        m_pDevice->debugMarkerPush("Drawing skybox...");

        hlslpp::float4x4 skyboxProjection = cameraComponent->getProjectionMatrix();

        switch (scene.lightingParams.skybox.type) {
        case SkyboxType::Procedural:
        {

            // Set geometry cbuffer on bind slot 0
            m_pDevice->setConstantBuffer(m_geometryCbuffer, 0);
            GeometryCBuffer* geometryView = nullptr;
            m_pDevice->mapBuffer(m_geometryCbuffer, 0, sizeof(GeometryCBuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&geometryView));
            if (geometryView != nullptr) {

                // Skybox is a special case, we need the inverse view without translation
                auto rot = cameraComponent->getEntity()->transform.getRotation();
                hlslpp::float3 forward = hlslpp::normalize(hlslpp::mul(cameraComponent->getEntity()->transform.getRotation(), hlslpp::float3(0.0f, 0.0f, -1.0f)));
                // hlslpp::float3 up = hlslpp::normalize(hlslpp::mul(cameraComponent->getEntity()->transform.getRotation(), hlslpp::float3(0.0f, 1.0f, 0.0f)));
                geometryView->view = hlslpp::float4x4::look_at(hlslpp::float3(0,0,0), forward, hlslpp::float3(0.0f, 1.0f, 0.0f));

                geometryView->model = hlslpp::float4x4::identity();
                geometryView->projection = cameraComponent->getProjectionMatrix();
                geometryView->cameraPos = cameraComponent->getEntity()->transform.getPosition();
                m_pDevice->unmapBuffer(m_geometryCbuffer);
            }

            // Set lights cbuffer on bind slot 2
            m_pDevice->setConstantBuffer(m_lightsCbuffer, 2);
            LightsCbuffer* lightsView = nullptr;
            m_pDevice->mapBuffer(m_lightsCbuffer, 0, sizeof(LightsCbuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&lightsView));
            if (lightsView != nullptr) {
                memset(lightsView, 0, sizeof(LightsCbuffer));

#define BIND_LIGHT(CbufferLight, LightComponent) \
    CbufferLight.type         = (uint32_t) LightComponent->type; \
    CbufferLight.intensity    = LightComponent->intensity; \
    CbufferLight.innerRadius  = LightComponent->innerRadius; \
    CbufferLight.outerRadius  = LightComponent->outerRadius; \
    CbufferLight.position     = LightComponent->getPosition(); \
    CbufferLight.direction    = LightComponent->getDirection(); \
    CbufferLight.colour       = LightComponent->colour

                if (sunLight != nullptr) {
                    BIND_LIGHT(lightsView->light[0], sunLight);
                }
#undef BIND_LIGHT
                m_pDevice->unmapBuffer(m_lightsCbuffer);
            }
            
            // Draw procedural skybox
            m_pDevice->drawIndexed({
                .vertexBufer = m_skyboxQuad.vertexBuffer,
                .indexBuffer = m_skyboxQuad.indexBuffer,
                .shader = m_skyboxProceduralShader,
                .vertexLayout = m_skyboxQuad.vertexLayout,
                }, m_skyboxQuad.triangleCount
            );
            break;
        }
        case SkyboxType::HDRI:
        {
            // @TODO: Draw HDRI skybox (read: cubemap)
            break;
        }
        default:
            LOG_WARN("Unknown skybox type {}! Not drawing skybox...", (uint8_t) scene.lightingParams.skybox.type);
            break;
        }
        m_pDevice->debugMarkerPop();
    }

    void SceneRenderer::drawRenderList(std::vector<RenderListElement>& drawables, Camera* cameraComponent, Light* sunLight) {
        // Entity didn't have any components we wanted attached to itself, check children
        for (RenderListElement drawable : drawables) {
            MeshRenderer* pRenderer = drawable.pMeshRenderer;
            // may return null, if not null its what we're after anyway
            if (pRenderer->enabled && pRenderer->getEntity()->enabled) {
                if (pRenderer->mesh.triangleCount < 1) {
                    if (pRenderer->mesh.vertexBuffer) {
                        LOG_WARN("Tried rendering mesh with no triangles. Skipping...");
                    }
                    else {
                        LOG_WARN("Tried rendering mesh with no vertex buffer. Skipping...");
                    }
                } else {

                    // Set geometry cbuffer on bind slot 0
                    m_pDevice->setConstantBuffer(m_geometryCbuffer, 0);
                    GeometryCBuffer* geometryView = nullptr;
                    m_pDevice->mapBuffer(m_geometryCbuffer, 0, sizeof(GeometryCBuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&geometryView));
                    if (geometryView != nullptr) {
                        // @TODO: May need to flip
                        geometryView->model = hlslpp::mul(drawable.parentMatrix, pRenderer->getEntity()->transform.getModel());
                        geometryView->view = cameraComponent->getViewMatrix();
                        geometryView->projection = cameraComponent->getProjectionMatrix();
                        geometryView->cameraPos = cameraComponent->getEntity()->transform.getPosition();
                        m_pDevice->unmapBuffer(m_geometryCbuffer);
                    }

                    // Set material cbuffer on bind slot 1
                    m_pDevice->setConstantBuffer(m_materialCbuffer, 1);
                    MaterialCBuffer* materialView = nullptr;
                    m_pDevice->mapBuffer(m_materialCbuffer, 0, sizeof(MaterialCBuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&materialView));
                    if (materialView != nullptr) {
                        materialView->ambient = pRenderer->material.ambient;
                        materialView->diffuse = pRenderer->material.diffuse;
                        materialView->specular = pRenderer->material.specular;
                        materialView->emissionColour = pRenderer->material.emissionColour;
                        materialView->roughness = pRenderer->material.roughness;
                        materialView->metallic = pRenderer->material.metallic;
                        materialView->emissionIntensity = pRenderer->material.emissionIntensity;
                        m_pDevice->unmapBuffer(m_materialCbuffer);
                    }

                    // Set lights cbuffer on bind slot 2
                    m_pDevice->setConstantBuffer(m_lightsCbuffer, 2);
                    LightsCbuffer* lightsView = nullptr;
                    m_pDevice->mapBuffer(m_lightsCbuffer, 0, sizeof(LightsCbuffer), gpu::MapAccessFlags::Write | gpu::MapAccessFlags::InvalidateBuffer, reinterpret_cast<void**>(&lightsView));
                    if (lightsView != nullptr) {
                        memset(lightsView, 0, sizeof(LightsCbuffer));

#define BIND_LIGHT(CbufferLight, LightComponent) \
    CbufferLight.type         = (uint32_t) LightComponent->type; \
    CbufferLight.intensity    = LightComponent->intensity; \
    CbufferLight.innerRadius  = LightComponent->innerRadius; \
    CbufferLight.outerRadius  = LightComponent->outerRadius; \
    CbufferLight.position     = LightComponent->getPosition(); \
    CbufferLight.direction    = LightComponent->getDirection(); \
    CbufferLight.colour       = LightComponent->colour

                        if (sunLight == nullptr) {
                            for (int i = 0; i < m_lights.size() && i < k_MAX_LIGHTS; i++) {
                                BIND_LIGHT(lightsView->light[i], m_lights[i]);
                            }

                        } else {
                            BIND_LIGHT(lightsView->light[0], sunLight);
                            int lightWriteIdx = 1;
                            for (int i = 0; i < m_lights.size() && i < k_MAX_LIGHTS && lightWriteIdx < k_MAX_LIGHTS; i++) {
                                if (m_lights[i] != sunLight) {
                                    BIND_LIGHT(lightsView->light[lightWriteIdx], m_lights[i]);
                                    lightWriteIdx++;
                                }
                            }
                        }
#undef BIND_LIGHT
                        m_pDevice->unmapBuffer(m_lightsCbuffer);
                    }

                    // Bind textures with trillinearAniso16ClampSampler at slots 0, 1, 2, falling back to the built-in white texture if not set
                    if (pRenderer->material.diffuseTex) {
                        m_pDevice->bindTexture(pRenderer->material.diffuseTex, m_trillinearAniso16ClampSampler, 0);
                    } else {
                        m_pDevice->bindTexture(m_pAssetManager->fetchWhiteTexture(), m_trillinearAniso16ClampSampler, 0);
                    }

                    if (pRenderer->material.metaTex) {
                        m_pDevice->bindTexture(pRenderer->material.metaTex, m_trillinearAniso16ClampSampler, 1);
                    } else {
                        m_pDevice->bindTexture(m_pAssetManager->fetchWhiteTexture(), m_trillinearAniso16ClampSampler, 1);
                    }

                    if (pRenderer->material.emissionTex) {
                        m_pDevice->bindTexture(pRenderer->material.emissionTex, m_trillinearAniso16ClampSampler, 2);
                    } else {
                        m_pDevice->bindTexture(m_pAssetManager->fetchWhiteTexture(), m_trillinearAniso16ClampSampler, 2);
                    }

                    if (pRenderer->material.matcapTex) {
                        m_pDevice->bindTexture(pRenderer->material.matcapTex, m_trillinearAniso16ClampSampler, 3);
                    } else {
                        m_pDevice->bindTexture(m_pAssetManager->fetchWhiteTexture(), m_trillinearAniso16ClampSampler, 3);
                    }

                    if (pRenderer->material.brdfLutTex) {
                        m_pDevice->bindTexture(pRenderer->material.brdfLutTex, m_trillinearAniso16ClampSampler, 4);
                    } else {
                        m_pDevice->bindTexture(m_pAssetManager->fetchWhiteTexture(), m_trillinearAniso16ClampSampler, 4);
                    }

                    // Issue draw call
                    m_pDevice->drawIndexed({
                        .vertexBufer = pRenderer->mesh.vertexBuffer,
                        .indexBuffer = pRenderer->mesh.indexBuffer,
                        .shader = pRenderer->material.shader,
                        .vertexLayout = pRenderer->mesh.vertexLayout,
                        }, pRenderer->mesh.triangleCount
                    );
                }
            }
        }
    }

    void SceneRenderer::buildForwardRenderGraph(Entity* entity, hlslpp::float4x4 parentMatrix) {

        // iterate through the scene and push entities / renderers into 
        ASSERT(entity != nullptr);

        // entity.children
        if (entity->enabled) {

            for (const std::shared_ptr<IComponent> component : entity->components) {
                if (component->getComponentType() == render::ComponentType::MeshRenderer) {
                    MeshRenderer* pRenderer = (MeshRenderer*)component.get();
                    if (pRenderer->enabled) {
                        if (pRenderer->material.drawOrder <= k_drawOrder_Opaque) {
                            m_forwardOpaqueList.push_back({.pMeshRenderer = pRenderer, .parentMatrix = parentMatrix });
                        } else {
                            m_forwardTransparentList.push_back({ .pMeshRenderer = pRenderer, .parentMatrix = parentMatrix });
                        }
                    }
                }

                if (component->getComponentType() == render::ComponentType::Light) {
                    Light* pLight = (Light*)component.get();
                    if (pLight->enabled) {
                        m_lights.push_back(pLight);
                    }
                }
            }

            // Entity didn't have any components we wanted attached to itself, check children
            for (const std::shared_ptr<Entity> childEntity : entity->children) {
                // may return null, if not null its what we're after anyway
                if (childEntity->enabled) {
                    // @TODO: May need to flip
                    buildForwardRenderGraph(childEntity.get(), hlslpp::mul(entity->transform.getModel(), parentMatrix));
                }
            }
        }
    }

    void SceneRenderer::draw(Scene& scene, const float aspect) {
        // @TODO: We could do a more complex scene graph to optimise searching for entities but it doesn't harm performance enough to matter

        // Find the active camera
        // @TODO: Consider caching the camera entity??
        Entity* cameraEntity = scene.findEntityWithType(ComponentType::Camera);
        if (!cameraEntity) {
            return;
        }
        Camera* cameraComponent = (Camera*) cameraEntity->findComponent(ComponentType::Camera);
        if (!cameraComponent) {
            return;
        }
        if (!cameraEntity->enabled || !cameraComponent->enabled) {
            // Can't draw if the camera is disabled
            return;
        }
        cameraComponent->setAspect(aspect); // Update aspect ratio

        // forward rendering is simple:
        //   split scene by opaque and transparent meshes
        //   for each mesh, sort by draw order, then distance from camera
        //   draw opaque stuff first, front to back
        //   then draw the skybox (to take advantage of early-z discard)
        //   then draw transparent meshes, back to front

        // clear draw lists
        m_lights.clear();
        m_forwardOpaqueList.clear();
        m_forwardTransparentList.clear();
        // find lights and meshes
        buildForwardRenderGraph(&scene.root, hlslpp::float4x4::identity());

        // sort draw graphs
        auto compareByDrawOrderFrontToBack = [cameraComponent](RenderListElement a, RenderListElement b) -> bool {
            if (a.pMeshRenderer->material.drawOrder == b.pMeshRenderer->material.drawOrder) {
                float distCamA = hlslpp::length(cameraComponent->getEntity()->transform.getPosition() - a.pMeshRenderer->getEntity()->transform.getPosition());
                float distCamB = hlslpp::length(cameraComponent->getEntity()->transform.getPosition() - b.pMeshRenderer->getEntity()->transform.getPosition());
                return distCamA < distCamB;
            }
            return a.pMeshRenderer->material.drawOrder < b.pMeshRenderer->material.drawOrder;
        };
        auto compareByDrawOrderBackToFront = [cameraComponent](RenderListElement a, RenderListElement b) -> bool {
            if (a.pMeshRenderer->material.drawOrder == b.pMeshRenderer->material.drawOrder) {
                float distCamA = hlslpp::length(cameraComponent->getEntity()->transform.getPosition() - a.pMeshRenderer->getEntity()->transform.getPosition());
                float distCamB = hlslpp::length(cameraComponent->getEntity()->transform.getPosition() - b.pMeshRenderer->getEntity()->transform.getPosition());
                return distCamA > distCamB;
            }
            return a.pMeshRenderer->material.drawOrder < b.pMeshRenderer->material.drawOrder;
        };

        // sort opaque front to back, transparent back to front for optimal rendering
        std::sort(m_forwardOpaqueList.begin(), m_forwardOpaqueList.end(), compareByDrawOrderFrontToBack);
        std::sort(m_forwardTransparentList.begin(), m_forwardTransparentList.end(), compareByDrawOrderBackToFront);

        // issue draw calls
        m_pDevice->debugMarkerPush("Drawing scene...");

        m_pDevice->bindBlendState(m_opaque_BlendState);
        drawRenderList(m_forwardOpaqueList, cameraComponent, scene.lightingParams.sunLight);

        // Skybox is rendered after opaque materials and before transparent ones
        // this is to take advantage of an optimisation with opaque rendering.
        // Since most opaque materials write to the backbuffer during rendering we
        // would be overwriting the data in some pixel P with the skybox's fragment
        // leading to overdraw. By enabling the depth buffer, one can know whether an
        // opaque pixel was written to, and by doing a depth test with no depth writing
        // we can force the skybox to draw at the furthest depth value. This allows us
        // to use Early-Z discard, a hardware optimisation of the rasterisation stage
        // of the rendering pipeline, where the GPU discards fragments of pixels which
        // have already been written to.
        drawSkybox(scene, cameraComponent, scene.lightingParams.sunLight);
        
        m_pDevice->bindBlendState(m_alphaBlend_BlendState);
        drawRenderList(m_forwardTransparentList, cameraComponent, scene.lightingParams.sunLight);
        
        m_pDevice->bindBlendState(m_opaque_BlendState);

        m_pDevice->debugMarkerPop();
    }
}