#ifndef I_GAMEPLAY_SYSTEM
#define I_GAMEPLAY_SYSTEM

#include "system-structs.h"

class IGameplaySystem 
{
public:
    virtual ~IGameplaySystem() = default;
    virtual void Update(SystemsContext& context) = 0;
    virtual GameplayPhase GetPhase() const = 0;
    virtual int GetOrder() const = 0;
    virtual bool CanReplay() { return false; };
};

#endif
