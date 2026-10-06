#include "clang/Basic/ParsedAttrInfo.h"
#include "tools/reflect/attribute.h"

namespace pulse::reflect::plugin {

namespace {

clang::ParsedAttrInfoRegistry::Add<PulseReflectAttrInfo> attribute_registration(
    "pulse_reflect_attr", "Marks a class for static reflection");

}  // namespace

}  // namespace pulse::reflect::plugin
