#pragma once
#include "ioports.h"

namespace cph
{
namespace io
{

	class VPortBase :public NativePortBase
	{
		VPortBase(VPortBase &);
		VPortBase &operator=(VPortBase &);
	public:
		VPortBase()
		{

		}
		virtual void Write(DataT value)const=0;
		virtual DataT Read()const=0;
		virtual void Set(DataT value)const=0;
		virtual void Clear(DataT value)const=0;
		virtual DataT PinRead()const=0;
		virtual void SetConfiguration(DataT mask, Configuration configuration)const=0;
	};
	
	template<class Port>
	class VPort: public VPortBase
	{
	public:
		virtual void Write(DataT value)const
		{
			Port::Write(value);
		}
		
		virtual DataT Read()const
		{
			return Port::Read();
		}
		
		virtual void Set(DataT value)const
		{
			Port::Set(value);
		}
		
		virtual void Clear(DataT value)const
		{
			Port::Clear(value);
		}

		virtual DataT PinRead()const
		{
			return Port::PinRead();
		}
		
		virtual void SetConfiguration(DataT mask, VPortBase::Configuration configuration)const
		{
			Port::SetConfiguration(mask, configuration);
		}
	};
	
	class VPin 
	{
	public:
		//const uint8_t Number;
		const uint8_t Mask;
		const VPortBase &Port;
		typedef NativePortBase ConfigPort;
		typedef NativePortBase::Configuration Configuration;
		
		VPin(VPortBase &port, uint8_t number)
		:	//Number(number), 
			Mask(1 << number), 
			Port(port)
		{	}
		
		void Set()
		{
			Port.Set(Mask);
		}
		
		void Set(bool val)
		{
			if(val)
				Port.Set(Mask);
			else
				Port.Clear(Mask);
		}
		
		void SetDir(bool val)
		{
			if(val)
				Port.SetConfiguration(Mask, ConfigPort::Out);
			else
				Port.SetConfiguration(Mask, ConfigPort::In);
		}

		void Clear()
		{
			Port.Clear(Mask);
		}
		
		void SetDirRead()
		{
			Port.SetConfiguration(Mask, ConfigPort::In);
		}
		
		void SetDirWrite()
		{
			Port.SetConfiguration(Mask, ConfigPort::Out);
		}
		
		void SetConfiguration(Configuration configuration)
		{
			Port.SetConfiguration(Mask, configuration);
		}

		bool IsSet()
		{
			return Port.PinRead() & Mask;
		}
	};
}
}

