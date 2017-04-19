/*
*		This code was based upon the OpenGL program framework from
*		Jeff Molofee's tutorials on his website nehe.gamedev.net
*		as well as Rob Bateman's basic OpenGL UI tutorials.
*/

#define _USE_MATH_DEFINES
#define ESCAPE 27
#define UP_ARROW 72
#define DOWN_ARROW 80
#define LEFT_ARROW 75
#define RIGHT_ARROW 77

#include <cmath>
#include <stdio.h>			// Header File For Standard Input/Output
#include <stdlib.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <time.h>
#include <gl\glew.h>
#include <gl\glut.h>
#include <gl\gl.h>			// Header File For The OpenGL32 Library
#include <gl\glu.h>			// Header File For The GLu32 Library
#include "SOIL.h"
#include <assert.h>

const float piover180 = 0.0174532925f;
const float pi = 3.1415926536f;

GLfloat	yrot;						// Y Rotation
GLfloat lookupdown = 0.0f;			// X Rotation
GLfloat diffrot = 0.0f;
GLfloat	z = 0.0f;			// Depth Into The Screen
GLUquadric *quad;			//Object for sphere draws

GLfloat LightAmbient[] = { 10.0f, 10.0f, 10.0f, 1.0f };	//Ambient Light Values
GLfloat LightDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };	//Diffuse Light Values
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };	//Diffuse Light Position
GLfloat xpos = 0.0f;									//Camera x position
GLfloat zpos = 0.0f;									//Camera z position

clock_t swapPatternTime;

GLuint	listLED;		//Draw Lists
GLuint	screen = 1;
GLuint	numLED = 64;

int winw = 1280;
int winh = 720;
int xOffset = winw * 0.875;
int yOffset = winh - 40;
int GlobalRef = 0;
int layer = 0;
int window;



/*
*	A structure to represent the mouse information
*/
struct Mouse
{
	int x;		/*	the x coordinate of the mouse cursor	*/
	int y;		/*	the y coordinate of the mouse cursor	*/
	int lmb;	/*	is the left button pressed?		*/
	int mmb;	/*	is the middle button pressed?	*/
	int rmb;	/*	is the right button pressed?	*/

	int xpress; /*	stores the x-coord of when the first button press occurred	*/
	int ypress; /*	stores the y-coord of when the first button press occurred	*/
};

typedef struct Mouse Mouse;

Mouse TheMouse = { 0, 0, 0, 0, 0 };

struct LED
{
	GLfloat x, y, z;
	GLuint on;
};

struct LEDCUBE
{
	LED pattern[16][4];
	std::vector<std::string> patternStorage;
	GLuint patternIndex;
	clock_t patternTime;
};

/*
*	We will define a function pointer type. ButtonCallback is a pointer to a function that
*	looks a bit like this :
*
*	void func() {
*	}
*/
typedef void(*ButtonCallback)(int num);

/*
*	This is a simple structure that holds a button.
*/
struct Button
{
	int   x;							/* top left x coord of the button */
	int   y;							/* top left y coord of the button */
	int   w;							/* the width of the button */
	int   h;							/* the height of the button */
	int	  state;						/* the state, 1 if pressed, 0 otherwise */
	int	  highlighted;					/* is the mouse cursor over the control? */
	std::string label;						/* the text label of the button */
	ButtonCallback callbackFunction;	/* A pointer to a function to call if the button is pressed */

	int id;								/* A unique ID to represent this Button */

	struct Button* next;				/* a pointer to the next node in the linked list */
};
typedef struct Button Button;

/*
*	The start of a linked list of buttons
*/
Button* pButtonList = NULL;

LEDCUBE theCube;

clock_t getCurrentTime()
{
	return (clock() / (CLOCKS_PER_SEC / 1000));
}

void storePattern()
{
	std::vector<std::string> temp;
	std::ifstream infile("pattern.txt");
	std::string line;

	if (infile.is_open())
	{
		while (std::getline(infile, line))
		{
			if (line.length() != 0 && line.find("//"))
			{
				theCube.patternStorage.push_back(line);
			}
		}
		infile.close();
	}

	else
	{
		std::cout << "Error opening input file\n";
	}
}

void setPattern()
{
	std::string currPattern = theCube.patternStorage[theCube.patternIndex];
	std::istringstream iss(currPattern);
	bool looping = true;
	int layer = 0;
	int column = 0;
	std::string temp;

	for (int i = 0; i < 16; ++i)
	{
		iss >> temp;
		temp.erase(0, 1);
		temp.erase(4, 5);

		theCube.pattern[i][0].on = (int)temp[0] - '0';
		theCube.pattern[i][1].on = (int)temp[1] - '0';
		theCube.pattern[i][2].on = (int)temp[2] - '0';
		theCube.pattern[i][3].on = (int)temp[3] - '0';

	}

	iss >> theCube.patternTime;
	swapPatternTime = getCurrentTime() + (theCube.patternTime * 20);
	theCube.patternIndex++;

	//Reset pattern to first after reaching final pattern
	if (theCube.patternIndex >= theCube.patternStorage.size())
	{
		theCube.patternIndex = 0;
	}
}

void initCube()
{
	//Set Default Settings

	theCube.patternIndex = 0;
	theCube.patternTime = 0;

	//Set LED positions for drawing

	float xOffset = -3.0f;
	float yOffset = -3.0f;
	float zOffset = -3.0f;

	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			theCube.pattern[i][j].x = xOffset;
			theCube.pattern[i][j].y = yOffset;
			theCube.pattern[i][j].z = zOffset;
			theCube.pattern[i][j].on = false;

			yOffset += 2.0f;

			if (j == 3)
			{
				yOffset = -3.0f;
				xOffset += 2.0f;
			}

			if (xOffset > 3.0f)
			{
				xOffset = -3.0f;
				zOffset += 2.0f;
			}
		}
	}

	//Store Patterns

	storePattern();

	//Set Initial Pattern

	setPattern();

}

GLint initLEDList(GLuint list, GLUquadric *quad)		//Initialize draw lists for the cube
{
	for (GLint i = 0; i < numLED; ++i)
	{
		gluQuadricDrawStyle(quad, GLU_FILL);
		gluQuadricNormals(quad, GLU_SMOOTH);
		gluQuadricOrientation(quad, GLU_OUTSIDE);
		glNewList(list + i, GL_COMPILE);
		gluSphere(quad, 1.0, 100, 100);
		glEndList();
	}
	return list;
}

void drawCube()
{
	for (int i = 0; i < 16; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (theCube.pattern[i][j].on)
			{
				glPushMatrix();
				glColor4f(0.2f, 0.2f, 0.75f, 1.0f);
				glTranslatef(theCube.pattern[i][j].x, theCube.pattern[i][j].y, theCube.pattern[i][j].z);
				glScalef(0.25f, 0.25f, 0.25f);
				glCallList(listLED);
				glPopMatrix();
			}
			else
			{
				glPushMatrix();
				glColor4f(0.75f, 0.75f, 0.75f, 0.02f);
				glTranslatef(theCube.pattern[i][j].x, theCube.pattern[i][j].y, theCube.pattern[i][j].z);
				glScalef(0.25f, 0.25f, 0.25f);
				glCallList(listLED);
				glPopMatrix();
			}
		}
	}
}

/*
*
*/
int CreateButton(std::string label, ButtonCallback cb, int x, int y, int w, int h)
{
	Button* p = (Button*)malloc(sizeof(Button));
	assert(p);
	memset(p, 0, sizeof(Button));
	p->x = x;
	p->y = y;
	p->w = w;
	p->h = h;
	p->callbackFunction = cb;
	p->label = label;

	p->next = pButtonList;
	pButtonList = p;

	return p->id = ++GlobalRef;
}

int DeleteButtonByName(std::string label)
{
	Button* previous = NULL, *curr = pButtonList;
	while (curr != NULL) {
		if (label.compare(curr->label) == 0) {
			if (previous)
				previous->next = curr->next;
			else
				pButtonList = curr->next;
			free(curr);
			return 1;
		}
		previous = curr;
		curr = curr->next;
	}
	return 0;
}

int DeleteButtonById(int id)
{
	Button	*previous = NULL,
		*curr = pButtonList;

	while (curr != NULL)
	{
		if (id == curr->id)
		{
			if (previous)
				previous->next = curr->next;
			else
				pButtonList = curr->next;
			free(curr);
			return 1;
		}
		previous = curr;
		curr = curr->next;
	}
	return 0;
}

/*----------------------------------------------------------------------------------------
*	This is an example callback function. Notice that it's type is the same
*	an the ButtonCallback type. We can assign a pointer to this function which
*	we can store and later call.
*/
static void TheButtonCallback(int num)
{
	printf("I have been called %d\n", num);
}

static void ResetCallback(int num)
{
	printf("I have been called %d\n", num);
}

static void SaveCallback(int num)
{
	printf("I have been called %d\n", num);
}

static void LayerUpCallback(int num)
{
	printf("I have been called %d and layer is %d\n", num, layer);
	if (layer == 3)
	{
		layer = 0;
	}
	else
	{
		layer++;
	}
	printf("Layer is now %d\n", layer);
}

static void LayerDownCallback(int num)
{
	printf("I have been called %d and layer is %d\n", num, layer);
	if (layer == 0)
	{
		layer = 3;
	}
	else
	{
		layer--;
	}
	printf("Layer is now %d\n", layer);
}

/*----------------------------------------------------------------------------------------
*	\brief	This function draws a text string to the screen using glut bitmap fonts.
*	\param	font	-	the font to use. it can be one of the following :
*
*					GLUT_BITMAP_9_BY_15
*					GLUT_BITMAP_8_BY_13
*					GLUT_BITMAP_TIMES_ROMAN_10
*					GLUT_BITMAP_TIMES_ROMAN_24
*					GLUT_BITMAP_HELVETICA_10
*					GLUT_BITMAP_HELVETICA_12
*					GLUT_BITMAP_HELVETICA_18
*
*	\param	text	-	the text string to output
*	\param	x		-	the x co-ordinate
*	\param	y		-	the y co-ordinate
*/
void Font(void *font, std::string text, int x, int y)
{
	glRasterPos2i(x, y);

	for (std::string::iterator it = text.begin(); it != text.end(); ++it)
	{
		glutBitmapCharacter(font, *it);
	}
}


/*----------------------------------------------------------------------------------------
*	\brief	This function is used to see if a mouse click or event is within a button
*			client area.
*	\param	b	-	a pointer to the button to test
*	\param	x	-	the x coord to test
*	\param	y	-	the y-coord to test
*/
int ButtonClickTest(Button* b, int x, int y)
{
	if (b)
	{
		/*
		*	If clicked within button area, then return true
		*/
		if (x > b->x      &&
			x < b->x + b->w &&
			y > b->y      &&
			y < b->y + b->h) {
			return 1;
		}
	}

	/*
	*	otherwise false.
	*/
	return 0;
}

/*----------------------------------------------------------------------------------------
*	\brief	This function draws the specified button.
*	\param	b	-	a pointer to the button to check.
*	\param	x	-	the x location of the mouse cursor.
*	\param	y	-	the y location of the mouse cursor.
*/
void ButtonRelease(int x, int y)
{
	Button* b = pButtonList;
	while (b)
	{
		/*
		*	If the mouse button was pressed within the button area
		*	as well as being released on the button.....
		*/
		if (ButtonClickTest(b, TheMouse.xpress, TheMouse.ypress) &&
			ButtonClickTest(b, x, y))
		{
			/*
			*	Then if a callback function has been set, call it.
			*/

			if (b->callbackFunction)
			{
				b->callbackFunction(b->id);
			}
		}

		/*
		*	Set state back to zero.
		*/
		b->state = 0;

		b = b->next;
	}
}

/*----------------------------------------------------------------------------------------
*	\brief	This function draws the specified button.
*	\param	b	-	a pointer to the button to check.
*	\param	x	-	the x location of the mouse cursor.
*	\param	y	-	the y location of the mouse cursor.
*/
void ButtonPress(int x, int y)
{
	Button* b = pButtonList;
	while (b)
	{
		/*
		*	if the mouse click was within the buttons client area,
		*	set the state to true.
		*/
		if (ButtonClickTest(b, x, y))
		{
			b->state = 1;
		}
		b = b->next;
	}
}


/*----------------------------------------------------------------------------------------
*	\brief	This function draws the specified button.
*	\param	b	-	a pointer to the button to check.
*	\param	x	-	the x location of the mouse cursor.
*	\param	y	-	the y location of the mouse cursor.
*/
void ButtonPassive(int x, int y)
{
	int needRedraw = 0;
	Button* b = pButtonList;
	while (b)
	{
		/*
		*	if the mouse moved over the control
		*/
		if (ButtonClickTest(b, x, y))
		{
			/*
			*	If the cursor has just arrived over the control, set the highlighted flag
			*	and force a redraw. The screen will not be redrawn again until the mouse
			*	is no longer over this control
			*/
			if (b->highlighted == 0) {
				b->highlighted = 1;
				needRedraw = 1;
			}
		}
		/*
		*	If the cursor is no longer over the control, then if the control
		*	is highlighted (ie, the mouse has JUST moved off the control) then
		*	we set the highlighting back to false, and force a redraw.
		*/
		else if (b->highlighted == 1)
		{
			b->highlighted = 0;
			needRedraw = 1;
		}
		if (b->id == 18 && b->label != std::to_string(layer))
		{
			b->label = std::to_string(layer);
		}

		b = b->next;
	}
	if (needRedraw) {
		glutPostRedisplay();
	}
}

/*----------------------------------------------------------------------------------------
*	\brief	This function draws the specified button.
*/
void ButtonDraw()
{
	int fontx;
	int fonty;

	Button* b = pButtonList;
	while (b)
	{
		/*
		*	We will indicate that the mouse cursor is over the button by changing its
		*	colour.
		*/
		if (b->highlighted)
			glColor3f(0.7f, 0.7f, 0.8f);
		else
			glColor3f(0.6f, 0.6f, 0.6f);

		/*
		*	draw background for the button.
		*/
		glBegin(GL_QUADS);
		glVertex2i(b->x, b->y);
		glVertex2i(b->x, b->y + b->h);
		glVertex2i(b->x + b->w, b->y + b->h);
		glVertex2i(b->x + b->w, b->y);
		glEnd();

		/*
		*	Draw an outline around the button with width 3
		*/
		glLineWidth(1);

		/*
		*	The colours for the outline are reversed when the button.
		*/
		if (b->state)
			glColor3f(0.4f, 0.4f, 0.4f);
		else
			glColor3f(0.8f, 0.8f, 0.8f);

		glBegin(GL_LINE_STRIP);
		glVertex2i(b->x + b->w, b->y);
		glVertex2i(b->x, b->y);
		glVertex2i(b->x, b->y + b->h);
		glEnd();

		if (b->state)
			glColor3f(0.8f, 0.8f, 0.8f);
		else
			glColor3f(0.4f, 0.4f, 0.4f);

		glBegin(GL_LINE_STRIP);
		glVertex2i(b->x, b->y + b->h);
		glVertex2i(b->x + b->w, b->y + b->h);
		glVertex2i(b->x + b->w, b->y);
		glEnd();

		glLineWidth(1);


		/*
		*	Calculate the x and y coords for the text string in order to center it.
		*/
		fontx = b->x + (b->w - glutBitmapLength(GLUT_BITMAP_HELVETICA_10, (const unsigned char*)b->label.c_str())) / 2;
		fonty = b->y + (b->h + 10) / 2;

		/*
		*	if the button is pressed, make it look as though the string has been pushed
		*	down. It's just a visual thing to help with the overall look....
		*/
		if (b->state) {
			fontx += 2;
			fonty += 2;
		}

		/*
		*	If the cursor is currently over the button we offset the text string and draw a shadow
		*/
		if (b->highlighted)
		{
			glColor3f(0, 0, 0);
			Font(GLUT_BITMAP_HELVETICA_10, b->label, fontx, fonty);
			fontx--;
			fonty--;
		}

		glColor3f(1, 1, 1);
		Font(GLUT_BITMAP_HELVETICA_10, b->label, fontx, fonty);

		b = b->next;
	}
}



void InitButtons()
{
	for (int i = 0; i < 16; ++i)
	{
		if (i % 4 == 0 && i != 0)
		{
			xOffset = winw * 0.875;
			yOffset -= 40;
		}
		CreateButton(std::to_string(i), TheButtonCallback, xOffset, yOffset, 30, 30);
		xOffset += 40;
	}

	xOffset = winw * 0.875;
	yOffset = winh - 40;

	CreateButton("UP", LayerUpCallback, xOffset - 50, yOffset - 90, 40, 25);
	CreateButton(std::to_string(layer), TheButtonCallback, xOffset - 50, yOffset - 60, 40, 25);
	CreateButton("DOWN", LayerDownCallback, xOffset - 50, yOffset - 30, 40, 25);
	CreateButton("Save", SaveCallback, (winw * 0.5) - 50, winh * 0.95, 50, 25);
	CreateButton("Reset", ResetCallback, (winw * 0.5) + 25, winh * 0.95, 50, 25);
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	glEnable(GL_TEXTURE_2D);							// Enable Texture Mapping
	glShadeModel(GL_SMOOTH);							// Enable Smooth Shading
	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);				// Black Background
	glClearDepth(1.0f);									// Depth Buffer Setup
	glEnable(GL_DEPTH_TEST);							// Enables Depth Testing
	glDepthFunc(GL_LEQUAL);								// The Type Of Depth Testing To Do
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations

	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);		// Setup The Ambient Light
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);		// Setup The Diffuse Light
	glLightfv(GL_LIGHT1, GL_POSITION, LightPosition);	// Position The Light
	glEnable(GL_LIGHT1);								// Enable Light One
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LIGHTING);

	initCube();
	InitButtons();
	listLED = glGenLists(numLED);
	quad = gluNewQuadric();
	listLED = initLEDList(listLED, quad);

	return true;										// Initialization Went OK
}

/*----------------------------------------------------------------------------------------
*	\brief	This function is called whenever a mouse button is pressed or released
*	\param	button	-	GLUT_LEFT_BUTTON, GLUT_RIGHT_BUTTON, or GLUT_MIDDLE_BUTTON
*	\param	state	-	GLUT_UP or GLUT_DOWN depending on whether the mouse was released
*						or pressed respectivly.
*	\param	x		-	the x-coord of the mouse cursor.
*	\param	y		-	the y-coord of the mouse cursor.
*/
void MouseButton(int button, int state, int x, int y)
{
	/*
	*	update the mouse position
	*/
	TheMouse.x = x;
	TheMouse.y = y;

	/*
	*	has the button been pressed or released?
	*/
	if (state == GLUT_DOWN)
	{
		/*
		*	This holds the location of the first mouse click
		*/
		//if ( !(TheMouse.lmb || TheMouse.mmb || TheMouse.rmb) ) {
		TheMouse.xpress = x;
		TheMouse.ypress = y;
		//}

		/*
		*	Which button was pressed?
		*/
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			TheMouse.lmb = 1;
			ButtonPress(x, y);
		case GLUT_MIDDLE_BUTTON:
			TheMouse.mmb = 1;
			break;
		case GLUT_RIGHT_BUTTON:
			TheMouse.rmb = 1;
			break;
		}
	}
	else
	{
		/*
		*	Which button was released?
		*/
		switch (button)
		{
		case GLUT_LEFT_BUTTON:
			TheMouse.lmb = 0;
			ButtonRelease(x, y);
			break;
		case GLUT_MIDDLE_BUTTON:
			TheMouse.mmb = 0;
			break;
		case GLUT_RIGHT_BUTTON:
			TheMouse.rmb = 0;
			break;
		}
	}

	/*
	*	Force a redraw of the screen. If we later want interactions with the mouse
	*	and the 3D scene, we will need to redraw the changes.
	*/
	glutPostRedisplay();
}

/*----------------------------------------------------------------------------------------
*	\brief	This function is called whenever the mouse cursor is moved AND A BUTTON IS HELD.
*	\param	x	-	the new x-coord of the mouse cursor.
*	\param	y	-	the new y-coord of the mouse cursor.
*/
void MouseMotion(int x, int y)
{
	/*
	*	Calculate how much the mouse actually moved
	*/
	int dx = x - TheMouse.x;
	int dy = y - TheMouse.y;

	/*
	*	update the mouse position
	*/
	TheMouse.x = x;
	TheMouse.y = y;


	/*
	*	Check MyButton to see if we should highlight it cos the mouse is over it
	*/
	ButtonPassive(x, y);

	/*
	*	Force a redraw of the screen
	*/
	glutPostRedisplay();
}

/*----------------------------------------------------------------------------------------
*	\brief	This function is called whenever the mouse cursor is moved AND NO BUTTONS ARE HELD.
*	\param	x	-	the new x-coord of the mouse cursor.
*	\param	y	-	the new y-coord of the mouse cursor.
*/
void MousePassiveMotion(int x, int y)
{
	/*
	*	Calculate how much the mouse actually moved
	*/
	int dx = x - TheMouse.x;
	int dy = y - TheMouse.y;

	/*
	*	update the mouse position
	*/
	TheMouse.x = x;
	TheMouse.y = y;

	/*
	*	Check MyButton to see if we should highlight it cos the mouse is over it
	*/
	ButtonPassive(x, y);

}

GLvoid DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (winh == 0) ? (1) : ((float)winw / winh), 1, 100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	GLfloat xtrans = -xpos;
	GLfloat ztrans = -zpos;
	GLfloat ytrans = 4.0f;
	GLfloat sceneroty = 360.0f - yrot;

	glTranslatef(0.0f, 0.0f, -25.0f);
	glTranslatef(0.0f, 0.0f, ztrans);

	glPushMatrix();
	glRotatef(sceneroty, 0, 1.0f, 0);
	glRotatef(lookupdown, 1.0f, 0, 0);
	drawCube();
	glPopMatrix();

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	/*
	*	Set the orthographic viewing transformation
	*/
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, winw, winh, 0, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/*
	*	Draw the 2D overlay
	*/
	ButtonDraw();

	/*
	*	Bring the back buffer to the front and vice-versa.
	*/
	glutSwapBuffers();

	if (getCurrentTime() >= swapPatternTime)
	{
		setPattern();
	}
}

/*----------------------------------------------------------------------------------------
*	This function is called when the window is resized. All this does is simply
*	store the new width and height of the window which are then referenced by
*	the draw function to set the correct viewing transforms
*/
GLvoid ReSizeGLScene(GLsizei Width, GLsizei Height)
{
	if (Height == 0)				// Prevent A Divide By Zero If The Window Is Too Small
		Height = 1;

	glViewport(0, 0, Width, Height);		// Reset The Current Viewport And Perspective Transformation

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	gluPerspective(45.0f, (GLfloat)Width / (GLfloat)Height, 0.1f, 100.0f);
	glMatrixMode(GL_MODELVIEW);
}

void usleep(__int64 usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

/* The function called whenever a normal key is pressed. */
void keyPressed(unsigned char key, int x, int y)
{
	/* avoid thrashing this procedure */
	usleep(50);

	switch (key)
	{
	case ESCAPE: // kill everything.
		glutDestroyWindow(window);
		break;
	case 's':
	case 'S':
		diffrot = (yrot + yrot);
		xpos += (float)sin(yrot*piover180) * 0.075f;
		zpos += (float)cos(yrot*piover180) * 0.075f;
		break;
	case 'w':
	case 'W':
		diffrot = (yrot + yrot);
		xpos -= (float)sin(yrot*piover180) * 0.075f;
		zpos -= (float)cos(yrot*piover180) * 0.075f;
		break;
	default:
		printf("Key %d pressed. No action there yet.\n", key);
		break;
	}
}

/* The function called whenever a normal key is pressed. */
void specialKeyPressed(int key, int x, int y)
{
	/* avoid thrashing this procedure */
	usleep(50);

	switch (key)
	{
	case GLUT_KEY_UP:
		if (lookupdown <= -359.0f)
		{
			lookupdown = 0.0f;
		}
		lookupdown -= 1.0f;
		break;

	case GLUT_KEY_DOWN:
		if (lookupdown >= 359.0f)
		{
			lookupdown = 0.0f;
		}
		lookupdown += 1.0f;
		break;

	case GLUT_KEY_LEFT:
		if (yrot >= 359.0f)
		{
			yrot = 0.0f;
		}
		yrot += 1.0f;
		break;

	case GLUT_KEY_RIGHT:
		if (yrot <= -359.0f)
		{
			yrot = 0.0f;
		}
		yrot -= 1.0f;
		break;

	default:
		printf("Special key %d pressed. No action there yet.\n", key);
		break;
	}
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH | GLUT_ALPHA);
	glutInitWindowSize(winw, winh);
	glutInitWindowPosition(200, 100);
	window = glutCreateWindow("LED Cube Simulation");

	glutDisplayFunc(&DrawGLScene);
	glutIdleFunc(&DrawGLScene);
	glutReshapeFunc(&ReSizeGLScene);
	glutKeyboardFunc(&keyPressed);
	glutSpecialFunc(&specialKeyPressed);
	glutMouseFunc(&MouseButton);
	glutMotionFunc(&MouseMotion);
	glutPassiveMotionFunc(&MousePassiveMotion);

	InitGL();
	
	glutMainLoop();
}