#pragma once


class Core
{
public:
	struct Inputs
	{
		double RodPosition;
		double Min;
		double WheatPR;
		double Msgin;
		double MrIn;
	};

	struct Outputs
	{
		double Wr;
		double Mpr; 
		double Ppr; 
		double Lpr; 
		double Psg;
		double Tout;
	};

	struct State
	{
		double N;
		double Mpc;
		double Tpc;
		double Tpr;
		double Msg;
		double Tsg;
		double Tw;
	};

private:
	Inputs inputs_;
	Outputs outputs_;
	State state_;

public:
	Core();

	inline Inputs get_inputs() const
	{
		return inputs_;
	}

	inline void set_inputs(const Inputs& inputs)
	{
		inputs_ = inputs;
	}

	inline Outputs get_outputs() const
	{
		return outputs_;
	}

	inline void set_state(const State& state)
	{
		state_ = state;
	}

	inline State get_state() const
	{
		return state_;
	}

	inline double get_flux() const
	{
		return state_.N;
	}

	void simulate(double dt);
};