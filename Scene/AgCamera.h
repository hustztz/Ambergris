#pragma once

namespace ambergris {

	struct AgCamera
	{
		enum MoveStateFlag
		{
			E_MOVE_FORWARD	= 1 << 0,
			E_MOVE_BACKWARD	= 1 << 1,
			E_MOVE_LEFT		= 1 << 2,
			E_MOVE_RIGHT		= 1 << 3,
			E_MOVE_UP		= 1 << 4,
			E_MOVE_DOWN		= 1 << 5,
		};
		AgCamera();
		~AgCamera();

		void reset();
		void move(MoveStateFlag _state, float _delta);
		void orbit(float _dx, float _dy);
		void dolly(float _dz);
		void consumeOrbit(float _amount);
		void update(float _dt);

		void getViewMtx(float* _viewMtx);
		void getBackViewMtx(float* _viewMtx);
		void setPosition(const float* _pos);
		void getPosition(float* _pos) const;
		void setTarget(const float* _at);
		void getTarget(float* _at) const;
		void setVerticalAngle(float _verticalAngle);
		void updateVerticalAngle(float _delta) { m_verticalAngle += _delta; }
		void setHorizontalAngle(float _horizontalAngle);
		void updateHorizontalAngle(float _delta) { m_horizontalAngle += _delta; }
	protected:
		struct Interp3f
		{
			float curr[3];
			float dest[3];
		};
		Interp3f m_eye;
		Interp3f m_target;
		float m_up[3];
		float m_orbit[2];
		float m_horizontalAngle;
		float m_verticalAngle;
	};
}