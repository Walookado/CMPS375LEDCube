/*
*		This code was based upon the OpenGL program framework from
*		Jeff Molofee's tutorials on his website nehe.gamedev.net
*/

#define _USE_MATH_DEFINES
#include <cmath>
#include <windows.h>		// Header File For Windows
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

HDC			hDC = NULL;		// Private GDI Device Context
HGLRC		hRC = NULL;		// Permanent Rendering Context
HWND		hWnd = NULL;		// Holds Our Window Handle
HINSTANCE	hInstance;		// Holds The Instance Of The Application

bool	keys[256];			// Array Used For The Keyboard Routine
bool	active = TRUE;		// Window Active Flag Set To TRUE By Default
bool	fullscreen = TRUE;	// Fullscreen Flag Set To Fullscreen Mode By Default
bool	light;				// Lighting ON/OFF ( NEW )
bool	lp = true;			// L Pressed? ( NEW )
bool	fp;					// F Pressed? ( NEW )
bool	movement = TRUE;
bool	eSwitch = FALSE;
bool	shot = FALSE;
bool	gameover = FALSE;

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

GLuint	filter;						// Which Filter To Use
GLuint	listLED;		//Draw Lists
GLuint	seed = time(0);
GLuint	screen = 1;

GLint	numLED = 64;


LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);	// Declaration For WndProc

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

GLvoid ReSizeGLScene(GLsizei width, GLsizei height)		// Resize And Initialize The GL Window
{
	if (height == 0)										// Prevent A Divide By Zero By
	{
		height = 1;										// Making Height Equal One
	}

	glViewport(0, 0, width, height);						// Reset The Current Viewport

	glMatrixMode(GL_PROJECTION);						// Select The Projection Matrix
	glLoadIdentity();									// Reset The Projection Matrix

	// Calculate The Aspect Ratio Of The Window
	gluPerspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

	glMatrixMode(GL_MODELVIEW);							// Select The Modelview Matrix
	glLoadIdentity();									// Reset The Modelview Matrix
}

int InitGL(GLvoid)										// All Setup For OpenGL Goes Here
{
	/*
	if (!LoadGLTextures())								// Jump To Texture Loading Routine
	{
	return FALSE;									// If Texture Didn't Load Return FALSE
	}
	*/
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
	//glEnable(GL_LIGHTING);

	srand(seed);

	initCube();

	listLED = glGenLists(numLED);
	quad = gluNewQuadric();
	listLED = initLEDList(listLED, quad);

	return TRUE;										// Initialization Went OK
}

int DrawGLScene(GLvoid)									// Here's Where We Do All The Drawing
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	// Clear The Screen And The Depth Buffer
	glLoadIdentity();									// Reset The View

	if (screen == 0)
	{
		//drawScreen(texture, 0);
	}
	if (screen == 1)
	{
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
	}

	return TRUE;										// Keep Going
}

GLvoid KillGLWindow(GLvoid)								// Properly Kill The Window
{
	if (fullscreen)										// Are We In Fullscreen Mode?
	{
		ChangeDisplaySettings(NULL, 0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);								// Show Mouse Pointer
	}

	if (hRC)											// Do We Have A Rendering Context?
	{
		if (!wglMakeCurrent(NULL, NULL))					// Are We Able To Release The DC And RC Contexts?
		{
			MessageBox(NULL, "Release Of DC And RC Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}

		if (!wglDeleteContext(hRC))						// Are We Able To Delete The RC?
		{
			MessageBox(NULL, "Release Rendering Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		}
		hRC = NULL;										// Set RC To NULL
	}

	if (hDC && !ReleaseDC(hWnd, hDC))					// Are We Able To Release The DC
	{
		MessageBox(NULL, "Release Device Context Failed.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hDC = NULL;										// Set DC To NULL
	}

	if (hWnd && !DestroyWindow(hWnd))					// Are We Able To Destroy The Window?
	{
		MessageBox(NULL, "Could Not Release hWnd.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hWnd = NULL;										// Set hWnd To NULL
	}

	if (!UnregisterClass("OpenGL", hInstance))			// Are We Able To Unregister Class
	{
		MessageBox(NULL, "Could Not Unregister Class.", "SHUTDOWN ERROR", MB_OK | MB_ICONINFORMATION);
		hInstance = NULL;									// Set hInstance To NULL
	}
}

/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
*	title			- Title To Appear At The Top Of The Window				*
*	width			- Width Of The GL Window Or Fullscreen Mode				*
*	height			- Height Of The GL Window Or Fullscreen Mode			*
*	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
*	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/

BOOL CreateGLWindow(char* title, int width, int height, int bits, bool fullscreenflag)
{
	GLuint		PixelFormat;			// Holds The Results After Searching For A Match
	WNDCLASS	wc;						// Windows Class Structure
	DWORD		dwExStyle;				// Window Extended Style
	DWORD		dwStyle;				// Window Style
	RECT		WindowRect;				// Grabs Rectangle Upper Left / Lower Right Values
	WindowRect.left = (long)0;			// Set Left Value To 0
	WindowRect.right = (long)width;		// Set Right Value To Requested Width
	WindowRect.top = (long)0;				// Set Top Value To 0
	WindowRect.bottom = (long)height;		// Set Bottom Value To Requested Height

	fullscreen = fullscreenflag;			// Set The Global Fullscreen Flag

	hInstance = GetModuleHandle(NULL);				// Grab An Instance For Our Window
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;	// Redraw On Size, And Own DC For Window.
	wc.lpfnWndProc = (WNDPROC)WndProc;					// WndProc Handles Messages
	wc.cbClsExtra = 0;									// No Extra Window Data
	wc.cbWndExtra = 0;									// No Extra Window Data
	wc.hInstance = hInstance;							// Set The Instance
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);			// Load The Default Icon
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);			// Load The Arrow Pointer
	wc.hbrBackground = NULL;									// No Background Required For GL
	wc.lpszMenuName = NULL;									// We Don't Want A Menu
	wc.lpszClassName = "OpenGL";								// Set The Class Name

	if (!RegisterClass(&wc))									// Attempt To Register The Window Class
	{
		MessageBox(NULL, "Failed To Register The Window Class.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;											// Return FALSE
	}

	if (fullscreen)												// Attempt Fullscreen Mode?
	{
		DEVMODE dmScreenSettings;								// Device Mode
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));	// Makes Sure Memory's Cleared
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);		// Size Of The Devmode Structure
		dmScreenSettings.dmPelsWidth = width;				// Selected Screen Width
		dmScreenSettings.dmPelsHeight = height;				// Selected Screen Height
		dmScreenSettings.dmBitsPerPel = bits;					// Selected Bits Per Pixel
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		// Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			if (MessageBox(NULL, "The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "NeHe GL", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
			{
				fullscreen = FALSE;		// Windowed Mode Selected.  Fullscreen = FALSE
			}
			else
			{
				// Pop Up A Message Box Letting User Know The Program Is Closing.
				MessageBox(NULL, "Program Will Now Close.", "ERROR", MB_OK | MB_ICONSTOP);
				return FALSE;									// Return FALSE
			}
		}
	}

	if (fullscreen)												// Are We Still In Fullscreen Mode?
	{
		dwExStyle = WS_EX_APPWINDOW;								// Window Extended Style
		dwStyle = WS_POPUP;										// Windows Style
		ShowCursor(FALSE);										// Hide Mouse Pointer
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// Window Extended Style
		dwStyle = WS_OVERLAPPEDWINDOW;							// Windows Style
	}

	AdjustWindowRectEx(&WindowRect, dwStyle, FALSE, dwExStyle);		// Adjust Window To True Requested Size

	// Create The Window
	if (!(hWnd = CreateWindowEx(dwExStyle,							// Extended Style For The Window
		"OpenGL",							// Class Name
		title,								// Window Title
		dwStyle |							// Defined Window Style
		WS_CLIPSIBLINGS |					// Required Window Style
		WS_CLIPCHILDREN,					// Required Window Style
		0, 0,								// Window Position
		WindowRect.right - WindowRect.left,	// Calculate Window Width
		WindowRect.bottom - WindowRect.top,	// Calculate Window Height
		NULL,								// No Parent Window
		NULL,								// No Menu
		hInstance,							// Instance
		NULL)))								// Dont Pass Anything To WM_CREATE
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Window Creation Error.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	static	PIXELFORMATDESCRIPTOR pfd =				// pfd Tells Windows How We Want Things To Be
	{
		sizeof(PIXELFORMATDESCRIPTOR),				// Size Of This Pixel Format Descriptor
		1,											// Version Number
		PFD_DRAW_TO_WINDOW |						// Format Must Support Window
		PFD_SUPPORT_OPENGL |						// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,							// Must Support Double Buffering
		PFD_TYPE_RGBA,								// Request An RGBA Format
		bits,										// Select Our Color Depth
		0, 0, 0, 0, 0, 0,							// Color Bits Ignored
		0,											// No Alpha Buffer
		0,											// Shift Bit Ignored
		0,											// No Accumulation Buffer
		0, 0, 0, 0,									// Accumulation Bits Ignored
		16,											// 16Bit Z-Buffer (Depth Buffer)  
		0,											// No Stencil Buffer
		0,											// No Auxiliary Buffer
		PFD_MAIN_PLANE,								// Main Drawing Layer
		0,											// Reserved
		0, 0, 0										// Layer Masks Ignored
	};

	if (!(hDC = GetDC(hWnd)))							// Did We Get A Device Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Device Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(PixelFormat = ChoosePixelFormat(hDC, &pfd)))	// Did Windows Find A Matching Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Find A Suitable PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!SetPixelFormat(hDC, PixelFormat, &pfd))		// Are We Able To Set The Pixel Format?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Set The PixelFormat.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!(hRC = wglCreateContext(hDC)))				// Are We Able To Get A Rendering Context?
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Create A GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	if (!wglMakeCurrent(hDC, hRC))					// Try To Activate The Rendering Context
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Can't Activate The GL Rendering Context.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	ShowWindow(hWnd, SW_SHOW);						// Show The Window
	SetForegroundWindow(hWnd);						// Slightly Higher Priority
	SetFocus(hWnd);									// Sets Keyboard Focus To The Window
	ReSizeGLScene(width, height);					// Set Up Our Perspective GL Screen

	if (!InitGL())									// Initialize Our Newly Created GL Window
	{
		KillGLWindow();								// Reset The Display
		MessageBox(NULL, "Initialization Failed.", "ERROR", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;								// Return FALSE
	}

	return TRUE;									// Success
}

LRESULT CALLBACK WndProc(HWND	hWnd,			// Handle For This Window
	UINT	uMsg,			// Message For This Window
	WPARAM	wParam,			// Additional Message Information
	LPARAM	lParam)			// Additional Message Information
{
	switch (uMsg)									// Check For Windows Messages
	{
	case WM_ACTIVATE:							// Watch For Window Activate Message
	{
		if (!HIWORD(wParam))					// Check Minimization State
		{
			active = TRUE;						// Program Is Active
		}
		else
		{
			active = FALSE;						// Program Is No Longer Active
		}

		return 0;								// Return To The Message Loop
	}

	case WM_SYSCOMMAND:							// Intercept System Commands
	{
		switch (wParam)							// Check System Calls
		{
		case SC_SCREENSAVE:					// Screensaver Trying To Start?
		case SC_MONITORPOWER:				// Monitor Trying To Enter Powersave?
			return 0;							// Prevent From Happening
		}
		break;									// Exit
	}

	case WM_CLOSE:								// Did We Receive A Close Message?
	{
		PostQuitMessage(0);						// Send A Quit Message
		return 0;								// Jump Back
	}

	case WM_KEYDOWN:							// Is A Key Being Held Down?
	{
		keys[wParam] = TRUE;					// If So, Mark It As TRUE
		return 0;								// Jump Back
	}

	case WM_KEYUP:								// Has A Key Been Released?
	{
		keys[wParam] = FALSE;					// If So, Mark It As FALSE
		return 0;								// Jump Back
	}

	case WM_SIZE:								// Resize The OpenGL Window
	{
		ReSizeGLScene(LOWORD(lParam), HIWORD(lParam));  // LoWord=Width, HiWord=Height
		return 0;								// Jump Back
	}
	}

	// Pass All Unhandled Messages To DefWindowProc
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE	hInstance,			// Instance
	HINSTANCE	hPrevInstance,		// Previous Instance
	LPSTR		lpCmdLine,			// Command Line Parameters
	int			nCmdShow)			// Window Show State
{
	MSG		msg;									// Windows Message Structure
	BOOL	done = FALSE;								// Bool Variable To Exit Loop

	/*
	// Ask The User Which Screen Mode They Prefer
	if (MessageBox(NULL, "Would You Like To Run In Fullscreen Mode?", "Start FullScreen?", MB_YESNO | MB_ICONQUESTION) == IDNO)
	{
	fullscreen = FALSE;							// Windowed Mode
	}
	*/
	fullscreen = FALSE;

	// Create Our OpenGL Window
	if (!CreateGLWindow("LED Cube Simulation", 1280, 720, 16, fullscreen))
	{
		return 0;									// Quit If Window Was Not Created
	}

	while (!done)									// Loop That Runs While done=FALSE
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))	// Is There A Message Waiting?
		{
			if (msg.message == WM_QUIT)				// Have We Received A Quit Message?
			{
				done = TRUE;							// If So done=TRUE
			}
			else									// If Not, Deal With Window Messages
			{
				TranslateMessage(&msg);				// Translate The Message
				DispatchMessage(&msg);				// Dispatch The Message
			}
		}
		else										// If There Are No Messages
		{
			// Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			if ((active && !DrawGLScene()) || keys[VK_ESCAPE])	// Active?  Was There A Quit Received?
			{
				done = TRUE;							// ESC or DrawGLScene Signalled A Quit
			}
			else									// Not Time To Quit, Update Screen
			{
				//Logic Update Section
				SwapBuffers(hDC);					// Swap Buffers (Double Buffering)

				if (getCurrentTime() >= swapPatternTime)
				{
					setPattern();
				}

				if (keys[VK_RIGHT])
				{
					if (yrot <= -359.0f)
					{
						yrot = 0.0f;
					}
					yrot -= 1.0f;
				}
				if (keys[VK_LEFT])
				{
					if (yrot >= 359.0f)
					{
						yrot = 0.0f;
					}
					yrot += 1.0f;
				}
				if (keys[VK_UP])
				{
					if (lookupdown <= -359.0f)
					{
						lookupdown = 0.0f;
					}
					lookupdown -= 1.0f;
				}
				if (keys[VK_DOWN])
				{
					if (lookupdown >= 359.0f)
					{
						lookupdown = 0.0f;
					}
					lookupdown += 1.0f;
				}
				if (keys['S'])
				{
					diffrot = (yrot + yrot);
					xpos += (float)sin(yrot*piover180) * 0.075f;
					zpos += (float)cos(yrot*piover180) * 0.075f;

				}
				if (keys['W'])
				{
					diffrot = (yrot + yrot);
					xpos -= (float)sin(yrot*piover180) * 0.075f;
					zpos -= (float)cos(yrot*piover180) * 0.075f;
				}
				if (keys['A'])
				{
					diffrot = (yrot + 180.0f);
					xpos += (float)sin((yrot - 90.0f)*piover180) * 0.075f;
					zpos += (float)cos((yrot - 90.0f)*piover180) * 0.075f;
				}
				if (keys['D'])
				{
					diffrot = (-yrot - 180.0f);
					xpos -= (float)sin((yrot - 90.0f)*piover180) * 0.075f;
					zpos -= (float)cos((yrot - 90.0f)*piover180) * 0.075f;
				}
				/*
				if (keys[VK_SPACE])
				{
				currT = GetTickCount();
				if ((currT - lastShot) >= 2000)
				{
				if (screen == 1)
				{
				//P = shotUpdate(P);
				}
				}
				}
				*/
				if (keys[VK_RETURN])
				{
					if (screen == 0)
					{
						screen = 1;
					}
				}
				if (keys[VK_F1])						// Is F1 Being Pressed?
				{
					keys[VK_F1] = FALSE;					// If So Make Key FALSE
					KillGLWindow();						// Kill Our Current Window
					fullscreen = !fullscreen;				// Toggle Fullscreen / Windowed Mode
					// Recreate Our OpenGL Window
					if (!CreateGLWindow("Attack of the Evil Hydrogen and Oxygen Atoms!", 1280, 720, 16, fullscreen))
					{
						return 0;						// Quit If Window Was Not Created
					}
				}
			}
		}
	}

	// Shutdown
	KillGLWindow();									// Kill The Window
	return (msg.wParam);							// Exit The Program
}