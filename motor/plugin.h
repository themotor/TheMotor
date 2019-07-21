#ifndef _MOTOR_PLUGIN_H_
#define _MOTOR_PLUGIN_H_

#include <cassert>
#include <memory>
#include <type_traits>

namespace motor
{
template <typename Base>
class SinglePluginRegistry
{
 public:
  template <typename Derived>
  class Set
  {
   private:
    static std::unique_ptr<Base> Create()
    {
      static_assert(std::is_base_of<Base, Derived>::value);
      return std::make_unique<Derived>();
    }

   public:
    Set()
    {
      assert(create == nullptr && "A plugin has already been registered!");
      create = Create;
    }
  };

  static std::unique_ptr<Base> Create()
  {
    assert(create != nullptr && "No plugin has been registered");
    return create();
  }

 private:
  static std::unique_ptr<Base> (*create)();
};

template <typename T>
std::unique_ptr<T> (*SinglePluginRegistry<T>::create)() = nullptr;

}  // namespace motor

#endif
