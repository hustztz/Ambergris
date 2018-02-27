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
		void update(MoveStateFlag _state, float _delta);
		void getViewMtx(float* _viewMtx);
		void setPosition(const float* _pos);
		void getPosition(float* _pos) const;
		void setAt(const float* _at);
		void getAt(float* _at) const;
		void setVerticalAngle(float _verticalAngle);
		void updateVerticalAngle(float _delta) { m_verticalAngle += _delta; }
		void setHorizontalAngle(float _horizontalAngle);
		void updateHorizontalAngle(float _delta) { m_horizontalAngle += _delta; }
	protected:
		float m_eye[3];
		float m_at[3];
		float m_up[3];
		float m_horizontalAngle;
		float m_verticalAngle;
	};
}