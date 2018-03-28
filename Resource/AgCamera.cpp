#include "AgCamera.h"

#include <bx/math.h>

#include <float.h>

namespace ambergris {

	AgCamera::AgCamera()
		: m_camFovy(60.0f)
	{
		reset();
	}

	AgCamera::~AgCamera()
	{
	}

	void AgCamera::reset()
	{
		m_eye.curr[0] = 0.0f;
		m_eye.curr[1] = 0.0f;
		m_eye.curr[2] = -35.0f;
		m_eye.dest[0] = 0.0f;
		m_eye.dest[1] = 0.0f;
		m_eye.dest[2] = -35.0f;
		m_target.curr[0] = 0.0f;
		m_target.curr[1] = 0.0f;
		m_target.curr[2] = -1.0f;
		m_target.dest[0] = 0.0f;
		m_target.dest[1] = 0.0f;
		m_target.dest[2] = -1.0f;
		m_up[0] = 0.0f;
		m_up[1] = 1.0f;
		m_up[2] = 0.0f;
		m_horizontalAngle = 0.01f;
		m_verticalAngle = 0.0f;
	}

	void AgCamera::move(MoveStateFlag _state, float _delta)
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
			bx::vec3Move(pos, m_eye.curr);

			float tmp[3];
			bx::vec3Mul(tmp, direction, _delta);

			bx::vec3Add(m_eye.curr, pos, tmp);
		}

		if (_state & E_MOVE_BACKWARD)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye.curr);

			float tmp[3];
			bx::vec3Mul(tmp, direction, _delta);

			bx::vec3Sub(m_eye.curr, pos, tmp);
		}

		if (_state & E_MOVE_LEFT)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye.curr);

			float tmp[3];
			bx::vec3Mul(tmp, right, _delta);

			bx::vec3Add(m_eye.curr, pos, tmp);
		}

		if (_state & E_MOVE_RIGHT)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye.curr);

			float tmp[3];
			bx::vec3Mul(tmp, right, _delta);

			bx::vec3Sub(m_eye.curr, pos, tmp);
		}

		if (_state & E_MOVE_UP)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye.curr);

			float tmp[3];
			bx::vec3Mul(tmp, up, _delta);

			bx::vec3Add(m_eye.curr, pos, tmp);
		}

		if (_state & E_MOVE_DOWN)
		{
			float pos[3];
			bx::vec3Move(pos, m_eye.curr);

			float tmp[3];
			bx::vec3Mul(tmp, up, _delta);

			bx::vec3Sub(m_eye.curr, pos, tmp);
		}

		bx::vec3Add(m_target.curr, m_eye.curr, direction);
		bx::vec3Cross(m_up, right, direction);
	}

	void AgCamera::orbit(float _dx, float _dy)
	{
		m_orbit[0] += _dx;
		m_orbit[1] += _dy;
	}

	void AgCamera::dolly(float _dz)
	{
		const float cnear = 0.01f;
		const float cfar = 10000.0f;

		const float toTarget[3] =
		{
			m_target.dest[0] - m_eye.dest[0],
			m_target.dest[1] - m_eye.dest[1],
			m_target.dest[2] - m_eye.dest[2],
		};
		const float toTargetLen = bx::vec3Length(toTarget);
		const float invToTargetLen = 1.0f / (toTargetLen + FLT_MIN);
		const float toTargetNorm[3] =
		{
			toTarget[0] * invToTargetLen,
			toTarget[1] * invToTargetLen,
			toTarget[2] * invToTargetLen,
		};

		float delta = toTargetLen*_dz;
		float newLen = toTargetLen + delta;
		if ((cnear < newLen || _dz < 0.0f)
			&& (newLen < cfar || _dz > 0.0f))
		{
			m_eye.dest[0] += toTargetNorm[0] * delta;
			m_eye.dest[1] += toTargetNorm[1] * delta;
			m_eye.dest[2] += toTargetNorm[2] * delta;
		}
	}

	void AgCamera::consumeOrbit(float _amount)
	{
		float consume[2];
		consume[0] = m_orbit[0] * _amount;
		consume[1] = m_orbit[1] * _amount;
		m_orbit[0] -= consume[0];
		m_orbit[1] -= consume[1];

		const float toEye[3] =
		{
			m_eye.curr[0] - m_target.curr[0],
			m_eye.curr[1] - m_target.curr[1],
			m_eye.curr[2] - m_target.curr[2],
		};
		const float toEyeLen = bx::vec3Length(toEye);
		const float invToEyeLen = 1.0f / (toEyeLen + FLT_MIN);
		const float toEyeNorm[3] =
		{
			toEye[0] * invToEyeLen,
			toEye[1] * invToEyeLen,
			toEye[2] * invToEyeLen,
		};

		float ll[2];
		bx::vec3ToLatLong(&ll[0], &ll[1], toEyeNorm);
		ll[0] += consume[0];
		ll[1] -= consume[1];
		ll[1] = bx::clamp(ll[1], 0.02f, 0.98f);

		float tmp[3];
		bx::vec3FromLatLong(tmp, ll[0], ll[1]);

		float diff[3];
		diff[0] = (tmp[0] - toEyeNorm[0])*toEyeLen;
		diff[1] = (tmp[1] - toEyeNorm[1])*toEyeLen;
		diff[2] = (tmp[2] - toEyeNorm[2])*toEyeLen;

		m_eye.curr[0] += diff[0];
		m_eye.curr[1] += diff[1];
		m_eye.curr[2] += diff[2];
		m_eye.dest[0] += diff[0];
		m_eye.dest[1] += diff[1];
		m_eye.dest[2] += diff[2];
	}

	void AgCamera::update(float _dt)
	{
		const float amount = bx::min(_dt / 0.12f, 1.0f);
		consumeOrbit(amount);

		m_target.curr[0] = bx::lerp(m_target.curr[0], m_target.dest[0], amount);
		m_target.curr[1] = bx::lerp(m_target.curr[1], m_target.dest[1], amount);
		m_target.curr[2] = bx::lerp(m_target.curr[2], m_target.dest[2], amount);
		m_eye.curr[0] = bx::lerp(m_eye.curr[0], m_eye.dest[0], amount);
		m_eye.curr[1] = bx::lerp(m_eye.curr[1], m_eye.dest[1], amount);
		m_eye.curr[2] = bx::lerp(m_eye.curr[2], m_eye.dest[2], amount);
	}

	void AgCamera::getViewMtx(float* _viewMtx) const
	{
		bx::mtxLookAt(_viewMtx, m_eye.curr, m_target.curr, m_up);
	}

	void AgCamera::getBackViewMtx(float* _viewMtx) const
	{
		float eye[3];
		eye[0] = - m_eye.curr[0];
		eye[1] = m_eye.curr[1];
		eye[2] = m_eye.curr[2];
		bx::mtxLookAt(_viewMtx, eye, m_target.curr, m_up);
	}

	void AgCamera::setPosition(const float* _pos)
	{
		bx::memCopy(m_eye.curr, _pos, sizeof(float) * 3);
		bx::memCopy(m_eye.dest, _pos, sizeof(float) * 3);
	}

	void AgCamera::getPosition(float* _pos) const
	{
		bx::memCopy(_pos, m_eye.curr, 3 * sizeof(float));
	}

	void AgCamera::setTarget(const float* _at)
	{
		bx::memCopy(m_target.curr, _at, sizeof(float) * 3);
		bx::memCopy(m_target.dest, _at, sizeof(float) * 3);
	}

	void AgCamera::getTarget(float* _at) const
	{
		bx::memCopy(_at, m_target.curr, 3 * sizeof(float));
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