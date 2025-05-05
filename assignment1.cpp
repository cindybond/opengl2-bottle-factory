// COSC363 Assignment 1 
// Glass Bottle Factory
// Author Cindy Bond cbo680

#include <GL/freeglut.h>
#include <iostream>
#include <fstream>
#include <climits>
#include "loadTGA.h"
#include "loadBMP.h"
#include <vector>
#include <cmath>
#include <list>
using namespace std;

// ============================== GLOBALS ======================================= // 
#define GL_CLAMP_TO_EDGE 0x812F
#define M_PI 3.14159265358979323846

GLuint txId[10];
GLUquadricObj* q;
float* x, * y, * z;					//vertex coordinates
int* nv, * t1, * t2, * t3, * t4;		//number of vertices and vertex indices of each face
int nvert, nface;					//total number of vertices and faces
float angle = -10.0;
float cam_z = 70.0;
bool wireframe_mode = false;
float s, t = 0.0;
float texture_s, texture_t = 0.0;
float speed = 0.8;
float theta = 0.0;
float cutter_direction = 1.0;
float mold_direction = 1.0;
float spv_direction = 1.0;
float mold_z = 12.0;
float gob_y = 39.0;
float gob_length = 2.0;
float cut_gob_y = 33.0;
float spv_z = 30.0;
bool cut = false;
bool bottle_created = false;
float r = 5.0;							//radius of circle for fan casing
float vx[18], vy[18], bx[18], by[18]; // vertices for fan and blade
float blade_rotate = 1.0;
int tick = 0;


struct particle {	//A particle 
	int t;			//Life time  (0 - 200)
	float col;		//Color  (0 - 1)
	float size;		//Size   (5 - 25)
	float pos[3];	//Position
	float vel[3];	//Velocity
};

list<particle> parList;	//List of particles
vector<float> bottles = { -26.0, -13.0, -2.0, 9., 20., 31. };

//-- Loads mesh data in OFF format    --------------------------------------
void loadMeshFile(const char* fname) {
	ifstream fp_in;
	int nedge;

	fp_in.open(fname, ios::in);
	if (!fp_in.is_open()) {
		cout << "Error opening mesh file" << endl;
		exit(1);
	}

	fp_in.ignore(INT_MAX, '\n');		//ignore first line
	fp_in >> nvert >> nface >> nedge;	//read number of vertices, polygons, edges (not used)

	x = new float[nvert];	//create arrays
	y = new float[nvert];
	z = new float[nvert];

	nv = new int[nface];
	t1 = new int[nface];
	t2 = new int[nface];
	t3 = new int[nface];
	t4 = new int[nface];

	for (int i = 0; i < nvert; i++) {	//read vertex list 
		fp_in >> x[i] >> y[i] >> z[i];
	}
	for (int i = 0; i < nface; i++) {	//read polygon list 
		fp_in >> nv[i] >> t1[i] >> t2[i] >> t3[i];
		if (nv[i] == 4) {
			fp_in >> t4[i];
		}
		else {
			t4[i] = -1;
		}

		for( int j = 0; j < 3; ++j) { // ignoring extra rgb values in the OFF
			int rgb;
			fp_in >> rgb; 
		}
		fp_in.ignore(INT_MAX, '\n');
	}

	fp_in.close();
	cout << " File successfully read." << endl;
}

//-- Function to compute the normal vector of a triangle with index indx ---
void normal(int indx) {
	float x1 = x[t1[indx]], x2 = x[t2[indx]], x3 = x[t3[indx]];
	float y1 = y[t1[indx]], y2 = y[t2[indx]], y3 = y[t3[indx]];
	float z1 = z[t1[indx]], z2 = z[t2[indx]], z3 = z[t3[indx]];
	float nx, ny, nz;
	nx = y1 * (z2 - z3) + y2 * (z3 - z1) + y3 * (z1 - z2);
	ny = z1 * (x2 - x3) + z2 * (x3 - x1) + z3 * (x1 - x2);
	nz = x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2);
	glNormal3f(nx, ny, nz);
}

// load sky box
void loadTexture() {
	glGenTextures(10, txId);

	glBindTexture(GL_TEXTURE_2D, txId[0]);
	loadTGA("../textures/left.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[1]);
	loadTGA("../textures/front.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[2]);
	loadTGA("../textures/right.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[3]);
	loadTGA("../textures/back.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[4]);
	loadTGA("../textures/up.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[5]);
	loadTGA("../textures/down.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[6]);
	loadTGA("../textures/belt.tga");
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[7]);
	loadTGA("../textures/supervisor.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[8]);
	loadTGA("../textures/fan.tga");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[9]);
	loadBMP("../textures/Glow.bmp");

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}

//-- Draw a floor plane ----------------------------------------------------
void drawFloor() {
	glBindTexture(GL_TEXTURE_2D, txId[5]);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-100, 0, 100);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-100, 0, -100);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(100, 0, -100);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(100, 0, 100);
	glEnd();
}

void drawWalls() {
	glBindTexture(GL_TEXTURE_2D, txId[0]);
	glBegin(GL_QUADS);
		////////////////////// LEFT WALL ///////////////////////
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-100, 0, 100);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-100, 200, 100);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-100, 200, -100);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-100, 0, -100);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, txId[1]);
	glBegin(GL_QUADS);
		////////////////////// FRONT WALL ///////////////////////
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-100, 0, -100);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-100, 200, -100);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(100, 200, -100);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(100, 0, -100);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, txId[2]);
	glBegin(GL_QUADS);
		////////////////////// RIGHT WALL ///////////////////////
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(100, 0, -100);
	
		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(100, 200, -100);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(100, 200, 100);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(100, 0, 100);

		
	glEnd();

	glBindTexture(GL_TEXTURE_2D, txId[3]);
	glBegin(GL_QUADS);
		////////////////////// BACK WALL ///////////////////////
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(100, 0, 100);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(100, 200, 100);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(-100, 200, 100);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(-100, 0, 100);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, txId[4]);
	glBegin(GL_QUADS);
		////////////////////// UP WALL ///////////////////////
		glTexCoord2f(0.0f, 1.0f);
		glVertex3f(-100, 200, -100);

		glTexCoord2f(0.0f, 0.0f);
		glVertex3f(-100, 200, 100);

		glTexCoord2f(1.0f, 0.0f);
		glVertex3f(100, 200, 100);

		glTexCoord2f(1.0f, 1.0f);
		glVertex3f(100, 200, -100);
	glEnd();
}


//================================ SCENE MODELS ================================//
// Draw Conveyor Sides
void drawConveyor() {

	// Conveyor side #1
	glPushMatrix();
		glColor3f(0.3, 0.3, 0.3);
		glTranslatef(-6.0, 5.0, 7.0);
		glScalef(100.0, 8.0, 1.0);
		glutSolidCube(1);
	glPopMatrix();

	// Conveyor side #2
	glPushMatrix();
		glColor3f(0.3, 0.3, 0.3);
		glTranslatef(-6.0, 5.0, -7.0);
		glScalef(100.0, 8.0, 1.0);
		glutSolidCube(1);
	glPopMatrix();

}

// Conveyor top
void drawConveyorTop() {
	glBindTexture(GL_TEXTURE_2D, txId[6]);
	s += 0.006;
	t += 0.006;
	glPushMatrix();
		glColor3f(0.5, 0.5, 0.5);
		glTranslatef(-56.0, 9.0, -7.0);
		glScalef(100.0, 1.0, 14.0);
		glBegin(GL_QUADS);
			glTexCoord2f(s, 4. + t);
			glVertex3f(0., 0., 0.);
			glTexCoord2f(s, t);
			glVertex3f(1., 0., 0.);
			glTexCoord2f(4. + t, s);
			glVertex3f(1., 0., 1.);
			glTexCoord2f(4. + s, 4. + t);
			glVertex3f(0., 0., 1.);
		glEnd();
	glPopMatrix();


}

// Draw Cutting Machine
void drawCutMachine() {
	// Machine Arms
	
	glPushMatrix();
		glColor3f(0., 0.5, 0.);
		glTranslatef(-36.0, 25.0, -20.0);
		glScalef(5.0, 50.0, 5.0);
		glutSolidCube(1);
	glPopMatrix();

	// Cutter Knife
	glPushMatrix();
		glTranslatef(-36.0, 38.0, -15.0);
		glRotatef(-theta, 0., 1., 0.);
		glTranslatef(36.0, -38.0, 15.0);
		glTranslatef(-36.0, 38.0, -5.0);
		glScalef(0.7, 1.0, 25.0);
		glutSolidCube(1);
	glPopMatrix();
}

// Draw Molding Machine
void drawMoldMachine() {
	// Molding Machine Arm #1
	glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		glTranslatef(-34.0, 7.0, 15.0);
		glScalef(8.0, 30.0, 5.0);
		glutSolidCube(1);
	glPopMatrix();

	// Molding Machine Arm #2
	glPushMatrix();
		glColor3f(0.2, 0.2, 0.2);
		glTranslatef(-34.0, 7.0, -15.0);
		glScalef(8.0, 30.0, 5.0);
		glutSolidCube(1);
	glPopMatrix();

	// Molding Machine Press #1
	glPushMatrix();
	glColor3f(0.3, 0.3, 0.3);
	glTranslatef(-34.0, 15.0, mold_z);
	glScalef(6.0, 11.0, 2.0);
	glutSolidCube(1);
	glPopMatrix();

	// Molding Machine Press #2
	glPushMatrix();
		glColor3f(0.3, 0.3, 0.3);
		glTranslatef(-34.0, 15.0, -mold_z);
		glScalef(6.0, 11.0, 2.0);
		glutSolidCube(1);
	glPopMatrix();
}
// Draw Furnace
void drawFurnace() {
	// Furnace Foundation
	glColor3f(0.2, 0.2, 0.2);
	glPushMatrix();
		glTranslatef(-65.0, 30.0, 0.0);
		glScalef(17.0, 60.0, 14.0);
		glutSolidCube(1);
	glPopMatrix();

	// Furnace Door
	glColor3f(0.3, 0.3, 0.3);
	glPushMatrix();
		glTranslatef(-55.0, 52.0, 0.0);
		glScalef(2.0, 10.0, 10.0);
		glutSolidCube(1);
	glPopMatrix();

	// Furnace Output Pipe
	glPushMatrix();
		glColor3f(0.0, 1.0, 1.0);
		glTranslatef(-52.0, 40.0, 0.0);
		glScalef(25.0, 1.0, 2.0);
		glutSolidCube(1);
	glPopMatrix();

	// Furnace Output Sphere
	glPushMatrix();
		glTranslatef(-35.0, 40.0, 0.0);
		glRotatef(90.0, 1.0, 0.0, 0.0);
		glScalef(1.0, 1.0, 0.3);
		glutSolidTorus(3, 3, 10, 20.0);
	glPopMatrix();

	// Furnace chimney
	glPushMatrix();
		glColor3f(0.5, 0.5, 0.5);
		glTranslatef(-65.0, 65.0, 0.0);
		glScalef(2.0, 8.0, 2.0);
		glutSolidCube(1);
	glPopMatrix();
}

// Draw bottle 
void drawBottle() {
	glColor3f(0.55, 0.8, 0.78);

	//Construct the object model here using triangles read from OFF file
	glBegin(GL_TRIANGLES);
	for (int tindx = 0; tindx < nface; tindx++) {
		normal(tindx);
		glVertex3d(x[t1[tindx]], y[t1[tindx]], z[t1[tindx]]);
		glVertex3d(x[t2[tindx]], y[t2[tindx]], z[t2[tindx]]);
		glVertex3d(x[t3[tindx]], y[t3[tindx]], z[t3[tindx]]);
	}
	glEnd();
}

// Draw gob
void drawGob() {
	glPushMatrix();
		glColor3f(0.85, 0.33, 0.05);
		float emission[4] = { 1.0, 0.4, 0.4, 0.0};
		glMaterialfv(GL_FRONT, GL_EMISSION, emission);

		glTranslatef(-35.0, gob_y, 0.0);
		glScalef(2.0, gob_length, 2.0);
		glutSolidCube(1);

		//so emission does not impact everything on screen
		float no_emission[4] = { 0., 0., 0., 0.0};
		glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
	glPopMatrix();
}

// Draw Cut Gob
void drawCutGob() {
	glPushMatrix();
		glColor3f(0.85, 0.33, 0.05);
		float emission[4] = { 1.0, 0.4, 0.4, 0.0 };
		glMaterialfv(GL_FRONT, GL_EMISSION, emission);

		glTranslatef(-35.0, cut_gob_y, 0.0);
		glScalef(2.0, 10.0, 2.0);
		glutSolidCube(1);

		//so emission does not impact everything on screen
		float no_emission[4] = { 0., 0., 0., 0.0 };
		glMaterialfv(GL_FRONT, GL_EMISSION, no_emission);
	glPopMatrix();
}

// Draw Factory Supervisor
void drawSupervisor() {
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, txId[7]);
		glTranslatef(30.0, 30.0, spv_z);
		gluSphere(q, 2.0, 36, 17);
	glPopMatrix();
}

// Calculation for the vertices for the fan casing which is a circle
void makeCircle() {
	for (int i = 0; i < 18; i++) {
		float theta = (2.0 * M_PI * i) / 18;
		vx[i] = r * cos(theta);
		vy[i] = r * sin(theta);
	}
}

// Draw Fan Casing 
void drawFanCasing() {
	// Fan casing 
	glPushMatrix();
		glTranslatef(5.0, 25.0, -24.0);
		glScalef(3.0, 3.0, 1.0);
		glBegin(GL_QUAD_STRIP);
			for (int i = 0; i < 18; i++) {
				glVertex3f(vx[i], vy[i], 0);
				glVertex3f(vx[i], vy[i], 7);
			}
			glVertex3f(vx[0], vy[0], 0);
			glVertex3f(vx[0], vy[0], 7);
		glEnd();
	glPopMatrix();
}

void drawFan() {
	// Fan center
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, txId[8]);
		glTranslatef(5.0, 25.0, -20.0);
		gluSphere(q, 2.0, 36, 17);
	glPopMatrix();
	
	// Fan blades
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, txId[8]);
		glTranslatef(5.0, 25.0, -20.0);
		glRotatef(blade_rotate, 0.0, 0.0, 1.0);
		// Outer for loop for drawing four blades
		for (int blade_num = 0; blade_num < 4; blade_num++) {
			glPushMatrix();
				//Rotate each blade by 90 degrees by the z axis
				glRotatef(90.0 * blade_num, 0.0, 0.0, 1.0);
				
				// Inner for loop for drawing each blade shaped like almond eye
				glBegin(GL_QUAD_STRIP);
					for (int i = 0; i <= 12; i++) {
						float t = (float)i / 12;
						float x = 12 * t;
						float y = 5 * sin(t * M_PI) / 8;

						glTexCoord2f(t, 0.0);
						glVertex3f(x, y, 0);

						glTexCoord2f(t, 1.0);
						glVertex3f(x, -y, 0);
					}
				glEnd();

			glPopMatrix();
		}
	glPopMatrix();
}

//================================= PARTICLES ==============================//
//-- Draws a single particle as two texture mapped quads -------------------
void drawParticle(float col, float size, float px, float py, float pz) {
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glColor3f(col, col, col);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, txId[9]);

	glPushMatrix();
	glTranslatef(px, py, pz);
	glScalef(size, size, size);

	glBegin(GL_QUADS);
	//A quad on the xy-plane
	glTexCoord2f(0, 0);
	glVertex3f(-0.5, -0.5, 0);
	glTexCoord2f(1, 0);
	glVertex3f(0.5, -0.5, 0);
	glTexCoord2f(1, 1);
	glVertex3f(0.5, 0.5, 0);
	glTexCoord2f(0, 1);
	glVertex3f(-0.5, 0.5, 0);

	//A quad on the yz-plane
	glTexCoord2f(0, 0);
	glVertex3f(0, -0.5, -0.5);
	glTexCoord2f(1, 0);
	glVertex3f(0, 0.5, -0.5);
	glTexCoord2f(1, 1);
	glVertex3f(0, 0.5, 0.5);
	glTexCoord2f(0, 1);
	glVertex3f(0, -0.5, 0.5);
	glEnd();
	glPopMatrix();

	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
}
//-- Creates a new particle and initializes its data fields ----------------
void newParticle() {
	particle p = { 0 };

	p.pos[0] = -65.0;
	p.pos[1] = 70.0;	//This point is at the top end of the smoke stack
	p.pos[2] = 0;

	p.vel[0] = 0.06 * (rand() / (float)RAND_MAX);
	p.vel[1] = 0.3;
	p.vel[2] = 0.06 * (rand() / (float)RAND_MAX);

	p.col = 1;
	p.size = 5;

	parList.push_back(p);
}

//-- Updates the particle queue --------------------------------------------
void updateQueue() {
	const int LIFETIME = 200;
	list<particle>::iterator it;
	particle p;
	int tval;
	float delta;

	//Remove particles that have passed lifetime
	if (!parList.empty()) {
		p = parList.front();
		if (p.t > LIFETIME) parList.pop_front();
	}

	for (it = parList.begin(); it != parList.end(); it++) {
		tval = it->t;
		it->t = tval + 1;
		delta = (float)tval / (float)LIFETIME;

		for (int i = 0; i < 3; i++)	(it->pos[i]) += it->vel[i];

		it->vel[0] = delta * 0.4; // wind effect
		it->size = delta * 20 + 5;	// (5 - 25)
		it->col = 1 - delta;		// (1 - 0)
	}

	if (tick % 2 == 0) newParticle();   //Create a new particle every sec.
}



void initialize() {
	float white[4] = { 1., 1., 1., 1. };
	float offWhite[4] = { 1., 0.8, 0.8, 1. };
	float shadow[4] = { 0.3, 0.3, 0.3, 1. };

	GLfloat shininess = 50.0f;

	loadTexture();
	loadMeshFile("../bottle.off");
	
	makeCircle();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 
	glEnable(GL_COLOR_MATERIAL);
	

	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
	

	glLightfv(GL_LIGHT0, GL_DIFFUSE, offWhite);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white);	
	glLightfv(GL_LIGHT0, GL_AMBIENT, shadow);

	glLightfv(GL_LIGHT1, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT1, GL_SPECULAR, white);

	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 30.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);

	q = gluNewQuadric();
	gluQuadricDrawStyle(q, GLU_FILL);
	gluQuadricTexture(q, GL_TRUE);


	glClearColor(0., 0., 0., 0.);
	glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(90.0, 1.0, 5.0, 1000.0);
}

void display() {
	float light[4] = { 80.0, 50.0, 80.0, 1.0 };
	float shadowMat[16] = { light[1], 0, 0, 0, -light[0], 0, -light[2], -1,
							0, 0, light[1], 0, 0, 0, 0, light[1] };
	float spotDir[] = { 0.0, -1.0, 0.0 };
	float spotPosn[] = { 30.0, 30.0, spv_z, 1.0 };

	if (wireframe_mode) {
		glDisable(GL_LIGHTING);
		glDisable(GL_COLOR_MATERIAL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else {
		glEnable(GL_LIGHTING);
		glEnable(GL_COLOR_MATERIAL);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	
	}
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	

	gluLookAt(20.0, 20.0, cam_z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
	glLightfv(GL_LIGHT0, GL_POSITION, light);	
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
	glLightfv(GL_LIGHT1, GL_POSITION, spotPosn);
	
	glRotatef(angle, 0.0, 1.0, 0.0); // Rotate camera angle

	glEnable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	drawFloor();
	drawWalls();

	glEnable(GL_LIGHTING);
	
	
	
	//drawSupervisor()
	glPushMatrix();
		drawSupervisor();
		glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, spotDir);
		glLightfv(GL_LIGHT1, GL_POSITION, spotPosn);
	glPopMatrix();

	drawConveyorTop();
	drawFan();
	
	glDisable(GL_LIGHTING);

	//Cutter machine's shadow
	glColor3f(0.2, 0.2, 0.2);
	glPushMatrix();
		glMultMatrixf(shadowMat);
		glTranslatef(-36.0, 25.0, -20.0);
		glScalef(5.0, 50.0, 5.0);
		glutSolidCube(1);
	glPopMatrix();

	//Furnace's shadow
	glColor3f(0.2, 0.2, 0.2);
	glPushMatrix();
		glMultMatrixf(shadowMat);
		glTranslatef(-65.0, 30.0, 0.0);
		glScalef(17.0, 60.0, 14.0);
		glutSolidCube(1);
	glPopMatrix();
	
	glEnable(GL_LIGHTING);
	
	list<particle>::iterator it;
	for (it = parList.begin(); it != parList.end(); it++) {
		drawParticle(it->col, it->size, it->pos[0], it->pos[1], it->pos[2]);
	}

	glDisable(GL_TEXTURE_2D);

	drawConveyor();
	drawCutMachine();
	drawMoldMachine();
	drawFurnace();
	drawFanCasing();

	glDisable(GL_LIGHT0);
	drawGob();
	drawCutGob();
	glEnable(GL_LIGHT0);

	

	//drawBottle()
	for (int i = 0; i < bottles.size(); i++) {
		glPushMatrix();
		glTranslatef(bottles[i], 14.0, 0.0);
		glScalef(30.0, 30.0, 30.0);
		drawBottle();
		glPopMatrix();
	}

	glColor3f(1, 1, 1);

	glutSwapBuffers();
}

//=============================== KEYBOARD EVENTS =====================================//
void special(int key, int x, int y) {
	if (key == GLUT_KEY_LEFT) 	angle++;
	else if (key == GLUT_KEY_RIGHT) angle--;
	else if (key == GLUT_KEY_UP) 	cam_z--;
	else if (key == GLUT_KEY_DOWN) 	cam_z++;
	else if (key == GLUT_KEY_F1 && !wireframe_mode)	wireframe_mode = true;
	else if (key == GLUT_KEY_F1 && wireframe_mode)	wireframe_mode = false;
	if (cam_z >= 100.) cam_z = 100.;
	else if (cam_z < 10.)  cam_z = 10.;

	glutPostRedisplay();
}

//================================== TIMER ============================================//
void timer(int value) {
	// Cutting machine knife movement 45 degree angle
	theta += (speed * 1.5) * cutter_direction;

	if (theta >= 45.0) {
		theta = 45.0;
		cutter_direction = -1.0;
		cut = false;
	}
	else if (theta <= -45.0) {
		theta = -45.0;
		cutter_direction = 1.0;
		cut = false;
	}
	else if (theta <= 3.0 && theta >= -3.0) {
		if (!cut) {
			cut = true;
			cut_gob_y = 33.0;
			gob_length = 2.0;
			gob_y = 39.0;
		}
	}
	
	// Cut Gob falling movement
	if (cut) {
		if (cut_gob_y >= 15.0) {
			cut_gob_y--;
		} else {
			cut = false;
		}
	
	}
	else {
		if (gob_length <= 11.0) {
			gob_length += speed;
		}

		if (gob_y >= 34.0) {
			gob_y -= (speed * 0.5);
		}
	}

	// Bottle movement
	for (int i = 0; i < bottles.size(); i++) {
		bottles[i] += speed * 0.15;
		if (bottles[i] >= 43.0) {
			bottles.erase(bottles.begin() + i);
			i--;
		}
	}
	
	// Molding machine press movement 
	if (mold_z <= 0.0) {
		mold_direction = -1.0;
	}
	else if (mold_z >= 12.0) {
		mold_direction = 1.0;
	}

	mold_z -= (speed * 0.4) * mold_direction;
	if (mold_z <= 1.0 && mold_z >= -1.0 && !bottle_created) {
		cut_gob_y = 33.0;
		bottles.push_back(-34.0);
		bottle_created = true;
	}
	else if (mold_z > 1.0 || mold_z < -1.0) {
		bottle_created = false;
	}

	

	// Move supervisor
	if (spv_z >= 30.0) {
		spv_direction = -1.0;
	}
	else if (spv_z <= -30.0) {
		spv_direction = 1.0;
	}
	spv_z += spv_direction;


	// Fan blades rotation 
	if (blade_rotate >= 1.0 ) {
		blade_rotate += 40;
	}

	tick++;
	if (tick == INT_MAX) tick = 0;

	updateQueue();
	
	glutPostRedisplay();
	glutTimerFunc(60, timer, 0);
	
}

int main(int argc, char** argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(960, 960);
	glutInitWindowPosition(10, 10);
	glutCreateWindow("Glass Bottle Factory");
	initialize();

	glutTimerFunc(60, timer, 0);
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	

	glutMainLoop();
	return 0;
}