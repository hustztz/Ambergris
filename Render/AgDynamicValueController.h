#pragma once

#include <map>
#include <bx/math.h>

namespace ambergris {

	// Performs piecewise linear interpolation of a Color parameter.
	class AgDynamicValueController
	{
	public:
		// Represents color. Color-space depends on context.
		// In the code below, used to represent color in XYZ, and RGB color-space
		union Color
		{
			struct {
				float X;
				float Y;
				float Z;
			};
			struct {
				float r;
				float g;
				float b;
			};

			float data[3];
		};
		
		// Converts color repesentation from CIE XYZ to RGB color-space.
		static Color XYZToRGB(const Color& xyz);
	private:
		typedef Color ValueType;
		typedef std::map<float, ValueType> KeyMap;
	public:
		AgDynamicValueController() {};
		~AgDynamicValueController() {};

		void SetMap(const KeyMap& keymap)
		{
			m_keyMap = keymap;
		}

		ValueType GetValue(float time) const
		{
			typename KeyMap::const_iterator itUpper = m_keyMap.upper_bound(time + 1e-6f);
			typename KeyMap::const_iterator itLower = itUpper;
			--itLower;
			if (itLower == m_keyMap.end())
			{
				return itUpper->second;
			}
			if (itUpper == m_keyMap.end())
			{
				return itLower->second;
			}
			float lowerTime = itLower->first;
			const ValueType& lowerVal = itLower->second;
			float upperTime = itUpper->first;
			const ValueType& upperVal = itUpper->second;
			if (lowerTime == upperTime)
			{
				return lowerVal;
			}
			return interpolate(lowerTime, lowerVal, upperTime, upperVal, time);
		};

		void Clear()
		{
			m_keyMap.clear();
		};

	private:
		const ValueType interpolate(float lowerTime, const ValueType& lowerVal, float upperTime, const ValueType& upperVal, float time) const
		{
			float x = (time - lowerTime) / (upperTime - lowerTime);
			ValueType result;
			bx::vec3Lerp(result.data, lowerVal.data, upperVal.data, x);
			return result;
		};

		KeyMap	m_keyMap;
	};
}
