#include "clang/Basic/ParsedAttrInfo.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "tools/reflect/action.h"
#include "tools/reflect/attribute.h"

namespace pulse::reflect::plugin {

namespace {

clang::FrontendPluginRegistry::Add<ReflectAction> action_registration(
    "pulse_reflect", "Static reflection for [[pulse::reflect]]");

clang::ParsedAttrInfoRegistry::Add<PulseReflectAttrInfo> attribute_registration(
    "pulse_reflect_attr", "Marks a struct or class for static reflection");

}  // namespace

}  // namespace pulse::reflect::plugin
