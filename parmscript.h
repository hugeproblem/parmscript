#pragma once

#include <algorithm>
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
class ParmSet;
using ParmPtr = std::shared_ptr<Parm>;

typedef struct lua_State lua_State;

class Parm
{
public:
  struct float2 { float x,y; };
  struct float3 { float x,y,z; };
  struct float4 { float x,y,z,w; };
  struct color  { float r,g,b,a; };
  using  string=std::string;
  template <class K, class V>
  using  hashmap=std::unordered_map<K,V>;
  template <class T>
  using  hashset=std::unordered_set<T>;

  enum class value_type_enum : size_t {
    NONE,           BOOL, INT, FLOAT, DOUBLE, FLOAT2, FLOAT3, FLOAT4, COLOR, STRING};
  using value_type = std::variant<
    std::monostate, bool, int, float, double, float2, float3, float4, color, string>;
  enum class ui_type_enum {
    FIELD, LABEL, BUTTON, SPACER, SEPARATOR, MENU, GROUP, STRUCT, LIST};

protected:
  ParmSet                    *root_=nullptr;
  ui_type_enum                ui_type_=ui_type_enum::LABEL;
  value_type_enum             expected_value_type_;
  value_type                  value_;
  value_type                  default_;
  string                      name_;
  string                      path_;
  string                      label_;
  hashmap<string, value_type> meta_;
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


  static string titleize(string s)
  {
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

  template <class T>
  T getMeta(std::string const& key, T const& defaultval)
  {
    if(auto itr=meta_.find(key); itr!=meta_.end()) {
      if (std::holds_alternative<T>(itr->second)) {
        return std::get<T>(itr->second);
      } else {
        fprintf(stderr, "bad type held in %s: %d\n", key.c_str(), itr->second.index());
      }
    }
    return defaultval;
  }

public:
  Parm(ParmSet* root):root_(root){}
  Parm(Parm&&)=default;
  ~Parm()=default;
  Parm(Parm const& that)
    : root_(that.root_)
    , ui_type_(that.ui_type_)
    , value_(that.value_)
    , expected_value_type_(that.expected_value_type_)
    , default_(that.default_)
    , name_(that.name_)
    , path_(that.path_)
    , label_(that.label_)
    , meta_(that.meta_)
    , menu_values_(that.menu_values_)
    , menu_items_(that.menu_items_)
    , menu_labels_(that.menu_labels_)
  {
    fields_.reserve(that.fields_.size());
    for (auto f: that.fields_)
      fields_.push_back(std::make_shared<Parm>(*f));
    listValues_.reserve(that.listValues_.size());
    for (auto v: that.listValues_)
      listValues_.push_back(std::make_shared<Parm>(*v));
  }

  auto const& name() const { return name_; }
  auto const& label() const { return label_; }
  auto const& path() const { return path_; }
  auto const& value() const { return value_; }
  auto const& defaultValue() const { return default_; }
  auto const  type() const { return expected_value_type_; }
  auto const  ui() const { return ui_type_; }

  // retrieve value:
  template <class T>
  auto as() const { return std::get<T>(value_); }

  // special case for menu:
  template <>
  auto as<int>() const {
    if (ui_type_ == ui_type_enum::MENU) {
      int idx = std::get<int>(value_);
      if (menu_values_.size() == menu_items_.size() && idx>=0 && idx<menu_values_.size()) {
        return menu_values_[idx];
      } else {
        return -1;
      }
    } else {
      return std::get<int>(value_);
    }
  }
  template <>
  auto as<string>() const {
     if (ui_type_ == ui_type_enum::MENU) {
      int idx = std::get<int>(value_);
      if (idx>=0 && idx<menu_items_.size()) {
        return menu_items_[idx];
      } else {
        return string();
      }
    } else {
      return std::get<string>(value_);
    }   
  }

  auto    numFields() const { return fields_.size(); }
  ParmPtr getField(size_t i) const {
    if (i<fields_.size())
      return fields_[i];
    return nullptr;
  }

  ParmPtr getField(string const& relpath);

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
        return listValues_[i]->fields_[f]->as<T>();
      }
    }
    return nullptr;
  }
  
  // set values:
  template <class T>
  bool set(T value) {
    if (auto* ptr = std::get_if<T>(&value_)) {
      *ptr = std::move(value);
      return true;
    }
    return false;
  }

  // set list:
  void resizeList(size_t cnt) {
    if (auto oldsize=listValues_.size(); oldsize<cnt) {
      for (auto i=oldsize; i<cnt; ++i) {
        auto newItem = std::make_shared<Parm>(root_);
        string indexstr = "["+std::to_string(i)+"]";
        newItem->setUI(ui_type_enum::STRUCT);
        newItem->setPath(path_+indexstr);
        listValues_.push_back(newItem);
        for (auto f: fields_) {
          auto newField = std::make_shared<Parm>(*f);
          newField->setPath(newItem->path()+"."+f->name());
          newField->setLabel(newField->label()+indexstr);
          newItem->fields_.push_back(newField);
        }
      }
    } else {
      listValues_.resize(cnt);
    }
  }
  template <class T>
  bool setListValue(size_t i, size_t f, T value) {
    return listValues_.at(i)->fields_.at(f)->set(std::move(value));
  }

  bool updateInspector(hashset<string>& dirty, lua_State* L = nullptr);

protected:
  friend class ParmSet;
  // setup function should only be called by ParmSet
  void setName(string name) { name_ = std::move(name); }
  void setPath(string path) { path_ = std::move(path); }
  void setUI(ui_type_enum type) {ui_type_ = type; }
  void setType(value_type_enum type) {expected_value_type_ = type;}
  void setAsField(value_type defaultValue) {
    ui_type_ = ui_type_enum::FIELD;
    value_ = defaultValue;
    default_ = defaultValue;
  }
  void setLabel(string label) { label_ = std::move(label); }
  void setMeta(string const& key, value_type value) { meta_[key] = std::move(value_); }
  template <class T>
  void setMeta(string const& key, T const& value) {
    meta_[key].emplace<T>(value);
  }
  void setMenu(std::vector<string> items, int defaultValue, std::vector<string> labels={}, std::vector<int> values={}) {
    ui_type_ = ui_type_enum::MENU;
    menu_items_ = std::move(items);
    value_ = defaultValue;
    default_ = defaultValue;

    if (labels.size() == menu_items_.size())
      menu_labels_ = std::move(labels);
    else {
      menu_labels_.resize(menu_items_.size());
      std::transform(menu_items_.begin(), menu_items_.end(), menu_labels_.begin(), titleize);
    }

    if (values.size() == menu_items_.size())
      menu_values_ = std::move(values);
    else {
      menu_values_.resize(menu_values_.size());
      std::iota(menu_values_.begin(), menu_values_.end(), 0);
    }
  }
  void setup(string name, string path, string label, ui_type_enum ui, value_type_enum type, value_type defaultValue) {
    if (defaultValue.index() != static_cast<size_t>(type)) {
      throw std::invalid_argument("default value does not match type");
    }
    name_    = std::move(name);
    path_    = std::move(path);
    label_   = std::move(label);
    ui_type_ = ui;
    expected_value_type_ = type;
    default_ = defaultValue;
    value_   = defaultValue;
  }
  void addField(ParmPtr child) {
    fields_.push_back(child);
    if (!listValues_.empty()) {
      for (auto item: listValues_) {
        item->fields_.push_back(std::make_shared<Parm>(*child));
      }
    }
  }
  /* TODO: runtime adjusts on fields
  void removeField(size_t idx) {
  }
  */
};


class ParmSet
{
public:
  using string=std::string;
  template <class K, class V>
  using hashmap=std::unordered_map<K,V>;
  template <class T>
  using hashset=std::unordered_set<T>;

protected:
  ParmPtr                  root_;
  std::vector<ParmPtr>     parms_;
  hashset<string>          dirty_;
  bool                     loaded_ = false;

  static lua_State *defaultLuaRuntime(); // shared lua runtime for parmscript parsing and `disablewhen` expression evaluation

  static int processLuaParm(lua_State* lua);
  static int evalParm(lua_State* lua);

  friend class Parm;

public:
  bool loadScript(string const& s, lua_State* runtime=nullptr); // if no lua runtime was given, default shared lua runtime will be used
  bool updateInspector(lua_State* runtime=nullptr);

  auto const& dirtyEntries() const { return dirty_; }
  bool transferTo(ParmSet& that); // convert this into that, try best to respect other's existing type & format
  bool loaded() const { return loaded_; }

  ParmPtr get(string const& key) {
    return root_->getField(key);
  }
};

