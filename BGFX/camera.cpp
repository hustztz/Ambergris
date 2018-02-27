/*
 * Copyright 2013 Dario Manesku. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx#license-bsd-2-clause
 */

#include <bx/timer.h>
#include "Scene/AgCamera.h"
#include "camera.h"
#include "entry/entry.h"
#include "entry/cmd.h"
#include "entry/input.h"
#include <bx/allocator.h>

int cmdMove(CmdContext* /*_context*/, void* /*_userData*/, int _argc, char const* const* _argv)
{
	if (_argc > 1)
	{
		if (0 == bx::strCmp(_argv[1], "forward") )
		{
			cameraSetKeyState(CAMERA_KEY_FORWARD, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "left") )
		{
			cameraSetKeyState(CAMERA_KEY_LEFT, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "right") )
		{
			cameraSetKeyState(CAMERA_KEY_RIGHT, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "backward") )
		{
			cameraSetKeyState(CAMERA_KEY_BACKWARD, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "up") )
		{
			cameraSetKeyState(CAMERA_KEY_UP, true);
			return 0;
		}
		else if (0 == bx::strCmp(_argv[1], "down") )
		{
			cameraSetKeyState(CAMERA_KEY_DOWN, true);
			return 0;
		}
	}

	return 1;
}

static void cmd(const void* _userData)
{
	cmdExec( (const char*)_userData);
}

static const InputBinding s_camBindings[] =
{
	{ entry::Key::KeyW,             entry::Modifier::None, 0, cmd, "move forward"  },
	{ entry::Key::GamepadUp,        entry::Modifier::None, 0, cmd, "move forward"  },
	{ entry::Key::KeyA,             entry::Modifier::None, 0, cmd, "move left"     },
	{ entry::Key::GamepadLeft,      entry::Modifier::None, 0, cmd, "move left"     },
	{ entry::Key::KeyS,             entry::Modifier::None, 0, cmd, "move backward" },
	{ entry::Key::GamepadDown,      entry::Modifier::None, 0, cmd, "move backward" },
	{ entry::Key::KeyD,             entry::Modifier::None, 0, cmd, "move right"    },
	{ entry::Key::GamepadRight,     entry::Modifier::None, 0, cmd, "move right"    },
	{ entry::Key::KeyQ,             entry::Modifier::None, 0, cmd, "move down"     },
	{ entry::Key::GamepadShoulderL, entry::Modifier::None, 0, cmd, "move down"     },
	{ entry::Key::KeyE,             entry::Modifier::None, 0, cmd, "move up"       },
	{ entry::Key::GamepadShoulderR, entry::Modifier::None, 0, cmd, "move up"       },

	INPUT_BINDING_END
};

struct Camera
{
	struct MouseCoords
	{
		int32_t m_mx;
		int32_t m_my;
	};

	Camera()
	{
		reset();
		entry::MouseState mouseState;
		update(0.0f, mouseState);

		cmdAdd("move", cmdMove);
		inputAddBindings("camBindings", s_camBindings);
	}

	~Camera()
	{
		inputRemoveBindings("camBindings");
	}

	void reset()
	{
		m_mouseNow.m_mx  = 0;
		m_mouseNow.m_my  = 0;
		m_mouseLast.m_mx = 0;
		m_mouseLast.m_my = 0;
		m_mouseSpeed = 0.0020f;
		m_gamepadSpeed = 0.04f;
		m_moveSpeed = 100.0f;
		m_keys = 0;
		m_mouseDown = false;
		m_camera.reset();
	}

	void setKeyState(uint8_t _key, bool _down)
	{
		m_keys &= ~_key;
		m_keys |= _down ? _key : 0;
	}

	void update(float _deltaTime, const entry::MouseState& _mouseState)
	{
		if (!m_mouseDown)
		{
			m_mouseLast.m_mx = _mouseState.m_mx;
			m_mouseLast.m_my = _mouseState.m_my;
		}

		m_mouseDown = !!_mouseState.m_buttons[entry::MouseButton::Right];

		if (m_mouseDown)
		{
			m_mouseNow.m_mx = _mouseState.m_mx;
			m_mouseNow.m_my = _mouseState.m_my;
		}

		if (m_mouseDown)
		{
			int32_t deltaX = m_mouseNow.m_mx - m_mouseLast.m_mx;
			int32_t deltaY = m_mouseNow.m_my - m_mouseLast.m_my;

			m_camera.updateHorizontalAngle( m_mouseSpeed * float(deltaX) );
			m_camera.updateVerticalAngle( - m_mouseSpeed * float(deltaY));

			m_mouseLast.m_mx = m_mouseNow.m_mx;
			m_mouseLast.m_my = m_mouseNow.m_my;
		}

		entry::GamepadHandle handle = { 0 };
		m_camera.updateHorizontalAngle( m_gamepadSpeed * inputGetGamepadAxis(handle, entry::GamepadAxis::RightX) / 32768.0f );
		m_camera.updateVerticalAngle( - m_gamepadSpeed * inputGetGamepadAxis(handle, entry::GamepadAxis::RightY) / 32768.0f );
		const int32_t gpx = inputGetGamepadAxis(handle, entry::GamepadAxis::LeftX);
		const int32_t gpy = inputGetGamepadAxis(handle, entry::GamepadAxis::LeftY);
		m_keys |= gpx < -16834 ? CAMERA_KEY_LEFT : 0;
		m_keys |= gpx > 16834 ? CAMERA_KEY_RIGHT : 0;
		m_keys |= gpy < -16834 ? CAMERA_KEY_FORWARD : 0;
		m_keys |= gpy > 16834 ? CAMERA_KEY_BACKWARD : 0;

		int _camera_state = 0;
		if (m_keys & CAMERA_KEY_FORWARD)
		{
			_camera_state |= ambergris::AgCamera::E_MOVE_FORWARD;
		}
		if (m_keys & CAMERA_KEY_BACKWARD)
		{
			_camera_state |= ambergris::AgCamera::E_MOVE_BACKWARD;
		}
		if (m_keys & CAMERA_KEY_LEFT)
		{
			_camera_state |= ambergris::AgCamera::E_MOVE_LEFT;
		}
		if (m_keys & CAMERA_KEY_RIGHT)
		{
			_camera_state |= ambergris::AgCamera::E_MOVE_RIGHT;
		}
		if (m_keys & CAMERA_KEY_UP)
		{
			_camera_state |= ambergris::AgCamera::E_MOVE_UP;
		}
		if (m_keys & CAMERA_KEY_DOWN)
		{
			_camera_state |= ambergris::AgCamera::E_MOVE_DOWN;
		}

		m_camera.update((ambergris::AgCamera::MoveStateFlag)_camera_state, _deltaTime * m_moveSpeed);

		if (m_keys & CAMERA_KEY_FORWARD)
		{
			setKeyState(CAMERA_KEY_FORWARD, false);
		}
		if (m_keys & CAMERA_KEY_BACKWARD)
		{
			setKeyState(CAMERA_KEY_BACKWARD, false);
		}
		if (m_keys & CAMERA_KEY_LEFT)
		{
			setKeyState(CAMERA_KEY_LEFT, false);
		}
		if (m_keys & CAMERA_KEY_RIGHT)
		{
			setKeyState(CAMERA_KEY_RIGHT, false);
		}
		if (m_keys & CAMERA_KEY_UP)
		{
			setKeyState(CAMERA_KEY_UP, false);
		}
		if (m_keys & CAMERA_KEY_DOWN)
		{
			setKeyState(CAMERA_KEY_DOWN, false);
		}
	}

	void getViewMtx(float* _viewMtx)
	{
		m_camera.getViewMtx(_viewMtx);
	}

	void setPosition(const float* _pos)
	{
		m_camera.setPosition(_pos);
	}

	void setFocus(const float* _pos)
	{
		m_camera.setAt(_pos);
	}

	void setVerticalAngle(float _verticalAngle)
	{
		m_camera.setVerticalAngle(_verticalAngle);
	}

	void setHorizontalAngle(float _horizontalAngle)
	{
		m_camera.setHorizontalAngle(_horizontalAngle);
	}

	MouseCoords m_mouseNow;
	MouseCoords m_mouseLast;

	ambergris::AgCamera m_camera;

	float m_mouseSpeed;
	float m_gamepadSpeed;
	float m_moveSpeed;

	uint8_t m_keys;
	bool m_mouseDown;
};

static Camera* s_camera = NULL;

void cameraCreate()
{
	s_camera = BX_NEW(entry::getAllocator(), Camera);
}

void cameraDestroy()
{
	BX_DELETE(entry::getAllocator(), s_camera);
	s_camera = NULL;
}

void cameraSetPosition(const float* _pos)
{
	s_camera->setPosition(_pos);
}

void cameraSetFocus(const float* _pos)
{
	s_camera->setFocus(_pos);
}

void cameraSetHorizontalAngle(float _horizontalAngle)
{
	s_camera->setHorizontalAngle(_horizontalAngle);
}

void cameraSetVerticalAngle(float _verticalAngle)
{
	s_camera->setVerticalAngle(_verticalAngle);
}

void cameraSetKeyState(uint8_t _key, bool _down)
{
	s_camera->setKeyState(_key, _down);
}

void cameraGetViewMtx(float* _viewMtx)
{
	s_camera->getViewMtx(_viewMtx);
}

void cameraGetPosition(float* _pos)
{
	s_camera->m_camera.getPosition(_pos);
}

void cameraGetAt(float* _at)
{
	s_camera->m_camera.getAt(_at);
}

void cameraUpdate(float _deltaTime, const entry::MouseState& _mouseState)
{
	s_camera->update(_deltaTime, _mouseState);
}
