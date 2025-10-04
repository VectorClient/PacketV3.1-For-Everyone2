#pragma once

typedef void* UnorderedMap;

class Remapper {
public:
	static void remapFunction(UnorderedMap map);

	static void remapOffset(UnorderedMap map);
};
