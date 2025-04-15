#pragma once

#include <concepts>

template<typename T>
concept NonVoidType = !std::is_void_v<T>;
