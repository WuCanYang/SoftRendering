#include "Hash.h"

unsigned int HashPoint(const Vector3& p)
{
	union { float f; unsigned int i; } x;
	union { float f; unsigned int i; } y;
	union { float f; unsigned int i; } z;

	x.f = p._x;
	y.f = p._y;
	z.f = p._z;

	return Murmur32({ x.i, y.i, z.i });
}


unsigned int Murmur32(std::initializer_list< unsigned int > InitList)
{
	unsigned int Hash = 0;
	for (auto Element : InitList)
	{
		Element *= 0xcc9e2d51;
		Element = (Element << 15) | (Element >> (32 - 15));
		Element *= 0x1b873593;

		Hash ^= Element;
		Hash = (Hash << 13) | (Hash >> (32 - 13));
		Hash = Hash * 5 + 0xe6546b64;
	}

	return MurmurFinalize32(Hash);
}


unsigned int MurmurFinalize32(unsigned int Hash)
{
	Hash ^= Hash >> 16;
	Hash *= 0x85ebca6b;
	Hash ^= Hash >> 13;
	Hash *= 0xc2b2ae35;
	Hash ^= Hash >> 16;
	return Hash;
}