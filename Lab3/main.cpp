// CS3241Lab3.cpp : Defines the entry point for the console application.
//#include <cmath>
#include "math.h"
#include <iostream>
#include <fstream>

#ifdef _WIN32
#include <Windows.h>
#include "glut.h"
#define M_PI 3.141592654
#elif __APPLE__
#include <OpenGL/gl.h>
#include <GLUT/GLUT.h>
#endif

#define INIT_ZOOM 1.0
#define INIT_FOV 40.0
#define INIT_ANGLE 0
#define INIT_ANGLE_2 0
#define INIT_X_TRANSLATION 0
#define INIT_NEAR_CP 1
#define INIT_FAR_CP 80

// global variable

bool m_Smooth = false;
bool m_Highlight = false;
bool m_Emission = false;
GLfloat angle = INIT_ANGLE;   /* in degrees */
GLfloat angle2 = INIT_ANGLE_2;   /* in degrees */
GLfloat zoom = INIT_ZOOM;
GLfloat field_of_view = INIT_FOV;
GLfloat x_translation = INIT_X_TRANSLATION;
GLfloat near_clipping_plane = INIT_NEAR_CP;
GLfloat far_clipping_plane = INIT_FAR_CP;

int mouseButton = 0;
int moving, startx, starty;

#define NO_OBJECT 4;
int current_object = 0;

using namespace std;

// Ball components
#define numPoints 500
#define ANGLE_PRECISION 36000.0
#define POSITION_PRECISION 1000.0
#define PARTICLE_BOUNDS 5.0

#define DEBUG_VECTORS 0

double x[numPoints],y[numPoints],z[numPoints];

void setupLighting()
{
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);

	// Lights, material properties
	GLfloat	ambientProperties[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	GLfloat	diffuseProperties[] = { 0.8f, 0.8f, 0.8f, 1.0f };
	GLfloat	specularProperties[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat lightPosition[] = { -100.0f, 100.0f, 100.0f, 1.0f };

	glClearDepth(1.0);

	glLightfv(GL_LIGHT0, GL_AMBIENT, ambientProperties);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseProperties);
	glLightfv(GL_LIGHT0, GL_SPECULAR, specularProperties);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);

	glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 0.0);

	// Default : lighting
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

}

void configureMaterials() {
    float no_mat[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    float mat_ambient[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    float mat_diffuse[] = { 0.1f, 0.5f, 0.8f, 1.0f };
    float mat_emission[] = { 0.2f, 0.2f, 0.2f, 0.0f };
    float mat_specular[] = { 0.3f, 0.3f, 0.3f, 1.0f };
    float shininess = 5.0f;
    float no_shininess = 0.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
    
    if (m_Highlight) {
        // your codes for highlight here
        glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
        glMaterialf(GL_FRONT, GL_SHININESS, shininess);
    } else {
        glMaterialfv(GL_FRONT, GL_SPECULAR, no_mat);
        glMaterialf(GL_FRONT, GL_SHININESS, no_shininess);
    }
    
    if (m_Emission) {
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    } else {
        glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);
    }
}


float generate_random_number(float precision = 100, float lower = 0, float upper = 1){
    return rand()%(int)precision/precision * (upper - lower) + lower;
}

float square(float a) {
    return a*a;
}

void init() {
    glClearColor(0.1, 0.1, 0.1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    
    for(int i = 0; i < numPoints; i++) {
        float theta = (rand()%(int)ANGLE_PRECISION) / ANGLE_PRECISION * 2 * M_PI;
        float phi = (rand()%(int)ANGLE_PRECISION) / ANGLE_PRECISION * 2 * M_PI;
        x[i] = generate_random_number(POSITION_PRECISION, 0, PARTICLE_BOUNDS) * sin(theta) * cos(phi);
        y[i] = generate_random_number(POSITION_PRECISION, 0, PARTICLE_BOUNDS) * sin(theta) * sin(phi);
        z[i] = generate_random_number(POSITION_PRECISION, 0, PARTICLE_BOUNDS) * cos(theta);
    }
}

void drawSphere(int n, double x, double y, double z, double r)
{
    configureMaterials();

	int i, j;
//    int n = 20;
    for (i = 0; i<n; i++) {
        for (j = 0; j<2 * n; j++) {
            
            // Four verticies of a quad of a sphere
            
            float v1x = r*sin(i*M_PI / n)*cos(j*M_PI / n);
            float v1y = r*cos(i*M_PI / n)*cos(j*M_PI / n);
            float v1z = r*sin(j*M_PI / n);
            
            float v2x = r*sin((i + 1)*M_PI / n)*cos(j*M_PI / n);
            float v2y = r*cos((i + 1)*M_PI / n)*cos(j*M_PI / n);
            float v2z = r*sin(j*M_PI / n);
            
            float v3x = r*sin((i + 1)*M_PI / n)*cos((j + 1)*M_PI / n);
            float v3y = r*cos((i + 1)*M_PI / n)*cos((j + 1)*M_PI / n);
            float v3z = r*sin((j + 1)*M_PI / n);
            
            float v4x = r*sin(i*M_PI / n)*cos((j + 1)*M_PI / n);
            float v4y = r*cos(i*M_PI / n)*cos((j + 1)*M_PI / n);
            float v4z = r*sin((j + 1)*M_PI / n);
            
            glBegin(GL_POLYGON);
            
            // the normal of each vertex is actaully its own coordinates normalized for a sphere
            if(m_Smooth) glNormal3d(v1x, v1y, v1z);
            // Explanation: the normal of the whole polygon is the coordinate of the center of the polygon for a sphere
            else glNormal3d(sin((i + 0.5)*M_PI / n)*cos((j + 0.5)*M_PI / n), cos((i + 0.5)*M_PI / n)*cos((j + 0.5)*M_PI / n), sin((j + 0.5)*M_PI / n));
            glVertex3d(v1x + x, v1y + y, v1z + z);
            
            if(m_Smooth) glNormal3d(v2x, v2y, v2z);
            glVertex3d(v2x + x, v2y + y, v2z + z);
            
            if(m_Smooth) glNormal3d(v3x, v3y, v3z);
            glVertex3d(v3x + x, v3y + y, v3z + z);
            
            if(m_Smooth) glNormal3d(v4x, v4y, v4z);
            glVertex3d(v4x + x, v4y + y, v4z + z);
            
            glEnd();
        }
    }

}

void drawCylinder(double x, double y, double z, double r, double h)
{
    configureMaterials();
    int i;
    int n = 20;
    
    for (i = 0; i<n; i++) {
        float angle = i * M_PI * 2 / n;
        float nextAngle = (i + 1) * M_PI * 2 / n;
        
        // Vertex top center
        GLfloat vtc_x = x;
        GLfloat vtc_y = y + 0.5f*h;
        GLfloat vtc_z = z;
        
        // Vertex top edge 1
        GLfloat vt1_x = x + r * sin(angle);
        GLfloat vt1_y = vtc_y;
        GLfloat vt1_z = z + r * cos(angle);
        
        // Vertex top edge 2
        GLfloat vt2_x = x + r * sin(nextAngle);
        GLfloat vt2_y = vtc_y;
        GLfloat vt2_z = z + r * cos(nextAngle);
        
        // Vertex bottom center
        GLfloat vbc_x = vtc_x;
        GLfloat vbc_y = vtc_y - h;
        GLfloat vbc_z = vtc_z;
        
        // Vertex bottom edge 1
        GLfloat vb1_x = vt1_x;
        GLfloat vb1_y = vt1_y - h;
        GLfloat vb1_z = vt1_z;
        
        // Vertex bottom edge 2
        GLfloat vb2_x = vt2_x;
        GLfloat vb2_y = vt2_y - h;
        GLfloat vb2_z = vt2_z;
        
        // Draw Top Cap
        glBegin(GL_POLYGON);
        
        glNormal3d(0, 1, 0);
        glVertex3d(vtc_x, vtc_y, vtc_z);
        
        if(m_Smooth) glNormal3d(0, 1, 0);
        glVertex3d(vt1_x, vt1_y, vt1_z);
        
        if(m_Smooth) glNormal3d(0, 1, 0);
        glVertex3d(vt2_x, vt2_y, vt2_z);
        
        glEnd();
        
        // Draw cylinder sides
        glBegin(GL_POLYGON);
        
        GLfloat avAngle = (angle+nextAngle)/2;
        if(m_Smooth) glNormal3d(r*sin(angle), 0, r*cos(angle));
        else glNormal3d(r*sin(avAngle), 0, r*cos(avAngle));
        glVertex3d(vt1_x, vt1_y, vt1_z);
        
        if(m_Smooth) glNormal3d(r*sin(nextAngle), 0, r*cos(nextAngle));
        glVertex3d(vt2_x, vt2_y, vt2_z);
        
        if(m_Smooth) glNormal3d(r*sin(nextAngle), 0, r*cos(nextAngle));
        glVertex3d(vb2_x, vb2_y, vb2_z);
        
        if(m_Smooth) glNormal3d(r*sin(angle), 0, r*cos(angle));
        glVertex3d(vb1_x, vb1_y, vb1_z);
        
        glEnd();
        
        // Draw Bottom Cap
        glBegin(GL_POLYGON);
        
        glNormal3d(0, -1, 0);
        glVertex3d(vbc_x, vbc_y, vbc_z);
        
        if(m_Smooth) glNormal3d(0, -1, 0);
        glVertex3d(vb1_x, vb1_y, vb1_z);
        
        if(m_Smooth) glNormal3d(0, -1, 0);
        glVertex3d(vb2_x, vb2_y, vb2_z);
        
        glEnd();
    }
}


void drawPipe(float x, float y, float z, float a, float b, float c) {
    
    // mid points between two points
    float midX = x + (a - x)/2;
    float midY = y + (b - y)/2;
    float midZ = z + (c - z)/2;
    
    // delta vector that points towards the other point
    float deltaX = a - x;
    float deltaY = b - y;
    float deltaZ = c - z;
    
    // distance between two points
    float distance = sqrtf(square(deltaX) + square(deltaY) + square(deltaZ));
    
    // Normal of up vector with delta vector
    float normalX = -deltaZ / sqrt(square(deltaZ) + square(deltaX));
    float normalY = 0;
    float normalZ = deltaX / sqrt(square(deltaZ) + square(deltaX));
    
    
    if(DEBUG_VECTORS) {
        //debug - draw up vector
        glBegin(GL_LINES);
        glColor3f(1, 1, 1);
        glVertex3f(midX, midY, midZ);
        glVertex3f(midX, midY + 1, midZ);
        glEnd();
        
        //debug - draw delta vector
        glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(midX, midY, midZ);
        glVertex3f(midX + deltaX, midY + deltaY, midZ + deltaZ);
        glEnd();
        
        //debug - draw normal vector
        glBegin(GL_LINES);
        glColor3f(0, 1, 0);
        glVertex3f(midX, midY, midZ);
        glVertex3f(midX + normalX, midY + normalY, midZ + normalZ);
        glEnd();
    }
    
    
    // angle between up vector and delta vector using cosine angle formula
    float deltaAngle = acos(deltaY / sqrt(square(deltaX) + square(deltaY) + square(deltaZ))) * 180 / M_PI;
    
    glPushMatrix();
    
    // Set local transform to where the cylinder is
    glTranslatef(midX, midY, midZ);
    
    // Rotate up vector towards delta vector (aligns a object that is aligned up to align with two points)
    glRotatef(-deltaAngle, normalX, normalY, normalZ);
    
    drawCylinder(0, 0, 0, 0.0025, distance);
    
    glPopMatrix();
}

void drawSecondComposite() {
    glPushMatrix();
    glRotatef(((glutGet(GLUT_ELAPSED_TIME)/36.0)), 0, 1, 0);
    
    for(int i=0; i<numPoints; i++) {
        drawSphere(5, x[i], y[i], z[i], 0.01);
    }
    
    int drawPipes = 0;
    if(drawPipes == 1) {
        for(int i=0; i<numPoints-1; i++) {
            drawPipe(x[i],y[i],z[i],x[i+1],y[i+1],z[i+1]);
        }
    }
    
    glPopMatrix();
    glutPostRedisplay();
}

void drawFirstComposite() {
    
    
    
    glutPostRedisplay();
}

void display(void)
{//Add Projection tool and Camera Movement somewhere here
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(field_of_view, 1.0, near_clipping_plane, far_clipping_plane);
    glMatrixMode(GL_MODELVIEW);
    gluLookAt(0, 0, 0, 1, 1, -1, 0, 1, 0); // eye, center, up

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushMatrix();
	glTranslatef(0, 0, -6);
	glRotatef(angle2, 1.0, 0.0, 0.0);
	glRotatef(angle, 0.0, 1.0, 0.0);

	glScalef(zoom, zoom, zoom);
	
	switch (current_object) {
	case 0:
		drawSphere(20,0,0,0,1);
		break;
	case 1:
		// draw your second primitive object here
        drawCylinder(0,0,0,1,2);
		break;
	case 2:
		// draw your first composite object here
        drawFirstComposite();
		break;
	case 3:
		// draw your second composite object here
        drawSecondComposite();
		break;
	default:
		break;
	};
	glPopMatrix();
	glutSwapBuffers();
}


void resetCamera(){
    //fill in values below.
    zoom = INIT_ZOOM;
    angle =   INIT_ANGLE;
    angle2 =   INIT_ANGLE_2;
    zoom = INIT_ZOOM;
    field_of_view = INIT_FOV;
    x_translation = INIT_X_TRANSLATION;
    // include resetting of gluPerspective & gluLookAt.
	return;
}

void setCameraBestAngle() {
    //fill in values below
    zoom = INIT_ZOOM;
    angle =   INIT_ANGLE;
    angle2 =   INIT_ANGLE_2;
    zoom = 2.0;
    field_of_view = INIT_FOV;
    x_translation = INIT_X_TRANSLATION;
    //TIPS: Adjust gluLookAt function to change camera position
    
	return;
}

void keyboard(unsigned char key, int x, int y)
{//add additional commands here to change Field of View and movement
	switch (key) {
	case 'p':
	case 'P':
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case 'w':
	case 'W':
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case 'v':
	case 'V':
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
		break;
	case 's':
	case 'S':
		m_Smooth = !m_Smooth;
		break;
	case 'h':
	case 'H':
		m_Highlight = !m_Highlight;
		break;
        
    case 'e':
    case 'E':
        m_Emission = !m_Emission;
        break;

    case 'n':
    near_clipping_plane+=0.1f;
    cout << "Near Clipping Plane: " << near_clipping_plane << "\n";
    break;
    
    case 'N':
    near_clipping_plane < 0.1f ? 0 : near_clipping_plane-=0.1f;
    cout << "Near Clipping Plane: " << near_clipping_plane << "\n";
    break;
    
    case 'f':
    far_clipping_plane ++;
    cout << "Far Clipping Plane: " << far_clipping_plane << "\n";
    break;
    
    case 'F':
    far_clipping_plane < 1 ? 0 : far_clipping_plane--;
    cout << "Far Clipping Plane: " << far_clipping_plane << "\n";
    break;
    
    case 'o':
    field_of_view++;
    cout << "FOV: " << field_of_view << "\n";
    break;
    
    case 'O':
    field_of_view < 1 ? 0 : field_of_view--;
    cout << "FOV: " << field_of_view << "\n";
    break;

	case 'r':
		resetCamera();
		break;

	case 'R':
		setCameraBestAngle();
		break;

	case '1':
	case '2':
	case '3':
	case '4':
		current_object = key - '1';
		break;

	case 27:
		exit(0);
		break;

	default:
		break;
	}

	glutPostRedisplay();
}



void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		mouseButton = button;
		moving = 1;
		startx = x;
		starty = y;
	}
	if (state == GLUT_UP) {
		mouseButton = button;
		moving = 0;
	}
}

void motion(int x, int y)
{
	if (moving) {
		if (mouseButton == GLUT_LEFT_BUTTON)
		{
			angle = angle + (x - startx);
			angle2 = angle2 + (y - starty);
		}
		else zoom += ((y - starty)*0.01);
		startx = x;
		starty = y;
		glutPostRedisplay();
	}

}


int main(int argc, char **argv)
{
	cout << "CS3241 Lab 3" << endl << endl;

	cout << "1-4: Draw different objects" << endl;
	cout << "S: Toggle Smooth Shading" << endl;
	cout << "H: Toggle Highlight" << endl;
	cout << "W: Draw Wireframe" << endl;
	cout << "P: Draw Polygon" << endl;
	cout << "V: Draw Vertices" << endl;
	cout << "n, N: Reduce or increase the distance of the near plane from the camera" << endl;
	cout << "f, F: Reduce or increase the distance of the far plane from the camera" << endl;
	cout << "o, O: Reduce or increase the distance of the povy plane from the camera" << endl;
	cout << "r: Reset camera to the initial parameters when the program starts" << endl;
	cout << "R: Change camera to another setting that is has the best viewing angle for your object" << endl;
	cout << "ESC: Quit" << endl << endl;


	cout << "Left mouse click and drag: rotate the object" << endl;
	cout << "Right mouse click and drag: zooming" << endl;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(600, 600);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("CS3241 Assignment 3");
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glutDisplayFunc(display);
	glMatrixMode(GL_PROJECTION);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutKeyboardFunc(keyboard);
	setupLighting();
    srand((uint32_t)time(NULL));
    init();
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);

	glutMainLoop();

	return 0;
}
