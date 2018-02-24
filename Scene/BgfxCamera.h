#pragma once

#include "Foundation/Singleton.h"
#include <stdint.h>

namespace ambergris {

	struct MouseState;

	struct BgfxCamera
	{
		BgfxCamera();
		~BgfxCamera();

		void reset();
		void setKeyState(uint8_t _key, bool _down);
		void update(float _deltaTime, const MouseState& _mouseState);
		void getViewMtx(float* _viewMtx);
		void setPosition(const float* _pos);
		void setVerticalAngle(float _verticalAngle);
		void setHorizontalAngle(float _horizontalAngle);
	protected:
		struct MouseCoords
		{
			int32_t m_mx;
			int32_t m_my;
		};

		MouseCoords m_mouseNow;
		MouseCoords m_mouseLast;

		float m_eye[3];
		float m_at[3];
		float m_up[3];
		float m_horizontalAngle;
		float m_verticalAngle;

		float m_mouseSpeed;
		float m_gamepadSpeed;
		float m_moveSpeed;

		uint8_t m_keys;
		bool m_mouseDown;
	};
}