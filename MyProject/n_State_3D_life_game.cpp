#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include <random>
#include <string.h>
#include <vector>
#include <iostream>
#include <thread>

#define WINDOW_X (1000)
#define WINDOW_Y (1000)
#define WINDOW_NAME "test2"

// GL関係のプロトタイプ宣言
void init_GL(int argc, char *argv[]);
void init();
void set_callback_functions();

void glut_display();
void glut_keyboard(unsigned char key, int x, int y);
void glut_special_keyboard(int key, int x, int y);
void glut_mouse(int button, int state, int x, int y);
void glut_motion(int x, int y);
void glut_idle();
void timer(int value);

// 定数
GLfloat lightblue[] = {0.0,0.0, 1.0, 1.0}; // color
GLfloat red[] = {1.0, 0.0, 0.0, 1.0};       // color
GLfloat green[] = {0.0, 1.0, 1.0, 1.0};
GLfloat yellow[] = {1.0, 1.0, 0.0, 1.0};
const int GRID_SIZE_X = 50;             //gridの縦と横
const int GRID_SIZE_Y = 50;
const int GRID_SIZE_Z = 50;
const int BUFSIZE = 1000;
const double GRID_OFFSET = 0.5;
const int TIME_SLICE = 1;
const double INIT_CELL_PROPOTION = 0.2; //初期のcellの割合 0はすべて死滅(のはず)
const double ALPHA = 0.8;

const int N_STATE = 5;
const int birth[] = {4};
const int survive[] = {4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};//,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};

const int color_ver = 1;  // 0 は原点から最奥までのグラデーション, 
                          // 1は状態ごとの色分け
                          // 2は単色, 
                          // 3は中心からのグラデーション
static int N_birth;
static int N_survive;
static int generation = 0;

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


// lifegame関連のプロトタイプ宣言
int cycle_x(int x);
int cycle_y(int y);
int cycle_z(int z);
void init_cells();
int count_adjacent_cells(int x, int y, int z);
int count_adjacent_cells_2(int x, int y, int z);
void update_cells();
bool check_around(int dx, int dy, int dz, int state);

bool check_near(int x, int y, int z);

// 色変換のプロトタイプ宣言
void rgbTolab(GLfloat rgb_color[], GLfloat lab_color[]);
void labtoRGB(GLfloat lab_color[], GLfloat rgb_color[]);
void hsv2rgb(GLfloat hsv_color[], GLfloat rgb_color[]);
void rgb2hsv(GLfloat rgb_color[], GLfloat hsv_color[]);

// 物体描画関連のプロトタイプ宣言
void draw_pyramid();
void draw_cube(Point p, GLfloat cube_color[]);
void draw_cube_trans(Point p, GLfloat cube_color[]); 
void draw_grid();
void draw_cubic_line(int mode);
void draw_lifegame();
void draw_axis(double length);
void draw_string(std::vector<std::string> strs, int w, int h, int x0, int y0);

// グローバル変数
double g_angle1 = 0.0;
double g_angle2 = 0.0;
double g_distance = 3 * GRID_SIZE_X;

bool g_isLeftButtonOn = false;
bool g_isRightButtonOn = false;
bool is_stop = false;
bool is_move = false;
static bool is_editor_mode = false;
static int INPUT_NUM = 4; //0 なら　tmp.txtから持ってくる
                         //1 はランダム
                         //2はN_STATE-1のランダムは
                         //3は固定値
                         //4は固定値

int cell[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
int cell_next[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
int cell_temp[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
//double cell_size[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
static int counter = 0;
static int speed = 15;
static int edit_now[3] = {GRID_SIZE_X/2, GRID_SIZE_Y/2, GRID_SIZE_Z/2};



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
  glClearColor(0.2, 0.2, 0.2, 0.2);         // 背景の塗りつぶし色を指定
  init_cells();
  N_birth = sizeof(birth)/sizeof(*birth);
  N_survive = sizeof(survive)/sizeof(*survive);
}

void set_callback_functions(){
  glutDisplayFunc(glut_display);
  glutKeyboardFunc(glut_keyboard);
  glutSpecialFunc(glut_special_keyboard);
  glutMouseFunc(glut_mouse);
  glutMotionFunc(glut_motion);
  glutPassiveMotionFunc(glut_motion);
  //glutIdleFunc(glut_idle);
  glutTimerFunc(TIME_SLICE , timer , 0);
}

void glut_keyboard(unsigned char key, int x, int y){
  switch(key){
  
  case 'v':
    if (is_move == true)
    {
      is_move = false;
    } else {
      is_move = true;
      g_angle2 = M_1_PI * 1.41421356;
    } 
    if (is_move == true && is_stop == true){
      is_move = false;
    }
    
    break;

  // 一時停止
  // 開始
  case 's':
    if (is_stop == true){
      is_stop = false;
      is_editor_mode = false;
    } else {
      is_stop = true;
      is_move = false;
    }
    break;

  // リセット
  case 'r':
    init_cells();
    break;

  // スピードアップ
  case 'f':
    if (is_stop == true) break;
    if (speed > 2) speed--;
    break;
  
  // スピードダウン
  case 'd':
    if (is_stop == true) break;
    speed++;
    break;

  // 視点を戻す
  case 'l':
    g_angle1 = 0.0;
    g_angle2 = 0.0;
    g_distance = 3 * GRID_SIZE_X;
    break;
  
  // 初期値変更
  case 'c':
    if (INPUT_NUM < 4){
      INPUT_NUM++;
    } else {
      INPUT_NUM = 1;
    }
    break;
    
    
  // 編集
  case 'e':
    is_editor_mode = true;
    is_stop = true;
    break;

    // z軸に-1
    case 'i':
      if (is_editor_mode == true)
      {
        edit_now[2]--;
        edit_now[2] = cycle_z(edit_now[2]);
      }
      break;
    
    case 'p':
      if (is_editor_mode == true){
        if (cell[edit_now[0]][edit_now[1]][edit_now[2]] == 0){
          cell[edit_now[0]][edit_now[1]][edit_now[2]] = N_STATE-1;
        } else {
          cell[edit_now[0]][edit_now[1]][edit_now[2]] --;
        }

      }
      break;
    // z軸に+1
    case 'o':
      if (is_editor_mode == true)
      {
        edit_now[2]++;
        edit_now[2] = cycle_z(edit_now[2]);
      }
      break;

    // 終了
    case 'q':
    case 'Q':
    case '\033':
      exit(0);
  }

  glutPostRedisplay();
}

void glut_special_keyboard(int key, int x, int y)
{
  if (is_editor_mode == true){
    // x軸に-1
    if (key == GLUT_KEY_LEFT){
      edit_now[0]--;
      edit_now[0] = cycle_x(edit_now[0]);
      }

    // y軸に-1
    if (key == GLUT_KEY_DOWN){
      edit_now[1]--;
      edit_now[1] = cycle_y(edit_now[1]);
    }
    
    // y軸に+1
    if (key == GLUT_KEY_UP){
      edit_now[1]++;
      edit_now[1] = cycle_y(edit_now[1]);
    }

    // x軸に+1
    if (key == GLUT_KEY_RIGHT){
      edit_now[0]++;
      edit_now[0] = cycle_x(edit_now[0]);
    }
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
      g_distance += (double) (y - py) / 10;
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
  gluPerspective(40.0, 1.0, 0.1, g_distance * 10);  // ここでカメラの写せる範囲を指定している

  // つぎにモデル・ビュー変換
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  if (is_move == false){
    if (cos(g_angle2) > 0){
      gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                g_distance * sin(g_angle2),
                g_distance * cos(g_angle2) * cos(g_angle1),
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);
    } else {
      gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                g_distance * sin(g_angle2),
                g_distance * cos(g_angle2) * cos(g_angle1),
                0.0, 0.0, 0.0
                , 0.0, -1.0, 0.0);
    }
  } else {
    if (cos(g_angle2) > 0)
    {
      gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                g_distance * sin(g_angle2),
                g_distance * cos(g_angle2) * cos(g_angle1),
                0.0, 0.0, 0.0,
                0.0, 1.0, 0.0);
    }
    else
    {
      gluLookAt(g_distance * cos(g_angle2) * sin(g_angle1),
                g_distance * sin(g_angle2),
                g_distance * cos(g_angle2) * cos(g_angle1),
                0.0, 0.0, 0.0, 0.0, -1.0, 0.0);
    }
  }

  
/*
  GLfloat lightpos[] = {(float)(g_distance * cos(g_angle2) * sin(g_angle1)),
          (float)(g_distance * sin(g_angle2)),
          (float)(g_distance * cos(g_angle2) * cos(g_angle1)),
          1.0};
*/
  GLfloat lightpos[] = {GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z};
  GLfloat lightpos1[] = {GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z};
  GLfloat diffuse[] = {1.0, 1.0, 1.0, 0.5};
  GLfloat ambient[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat specular[] = {0.7, 0.7, 0.7, 1.0};

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_LINE_SMOOTH);
  //glEnable(GL_CULL_FACE);

  if (is_editor_mode == true)
  {
    glPushMatrix();
    glTranslatef(-GRID_SIZE_X / 2, -GRID_SIZE_Y / 2, -GRID_SIZE_Z / 2);
    draw_axis(GRID_SIZE_X);
    glPopMatrix();
  }

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  //glCullFace(GL_FRONT);

  /*
  glPushMatrix();
  glTranslatef(g_distance * 0.5, g_distance * 0.5, g_distance * 0.5);
  glutSolidSphere(5.0, 50, 50);
  glPopMatrix();
*/
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);

  glLightfv(GL_LIGHT1, GL_POSITION, lightpos1);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
  glLightfv(GL_LIGHT1, GL_AMBIENT, ambient);
  glLightfv(GL_LIGHT1, GL_SPECULAR, specular);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


  glPushMatrix();
  glTranslatef(-GRID_SIZE_X / 2.0, - GRID_SIZE_Y / 2.0, - GRID_SIZE_Z / 2.0);
  
  draw_lifegame();

  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHT1);
  glDisable(GL_LIGHTING);

  //draw_grid();

  if (is_editor_mode == true){
    draw_cubic_line(1);
  } else if (is_stop == true) {
    draw_cubic_line(2);
  } else {
    draw_cubic_line(0);
  }
  
  glPopMatrix();

  //glPushMatrix();
  //glutSolidSphere(10, 50, 50);
  //glPopMatrix();

  glFlush();

  glColor3d(1.0, 1.0, 1.0);

  if (is_editor_mode){
    std::vector<std::string> strs;
    strs.push_back("generation: " + std::to_string(generation));
    strs.push_back("Mode : Edit");
    strs.push_back("\"s\" key : start");
    strs.push_back("\"r\" key : reset");
    strs.push_back("right/left key : x axis move");
    strs.push_back("up/down key : y axis move");
    strs.push_back(" \"i\", \"o\" key : z axis move");
    strs.push_back("\"p\" key : state change");
    draw_string(strs, WINDOW_X, WINDOW_Y, 10, 10);
  } else if (is_stop){
    std::vector<std::string> strs;
    strs.push_back("generation: " + std::to_string(generation));
    strs.push_back("Mode : Stop");
    strs.push_back(" \"s\" key : start");
    strs.push_back(" \"r\" key : reset" );
    draw_string(strs, WINDOW_X, WINDOW_Y, 10, 10);
  } else if (is_move){
    std::vector<std::string> strs;
    strs.push_back("generation: " + std::to_string(generation));
    strs.push_back("Mode : Move");
    strs.push_back(" \"s\" key : stop");
    strs.push_back(" \"r\" key : reset");
    strs.push_back(" \"e\" key : edit");
    strs.push_back(" \"f\", \"d\" key : fast/slow");
    strs.push_back(" \"q\" key : quit");
    draw_string(strs, WINDOW_X, WINDOW_Y, 10, 10);
  } else {
    std::vector<std::string> strs;
    strs.push_back("generation: " + std::to_string(generation));
    strs.push_back("Mode : Normal");
    strs.push_back(" \"s\" key : stop");
    strs.push_back(" \"r\" key : reset");
    strs.push_back(" \"e\" key : edit");
    strs.push_back(" \"f\", \"d\" key : fast/slow");
    strs.push_back(" \"q\" key : quit");
    draw_string(strs, WINDOW_X, WINDOW_Y, 10, 10);
  }

  glDisable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);


  glutSwapBuffers();
}


// 背景にグリッドを生成
// gridの3次元はできてない
// 必要になったらやる
void draw_grid(){
    GLdouble pointO[] = {0.0, 0.0, -GRID_OFFSET};
    GLdouble pointA[] = {GRID_SIZE_X, 0.0, -GRID_OFFSET};
    for (int i = 0; i <= GRID_SIZE_Y; i++){
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
    GLdouble pointB[] = {0.0, 0.0, -GRID_OFFSET};
    GLdouble pointC[] = {0.0, GRID_SIZE_Y, -GRID_OFFSET};
    for (int i = 0; i <= GRID_SIZE_X; i++){
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

void draw_cubic_line(int mode){
    GLfloat vertics[8][3] =
        {
            {0.0f, 0.0f, 0.0f},                         // 0
            {GRID_SIZE_X, 0.0f, 0.0f},                  // 1
            {0.0f, GRID_SIZE_Y, 0.0f},                  // 2
            {0.0f, 0.0f, GRID_SIZE_Z},                  // 3
            {GRID_SIZE_X, GRID_SIZE_Y, 0.0f},           // 4
            {0.0f, GRID_SIZE_Y, GRID_SIZE_Z},           // 5
            {GRID_SIZE_X, 0.0f, GRID_SIZE_Z},           // 6
            {GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z},    // 7
        };

    glLineWidth(1.0);
    switch(mode){
      case 0:
        glColor3f(1.0, 1.0, 1.0);
        break;
      case 1:
        glColor3d(0.0, 1.0, 0.0);
        break;
      case 2:
        glColor3d(1.0, 0.0, 0.0);
        break;
      default:
        exit(0);
    }
     

    for (int i = 0; i < 4; i++){
        glBegin(GL_LINES);
        glVertex3fv(vertics[0]);
        glVertex3fv(vertics[i]);
        glEnd();
    }
    
    for (int i = 4; i < 7; i++){
        glBegin(GL_LINES);
        glVertex3fv(vertics[7]);
        glVertex3fv(vertics[i]);
        glEnd();
    }
    glBegin(GL_LINES);
    glVertex3fv(vertics[1]);
    glVertex3fv(vertics[4]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[2]);
    glVertex3fv(vertics[5]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[3]);
    glVertex3fv(vertics[6]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[1]);
    glVertex3fv(vertics[6]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[2]);
    glVertex3fv(vertics[4]);
    glEnd();

    glBegin(GL_LINES);
    glVertex3fv(vertics[3]);
    glVertex3fv(vertics[5]);
    glEnd();
}


void timer(int value) {
  if(counter % speed == 0){
    if(is_move){
      g_angle1 += M_1_PI * 0.015;
    }
    update_cells();
    if (is_move)
    {
      g_angle1 += M_1_PI * 0.015;
    }
  }
  if (!is_stop) counter++;
  if (counter > 1000) counter = 0;
  if (is_move){
    g_angle1 += M_1_PI * 0.015;
  }
  
	glutPostRedisplay();
	glutTimerFunc(TIME_SLICE , timer , 0);
}


// Point{x, y, z}を起点にPoint{x+1, y+1, z+1}の立方体を描画する。
void draw_cube(Point p, GLfloat cube_color[]){

  // 1.0だと同一平面上に描画することになり、いろいろあれなので微小区間だけ縮めた
  GLdouble pointO[] = {p.x, p.y, p.z};
  GLdouble pointA[] = {p.x, p.y+0.999, p.z};
  GLdouble pointB[] = {p.x, p.y+0.999, p.z+0.999};
  GLdouble pointC[] = {p.x, p.y, p.z+0.999};
  GLdouble pointD[] = {p.x+0.999, p.y, p.z};
  GLdouble pointE[] = {p.x+0.999, p.y+0.999, p.z};
  GLdouble pointF[] = {p.x+0.999, p.y+0.999, p.z+0.999};
  GLdouble pointG[] = {p.x+0.999, p.y, p.z+0.999};
  
  //glPushMatrix();
  //glTranslatef(-GRID_SIZE_X/ 2.0, GRID_SIZE_Y/2.0, GRID_SIZE_Z/2.0);

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

  //glPopMatrix();
}

// Point{x, y, z}を起点にPoint{x+1, y+1, z+1}の立方体を描画する。
// ただし透過する
void draw_cube_trans(Point p, GLfloat cube_color[]){
// counter 
  // 1.0だと同一平面上に描画することになり、いろいろあれなので微小区間だけ縮めた
  GLdouble pointO[] = {p.x+0.0001, p.y+0.0001, p.z+0.0001};
  GLdouble pointA[] = {p.x+0.0001, p.y+0.9999, p.z+0.0001};
  GLdouble pointB[] = {p.x+0.0001, p.y+0.9999, p.z+0.9999};
  GLdouble pointC[] = {p.x+0.0001, p.y+0.0001, p.z+0.9999};
  GLdouble pointD[] = {p.x+0.9999, p.y+0.0001, p.z+0.0001};
  GLdouble pointE[] = {p.x+0.9999, p.y+0.9999, p.z+0.0001};
  GLdouble pointF[] = {p.x+0.9999, p.y+0.9999, p.z+0.9999};
  GLdouble pointG[] = {p.x+0.9999, p.y+0.0001, p.z+0.9999};
  
  GLfloat specular[] = {0.2, 0.2, 0.2, 1.0};
  GLfloat ambient[] = {cube_color[0] * 0.5, cube_color[1] * 0.5, cube_color[2] * 0.5, 1.0};

  glMaterialfv(GL_FRONT, GL_DIFFUSE, cube_color);
  glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
  glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);

  glNormal3d(-1.0, 0.0, 0.0);
  //glColor4dv(cube_color);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glEnd();


  glNormal3d(0.0, 0.0, -1.0);
  //glColor4dv(cube_color);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointA);
  glVertex3dv(pointE);
  glVertex3dv(pointD);
  glEnd();


  glNormal3d(0.0, -1.0, 0.0);
  //glColor4dv(cube_color);
  glBegin(GL_POLYGON);
  glVertex3dv(pointO);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glVertex3dv(pointD);
  glEnd();


  glNormal3d(0.0, 1.0, 0.0);
  //glColor4dv(cube_color);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointA);
  glVertex3dv(pointE);
  glEnd();


  glNormal3d(0.0, 0.0, 1.0);
  //glColor4dv(cube_color);
  glBegin(GL_POLYGON);
  glVertex3dv(pointF);
  glVertex3dv(pointB);
  glVertex3dv(pointC);
  glVertex3dv(pointG);
  glEnd();


  glNormal3d(1.0, 0.0, 0.0);
  //glColor4dv(cube_color);
  glBegin(GL_POLYGON);
  glVertex3dv(pointD);
  glVertex3dv(pointE);
  glVertex3dv(pointF);
  glVertex3dv(pointG);
  glEnd();



  GLfloat vertics[8][3] =
      {
          {p.x+0.0f, p.y+0.0f, p.z+0.0f},                      // 0
          {p.x+1.0f, p.y+0.0f, p.z+0.0f},               // 1
          {p.x+0.0f, p.y+1.0f, p.z+0.0f},               // 2
          {p.x+0.0f, p.y+0.0f, p.z+1.0f},               // 3
          {p.x+1.0f, p.y+1.0f, p.z+0.0f},        // 4
          {p.x+0.0f, p.y+1.0f, p.z+1.0f},        // 5
          {p.x+1.0f, p.y+0.0f, p.z+1.0f},        // 6
          {p.x+1.0f, p.y+1.0f, p.z+1.0f}, // 7
      };

  

  glLineWidth(2.0);
  glColor3f(1.0, 1.0, 1.0);

  for (int i = 0; i < 4; i++)
  {
    glBegin(GL_LINES);
    glVertex3fv(vertics[0]);
    glVertex3fv(vertics[i]);
    glEnd();
  }

  for (int i = 4; i < 7; i++)
  {
    glBegin(GL_LINES);
    glVertex3fv(vertics[7]);
    glVertex3fv(vertics[i]);
    glEnd();
  }
  glBegin(GL_LINES);
  glVertex3fv(vertics[1]);
  glVertex3fv(vertics[4]);
  glEnd();

  glBegin(GL_LINES);
  glVertex3fv(vertics[2]);
  glVertex3fv(vertics[5]);
  glEnd();

  glBegin(GL_LINES);
  glVertex3fv(vertics[3]);
  glVertex3fv(vertics[6]);
  glEnd();

  glBegin(GL_LINES);
  glVertex3fv(vertics[1]);
  glVertex3fv(vertics[6]);
  glEnd();

  glBegin(GL_LINES);
  glVertex3fv(vertics[2]);
  glVertex3fv(vertics[4]);
  glEnd();

  glBegin(GL_LINES);
  glVertex3fv(vertics[3]);
  glVertex3fv(vertics[5]);
  glEnd();
}


// おそらくswitch文で書き換えるべき

void draw_lifegame(){
  GLfloat point_color[4];
  if (color_ver == 0){ 
    // 原点から再奥までのグラデーション
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]){
            point_color[0] = (double)(x) / GRID_SIZE_X;
            point_color[1] = (double)(y) / GRID_SIZE_Y;
            point_color[2] = (double)(z) / GRID_SIZE_Z;
            point_color[3] = 1.0;
            draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  } else if (color_ver == 1){
    // 状態に応じて色を変える
    GLfloat hsv_color[4] = {0.0, 1.0, 1.0};
    GLfloat rgb_colors[N_STATE-1][4];
    for (int i = 0; i < N_STATE-1; i++){
      hsv_color[0] = 10.0 * (float)(N_STATE-2-i);
      //std::cout << hsv_color[0] << std::endl;
      hsv2rgb(hsv_color, rgb_colors[i]);
      //std::cout << rgb_colors[i][0] << rgb_colors[i][1] << rgb_colors[i][2] << std::endl;
    }
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]){
            for (int i = 0; i < N_STATE-1; i++){
              if (cell[x][y][z] == i+1){
                point_color[0] = rgb_colors[i][0];
                point_color[1] = rgb_colors[i][1];
                point_color[2] = rgb_colors[i][2];
              }
            }
/*

            if (cell[x][y][z] == N_STATE - 1){
              point_color[0] = red[0];
              point_color[1] = red[1];
              point_color[2] = red[2];
            }
            else if (cell[x][y][z] == N_STATE - 2)
            {
              point_color[0] = lightblue[0];
              point_color[1] = lightblue[1];
              point_color[2] = lightblue[2];
            }
            else if (cell[x][y][z] == 2){
              point_color[0] = green[0];
              point_color[1] = green[1];
              point_color[2] = green[2];
            }
            else if (cell[x][y][z] == 1)
            {
              point_color[0] = yellow[0];
              point_color[1] = yellow[1];
              point_color[2] = yellow[2];
            }
            */

            if (is_editor_mode == false){
              point_color[3] = 1.0;
              draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
            } else {
              if (x == edit_now[0] && y == edit_now[1] && z == edit_now[2]){
                point_color[3] = 1.0;
                if (0 < x && x < GRID_SIZE_X && 0 < y && y < GRID_SIZE_Y && 0 < z &&z < GRID_SIZE_Z){
                  if (!check_near(x, y, z)){
                    draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
                  }
                } else {
                 draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
                }
              } else {
                point_color[3] = 0.2;
                draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
              }
            }
            
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
          else if (is_editor_mode == true)
          {
            if (x == edit_now[0] && y == edit_now[1] && z == edit_now[2])
            {
              point_color[0] = point_color[1] = point_color[2] = point_color[3] = 1.0;
              draw_cube_trans(Point((double)x, (double)y, double(z)), point_color);
            }
          }
        }
      }
    }
  } else if (color_ver == 2){
    // 単色
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]) {
            draw_cube_trans(Point((double)x, (double)y, double(z)), red);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  } else if (color_ver == 3){
    // 中心からのグラデーション
    for (int y = 0; y < GRID_SIZE_Y; y++){
      for (int x = 0; x < GRID_SIZE_X; x++){
        for (int z = 0; z < GRID_SIZE_Z; z++){
          if (cell[x][y][z]){
            point_color[0] = (double)std::abs(x - GRID_SIZE_X / 2) / GRID_SIZE_X;
            point_color[1] = 0.2;
            point_color[2] = 0.2;
            point_color[3] = 1.0;
            draw_cube_trans(Point((double)x, (double)y, double(z)), red);
            // ここに
            // draw_cube_fade(Point((double)x, (double)y, double(z)), point_color, cell_size);
            // みたいなのを付け足したい。
          }
        }
      }
    }
  }
}



void init_cells(){
  int x = 0, y;
  char buf[BUFSIZE];
  FILE *fp;
  const char *filename = "tmp.txt";

  generation = 0;

  for (int y = 0; y < GRID_SIZE_Y; y++)
  {
    for (int x = 0; x < GRID_SIZE_X; x++)
    {
      for (int z = 0; z < GRID_SIZE_Z; z++)
      {
        cell[x][y][z] = 0;
      }
    }
  }

  if (INPUT_NUM == 0){ //  仕方がないので z = 0 として初期化を行う
    if ((fp = fopen(filename, "r")) == NULL) {
      printf("error: can't open %s\n", filename);
      return;
    }

    while (fgets(buf, BUFSIZE, fp) != NULL) {
      size_t len = strlen(buf) - 1;
      for (x = 0; x < len; x++){
        if (buf[x] == ' '){
          cell[x][y][0] = 0;
        } else {
          cell[x][y][0] = N_STATE-1;
        }
      }
      y++;
    }
    fclose(fp);
  } else if (INPUT_NUM == 1){
    // 乱数に従って最初の分布を生成
    std::random_device seed_gen;
    std::default_random_engine engine(seed_gen());
    std::bernoulli_distribution distribution(INIT_CELL_PROPOTION);
    for (int y = GRID_SIZE_Y / 4; y < GRID_SIZE_Y / 4 * 3; y++){
      for (int x = GRID_SIZE_X / 4; x < GRID_SIZE_X / 4 * 3; x++){
        for (int z = GRID_SIZE_Z / 4; z < GRID_SIZE_Z / 4 * 3; z++)
        {
          for (int i = 1; i < N_STATE; i++){
            if (distribution(engine)){
              cell[x][y][z] = N_STATE - i;
            }
          }
          //if (distribution(engine))
          //{
          //  cell[x][y][z] = N_STATE - 1;
          //}
        
        }
        }
    }
  }else if (INPUT_NUM == 2){
      // 乱数に従って最初の分布を生成
      std::random_device seed_gen;
      std::default_random_engine engine(seed_gen());
      std::bernoulli_distribution distribution(INIT_CELL_PROPOTION);
      for (int y = GRID_SIZE_Y / 4; y < GRID_SIZE_Y / 4 * 3; y++)
      {
        for (int x = GRID_SIZE_X / 4; x < GRID_SIZE_X / 4 * 3; x++)
        {
          for (int z = GRID_SIZE_Z / 4; z < GRID_SIZE_Z / 4 * 3; z++)
          {
            if (distribution(engine))
            {
              cell[x][y][z] = N_STATE - 1;
            }
          }
        }
      }
  }else if (INPUT_NUM == 3){
    for (int x = 0; x < N_STATE; x++){
      for (int y = GRID_SIZE_Y/2-1; y <= GRID_SIZE_Y/2; y++){
        for (int z = GRID_SIZE_Z / 2 - 1; z <= GRID_SIZE_Z / 2; z++){
          cell[x + GRID_SIZE_X/2][y][z] = x;
        }
      }
    }
  }
  else if (INPUT_NUM == 4)
  {
    for (int x = 0; x < N_STATE; x++)
    {
      for (int y = GRID_SIZE_Y / 2 - 1; y <= GRID_SIZE_Y / 2; y++)
      {
        for (int z = GRID_SIZE_Z / 2 - 1; z <= GRID_SIZE_Z / 2 + 1; z++)
        {
          if (z == GRID_SIZE_Z/2 - 1){
            cell[x + GRID_SIZE_X / 2][y][z] = N_STATE-1-x;
          } else if (z == GRID_SIZE_Z/2){
            cell[x + GRID_SIZE_X / 2][y][z] = N_STATE - 1 - x;
            cell[GRID_SIZE_X / 2 + N_STATE-2][y][z] = 0;
          } else {
            cell[x + GRID_SIZE_X / 2 + 1][y][z] = N_STATE - 1 - x;
            cell[GRID_SIZE_X / 2 + N_STATE - 1][y][z] = 0;
          }
        }
      }
    }
  }
}


// 周り 3^3 - 1 = 26 個を探索する
//intをboolでやる方法がよくわからんけど

// ここのcheck_around()は編集

int count_adjacent_cells(int x, int y, int z){
  int n = 0;
  int dx, dy, dz;
  int state = cell[x][y][z];

  for (dx = x - 1; dx <= x + 1; dx++) {
    for (dy = y - 1; dy <= y + 1; dy++) {
      for (dz = z - 1; dz <= z + 1; dz++){
        if (dx == x && dy == y && dz == z) continue;
        if (check_around(cycle_x(dx), cycle_y(dy), cycle_z(dz), state) == true){
          n++;
        }
      }
    }
  }
  return n;
}

int count_adjacent_cells_2(int x, int y, int z)
{
  int n = 0;
  int dx, dy, dz;
  int state = cell[x][y][z];

  for (dx = x - 1; dx <= x + 1; dx++)
  {
    for (dy = y - 1; dy <= y + 1; dy++)
    {
      for (dz = z - 1; dz <= z + 1; dz++)
      {
        if (dx == x && dy == y && dz == z)
          continue;
        if (check_around(cycle_x(dx), cycle_y(dy), cycle_z(dz), state+1) == true)
        {
          n++;
        }
      }
    }
  }
  return n;
}

// 自分の状態によって数える周りの状態が変わる
// dx, dy, dz は周りのセル
// state は自分の状態

bool check_around(int dx, int dy, int dz, int state){
  if (state == 0){
    // 新しいセルを発生させられるかどうか
    return cell[dx][dy][dz] >= N_STATE - 1;   
  } else {
    // 自分の状態より「良い」状態のセルかどうか
    return cell[dx][dy][dz] == state - 1; //  == true;
    // ここは >= stateが一番動いてるけど...
    // > state, やってほしいのは   == state+1
  }
}

bool check_birth(int x, int y, int z, int n){
    bool birth_val = false;
    if (cell[x][y][z] > 0) return false;
    for (int i = 0; i < N_birth; i++){
      if (birth[i] == n){
        birth_val = true;
        break;
      }
    }
    return birth_val;
}

bool check_survive(int x, int y, int z, int n){
  bool survive_val = false;
  // 自分がsurviveじゃないなら関係ない。
  if (cell[x][y][z] == 0) return false;
  for (int i = 0; i < N_survive; i++){
    if (survive[i] == n){
      survive_val = true;
      break;
    }
  }
  return survive_val;
}

// この関数によって次の状態の細胞の生死を決定するが、、、
// 3Dのあれは想像できないので、今は適当な数字を考える。
void update_cells(){
  int x, y, z;

  for (x = 0; x < GRID_SIZE_X; x++) {
    for (y = 0; y < GRID_SIZE_Y; y++) {
      for (z = 0; z < GRID_SIZE_Z; z++){
        const int n = count_adjacent_cells(x, y, z);
        const int m = count_adjacent_cells_2(x, y, z);
        if (check_survive(x,y,z,n) || check_survive(x, y, z, m)){           // survive
          cell_next[x][y][z] = cell[x][y][z];
          if (cell_next[x][y][z] > 0){
            cell_next[x][y][z]--;
            //if (generation >= 0){
            //  printf("%d, %d\n",cell[x][y][z],cell_next[x][y][z]);
            //}
          }
        } else if (check_birth(x,y,z,n)){ // birth
          cell_next[x][y][z] = N_STATE - 1;     
        } else {
          cell_next[x][y][z] = 0;
        }
      }
    }
  }

  for (x = 0; x < GRID_SIZE_X; x++) {
    for (y = 0; y < GRID_SIZE_Y; y++) {
      for (z = 0; z < GRID_SIZE_Z; z++){
        cell_temp[x][y][z] = cell[x][y][z];
        cell[x][y][z] = cell_next[x][y][z];
      }
    }
  }
  generation++;
}

int cycle_y(int y){        //境界を消去
  if (y >= GRID_SIZE_Y){
    return y - GRID_SIZE_Y;
  } else if (y < 0){
    return y + GRID_SIZE_Y;
  } else{
    return y;
  }
}

int cycle_x(int x){          //境界を消去
  if (x >= GRID_SIZE_X){
    return x - GRID_SIZE_X;
  } else if (x < 0){
    return x + GRID_SIZE_X;
  } else {
    return x;
  }
}

int cycle_z(int z){        //境界を消去
  if (z >= GRID_SIZE_Z){
    return z - GRID_SIZE_Z;
  } else if (z < 0){
    return z + GRID_SIZE_Z;
  } else{
    return z;
  }
}

// https://qiita.com/hachisukansw/items/09caabe6bec46a2a0858
void rgbTolab(GLfloat rgb_color[], GLfloat lab_color[]){

  float r = rgb_color[0];
  float g = rgb_color[1];
  float b = rgb_color[2];

  float x = ((r * 0.49000) + (g * 0.31000) + (b * 0.20000)) / 0.17697;
  float y = ((r * 0.17697) + (g * 0.81240) + (b * 0.01063)) / 0.17697;
  float z = (                (g * 0.01000) + (b * 0.99000)) / 0.17697;

  //https://en.wikipedia.org/wiki/Lab_color_space#CIELAB-CIEXYZ_conversions

  double delta = 6.0 / 29.0;

  x *= 100 / 95.047;
  y *= 100 / 100;
  z *= 100 / 108.883;

  x = x > std::pow(delta, 3.0) ? std::pow(x, 1.0 / 3.0) : (x/3.0 / std::pow(delta, 2.0)) + (4 / 29);
  y = y > std::pow(delta, 3.0) ? std::pow(y, 1.0 / 3.0) : (y/3.0 / std::pow(delta, 2.0)) + (4 / 29);
  z = z > std::pow(delta, 3.0) ? std::pow(z, 1.0 / 3.0) : (z/3.0 / std::pow(delta, 2.0)) + (4 / 29);

  lab_color[0] = (116 * y) - 16;
  lab_color[1] = 500 * (x - y);
  lab_color[2] = 200 * (y - z);

  return ;
}

void labtoRGB(GLfloat lab_color[], GLfloat rgb_color[]){

  float L = lab_color[0];
  float a = lab_color[1];
  float b = lab_color[2];
  
  float x = (L + 16) / 116 + a / 500;
  float y = (L + 16) / 116;
  float z = (L + 16) / 116 - b / 200;

  double delta = 6.0 / 29.0;

  x = x > delta ? std::pow(x, 3.0) : 3 * std::pow(delta, 2.0) * (x - 4.0 / 29.0);
  y = y > delta ? std::pow(y, 3.0) : 3 * std::pow(delta, 2.0) * (y - 4.0 / 29.0);
  z = z > delta ? std::pow(z, 3.0) : 3 * std::pow(delta, 2.0) * (z - 4.0 / 29.0);

  x *= 95.047/100;
  y *= 100/100;
  z *= 108.883/100;

  rgb_color[0] = ((x * 0.49000) + (y * 0.31000) + (z * 0.20000)) / 0.17697;
  rgb_color[1] = ((y * 0.17697) + (y * 0.81240) + (z * 0.01063)) / 0.17697;
  rgb_color[2] = ((z * 0.01000) + (y * 0.99000)) / 0.17697;
}

void rgb2hsv(GLfloat rgb_color[], GLfloat hsv_color[]){
  float rgb_max, rgb_min;
  float diff;
  float base;
  float div1;
  float div2;

  rgb_max = std::max(std::max(rgb_color[0], rgb_color[1]), rgb_color[2]);
  rgb_min = std::min(std::min(rgb_color[0], rgb_color[1]), rgb_color[2]);
  if (rgb_max == rgb_color[0])
  {
    diff = rgb_color[1] - rgb_color[2]; // G-B
    base = (rgb_color[1] < rgb_color[2]) ? 360.0f : 0.0f;
  }
  if (rgb_max == rgb_color[1])
  {
    diff = rgb_color[2] - rgb_color[0]; // B-R
    base = 120.0f;
  }
  if (rgb_max == rgb_color[2])
  {
    diff = rgb_color[0] - rgb_color[1]; // R-G
    base = 240.0f;
  }

  div1 = (rgb_max == rgb_min) ? 1.0f : (rgb_max - rgb_min);
  div2 = (rgb_max > 0.0f) ? rgb_max : 1.0f;
  hsv_color[0] = 60.0f * diff / div1 + base;
  hsv_color[1] = (rgb_max - rgb_min) / div2;
  hsv_color[2] = rgb_max;
}

void hsv2rgb(GLfloat hsv_color[], GLfloat rgb_color[]){
   int Hi;
   float f;
   float p;
   float q;
   float t;
   Hi = fmod(floor(hsv_color[0] / 60.0f), 6.0f);
   f = hsv_color[0] / 60.0f - Hi;
   p = hsv_color[2] * (1.0f - hsv_color[1]);
   q = hsv_color[2] * (1.0f - f * hsv_color[1]);
   t = hsv_color[2] * (1.0f - (1.0f - f) * hsv_color[1]);
   if (Hi == 0)
   {
     rgb_color[0] = hsv_color[2];
     rgb_color[1] = t;
     rgb_color[2] = p;
   }
   if (Hi == 1)
   {
     rgb_color[0] = q;
     rgb_color[1] = hsv_color[2];
     rgb_color[2] = p;
   }
   if (Hi == 2)
   {
     rgb_color[0] = p;
     rgb_color[1] = hsv_color[2];
     rgb_color[2] = t;
   }
   if (Hi == 3)
   {
     rgb_color[0] = p;
     rgb_color[1] = q;
     rgb_color[2] = hsv_color[2];
   }
   if (Hi == 4)
   {
     rgb_color[0] = t;
     rgb_color[1] = p;
     rgb_color[2] = hsv_color[2];
   }
   if (Hi == 5)
   {
     rgb_color[0] = hsv_color[2];
     rgb_color[1] = p;
     rgb_color[2] = q;
   }
 }


void draw_axis(double length){
   GLUquadricObj *arrows[3];

  glLineWidth(2.0);

  // Draw X-axis
  glColor3ub(255, 0, 0);
  glBegin(GL_LINES);
    glVertex3d(0, 0, 0);
    glVertex3d(length, 0, 0);
  glEnd();
  glPushMatrix();
    arrows[0] = gluNewQuadric();
    gluQuadricDrawStyle(arrows[0], GLU_FILL);
    glTranslated(length, 0.0f, 0.0f);
    glScalef(2.5, 2.5, 2.5);
    glRotated(90.0f, 0, 1, 0);
    gluCylinder(arrows[0], length / 40, 0.0f, length / 10, 8, 8);
  glPopMatrix();

  // Draw Y-axis
  glColor3ub(0, 255, 0);
  glBegin(GL_LINES);
    glVertex3d(0, 0, 0);
    glVertex3d(0, length, 0);
  glEnd();
  glPushMatrix();
    arrows[1] = gluNewQuadric();
    gluQuadricDrawStyle(arrows[1], GLU_FILL);
    glTranslated(0.0f, length, 0.0f);
    glScalef(2.5, 2.5, 2.5);
    glRotated(-90.0f, 1, 0, 0);
    gluCylinder(arrows[1], length / 40, 0.0f, length / 10, 8, 8);
  glPopMatrix();

  // Draw Z-axis
  glColor3ub(0, 128, 255);
  glBegin(GL_LINES);
    glVertex3d(0, 0, 0);
    glVertex3d(0, 0, length);
  glEnd();
  glPushMatrix();
    arrows[2] = gluNewQuadric();
    gluQuadricDrawStyle(arrows[2], GLU_FILL);
    glTranslated(0.0f, 0.0f, length);
    glScalef(2.5, 2.5, 2.5);
    gluCylinder(arrows[2], length / 40, 0.0f, length / 10, 8, 8);
  glPopMatrix();
  glColor3ub(255, 255, 255);
  glDisable(GL_DEPTH_TEST);
  glEnable(GL_DEPTH_TEST);
}

void draw_string(std::vector<std::string> strs, int w, int h, int x0, int y0)
{
  glDisable(GL_LIGHTING);
  // 平行投影にする
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, w, h, 0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  y0 += 20;

  // 画面上にテキスト描画
  for (int j = 0; j < (int)strs.size(); ++j)
  {
    glRasterPos2f(x0, y0);
    int size = (int)strs[j].size();
    for (int i = 0; i < size; ++i)
    {
      char ic = strs[j][i];
      glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, ic);
    }
    y0 += 30;
  }

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}


bool check_near(int x, int y, int z){
  return (cell[x+1][y][z] > 0 && 
          cell[x-1][y][z] > 0 &&
          cell[x][y+1][z] > 0 &&
          cell[x][y-1][z] > 0 &&
          cell[x][y][z+1] > 0 &&
          cell[x][y][z-1] > 0);
}