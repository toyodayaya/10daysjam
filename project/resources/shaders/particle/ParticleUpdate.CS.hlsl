struct Particle
{
    float32_t3 translate;
    float32_t3 scale;
    float32_t lifeTime;
    float32_t3 velocity;
    float32_t currentTime;
    float32_t4 color;
};

struct PerFrame
{
    float32_t time;
    float32_t deltaTime;
};

static const uint32_t kMaxParticles = 1024;
RWStructuredBuffer<Particle> gParticle : register(u0);
RWStructuredBuffer<int32_t> gFreeListIndex : register(u1);
RWStructuredBuffer<uint32_t> gFreeList : register(u2);
ConstantBuffer<PerFrame> gParFrame : register(b1);

[numthreads(1, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
    uint32_t particleIndex = DTid.x;
    if(particleIndex < kMaxParticles)
    {
        if(gParticle[particleIndex].color.a == 0)
        {
            gParticle[particleIndex].scale = float32_t3(0.0f, 0.0f, 0.0f);
            int32_t freeListIndex;
            InterlockedAdd(gFreeListIndex[0], 1, freeListIndex);
            
            if((freeListIndex + 1) < kMaxParticles)
            {
                gFreeList[freeListIndex + 1] = particleIndex;
            }
            else
            {
                InterlockedAdd(gFreeListIndex[0], -1, freeListIndex);
            }
        }
        else
        {
            gParticle[particleIndex].translate += gParticle[particleIndex].velocity;
            gParticle[particleIndex].currentTime += gParFrame.deltaTime;
            float32_t alpha = 1.0f - (gParticle[particleIndex].currentTime / gParticle[particleIndex].lifeTime);
            gParticle[particleIndex].color.a = saturate(alpha);
        }
    }
}