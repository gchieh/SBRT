#ifndef __MATRIX_H
#define __MATRIX_H


#if ( (defined(__MACH__)) && (defined(__APPLE__)) )   
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <OpenGL/gl.h>
#include <GLUT/glut.h>
#include <OpenGL/glext.h>
#else
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <GL/glext.h>
#endif


void crossProduct( float *a, float *b, float *res);
void normalize(float *a);
void setIdentityMatrix( float *mat, int size);
void multMatrix(float *a, float *b);
void setTranslationMatrix(float *mat, float x, float y, float z);

#endif //end matrix.h