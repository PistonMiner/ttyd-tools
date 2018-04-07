#pragma once

namespace mod {

template<typename ValueType>
class Timer
{
public:
	Timer() {}

	void start()
	{
		mRunning = true;
	}
	void stop()
	{
		mRunning = false;
	}
	void tick()
	{
		if (mRunning)
		{
			++mValue;
		}
	}
	void setValue(const ValueType &value)
	{
		mValue = value;
	}
	ValueType getValue()
	{
		return mValue;
	}
	
private:
	bool mRunning = false;
	ValueType mValue;
};

}