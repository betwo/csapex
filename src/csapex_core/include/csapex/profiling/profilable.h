#ifndef PROFILABLE_H
#define PROFILABLE_H

/// COMPONENT
#include <csapex_core/csapex_profiling_export.h>

/// SYSTEM
#include <memory>

namespace csapex
{
class Profiler;

class CSAPEX_PROFILING_EXPORT Profilable
{
public:
    Profilable();
    Profilable(std::shared_ptr<Profiler> profiler);

    virtual void useProfiler(std::shared_ptr<Profiler> profiler);
    std::shared_ptr<Profiler> getProfiler() const;

protected:
    std::shared_ptr<Profiler> profiler_;
};

}  // namespace csapex

#endif  // PROFILABLE_H
