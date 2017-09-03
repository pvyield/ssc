/*******************************************************************************************************
*
* This code is licensed with the MIT license
* https://opensource.org/licenses/MIT
*
* Copyright 2017 - pvyield GmbH / Timo Richert
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
* associated documentation files(the "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
* sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject
* to the following conditions :
*    -  The above copyright notice and this permission notice shall be included in all copies or substantial
*       portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN
* NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*******************************************************************************************************/

/*******************************************************************************************************
* Implementation of the Mermoud/Thibault single-diode model
*
* SOURCES
* [1] "Performance assessment of a simulation model for PV modules of any available technology",
*     André Mermoud and Thibault Lejeune, 2010 (https://archive-ouverte.unige.ch/unige:38547)
*******************************************************************************************************/

#include "lib_mlmodel.h"

static const double k = 1.38064852e-23; // Boltzmann constant [J/K]
static const double q = 1.60217662e-19; // Elemenatry charge [C]
static const double T_0 = 273.15; // 0 degrees Celsius in Kelvin [K]
static const double PI = 3.1415926535897932; // pi

// 1: NOCT
static const int T_MODE_NOCT = 1;

// 1: Use ASHRAE formula, 2: Use Sandia polynomial, 3: Use cubic spline with user-supplied data
static const int IAM_MODE_ASHRAE = 1;
static const int IAM_MODE_SANDIA = 2;
static const int IAM_MODE_SPLINE = 3;

// 1: Use Sandia polynomial [corr=f(AM)], 2: Use standard coefficients from DeSoto model [corr=f(AM)], 3: Use First Solar polynomial [corr=f(AM, p_wat)]
static const int AM_MODE_SANDIA = 1;
static const int AM_MODE_DESOTO = 2;
static const int AM_MODE_LEE_PANCHULA = 3;

mlmodel_module_t::mlmodel_module_t()
{
	Width = Length = V_mp_ref = I_mp_ref = V_oc_ref = I_sc_ref = S_ref = T_ref
		= R_shref = R_sh0 = R_shexp = R_s
		= alpha_isc = E_g = n_0 = mu_n = T_c_no_tnoct = std::numeric_limits<double>::quiet_NaN();
}
bool mlmodel_module_t::operator() (pvinput_t &input, double T_C, double opvoltage, pvoutput_t &out)
{
	// initialize output first
	out.Power = out.Voltage = out.Current = out.Efficiency = out.Voc_oper = out.Isc_oper = 0.0;
	
	// Irradiance calculation
	double G_total, Geff_total;
	if (input.radmode != 3) { // Determine if the model needs to skip the cover effects (will only be skipped if the user is using POA reference cell data) 
		G_total = input.Ibeam + input.Idiff + input.Ignd; // total incident irradiance on tilted surface, W/m2
		Geff_total = G_total;
	}
	else { // Even though we're using POA ref. data, we may still need to use the decomposed poa
		if (input.usePOAFromWF)
			G_total = Geff_total = input.poaIrr;
		else {
			G_total = input.poaIrr;
			Geff_total = input.Ibeam + input.Idiff + input.Ignd;
		}
	}

	// Incidence Angle Modifier
	double f_IAM = 0;
	double IncAngRad = input.IncAng / 180 * PI;
	if (IAM_mode == IAM_MODE_ASHRAE)
	{
		f_IAM = 1 - IAM_c_as * (1 / cos(IncAngRad) - 1);
	}
	else if (IAM_mode == IAM_MODE_SANDIA)
	{
		f_IAM = IAM_c_sa[0] + IAM_c_sa[1] * pow(IncAngRad, 1) + IAM_c_sa[2] * pow(IncAngRad, 2) + IAM_c_sa[3] * pow(IncAngRad, 3) + IAM_c_sa[4] * pow(IncAngRad, 4) + IAM_c_sa[5] * pow(IncAngRad, 5);
	}
	else if (IAM_mode == IAM_MODE_SPLINE)
	{
		// throw std::invalid_argument("IAM_mode to be implemented.");
	}
	else {
		// throw std::invalid_argument("Unknown IAM_mode.");
	}

	// Spectral correction function
	double f_AM = 0;
	if (AM_mode == AM_MODE_SANDIA)
	{
		f_AM = air_mass_modifier(input.Zenith, input.Elev, AM_c_sa);
	}
	else if (AM_mode == AM_MODE_DESOTO)
	{
		double amavec[5] = { 0.918093, 0.086257, -0.024459, 0.002816, -0.000126 };
		f_AM = air_mass_modifier(input.Zenith, input.Elev, amavec);
	}
	else if (AM_mode == AM_MODE_LEE_PANCHULA)
	{
		// throw std::invalid_argument("AM_mode to be implemented.");
	}
	else
	{
		// throw std::invalid_argument("Unknown AM_mode.");
	}

	double S = Geff_total * f_IAM * f_AM;

	// Single diode model acc. to [1]
	if (S >= 1)
	{
		double nVT, I_0ref, I_Lref, n, a, I_L, I_0, R_sh, I_sc;
		double V_oc = V_oc_ref; // V_oc_ref as initial guess
		double P, V, I;

		nVT = N_series * n_0 * k * (T_ref + T_0) / q;
		I_0ref = (I_sc_ref + (I_sc_ref * R_s + V_oc_ref) / R_shref) / (exp(V_oc_ref / nVT - 1) - exp((I_sc_ref * R_s) / nVT - 1));
		I_Lref = I_0ref * exp(V_oc_ref / nVT - 1) + V_oc_ref / R_shref;

		n = n_0 + mu_n * (T_C - T_ref);
		a = N_series * k * (T_C + T_0) * n / q;
		I_L = (S / S_ref) * (I_Lref + alpha_isc *(T_C - T_ref));
		I_0 = I_0ref * pow(((T_C + T_0) / (T_ref + T_0)), 3) * exp((q * E_g) / (n * k)* (1 / (T_ref + T_0) - 1 / (T_C + T_0)));
		R_sh = R_shref + (R_sh0 - R_shref) * exp(-R_shexp * (S / S_ref));

		V_oc = openvoltage_5par(V_oc, a, I_L, I_0, R_sh);
		I_sc = I_L / (1 + R_s / R_sh);

		if (opvoltage < 0)
		{
			P = maxpower_5par(V_oc, a, I_L, I_0, R_s, R_sh, &V, &I);
		}
		else
		{ // calculate power at specified operating voltage
			V = opvoltage;
			if (V >= V_oc) I = 0;
			else I = current_5par(V, 0.9*I_L, a, I_L, I_0, R_s, R_sh);
			P = V*I;
		}

		out.Power = P;
		out.Voltage = V;
		out.Current = I;
		out.Efficiency = P / ((Width * Length) * S);
		out.Voc_oper = V_oc;
		out.Isc_oper = I_sc;
		out.CellTemp = T_C;
	}

	return out.Power >= 0;
}