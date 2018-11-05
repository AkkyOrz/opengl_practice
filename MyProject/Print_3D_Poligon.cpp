#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <random>


#define WINDOW_X (1000)
#define WINDOW_Y (1000)
#define WINDOW_NAME "test2"

// GL関係のプロトタイプ宣言
void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle();

class Point {
public:
    double x;
    double y;
    double z;
    Point()
    {
        x = y = z = 0.0;
    }
    Point(double x0, double y0, double z0)
    {
        x = x0;
        y = y0;
        z = z0;
    }
};

// 定数
GLdouble lightblue[] = {0.5, 1.0, 1.0}; // color
GLdouble red[] = {1.0, 0.5, 0.5}; // color
const int GRID_WIDTH = 20; //gridの縦と横
const int GRID_HEIGHT = 20;

// 物体描画関連のプロトタイプ宣言
void draw_pyramid();
void draw_cube(Point p, GLdouble cube_color[]);
void draw_grid();
void draw_lifegame();


// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 50.0;
bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
bool is_live[GRID_WIDTH][GRID_HEIGHT];



int main(int argc, char *argv[]){
  /* OpenGLの初期化 */
  init_GL(argc,argv);

  /* このプログラム特有の初期化 */
  init();

  /* コールバック関数の登録 */
  set_callback_functions();

  /* メインループ */
  glutMainLoop();

  return 0;
}

void init_GL(int argc, char *argv[]){
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(WINDOW_X,WINDOW_Y);
  glutCreateWindow(WINDOW_NAME);
}

void init(){
  glClearColor(0.0, 0.0, 0.0, 0.0);         // 背景の塗りつぶし色を指定
}

void set_callback_functions(){
  glutDisplayFunc(glut_display);
  glutKeyboardFunc(glut_keyboard);
  glutMouseFunc(glut_mouse);
  glutMotionFunc(glut_motion);
  glutPassiveMotionFunc(glut_motion);
  glutIdleFunc(glut_idle);
}

void glut_keyboard(unsigned char key, int x, int y){
  switch(key){
  case 'q':
  case 'Q':
  case '\033':
    exit(0);
  }

  glutPostRedisplay();
}

void glut_mouse(int button, int state, int x, int y){
  if(button == GLUT_LEFT_BUTTON){
    if(state == GLUT_UP){
      g_isLeftButtonOn = false;
    }else if(state == GLUT_DOWN){
      g_isLeftButtonOn = true;
    }
  }

  if(button == GLUT_RIGHT_BUTTON){
    if(state == GLUT_UP){
      g_isRightButtonOn = false;
    }else if(state == GLUT_DOWN){
      g_isRightButtonOn = true;
    }
  }
}

void glut_motion(int x, int y){
  static int px = -1, py = -1;
  if(g_isLeftButtonOn == true){
    if(px >= 0 && py >= 0){
      g_angle1 += (double) -(x - px) / 20;
      g_angle2 += (double) -(y - py) / 20;
    }
    px = x;
    py = y;
  }else if(g_isRightButtonOn == true){
    if(px >= 0 && py >= 0){
      g_distance += (double) (y - py) / 20;
    }
    px = x;
    py = y;
  }else{
    px = -1;
    py = -1;
  }
  glutPostRedisplay();
}

void glut_display(){
  // まず投影変換
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, 1.0, 0.1, 100);

  // つぎにモデル・ビュー変換
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  if (cos(g_angle2) > 0){
    gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
              g_distance * sin(g_angle2),
              g_distance * cos(g_angle2) * cos(g_angle1),
              GRID_WIDTH/2.0, GRID_HEIGHT/2.0, 0.0,
              0.0, 1.0, 0.0);
  } else {
    gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
              g_distance * sin(g_angle2),
              g_distance * cos(g_angle2) * cos(g_angle1),
              GRID_WIDTH/2.0, GRID_HEIGHT/2.0, 0.0
              , 0.0, -1.0, 0.0);
  }
  
  gluLookAt(3, 3, 3, 0, 0, 0, 0, 1, 0);

  glRotatef(g_angle1*3, 1.0, 0.0, 0.0);
  glRotatef(g_angle2*3, 0.0, 1.0, 0.0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  Point p = {0.0, 0.0, 0.0};
  Point q = {1.0, 0.0, 0.0};
  draw_grid();
  
  draw_lifegame();
  //draw_cube(p, lightblue);
  //draw_cube(q, red);
  glDisable(GL_DEPTH_TEST);

  glutSwapBuffers();
}


// 背景にグリッドを生成
void draw_grid(){
    GLdouble pointO[] = {0.0, 0.0, -1.0};
    GLdouble pointA[] = {GRID_WIDTH, 0.0, -1.0};
    for (int i = 0; i <= GRID_HEIGHT; i++){
        // Lineの太さは0.5が最小...?
        glLineWidth(0.5);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex3dv(pointO);
        glVertex3dv(pointA);
        glEnd();
        pointO[1] += 1.0;
        pointA[1] += 1.0;
    }
    GLdouble pointB[] = {0.0, 0.0, -1.0};
    GLdouble pointC[] = {0.0, GRID_HEIGHT, -1.0};
    for (int i = 0; i <= GRID_WIDTH; i++){
        glLineWidth(0.5);
        glColor3f(1.0, 1.0, 1.0);
        glBegin(GL_LINES);
        glVertex3dv(pointB);
        glVertex3dv(pointC);
        glEnd();
        pointB[0] += 1.0;
        pointC[0] += 1.0;
    }
}

void glut_idle(){
  static int counter = 0;

  if(counter == 0){
    draw_lifegame();
  }

  counter++;
  if(counter > 100) counter = 0;

  glutPostRedisplay();
}

// Point{x, y, z}を起点にPoint{x+1, y+1, z+1}の立方体を描画する。
void draw_cube(Point p, GLdouble cube_color[]){

  // 1.0だと同一平面上に描画することになり、いろいろあれなので微小区間だけ縮めた
  GLdouble pointO[] = {p.x, p.y, p.z};
  GLdouble pointA[] = {p.x, p.y+0.999, p.z};
  GLdouble pointB[] = {p.x, p.y+0.999, p.z+0.999};
  GLdouble pointC[] = {p.x, p.y, p.z+0.999};
  GLdouble pointD[] = {p.x+0.999, p.y, p.z};
  GLdouble pointE[] = {p.x+0.999, p.y+0.999, p.z};
  GLdouble pointF[] = {p.x+0.999, p.y+0.999, p.z+0.999};
  GLdouble pointG[] = {p.x+0.999, p.y, p.z+0.999};
  

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointE);
  glVertex3dv(pointD);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointE);
  glVertex3dv(pointA);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glEnd();

  glColor3d(cube_color[0], cube_color[1], cube_color[2]);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glVertex3dv(pointE);
  glEnd();
}

void draw_lifegame(){
    std::default_random_engine generator;
    std::bernoulli_distribution distribution(0.5);
    std::uniform_int_distribution<> dist1(-1.0, 1.0);
    for (int y = 0; y <= GRID_HEIGHT; y++){
        for (int x = 0; x <= GRID_WIDTH; x++){
            if (distribution(generator)){
                is_live[y][x] = true;
            } else {
                is_live[y][x] = false;
            }
            if (is_live[y][x]){
                draw_cube(Point((double)x, (double)y, 0.0), lightblue);
            }
        }
    }
}