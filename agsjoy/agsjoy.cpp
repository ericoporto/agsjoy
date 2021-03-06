﻿// agsjoy.cpp : Defines the exported functions for the DLL application.
//


#include "stdafx.h" 

#ifdef __linux__ 
#define LINUX_VERSION 1
#endif

#include <stdio.h>
#include <queue>          // std::queue

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
IAGSEditor *editor; // Editor interface
IAGSEngine* engine; // Engine interface

char const* joystructname = "joystick";


//------------------------------------------------------------
// BEGIN  joystick filter to handle multiple SDL poll event
//------------------------------------------------------------

//queue just for joystick events
std::queue<SDL_Event> joyEventQueue; 
int32 dummydata = 0;
static SDL_mutex *joyEventQueueMutex = nullptr;

bool isOtherSDL_PollEvents_Running = false;
 
// warning, this function could be called from another thread, so wrap in mutexes and do very little!!
static int joyEventWatch(void *userdata, SDL_Event *event) {
    if (event->type == SDL_JOYAXISMOTION || event->type == SDL_JOYBUTTONUP || event->type == SDL_JOYBUTTONDOWN || event->type == SDL_JOYHATMOTION ) {
        SDL_LockMutex(joyEventQueueMutex);
        joyEventQueue.push(*event);
        SDL_UnlockMutex(joyEventQueueMutex);
     }
}

SDL_Event getJoyEvent() {
	SDL_Event result; //  can't result = { .type = 0 } because MSVC dislikes it.
	result.type = 0;  // if 0, means there wasn't an event.

    SDL_LockMutex(joyEventQueueMutex);
    if (!joyEventQueue.empty()) {
        result = joyEventQueue.front();
        joyEventQueue.pop();
    }
    SDL_UnlockMutex(joyEventQueueMutex);
    
    return result;
}

void set_joy_event_watch() {
    joyEventQueueMutex = SDL_CreateMutex();
    SDL_AddEventWatch(joyEventWatch, nullptr);
}

//------------------------------------------------------------
// END  joystick filter to handle multiple SDL poll event
//------------------------------------------------------------


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
Joystick joyInAGS;

void joyreader::Unserialize(int key, const char *serializedData, int dataSize)
{
	engine->RegisterUnserializedObject(key, &joyInAGS, &joyintf);
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

		joyInAGS.button_count = SDL_JoystickNumButtons(sdljoy);

		// printf(" butts %i \n", joyInAGS.button_count);
		joyInAGS.axes_count = SDL_JoystickNumAxes(sdljoy);

		// printf(" axes %i \n", joyInAGS.axes_count);

		for (ax = 0; ax<16; ax = ax + 1)
		{
			joyInAGS.axes[ax] = 0;
		}

		for (b = 0; b<32; b = b + 1)
		{
			joyInAGS.buttstate[b] = SDL_RELEASED;
		}

		int AMAXINT = 0;//131072;

		joyInAGS.id = AMAXINT;

		joyInAGS.x = AMAXINT;
		joyInAGS.y = AMAXINT;
		joyInAGS.z = AMAXINT;
		joyInAGS.u = AMAXINT;
		joyInAGS.v = AMAXINT;
		joyInAGS.w = AMAXINT;
		joyInAGS.pov = AMAXINT;
		joyInAGS.buttons = AMAXINT;

		joy = &joyInAGS;
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
	// joyInAGS.buttstate[b] = SDL_RELEASED;
	//  }

	SDL_Event ev;
	
	if(!isOtherSDL_PollEvents_Running){
		while (SDL_PollEvent(&ev));
	}
	
	ev = getJoyEvent();

	while (ev.type != 0) {

		// printf("polling ");

		switch (ev.type) {

		case SDL_JOYAXISMOTION:
			joyInAGS.axes[ev.jaxis.axis] = ev.jaxis.value;

			// printf(" [i] moved axis %i \n", ev.jaxis.axis);

			break;

		case SDL_JOYBUTTONUP:
			joyInAGS.buttstate[ev.jbutton.button] = ev.jbutton.state;
			joyInAGS.buttons = joyInAGS.buttons & (4294967295 - (1 << ev.jbutton.button));

			// printf(" [i] released butt %i \n", ev.jbutton.button);

			break;

		case SDL_JOYBUTTONDOWN:
			joyInAGS.buttstate[ev.jbutton.button] = ev.jbutton.state;
			joyInAGS.buttons = joyInAGS.buttons | (1 << ev.jbutton.button);

			// printf(" [i] pressed butt %i \n", ev.jbutton.button);

			break;

		case SDL_JOYHATMOTION:
			// numbers below were reverse engineered to match joy POV identifiers
			if (ev.jhat.value == SDL_HAT_CENTERED) {
				joyInAGS.pov = 0;
			}
			else if (ev.jhat.value == SDL_HAT_DOWN) {
				joyInAGS.pov = 2 ^ 6;
			}
			else if (ev.jhat.value == SDL_HAT_LEFT) {
				joyInAGS.pov = 2 ^ 10;
			}
			else if (ev.jhat.value == SDL_HAT_RIGHT) {
				joyInAGS.pov = 2 ^ 0;
			}
			else if (ev.jhat.value == SDL_HAT_UP) {
				joyInAGS.pov = 2 ^ 3;
			}
			else if (ev.jhat.value == SDL_HAT_LEFTDOWN) {
				joyInAGS.pov = 2 ^ 14;
			}
			else if (ev.jhat.value == SDL_HAT_RIGHTDOWN) {
				joyInAGS.pov = 2 ^ 4;
			}
			else if (ev.jhat.value == SDL_HAT_LEFTUP) {
				joyInAGS.pov = 2 ^ 11;
			}
			else if (ev.jhat.value == SDL_HAT_RIGHTUP) {
				joyInAGS.pov = 2 ^ 1;
			}

			break;


		}

		ev = getJoyEvent();
	}

}

int Joystick_GetAxis(Joystick* joy, int axis)
{
	//printf(" [i] joystick getaxis %i \n", axis);
	updjoy(&joyInAGS);

	if (axis>/*16*/joy->axes_count || axis<0) { return 0; }
	//printf("checking axis %i / %i \n", axis, joy->axes_count);

	//return SDL_JoystickGetAxis(sdljoy, axis);
	return joyInAGS.axes[axis];
}

// thiscall 

int Joystick_IsButtonDown(Joystick* joy, int butt)
{

	// printf(" [i] joystick isbuttondown %i \n", butt);
	updjoy(&joyInAGS);

	if (butt>32 || butt<0) { return 0; }

	if (joyInAGS.buttstate[butt] == SDL_PRESSED)
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


//------------------------------------------------------------
// Engine stuff
//------------------------------------------------------------

void AGS_EngineStartup(IAGSEngine *lpEngine)
{
  Uint32 subsystem_init;

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

  subsystem_init = SDL_WasInit(SDL_INIT_EVERYTHING);

  if (subsystem_init != 0) {  
	  isOtherSDL_PollEvents_Running = true;
  } else {
	  isOtherSDL_PollEvents_Running = false;
  }

	SDL_Init(SDL_INIT_JOYSTICK);
	set_joy_event_watch();

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


//------------------------------------------------------------
// Editor stuff
//------------------------------------------------------------

const char* AGS_GetPluginName(void)
{
  // Return the plugin description
  return "agsjoy";
}

void AGS_EditorLoadGame(char *buffer, int bufsize)
{
    // Nothing to load for this plugin
}

void AGS_EditorProperties(HWND parent)
{
    // Nothing here, but could show up a message box
}

int AGS_EditorSaveGame(char *buffer, int bufsize)
{
    // We don't want to save any persistent data
    return 0;
}

const char* scriptHeader =
"/// Returns the number of gamecontrollers found\r\n"
"import int JoystickCount (); \r\n"
"\r\n"
"/// Scans for newly installed gamecontrollers. Retuns true when found\r\n"
"import bool JoystickRescan (); \r\n"
"\r\n"
"/// Returns the   name of the specified gamecontroller. (0-15)\r\n"
"import String JoystickName (int ID);\r\n"
"\r\n"
"enum JoystickPOV {\r\n"
"  ePOVCenter = 0,\r\n"
"  ePOVUp = 1,\r\n"
"  ePOVRight = 2,\r\n"
"  ePOVUpRight = 3,\r\n"
"  ePOVDown = 4,\r\n"
"  ePOVDownRight = 6,\r\n"
"  ePOVLeft = 8,\r\n"
"  ePOVUpLeft = 9,\r\n"
"  ePOVDownLeft = 12\r\n"
"};\r\n"
"#define JOY_RANGE 32768\r\n"
"\r\n"
"managed struct Joystick {\r\n"
"readonly int ID;\r\n"
"readonly int ButtonCount;\r\n"
"readonly int AxesCount;\r\n"
"readonly int x;\r\n"
"readonly int y;\r\n"
"readonly int z;\r\n"
"readonly int u;\r\n"
"readonly int v;\r\n"
"readonly int w;\r\n"
"readonly JoystickPOV POV;\r\n"
"readonly int buttons; // $AUTOCOMPLETEIGNORE$\r\n"
"\r\n"
"/// Opens specified controller. (0-15)\r\n"
"import static Joystick* Open (int ID); // $AUTOCOMPLETESTATICONLY$\r\n"
"\r\n"
"/// Checks if specified controller has been opened. (0-15)\r\n"
"import static bool IsOpen (int ID); // $AUTOCOMPLETESTATICONLY$\r\n"
"\r\n"
"/// Simulates a mouseclick at the current cursor position\r\n"
"import static void Click (MouseButton button);\r\n"
"\r\n"
"/// Closes controller\r\n"
"import void Close ();\r\n"
"\r\n"
"/// Returns whether the controller is valid. (use this when loading save games)\r\n"
"import bool Valid ();\r\n"
"\r\n"
"/// Returns if the controller is currently unplugged\r\n"
"import bool Unplugged ();\r\n"
"\r\n"
"/// Returns the controller name\r\n"
"import String GetName ();\r\n"
"\r\n"
"/// Returns axis value bynumber. (0-5)\r\n"
"import int GetAxis (int axis);\r\n"
"\r\n"
"/// Returns true when the specified button is currently down. (0-31)\r\n"
"import bool IsButtonDown (int button);\r\n"
"\r\n"
"/// Forces an update of the controller axis, button and pov state\r\n"
"import void Update ();\r\n"
"\r\n"
"/// Enables events on axis move, button press and pov. (0 = global (default), 1 = room)\r\n"
"import void EnableEvents (int scope = 0);\r\n"
"\r\n"
"/// Disable events. (disabled by default)\r\n"
"import void DisableEvents ();\r\n"
"\r\n"
"};\r\n"
  ;

int  AGS_EditorStartup(IAGSEditor* lpEditor)
{
  // User has checked the plugin to use it in their game

  // If it's an earlier version than what we need, abort.
  if (lpEditor->version < 1)
    return -1;

  editor = lpEditor;
  editor->RegisterScriptHeader(scriptHeader);

  // Return 0 to indicate success
  return 0;
}

void AGS_EditorShutdown()
{
    // User has un-checked the plugin, unregister the header!
    if (scriptHeader != nullptr) editor->UnregisterScriptHeader(scriptHeader);
}


