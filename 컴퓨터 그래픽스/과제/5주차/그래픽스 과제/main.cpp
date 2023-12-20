#define _CRT_SECURE_NO_WARNINGS
#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include "ObjLoader.h"

//viewing transformation parameters

const GLfloat dr = 5.0 * DegreesToRadians;

GLfloat radius = 2.0;
GLfloat theta = 0.0;
GLfloat phi = 0.0;

GLuint view; // model-view matrix uniform shader variable location
GLuint model;

// Projection transformation parameters
GLfloat fovy = 45.0; //field-of-view in y direction angle (in degrees)
GLfloat aspect; //Viewport aspect ratio
GLfloat zNear = 0.1, zFar = 10.0;
GLfloat winLeft = -2, winRight = 2, winBottom = -2, winTop = 2;
GLuint projection; //projection matrix uniform shader variable location
GLuint color_loc;



class TriMesh
{
public:
    int NumVertices;
    int NumTris;

    GLuint vao;
    GLuint vbo;
    GLuint ebo; // element buffer object for indices
    
    std::vector<vec4> vertices;

    vec3 color;
    mat4 modelTransform;

    std::vector<unsigned int> indices;

    TriMesh() {
        NumVertices = 0;
        NumTris = 0;
    }

    ~TriMesh() {
        glDeleteBuffers(1, &vbo);
        glDeleteBuffers(1, &ebo);
        glDeleteVertexArrays(1, &vao);
    }

    void init() {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
    }

    void Render() {
        //set a constant color
        glUniform3fv(color_loc, 1, color);
        glUniformMatrix4fv(model, 1, GL_TRUE, modelTransform);

        glBindVertexArray(vao);
        glEnableVertexAttribArray(0);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vec4) * vertices.size(), vertices.data(),
            GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(),
            GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
    }
};

vec3 profileCurve(GLfloat t) {
    vec3 a = vec3(0.2, 0.0, -1.0);
    vec3 b = vec3(3.6, 0.0, 0.2);
    vec3 c = vec3(0.5, 0.0, 2.0);
    vec3 sol;
    for (int i = 0; i < 3; i++)
        sol[i] = a[i] * (1 - t) * (1 - t) + b[i] * 2 * (1 - t) * t + c[i] * t * t;
    return sol;
}

int MakeCylinder(TriMesh *mesh, GLfloat radius, GLfloat height, int nR, int nV)
{
    if (nR < 2 || nV < 1)
        return -1;

    GLfloat deltaR = 2.0 * M_PI / (GLfloat)nR;
    GLfloat deltaV = height / (GLfloat)nV;

    mesh->NumVertices = (nR + 1) * (nV + 1);
    mesh->NumTris = nR * nV * 2;

    mesh->vertices.resize(mesh -> NumVertices);
    mesh->indices.resize(mesh->NumTris * 3);

    for (int j = 0; j <= nV; j++) {
        for (int i = 0; i <= nR; i++) {
            vec3 pt;
            vec3 t = profileCurve(deltaV * (GLfloat)j);
            pt.x = t.x * cos(deltaR * (GLfloat)i);
            pt.y = t.x * sin(deltaR * (GLfloat)i);
            pt.z = t.z;
            mesh->vertices[(nR + 1) * j + i] = vec4(pt, 1.0);
        }
    }

    for (int j = 0; j < nV; j++) {
        for (int i = 0; i < nR; i++) {
            int quadId = nR * j + i;
            mesh->indices[6 * quadId] = (nR + 1) * j + i;
            mesh->indices[6 * quadId + 1] = (nR + 1) * (j+1) + i;
            mesh->indices[6 * quadId + 2] = (nR + 1) * j + i+1;
            mesh->indices[6 * quadId + 3] = (nR + 1) * (j+1) + i;
            mesh->indices[6 * quadId + 4] = (nR + 1) * (j+1) + i+1;
            mesh->indices[6 * quadId + 5] = (nR + 1) * j + i+1;
        }
    }
    return 0;
}

//TriMesh Cylinder;
TriMesh bunny;
TriMesh Cylinder;

void initObj(TriMesh *b, vec3 c) {
    CObjLoader* loader = new CObjLoader();
    if (!loader->Load("spot.obj", NULL)) {
        std::cout << "Cannot read the input obj file\n";
        return;
    }

    b->NumVertices = loader->vertexes.size();
    b->NumTris = loader->parts[0].faces.size();

    b->vertices.resize(b->NumVertices);
    for (int i = 0; i < b->NumVertices; i++) {
        tVertex vtx = loader->getVertex(i);
        b->vertices[i] = vec4(vtx.x, vtx.y, vtx.z, 1.0);
    }

    b->indices.resize(b->NumTris * 3);
    for (int i = 0; i < b->NumTris; i++) {
        tFace* face = &(loader->parts[0].faces[i]);

        for (int j = 0; j < 3; j++)
            b->indices[3 * i + j] = (unsigned int)(face->v[j] - 1);
    }
    delete loader;

    b->modelTransform = Translate(c) * RotateY(180) * Scale(0.5);
    b->init();
}

void initCyl(TriMesh *a, vec3 b) {
    // 실린더
    MakeCylinder(a, 1.0, 1.0, 12, 10);
    a->color = vec3(0, 1.0, 0);
    a->modelTransform = Translate(b) * RotateX(45.0) * Scale(0.2, 0.2, 0.4);
    a->init();
}

void init( void )
{  
   
    initCyl(&Cylinder, vec3(0.5,0.0,0.0));
    //initObj();
    initObj(&bunny, vec3(-0.5,0.0,0.0));

    //Set up shaders
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);


    
    //initialize vertex position attribute from vertex shader
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    //initialize uniform variable from vertex shander
    model = glGetUniformLocation(program, "model");
    view = glGetUniformLocation(program, "view");
    projection = glGetUniformLocation(program, "projection");
    color_loc = glGetUniformLocation(program, "segColor");

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

    mat4 vmat = LookAt(eye, at, up);
    glUniformMatrix4fv(view, 1, GL_TRUE, vmat);

    mat4 p = Perspective(fovy, aspect, zNear, zFar);

    glUniformMatrix4fv(projection, 1, GL_TRUE, p);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0, 1.0);
    bunny.color = vec3(102.0/255.0, 178.0/255.0, 1.0);
    bunny.Render();

    Cylinder.color = vec3(0.0, 1.0, 0.0);
    Cylinder.Render();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    bunny.color = vec3(0.0, 0.0, 0.0);
    bunny.Render();

    Cylinder.color = vec3(1.0, 0.0, 0.0);
    Cylinder.Render();
    glutSwapBuffers();
}

void
keyboard( unsigned char key, int x, int y )
{
    switch ( key ) {
    case 033:
        exit( EXIT_SUCCESS );
        break;
    case 'z': zNear *= 1.1; zFar *= 1.1; break;
    case 'Z': zNear *= 0.9; zFar *= 0.9; break;
    case 'r': radius *= 2.0; break;
    case 'R': radius *= 0.5; break;
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

    //glutInitContextVersion(4, 3);
    //glutInitContextProfie(GLUT_CORE_PROFILE);
    
    glutCreateWindow("2018037356안동현");

    glewInit();
    init();
   
    /*
    printf("OpenGL %s, GLSL %s\n",                         
	    glGetString(GL_VERSION),
            glGetString(GL_SHADING_LANGUAGE_VERSION));
    */

    glutDisplayFunc(display);
    glutKeyboardFunc( keyboard );
    
    glutMainLoop();

    return 0;
}