#include "core.h"

#include <cmath>
#include <cstring>

namespace Constants
{
	double RodParams [3] = { -1.36e-4, -6.05e-5, -2.88e-4 };
	double Lambda  = 1.0e-5;	// 
	double S       = 2859.0;	// %/s
	double CpPC    = 5281.0;	// J/kg/K
	double KtSG1   = 9.19e6;	// W/K
	double KlossPC = 3.0e6;		// W/K
	double alpha   = 1.097;		// --
	double Msg0    = 31810.0;	// kg
	double CpSG    = 4651.1;	// J/kg/K
	double KlossSG = 1.52e8;	// W
	double Ktsg2   = 3.30e6;	// W/K
	double beta    = 2.004;		// --
	double CpWMw   = 2.031e7;	// J/K
	double Tw0     = 267.9;		// deg C
	double CpPR    = 5895.4;  	// J/kg/K
	double WlossPR = 1.48e5;   	// W
	double Cpsi    = 13.75e6;   // W / %
	double Apr     = 4.52;		// m^2
	double V0pc    = 242.0;     // m^3
	double mout    = 2.9722;    // kg/s
	double min     = 1.4222;    // kg/s
	double TpcLoss = 10.0;		// temperature loss between Tpc,CL and Tpci
	double ToutPC  = 293.0;
	double Msg     = 120.56;	// mass of steam / water through steam-generator
}

namespace
{
	// Estimates the water density at a given temperature kelvin
	inline double water_density(double temperature)
	{
		constexpr double C0 = 581.2;
		constexpr double C1 = 2.98;
		constexpr double C2 = -0.00848;

		return C0 + C1 * temperature + C2 * temperature * temperature;
	}

	// Estimates the saturated vapor pressure using centigrade
	// returns in units of kPa
	inline double saturated_vapor_pressure(double temperature)
	{
		return 28884.78 - 258.01*temperature + 0.63*temperature*temperature;
	}

	inline double my_pow(double x, double y)
	{
		if(x < 0.0f)
		{
			return -1.0 * pow(-x, y);
		}
		else
		{
			return pow(x,y);
		}
	}
}

Core::Core()
{
	memset(&inputs_, 0, sizeof(inputs_));
	memset(&state_, 0, sizeof(state_));
	memset(&outputs_, 0, sizeof(outputs_));

	inputs_.MrIn = 2.9722;
	outputs_.Tout = 200.0;

	state_.Tpc = 200.0;
	state_.Tw = Constants::Tw0;
	state_.Mpc = 200000.0;
	state_.Tpr = 326.57;
	state_.N   = 10.0;

	outputs_.Mpr = 19400;
	outputs_.Lpr = 4.80;

}

void Core::simulate(double dt)
{
	static double timebank = 0.0;
	timebank += dt;

	constexpr double FixedTimestep = 1.0 / 60.0;

	while(timebank > FixedTimestep)
	{
		timebank -= FixedTimestep;

		// Integrate flux
		double dN = (1.0 / Constants::Lambda) * (Constants::RodParams[0] * my_pow(inputs_.RodPosition, 2.0) + Constants::RodParams[1] * inputs_.RodPosition + Constants::RodParams[2]) * state_.N + Constants::S;
		state_.N = state_.N + dN * FixedTimestep;

		double reactor_thermal_output = Constants::Cpsi * state_.N;

		// Integrate Mpc
		//double dMpc = Constants::min - Constants::mout;
		//state_.Mpc = state_.Mpc + dMpc * FixedTimestep;

		// Integrate Tpc
		//double dTpc = 1.0 / (Constants::CpPC * state_.Mpc) * (Constants::CpPC * Constants::min * (-Constants::TpcLoss) 
		//			+ outputs_.Wr 
		//			+ Constants::CpPC * Constants::mout * 15.0
		//			- 6.0 * Constants::KtSG1*my_pow(state_.Tpc - state_.Tw, Constants::alpha)
		//			- Constants::KlossPC * (state_.Tpc - Constants::ToutPC));
		//state_.Tpc += state_.Tpc * FixedTimestep * dTpc;

		// Integrate Tsg
		//const double Msgin = Msg - 100;
		//double dTsg = 1.0 / (Constants::CpSG * state_.Msg) * (Constants::CpSG * Msg)

		// Integrate Mpr
		//outputs_.Mpr = (Constants::min - Constants::mout) - Constants::V0pc * water_density(dTpc);

		inputs_.RodPosition += 1e-2 * FixedTimestep;
		if(inputs_.RodPosition > 2.25) inputs_.RodPosition = 2.25f;

		// update outputs
		outputs_.Wr  = Constants::Cpsi * state_.N;
		//outputs_.Psg = saturated_vapor_pressure(state_.Tpr);
		//outputs_.Lpr = (1.0 / Constants::Apr) * ((state_.Mpc / water_density(state_.Tpc)) - Constants::V0pc);
		//outputs_.Ppr = saturated_vapor_pressure(state_.Tpr);
	}

}