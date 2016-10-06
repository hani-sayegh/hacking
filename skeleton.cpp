#include "skeleton.h"
#include "splitstring.h"
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

/*
 * Load skeleton file
 */
void Skeleton::loadSkeleton(std::string skelFileName)
{
 std::string strBone;
 std::ifstream skelFile(skelFileName.c_str());
 if (skelFile.is_open())
 {
  while ( std::getline(skelFile, strBone)) { //Read a line to build a bone
   std::vector<std::string> boneParams;
   splitstring splitStr(strBone);
   boneParams = splitStr.split(' ');
   Joint temp;
   temp.position.x = std::atof(boneParams[1].c_str());
   temp.position.y = std::atof(boneParams[2].c_str());
   temp.position.z = std::atof(boneParams[3].c_str());
   temp.globalP=glm::vec4(temp.position.x, temp.position.y, temp.position.z, 1.0);

   temp.parent=std::atoi(boneParams[4].c_str());
   if(temp.parent != -1)
   {
    joints[temp.parent].child=std::atoi(boneParams[0].c_str());
   }

   if (std::atoi(boneParams[0].c_str()) != joints.size())
   {
    std::cout<<"[Warning!!!] Bone index does not match\n";
   }
   joints.push_back(temp);
  }
 }
}

/*
 * Load Animation
 */
void Skeleton::loadAnimation(std::string skelFileName)
{
}


glm::vec3 axis=glm::vec3(0, 0, 1);

/*
 * Draw skeleton with OpenGL
 */
//this is for the joints
//function is called how often?
void Skeleton::glDrawSkeleton()
{
 //reset coordinates
 for(auto i = 0; i != joints.size(); ++i)
 {
  joints[i].globalP = glm::vec4(joints[i].position.x, joints[i].position.y, joints[i].position.z, 1.0);
  /* joints[i].globalAngle = 0; */
  joints[i].T = glm::mat4(1.f);
 }

 //Rigging skeleton
 glDisable(GL_DEPTH_TEST);

 glPushMatrix();
 //why
 glTranslatef(-0.9,-0.9,-0.9);
 //why
 glScalef(1.8,1.8,1.8);
 //why
 glPointSize(1);
 //why
 glColor3f(1,0,0);


 for (unsigned i=0; i< joints.size(); i++)
 {
  Vec3 &j=joints[i].position;
  glm::vec4 parentJ=joints[0].globalP;


  if(joints[i].parent != -1)
  {
   parentJ=joints[joints[i].parent].globalP;
  }


  double angle=joints[i].angle;


  Joint currJoint=joints[i];
  Joint parJoint=joints[currJoint.parent];

  //default value for final position of joint after all transformations

  int temp = i;
  while(joints[temp].angle != 0)
  {

   glm::vec3 diff;

   diff=glm::vec3(currJoint.globalP) - glm::vec3(parentJ);

   glm::mat4 tran = glm::translate(glm::mat4(1.f), diff);
   /* glm::mat4 rot  = glm::rotate(glm::mat4(1.f), float(angle), axis); */
   glm::mat4 rot  = glm::rotate(glm::mat4(1.f), float(angle), glm::vec3(0, 0, 1));
   glm::mat4 tran2= glm::translate(glm::mat4(1.f), glm::vec3(parentJ));
   glm::vec4 finalMult =tran2 * rot * tran * glm::vec4(0.0, 0.0, 0.0, 1.0); 



   glm::mat4 tran3 = glm::translate(glm::mat4(1.f), glm::vec3(parentJ));
   glm::mat4 tran4 = glm::translate(glm::mat4(1.f), glm::vec3(-parentJ));
   //cannot just do -tran3, since that is different that translate with -parentJ
   joints[i].T = tran3 * rot * tran4 * joints[i].T;



   joints[i].globalP=finalMult;

   i = currJoint.child;

   if(i == -1)
   {
    i = temp;
    currJoint=joints[i];
    break;
   }

   currJoint=joints[i];
  }

  glm::vec4  go=(joints[0].globalP);
  if(joints[i].parent != -1)
  {
   go=(joints[joints[i].parent].globalP);
  }

  glm::vec4 fp = currJoint.globalP;

  /* glm::vec4 & fp=currJoint.globalP; */

  Vec3 pos=Vec3(fp.x, fp.y, fp.z);
  Vec3 posP=Vec3(go.x, go.y, go.z);

  glColor3f(1, 0,0);
  glBegin(GL_LINES);
  vertex(pos);
  vertex(posP);
  glEnd();

  if (joints[i].isPicked)
   glColor3f(1.0, 0.0, 0.0);
  else if (joints[i].isHovered)
   glColor3f(0.0, 0.0, 1.0);
  else
   glColor3f(0.0, 1.0, 0.0);

  glTranslated(fp.x, fp.y, fp.z);

  glutSolidSphere(0.015, 15, 15);

  glTranslated(-fp.x, -fp.y, -fp.z);

 }
 //has to be here ...why?
 updateScreenCoord();
 glPopMatrix();
 glEnable(GL_DEPTH_TEST);
}

void Skeleton::updateScreenCoord()
{
 GLint viewport[4];
 GLdouble modelview[16];
 GLdouble projection[16];
 GLdouble winX, winY, winZ;

 glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
 glGetDoublev( GL_PROJECTION_MATRIX, projection );
 glGetIntegerv( GL_VIEWPORT, viewport );

 gluUnProject(0.2, 0.2, -0.2,
   modelview, projection, viewport,
   &winX, &winY, &winZ );
 glm::vec3 now(winX, winY, winZ);
/* cout << winX << " " << winY << " " << winZ << endl; */

 gluUnProject(0.5, 0.5, -0.5,
   modelview, projection, viewport,
   &winX, &winY, &winZ );
 glm::vec3 now2(winX, winY, winZ);

 gluUnProject(0.7, 0.7, -0.7,
   modelview, projection, viewport,
   &winX, &winY, &winZ );
 glm::vec3 now3(winX, winY, winZ);
 now2 = now2 - now;
 now3 = now3 - now;

 glm::vec3 finish= glm::cross(now2, now3);
 axis.x = finish.x; 
 axis.y = finish.y; 
 axis.z = finish.z; 

 for (unsigned i=0; i<joints.size(); i++)
 {
  gluProject((GLdouble)joints[i].globalP.x, (GLdouble)joints[i].globalP.y, (GLdouble)joints[i].globalP.z,
    modelview, projection, viewport,
    &winX, &winY, &winZ );
  joints[i].screenCoord.x = winX;
  joints[i].screenCoord.y = (double)glutGet(GLUT_WINDOW_HEIGHT)-winY;
 }
}
void Skeleton::checkHoveringStatus(int x, int y)
{
 double distance = 0.0f;
 double minDistance = 1000.0f;
 int hoveredJoint = -1;
 for(unsigned i=0; i < joints.size(); i++)
 {
  joints[i].isHovered = false;
  distance = sqrt((x - joints[i].screenCoord.x)*(x - joints[i].screenCoord.x) 
    + (y - joints[i].screenCoord.y)*(y - joints[i].screenCoord.y));
  if (distance > 50) continue;
  if (distance < minDistance)
  {
   hoveredJoint = i;
   minDistance = distance;
  }
 }
 if (hoveredJoint != -1) joints[hoveredJoint].isHovered = true;
}

void Skeleton::release()
{
 hasJointSelected = false;
 for (unsigned i=0; i<joints.size(); i++)
 {
  joints[i].isPicked = false;
 }
}



std::ostream& operator<<(std::ostream& os, const Joint& j)
{
 Vec3 pos=j.position;
 os << pos.x << ' ' << pos.y << ' ' << pos.z;
 return os;
}


void Skeleton::selectOrReleaseJoint()
{
 bool hasHovered=false;
 for (unsigned i=0; i<joints.size(); i++)
 {
  joints[i].isPicked = false;
  if (joints[i].isHovered)
  {
   hasHovered = true;
   joints[i].isPicked = true;
   hasJointSelected = true;
   selectedJoint=i;
  }
 }
 if (!hasHovered)    //Release joint
  hasJointSelected = false;
}

