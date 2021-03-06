#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include "GL/glut.h"
#else
#include <GL/freeglut.h>
#endif
#endif

#include <iostream>
#include <cmath>
#include <cstring>
#include "skeleton.h"
#include "defMesh.h"
#include <glm/gtc/matrix_transform.hpp>
#include "quaternion.h"

using namespace std;


//Mesh
DefMesh myDefMesh;

//Switches
int meshModel=0;
bool drawSkeleton=true;
bool add=false;

//Window parameters
int width = 1024;
int height = 768;
///* Ortho (if used) */
double _left = 0.0;		/* ortho view volume params */
double _right = 0.0;
double _bottom = 0.0;
double _top = 0.0;
double _zNear = 0.1;
double _zFar = 50.0;
double fovy = 45.0;
double prev_z = 0;

//Model matrices
double _matrix[16];
double _matrixI[16];

/* Mouse Interface  */
int _mouseX = 0;		/* mouse control variables */
int _mouseY = 0;
bool _mouseLeft = false;
bool _mouseMiddle = false;
bool _mouseRight = false;

double _dragPosX = 0.0;
double _dragPosY = 0.0;
double _dragPosZ = 0.0;

double vlen(double x, double y, double z)
{
 return sqrt(x * x + y * y + z * z);
}

void invertMatrix(const GLdouble * m, GLdouble * out)
{

 /* NB. OpenGL Matrices are COLUMN major. */
#define MAT(m,r,c) (m)[(c)*4+(r)]

 /* MACRO (row,column) to index. */
#define m11 MAT(m,0,0)
#define m12 MAT(m,0,1)
#define m13 MAT(m,0,2)
#define m14 MAT(m,0,3)
#define m21 MAT(m,1,0)
#define m22 MAT(m,1,1)
#define m23 MAT(m,1,2)
#define m24 MAT(m,1,3)
#define m31 MAT(m,2,0)
#define m32 MAT(m,2,1)
#define m33 MAT(m,2,2)
#define m34 MAT(m,2,3)
#define m41 MAT(m,3,0)
#define m42 MAT(m,3,1)
#define m43 MAT(m,3,2)
#define m44 MAT(m,3,3)

 GLdouble det;
 GLdouble d12, d13, d23, d24, d34, d41;
 GLdouble tmp[16];		/* Allow out == in. */

 /* Inverse = adjoint / det. (See linear algebra texts.) */

 /* pre-compute 2x2 dets for last two rows when computing */
 /* cofactors of first two rows. */
 d12 = (m31 * m42 - m41 * m32);
 d13 = (m31 * m43 - m41 * m33);
 d23 = (m32 * m43 - m42 * m33);
 d24 = (m32 * m44 - m42 * m34);
 d34 = (m33 * m44 - m43 * m34);
 d41 = (m34 * m41 - m44 * m31);

 tmp[0] = (m22 * d34 - m23 * d24 + m24 * d23);
 tmp[1] = -(m21 * d34 + m23 * d41 + m24 * d13);
 tmp[2] = (m21 * d24 + m22 * d41 + m24 * d12);
 tmp[3] = -(m21 * d23 - m22 * d13 + m23 * d12);

 /* Compute determinant as early as possible using these cofactors. */
 det = m11 * tmp[0] + m12 * tmp[1] + m13 * tmp[2] + m14 * tmp[3];

 /* Run singularity test. */
 if (det == 0.0) {
  /* printf("invert_matrix: Warning: Singular matrix.\n"); */
  /* 	  memcpy(out,_identity,16*sizeof(double)); */
 } else {
  GLdouble invDet = 1.0 / det;
  /* Compute rest of inverse. */
  tmp[0] *= invDet;
  tmp[1] *= invDet;
  tmp[2] *= invDet;
  tmp[3] *= invDet;

  tmp[4] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
  tmp[5] = (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
  tmp[6] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
  tmp[7] = (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

  /* Pre-compute 2x2 dets for first two rows when computing */
  /* cofactors of last two rows. */
  d12 = m11 * m22 - m21 * m12;
  d13 = m11 * m23 - m21 * m13;
  d23 = m12 * m23 - m22 * m13;
  d24 = m12 * m24 - m22 * m14;
  d34 = m13 * m24 - m23 * m14;
  d41 = m14 * m21 - m24 * m11;

  tmp[8] = (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
  tmp[9] = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
  tmp[10] = (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
  tmp[11] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
  tmp[12] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
  tmp[13] = (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
  tmp[14] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
  tmp[15] = (m31 * d23 - m32 * d13 + m33 * d12) * invDet;

  memcpy(out, tmp, 16 * sizeof(GLdouble));
 }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT
}



void pos(double *px, double *py, double *pz, const int x, const int y,
  const int *viewport)
{
 /*
    Use the ortho projection and viewport information
    to map from mouse co-ordinates back into world 
    co-ordinates
  */

 *px = (double) (x - viewport[0]) / (double) (viewport[2]);
 *py = (double) (y - viewport[1]) / (double) (viewport[3]);

 *px = _left + (*px) * (_right - _left);
 *py = _top + (*py) * (_bottom - _top);
 *pz = _zNear;
}

void getMatrix()
{
 glGetDoublev(GL_MODELVIEW_MATRIX, _matrix);
 invertMatrix(_matrix, _matrixI);
}

void init()
{
 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 gluPerspective(fovy, (double)width / (double)height, _zNear, _zFar);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();

 //Light values and coordinates
 GLfloat ambientLight[] = { 0.3f, 0.3f, 0.3f, 1.0f };
 GLfloat diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f };
 GLfloat lightPos[] = {20.0f, 20.0f, 50.0f};
 glEnable(GL_DEPTH_TEST);
 glFrontFace(GL_CCW);
 //glEnable(GL_CULL_FACE);
 glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
 // Hidden surface removal // Counterclockwise polygons face out // Do not calculate inside of jet // Enable lighting
 glEnable(GL_LIGHTING);
 // Set up and enable light 0
 glLightfv(GL_LIGHT0,GL_AMBIENT,ambientLight);
 glLightfv(GL_LIGHT0,GL_DIFFUSE,diffuseLight);
 glEnable(GL_LIGHT0);
 // Enable color tracking
 glEnable(GL_COLOR_MATERIAL);
 // Set material properties to follow glColor values
 glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

 glClearColor(0.2f, 0.2f, 0.2f, 3.0f );

 //Rescale normals to unit length
 glEnable(GL_NORMALIZE);
 glLightfv(GL_LIGHT0,GL_POSITION,lightPos);

 glShadeModel(GL_FLAT);
 getMatrix(); //Init matrix

 //Translate camera
 glPushMatrix();
 glLoadIdentity();
 glTranslatef(0,0,-5.0);
 glMultMatrixd(_matrix);
 getMatrix();
 glPopMatrix();

}

void recordPose();
void playPose(char);

struct Pose
{
 vector<float> angles;
 vector<glm::mat4> Ts;
 vector<glm::fquat> qs;

 vector<Quaternion> as;

 int nPose=0;
};

static Pose p;

static float stepSize = 0.1;
bool noAnimate=true;
int interpolationType = 4;

void animate(int value)
{
 if(noAnimate  )
  return;

 static float t=0;
 static int pose=0;

 //do animation here
 //fi = f1 + t(f2-f1)

 glm::mat4 interpolated [17];

 for(auto i = 0; i != 17; ++i)
 {
  Joint &currJoint = myDefMesh.mySkeleton.joints[i+1];
  int add = 1 + pose * 18;

  if(interpolationType == 1)
  {
   /* glm::mat4 f1 = p.Ts[i + add]; */
   /* glm::mat4 f2 = p.Ts[i + add + 18] - f1; */
   /* interpolated[i] = f1 + t*f2; */

   glm::mat4 test= Quaternion::matrix(p.as[i + add]);
   test = test + t *(Quaternion::matrix(p.as[i + add + 18]) - test);
   interpolated[i] = test;
   currJoint.T = interpolated[i];
   currJoint.globalP = currJoint.T * glm::vec4(currJoint.position.x, currJoint.position.y, currJoint.position.z, 1.0);
  }
  else if(interpolationType == 2)
  {
   glm::mat4 test= Quaternion::eulerLerp(p.as[i + add], p.as[i + add + 18], t);
   interpolated[i] = test;
  }
  else if(interpolationType == 3)
  {
   /* glm::mat4 test=glm::mat4_cast(glm::lerp(p.qs[i + add], p.qs[i + add + 18], t)); */
   /* interpolated[i] = test; */

   glm::mat4 test=Quaternion::matrix(Quaternion::lerp(p.as[i + add], p.as[i + add + 18], t));
   interpolated[i] = test;
  }
  else if(interpolationType == 4)
  {
   glm::mat4 test= Quaternion::slerp(p.as[i + add], p.as[i + add + 18], t);
   interpolated[i] = test;
  }
 }

 glm::mat4 justfnow [17];
 if(interpolationType != 0)
 {
  //below is only for quat
  auto & access = myDefMesh.mySkeleton.joints;
  for(auto i = 0; i != 17; ++i)
  {
   Joint &currJoint = myDefMesh.mySkeleton.joints[i+1];
   glm::vec4 parentJ=access[access[i + 1].parent].globalP;

   glm::mat4 tParentPos = glm::translate(glm::mat4(1.f), glm::vec3(parentJ));
   glm::mat4 rot = interpolated[i];
   glm::mat4 tNegParentPos = glm::translate(glm::mat4(1.f), glm::vec3(-parentJ));

   int currentJointIndex = i;
   //bug here when loading from file
   //&& access[i + 1].angle != 0
   while(currentJointIndex != -2 )
   {
    justfnow[currentJointIndex]= tParentPos * rot * tNegParentPos * justfnow[currentJointIndex];
    currentJointIndex = access[currentJointIndex + 1].child - 1;
   }
   currJoint.T = justfnow[i];
   currJoint.globalP = currJoint.T * glm::vec4(currJoint.position.x, currJoint.position.y, currJoint.position.z, 1.0);
  }
 }

 for(auto i = 3; i != 3 * 6670; i+=3)
 {
  //for each vertex find its final poistion
  glm::vec4 fp;
  glm::vec4 iP=glm::vec4(myDefMesh.cpy[i], myDefMesh.cpy[i + 1], myDefMesh.cpy[i + 2], 1.0);
  for(auto j = 0; j != 17; ++j)
  {
   float cW = myDefMesh.weights[j + (i / 3 - 1) * 17];

   auto ppos =myDefMesh.mySkeleton.joints[myDefMesh.mySkeleton.joints[j + 1].parent].globalP;

   /* if(interpolationType == 1) */
   /*  fp += cW * interpolated[j] * iP; */ 

   if(interpolationType != 0)
    fp += cW * justfnow[j] * iP; 
  }
  myDefMesh.pmodel->vertices[i] = fp.x, myDefMesh.pmodel->vertices[i + 1] = fp.y, myDefMesh.pmodel->vertices[i + 2] = fp.z;
 }


 t+=stepSize;
 //interesting effect when t is not bounded
 if(t > 1)
 {
  t=0;
  ++pose;
 }

 if(pose == p.nPose - 1)
  pose=0;

 glutPostRedisplay();

 //call every tenth of a second
 glutTimerFunc(100, animate, 1);
}


void changeSize(int w, int h)
{
 glViewport(0, 0, w, h);

 _top = 1.0;
 _bottom = -1.0;
 _left = -(double) w / (double) h;
 _right = -_left;

 glMatrixMode(GL_PROJECTION);
 glLoadIdentity();
 /* glOrtho(_left,_right,_bottom,_top,_zNear,_zFar);  Ortho */
 gluPerspective(fovy, (double) w / (double) h, _zNear, _zFar);

 glMatrixMode(GL_MODELVIEW);
}

void timerFunction(int value)       
{
 glutTimerFunc(10,timerFunction,1);
 glutPostRedisplay();
}

void save()
{
 cout << "Choose file name you would like to save to: " << std::flush;
 string filename = "test";
 string extension = ".anim";
 cin >> filename;
 string finalFile = filename + extension;

 std::ofstream f;
 f.open(finalFile);
 const auto & quaternions = p.as;
 f << p.nPose << '\n';
 for(auto quat : quaternions)
 {
  f << quat.angle;
  f << ' ' << quat.axis.x;
  f << ' ' << quat.axis.y;
  f << ' ' << quat.axis.z;
  f << '\n';
 }

 f << 4002 << '\n';
 for(auto a : p.angles)
 {
  f << a << '\n';
 }

 f << 4002 << '\n';
 for(auto too: p.Ts)
 {
  f <<  too[0][0];
  f<< '\n';
  f <<  too[0][1];
  f<< '\n';
  f <<  too[0][2];
  f<< '\n';
  f <<  too[0][3];
  f<< '\n';

  f <<  too[1][0];
  f<< '\n';
  f <<  too[1][1] ;
  f<< '\n';
  f <<  too[1][2] ;
  f<< '\n';
  f <<  too[1][3] ;
  f<< '\n';

  f <<  too[2][0] ;
  f<< '\n';
  f <<  too[2][1] ;
  f<< '\n';
  f <<  too[2][2] ;
  f<< '\n';
  f <<  too[2][3] ;
  f<< '\n';

  f <<  too[3][0] ;
  f<< '\n';
  f <<  too[3][1] ;
  f<< '\n';
  f <<  too[3][2] ;
  f<< '\n';
  f <<  too[3][3];
  f<< '\n';
 }
 cout << "Done saving data to animation file: " << finalFile << endl;
}

void load()
{
 cout << "Choose file name you would like to load from: " << std::flush;
 string filename = "test";
 string extension = ".anim";
 cin >> filename;

 string finalFile = filename + extension;
 std::ifstream f(finalFile);
 p.as.clear();
 p.angles.clear();
 p.Ts.clear();

 f >> p.nPose;
 float val;

 while(f >> val)
 {
  if(val == 4002)
  {
   break;
  }

  float x, y, z;
  f >> x >> y >> z;

  Quaternion q;
  q.angle = val;
  q.axis = glm::vec3(x, y, z);
  p.as.push_back(q);
 }

 float finally;
 while(f >> finally)
 {
  if(finally == 4002)
   break;
  p.angles.push_back(finally);
 }

 float r;
 while(f >> r)
 {
  glm::mat4 good;
  for(auto i = 0; i != 4; ++i)
  {
   for(auto j = 0; j != 4; ++j)
   {
    good [i][j]=r;

    if(!(i == 3 && j == 3))
     f >> r;
   }
  }
  p.Ts.push_back(good);
 }

 cout << "Done loading animation file: " << finalFile << endl;
}

void handleKeyPress(unsigned char key, int x, int y)
{ 
 switch(key)
 {
  case 'm':
   meshModel = (meshModel+1)%3; break;
  case 't':
   cout << "recording pose" << endl;
   recordPose();
   break;
  case '=':
   playPose('=');
   break;
  case '-':
   playPose('-');
   break;
  case 'p':
   if(p.nPose < 2)
    return;
   noAnimate=!noAnimate;
   animate(1);
   break;
  case 'k':
   stepSize+=0.1;
   stepSize =min(1.f, stepSize);
   break;
  case 'j':
   stepSize-=0.1;
   stepSize =max(0.f, stepSize);
   break;
  case '1':
   cout << "You have chosen matrix linear interpolation" << endl;
   interpolationType = 1;
   break;
  case '2':
   cout << "You have chosen euler angle interpolation" << endl;
   interpolationType = 2;
   break;
  case '3':
   cout << "You have chosen quaternion linear interpolation" << endl;
   interpolationType = 3;
   break;
  case '4':
   cout << "You have chosen Slerp" << endl;
   interpolationType = 4;
   break;
  case 'w':
   save();
   break;
  case 'l':
   load();
   break;
  case 'q':
   exit(0);
 }
}



void playPose(char direction)
{
 static int poseNumber = -1;

 if(direction == '=')
 {
  if(poseNumber == p.nPose - 1)
   return;
  ++poseNumber;
 }
 else if(direction == '-')
 {
  if(poseNumber == -1)
   ++poseNumber;
  else if(poseNumber != 0)
   --poseNumber;
 }

 std::vector<Joint> &j=myDefMesh.mySkeleton.joints;

 for(auto i = 0; i != j.size(); ++i)
 {
  int index= i + poseNumber*18;
  j[i].angle=p.angles[index];
  j[i].T=p.Ts[index];
 }

 for(auto i = 3; i != 3 * 6670; i+=3)
 {
  //for each vertex find its final poistion
  glm::vec4 fp;
  glm::vec4 iP=glm::vec4(myDefMesh.cpy[i], myDefMesh.cpy[i + 1], myDefMesh.cpy[i + 2], 1.0);
  for(auto j = 0; j != 17; ++j)
  {
   /* cout << j + (i / 3 - 1) * 17 << endl; */
   float cW = myDefMesh.weights[j + (i / 3 - 1) * 17];
   /* cout << myDefMesh.weights.size() << endl; */

   fp += cW * myDefMesh.mySkeleton.joints[j + 1].T * iP; 
  }
  myDefMesh.pmodel->vertices[i] = fp.x, myDefMesh.pmodel->vertices[i + 1] = fp.y, myDefMesh.pmodel->vertices[i + 2] = fp.z;
 }
}

//Go through each joint a store its local angle and gloabl transformation
void recordPose()
{
 ++p.nPose;
 std::vector<Joint> &j=myDefMesh.mySkeleton.joints;
 for(auto i = 0; i != j.size(); ++i)
 {
  p.angles.push_back(j[i].angle);
  p.Ts.push_back(j[i].T);
  p.qs.push_back(j[i].rot);
  p.as.push_back(j[i].customQ);
 }

}

void mouseEvent(int button, int state, int x, int y)
{
 int viewport[4];

 _mouseX = x;
 _mouseY = y;

 /* printf("x: %d  y: %d\n", x, y); */
 if (state == GLUT_UP)
  switch (button) {
   case GLUT_LEFT_BUTTON:
    myDefMesh.mySkeleton.release();
    break;
   case GLUT_MIDDLE_BUTTON:
    _mouseMiddle = false;
    break;
   case GLUT_RIGHT_BUTTON:
    _mouseRight = false;
    break;
  } else
   switch (button) {
    case GLUT_LEFT_BUTTON:
     myDefMesh.mySkeleton.selectOrReleaseJoint();
     _mouseLeft = true;
     add=true;
     break;
    case GLUT_MIDDLE_BUTTON:
     _mouseMiddle = true;
     break;
    case GLUT_RIGHT_BUTTON:
     _mouseRight = true;
     break;
    case 4:         //Zoomout
     glLoadIdentity();
     glTranslatef(0,0,-0.1);
     glMultMatrixd(_matrix);
     getMatrix();
     glutPostRedisplay();
     break;
    case 3:         //Zoomin
     glLoadIdentity();
     glTranslatef(0,0,0.1);
     glMultMatrixd(_matrix);
     getMatrix();
     glutPostRedisplay();
     break;
    default:
     break;
   }

  glGetIntegerv(GL_VIEWPORT, viewport);
  pos(&_dragPosX, &_dragPosY, &_dragPosZ, x, y, viewport);
}

void mousePassiveFunc(int x, int y)
{
 myDefMesh.mySkeleton.checkHoveringStatus(x, y);
}

double amount=0;
double preAngle=0;
void mouseMoveEvent(int x, int y)
{
 if (!myDefMesh.mySkeleton.hasJointSelected)
 {
  bool changed = false;

  const int dx = x - _mouseX;
  const int dy = y - _mouseY;
  /* printf("x: %d  y: %d\n", dx, dy); */

  int viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);

  if (dx == 0 && dy == 0)
   return;

  if (_mouseMiddle || (_mouseLeft && _mouseRight)) {

   glLoadIdentity();
   glTranslatef(0, 0, dy * 0.01);
   glMultMatrixd(_matrix);

   changed = true;
  } else if (_mouseLeft) {
   double ax, ay, az;
   double bx, by, bz;
   double angle;

   ax = dy;
   ay = dx;
   az = 0.0;
   angle = vlen(ax, ay, az) / (double) (viewport[2] + 1) * 180.0;

   /* Use inverse matrix to determine local axis of rotation */

   bx = _matrixI[0] * ax + _matrixI[4] * ay + _matrixI[8] * az;
   by = _matrixI[1] * ax + _matrixI[5] * ay + _matrixI[9] * az;
   bz = _matrixI[2] * ax + _matrixI[6] * ay + _matrixI[10] * az;

   glRotatef(angle, bx, by, bz);

   changed = true;
  } else if (_mouseRight) {
   double px, py, pz;

   pos(&px, &py, &pz, x, y, viewport);

   glLoadIdentity();
   glTranslatef(px - _dragPosX, py - _dragPosY, pz - _dragPosZ);
   glMultMatrixd(_matrix);

   _dragPosX = px;
   _dragPosY = py;
   _dragPosZ = pz;

   changed = true;
  }

  _mouseX = x;
  _mouseY = y;

  if (changed) {
   getMatrix();
   glutPostRedisplay();
  }
 }
 else    
 {
  //position is with bottom left being 0,0
  Skeleton &s=myDefMesh.mySkeleton;
  Joint &selectedJoint=s.joints[s.selectedJoint];

  //never change position of the parent.
  if(selectedJoint.parent == -1)
  {
   return;
  }
  else
  {
   Joint &parentJoint=s.joints[selectedJoint.parent];

   Vec3 &parentPosition = parentJoint.position;
   Vec3 &childPosition  = selectedJoint.position;
   Vec3 difference      = childPosition-parentPosition;

   Vec2 parentPostionVector(parentJoint.screenCoord.x, parentJoint.screenCoord.y);
   Vec2 initMPos(_mouseX, _mouseY);
   Vec2 currMPos(x, y);

   Vec2 diff=initMPos-parentPostionVector;
   Vec2 diff2=currMPos-parentPostionVector;

   diff.y = -diff.y;
   diff2.y = -diff2.y;


   double what= (diff.x*diff2.y) - (diff.y*diff2.x);
   double dot=dot2(diff, diff2);
   double mag1=mag(diff);
   double mag2=mag(diff2);
   double tMag=mag1*mag2;


   //added to original
   double angle=acos(dot/tMag);

   if(add)
   {
    //original angle
    amount=selectedJoint.angle;
    preAngle=0;
    add=false;
   }

   int orientation=what >0;
   if(orientation)
   {
    selectedJoint.angle=angle + amount;
   }
   else
   {
    selectedJoint.angle=-angle + amount;
   }

   for(auto i = 3; i != 3 * 6670; i+=3)
   {
    //for each vertex find its final poistion
    glm::vec4 fp;
    /* glm::vec4 iP=glm::vec4(myDefMesh.cpy[i], myDefMesh.cpy[i + 1], myDefMesh.cpy[i + 2], 1.0); */
    glm::vec4 iP=glm::vec4(myDefMesh.cpy[i], myDefMesh.cpy[i + 1], myDefMesh.cpy[i + 2], 1.0);
    for(auto j = 0; j != 17; ++j)
    {
     float cW = myDefMesh.weights[j + (i / 3 - 1) * 17];

     fp += cW * s.joints[j + 1].T * iP; 
     /* fp += cW * (s.joints[s.joints[j+1].parent].globalP + s.joints[j + 1].rot * (iP - s.joints[s.joints[j+1].parent].globalP)); */ 
     /* fp += cW *  s.joints[j + 1].rot * iP; */ 

    }
    myDefMesh.pmodel->vertices[i] = fp.x, myDefMesh.pmodel->vertices[i + 1] = fp.y, myDefMesh.pmodel->vertices[i + 2] = fp.z;
   }

   preAngle = angle * -((orientation - (3 * orientation)) + 1)  ;
  }
 }
}
void display()
{
 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 glMatrixMode(GL_MODELVIEW);
 glLoadIdentity();
 glMultMatrixd(_matrix);

 glColor3f(0.5,0.5,0.5);
 glPushMatrix();													//draw terrain
 glColor3f(0.7,0.7,0.7);
 glNormal3f(0.0, 1.0, 0.0);
 glBegin(GL_QUADS);
 glVertex3f(-3,-0.85,3);
 glVertex3f(3,-0.85,3);
 glVertex3f(3,-0.85,-3);
 glVertex3f(-3,-0.85,-3);
 glEnd();
 glPopMatrix();

 glPushMatrix();
 myDefMesh.glDraw(meshModel);

 glPopMatrix();

 glutSwapBuffers();
}

int main(int argc, char **argv)
{

 glutInit(&argc, argv);

 glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);	//double buffer
 glutInitWindowSize(width, height);
 glutInitWindowPosition(0, 0);
 glutCreateWindow("COMP477");
 glutDisplayFunc(display);
 glutReshapeFunc(changeSize);
 glutTimerFunc(10, timerFunction, 1);

 glutMouseFunc(mouseEvent);
 glutMotionFunc(mouseMoveEvent);
 glutKeyboardFunc(handleKeyPress);
 glutPassiveMotionFunc(mousePassiveFunc);


 init();
 glutMainLoop();

 return 0;
}
