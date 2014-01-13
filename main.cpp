/*
	Shader Based Ray Tracing v 2.0
	GChieh
	
*/


#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <time.h>

//OpenGL Framework
#if ( (defined(__MACH__)) && (defined(__APPLE__)) ) 
#include <OpenGL/OpenGL.h>
#include <OpenGL/gl3.h>
#include <GLUT/glut.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#endif

// Source headers
#include "textfile.h"
#include "shader.h"
#include "matrix.h"
 


#define M_PI       3.14159265358979323846
const int triangle_num = 12;
const int vertices = 8;
#define DATA_NAME "cube.obj"
using namespace std;
GLuint indicesImage;
GLuint verticesImage;
GLfloat v_1[vertices], v_2[vertices], v_3[vertices],vertex[3*vertices];
GLfloat f_1[triangle_num],f_2[triangle_num],f_3[triangle_num], face[3*triangle_num];

// Data for drawing Axis


float vertices1[] = { 
			-256.0f, 256.0f, 0.0f,1.0f,
            256.0f, 256.0f, 0.0f,1.0f,
            -256.0f, -256.0f, 0.0f,1.0f,
			256.0f, -256.0f, 0.0f,1.0f
			};

// Shader Names
//char *vertexFileName = "trace.vert";
//char *fragmentFileName = "trace.frag";
 
// Program and Shader Identifiers
//GLuint p,v,f;
 
// Vertex Attribute Locations
GLuint vertexLoc;
 
// Uniform variable Locations
GLuint projMatrixLoc, viewMatrixLoc;
 
// Vertex Array Objects Identifiers
GLuint vao[3];
 
// storage for Matrices
float projMatrix[16];
float viewMatrix[16];

//Light Resource
float lpos[4] = {2.0,0.0,2.0,1};
float light_pos[3]={lpos[0],lpos[1],lpos[2]};

Shader shader;





//-----------------------------------------------------
//Calculate FPS
bool finish_without_update = false;

bool odd_frame = false, z_trick = false;
float fps = 0;

//  The number of frames
int frameCount = 0;

//  Number of frames per second
//float fps = 0;

//  currentTime - previousTime is the time elapsed
//  between every call of the Idle function
int currentTime = 0, previousTime = 0;

void calculateFPS()
{
    //  Increase frame count
    frameCount++;

    //  Get the number of milliseconds since glutInit called 
    //  (or first call to glutGet(GLUT ELAPSED TIME)).
    currentTime = glutGet(GLUT_ELAPSED_TIME);

    //  Calculate time passed
    int timeInterval = currentTime - previousTime;

    if(timeInterval > 1000)
    {
        //  calculate the number of frames per second
        fps = frameCount / (timeInterval / 1000.0f);

        //  Set time
        previousTime = currentTime;

        //  Reset frame count
        frameCount = 0;
    }
	printf("FPS: %f\n", fps);
}
//-----------------------------------------------------
//-----------------------------------------------------
//Load Object
void loadOBJ()
{
	int i = 0,k = 0,z = 1,w;//i是v的counter,k是f的counter
    int f1,f2,f3;//拿來做f的暫存
    int t;//trash 
    
    char data;
	char get_s[100];
	char v;
    char f;
    
    float v1;
    float v2;
    float v3;

    char all_data;
  
    FILE *cfPtr;
    
    if((cfPtr = fopen(DATA_NAME,"r")) == NULL) //讀入檔案 
                 
         printf("File could not be opened\n");
    
    else 
    {
         
        while(fgets(get_s,100,cfPtr)!= NULL )//一次讀一行將之寫入get_s array
        {
			if(get_s[0]=='v' && get_s[1]!='n')//讀入v 
			{
				sscanf(get_s,"%c %f %f %f ",&v,&v1,&v2,&v3);
				v_1[i] = v1+0.5; //將讀入的數字寫進 v_1  陣列 
                v_2[i] = v2+0.5; 
                v_3[i] = v3+0.5;
				vertex[3*i] = v1;
				vertex[3*i+1] = v2;
				vertex[3*i+2] = v3;
		

                i++;
			}
			if(get_s[0]=='f'&& get_s[1]!='s')//hoofs
            {          
				sscanf(get_s,"%c %d %d %d ",&f,&f1,&f2,&f3); 
               //f_1[k] = f1-1;
               //f_2[k] = f2-1;
               //f_3[k] = f3-1;
			   face[3*k] = f1-1;
			   face[3*k+1] = f2-1;
			   face[3*k+2] = f3-1;
			   k++;
            }
   	     }
	}
	fclose( cfPtr );
}
//-----------------------------------------------------
// VECTOR STUFF
// res = a cross b;
/*
void crossProduct( float *a, float *b, float *res) {
 
    res[0] = a[1] * b[2]  -  b[1] * a[2];
    res[1] = a[2] * b[0]  -  b[2] * a[0];
    res[2] = a[0] * b[1]  -  b[0] * a[1];
}
 
// Normalize a vec3
void normalize(float *a) {
 
    float mag = sqrt(a[0] * a[0]  +  a[1] * a[1]  +  a[2] * a[2]);
 
    a[0] /= mag;
    a[1] /= mag;
    a[2] /= mag;
}
 
// ----------------------------------------------------
// MATRIX STUFF
//
// sets the square matrix mat to the identity matrix,
// size refers to the number of rows (or columns)
void setIdentityMatrix( float *mat, int size) {
 
    // fill matrix with 0s
    for (int i = 0; i < size * size; ++i)
            mat[i] = 0.0f;
 
    // fill diagonal with 1s
    for (int i = 0; i < size; ++i)
        mat[i + i * size] = 1.0f;
}
//-----------------------------------------------------
// a = a * b;
void multMatrix(float *a, float *b) {
 
    float res[16];
 
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            res[j*4 + i] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                res[j*4 + i] += a[k*4 + i] * b[j*4 + k];
            }
        }
    }
    memcpy(a, res, 16 * sizeof(float));
 
}
//-----------------------------------------------------
// Defines a transformation matrix mat with a translation
void setTranslationMatrix(float *mat, float x, float y, float z) {
 
    setIdentityMatrix(mat,4);
    mat[12] = x;
    mat[13] = y;
    mat[14] = z;
}*/
// ----------------------------------------------------
// Projection Matrix
void buildProjectionMatrix(float fov, float ratio, float nearP, float farP) {
 
    float f = 1.0f / tan (fov * (M_PI / 360.0));
 
    setIdentityMatrix(projMatrix,4);
 
    projMatrix[0] = f / ratio;
    projMatrix[1 * 4 + 1] = f;
    projMatrix[2 * 4 + 2] = (farP + nearP) / (nearP - farP);
    projMatrix[3 * 4 + 2] = (2.0f * farP * nearP) / (nearP - farP);
    projMatrix[2 * 4 + 3] = -1.0f;
    projMatrix[3 * 4 + 3] = 0.0f;
}
 
// ----------------------------------------------------
// View Matrix
//
// note: it assumes the camera is not tilted,
// i.e. a vertical up vector (remmeber gluLookAt?)
//
void setCamera(float posX, float posY, float posZ,
               float lookAtX, float lookAtY, float lookAtZ) {
 
    float dir[3], right[3], up[3];
 
    up[0] = 0.0f;   up[1] = 1.0f;   up[2] = 0.0f;
 
    dir[0] =  (lookAtX - posX);
    dir[1] =  (lookAtY - posY);
    dir[2] =  (lookAtZ - posZ);
    normalize(dir);
 
    crossProduct(dir,up,right);
    normalize(right);
 
    crossProduct(right,dir,up);
    normalize(up);
 
    float aux[16];
 
    viewMatrix[0]  = right[0];
    viewMatrix[4]  = right[1];
    viewMatrix[8]  = right[2];
    viewMatrix[12] = 0.0f;
 
    viewMatrix[1]  = up[0];
    viewMatrix[5]  = up[1];
    viewMatrix[9]  = up[2];
    viewMatrix[13] = 0.0f;
 
    viewMatrix[2]  = -dir[0];
    viewMatrix[6]  = -dir[1];
    viewMatrix[10] = -dir[2];
    viewMatrix[14] =  0.0f;
 
    viewMatrix[3]  = 0.0f;
    viewMatrix[7]  = 0.0f;
    viewMatrix[11] = 0.0f;
    viewMatrix[15] = 1.0f;
 
    setTranslationMatrix(aux, -posX, -posY, -posZ);
 
    multMatrix(viewMatrix, aux);
}
// ----------------------------------------------------
void changeSize(int w, int h) {
	glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if (w <= h)
		buildProjectionMatrix(90.0f, (GLfloat) h / (GLfloat) w, 10.0f, 200.0f);
    // Set the viewport to be the entire window
	else 
		buildProjectionMatrix(90.0f, (GLfloat) w / (GLfloat) h, 10.0f, 200.0f);
}
//-----------------------------------------------------
void setupBuffers() {
 
    GLuint buffers[2];
 
    glGenVertexArrays(3, vao);
    //
    // VAO for first triangle
    //
    glBindVertexArray(vao[0]);
    // Generate two slots for the vertex and color buffers
    glGenBuffers(2, buffers);
    // bind buffer for vertices and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices1), vertices1, GL_STATIC_DRAW);
    glEnableVertexAttribArray(vertexLoc);
    glVertexAttribPointer(vertexLoc, 4, GL_FLOAT, 0, 0, 0);
 /*
    // bind buffer for colors and copy data into buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(colors1), colors1, GL_STATIC_DRAW);
    glEnableVertexAttribArray(colorLoc);
    //glVertexAttribPointer(colorLoc, 4, GL_FLOAT, 0, 0, 0);*/

}
 
void setUniforms() {
    // must be called after glUseProgram
    glUniformMatrix4fv(projMatrixLoc,  1, false, projMatrix);
    glUniformMatrix4fv(viewMatrixLoc,  1, false, viewMatrix);
}
 int count = 1;

 /*
GLuint  LoadTexture(GLuint texture)
{
  unsigned char * data;
  data = (unsigned char *)malloc( triangle_num* 3 );


	loadOBJ();
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_1D, texture);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F,  sizeof(data), 0, GL_RGB, GL_FLOAT, &data);

	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

free( data ); //free the texture

return texture; //return whether it was successfull
}
*/
void FreeTexture( GLuint texture )
{
glDeleteTextures( 1, &texture );
}




void renderScene(void) {
	

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setCamera(0,0,60,0,0,0);
    //glUseProgram(p);
	shader.bind();

	//GLint loc[12];

    
	setUniforms();
 
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_1D);
	int face_location = glGetUniformLocation(shader.id(),"faceArray");
	glUniform1i(face_location,0);
	glBindTexture(GL_TEXTURE_1D, indicesImage);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_1D);
	int vertice_location = glGetUniformLocation(shader.id(),"vertice");
	glUniform1i(vertice_location,1);
	glBindTexture(GL_TEXTURE_1D, verticesImage);


	
	glBindVertexArray(vao[0]);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
    /*glEndQuery(GL_TIME_ELAPSED);
	// get the query result
	glGetQueryObjectui64v(query, GL_QUERY_RESULT, &elapsed_time);
	printf("Time Elapsed: %f ms\n", elapsed_time / 1000000.0);*/

	
	
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_1D, 0);
	glDisable(GL_TEXTURE_1D);

	shader.unbind();
	
	
	
	
	
	
	/*
	if( finish_without_update )
		glFinish();
	else*/
		glutSwapBuffers();

}
 
void processNormalKeys(unsigned char key, int x, int y) {
 
    if (key == 27) {
        glDeleteVertexArrays(3,vao);
		glDeleteProgram(shader.id());
        //glDeleteShader(v);
        //glDeleteShader(f);
        exit(0);
    }
	else if(key == 'F' || key == 'f')
	{
		finish_without_update = true;
		//printf( "%f fps\n", g_fps( display, 100 ) );
		calculateFPS();
		//g_fps(display, 100);
		finish_without_update = false;
	}
}

void setShaders() {
	#if ( (defined(__MACH__)) && (defined(__APPLE__)) )
	#else
	glewInit();
	#endif
	if (glewIsSupported("GL_VERSION_3_3"))
		printf("Ready for OpenGL 3.3\n");
	else {
		printf("OpenGL 3.3 not supported\n");
		exit(1);
	}

	shader.init("trace.vert", "trace.frag");

	glBindFragDataLocation(shader.id(), 0, "outputF");
    glLinkProgram(shader.id());
    //printProgramInfoLog(shader.id());
 
    vertexLoc = glGetAttribLocation(shader.id(),"position");
    projMatrixLoc = glGetUniformLocation(shader.id(), "projMatrix");
    viewMatrixLoc = glGetUniformLocation(shader.id(), "viewMatrix");

}




 
int main(int argc, char **argv) {	
	//clock_t start_time, end_time;
	//float total_time = 0;
	//start_time = clock(); /* mircosecond */
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	loadOBJ();
    glutInitWindowPosition(100,100);
    glutInitWindowSize(512,512);
    glutCreateWindow("SBRT v2.0");

    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processNormalKeys);
 
    
	glewInit();
    
 
    glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
    glClearColor(1.0,1.0,1.0,1.0);
 
    
    setupBuffers();


	setShaders();
	
//Load Texture
	glGenTextures(1, &indicesImage);
	glBindTexture(GL_TEXTURE_1D, indicesImage);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F,  sizeof(face), 0, GL_RGB, GL_FLOAT, &face);

	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	glGenTextures(1, &verticesImage);
	glBindTexture(GL_TEXTURE_1D, verticesImage);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB16F, sizeof(vertex), 0, GL_RGB, GL_FLOAT, &vertex);

	
	glTexParameterf(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);  
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
	
  	//end_time = clock();
	//total_time = (float)(end_time - start_time)/CLOCKS_PER_SEC;
	//printf("執行時間 : %f sec \n", total_time);

	
	glutMainLoop();

    return(0);
}