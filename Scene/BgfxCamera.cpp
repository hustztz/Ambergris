#include "BgfxCamera.h"
#include "InteractiveDef.h"

#include <bx/timer.h>
#include <bx/math.h>

#define CAMERA_KEY_FORWARD   UINT8_C(0x01)
#define CAMERA_KEY_BACKWARD  UINT8_C(0x02)
#define CAMERA_KEY_LEFT      UINT8_C(0x04)
#define CAMERA_KEY_RIGHT     UINT8_C(0x08)
#define CAMERA_KEY_UP        UINT8_C(0x10)
#define CAMERA_KEY_DOWN      UINT8_C(0x20)

namespace ambergris {

	BgfxCamera::BgfxCamera()
	{
		reset();
		MouseState mouseState;
		update(0.0f, mouseState);
	}

	BgfxCamera::~BgfxCamera()
	{
	}

	void BgfxCamera::reset()
	{
		m_mouseNow.m_mx = 0;
		m_mouseNow.m_my = 0;
		m_mouseLast.m_mx = 0;
		m_mouseLast.m_my = 0;
		m_eye[0] = 0.0f;
		m_eye[1] = 0.0f;
		m_eye[2] = -35.0f;
		m_at[0] = 0.0f;
		m_at[1] = 0.0f;
		m_at[2] = -1.0f;
		m_up[0] = 0.0f;
		m_up[1] = 1.0f;
		m_up[2] = 0.0f;
		m_horizontalAngle = 0.01f;
		m_verticalAngle = 0.0f;
		m_mouseSpeed = 0.0020f;
		m_gamepadSpeed = 0.04f;
		m_moveSpeed = 30.0f;
		m_keys = 0;
		m_mouseDown = false;
	}

	void BgfxCamera::setKeyState(uint8_t _key, bool _down)
	{
		m_keys &= ~_key;
		m_keys |= _down ? _key : 0;
	}

	void BgfxCamera::update(float _deltaTime, const MouseState& _mouseState)
	{
		if (!m_mouseDown)
		{
			m_mouseLast.m_mx = _mouseState.m_mx;
			m_mouseLast.m_my = _mouseState.m_my;
		}

		m_mouseDown = !!_mouseState.m_buttons[MouseButton::Right];

		if (m_mouseDown)
		{
			m_mouseNow.m_mx = _mouseState.m_mx;
			m_mouseNow.m_my = _mouseState.m_my;
		}

		if (m_mouseDown)
		{
			int32_t deltaX = m_mouseNow.m_mx - m_mouseLast.m_mx;
			int32_t deltaY = m_mouseNow.m_my - m_mouseLast.m_my;

			m_horizontalAngle += m_mouseSpeed * float(deltaX);
			m_verticalAngle -= m_mouseSpeed * float(deltaY);

			m_mouseLast.m_mx = m_mouseNow.m_mx;
			m_mouseLast.m_my = m_mouseNow.m_my;
		}

		/*GamepadHandle handle = { 0 };
		m_horizontalAngle += m_gamepadSpeed * inputGetGamepadAxis(handle, GamepadAxis::RightX) / 32768.0f;
		m_verticalAngle -= m_gamepadSpeed * inputGetGamepadAxis(handle, GamepadAxis::RightY) / 32768.0f;
		const int32_t gpx = inputGetGamepadAxis(handle, GamepadAxis::LeftX);
		const int32_t gpy = inputGetGamepadAxis(handle, GamepadAxis::LeftY);
		m_keys |= gpx < -16834 ? CAMERA_KEY_LEFT : 0;
		m_keys |= gpx > 16834 ? CAMERA_KEY_RIGHT : 0;
		m_keys |= gpy < -16834 ? CAMERA_KEY_FORWARD : 0;
		m_keys |= gpy > 16834 ? CAMERA_KEY_BACKWARD : 0;*/

		float direction[3] =
		{
			bx::cos(m_verticalAngle) * bx::sin(m_horizontalAngle),
			bx::sin(m_verticalAngle),
			bx::cos(m_verticalAngle) * bx::cos(m_horizontalAngle),
		};

		float right[3] =
		{
			bx::sin(m_horizontalAngle - bx::kPiHalf),
			0,
			bx::cos(m_horizontalAngle - bx::kPiHalf),
		};

		float up[3];
		bx::vec3Cross(up, right, direction);

		if (m_keys & CAMERA_KEY_FORWARD)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, direction, _deltaTime * m_moveSpeed);

			bx::vec3Add(m_eye, pos, tmp);
			setKeyState(CAMERA_KEY_FORWARD, false);
		}

		if (m_keys & CAMERA_KEY_BACKWARD)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, direction, _deltaTime * m_moveSpeed);

			bx::vec3Sub(m_eye, pos, tmp);
			setKeyState(CAMERA_KEY_BACKWARD, false);
		}

		if (m_keys & CAMERA_KEY_LEFT)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, right, _deltaTime * m_moveSpeed);

			bx::vec3Add(m_eye, pos, tmp);
			setKeyState(CAMERA_KEY_LEFT, false);
		}

		if (m_keys & CAMERA_KEY_RIGHT)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, right, _deltaTime * m_moveSpeed);

			bx::vec3Sub(m_eye, pos, tmp);
			setKeyState(CAMERA_KEY_RIGHT, false);
		}

		if (m_keys & CAMERA_KEY_UP)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, up, _deltaTime * m_moveSpeed);

			bx::vec3Add(m_eye, pos, tmp);
			setKeyState(CAMERA_KEY_UP, false);
		}

		if (m_keys & CAMERA_KEY_DOWN)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, up, _deltaTime * m_moveSpeed);

			bx::vec3Sub(m_eye, pos, tmp);
			setKeyState(CAMERA_KEY_DOWN, false);
		}

		bx::vec3Add(m_at, m_eye, direction);
		bx::vec3Cross(m_up, right, direction);
	}

	void BgfxCamera::getViewMtx(float* _viewMtx)
	{
		bx::mtxLookAt(_viewMtx, m_eye, m_at, m_up);
	}

	void BgfxCamera::setPosition(const float* _pos)
	{
		bx::memCopy(m_eye, _pos, sizeof(float) * 3);
	}

	void BgfxCamera::setVerticalAngle(float _verticalAngle)
	{
		m_verticalAngle = _verticalAngle;
	}

	void BgfxCamera::setHorizontalAngle(float _horizontalAngle)
	{
		m_horizontalAngle = _horizontalAngle;
	}
}