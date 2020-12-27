#pragma once
namespace cph
{
struct ITM_Output
{
public:
	ITM_Output()
	{
		enum
		{
			ITM_LAR_ACCESS = 0xc5acce55
		};
		ITM->LAR = ITM_LAR_ACCESS;
		ITM->TCR = 0x1;
		ITM->TPR = 0x1;
		ITM->TER = 0xf;
		CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	}
	void operator()(char c) const
	{
		ITM_SendChar(c);
	}
};

} // namespace cph