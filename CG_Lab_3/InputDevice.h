
#pragma once

#include <unordered_map>
#include <windows.h>

namespace CGLib {
    class InputDevice
    {
    public:
        void ProcessKeyDown(unsigned int key) { m_keys[key] = true; }
        void ProcessKeyUp(unsigned int key) { m_keys[key] = false; }

        bool IsKeyPressed(unsigned int key) const {
            auto it = m_keys.find(key);
            return it != m_keys.end() && it->second;
        }

		void ProcessMouseMove(int dx, int dy)
		{
			mouseDelta_.x += dx;
			mouseDelta_.y += dy;
		}

		int GetWheelDelta()
		{
			int delta = wheelDelta_;
			wheelDelta_ = 0; // сброс после чтения
			return delta;
		}
		POINT GetMouseDelta()
		{
			POINT d = mouseDelta_;
			mouseDelta_ = { 0,0 }; // сброс после чтения
			return d;
		}
		//POINT lastMousePos_{ 0,0 };
		POINT mouseDelta_{ 0,0 };
		int wheelDelta_ = 0; // накопленная дельта колеса

    private:    
        std::unordered_map<unsigned int, bool> m_keys;
		

		
    };
}
