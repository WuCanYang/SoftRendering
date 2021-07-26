#pragma once
#include <initializer_list>
#include "Math/Vector3.h"

unsigned int HashPoint(const Vector3& p);

unsigned int Murmur32(std::initializer_list< unsigned int > InitList);

unsigned int MurmurFinalize32(unsigned int Hash);
