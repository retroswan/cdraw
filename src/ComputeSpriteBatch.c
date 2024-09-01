#include "Common.h"
#include <stdlib.h> // for srand

// TODO: don't use compute for building vertices
// TODO: more than one texture
// TODO: load PNG textures
// TODO: apply additional shaders
// FIXME: alpha doesn't work

static SDL_GPUGraphicsPipeline* RenderPipeline;
static SDL_GPUSampler* Sampler;
static SDL_GPUTexture* Texture;
static SDL_GPUTransferBuffer* SpriteVertexTransferBuffer;
static SDL_GPUBuffer* SpriteVertexBuffer;
static SDL_GPUBuffer* SpriteIndexBuffer;

typedef struct PositionTextureColorVertex
{
    float x, y, z, w;
    float u, v;
    float padding_a, padding_b;
    float r, g, b, a;
} PositionTextureColorVertex;

const Uint32 SPRITE_COUNT = 4098;

int Init(Context* context)
{
    int result = CommonInit(context, 0);
    if (result < 0)
    {
        return result;
    }

    SDL_GPUPresentMode presentMode = SDL_GPU_PRESENTMODE_VSYNC;
    // TODO: don't do immediate I think?
    if (SDL_SupportsGPUPresentMode(
        context->Device,
        context->Window,
        SDL_GPU_PRESENTMODE_IMMEDIATE
    )) {
        presentMode = SDL_GPU_PRESENTMODE_IMMEDIATE;
    }
    else if (SDL_SupportsGPUPresentMode(
        context->Device,
        context->Window,
        SDL_GPU_PRESENTMODE_MAILBOX
    )) {
        presentMode = SDL_GPU_PRESENTMODE_MAILBOX;
    }

    SDL_SetGPUSwapchainParameters(
        context->Device,
        context->Window,
        SDL_GPU_SWAPCHAINCOMPOSITION_SDR,
        presentMode
    );

    srand(0);

    // Create the shaders
    SDL_GPUShader* vertShader = LoadShader(
        context->Device,
        "TexturedQuadColorWithMatrix.vert",
        0,
        1,
        0,
        0,
        SDL_GPU_SHADERSTAGE_VERTEX
    );

    SDL_GPUShader* fragShader = LoadShader(
        context->Device,
        "TexturedQuadColor.frag",
        1,
        0,
        0,
        0,
        SDL_GPU_SHADERSTAGE_FRAGMENT
    );

    // Create the sprite render pipeline
    RenderPipeline = SDL_CreateGPUGraphicsPipeline(
        context->Device,
        &(SDL_GPUGraphicsPipelineCreateInfo){
            .attachmentInfo = (SDL_GPUGraphicsPipelineAttachmentInfo){
                .colorAttachmentCount = 1,
                .colorAttachmentDescriptions = (SDL_GPUColorAttachmentDescription[]){{
                    .format = SDL_GetGPUSwapchainTextureFormat(context->Device, context->Window),
                    .blendState = (SDL_GPUColorAttachmentBlendState){
                        .blendEnable = SDL_TRUE,
                        .alphaBlendOp = SDL_GPU_BLENDOP_ADD,
                        .colorBlendOp = SDL_GPU_BLENDOP_ADD,
                        .colorWriteMask = 0xF,
                        .srcColorBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
                        .srcAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
                        .dstColorBlendFactor = SDL_GPU_BLENDFACTOR_ZERO,
                        .dstAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ZERO
                    }
                }}
            },
            .vertexInputState = (SDL_GPUVertexInputState){
                .vertexBindingCount = 1,
                .vertexBindings = (SDL_GPUVertexBinding[]){{
                    .binding = 0,
                    .inputRate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                    .instanceStepRate = 0,
                    .stride = sizeof(PositionTextureColorVertex)
                }},
                .vertexAttributeCount = 3,
                .vertexAttributes = (SDL_GPUVertexAttribute[]){{
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    .location = 0,
                    .offset = 0
                }, {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 1,
                    .offset = 16
                }, {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    .location = 2,
                    .offset = 32
                }}
            },
            .multisampleState.sampleMask = 0xFFFF,
            .primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
            .vertexShader = vertShader,
            .fragmentShader = fragShader
        }
    );

    SDL_ReleaseGPUShader(context->Device, vertShader);
    SDL_ReleaseGPUShader(context->Device, fragShader);

    // Load the image data
    SDL_Surface *imageData = LoadImage("ravioli.bmp", 4);
    if (imageData == NULL)
    {
        SDL_Log("Could not load image data!");
        return -1;
    }

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
        context->Device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = imageData->w * imageData->h * 4
        }
    );

    Uint8 *textureTransferPtr = SDL_MapGPUTransferBuffer(
        context->Device,
        textureTransferBuffer,
        SDL_FALSE
    );
    SDL_memcpy(textureTransferPtr, imageData->pixels, imageData->w * imageData->h * 4);
    SDL_UnmapGPUTransferBuffer(context->Device, textureTransferBuffer);

    // Create the GPU resources
    Texture = SDL_CreateGPUTexture(
        context->Device,
        &(SDL_GPUTextureCreateInfo){
            .type = SDL_GPU_TEXTURETYPE_2D,
            .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
            .width = imageData->w,
            .height = imageData->h,
            .layerCountOrDepth = 1,
            .levelCount = 1,
            .usageFlags = SDL_GPU_TEXTUREUSAGE_SAMPLER_BIT
        }
    );

    Sampler = SDL_CreateGPUSampler(
        context->Device,
        &(SDL_GPUSamplerCreateInfo){
            .minFilter = SDL_GPU_FILTER_NEAREST,
            .magFilter = SDL_GPU_FILTER_NEAREST,
            .mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
            .addressModeU = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .addressModeV = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
            .addressModeW = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE
        }
    );

    SpriteVertexTransferBuffer = SDL_CreateGPUTransferBuffer(
        context->Device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = 0,
            .sizeInBytes = SPRITE_COUNT * 4 * sizeof(PositionTextureColorVertex)
        }
    );
    SpriteVertexBuffer = SDL_CreateGPUBuffer(
        context->Device,
        &(SDL_GPUBufferCreateInfo) {
            // TODO: maybe don't need compute bit anymore?
            .usageFlags = SDL_GPU_BUFFERUSAGE_COMPUTE_STORAGE_WRITE_BIT | SDL_GPU_BUFFERUSAGE_VERTEX_BIT,
            .sizeInBytes = SPRITE_COUNT * 4 * sizeof(PositionTextureColorVertex)
        }
    );
    
    SpriteIndexBuffer = SDL_CreateGPUBuffer(
        context->Device,
        &(SDL_GPUBufferCreateInfo) {
            .usageFlags = SDL_GPU_BUFFERUSAGE_INDEX_BIT,
            .sizeInBytes = SPRITE_COUNT * 6 * sizeof(Uint32)
        }
    );

    // Transfer the up-front data
    SDL_GPUTransferBuffer* indexBufferTransferBuffer = SDL_CreateGPUTransferBuffer(
        context->Device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = SPRITE_COUNT * 6 * sizeof(Uint32)
        }
    );

    Uint32* indexTransferPtr = SDL_MapGPUTransferBuffer(
        context->Device,
        indexBufferTransferBuffer,
        SDL_FALSE
    );

    for (Uint32 i = 0, j = 0; i < SPRITE_COUNT * 6; i += 6, j += 4)
    {
        indexTransferPtr[i]     =  j;
        indexTransferPtr[i + 1] =  j + 1;
        indexTransferPtr[i + 2] =  j + 2;
        indexTransferPtr[i + 3] =  j + 3;
        indexTransferPtr[i + 4] =  j + 2;
        indexTransferPtr[i + 5] =  j + 1;
    }

    SDL_UnmapGPUTransferBuffer(
        context->Device,
        indexBufferTransferBuffer
    );

    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->Device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);

    SDL_UploadToGPUTexture(
        copyPass,
        &(SDL_GPUTextureTransferInfo) {
            .transferBuffer = textureTransferBuffer,
            .offset = 0, /* Zeroes out the rest */
        },
        &(SDL_GPUTextureRegion){
            .texture = Texture,
            .w = imageData->w,
            .h = imageData->h,
            .d = 1
        },
        SDL_FALSE
    );

    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) {
            .transferBuffer = indexBufferTransferBuffer,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = SpriteIndexBuffer,
            .offset = 0,
            .size = SPRITE_COUNT * 6 * sizeof(Uint32)
        },
        SDL_FALSE
    );

    SDL_DestroySurface(imageData);
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPU(uploadCmdBuf);
    SDL_ReleaseGPUTransferBuffer(context->Device, textureTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(context->Device, indexBufferTransferBuffer);

    return 0;
}

int Update(Context* context)
{
    return 0;
}

int Draw(Context* context)
{
    Matrix4x4 cameraMatrix = Matrix4x4_CreateOrthographicOffCenter(
        0,
        640,
        480,
        0,
        0,
        -1
    );

    Uint32 w, h;

    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(context->Device);
    SDL_GPUTexture* swapchainTexture = SDL_AcquireGPUSwapchainTexture(
        cmdBuf,
        context->Window,
        &w,
        &h
    );

    if (swapchainTexture != NULL) {
        Uint32 count = 0;
        static int x = 0, y = 0;
        static float rotation = 0;
        
        x += 1;
        y += 1;
        rotation += 0.05f;
        
        // TODO: unmap me?
        PositionTextureColorVertex* vertices = SDL_MapGPUTransferBuffer(
            context->Device,
            SpriteVertexTransferBuffer,
            SDL_TRUE
        );
        for (Uint32 i = 0; i < 1; i ++) {
            count++;
            
            const int u = i * 4;
            
            // Top Left
            {
                vertices[u + 0].x = x;
                vertices[u + 0].y = y;
                vertices[u + 0].z = 1;
                vertices[u + 0].w = 0;
                
                vertices[u + 0].u = 0;
                vertices[u + 0].v = 0;
                
                vertices[u + 0].r = 1.0f;
                vertices[u + 0].g = 1.0f;
                vertices[u + 0].b = 1.0f;
                vertices[u + 0].a = 1.0f;
            }
            
            // Top Right
            {
                vertices[u + 1].x = x + 32;
                vertices[u + 1].y = y;
                vertices[u + 1].z = 1;
                vertices[u + 1].w = 0;
                
                vertices[u + 1].u = 1;
                vertices[u + 1].v = 0;
                
                vertices[u + 1].r = 1.0f;
                vertices[u + 1].g = 1.0f;
                vertices[u + 1].b = 1.0f;
                vertices[u + 1].a = 1.0f;
            }
            
            // Bottom Left
            {
                vertices[u + 2].x = x;
                vertices[u + 2].y = y + 32;
                vertices[u + 2].z = 1;
                vertices[u + 2].w = 0;
                
                vertices[u + 2].u = 0;
                vertices[u + 2].v = 1;
                
                vertices[u + 2].r = 1.0f;
                vertices[u + 2].g = 1.0f;
                vertices[u + 2].b = 1.0f;
                vertices[u + 2].a = 1.0f;
            }
            
            // Bottom Right
            {
                vertices[u + 3].x = x + 32;
                vertices[u + 3].y = y + 32;
                vertices[u + 3].z = 1;
                vertices[u + 3].w = 0;
                
                vertices[u + 3].u = 1;
                vertices[u + 3].v = 1;
                
                vertices[u + 3].r = 1.0f;
                vertices[u + 3].g = 1.0f;
                vertices[u + 3].b = 1.0f;
                vertices[u + 3].a = 1.0f;
            }
        }
        SDL_UnmapGPUTransferBuffer(context->Device, SpriteVertexTransferBuffer);
        
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);
        SDL_UploadToGPUBuffer(
            copyPass,
            &(SDL_GPUTransferBufferLocation) {
                .transferBuffer = SpriteVertexTransferBuffer,
                .offset = 0
            },
            &(SDL_GPUBufferRegion) {
                .buffer = SpriteVertexBuffer,
                .offset = 0,
                .size = count * sizeof(PositionTextureColorVertex) * 4
            },
            SDL_TRUE
        );
        SDL_EndGPUCopyPass(copyPass);
        
        // Render sprites
        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(
            cmdBuf,
            &(SDL_GPUColorAttachmentInfo){
                .texture = swapchainTexture,
                .cycle = SDL_FALSE,
                .loadOp = SDL_GPU_LOADOP_CLEAR,
                .storeOp = SDL_GPU_STOREOP_STORE,
                .clearColor = { 0, 0, 0, 1 }
            },
            1,
            NULL
        );

        SDL_BindGPUGraphicsPipeline(renderPass, RenderPipeline);
        SDL_BindGPUVertexBuffers(
            renderPass,
            0,
            &(SDL_GPUBufferBinding){
                .buffer = SpriteVertexBuffer
            },
            1
        );
        SDL_BindGPUIndexBuffer(
            renderPass,
            &(SDL_GPUBufferBinding){
                .buffer = SpriteIndexBuffer
            },
            SDL_GPU_INDEXELEMENTSIZE_32BIT
        );
        SDL_BindGPUFragmentSamplers(
            renderPass,
            0,
            &(SDL_GPUTextureSamplerBinding){
                .texture = Texture,
                .sampler = Sampler
            },
            1
        );
        SDL_PushGPUVertexUniformData(
            cmdBuf,
            0,
            &cameraMatrix,
            sizeof(Matrix4x4)
        );
        SDL_DrawGPUIndexedPrimitives(
            renderPass,
            count * 6,
            1,
            0,
            0,
            0
        );

        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPU(cmdBuf);

    return 0;
}

void Quit(Context* context)
{
    SDL_ReleaseGPUGraphicsPipeline(context->Device, RenderPipeline);
    SDL_ReleaseGPUSampler(context->Device, Sampler);
    SDL_ReleaseGPUTexture(context->Device, Texture);
    SDL_ReleaseGPUTransferBuffer(context->Device, SpriteVertexTransferBuffer);
    SDL_ReleaseGPUBuffer(context->Device, SpriteVertexBuffer);
    SDL_ReleaseGPUBuffer(context->Device, SpriteIndexBuffer);

    CommonQuit(context);
}
