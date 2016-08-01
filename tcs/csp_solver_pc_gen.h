#ifndef __csp_solver_pc_gen_
#define __csp_solver_pc_gen_

#include "csp_solver_util.h"
#include "csp_solver_core.h"

class C_pc_gen : public C_csp_power_cycle
{
private:
	// *************************************************************
	// Define temperature and cp values so code interfaces with solver
	// *************************************************************
	double m_T_htf_cold_fixed;		//[K]
	double m_T_htf_hot_fixed;		//[K]
	double m_cp_htf_fixed;			//[kJ/kg-K]
	// *************************************************************

	// Values reset in 'converged' call
		// As of 3.15.16, no minimum power cycle startup time requirement
	double m_q_startup_remain;		//[MWt] power cycle startup energy required before electricity generation
	double m_q_startup_used;		//[MWt] power cycle startup energy used during timestep
	int m_pc_mode_prev;				//[-] power cycle operating mode in previous timestep
	int m_pc_mode;					//[-] power cycle operating mode in current timestep

	// Calculated constant member data
	double m_q_des;			//[MWt] Thermal power to cycle at design
	double m_qttmin;		//[MWt] Min thermal power to cycle
	double m_qttmax;		//[MWt] Max thermal power to cycle

	void check_double_params_are_set();

public:
	// Class to save messages for up stream classes
	C_csp_messages mc_csp_messages;

	struct S_params
	{
		double m_W_dot_des;		//[MWe] Design power cycle gross output
		double m_eta_des;		//[-] Design power cycle gross efficiency
		double m_f_wmax;		//[-] Maximum over-design power cycle operation fraction
		double m_f_wmin;		//[-] Minimum part-load power cycle operation fraction
		double m_f_startup;		//[hr] Equivalent full-load hours required for power system startup
		double m_T_pc_des;		//[C] Power conversion reference temperature, convert to K in init
		int m_PC_T_corr;		//[-] Power cycle temperature correction mode (1=wetb, 2=dryb)

		std::vector<double> mv_etaQ_coefs;	//[1/Mwt] Part-load power conversion efficiency adjustment coefficients
		std::vector<double> mv_etaT_coefs;	//[1/C] Temp.-based power conversion efficiency adjustment coefs

		S_params()
		{
			m_W_dot_des = m_eta_des = m_f_wmax = m_f_wmin = m_f_startup = m_T_pc_des = std::numeric_limits<double>::quiet_NaN();

			m_PC_T_corr = -1;

			mv_etaQ_coefs.resize(0);
			mv_etaT_coefs.resize(0);
		}
	};

	S_params ms_params;

	C_pc_gen();

	~C_pc_gen(){};

	void get_fixed_properties(double &T_htf_cold_fixed /*K*/, double &T_htf_hot_fixed /*K*/, double &cp_htf_fixed /*K*/);

	virtual void init(C_csp_power_cycle::S_solved_params &solved_params);

	virtual int get_operating_state();

	virtual double get_cold_startup_time();
	virtual double get_warm_startup_time();
	virtual double get_hot_startup_time();
	virtual double get_standby_energy_requirement();    //[MW]
	virtual double get_cold_startup_energy();    //[MWh]
	virtual double get_warm_startup_energy();    //[MWh]
	virtual double get_hot_startup_energy();    //[MWh]
	virtual double get_max_thermal_power();     //MW
	virtual double get_min_thermal_power();     //MW
	virtual double get_efficiency_at_TPH(double T_degC, double P_atm, double relhum_pct);
	virtual double get_efficiency_at_load(double load_frac);

	// This can vary between timesteps for Type224, depending on remaining startup energy and time
	virtual double get_max_q_pc_startup();		//[MWt]


	virtual void call(const C_csp_weatherreader::S_outputs &weather,
		C_csp_solver_htf_1state &htf_state_in,
		const C_csp_power_cycle::S_control_inputs &inputs,
		C_csp_power_cycle::S_csp_pc_out_solver &out_solver,
		C_csp_power_cycle::S_csp_pc_out_report &out_report,
		const C_csp_solver_sim_info &sim_info);

	virtual void converged();

};


#endif  //__csp_solver_pc_gen_