#include "Common.h"
#include "CDraw/Vertex.h"

static SDL_GPUGraphicsPipeline* Pipeline;
static SDL_GPUBuffer* VertexBuffer;
static SDL_GPUBuffer* IndexBuffer;
static SDL_GPUTexture* Texture;
static SDL_GPUSampler* Sampler;

static const int SPRITE_COUNT = 2;

static int SpriteBatchCount = 0;

static void DrawSprites(Context* context, SDL_GPUCommandBuffer* cmdbuf);

// TODO: abstract staging sprites to a method
// TODO: sprite rotation
// TODO: sprite scale
// TODO: sprite animation
// TODO: multiple textures
// TODO: PNG textures
// TODO: render targets

// FIXME: alpha broken

int Init(Context* context)
{
    int result = CommonInit(context, 0);
    if (result < 0)
    {
        return result;
    }

    // Create the shaders
    SDL_GPUShader* vertexShader = LoadShader(context->Device, "TexturedQuad.vert", 0, 1, 0, 0, SDL_GPU_SHADERSTAGE_VERTEX);
    if (vertexShader == NULL)
    {
        SDL_Log("Failed to create vertex shader!");
        return -1;
    }

    SDL_GPUShader* fragmentShader = LoadShader(context->Device, "TexturedQuad.frag", 1, 0, 0, 0, SDL_GPU_SHADERSTAGE_FRAGMENT);
    if (fragmentShader == NULL)
    {
        SDL_Log("Failed to create fragment shader!");
        return -1;
    }

    // Load the image
    SDL_Surface *imageData = LoadImage("ravioli.bmp", 4);
    if (imageData == NULL)
    {
        SDL_Log("Could not load image data!");
        return -1;
    }

    // Create the pipeline
    SDL_GPUGraphicsPipelineCreateInfo pipelineCreateInfo = {
        .attachmentInfo = {
            .colorAttachmentCount = 1,
            .colorAttachmentDescriptions = (SDL_GPUColorAttachmentDescription[]){{
                .format = SDL_GetGPUSwapchainTextureFormat(context->Device, context->Window),
                .blendState = {
                    .blendEnable = SDL_TRUE,
                    .alphaBlendOp = SDL_GPU_BLENDOP_ADD,
                    .colorBlendOp = SDL_GPU_BLENDOP_ADD,
                    .colorWriteMask = 0xF,
                    .srcColorBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
                    .srcAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ONE,
                    .dstColorBlendFactor = SDL_GPU_BLENDFACTOR_ZERO,
                    .dstAlphaBlendFactor = SDL_GPU_BLENDFACTOR_ZERO
                }
            }},
        },
        .vertexInputState = (SDL_GPUVertexInputState){
            .vertexBindingCount = 1,
            .vertexBindings = (SDL_GPUVertexBinding[]){{
                .binding = 0,
                .inputRate = SDL_GPU_VERTEXINPUTRATE_VERTEX,
                .instanceStepRate = 0,
                .stride = sizeof(CDraw_Vertex)
            }},
            .vertexAttributeCount = 3,
            .vertexAttributes = (SDL_GPUVertexAttribute[]){
                {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
                    .location = 0,
                    .offset = 0,
                },
                {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2,
                    .location = 1,
                    .offset = sizeof(float) * 3,
                },
                {
                    .binding = 0,
                    .format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT4,
                    .location = 2,
                    .offset = sizeof(float) * 5,
                },
            },
        },
        .multisampleState.sampleMask = 0xFFFF,
        .primitiveType = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST,
        .vertexShader = vertexShader,
        .fragmentShader = fragmentShader
    };

    Pipeline = SDL_CreateGPUGraphicsPipeline(context->Device, &pipelineCreateInfo);
    if (Pipeline == NULL)
    {
        SDL_Log("Failed to create pipeline!");
        return -1;
    }

    SDL_ReleaseGPUShader(context->Device, vertexShader);
    SDL_ReleaseGPUShader(context->Device, fragmentShader);

    // PointClamp
    Sampler = SDL_CreateGPUSampler(context->Device, &(SDL_GPUSamplerCreateInfo){
        .minFilter = SDL_GPU_FILTER_NEAREST,
        .magFilter = SDL_GPU_FILTER_NEAREST,
        .mipmapMode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST,
        .addressModeU = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .addressModeV = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
        .addressModeW = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE,
    });
    
    // Create the GPU resources
    VertexBuffer = SDL_CreateGPUBuffer(
        context->Device,
        &(SDL_GPUBufferCreateInfo) {
            .usageFlags = SDL_GPU_BUFFERUSAGE_VERTEX_BIT,
            .sizeInBytes = sizeof(CDraw_Vertex) * 4 * SPRITE_COUNT
        }
    );
    // TODO: we can probly delete this
    SDL_SetGPUBufferName(
        context->Device,
        VertexBuffer,
        "Ravioli Vertex Buffer 🥣"
    );

    IndexBuffer = SDL_CreateGPUBuffer(
        context->Device,
        &(SDL_GPUBufferCreateInfo) {
            .usageFlags = SDL_GPU_BUFFERUSAGE_INDEX_BIT,
            .sizeInBytes = sizeof(Uint16) * 6 * SPRITE_COUNT
        }
    );

    Texture = SDL_CreateGPUTexture(context->Device, &(SDL_GPUTextureCreateInfo){
        .type = SDL_GPU_TEXTURETYPE_2D,
        .format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
        .width = imageData->w,
        .height = imageData->h,
        .layerCountOrDepth = 1,
        .levelCount = 1,
        .usageFlags = SDL_GPU_TEXTUREUSAGE_SAMPLER_BIT
    });
    SDL_SetGPUTextureName(
        context->Device,
        Texture,
        "Ravioli Texture 🖼️"
    );

    // Set up buffer data
    SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
        context->Device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = (sizeof(CDraw_Vertex) * 4 * SPRITE_COUNT) + (sizeof(Uint16) * 6 * SPRITE_COUNT)
        }
    );

    CDraw_Vertex* transferData = SDL_MapGPUTransferBuffer(
        context->Device,
        bufferTransferBuffer,
        SDL_FALSE
    );
    
    Uint16* indexData = (Uint16*) &transferData[4 * SPRITE_COUNT];
    for (int i = 0; i < SPRITE_COUNT; i++) {
        const int u = i * 6;
        const int v = i * 4;
        indexData[u + 0] = v + 0;
        indexData[u + 1] = v + 1;
        indexData[u + 2] = v + 2;
        indexData[u + 3] = v + 0;
        indexData[u + 4] = v + 2;
        indexData[u + 5] = v + 3;
    }
    
    SDL_UnmapGPUTransferBuffer(context->Device, bufferTransferBuffer);
    
    // Set up texture data
    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(
        context->Device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = imageData->w * imageData->h * 4
        }
    );

    Uint8* textureTransferPtr = SDL_MapGPUTransferBuffer(
        context->Device,
        textureTransferBuffer,
        SDL_FALSE
    );
    SDL_memcpy(textureTransferPtr, imageData->pixels, imageData->w * imageData->h * 4);
    SDL_UnmapGPUTransferBuffer(context->Device, textureTransferBuffer);

    // Upload the transfer data to the GPU resources
    SDL_GPUCommandBuffer* uploadCmdBuf = SDL_AcquireGPUCommandBuffer(context->Device);
    
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(uploadCmdBuf);
    
    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) {
            .transferBuffer = bufferTransferBuffer,
            .offset = sizeof(CDraw_Vertex) * 4 * SPRITE_COUNT
        },
        &(SDL_GPUBufferRegion) {
            .buffer = IndexBuffer,
            .offset = 0,
            .size = sizeof(Uint16) * 6 * SPRITE_COUNT
        },
        SDL_FALSE
    );

    SDL_UploadToGPUTexture(
        copyPass,
        &(SDL_GPUTextureTransferInfo) {
            .transferBuffer = textureTransferBuffer,
            .offset = 0, /* Zeros out the rest */
        },
        &(SDL_GPUTextureRegion){
            .texture = Texture,
            .w = imageData->w,
            .h = imageData->h,
            .d = 1
        },
        SDL_FALSE
    );

    SDL_DestroySurface(imageData);
    SDL_EndGPUCopyPass(copyPass);
    SDL_ReleaseGPUTransferBuffer(context->Device, bufferTransferBuffer);
    SDL_ReleaseGPUTransferBuffer(context->Device, textureTransferBuffer);
    
    SDL_SubmitGPU(uploadCmdBuf);

    // Finally, print instructions!
    SDL_Log("Press Left/Right to switch between sampler states");

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
    
    SDL_GPUCommandBuffer* cmdbuf = SDL_AcquireGPUCommandBuffer(context->Device);
    if (cmdbuf == NULL)
    {
        SDL_Log("GPUAcquireCommandBuffer failed");
        return -1;
    }
    
    DrawSprites(context, cmdbuf);

    Uint32 w, h;
    SDL_GPUTexture* swapchainTexture = SDL_AcquireGPUSwapchainTexture(cmdbuf, context->Window, &w, &h);
    if (swapchainTexture != NULL)
    {
        SDL_GPUColorAttachmentInfo colorAttachmentInfo = { 0 };
        colorAttachmentInfo.texture = swapchainTexture;
        colorAttachmentInfo.clearColor = (SDL_FColor){ 0.0f, 0.0f, 0.0f, 1.0f };
        colorAttachmentInfo.loadOp = SDL_GPU_LOADOP_CLEAR;
        colorAttachmentInfo.storeOp = SDL_GPU_STOREOP_STORE;

        SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdbuf, &colorAttachmentInfo, 1, NULL);

        SDL_BindGPUGraphicsPipeline(renderPass, Pipeline);
        SDL_BindGPUVertexBuffers(renderPass, 0, &(SDL_GPUBufferBinding){ .buffer = VertexBuffer, .offset = 0 }, 1);
        SDL_BindGPUIndexBuffer(renderPass, &(SDL_GPUBufferBinding){ .buffer = IndexBuffer, .offset = 0 }, SDL_GPU_INDEXELEMENTSIZE_16BIT);
        SDL_BindGPUFragmentSamplers(renderPass, 0, &(SDL_GPUTextureSamplerBinding){ .texture = Texture, .sampler = Sampler }, 1);
        SDL_PushGPUVertexUniformData(
            cmdbuf,
            0,
            &cameraMatrix,
            sizeof(Matrix4x4)
        );
        SDL_DrawGPUIndexedPrimitives(renderPass, SpriteBatchCount * 6, 1, 0, 0, 0);

        SDL_EndGPURenderPass(renderPass);
    }

    SDL_SubmitGPU(cmdbuf);

    return 0;
}

void Quit(Context* context)
{
    SDL_ReleaseGPUGraphicsPipeline(context->Device, Pipeline);
    SDL_ReleaseGPUBuffer(context->Device, VertexBuffer);
    SDL_ReleaseGPUBuffer(context->Device, IndexBuffer);
    SDL_ReleaseGPUTexture(context->Device, Texture);
    SDL_ReleaseGPUSampler(context->Device, Sampler);

    CommonQuit(context);
}

static void DrawSprites(Context* context, SDL_GPUCommandBuffer* cmdbuf)
{
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdbuf);
    SDL_GPUTransferBuffer* bufferTransferBuffer = SDL_CreateGPUTransferBuffer(
        context->Device,
        &(SDL_GPUTransferBufferCreateInfo) {
            .usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
            .sizeInBytes = (sizeof(CDraw_Vertex) * 4 * SPRITE_COUNT) + (sizeof(Uint16) * 6 * SPRITE_COUNT)
        }
    );
    CDraw_Vertex* transferData = SDL_MapGPUTransferBuffer(
        context->Device,
        bufferTransferBuffer,
        SDL_FALSE
    );
    
    const CDraw_Color white = {
        .r = 1.0f,
        .g = 1.0f,
        .b = 1.0f,
        // TODO: alpha isn't working
        .a = 1.0f,
    };
    
    static float baseX = 0, baseY = 0;
    
    float x = baseX, y = baseY;
    float width = 32, height = 32;
    
    // Stage Sprites
    SpriteBatchCount = 2;
    // Sprite 1
    transferData[0] = (CDraw_Vertex) { .x = x, .y = y, .z = 0, .u = 0, .v = 0, .color = white };
    transferData[1] = (CDraw_Vertex) { .x = x + width, .y = y, .z = 0, .u = 1, .v = 0, .color = white };
    transferData[2] = (CDraw_Vertex) { .x = x + width, .y = y + height, .z = 0, .u = 1, .v = 1, .color = white };
    transferData[3] = (CDraw_Vertex) { .x = x, .y = y + height, .z = 0, .u = 0, .v = 1, .color = white };
    // Sprite 2
    x += 64;
    transferData[4] = (CDraw_Vertex) { .x = x, .y = y, .z = 0, .u = 0, .v = 0, .color = white };
    transferData[5] = (CDraw_Vertex) { .x = x + width, .y = y, .z = 0, .u = 1, .v = 0, .color = white };
    transferData[6] = (CDraw_Vertex) { .x = x + width, .y = y + height, .z = 0, .u = 1, .v = 1, .color = white };
    transferData[7] = (CDraw_Vertex) { .x = x, .y = y + height, .z = 0, .u = 0, .v = 1, .color = white };
    
    baseX++;
    baseY++;
    
    SDL_UploadToGPUBuffer(
        copyPass,
        &(SDL_GPUTransferBufferLocation) {
            .transferBuffer = bufferTransferBuffer,
            .offset = 0
        },
        &(SDL_GPUBufferRegion) {
            .buffer = VertexBuffer,
            .offset = 0,
            .size = sizeof(CDraw_Vertex) * 4 * SpriteBatchCount
        },
        SDL_FALSE
    );
    
    SDL_UnmapGPUTransferBuffer(context->Device, bufferTransferBuffer);
    SDL_EndGPUCopyPass(copyPass);
}
