#include "clang/Frontend/FrontendPluginRegistry.h"
#include "tools/reflect/action.h"

namespace pulse::reflect::plugin {

namespace {

clang::FrontendPluginRegistry::Add<ReflectAction> action_registration(
    "pulse_reflect", "Static reflection for pulse_reflect classes");

}  // namespace

}  // namespace pulse::reflect::plugin
