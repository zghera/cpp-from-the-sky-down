﻿// tagged_sqlite.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <type_traits>
#include "..//simple_type_name/simple_type_name.h"
#include "..//tagged_tuple/tagged_tuple.h"

template <typename Tag, typename... Tables>
struct define_database : Tables... {
  using database_tag_type = Tag;
};

template <typename Tag, typename... Columns>
struct define_table : Columns... {
  using table_tag_type = Tag;
};

template <typename Tag, typename Type>
struct define_column {
  using tag_type = Tag;
  using value_type = Type;
};

namespace detail {
template <typename T>
struct t2t {
  using type = T;
};

template <typename Tag, typename... Columns>
auto get_table_type(const define_table<Tag, Columns...>& t)
    -> t2t<define_table<Tag, Columns...>>;
template <typename Tag>
auto get_table_type(...) -> t2t<void>;

template <typename Tag, typename Type>
auto get_column_type(const define_column<Tag, Type>& t)
    -> t2t<define_column<Tag, Type>>;

template <typename Tag>
auto get_column_type(...) -> t2t<void>;

template <typename Database, typename Tag>
using table_type = typename decltype(get_table_type<Tag>(Database{}))::type;

template <typename Database, typename TableTag, typename ColumnTag>
using table_column_type = typename decltype(
    get_column_type<ColumnTag>(table_type<Database, TableTag>{}))::type;

template <typename Database, typename ColumnTag>
using column_type =
    typename decltype(get_column_type<ColumnTag>(Database{}))::type;

template <typename Database, typename Tag>
inline constexpr bool has_table =
    !std::is_same_v<decltype(get_table_type<Tag>(Database{})), t2t<void>>;

template <typename Database, typename TableTag, typename Tag>
inline constexpr bool has_column =
    decltype(test_has_column(table_type<Database, TableTag>{}))::value;

template <typename Tag, typename DbTag, typename... Tables>
constexpr bool has_unique_column_helper(define_database<DbTag, Tables...> db) {
  return (has_column<decltype(db), typename Tables::table_tag_type, Tag> ^ ...);
}

template <typename Database, typename Tag>
constexpr bool has_unique_column = has_unique_column_helper<Tag>(Database{});

}  // namespace detail

template <typename Alias, typename ColumnName, typename TableName>
struct column_alias_ref {};

template <typename E>
struct expression {
  E e_;

  template <typename Alias>
  constexpr auto as() const {
    return e_.as<Alias>();
  }
};

template <typename ColumnName, typename TableName = void>
struct column_ref {
  template <typename Alias>
  constexpr column_alias_ref<Alias, ColumnName, TableName> as() const {
    return {};
  }
  expression<column_ref> operator()() const { return {*this}; }
};

template <typename N1, typename N2>
struct column_ref_definer {
  using type = column_ref<N2, N1>;
};

template <typename Name, typename T>
struct parameter_ref {};

template <typename Name, typename T>
struct parameter_value {
  T t_;
};

enum class binary_ops {
  equal_ = 0,
  not_equal_,
  less_,
  less_equal_,
  greater_,
  greater_equal_,
  and_,
  or_,
  add_,
  sub_,
  mul_,
  div_,
  mod_,
  size_
};

template <typename Enum>
constexpr auto to_underlying(Enum e) {
  return static_cast<std::underlying_type_t<Enum>>(e);
}

inline constexpr std::array<std::string_view, to_underlying(binary_ops::size_)>
    binary_ops_to_string{
        "=",    "!=", "<", "<=", ">", ">=", " and ",
        " or ", "+",  "-", "*",  "/", "%",
    };

template <binary_ops bo, typename E1, typename E2>
struct binary_expression {
  E1 e1_;
  E2 e2_;
};

template <binary_ops bo, typename E1, typename E2>
auto make_binary_expression(E1 e1, E2 e2) {
  return binary_expression<bo, E1, E2>{std::move(e1), std::move(e2)};
}

template <typename E1, typename E2>
auto operator==(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::equal_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator!=(expression<E1> e1, expression<E2> e2) {
  return make_expression(make_binary_expression<binary_ops::not_equal_>(
      std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator<(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::less_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator<=(expression<E1> e1, expression<E2> e2) {
  return make_expression(make_binary_expression<binary_ops::less_equal_>(
      std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator>(expression<E1> e1, expression<E2> e2) {
  return make_expression(make_binary_expression<binary_ops::greater_>(
      std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator>=(expression<E1> e1, expression<E2> e2) {
  return make_expression(make_binary_expression<binary_ops::greater_equal_>(
      std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator&&(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::and_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator||(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::or_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator+(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::add_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator-(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::sub_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator*(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::mul_>(std::move(e1), std::move(e2)));
}

template <typename E1, typename E2>
auto operator/(expression<E1> e1, expression<E2> e2) {
  return make_expression(
      make_binary_expression<binary_ops::div_>(std::move(e1), std::move(e2)));
}

template <binary_ops bo, typename E1, typename E2>
std::string expression_to_string(
    const expression<binary_expression<bo, E1, E2>>& e) {
  return expression_to_string(e.e_.e1_) +
         std::string(binary_ops_to_string[to_underlying(bo)]) +
         expression_to_string(e.e_.e2_);
}

template <typename E>
auto make_expression(E e) {
  return expression<E>{std::move(e)};
}

template <typename T>
struct constant_holder {
  T e_;
};

template <typename T>
std::string expression_to_string(const expression<constant_holder<T>>& c) {
  return std::to_string(c.e_.e_);
}

inline std::string expression_to_string(
    const expression<constant_holder<std::string>>& s) {
  return s.e_.e_;
}

template <typename T>
constant_holder<T> make_constant(T t) {
  return {std::move(t)};
}

inline auto constant(std::string s) {
  return make_expression(make_constant(std::move(s)));
}

inline auto constant(int i) {
  return make_expression(make_constant(std::int64_t{i}));
}
inline auto constant(std::int64_t i) {
  return make_expression(make_constant(i));
}

inline auto constant(double d) { return make_expression(make_constant(d)); }

template <typename Column, typename Table>
std::string expression_to_string(const expression<column_ref<Column, Table>>&) {
  if constexpr (std::is_same_v<Table, void>) {
    return std::string(simple_type_name::short_name<Column>);
  } else {
    return std::string(simple_type_name::short_name<Table>) + "." +
           std::string(simple_type_name::short_name<Column>);
  }
}

template <typename Name, typename T>
std::string expression_to_string(expression<parameter_ref<Name, T>>) {
  return " :" + std::string(simple_type_name::short_name<Name>) + " ";
}

std::string expression_to_string(expression<std::int64_t> i) {
  return std::to_string(i.e_);
}

template <typename Name, typename T>
struct parameter_object {
  expression<parameter_ref<Name, T>> operator()() const { return {}; }

  parameter_value<Name, T> operator()(T t) const { return {std::move(t)}; }
};

template <typename Name, typename T>
inline constexpr auto parameter = parameter_object<Name, T>{};
template <typename N1>
struct column_ref_definer<N1, void> {
  using type = column_ref<N1, void>;
};

template <typename... ColumnRefs>
struct column_ref_holder {};

template <typename ColumnRef, typename NewName>
struct as_ref {};

template <typename N1, typename N2 = void>
inline constexpr auto column =
    expression<typename column_ref_definer<N1, N2>::type>{};

template <typename Table>
struct table_ref {};

template <typename Table>
inline constexpr auto table = table_ref<Table>{};

template <typename... Tables>
struct from_type {};

template <typename... ColumnRefs>
struct select_type {};

template <typename T1, typename T2>
struct catter_imp;

template <typename... M1, typename... M2>
struct catter_imp<tagged_tuple::ttuple<M1...>, tagged_tuple::ttuple<M2...>> {
  using type = tagged_tuple::ttuple<M1..., M2...>;
};

template <typename T1, typename T2>
using cat_t = typename catter_imp<T1, T2>::type;

class from_tag;
class select_tag;

template <typename Database, typename TTuple = tagged_tuple::ttuple<>>
struct query_builder {
  template <typename Database, typename TTuple>
  static auto make_query_builder(TTuple t) {
    return query_builder<Database, TTuple>{std::move(t)};
  }

  TTuple t_;

  template <typename... Tables>
  // query_builder<
  //     Database, cat_t<TTuple, >>
  auto from(table_ref<Tables>...) && {
    return make_query_builder<Database>(
        std::move(t_) |
        tagged_tuple::make_ttuple(
            tagged_tuple::make_member<from_tag>(from_type<Tables...>{})));
  }

  template <typename... Columns>
  auto select(Columns...) && {
    return make_query_builder<Database>(
        std::move(t_) |
        tagged_tuple::make_ttuple(
            tagged_tuple::make_member<select_tag>(select_type<Columns...>{})));
  }
};

template <typename Column, typename Table>
std::string to_column_string(expression<column_ref<Column, Table>>) {
  if constexpr (std::is_same_v<Table, void>) {
    return std::string(simple_type_name::short_name<Column>);
  } else {
    return std::string(simple_type_name::short_name<Table>) + "." +
           std::string(simple_type_name::short_name<Column>);
  }
}

template <typename Alias, typename Column, typename Table>
std::string to_column_string(column_alias_ref<Alias, Column, Table>) {
  return to_column_string(expression<column_ref<Column, Table>>{}) + " AS " +
         std::string(simple_type_name::short_name<Alias>);
}

#include <vector>

inline std::string join_vector(const std::vector<std::string>& v) {
  std::string ret;
  for (auto& s : v) {
    ret += s + ",";
  }
  if (ret.back() == ',') ret.pop_back();

  return ret;
}

template <typename... Columns>
std::string to_statement(select_type<Columns...>) {
  std::vector<std::string> v{to_column_string(Columns{})...};

  return std::string("SELECT ") + join_vector(v);
}

template <typename... Tables>
std::string to_statement(from_type<Tables...>) {
  std::vector<std::string> v{
      std::string(simple_type_name::short_name<Tables>)...};
  return std::string("\nFROM ") + join_vector(v);
}

template <typename Database, typename TTuple>
std::string to_statement(const query_builder<Database, TTuple>& t) {
  std::string ret;
  if constexpr (tagged_tuple::has_tag<select_tag, TTuple>) {
    ret += to_statement(get<select_tag>(t.t_));
  }

  if constexpr (tagged_tuple::has_tag<from_tag, TTuple>) {
    ret += to_statement(get<from_tag>(t.t_));
  }
  return ret;
}

#include <iostream>

// TODO: Reference additional headers your program requires here.