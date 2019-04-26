#pragma once 
#ifndef COMMON_H_
#define COMMON_H_

#include <assert.h>

#define SRGB_COMBINATIONS 16777216 // 256*256*256
#define SRGB_FULL_DIMENSION 4096

#define MAX(_A, _B) ((_A) > (_B) ? (_A) : (_B))
#define MIN(_A, _B) ((_A) < (_B) ? (_A) : (_B))
#define CLAMP(_A, _MIN, _MAX) ( _A > _MAX ? (_MAX) : (_A < _MIN ? (_MIN) : (_A) ) )

#ifndef STRIFY
	#define _STRIFY(_PD) #_PD
	#define STRIFY(_PD) _STRIFY(_PD)
#endif

#define PING() {fprintf(stderr,"PING! (%d:%s)\n",__LINE__,__FILE__);}

#define ASSERT_NO_GLERR() {\
	GLenum glErr = glGetError();\
	if(glErr != GL_NO_ERROR){\
		fprintf(stderr, "OpenGL Error on %s:%i\n", __FILE__, __LINE__);\
		switch(glErr){\
			case 0x0500:\
				fprintf(stderr, "'Invalid OpenGL enum was passed to function.\n'");\
				break;\
			case 0x0501:\
				fprintf(stderr, "'Invalid value was passed to an OpenGL function.\n'");\
				break;\
			case 0x0502:\
				fprintf(stderr, "'Invalid operation error, the OpenGL state conflicted with a command.\n'");\
				break;\
			case 0x0503:\
				fprintf(stderr, "'OpenGL stack overflow.\n'");\
				break;\
			case 0x0504:\
				fprintf(stderr, "'OpenGL stack underflow.\n'");\
				break;\
			case 0x0505:\
				fprintf(stderr, "'OpenGL out of memory error!\n'");\
				break;\
			case 0x0506:\
				fprintf(stderr, "'Invalid framebuffer operation!\n'");\
				break;\
			case 0x0507:\
				fprintf(stderr, "'The OpenGL context was lost!\n'");\
				break;\
			\
		}\
		assert(false);\
	}\
}

#define CHECKED_GL_CALL(_CALL) (_CALL); ASSERT_NO_GLERR()

#ifndef BASETYPES
	#define BASETYPES
	typedef unsigned UINT;
	typedef unsigned long ULONG;
	typedef ULONG *PULONG;
	typedef unsigned short USHORT;
	typedef USHORT *PUSHORT;
	typedef unsigned char UCHAR;
	typedef UCHAR *PUCHAR;
#endif


#endif