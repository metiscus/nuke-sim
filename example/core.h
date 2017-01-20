#pragma once


class Core
{
private:
	double flux_;
	double rod_position_;
	double primary_loop_coolant_mass_;
	double primary_loop_coolant_mass_nominal_;
	double primary_loop_coolant_temperature_;
	double primary_loop_coolant_pressure_;
	double primary_loop_coolant_volume_; 			// m3
	double primary_loop_coolant_volume_nominal_;	// m3

	double core_mass_flow_;
	
	double pressurizer_area_;						// m2
	double pressurizer_coolant_mass_;				// kg
	double pressurizer_relief_mass_nominal_;
	double pressurizer_relief_mass_;
	double pressurizer_coolant_level_;
	double pressurizer_coolant_volume_;

	double sump_mass_;

	double iodine_135_concentration_;
	double xenon_135_concentration_;

	double reactor_coolant_pump_mass_flow_;	// kg/s
	bool reactor_cooland_pump_online_;

public:
	Core();

	inline double get_flux() const
	{
		return flux_;
	}

	inline double get_rod_position() const
	{
		return rod_position_;
	}

	inline void set_rod_position(double pos)
	{
		rod_position_ = pos;
	}

	void simulate(double dt);

private:
	void simulate_nuclear_reaction(double dt);
	void simulate_inner_cooling_loop(double dt);
};