#ifndef TMR_PARAMETERIZED_OBJECT_HPP_
#define TMR_PARAMETERIZED_OBJECT_HPP_

#include <array>
#include <stdexcept>
#include <string>
#include <type_traits>

#include "tm_robot_listener/detail/tmr_constexpr_string.hpp"
#include "tm_robot_listener/detail/tmr_mt_helper.hpp"
#include "tm_robot_listener/detail/tmr_stringifier.hpp"
#include "tmr_variable.hpp"

namespace tm_robot_listener {

struct AttributeOwner {
  std::string name_;
};

template <typename MappingRule>
struct Item {
  detail::ConstString item_name_;

  constexpr Item(detail::ConstString const t_str) noexcept : item_name_{t_str} {}

  /**
   * @brief
   *
   * @param t_idx
   * @return auto
   *
   * @note  this factory prevents user from using Attribute with function "declare" (since this can only be
   *        r-value), which is a good thing.
   *
   * @note use fundamental type instead of index type
   */
  template <typename T>
  auto operator[](T const& t_idx) const noexcept {
    constexpr MappingRule mp{};
    return mp.apply_mapping(*this, t_idx);
  }
};

template <typename Attribute>
struct DefaultMapping {
  template <typename T>
  auto apply_mapping(T const& t_map_holder, std::string const& t_key) const noexcept {
    return Attribute{t_map_holder.item_name_.to_std_str() + '[' + lexical_cast_string<std::string>(t_key) + "]."};
  }
};

#define RW_ATTRIBUTE(AttributeName, ...) \
  Variable<__VA_ARGS__> AttributeName { name_ + #AttributeName }
#define R_ATTRIBUTE(AttributeName, ...) \
  Variable<__VA_ARGS__> const AttributeName { name_ + #AttributeName }

struct PointAttribute : AttributeOwner {
  explicit PointAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  RW_ATTRIBUTE(Value, std::array<float, 6>);
  RW_ATTRIBUTE(Pose, std::array<int, 3>);

  R_ATTRIBUTE(Flange, std::array<float, 6>);
  R_ATTRIBUTE(BaseName, std::string);
  R_ATTRIBUTE(TCPName, std::string);
  R_ATTRIBUTE(TeachValue, std::array<float, 6>);
  R_ATTRIBUTE(TeachPose, std::array<int, 3>);
};

constexpr auto Point = Item<DefaultMapping<PointAttribute>>{"Point"};

struct BaseAttribute : AttributeOwner {
  explicit BaseAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  RW_ATTRIBUTE(Value, std::array<float, 6>);

  R_ATTRIBUTE(Type, std::string);
  R_ATTRIBUTE(TeachValue, std::array<float, 6>);
};

constexpr auto Base = Item<DefaultMapping<BaseAttribute>>{"Base"};

/**
 * @brief Tag dispatch for TCP attribute mapping
 */
constexpr struct NOTOOLTag { static constexpr auto TCP_NAME = "NOTOOL"; } NOTOOL;

/**
 * @brief Tag dispatch for TCP attribute mapping
 */
constexpr struct HandCameraTag { static constexpr auto TCP_NAME = "HandCamera"; } HandCamera;

template <typename T>
struct is_tcp_tool : std::false_type {};

template <>
struct is_tcp_tool<NOTOOLTag> : std::true_type {};

template <>
struct is_tcp_tool<HandCameraTag> : std::true_type {};

template <typename T>
struct TCPAttribute : AttributeOwner {
  static constexpr auto IS_SYSTEM_TCP = std::is_same<T, NOTOOLTag>::value or std::is_same<T, HandCameraTag>::value;
  static_assert(is_tcp_tool<T>::value, "Not a valid tcp tool");
  static_assert(IS_SYSTEM_TCP or (T::TCP_NAME != "HandCamera" and T::TCP_NAME != "NOTOOL"), "Invalid tcp tool name");

  explicit TCPAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  template <typename U>
  using AttributeType = typename tmr_mt_helper::const_if<IS_SYSTEM_TCP, Variable<U>>::type;

  AttributeType<std::array<float, 6>> Value{name_ + "Value"};
  AttributeType<float> Mass{name_ + "Mass"};
  AttributeType<std::array<float, 3>> MOI{name_ + "MOI"};
  AttributeType<std::array<float, 6>> MCF{name_ + "MCF"};

  R_ATTRIBUTE(TeachValue, std::array<float, 6>);
  R_ATTRIBUTE(TeachMass, float);
  R_ATTRIBUTE(TeachMOI, std::array<float, 3>);
  R_ATTRIBUTE(TeachMCF, std::array<float, 6>);
};

struct TCPMapping {
  template <typename T, typename U>
  auto apply_mapping(T const& t_map, U const& /**/) const noexcept {
    return TCPAttribute<U>{t_map.item_name_.to_std_str() + '[' + lexical_cast_string<std::string>(U::TCP_NAME) + "]."};
  }
};

constexpr auto TCP = Item<TCPMapping>{"TCP"};

struct VPointAttribute : public AttributeOwner {
  explicit VPointAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  RW_ATTRIBUTE(Value, std::array<float, 6>);

  R_ATTRIBUTE(BaseName, std::string);
  R_ATTRIBUTE(TeachValue, std::array<float, 6>);
};

constexpr auto VPoint = Item<DefaultMapping<VPointAttribute>>{"VPoint"};

/**
 * @brief
 *
 */
constexpr struct ControlBoxTag {
  static constexpr auto DI = 16;
  static constexpr auto DO = 16;
  static constexpr auto AI = 1;
  static constexpr auto AO = 2;

  static constexpr auto IO_NAME = "ControlBox";
} ControlBox;

constexpr struct EndModuleTag {
  static constexpr auto DI = 16;
  static constexpr auto DO = 16;
  static constexpr auto AI = 1;
  static constexpr auto AO = 2;

  static constexpr auto IO_NAME = "EndModule";
} EndModule;

template <std::size_t N>
struct [[deprecated("not implemented yet")]] ExternalModule {};

constexpr struct SafetyTag { static constexpr auto IO_NAME = "Safety"; } Safety;

template <typename T>
struct IOAttribute : AttributeOwner {
  // static_assert()
  explicit IOAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  R_ATTRIBUTE(DI, std::array<std::uint8_t, T::DI>);
  RW_ATTRIBUTE(DO, std::array<std::uint8_t, T::DO>);
  R_ATTRIBUTE(AI, std::array<float, T::AI>);
  RW_ATTRIBUTE(AO, std::array<float, T::AO>);
  RW_ATTRIBUTE(InstantDO, std::array<std::uint8_t, T::DO>);
  RW_ATTRIBUTE(InstantAO, std::array<float, T::AO>);
};

template <>
struct IOAttribute<SafetyTag> : public AttributeOwner {
  explicit IOAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  R_ATTRIBUTE(SI, std::array<std::uint8_t, 5>);
  R_ATTRIBUTE(SO, std::array<std::uint8_t, 5>);
};

struct IOMapping {
  template <typename T, typename IOClass>
  auto apply_mapping(T const& t_map, IOClass const /*unused*/) const noexcept {
    auto const name = t_map.item_name_.to_std_str() + '[' + lexical_cast_string<std::string>(IOClass::IO_NAME) + "].";
    return IOAttribute<IOClass>{name};
  }
};

constexpr auto IO = Item<IOMapping>{"IO"};

struct RobotAttribute : AttributeOwner {
  explicit RobotAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  R_ATTRIBUTE(CoordRobot, std::array<float, 6>);
  R_ATTRIBUTE(CoordBase, std::array<float, 6>);
  R_ATTRIBUTE(Joint, std::array<float, 6>);
  R_ATTRIBUTE(BaseName, std::string);
  R_ATTRIBUTE(TCPName, std::string);
  RW_ATTRIBUTE(CameraLight, int);
  R_ATTRIBUTE(TCPForce3D, float);
  R_ATTRIBUTE(TCPSpeed3D, float);
};

struct RobotMapping {
  template <typename T>
  auto apply_mapping(T const& t_map, int const t_index) const {
    if (t_index != 0) throw std::invalid_argument{"The index of the robot fixed at 0"};
    auto const name = t_map.item_name_.to_std_str() + "[0].";
    return RobotAttribute{name};
  }
};

constexpr auto Robot = Item<RobotMapping>{"Robot"};

struct FTAttribute : AttributeOwner {
  explicit FTAttribute(std::string const& t_str) noexcept : AttributeOwner{t_str} {}

  R_ATTRIBUTE(X, float);
  R_ATTRIBUTE(Y, float);
  R_ATTRIBUTE(Z, float);
  R_ATTRIBUTE(TX, float);
  R_ATTRIBUTE(TY, float);
  R_ATTRIBUTE(TZ, float);
  R_ATTRIBUTE(F3D, float);
  R_ATTRIBUTE(T3D, float);
  R_ATTRIBUTE(ForceValue, std::array<float, 3>);
  R_ATTRIBUTE(TorqueValue, std::array<float, 3>);
  R_ATTRIBUTE(RefCoorX, float);
  R_ATTRIBUTE(RefCoorY, float);
  R_ATTRIBUTE(RefCoorZ, float);
  R_ATTRIBUTE(RefCoorTX, float);
  R_ATTRIBUTE(RefCoorTY, float);
  R_ATTRIBUTE(RefCoorTZ, float);
  R_ATTRIBUTE(RefCoorF3D, float);
  R_ATTRIBUTE(RefCoorT3D, float);
  R_ATTRIBUTE(RefCoorForceValue, std::array<float, 3>);
};

constexpr auto FT = Item<DefaultMapping<FTAttribute>>{"FT"};

}  // namespace tm_robot_listener

#endif