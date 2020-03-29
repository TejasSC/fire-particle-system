////////////////////////////////////////////////////////////////
// Department of Computer Science
// The University of Manchester
//
// This code is licensed under the terms of the Creative Commons
// Attribution 2.0 Generic (CC BY 3.0) License.
//
// Skeleton code for COMP37111 coursework, 2019-20
//
// Authors: Toby Howard and Arturs Bekasovs
//
/////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <math.h>

//#include "frames.h"

#ifdef MACOSX
  #include <GLUT/glut.h>
#else
  #include <GL/glut.h>
#endif
GLdouble lat,     lon;              /* View angles (degrees)    */
GLdouble mlat,    mlon;             /* Mouse look offset angles */
GLfloat  eyex,    eyey,    eyez;    /* Eye point                */
GLfloat  centerx, centery, centerz; /* Look point               */
GLfloat  upx,     upy,     upz;     /* View up vector           */

GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 };
/* Set to 0 or 1 for normal or reversed mouse Y direction */
#define INVERT_MOUSE 0

#define RUN_SPEED  1.6
#define TURN_ANGLE 4.0
#define DEG_TO_RAD 0.017453293
const float g = -9.81;
const int MAX_PARTICLES = 10000;
GLfloat ang = 0.0;
GLint width= 1000, height= 800;/*size of window*/
GLfloat PI = 3.141592654;
int TIME_STEP = 0.0;
float ringMin = -200.0; float ringMax = 200.0;
// Display list for coordinate axis
GLuint axisList;

float intensity = 3.0;
float intensity_rate = 0.2;
float fade_rate = 0.02;
float fade = 1.0;
float R= 150.0;
int AXIS_SIZE= 200;
int axisEnabled= 1;
int gravity_on = 1;
int red_colours = 1;
float change = 0.0;
char oxy_header[100];
char ox_rate[10];

clock_t start,end;
double cpu_time_used;

typedef struct Particles {
  // Life
  float fade_level; // decay
  // color
  float rgb[3];//red,green,blue
  // Position/direction
  float pos[3][3];//pos of x,y,z, but it's 3 because it's a triangle
  // Velocity/Direction, only goes down in y dir
  float vel[3];
} Particle;

Particle** firstParticles;
///////////////////////////////////////////////

double myRandom(){
  return (rand()/(double)RAND_MAX);
}

float dist (float x, float y, float z) {
  // returns distance to origin from point (x,y,z)
  return(fabs(sqrt(x*x + z*z)));
} // dist()

//////////////////////////////////////////////

int inside(float x, float y, float z, float inner, float outer) {
  // check if (x,y,z) lies between "inner" and "outer"
  float d;
  d= dist(x,y,z);
  if ((d > inner) && (d < outer)) {return(1);}
  else {return(0);}
} // inside()

Particle* makeParticle(float r, float g, float b, float x, float y, float z) {
  Particle* p = malloc(sizeof(Particle));
  p->fade_level = fade;
  int i;
  for (i = 0; i < 3; i++) {
    if (myRandom() > 0.5) {
      p->pos[0][i] = x+myRandom();
      p->pos[1][i] = y+myRandom();
      p->pos[2][i] = z+myRandom();
    } else {
      p->pos[0][i] = x-myRandom();
      p->pos[1][i] = y-myRandom();
      p->pos[2][i] = z-myRandom();
    }//if
  }//for
  //assign colours
  p->rgb[0] = r;
  p->rgb[1] = g;
  p->rgb[2] = b;
  //assign x y z velocities
  p->vel[0] = 0.2;
  p->vel[1] = 0.0;
  p->vel[2] = 0.2;
  return p;
}//makeParticle

void drawParticle(Particle* p){
  int i;
  if (red_colours) {
    glColor4f(p->rgb[0],p->rgb[1],p->rgb[2], fade);
  } else {
    glColor4f(p->rgb[2],p->rgb[1],p->rgb[0], fade);
  }//if
  //draws particle as triangle, not just a point
  for(i = 0; i < 3; i++){
    glVertex3f(p->pos[0][i],p->pos[1][i],p->pos[2][i]);
  }//for
}//drawParticle

void moveParticle(Particle* p, GLfloat ang){
  int i;
  float a = ang*DEG_TO_RAD;//ang in degrees, must convert to radians
  //update vertical velocity
  float amount = p->pos[1][1]/R;
  float accel = intensity*(cos(10*a));//will be controlled
  if(amount >= 0.0 && amount < 0.25){
    p->vel[1] = (0.1 + amount)*sin(a);
  } else if(amount >= 0.25 && amount < 0.5){
    p->vel[1] = (0.3 + amount)*sin(a);
  } else if(amount >= 0.5){
    p->vel[1] = (0.5 + amount)*sin(a);
  } else if(p->pos[1][1] <= R*(amount + 0.05)) {
    p->pos[1][1] = R*(amount + 0.05);
  }//if
  p->vel[1]+=accel;
  //position updates
  i = 0;
  for (i = 0; i < 3; i++) {
    p->pos[0][i] += p->vel[0]*cos(a);
    p->pos[2][i] += p->vel[2]*sin(a);
  }//for
  p->pos[1][1] += p->vel[1];
}//moveParticle

void makeParticles(Particle** particles){
  start = clock();
  int i;
  float x,y,z,g,b;
  Particle* p;
  for (i = 0; i < MAX_PARTICLES; i++) {
    do {
      //if it's the first bit
      x=R*(2*myRandom()-1);
      y=R*myRandom();
      z=R*(2*myRandom()-1);
    } while(!inside(x,y,z,ringMin,ringMax));
    float amount = 0.5*myRandom();
    if (y >= 0.0 && y < 0.2*R) {
      g = 1.0; b = 1.0;//white (with hint of yellow )
    } else if (y >= 0.2*R && y < 0.85*R){
      g = 1.0 - amount; b=0.0;//yellow
    } else if (y >= 0.85*R && y < R){
      g = 0.5 - amount; b=0.0;//orange
    } else {
      g = 0.0 + amount; b=0.0;//red
    }//if
    p = makeParticle(1.0,g,b,x,y,z);
    //if(p){printf("particle %d has been made\n", i);}
    particles[i] = p;
  }//for
  end = clock();
  cpu_time_used = ((double) (end-start))/CLOCKS_PER_SEC;
  printf("%.4f\n", cpu_time_used);
}//makeParticles

void drawParticles(Particle** particles){
  int i;
  glDisable(GL_LIGHTING); // don't want lighting on
  //transparency should work if next two lines included
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  float size = 3.0 + myRandom()*0.9;
  glPointSize(size);
  glBegin(GL_TRIANGLES);
  for (i = 0; i < MAX_PARTICLES; i++) {
    drawParticle(particles[i]);
  }//for
  glEnd();
}//drawParticles

void moveParticles(Particle** particles, GLfloat ang) {
  glRotatef(ang, 0.0, 1.0, 1.0);
  int i;
  for(i = 0; i < MAX_PARTICLES; i++) {
    moveParticle(particles[i],ang);
  }//for
}//moveParticles

void calculate_lookpoint(double lat, double lon, double eyex, double eyey, double eyez) {
  double dir_x, dir_y, dir_z;
  dir_x = cos((mlat+lat)*DEG_TO_RAD)*sin((mlon+lon)*DEG_TO_RAD);
  dir_y = sin((mlat+lat)*DEG_TO_RAD);
  dir_z = cos((mlat+lat)*DEG_TO_RAD)*cos((mlon+lon)*DEG_TO_RAD);
  //C = E + D
  centerx = eyex + dir_x;
  centery = eyey + dir_y;
  centerz = eyez + dir_z;
} // calculate_lookpoint()

///////////////////////////////////////////////

void menu (int menuentry) {
  switch (menuentry) {
    case 1: axisEnabled = 1 - axisEnabled;
    break;
    case 2: red_colours = 1 - red_colours;
    break;
    case 3: exit(0);
  }//switch
}//menu

void init(){
  glutCreateMenu (menu);
  glutAddMenuEntry ("Toggle axes", 1);
  glutAddMenuEntry ("Toggle colours", 2);
  glutAddMenuEntry ("Quit", 3);
  glutAttachMenu (GLUT_RIGHT_BUTTON);
  /* Set initial view parameters */
  eyex=  300.0; /* Set eyepoint at eye height within the scene */
  eyey=  150.0;
  eyez=  300.0;

  upx= 0.0;   /* Set up direction to the +Y axis  */
  upy= 1.0;
  upz= 0.0;

  lat= 0.0;   /* Look horizontally ...  */
  lon= 0.0;   /* ... along the +Z axis  */

  mlat= 0.0;  /* Zero mouse look angles */
  mlon= 0.0;
  firstParticles = malloc(MAX_PARTICLES*sizeof(Particle));
  //secondParticles = malloc(MAX_PARTICLES*sizeof(Particle));
  makeParticles(firstParticles);
  //makeParticles(secondParticles,1);
}//init

void physics(void) {
  //printf("yeet\n");
  ang= ang + 1.0; if (ang > 360.0) ang= ang - 360.0;
  moveParticles(firstParticles,ang);
  //moveParticles(secondParticles,ang);
  glutPostRedisplay();
}

///////////////////////////////////////////////

void drawString (void *font, float x, float y, char *str){
  char *current;
  glColor3f(1.0,1.0,1.0);
  glRasterPos2f(x,y);
  for (current = str; *current; current++)
    glutBitmapCharacter(font, (int) *current);
}//drawString

void display(){
  // Clear the screen
  glClear(GL_COLOR_BUFFER_BIT);
  //frameStart();
  glLoadIdentity();
  calculate_lookpoint(lat, lon, eyex, eyey, eyez);
  gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);
  // If enabled, draw coordinate axis
  if(axisEnabled) glCallList(axisList);
  drawParticles(firstParticles);
  //frameEnd(GLUT_BITMAP_HELVETICA_10, 1.0, 1.0, 1.0, 0.05, 0.95);
  glutSwapBuffers();
}//display

void freeParticle(Particle* p){
  free(p->pos);
  free(p->rgb);
  free(p->vel);
  free(p);
}//freeParticle
///////////////////////////////////////////////

void mouse_motion(int x, int y) {
  //compute mlon and mlat
  mlon = 50 - (100*x)/(double)width;
  mlat = 50 - (100*y)/(double)height;
} // mouse_motion()

void keyboard(unsigned char key, int x, int y){
  switch (key) {
    case 27: exit(0); break;
    case 97: axisEnabled = 1 - axisEnabled; break;//a for axes
    case 99: red_colours = 1 - red_colours; break;//c for colour change
    case 103: gravity_on  = 1 - gravity_on; break;//g for gravity on
    case 111: fade+=fade_rate; if(fade > 1.0){fade = 1.0;}
      break;//o for oxygen, more
    case 107: fade-=fade_rate; if(fade < 0.0){fade = 0.0;}
      break;//k for oxygen, less
    case 106: intensity+=intensity_rate; if(intensity > 3.0){intensity = 3.0;}
      break;//i for intensity, more
    case 105: intensity-=intensity_rate; if(intensity < 0.0){intensity = 0.0;}
      break;//j for intensity, less
    case 119:
      eyey += RUN_SPEED; break;//w for up
    case 115:
      eyey -= RUN_SPEED; break;//s for down
    case 44://comma
      eyex += RUN_SPEED*sin((lon+90)*DEG_TO_RAD);
      eyez += RUN_SPEED*cos((lon+90)*DEG_TO_RAD);
      break;
    case 46://full stop
      eyex += RUN_SPEED*sin((lon-90)*DEG_TO_RAD);
      eyez += RUN_SPEED*cos((lon-90)*DEG_TO_RAD);
      break;
  }//key
}//keyboard

void cursor_keys(int key, int x, int y) {
  switch (key) {
    case GLUT_KEY_LEFT:  lon += TURN_ANGLE; break;
    case GLUT_KEY_RIGHT: lon -= TURN_ANGLE; break;
    case GLUT_KEY_PAGE_UP:
      lat += TURN_ANGLE; if (lat >= 90.0) lat = 89.9; break;
    case GLUT_KEY_PAGE_DOWN:
      lat -= TURN_ANGLE; if (lat <= -90.0) lat = -89.9; break;
    case GLUT_KEY_HOME: lat = 0.0; break;
    case GLUT_KEY_UP:
      eyex += RUN_SPEED*sin(lon*DEG_TO_RAD);
      eyez += RUN_SPEED*cos(lon*DEG_TO_RAD);
      break;
    case GLUT_KEY_DOWN:
      eyex -= RUN_SPEED*sin(lon*DEG_TO_RAD);
      eyez -= RUN_SPEED*cos(lon*DEG_TO_RAD);
      break;
  }
} // cursor_keys()

///////////////////////////////////////////////

void reshape(int width, int height){
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glViewport(0, 0, (GLsizei)width, (GLsizei)height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60, (GLfloat)width / (GLfloat)height, 1.0, 10000.0);
  glMatrixMode(GL_MODELVIEW);
}

///////////////////////////////////////////////

void makeAxes() {
  axisList = glGenLists(1);
  glNewList(axisList, GL_COMPILE);
      glLineWidth(2.0);
      glBegin(GL_LINES);
      glColor3f(1.0, 0.0, 0.0);       // X axis - red
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(AXIS_SIZE, 0.0, 0.0);
      glColor3f(0.0, 1.0, 0.0);       // Y axis - green
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, AXIS_SIZE, 0.0);
      glColor3f(0.0, 0.0, 1.0);       // Z axis - blue
      glVertex3f(0.0, 0.0, 0.0);
      glVertex3f(0.0, 0.0, AXIS_SIZE);
    glEnd();
  glEndList();
}//make Axes

void timer(int ms){
  glutTimerFunc(ms, timer, ms);
  glutPostRedisplay();
}//timer

///////////////////////////////////////////////
void initGraphics(int argc, char *argv[]){
  strcpy(oxy_header, "O2 proportion is ");
  gcvt(fade,3,ox_rate);
  glutInit(&argc, argv);
  glutInitWindowSize(800, 600);
  glutInitWindowPosition(100, 100);
  glutInitDisplayMode(GLUT_DOUBLE);
  glutCreateWindow("COMP37111 Particles");
  glutDisplayFunc(display);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc (cursor_keys);
  glutPassiveMotionFunc (mouse_motion);
  glutReshapeFunc(reshape);
  glutIdleFunc(physics);
  glutTimerFunc(50, timer, 50);
  makeAxes();
}

/////////////////////////////////////////////////

int main(int argc, char *argv[]){
  srand(time(NULL));
  initGraphics(argc, argv);
  glEnable(GL_POINT_SMOOTH);
  init();
  glutMainLoop();
  return 0;
}
