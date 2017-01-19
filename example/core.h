#pragma once


class Core
{
private:
	double flux_;
	double rod_position_;
	double primary_loop_coolant_mass_;
	double primary_loop_coolant_temperature_;
	double primary_loop_coolant_pressure_;
	double primary_loop_coolant_volume_;
	int    pressurizer_count_;

	double iodine_135_concentration_;
	double xenon_135_concentration_;

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
};