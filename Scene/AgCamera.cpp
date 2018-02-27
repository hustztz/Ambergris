#include "AgCamera.h"

#include <bx/math.h>

namespace ambergris {

	AgCamera::AgCamera()
	{
		reset();
	}

	AgCamera::~AgCamera()
	{
	}

	void AgCamera::reset()
	{
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
	}

	void AgCamera::update(MoveStateFlag _state, float _delta)
	{
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

		if (_state & E_MOVE_FORWARD)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, direction, _delta);

			bx::vec3Add(m_eye, pos, tmp);
		}

		if (_state & E_MOVE_BACKWARD)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, direction, _delta);

			bx::vec3Sub(m_eye, pos, tmp);
		}

		if (_state & E_MOVE_LEFT)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, right, _delta);

			bx::vec3Add(m_eye, pos, tmp);
		}

		if (_state & E_MOVE_RIGHT)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, right, _delta);

			bx::vec3Sub(m_eye, pos, tmp);
		}

		if (_state & E_MOVE_UP)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, up, _delta);

			bx::vec3Add(m_eye, pos, tmp);
		}

		if (_state & E_MOVE_DOWN)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye);

			float tmp[3];
			bx::vec3Mul(tmp, up, _delta);

			bx::vec3Sub(m_eye, pos, tmp);
		}

		bx::vec3Add(m_at, m_eye, direction);
		bx::vec3Cross(m_up, right, direction);
	}

	void AgCamera::getViewMtx(float* _viewMtx)
	{
		bx::mtxLookAt(_viewMtx, m_eye, m_at, m_up);
	}

	void AgCamera::setPosition(const float* _pos)
	{
		bx::memCopy(m_eye, _pos, sizeof(float) * 3);
	}

	void AgCamera::getPosition(float* _pos) const
	{
		bx::memCopy(_pos, m_eye, 3 * sizeof(float));
	}

	void AgCamera::setAt(const float* _at)
	{
		bx::memCopy(m_at, _at, sizeof(float) * 3);
	}

	void AgCamera::getAt(float* _at) const
	{
		bx::memCopy(_at, m_at, 3 * sizeof(float));
	}

	void AgCamera::setVerticalAngle(float _verticalAngle)
	{
		m_verticalAngle = _verticalAngle;
	}

	void AgCamera::setHorizontalAngle(float _horizontalAngle)
	{
		m_horizontalAngle = _horizontalAngle;
	}
}