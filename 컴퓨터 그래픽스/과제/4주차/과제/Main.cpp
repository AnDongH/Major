#include <vgl.h>
#include <InitShader.h>
#include <mat.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <tuple>
#include <regex>

const int NumCrvVertices2D = 1024;
const int NumCrvVertices1D = 2;

int Index = 0;
std::string t_input;
char input;

GLfloat BezierBasis2D[NumCrvVertices2D][3];
GLfloat BezierBasis1D[NumCrvVertices1D][2];



std::string s;
std::vector<std::tuple<vec2, vec2, vec2>> pointsOf2D[123];
std::vector<std::tuple<vec2, vec2>> pointsOf1D[123];
vec4 orthoVal[123];
std::ifstream in;
int parsingIdx = 0;

bool isSingleValidCharacter(const std::string& input) {
    // 정확히 한 글자이며, 0~9, A~Z, a~z 문자만을 포함하는 정규 표현식
    std::regex pattern("^[0-9a-zA-Z]$");

    return std::regex_match(input, pattern);
}

struct BezierCurve2D {
    // p값
    vec2 BndPos[3];

    // 곡선 정보
    vec2 points[NumCrvVertices2D];
    vec3 colors[NumCrvVertices2D];
    GLuint vao;
    GLuint vbo;

    // 점 정보
    GLuint vaoBndPts;
    GLuint vboBndPts;
    vec2 bndPoints[3];
    vec3 bndColors[3];

    // 점의 직선 정보
    GLuint vaoBndLine;
    GLuint vboBndLine;
    vec2 linePoints[3];
    vec3 lineColors[3];

    BezierCurve2D() {
    }

    BezierCurve2D(const vec2& p0, const vec2& p1, const vec2& p2)
    {
        BndPos[0] = p0;
        BndPos[1] = p1;
        BndPos[2] = p2;
    }

    ~BezierCurve2D() {

    }

    void evalulate() {
        for (int i = 0; i < NumCrvVertices2D; i++) {
            for (int j = 0; j < 3; j++)
                points[i][j] = BezierBasis2D[i][0] * BndPos[0][j] + BezierBasis2D[i][1] * BndPos[1][j]
                + BezierBasis2D[i][2] * BndPos[2][j];
        }
    }

    void updateForRendering() {
        evalulate();
        bndPoints[0] = BndPos[0];
        bndPoints[1] = BndPos[1];
        bndPoints[2] = BndPos[2];

        linePoints[0] = BndPos[0];
        linePoints[1] = BndPos[1];
        linePoints[2] = BndPos[2];
    }
};

struct BezierCurve1D {
    // p값
    vec2 BndPos[2];

    // 곡선 정보
    vec2 points[NumCrvVertices1D];
    vec3 colors[NumCrvVertices1D];
    GLuint vao;
    GLuint vbo;

    // 점 정보
    GLuint vaoBndPts;
    GLuint vboBndPts;
    vec2 bndPoints[2];
    vec3 bndColors[2];

    // 점의 직선 정보
    GLuint vaoBndLine;
    GLuint vboBndLine;
    vec2 linePoints[2];
    vec3 lineColors[2];

    BezierCurve1D() {
    }

    BezierCurve1D(const vec2& p0, const vec2& p1)
    {
        BndPos[0] = p0;
        BndPos[1] = p1;
    }

    ~BezierCurve1D() {

    }

    void evalulate() {
        for (int i = 0; i < NumCrvVertices1D; i++) {
            for (int j = 0; j < 2; j++)
                points[i][j] = BezierBasis1D[i][0] * BndPos[0][j] + BezierBasis1D[i][1] * BndPos[1][j];
        }
    }

    void updateForRendering() {
        evalulate();
        bndPoints[0] = BndPos[0];
        bndPoints[1] = BndPos[1];

        linePoints[0] = BndPos[0];
        linePoints[1] = BndPos[1];
    }
};

BezierCurve2D curve;
BezierCurve1D curve1D;

// 2차 베지에 계산
void precomputeBezier2dBasis()
{
    GLfloat t, t_1_m;
    for (int i = 0; i < NumCrvVertices2D; i++) {
        t = i * 1.0 / (GLfloat)(NumCrvVertices2D - 1.0);
        t_1_m = (1 - t);
        BezierBasis2D[i][0] = t_1_m * t_1_m;
        BezierBasis2D[i][1] = 2 * t_1_m * t;
        BezierBasis2D[i][2] = t * t;
    }
}

// 1차 베지에 계산
void precomputeBezier1dBasis()
{
    GLfloat t, t_1_m;
    for (int i = 0; i < NumCrvVertices1D; i++) {
        t = i * 1.0 / (GLfloat)(NumCrvVertices1D - 1.0);
        t_1_m = (1 - t);
        BezierBasis1D[i][0] = t_1_m;
        BezierBasis1D[i][1] = t;
    }
}

//viewing transformation parameters
GLuint model_view; // model-view matrix uniform shader variable location

// Projection transformation parameters
GLuint projection; //projection matrix uniform shader variable location

int Width;
int Height;

GLfloat left = -1.0, right = 1.0;
GLfloat bottom = -1.0, top = 1.0;

void init(void)
{
    in = std::ifstream("times_font_partial.txt");
    if (!in.is_open()) {
        printf("no file\n");
        return;
    }

    while (in) {
        getline(in, s);
        std::stringstream ss(s);
        std::string word;
        std::vector<std::string> words;
        while (ss >> word) {
            words.push_back(word);
        }

        if (words[0] == "CharIndex") {
            parsingIdx = std::stoi(words[1]);
        }
        else if (words[0] == "BBox") {
            orthoVal[parsingIdx] = vec4(std::stof(words[1]), std::stof(words[3]), std::stof(words[2]), std::stof(words[4]));
        }
        else if (words[0] == "C") {
            pointsOf2D[parsingIdx].push_back(std::make_tuple(
                vec2(std::stof(words[1]), std::stof(words[2])), 
                vec2(std::stof(words[3]), std::stof(words[4])), 
                vec2(std::stof(words[5]), std::stof(words[6]))));
        }
        else if (words[0] == "L") {
            pointsOf1D[parsingIdx].push_back(std::make_tuple(
                vec2(std::stof(words[1]), std::stof(words[2])),
                vec2(std::stof(words[3]), std::stof(words[4]))));

        }
    }

    in.close();

    precomputeBezier2dBasis();
    precomputeBezier1dBasis();

    curve = BezierCurve2D(vec2(0.0, 0.0), vec2(0.0, 0.0), vec2(0.0, 0.0));
    curve1D = BezierCurve1D(vec2(0.0, 0.0), vec2(0.0, 0.0));

    // 곡선 칼라
    for (int i = 0; i < NumCrvVertices2D; i++)
        curve.colors[i] = vec3(0.0, 0.0, 0.0);
    for (int i = 0; i < NumCrvVertices1D; i++)
        curve1D.colors[i] = vec3(0.0, 0.0, 0.0);


    for (int i = 0; i < 3; i++) {
        curve.lineColors[i] = vec3(0.0, 1.0, 0.0);
        curve.bndColors[i] = vec3(0.0, 1.0, 0.0);
    }
    for (int i = 0; i < 2; i++) {
        curve1D.lineColors[i] = vec3(1.0, 0.0, 1.0);
        curve1D.bndColors[i] = vec3(1.0, 0.0, 1.0);
    }


    // 2D곡선
    glGenVertexArrays(1, &(curve.vao));
    glGenBuffers(1, &(curve.vbo));
    // 1D곡선
    glGenVertexArrays(1, &(curve1D.vao));
    glGenBuffers(1, &(curve1D.vbo));

    // 2D직선
    glGenVertexArrays(1, &(curve.vaoBndLine));
    glGenBuffers(1, &(curve.vboBndLine));
    // 1D직선
    glGenVertexArrays(1, &(curve1D.vaoBndLine));
    glGenBuffers(1, &(curve1D.vboBndLine));

    // 2Dp점
    glGenVertexArrays(1, &(curve.vaoBndPts));
    glGenBuffers(1, &(curve.vboBndPts));
    // 1Dp점
    glGenVertexArrays(1, &(curve1D.vaoBndPts));
    glGenBuffers(1, &(curve1D.vboBndPts));

    // 
    //load shaders
    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    //initialize vertex position attribute from vertex shader
   /*
    점이랑 선이랑 번갈아가면서 그려줘야 하기에, display에 써야한다.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve.points)));*/

    //initialize uniform variable from vertex shander
    model_view = glGetUniformLocation(program, "model_view");
    projection = glGetUniformLocation(program, "projection");

    //OpenGL rendering initiailization
    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0, 1.0, 1.0, 0.0);

}

void draw2D(int i) {
    for (int t = 0; t < pointsOf2D[i].size(); t++) {
        curve.BndPos[0] = std::get<0>(pointsOf2D[i][t]);
        curve.BndPos[1] = std::get<1>(pointsOf2D[i][t]);
        curve.BndPos[2] = std::get<2>(pointsOf2D[i][t]);

        curve.updateForRendering();

        // 곡선
        glBindVertexArray(curve.vao);
        glBindBuffer(GL_ARRAY_BUFFER, curve.vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(curve.points) + sizeof(curve.colors), NULL,
            GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.points), curve.points);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve.points), sizeof(curve.colors), curve.colors);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve.points)));


        glDrawArrays(GL_LINE_STRIP, 0, NumCrvVertices2D);
        glBindVertexArray(0); // 바인드 풀어주기

        // 직선
        glBindVertexArray(curve.vaoBndLine);
        glBindBuffer(GL_ARRAY_BUFFER, curve.vboBndLine);


        glBufferData(GL_ARRAY_BUFFER, sizeof(curve.linePoints) + sizeof(curve.lineColors), NULL,
            GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.linePoints), curve.linePoints);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve.linePoints), sizeof(curve.lineColors), curve.lineColors);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve.linePoints)));

        glDrawArrays(GL_LINE_STRIP, 0, 3);
        glBindVertexArray(0); // 바인드 풀어주기

        // 점
        glBindVertexArray(curve.vaoBndPts);
        glBindBuffer(GL_ARRAY_BUFFER, curve.vboBndPts);

        glBufferData(GL_ARRAY_BUFFER, sizeof(curve.bndPoints) + sizeof(curve.bndColors), NULL,
            GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve.bndPoints), curve.bndPoints);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve.bndPoints), sizeof(curve.bndColors), curve.bndColors);


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve.bndPoints)));

        glPointSize(10.0);
        glDrawArrays(GL_POINTS, 0, 3);
        glBindVertexArray(0);
    }
}

void draw1D(int i) {
    for (int t = 0; t < pointsOf1D[i].size(); t++) {
        curve1D.BndPos[0] = std::get<0>(pointsOf1D[i][t]);
        curve1D.BndPos[1] = std::get<1>(pointsOf1D[i][t]);
        curve1D.updateForRendering();

        // 곡선
        glBindVertexArray(curve1D.vao);
        glBindBuffer(GL_ARRAY_BUFFER, curve1D.vbo);

        glBufferData(GL_ARRAY_BUFFER, sizeof(curve1D.points) + sizeof(curve1D.colors), NULL,
            GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve1D.points), curve1D.points);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve1D.points), sizeof(curve1D.colors), curve1D.colors);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve1D.points)));


        glDrawArrays(GL_LINE_STRIP, 0, NumCrvVertices1D);
        glBindVertexArray(0); // 바인드 풀어주기

        // 직선
        glBindVertexArray(curve1D.vaoBndLine);
        glBindBuffer(GL_ARRAY_BUFFER, curve1D.vboBndLine);


        glBufferData(GL_ARRAY_BUFFER, sizeof(curve1D.linePoints) + sizeof(curve1D.lineColors), NULL,
            GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve1D.linePoints), curve1D.linePoints);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve1D.linePoints), sizeof(curve1D.lineColors), curve1D.lineColors);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve1D.linePoints)));

        glDrawArrays(GL_LINE_STRIP, 0, 2);
        glBindVertexArray(0); // 바인드 풀어주기

        // 점
        glBindVertexArray(curve1D.vaoBndPts);
        glBindBuffer(GL_ARRAY_BUFFER, curve1D.vboBndPts);

        glBufferData(GL_ARRAY_BUFFER, sizeof(curve1D.bndPoints) + sizeof(curve1D.bndColors), NULL,
            GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(curve1D.bndPoints), curve1D.bndPoints);
        glBufferSubData(GL_ARRAY_BUFFER, sizeof(curve1D.bndPoints), sizeof(curve1D.bndColors), curve1D.bndColors);


        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(curve1D.bndPoints)));

        glPointSize(10.0);
        glDrawArrays(GL_POINTS, 0, 2);
        glBindVertexArray(0);
    }
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 p;

    if (input >= 48 && input <= 57) {
        // 왼쪽 뷰포트 설정
        glViewport(0, 0, Width / 2, Height);

        left = orthoVal[input][0];
        right = orthoVal[input][1];
        bottom = orthoVal[input][2];
        top = orthoVal[input][3];
        p = Ortho2D(left, right, bottom, top);
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);

        draw2D(input);
        draw1D(input);
    }
    else if (input >= 65 && input <= 90) {
        // 왼쪽
        glViewport(0, 0, Width / 2, Height);
        left = orthoVal[input][0];
        right = orthoVal[input][1];
        bottom = orthoVal[input][2];
        top = orthoVal[input][3];
        p = Ortho2D(left, right, bottom, top);
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);

        draw2D(input);
        draw1D(input);
        // 오른쪽
        glViewport(Width / 2, 0, Width / 2, Height);
        left = orthoVal[input + 32][0];
        right = orthoVal[input + 32][1];
        bottom = orthoVal[input + 32][2];
        top = orthoVal[input + 32][3];
        p = Ortho2D(left, right, bottom, top);
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);

        draw2D(input + 32);
        draw1D(input + 32);
    }
    else if (input >= 97 && input <= 122) {
        // 왼쪽
        glViewport(0, 0, Width / 2, Height);
        left = orthoVal[input - 32][0];
        right = orthoVal[input - 32][1];
        bottom = orthoVal[input - 32][2];
        top = orthoVal[input - 32][3];
        p = Ortho2D(left, right, bottom, top);
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);

        draw2D(input - 32);
        draw1D(input - 32);
        // 오른쪽
        glViewport(Width / 2, 0, Width / 2, Height);
        left = orthoVal[input][0];
        right = orthoVal[input][1];
        bottom = orthoVal[input][2];
        top = orthoVal[input][3];
        p = Ortho2D(left, right, bottom, top);
        glUniformMatrix4fv(projection, 1, GL_TRUE, p);

        draw2D(input);
        draw1D(input);
    }


    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
    case 033:
        exit(EXIT_SUCCESS);
        break;

    case ' ':  // reset values to their defaults
        left = -1.0;
        right = 1.0;
        bottom = -1.0;
        top = 1.0;

        break;
    }
    glutPostRedisplay();
}

void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    Width = width;
    Height = height;
}

void mouse(GLint button, GLint action, GLint x, GLint y)
{
    if (GLUT_LEFT_BUTTON == button)
    {
        switch (action)
        {
        case GLUT_DOWN:
            while (true) {
                std::cout << "입력하세요. 0~9, a~z, A~Z" << "\n";
                std::cin >> t_input;
                if (isSingleValidCharacter(t_input)) {
                    input = t_input[0];
                    break;
                }
                else {
                    std::cout << "0~9, a~z, A~Z 중에서 입력해주세요" << "\n";
                }
            }
            break;
        case GLUT_UP:

            break;
        default: break;
        }
        glutPostRedisplay();
    }
}

int main(int argc, char** argv)
{
    Width = 1024;
    Height = 600;
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(Width, Height);

    glutCreateWindow("TrueType Font");

    glewInit();
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);

    glutMainLoop();
    return 0;
}