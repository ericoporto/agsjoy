// agsjoy.cpp : Defines the exported functions for the DLL application.
//


#include "stdafx.h" 

#ifdef __linux__ 
#define LINUX_VERSION 1
#endif

#include <stdio.h>

#define THIS_IS_THE_PLUGIN
#include "agsplugin.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_joystick.h>

struct Joystick
{
	// Exposed: <<<DO NOT CHANGE THE ORDER!!!>>>
	int32 id;
	int32 button_count;
	int32 axes_count;
	int32 x, y, z, u, v, w;
	int32 pov;
	unsigned int32 buttons;

	// Internal:
	// int events;
	// JoyState *state;

	int buttstate[32];
	int axes[16];

};

// ags stuff

IAGSEngine* engine;

char const* joystructname = "joystick";

int32 dummydata = 0;

class joyinterface : public IAGSScriptManagedObject {
public:
	// when a ref count reaches 0, this is called with the address
	// of the object. Return 1 to remove the object from memory, 0 to
	// leave it
	virtual int Dispose(const char *address, bool force);
	// return the type name of the object
	virtual const char *GetType();
	// serialize the object into BUFFER (which is BUFSIZE bytes)
	// return number of bytes used
	virtual int Serialize(const char *address, char *buffer, int bufsize);
};

class joyreader : public IAGSManagedObjectReader {
public:
	virtual void Unserialize(int key, const char *serializedData, int dataSize);
};

int joyinterface::Dispose(const char *address, bool force)
{
	// delete ((Joystick*) address);
	return 0; //1;
}

const char* joyinterface::GetType()
{
	return joystructname;
}

int joyinterface::Serialize(const char *address, char *buffer, int bufsize)
{
	// put 1 byte there
	memcpy(buffer, &dummydata, sizeof(dummydata));
	return sizeof(dummydata);
}

joyinterface joyintf;
joyreader joyread;
Joystick theJoy;

void joyreader::Unserialize(int key, const char *serializedData, int dataSize)
{
	engine->RegisterUnserializedObject(key, &theJoy, &joyintf);
}



//



int MaxJoysticks = 0;
int ControllerIndex = 0;
int openedthejoy = 0;

Joystick dummyJoy;
SDL_Joystick* sdljoy;

int JoystickCount()
{
	//printf(" [i] joystick count \n");
	// printf ("  [i] joystick count %i \n", SDL_NumJoysticks(), "! \n");

	return SDL_NumJoysticks();
}

void JoystickRescan()
{

	// printf(" [i] joystick rescan \n");
}

Joystick* Joystick_Open(int joy_num)
{
	// printf(" [i] joystick open %i \n", joy_num);
	Joystick* joy;
	int ax;
	int b;

	if (joy_num == -1)
	{
		// dummyJoy = new Joystick;
		// return dummyJoy;
		joy = &dummyJoy;
	}
	else
	{
		if (openedthejoy == 0)
		{
			sdljoy = SDL_JoystickOpen(0);
			openedthejoy = 1;
		}

		theJoy.button_count = SDL_JoystickNumButtons(sdljoy);

		// printf(" butts %i \n", theJoy.button_count);
		theJoy.axes_count = SDL_JoystickNumAxes(sdljoy);

		// printf(" axes %i \n", theJoy.axes_count);

		for (ax = 0; ax<16; ax = ax + 1)
		{
			theJoy.axes[ax] = 0;
		}

		for (b = 0; b<32; b = b + 1)
		{
			theJoy.buttstate[b] = SDL_RELEASED;
		}

		int AMAXINT = 0;//131072;

		theJoy.id = AMAXINT;

		theJoy.x = AMAXINT;
		theJoy.y = AMAXINT;
		theJoy.z = AMAXINT;
		theJoy.u = AMAXINT;
		theJoy.v = AMAXINT;
		theJoy.w = AMAXINT;
		theJoy.pov = AMAXINT;
		theJoy.buttons = AMAXINT;

		joy = &theJoy;
	}

	//if (openedthejoy==0)
	//{
	engine->RegisterManagedObject(joy, &joyintf);

	//  }

	return joy;
}

void Joystick_Close(Joystick* joy)
{
	// printf("close joystick \n");
	SDL_JoystickClose(0);
}

void Joystick_Click(Joystick* joy, int MouseMode)
{
	engine->SimulateMouseClick(1);
}

int Joystick_Valid(Joystick* joy)
{

	// printf(" [i] joystick valid \n");

	return 1;
}

int Joystick_Unplugged(Joystick* joy)
{

	// printf(" [i] joystick unplugged \n");

	return 0;
}

void updjoy(Joystick* joy)
{
	//printf("what the \n");
	int ax;
	int b;

	//  for (b=0; b<32; b = b + 1)
	//  {
	// theJoy.buttstate[b] = SDL_RELEASED;
	//  }

	SDL_Event ev;

	while (SDL_PollEvent(&ev)) {

		// printf("polling ");

		switch (ev.type) {

		case SDL_JOYAXISMOTION:
			theJoy.axes[ev.jaxis.axis] = ev.jaxis.value;

			// printf(" [i] moved axis %i \n", ev.jaxis.axis);

			break;

		case SDL_JOYBUTTONUP:
			theJoy.buttstate[ev.jbutton.button] = ev.jbutton.state;
			theJoy.buttons = theJoy.buttons & (4294967295 - 1 >> ev.jbutton.button);

			// printf(" [i] released butt %i \n", ev.jbutton.button);

			break;

		case SDL_JOYBUTTONDOWN:
			theJoy.buttstate[ev.jbutton.button] = ev.jbutton.state;
			theJoy.buttons = theJoy.buttons | (1 >> ev.jbutton.button);

			// printf(" [i] pressed butt %i \n", ev.jbutton.button);

			break;

		case SDL_JOYHATMOTION:
			// numbers below were reverse engineered to match joy POV identifiers
			if (ev.jhat.value == SDL_HAT_CENTERED) {
				theJoy.pov = 0;
			}
			else if (ev.jhat.value == SDL_HAT_DOWN) {
				theJoy.pov = 2 ^ 6;
			}
			else if (ev.jhat.value == SDL_HAT_LEFT) {
				theJoy.pov = 2 ^ 10;
			}
			else if (ev.jhat.value == SDL_HAT_RIGHT) {
				theJoy.pov = 2 ^ 0;
			}
			else if (ev.jhat.value == SDL_HAT_UP) {
				theJoy.pov = 2 ^ 3;
			}
			else if (ev.jhat.value == SDL_HAT_LEFTDOWN) {
				theJoy.pov = 2 ^ 14;
			}
			else if (ev.jhat.value == SDL_HAT_RIGHTDOWN) {
				theJoy.pov = 2 ^ 4;
			}
			else if (ev.jhat.value == SDL_HAT_LEFTUP) {
				theJoy.pov = 2 ^ 11;
			}
			else if (ev.jhat.value == SDL_HAT_RIGHTUP) {
				theJoy.pov = 2 ^ 1;
			}

			break;


		}

	}

}

int Joystick_GetAxis(Joystick* joy, int axis)
{
	//printf(" [i] joystick getaxis %i \n", axis);
	updjoy(&theJoy);

	if (axis>/*16*/joy->axes_count || axis<0) { return 0; }
	//printf("checking axis %i / %i \n", axis, joy->axes_count);

	//return SDL_JoystickGetAxis(sdljoy, axis);
	return theJoy.axes[axis];
}

// thiscall 

int Joystick_IsButtonDown(Joystick* joy, int butt)
{

	// printf(" [i] joystick isbuttondown %i \n", butt);
	updjoy(&theJoy);

	if (butt>32 || butt<0) { return 0; }

	if (theJoy.buttstate[butt] == SDL_PRESSED)
	{
		//printf("pressing butts %i \n", butt);
		// printf("pressing butts %i / %i \n", butt, joy->button_count);
		return 1;
	}
	else
	{
		//printf("disappointed at %i \n", butt);
		return 0;
	}
	//return SDL_JoystickGetButton(sdljoy, butt);
}

const char* Joystick_GetName(Joystick* joy)
{
	// return SDL_JoystickName(sdljoy);

	//printf(" joyname %s \n",SDL_JoystickName(sdljoy));
	if (openedthejoy == 0) {
		return engine->CreateScriptString("disconnected");
	}
	else if (SDL_JoystickName(sdljoy) != NULL) {
		return engine->CreateScriptString(SDL_JoystickName(sdljoy));
	}
	else {
		return engine->CreateScriptString("");
	}
}

///



void AGS_EngineStartup(IAGSEngine *lpEngine)
{

	/*
	expected entry points from AGS in ags/Engine/plugin/global_plugin.cpp

	ccAddExternalStaticFunction("JoystickCount",                Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("JoystickName",                 Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("JoystickRescan",               Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::Open^1",             Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::IsOpen^1",           Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::Click^1",            Sc_PluginStub_Void);
	ccAddExternalStaticFunction("Joystick::Close^0",            Sc_PluginStub_Void);
	ccAddExternalStaticFunction("Joystick::Valid^0",            Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::Unplugged^0",        Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::GetName^0",          Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::GetAxis^1",          Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::IsButtonDown^1",     Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::IsJoyBtnDown^1",     Sc_PluginStub_Int0);
	ccAddExternalStaticFunction("Joystick::Update^0",           Sc_PluginStub_Void);
	ccAddExternalStaticFunction("Joystick::DisableEvents^0",    Sc_PluginStub_Void);
	ccAddExternalStaticFunction("Joystick::EnableEvents^1",     Sc_PluginStub_Void);
	*/

	engine = lpEngine;

	engine->RegisterScriptFunction("JoystickCount", (void*)&JoystickCount);
	engine->RegisterScriptFunction("JoystickRescan", (void*)&JoystickRescan);
	engine->RegisterScriptFunction("Joystick::Open", (void*)&Joystick_Open);
	engine->RegisterScriptFunction("Joystick::Click", (void*)&Joystick_Click);
	engine->RegisterScriptFunction("Joystick::Valid", (void*)&Joystick_Valid);
	engine->RegisterScriptFunction("Joystick::Unplugged", (void*)&Joystick_Unplugged);
	engine->RegisterScriptFunction("Joystick::GetAxis", (void*)&Joystick_GetAxis);
	engine->RegisterScriptFunction("Joystick::IsButtonDown", (void*)&Joystick_IsButtonDown);
	engine->RegisterScriptFunction("Joystick::Close", (void*)&Joystick_Close);
	engine->RegisterScriptFunction("Joystick::GetName^0", reinterpret_cast<void *>(Joystick_GetName));

	// gamepad init  
	/*
	SDL_Init ( SDL_INIT_GAMECONTROLLER );

	#define MAX_CONTROLLERS 32
	SDL_GameController *ControllerHandles[MAX_CONTROLLERS];

	MaxJoysticks = SDL_NumJoysticks();

	for(int JoystickIndex=0; JoystickIndex < MaxJoysticks; ++JoystickIndex)
	{
	if (!SDL_IsGameController(JoystickIndex))
	{
	continue;
	}
	if (ControllerIndex >= MAX_CONTROLLERS)
	{
	break;
	}
	ControllerHandles[ControllerIndex] = SDL_GameControllerOpen(JoystickIndex);
	ControllerIndex++;
	}
	*/
	engine->AddManagedObjectReader(joystructname, &joyread);

	SDL_Init(SDL_INIT_JOYSTICK);

}

void AGS_EngineShutdown()
{

	SDL_Quit();
}

int AGS_EngineOnEvent(int event, int data)
{
	return 0;
}

int AGS_EngineDebugHook(const char *scriptName, int lineNum, int reserved)
{
	return 0;
}

void AGS_EngineInitGfx(const char *driverID, void *data)
{
}
