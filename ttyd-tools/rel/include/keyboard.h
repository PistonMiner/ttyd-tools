#pragma once

#include <cstdint>

#include <vector>

namespace mod {

enum class KeyCode : uint8_t
{
	kInvalid      = 0x00,

	kHome         = 0x06,
	kEnd          = 0x07,
	kPageUp       = 0x08,
	kPageDown     = 0x09,
	kScrollLock   = 0x0A,

	kA            = 0x10,
	kB            = 0x11,
	kC            = 0x12,
	kD            = 0x13,
	kE            = 0x14,
	kF            = 0x15,
	kG            = 0x16,
	kH            = 0x17,
	kI            = 0x18,
	kJ            = 0x19,
	kK            = 0x1a,
	kL            = 0x1b,
	kM            = 0x1c,
	kN            = 0x1d,
	kO            = 0x1e,
	kP            = 0x1f,
	kQ            = 0x20,
	kR            = 0x21,
	kS            = 0x22,
	kT            = 0x23,
	kU            = 0x24,
	kV            = 0x25,
	kW            = 0x26,
	kX            = 0x27,
	kY            = 0x28,
	kZ            = 0x29,
	k1            = 0x2a,
	k2            = 0x2b,
	k3            = 0x2c,
	k4            = 0x2d,
	k5            = 0x2e,
	k6            = 0x2f,
	k7            = 0x30,
	k8            = 0x31,
	k9            = 0x32,
	k0            = 0x33,
	kMinus        = 0x34,
	kPlus         = 0x35,
	kPrintScreen  = 0x36,
	kBracketOpen  = 0x37,
	kBracketClose = 0x38,
	kColon        = 0x39,
	kQuote        = 0x3a,
	kHash         = 0x3b,
	kComma        = 0x3c,
	kPeriod       = 0x3d,
	kSlash        = 0x3e,
	kBackslash    = 0x3f,
	kF1           = 0x40,
	kF2           = 0x41,
	kF3           = 0x42,
	kF4           = 0x43,
	kF5           = 0x44,
	kF6           = 0x45,
	kF7           = 0x46,
	kF8           = 0x47,
	kF9           = 0x48,
	kF10          = 0x49,
	kF11          = 0x4a,
	kF12          = 0x4b,
	kEscape       = 0x4c,
	kInsert       = 0x4d,
	kDelete       = 0x4e,
	kTilde        = 0x4f,
	kBackspace    = 0x50,
	kTab          = 0x51,

	kCapsLock     = 0x53,
	kLeftShift    = 0x54,
	kRightShift   = 0x55,
	kLeftControl  = 0x56,
	kRightAlt     = 0x57,
	kLeftWindows  = 0x58,
	kSpace        = 0x59,
	kRightWindows = 0x5a,
	kMenu         = 0x5b,
	kLeftArrow    = 0x5c,
	kDownArrow    = 0x5d,
	kUpArrow      = 0x5e,
	kRightArrow   = 0x5f,
	kEnter        = 0x61,
};

class Keyboard
{
public:
	Keyboard(int channel);

	void setChannel(int channel);
	void update();

	int getKeyDownCount()
	{
		return mKeysDownCount;
	}
	KeyCode getKeyDown(int index)
	{
		return mKeysDown[index];
	}
	bool isKeyDown(KeyCode key)
	{
		for (int i = 0; i < mKeysDownCount; ++i)
		{
			if (mKeysDown[i] == key)
			{
				return true;
			}
		}
		return false;
	}

	int getKeyPressedCount()
	{
		return mKeysPressedCount;
	}
	KeyCode getKeyPressed(int index)
	{
		return mKeysPressed[index];
	}
	bool isKeyPressed(KeyCode key)
	{
		for (int i = 0; i < mKeysPressedCount; ++i)
		{
			if (mKeysPressed[i] == key)
			{
				return true;
			}
		}
		return false;
	}

	int getKeyReleasedCount()
	{
		return mKeysReleasedCount;
	}
	KeyCode getKeyReleased(int index)
	{
		return mKeysReleased[index];
	}
	bool isKeyReleased(KeyCode key)
	{
		for (int i = 0; i < mKeysReleasedCount; ++i)
		{
			if (mKeysReleased[i] == key)
			{
				return true;
			}
		}
		return false;
	}

	static constexpr char getCharForKeycode(KeyCode code, bool shift = false)
	{
		if (code >= KeyCode::kA && code <= KeyCode::kZ)
		{
			return static_cast<char>(static_cast<int>(code) - static_cast<int>(KeyCode::kA) + (shift ? 'A' : 'a'));
		}
		else if (code >= KeyCode::k1 && code <= KeyCode::k9)
		{
			return static_cast<char>(static_cast<int>(code) - static_cast<int>(KeyCode::k1) + '1');
		}
		else if (code == KeyCode::k0)
		{
			return '0';
		}
		else if (code == KeyCode::kMinus)
		{
			return shift ? '_' : '-';
		}
		else if (code == KeyCode::kComma)
		{
			return shift ? ';' : ',';
		}
		else if (code == KeyCode::kSpace)
		{
			return ' ';
		}
		else
		{
			return '\0';
		}
	}

private:
	bool connect();
	void disconnect();

public:
	const static int cMaxKeysPressed = 3;

private:
	int mKeysPrevCount = 0;
	KeyCode mKeysPrev[cMaxKeysPressed];

	int mKeysDownCount = 0;
	KeyCode mKeysDown[cMaxKeysPressed];

	int mKeysReleasedCount = 0;
	KeyCode mKeysReleased[cMaxKeysPressed];

	int mKeysPressedCount = 0;
	KeyCode mKeysPressed[cMaxKeysPressed];

	bool mConnected = false;
	int mChannel = -1;
};

}