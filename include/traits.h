#pragma once


namespace ts
{

template<typename T>
struct is_reference : std::false_type
{};

template<typename T>
struct is_reference<T&> : std::true_type
{};

template<typename T>
constexpr bool is_reference_v{is_reference<T>::value};

}//ts

