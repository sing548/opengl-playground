#ifndef I_SCENE_RENDERABLE
#define I_SCENE_RENDERABLE

#include <string>
#include <unordered_map>

#include "render-list.h"

struct ISceneRenderable {
    virtual ~ISceneRenderable() = default;
    virtual void Render(const FrameGlobals&, const std::unordered_map<std::string, bool>&) = 0;
    virtual RenderPass GetRenderPass() = 0;
    virtual int GetOrder() = 0;
};

#endif
