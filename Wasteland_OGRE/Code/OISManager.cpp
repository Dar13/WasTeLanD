#include "StdAfx.h"

#include "OISManager.h"

#include <OgreStringConverter.h>

template<> OISManager* Ogre::Singleton<OISManager>::ms_Singleton=0;

OISManager::OISManager(unsigned long windowHandle)
{
	//init some of the member variables
	_appShutdown=false;

	//Just keeping this window handle around. In case I have to reinitialize.
	_windowHandle = windowHandle;
	
	//Putting the window handle into OIS-terms.
	OIS::ParamList pl;
	pl.insert(OIS::ParamList::value_type("WINDOW",Ogre::StringConverter::toString(windowHandle)));

	//Create the OIS input objects.
	_inputSystem = OIS::InputManager::createInputSystem(pl);
	_mouseObj = static_cast<OIS::Mouse*>(_inputSystem->createInputObject(OIS::OISMouse,true));
	_keyObj = static_cast<OIS::Keyboard*>(_inputSystem->createInputObject(OIS::OISKeyboard,true));

	_mouseObj->setEventCallback(this); //this class is a mouseListener
	_keyObj->setEventCallback(this); //this class is a keyboardListener

	//set up the keycode stuff
	_KC_map[OIS::KC_A] = 'a';
	_KC_map[OIS::KC_B] = 'b';
	_KC_map[OIS::KC_C] = 'c';
	_KC_map[OIS::KC_D] = 'd';
	_KC_map[OIS::KC_E] = 'e';
	_KC_map[OIS::KC_F] = 'f';
	_KC_map[OIS::KC_G] = 'g';
	_KC_map[OIS::KC_H] = 'h';
	_KC_map[OIS::KC_I] = 'i';
	_KC_map[OIS::KC_J] = 'j';
	_KC_map[OIS::KC_K] = 'k';
	_KC_map[OIS::KC_L] = 'l';
	_KC_map[OIS::KC_M] = 'm';
	_KC_map[OIS::KC_N] = 'n';
	_KC_map[OIS::KC_O] = 'o';
	_KC_map[OIS::KC_P] = 'p';
	_KC_map[OIS::KC_Q] = 'q';
	_KC_map[OIS::KC_R] = 'r';
	_KC_map[OIS::KC_S] = 's';
	_KC_map[OIS::KC_T] = 't';
	_KC_map[OIS::KC_U] = 'u';
	_KC_map[OIS::KC_V] = 'v';
	_KC_map[OIS::KC_W] = 'w';
	_KC_map[OIS::KC_X] = 'x';
	_KC_map[OIS::KC_Y] = 'y';
	_KC_map[OIS::KC_Z] = 'z';
}


//Cleaning up.
OISManager::~OISManager()
{
	if(_mouseObj)
		_inputSystem->destroyInputObject(_mouseObj);

	if(_keyObj)
		_inputSystem->destroyInputObject(_keyObj);

	OIS::InputManager::destroyInputSystem(_inputSystem);
}

void OISManager::setCaptureWindow(int width,int height)
{
	const OIS::MouseState &ms = _mouseObj->getMouseState();
	ms.width=width;
	ms.height=height;
}

void OISManager::capture()
{
	_mouseObj->capture();
	_keyObj->capture();
}

bool OISManager::isCFGKeyPressed(unsigned int key)
{
	return _keyDown[key];
}

bool OISManager::keyPressed(const OIS::KeyEvent &evt)
{
	//Checking for escape-key press
	if(evt.key==OIS::KC_ESCAPE)
	{
		_appShutdown=true;
	}

	//Will eventually inject input into CEGUI/Bullet/etc.
	//injection into CEGUI...
	/*
	CEGUI::System::getSingleton().injectKeyDown(evt.key);
	CEGUI::System::getSingleton().injectChar(evt.text);
	*/

	char character = evt.text;
	for(unsigned int i = FORWARD; i<MAXVALUE; ++i)
	{
		if((_keyValues[i])[0] == character)
		{
			_keyDown[i] = true;	
		}
	}

	return true;
}

bool OISManager::keyReleased(const OIS::KeyEvent &evt)
{
	//Will eventually inject input into CEGUI/Bullet/etc.
	//injection into CEGUI...
	//CEGUI::System::getSingleton().injectKeyUp(evt.key);
	
	char character = getCharFromKeyCode(evt.key);
	for(unsigned int i = FORWARD; i<MAXVALUE; ++i)
	{
		if((_keyValues[i])[0] == character)
		{
			_keyDown[i] = false;
		}
	}

	return true;
}

bool OISManager::mouseMoved(const OIS::MouseEvent &evt)
{
	
	return true;
}

bool OISManager::mousePressed(const OIS::MouseEvent &evt,OIS::MouseButtonID id)
{
	return true;
}

bool OISManager::mouseReleased(const OIS::MouseEvent &evt,OIS::MouseButtonID id)
{
	return true;
}

void OISManager::setConfiguration(configuration_t* config)
{
	_config = config;
	_keyValues.clear();
	//follows the convention set in CONFIG_KEY_VALUES.
	//This can be added to, but CONFIG_KEY_VALUES must also be changed as well.
	_config->movement().forward()[0] = tolower(_config->movement().forward()[0]);
	_config->movement().backward()[0] = tolower(_config->movement().backward()[0]);
	_config->movement().right()[0] = tolower(_config->movement().right()[0]);
	_config->movement().left()[0] = tolower(_config->movement().left()[0]);
	_config->movement().sprint()[0] = tolower(_config->movement().sprint()[0]);
	_config->movement().jump()[0] = tolower(_config->movement().jump()[0]);
	_config->action().use()[0] = tolower(_config->action().use()[0]);
	_config->action().envwarnsys()[0] = tolower(_config->action().envwarnsys()[0]);
	_config->action().reload()[0] = tolower(_config->action().reload()[0]);
	_keyValues.push_back(_config->movement().forward()); //forward first
	_keyValues.push_back(_config->movement().backward());
	_keyValues.push_back(_config->movement().right());
	_keyValues.push_back(_config->movement().left());
	_keyValues.push_back(_config->movement().jump());
	_keyValues.push_back(_config->movement().sprint());
	_keyValues.push_back(_config->action().use());
	_keyValues.push_back(_config->action().envwarnsys());
	_keyValues.push_back(_config->action().reload()); //reload last.

	//just setting up this vector
	_keyDown.clear();
	for(unsigned int i = 0; i<MAXVALUE; ++i)
	{
		_keyDown.push_back(false);
	}
}

char OISManager::getCharFromKeyCode(unsigned int keyCode)
{
	return _KC_map[keyCode];
}