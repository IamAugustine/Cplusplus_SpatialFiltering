#pragma once
#include "FilterBase.h"


class Sobel2D :
	public Filter2D
{

public:
	Sobel2D(Direction direction);
	~Sobel2D();
	ushort Calculate(deque<ushort> data);
	Direction DerivativeDirection;
};

