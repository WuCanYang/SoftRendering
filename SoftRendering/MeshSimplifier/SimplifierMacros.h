#pragma once

#define check(expr)	\
{\
	if(!(expr))	\
	{\
		std::cout << __FILE__ << "  " << __LINE__ << " condition not match" << std::endl;\
	}\
}


#define  MAX_TEXCOORDS 4