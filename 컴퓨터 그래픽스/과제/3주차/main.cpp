#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>

const int NumVertices = 36; // 6 faces * 2 tri/face * 3 vtx/tri

vec4 points[NumVertices];
vec4 colors[NumVertices];

int Index = 0, i = 0;

GLfloat MoveV[3] = { 0.0,0.0,0.0 };
GLuint transform_loc; // uniform shader variable location

// rotate val
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
GLfloat Theta[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint theta_loc; // uniform shader variable location

// 큐브 포지션
vec3 cubePositions[10] = {
    vec3(0.0f, 0.0f, 0.0f),
    vec3(2.0f, 5.0f, -15.0f),
    vec3(-1.5f, -2.2f, -2.5f),
    vec3(-3.8f, -2.0f, -12.3f),
    vec3(2.4f, -0.4f, -3.5f),
    vec3(-1.7f, 3.0f, -7.5f),
    vec3(1.3f, -2.0f, -2.5f),
    vec3(1.5f, 2.0f, -2.5f),
    vec3(1.5f, 0.2f, -1.5f),
    vec3(-1.3f, 1.0f, -1.5f)
};

// 8 corner points of a unit cube centered at the origin
vec4 vertices[8] = {
    vec4( -0.5, -0.5,  0.5, 1.0 ),
    vec4( -0.5,  0.5,  0.5, 1.0 ),
    vec4(  0.5,  0.5,  0.5, 1.0 ),
    vec4(  0.5, -0.5,  0.5, 1.0 ),
    vec4( -0.5, -0.5, -0.5, 1.0 ),
    vec4( -0.5,  0.5, -0.5, 1.0 ),
    vec4(  0.5,  0.5, -0.5, 1.0 ),
    vec4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA colors
vec4 vertex_colors[8] = {
    vec4( 0.0, 0.0, 0.0, 1.0 ),  // black
    vec4( 1.0, 0.0, 0.0, 1.0 ),  // red
    vec4( 1.0, 1.0, 0.0, 1.0 ),  // yellow
    vec4( 0.0, 1.0, 0.0, 1.0 ),  // green
    vec4( 0.0, 0.0, 1.0, 1.0 ),  // blue
    vec4( 1.0, 0.0, 1.0, 1.0 ),  // magenta
    vec4( 1.0, 1.0, 1.0, 1.0 ),  // white
    vec4( 0.0, 1.0, 1.0, 1.0 )   // cyan
};

//viewing transformation parameters

GLfloat radius = 4.0;
GLfloat theta = 0.0;
GLfloat phi = 0.0;

const GLfloat dr = 5.0 * DegreesToRadians;

GLuint model_view; // model matrix uniform shader variable location

// Projection transformation parameters
GLfloat fovy = 45.0; //field-of-view in y direction angle (in degrees)
GLfloat aspect; //Viewport aspect ratio
GLfloat zNear = 3.663124, zFar = 21.978741;

GLuint projection; //projection matrix uniform shader variable location

//make two triangls that divide a quad
void quad(int a, int b, int c, int d)
{
    //one triangle a-b-c
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[b]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;

    //one triangle a-c-d
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[c]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[d]; points[Index] = vertices[d]; Index++;
}

// generate a cube
void colorcube()
{
    quad( 1, 0, 3, 2);
    quad( 2, 3, 7, 6);
    quad( 3, 0, 4, 7);
    quad( 6, 5, 1, 2);
    quad( 4, 5, 6, 7);
    quad( 5, 4, 0, 1);
}
void init()
{
    colorcube();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //buffer object
    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, 
                 GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    //load shaders
    GLuint program = InitShader("vshader_perspective2.glsl", "fshader_perspective2.glsl");
    glUseProgram(program);

    //initialize vertex position attribute from vertex shader
    GLuint vPos = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPos);
    glVertexAttribPointer(vPos, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //initialize vertex color attribute from vertex shader
    GLuint vCol = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vCol);
    glVertexAttribPointer(vCol, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));

    //initialize uniform variable from vertex shander
    model_view = glGetUniformLocation(program, "model_view");
    projection = glGetUniformLocation(program, "projection");
    theta_loc = glGetUniformLocation(program, "theta");
    transform_loc = glGetUniformLocation(program, "transform");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void display()
{
    glClear( GL_COLOR_BUFFER_BIT  | GL_DEPTH_BUFFER_BIT );

    vec4 eye( radius * sin(theta) * cos(phi),
              radius * sin(theta) * sin(phi),
              radius * cos(theta),
              1.0);

    vec4 at(0.0, 0.0, 0.0, 1.0);
    vec4 up(0.0, 1.0, 0.0, 0.0);

    mat4 mv = LookAt(eye, at, up);
    
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);

    mat4 p = Perspective(fovy, aspect, zNear, zFar);
    glUniformMatrix4fv(projection, 1, GL_TRUE, p);

    // 10개의 큐브 그리기
    for (unsigned int i = 0; i < 10; i++)
    {
        // 각 큐브 위치 설정
        MoveV[0] = cubePositions[i].x;
        MoveV[1] = cubePositions[i].y;
        MoveV[2] = cubePositions[i].z;

        // 각 큐브 회전 설정
        Theta[Xaxis] = 20.0f * i;
        Theta[Yaxis] = 30.0f;

        // 쉐이더로 위치랑, 회전 적용
        glUniform3fv(theta_loc, 1, Theta);
        glUniform3fv(transform_loc, 1, MoveV);

        // 큐브 전부 그리기
        glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    }    
    glutSwapBuffers();
}

void keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
    case 'z': zNear *= 1.1; zFar *= 1.1; printf("%f, %f\n", zNear, zFar); break;
    case 'Z': zNear *= 0.9; zFar *= 0.9; printf("%f, %f\n", zNear, zFar); break;
    case 'r': radius *= 2.0; printf("%f\n", radius); break;
    case 'R': radius *= 0.5; printf("%f\n", radius); break;
    case 'o': theta += dr; break;
    case 'O': theta -= dr; break;
    case 'p': phi += dr; break;
    case 'P': phi -= dr; break;

    case ' ':  // reset values to their defaults
        zNear = 0.5;
        zFar = 3.0;

        radius = 1.0;
        theta = 0.0;
        phi = 0.0;
        break;
    }
    glutPostRedisplay();
}


void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = GLfloat(width) / height;
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);

    aspect = 512.0 / 512.0;

    glutInitContextVersion(4, 3);

    glutCreateWindow("10 cubes");

    glewInit();
    init();
   

    glutDisplayFunc(display);
    glutKeyboardFunc( keyboard );
    
    glutMainLoop();
    return 0;
}