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
* [1] André Mermoud and Thibault Lejeune, "Performance assessment of a simulation model for PV modules
*     of any available technology", 2010 (https://archive-ouverte.unige.ch/unige:38547)
* [2] John A. Duffie, "Solar Engineering of Thermal Processes", 4th Edition, 2013 by John Wiley & Sons
* [3] W. De Soto et al., “Improvement and validation of a model for photovoltaic array performance”,
*     Solar Energy, vol 80, pp. 78-88, 2006.
*******************************************************************************************************/

#include "lib_mlmodel.h"
#include "mlm_spline.h"

static const double k = 1.38064852e-23; // Boltzmann constant [J/K]
static const double q = 1.60217662e-19; // Elemenatry charge [C]
static const double T_0 = 273.15; // 0 degrees Celsius in Kelvin [K]
static const double PI = 3.1415926535897932; // pi

bool isInitialized = false;

double nVT = 0;
double I_0ref = 0;
double I_Lref = 0;

double amavec[5] = { 0.918093, 0.086257, -0.024459, 0.002816, -0.000126 }; // DeSoto IAM coefficients [3]

std::vector<double> X;
std::vector<double> Y;
tk::spline iamSpline;

// 1: NOCT, 2: Extended Faiman
static const int T_MODE_NOCT = 1;
static const int T_MODE_FAIMAN = 2;

// 1: Use ASHRAE formula, 2: Use Sandia polynomial, 3: Use cubic spline with user-supplied data
static const int IAM_MODE_ASHRAE = 1;
static const int IAM_MODE_SANDIA = 2;
static const int IAM_MODE_SPLINE = 3;

// 1: Do not use AM correction 1: Use Sandia polynomial [corr=f(AM)], 2: Use standard coefficients from DeSoto model [3] [corr=f(AM)], 3: Use First Solar polynomial [corr=f(AM, p_wat)]
static const int AM_MODE_OFF = 1;
static const int AM_MODE_SANDIA = 2;
static const int AM_MODE_DESOTO = 3;
static const int AM_MODE_LEE_PANCHULA = 4;

mlmodel_module_t::mlmodel_module_t()
{
	Width = Length = V_mp_ref = I_mp_ref = V_oc_ref = I_sc_ref = S_ref = T_ref
		= R_shref = R_sh0 = R_shexp = R_s
		= alpha_isc = E_g = n_0 = mu_n = T_c_no_tnoct
		= T_c_fa_alpha = T_c_fa_U0 = T_c_fa_U1 = std::numeric_limits<double>::quiet_NaN();

}

// IAM functions
double IAMvalue_ASHRAE(double b0, double theta)
{
	return (1 - b0 * (1 / cos(theta) - 1));
}
double IAMvalue_SANDIA(double coeff[], double theta)
{
	return coeff[0] + coeff[1] * theta + coeff[2] * pow(theta, 2) + coeff[3] * pow(theta, 3) + coeff[4] * pow(theta, 4) + coeff[5] * pow(theta, 5);
}

// Initialize - Calculates values that only need calculation once
void mlmodel_module_t::initializeManual()
{
	if (!isInitialized)
	{
		isInitialized = true;
		// Calculate values of constant reference values.
		double R_sh_STC = R_shref + (R_sh0 - R_shref) * exp(-R_shexp * (S_ref / S_ref));
		nVT = N_series * n_0 * k * (T_ref + T_0) / q;
		I_0ref = (I_sc_ref + (I_sc_ref * R_s - V_oc_ref) / R_sh_STC) / ((exp(V_oc_ref / nVT) - 1) - (exp((I_sc_ref * R_s) / nVT) - 1));
		I_Lref = I_0ref * (exp(V_oc_ref / nVT) - 1) + V_oc_ref / R_sh_STC;

		// set up IAM spline
		if (IAM_mode == IAM_MODE_SPLINE)
		{
			X.clear();
			Y.clear();
			for (int i = 0; i <= IAM_c_cs_elements - 1; i = i + 1) {
				X.push_back(IAM_c_cs_incAngle[i]);
				Y.push_back(IAM_c_cs_iamValue[i]);
			}
			iamSpline.set_points(X, Y);
		}
	}
}

// Main module model
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
	double f_IAM_beam = 0, f_IAM_diff = 0, f_IAM_gnd = 0;
	double theta_beam = input.IncAng;
	double theta_diff = (59.7 - 0.1388 * input.Tilt + 0.001497 * pow(input.Tilt, 2)); // from [2], equation 5.4.2
	double theta_gnd = (90.0 - 0.5788 * input.Tilt + 0.002693 * pow(input.Tilt, 2)); // from [2], equation 5.4.1

	switch (IAM_mode)
	{
		case IAM_MODE_ASHRAE:
			f_IAM_beam = IAMvalue_ASHRAE(IAM_c_as, theta_beam / 180 * PI);
			f_IAM_diff = IAMvalue_ASHRAE(IAM_c_as, theta_diff / 180 * PI);
			f_IAM_gnd = IAMvalue_ASHRAE(IAM_c_as, theta_gnd / 180 * PI);
			break;
		case IAM_MODE_SANDIA:
			f_IAM_beam = IAMvalue_SANDIA(IAM_c_sa, theta_beam / 180 * PI);
			f_IAM_diff = IAMvalue_SANDIA(IAM_c_sa, theta_diff / 180 * PI);
			f_IAM_gnd = IAMvalue_SANDIA(IAM_c_sa, theta_gnd / 180 * PI);
			break;
		case IAM_MODE_SPLINE:
			f_IAM_beam = std::min(iamSpline(theta_beam), 1.0);
			f_IAM_diff = std::min(iamSpline(theta_diff), 1.0);
			f_IAM_gnd = std::min(iamSpline(theta_gnd), 1.0);
			break;
	}

	// Spectral correction function
	double f_AM = 0;
	switch (AM_mode)
	{
	case AM_MODE_OFF:
			f_AM = 1.0;
			break;
		case AM_MODE_SANDIA:
			f_AM = air_mass_modifier(input.Zenith, input.Elev, AM_c_sa);
			break;
		case AM_MODE_DESOTO:
			f_AM = air_mass_modifier(input.Zenith, input.Elev, amavec);
			break;
		case AM_MODE_LEE_PANCHULA:
			f_AM = -1; // TO BE ADDED
			break;
	}

	double S = (f_IAM_beam * input.Ibeam + f_IAM_diff * input.Idiff + f_IAM_gnd * input.Ignd) * f_AM;

	// Single diode model acc. to [1]
	if (S >= 1)
	{
		double n, a, I_L, I_0, R_sh, I_sc;
		double V_oc = V_oc_ref; // V_oc_ref as initial guess
		double P, V, I, eff;
		double T_cell = T_C;

		int iterations = 1;
		if (T_mode == T_MODE_FAIMAN)
		{
			iterations = 2; // two iterations, 1st with guessed eff, 2nd with calculated efficiency
			eff = (I_mp_ref * V_mp_ref) / ((Width * Length) * S_ref); // efficiency guess for initial run
		}

		for (int i = 1; i <= iterations; i = i + 1) {
			if (T_mode == T_MODE_FAIMAN)
			{
				T_cell = input.Tdry + (T_c_fa_alpha * G_total * (1 - eff)) / (T_c_fa_U0 + input.Wspd * T_c_fa_U1);
			}

			n = n_0 + mu_n * (T_cell - T_ref);
			a = N_series * k * (T_cell + T_0) * n / q;
			I_L = (S / S_ref) * (I_Lref + alpha_isc *(T_cell - T_ref));
			I_0 = I_0ref * pow(((T_cell + T_0) / (T_ref + T_0)), 3) * exp((q * E_g) / (n * k)* (1 / (T_ref + T_0) - 1 / (T_cell + T_0)));
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
			eff = P / ((Width * Length) * S);
		}

		out.Power = P;
		out.Voltage = V;
		out.Current = I;
		out.Efficiency = eff;
		out.Voc_oper = V_oc;
		out.Isc_oper = I_sc;
		out.CellTemp = T_cell;
	}

	return out.Power >= 0;
}

// mockup cell temperature model
// to be used in cases when Tcell is calculated within the module model
bool mock_celltemp_t::operator() (pvinput_t &input, pvmodule_t &module, double, double &Tcell)
{
	Tcell = -999;
	return true;
}
