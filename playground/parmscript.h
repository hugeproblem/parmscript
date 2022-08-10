#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

class Parm;
using ParmPtr = std::shared_ptr<Parm>;

class Parm
{
public:
  struct float2 { float x,y; };
  struct float3 { float x,y,z; };
  struct float4 { float x,y,z,w; };
  struct color  { float r,g,b,a; };
  using  std::string;
  using  hashmap=std::unordered_map;

  enum class value_type_tag : size_t {
    NONE,           BOOL, INT, FLOAT, FLOAT2, FLOAT3, FLOAT4, COLOR, STRING};
  using value_type = std::variant<
    std::monostate, bool, int, float, float2, float3, float4, color, string>;
  enum class ui_type_tag {
    FIELD, LABEL, BUTTON, MENU, GROUP, ENDGROUP, STRUCT, ENDSTRUCT};

protected:
  value_type_tag              value_type_;
  ui_type_tag                 ui_type_;
  value_type                  value_;
  value_type                  default_;
  string                      label_;
  hashmap<stirng, value_type> meta_;
  std::vector<int>            menu_values_;
  std::vector<string>         menu_items_;
  std::vector<string>         menu_labels_;
  std::vector<ParmPtr>        fields_;
};


class ParmSet
{
public:
  using  std::string;
  using  hashmap=std::unordered_map;

protected:
  std::vector<ParmPtr>     parms_;
  hashmap<string, ParmPtr> parmlut_;
};

