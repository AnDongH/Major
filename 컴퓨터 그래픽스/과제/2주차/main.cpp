#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <math.h>

#define X .525731112119133606 
#define Z .850650808352039932

const int NumVertices = 60;
int Index = 0;

vec4 points[NumVertices];
vec4 colors[NumVertices];

// color val
int edgeColor[12][12];
bool flag1 = true, flag2 = true, flag3 = true;

// move val
int x_move = 0, y_move = 0;
GLfloat MoveV[3] = {0.0,0.0,0.0};
GLuint transform_loc; // uniform shader variable location

// scale val
int object_scale = 0;
GLfloat ScaleV[3] = { 1.0,1.0,1.0 };
GLuint scale_loc; // uniform shader variable location

// 12 corner points of a unit icosahedron centered at the origin
vec4 vertices[12] = {
    vec4(-X,0.0,Z,1.0),
    vec4(X,0.0,Z,1.0),
    vec4(-X,0.0,-Z,1.0),
    vec4(X,0.0,-Z,1.0),
    vec4(0.0,Z,X,1.0),
    vec4(0.0,Z,-X,1.0),
    vec4(0.0,-Z,X,1.0),
    vec4(0.0,-Z,-X,1.0),
    vec4(Z,X,0.0,1.0),
    vec4(-Z,X,0.0,1.0),
    vec4(Z,-X,0.0,1.0),
    vec4(-Z,-X,0.0,1.0)
};

// RGBA colors
vec4 vertex_colors[3] = {
    vec4(1.0, 1.0, 0.0, 1.0),  // yellow
    vec4(1.0, 0.0, 1.0, 1.0),  // magenta
    vec4(0.0, 1.0, 1.0, 1.0)   // cyan
};

// rotate val
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int Axis = Xaxis;
bool rot_flag = false;
GLfloat Theta[NumAxes] = { 0.0, 0.0, 0.0 };
GLuint theta_loc; // uniform shader variable location


//make triangle and color
void triangle(int a, int b, int c)
{

    // 겹치지 않는 컬러 선택
    int color = 0;
    if (edgeColor[a][b] == 0 || edgeColor[b][c] == 0 || edgeColor[a][c] == 0) flag1 = false;
    if (edgeColor[a][b] == 1 || edgeColor[b][c] == 1 || edgeColor[a][c] == 1) flag2 = false;
    if (edgeColor[a][b] == 2 || edgeColor[b][c] == 2 || edgeColor[a][c] == 2) flag3 = false;

    if (flag1) color = 0;
    if (flag2) color = 1;
    if (flag3) color = 2;

    edgeColor[a][b] = color;
    edgeColor[b][a] = color;
    edgeColor[a][c] = color;
    edgeColor[c][a] = color;
    edgeColor[b][c] = color;
    edgeColor[c][b] = color;

    //one triangle a-b-c
    colors[Index] = vertex_colors[color]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[color]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[color]; points[Index] = vertices[c]; Index++;

    flag1 = true, flag2 = true, flag3 = true;
}

// generate a Icosahedron
void colorIcosahedron()
{
    triangle(0, 4, 1);
    triangle(0, 9, 4);
    triangle(9, 0, 11);
    triangle(11, 0, 6);
    triangle(0, 1, 6);
    triangle(6, 1, 10);
    triangle(8, 10, 1);
    triangle(4, 8, 1);
    triangle(4, 5, 8);
    triangle(9, 5, 4);
    triangle(9, 2, 5);
    triangle(9, 11, 2);
    triangle(7, 2, 11);
    triangle(7, 11, 6);
    triangle(7, 6, 10);
    triangle(7, 10, 3);
    triangle(8, 3, 10);
    triangle(5, 3, 8);
    triangle(5, 2, 3);
    triangle(2, 7, 3);
}
void init(void)
{
    // edge init
    for (int i = 0; i < 12; i++)
        for (int j = 0; j < 12; j++)
            edgeColor[i][j] = -1;

    colorIcosahedron();

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
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
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
    theta_loc = glGetUniformLocation(program, "theta");
    transform_loc = glGetUniformLocation(program, "transform");
    scale_loc = glGetUniformLocation(program, "scale");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0, 0.0, 0.0, 0.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUniform3fv(theta_loc, 1, Theta);
    glUniform3fv(transform_loc, 1, MoveV);
    glUniform3fv(scale_loc, 1, ScaleV);

    glDrawArrays(GL_TRIANGLES, 0, NumVertices);
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 32:
            rot_flag = !rot_flag;
            break;
        case 97:      // a
            x_move = -1;
            break;
        case 100:     // d
            x_move = 1;
            break;
        case 113:     // q
            exit(EXIT_SUCCESS);
            break;
        case 115:     // s
            y_move = -1;
            break;
        case 119:     // w
            y_move = 1;
            break;
        case 120:     // x
            object_scale = 1;
            break;
        case 122:     // z
            object_scale = -1;
            break;
    }
    
}

void keyUp(unsigned char key, int x, int y) {
    switch (key) {
        case 97:      // a
        case 100:     // d
            x_move = 0;
            break;
        case 115:     // s
        case 119:     // w
            y_move = 0;
            break;
        case 120:     // x
        case 122:     // z
            object_scale = 0;
            break;
    }
}

void idle()
{

    if (rot_flag) {
        Theta[Axis] += 0.8;

        if (Theta[Axis] > 360.0) {
            Theta[Axis] -= 360.0;
        }
    }

    if (x_move != 0) {
        MoveV[0] += (0.01 * x_move);
    }

    if (y_move != 0) {
        MoveV[1] += (0.01 * y_move);
    }

    if (object_scale != 0) {
        ScaleV[0] += (0.01 * object_scale);
        ScaleV[1] += (0.01 * object_scale);
        ScaleV[2] += (0.01 * object_scale);
    }


    glutPostRedisplay();
}

void mouse(int button, int state, int x, int y)
{
    if (state == GLUT_DOWN) {
        switch (button) {
            case GLUT_LEFT_BUTTON: Axis = Xaxis; break;
            case GLUT_MIDDLE_BUTTON: Axis = Yaxis; break;
            case GLUT_RIGHT_BUTTON: Axis = Zaxis; break;
        }
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);

    glutInitContextVersion(4, 3);
    glutCreateWindow("HW1");

    glewInit();
    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutKeyboardUpFunc(keyUp);
    glutMouseFunc(mouse);
    glutIdleFunc(idle);

    glutMainLoop();
    return 0;
}