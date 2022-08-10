#pragma once

#include <cctype>
#include <functional>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
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

  enum class value_type_enum : size_t {
    NONE,           BOOL, INT, FLOAT, FLOAT2, FLOAT3, FLOAT4, COLOR, STRING};
  using value_type = std::variant<
    std::monostate, bool, int, float, float2, float3, float4, color, string>;
  enum class ui_type_enum {
    FIELD, LABEL, BUTTON, MENU, GROUP, STRUCT, LIST};

protected:
  ui_type_enum                ui_type_=ui_type_tag::LABEL;
  value_type                  value_;
  value_type                  default_;
  string                      name_;
  string                      label_;
  hashmap<stirng, value_type> meta_;
  std::vector<int>            menu_values_;
  std::vector<string>         menu_items_;
  std::vector<string>         menu_labels_;
  // if the scope is a plain struct, then this holds everything
  std::vector<ParmPtr>        fields_; 
  // if the scope is a list, fields_ holds the template (label / default value / everything)
  // and listValues_ holds the values.
  // e.g., for a list of {string name; int value;} pairs
  // fields_[0] == {def of string name}, fields_[1] = {def of int value}
  // listValues_[0].fields_[0] = value of first string,  i.e., list[0].name
  // listValues_[0].fields_[1] = value of first int,     i.e., list[0].value
  // listValues_[1].fields_[0] = value of second string, i.e., list[1].name
  // listValues_[1].fields_[1] = value of second int,    i.e., list[1].value
  std::vector<ParmPtr>        listValues_;


  static string titleize(string s) {
    bool space = true;
    for (auto& c: s) {
      if (std::isspace(c)) {
        space = true;
      } else if (space) {
        c = std::toupper(c);
        space = false;
      }
    }
    return s;
  }

public:
  Parm()=default;
  Parm(Parm const&)=default;
  Parm(Parm&&)=default;
  ~Parm()=default;

  void setName(string name) { name_ = std::move(name); }
  void setUI(ui_type_enum type) {ui_type_ = type; }
  void setAsField(value_type defaultValue) {
    ui_type_ = ui_type_enum::FIELD;
    value_ = defaultValue;
    default_ = defaultValue;
  }
  void setLabel(string label) { label_ = std::move(label); }
  void setMeta(string const& key, value_type value) { meta_[key] = std::move(value_); }
  void setMenu(std::vector<string> items, int defaultValue, std::vector<string> labels={}, std::vector<int> values={}) {
    ui_type_ = ui_type_enum::MENU;
    menu_items_ = std::move(items);
    value_ = defaultValue;
    default_ = defaultValue;

    if (labels.size() == menu_items_.size())
      menu_labels_ = std::move(menu_labels_);
    else {
      menu_labels_.resize(menu_items_.size());
      std::transform(menu_items_.begin(), menu_items_.end(), menu_labels_.begin(), titleize);
    }

    if (values.size() == menu_items_.size())
      menu_values_ = std::move(menu_values_);
    else {
      menu_values_.resize(menu_values_.size());
      std::iota(menu_values_.begin(), menu_values_.end(), 0);
    }
  }
  void addField(ParmPtr child) {
    fields_.push_back(child);
  }

  auto const& name() const { return name_; }
  auto const& value() const { return value_; }
  auto const& defaultValue() const { return defaultValue_; }

  // retrieve value:
  template <class T>
  auto as() const { return std::get<T>(value_); }
  template <class T>
  auto get() const { return std::get_if<T>(value_); }

  auto    numFields() const { return fields_.size(); }
  ParmPtr getField(size_t i) const {
    if (i<fields_.size())
      return fields_[i];
    return nullptr;
  }

  // retrieve list item:
  auto numListValues() const {
    return listValues_.size();
  }
  template <class T>
  T* getListValue(size_t i, size_t f) {
    if (i<listValues_.size()) {
      assert(listValues_[i]);
      assert(listValues_[i]->fields_.size() == fields_.size());
      if (f<listValues_[i]->fields_.size()) {
        assert(listValues_[i]->fields_[f]);
        return listValues_[i]->fields_[f]->get<T>();
      }
    }
    return nullptr;
  }

  // set values:
  template <class T>
  void set(T value) {
    value_ = std::move(value);
  }

  // set list:
  void resizeList(size_t cnt) {
    if (auto oldsize=listValues_.size(); oldsize<cnt) {
      for (auto i=oldsize; i<cnt; ++i) {
        listValues_.push_back(std::make_shared<Parm>())
        for (auto field : fields_) {
          listValues_.back()->fields_.push_back(std::make_shared(*field));
        }
      }
    } else {
      listValues_.resize(cnt);
    }
  }
};


class ParmSet
{
public:
  using  std::string;
  using  hashmap=std::unordered_map;

protected:
  std::vector<ParmPtr>       parms_;
  hashmap<string, ParmPtr>   parmlut_;
  std::unordered_set<string> dirty_;

public:
  void updateInspector();
  auto const& dirtyEntries() const { return dirty_; }

  ParmPtr get(string const& key) {
    if (auto itr=parmlut_.find(key); itr!=parmlut_.end())
      return itr->second;
    return nullptr;
  }
  Parm& operator[](string const& key) {
    return *parmlut_.at(key);
  }
};

