#include "core.h"

#include <cmath>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

/*
	References:
	[1] - A simple dynamic model for the primary circuit in VVER
	      plants for controller design purposes. - Fazekas
*/

namespace Constants
{
	double RodParams [3] = { -1.322e-4, -6.08e-5, -2.85e-4 }; // { m^2, m, dimensionless }

	double Lambda  = 10e-5;	// 
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

#if 0
	double iodine_fission_yield = 6.2e-2;
	//double iodine_fission_yield = 1.8e-3;
	double iodine_fission_yield_sq = 0.0;
#else
	double iodine_fission_yield = 1e-4;
	//double iodine_fission_yield = 1.8e-3;
	double iodine_fission_yield_sq = 1e-5;
#endif
	double iodine_half_life = 23760.0;
	double xenon_fission_yield  = 1.22e-2;
	double xenon_capture_probability = 2.65e-7; // per second
	//double xenon_capture_probability = 2.65e-3; // per second

	double iodine_capture_probability =  8e-3 * xenon_capture_probability; // per second
	//double xenon_capture_probability = 2.65e-2; // per second

	constexpr double neutron_scale = 1e4;
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
		return 28884.78 - 258.01 * temperature + 0.63 * temperature * temperature;
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
	: rod_position_(0.0)
	, flux_(1.0)
	, iodine_135_concentration_(0.0f)
	, xenon_135_concentration_(0.0f)
{
}

void Core::simulate(double dt)
{
	static double timebank = 0.0;
	timebank += dt;

	constexpr double FixedTimestep = 1.0 / 60.0;

	static double time_since_decay_step = 0.0;


	fprintf(stderr, "rods: %lf\tflux: %lf\tXe135: %lf\tI135: %lf\n", rod_position_, flux_, xenon_135_concentration_, iodine_135_concentration_);
	while(timebank > FixedTimestep)
	{
		timebank -= FixedTimestep;
		time_since_decay_step += FixedTimestep;

		
		// Integrate the neutron flux in the core
		double dFlux = (1.0 / Constants::Lambda) * (Constants::RodParams[0] * rod_position_ * rod_position_ + Constants::RodParams[1] * rod_position_ + Constants::RodParams[2]) * flux_ + Constants::S;

		// The capture of a neutron by Xe135 results in a loss of flux, as well as a loss of Xe135 concentration
		double neutron_flux = flux_ * Constants::neutron_scale;
		double xenon_flux_capture = FixedTimestep * neutron_flux * Constants::xenon_capture_probability * xenon_135_concentration_;
		xenon_135_concentration_ -= std::min(xenon_flux_capture, xenon_135_concentration_);

		//double iodine_flux_capture = FixedTimestep * neutron_flux * Constants::iodine_capture_probability * iodine_135_concentration_;
		//iodine_135_concentration_ -= std::min(iodine_flux_capture, iodine_135_concentration_);
		
		flux_ = flux_ + dFlux * FixedTimestep; /* - iodine_flux_capture; */
		neutron_flux = flux_ * Constants::neutron_scale;

		if(flux_ <= 0.0)
		{
			flux_ = 0.0;
		}

#if 0
		while(time_since_decay_step > 0.1)
		{
			time_since_decay_step -= 0.1;

			double iodine_decays = 0.5 * (iodine_135_concentration_ * 0.1 / Constants::iodine_half_life);
			iodine_135_concentration_ -= std::min(iodine_decays, iodine_135_concentration_);
			xenon_135_concentration_  += iodine_decays;
		}
#else
		double iodine_decays = 0.5 * (iodine_135_concentration_ * FixedTimestep / Constants::iodine_half_life);
		iodine_135_concentration_ -= std::min(iodine_decays, iodine_135_concentration_);
		xenon_135_concentration_  += iodine_decays;
#endif

		//fprintf(stderr, "flux: %lf\t xecap: %lf\n", flux_, xenon_flux_capture);

		// The reaction of U235 generates iodine 135 as a product of the fission of U235;
		// note that the yield is per atom, but we are measuring neutron flux which is 3x the number of reacting
		// U235 atoms.
		double iodine_135_generation =  neutron_flux / 3.0 * (FixedTimestep * Constants::iodine_fission_yield + Constants::iodine_fission_yield_sq * FixedTimestep*FixedTimestep);
		iodine_135_concentration_ += iodine_135_generation;

		// Some xenon in the reactor comes as direct fission products
		iodine_135_concentration_ += FixedTimestep * Constants::xenon_fission_yield * neutron_flux / 3.0;

		//fprintf(stderr, "I135: %lf\t Decay: %lf\n", 1e3*iodine_135_concentration_, iodine_decays);
		//fprintf(stderr, "Xe135: %lf\n", 1e3*xenon_135_concentration_);



		//double reactor_thermal_output = Constants::Cpsi * flux_;
		//double dTout = 1.0 / (Constants::CpPC * state_.Mpc) + reactor_thermal_output

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

		// update outputs
		//outputs_.Wr  = Constants::Cpsi * state_.N;
		//outputs_.Psg = saturated_vapor_pressure(state_.Tpr);
		//outputs_.Lpr = (1.0 / Constants::Apr) * ((state_.Mpc / water_density(state_.Tpc)) - Constants::V0pc);
		//outputs_.Ppr = saturated_vapor_pressure(state_.Tpr);

		//fprintf(stderr, "\trods: %lf\tflux: %lf\tXe135: %lf\tI135: %lf\n", rod_position_, flux_, xenon_135_concentration_, iodine_135_concentration_);
	}



	//if(rod_position_ > 0.5) rod_position_ -= 1.e-2 * FixedTimestep;

}