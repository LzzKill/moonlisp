/*
 * Code: constant.ixx
 *
 * @Author LzzKill
 * @License BSD4-Clause License
 *
 *
 * */

module;
#include <array>
#include <string_view>
#include <string>
export module moonlisp.constant;

export namespace moonlisp
{
  constexpr std::string_view VERSION{"0.1.0"};
  constexpr std::string_view AUTHOR{"LzzKill"};
  constexpr std::string_view LICENSE{"BSD-4-Clause"};

  constexpr std::array SYMBOL_TABLE = {
      '[', ']', '(', ')', '!', '+', '-', '=', '*',
      '<', '>', '*', '&', '^', '%', ',', '.', '\''}; // {}可作为字面量一部分
  constexpr std::array SPACE_TABLE = {' ', '\f', '\t'};
  constexpr std::array NUMBER_TABLE = {'0', '1', '2', '3', '4',
                                       '5', '6', '7', '8', '9'};
  constexpr std::array NEXT_TABLE = {'\n', '\r'};
  constexpr std::array NOTE_TABLE = {';', '#'};


  // Define Type
  using Place = std::array<int, 3>;

} // namespace moonlisp
