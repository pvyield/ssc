/*******************************************************************************************************
*  Copyright 2017 Alliance for Sustainable Energy, LLC
*
*  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
*  (“Alliance”) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
*  The Government retains for itself and others acting on its behalf a nonexclusive, paid-up,
*  irrevocable worldwide license in the software to reproduce, prepare derivative works, distribute
*  copies to the public, perform publicly and display publicly, and to permit others to do so.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted
*  provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer in the documentation and/or
*  other materials provided with the distribution.
*
*  3. The entire corresponding source code of any redistribution, with or without modification, by a
*  research entity, including but not limited to any contracting manager/operator of a United States
*  National Laboratory, any institution of higher learning, and any non-profit organization, must be
*  made publicly available under this license for as long as the redistribution is made available by
*  the research entity.
*
*  4. Redistribution of this software, without modification, must refer to the software by the same
*  designation. Redistribution of a modified version of this software (i) may not refer to the modified
*  version by the same designation, or by any confusingly similar designation, and (ii) must refer to
*  the underlying software originally provided by Alliance as “System Advisor Model” or “SAM”. Except
*  to comply with the foregoing, the terms “System Advisor Model”, “SAM”, or any confusingly similar
*  designation may not be used to refer to any modified version of this software or any modified
*  version of the underlying software originally provided by Alliance without the prior written consent
*  of Alliance.
*
*  5. The name of the copyright holder, contributors, the United States Government, the United States
*  Department of Energy, or any of their employees may not be used to endorse or promote products
*  derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER,
*  CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR
*  EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
*  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************************************/

#include <math.h>
#include <cmath>
#include <limits>
#include "lib_ondinv.h"
#include "mlm_spline.h" // spline interpolator for efficiency curves

bool isInitialized = false;

// efficiency spline variables
std::vector<double> X[2];
std::vector<double> Y[2];
tk::spline effSpline[2];


ond_inverter::ond_inverter()
{
	//Paco = Pdco = Vdco = Pso = Pntare = C0 = C1 = C2 = C3 = std::numeric_limits<double>::quiet_NaN();
}

// Initialize - Calculates values that only need calculation once
void ond_inverter::initializeManual()
{
	if (!isInitialized)
	{
		isInitialized = true;
		// Convert P_AC efficiency curve to P_DC

		// set up efficiency splines
		for (int j = 0; j <= 2; j = j + 1) {
			X[j].clear();
			Y[j].clear();
			for (int i = 0; i <= effCurve_elements - 1; i = i + 1) {
				X[j].push_back(effCurve_P[j][i]);
				Y[j].push_back(effCurve_eta[j][i]);
			}
			effSpline[j].set_points(X[j], Y[j]);
		}
	}
}

double tempDerateAC(double arrayT[], double arrayPAC[], double T) {
	double PAC_max;
	double T_low;
	double T_high;
	double PAC_low;
	double PAC_high;
	int arrayLength;

	PAC_max = -10 ^ 10;
	arrayLength = sizeof(arrayT);

	for (int i = 0; i <= arrayLength + 1; i = i + 1) {
		if (i = 0) {
			if (T <= arrayT[i]) {
				PAC_max = arrayPAC[i];
				break;
			}
		}
		else if (i = arrayLength + 1) {
			if (T > arrayT[i]) {
				PAC_max = arrayPAC[i];
				break;
			}
		}
		else {
			if (arrayT[i] > T && arrayT[i - 1] <= T) {
				T_low = arrayT[i - 1];
				T_high = arrayT[i];
				PAC_low = arrayPAC[i - 1];
				PAC_high = arrayPAC[i];
				PAC_max = PAC_low + (PAC_high - PAC_low) * (T - T_low) / (T_high - T_low);
				break;
			}
		}
	}
	if (PAC_max < 0) {
		throw std::invalid_argument("PAC_max is negative.");
	}
	return PAC_max;
}

bool ond_inverter::acpower(
	/* inputs */
	double Pdc,     /* Input power to inverter (Wdc) */
	double Vdc,     /* Voltage input to inverter (Vdc) */
	double Tamb,

	/* outputs */
	double *Pac,    /* AC output power (Wac) */
	double *Ppar,   /* AC parasitic power consumption (Wac) */
	double *Plr,    /* Part load ratio (Pdc_in/Pdc_rated, 0..1) */
	double *Eff,	    /* Conversion efficiency (0..1) */
	double *Pcliploss, /* Power loss due to clipping loss (Wac) */
	double *Psoloss, /* Power loss due to operating power consumption (Wdc) */
	double *Pntloss /* Power loss due to night time tare loss (Wac) */
)
{
	// calculate voltage drop in DC cabling
	double dV;
	double Vdc_eff;
	double Pdc_eff;
	dV = dV_nom * (Pdc / PNomDC);
	Vdc_eff = Vdc - dV;
	Pdc_eff = Pdc * (Vdc_eff / Vdc);

	// determine efficiency from splines
	double V_eta_low;
	double V_eta_high;
	double eta_low;
	double eta_high;
	int index_eta;

	if (Vdc_eff < VNomEff[1]) {
		index_eta = 0;
	}
	else {
		index_eta = 1;
	}

	V_eta_low = VNomEff[index_eta];
	V_eta_high = VNomEff[index_eta + 1];
	eta_low = effSpline[index_eta](Pdc_eff);
	eta_high = effSpline[index_eta + 1](Pdc_eff);
	*Eff = eta_low + (eta_high - eta_low) * (Vdc_eff - V_eta_low) / (V_eta_high - V_eta_low);

	//double etaArray[2];
	//for (int i = 0; i <= 2; i = i + 1) {
	//	etaArray[i] = effSpline[i](Pdc_eff);
	//}
	//*Eff = etaArray[1];

	if (*Eff < 0.0) *Eff = 0.0;
	*Pac = *Eff * Pdc_eff;

	// Limit Pac to temperature limit
	double Pac_max_T;
	double T_array[5] = {0, TPMax, TPNom, TPLim1, TPLimAbs};
	double PAC_array[5] = {PMaxOUT, PMaxOUT, PNomConv, PLim1, 0};
	Pac_max_T = tempDerateAC(T_array, PAC_array, Tamb);

	// Limit Pac to current limit
	double Pac_max_I;
	double Idc;
	Idc = Pdc_eff / Vdc_eff;
	Pac_max_I = Vdc_eff * min(Idc, INomDC);

	// Calculate clipping/limiting losses
	*Pcliploss = 0.0;
	double PacNoClip = *Pac;
	if (*Pac > Pac_max_T || *Pac > Pac_max_I)
	{
		*Pac = min(Pac_max_T, Pac_max_I);
		*Pcliploss = PacNoClip - *Pac;
	}

	// night time power loss Wac (note that if PacNoPso > Pso and Pac < Pso then the night time loss could be considered an operating power loss)
	// Pso: /* DC power require to start inversion process, or self-consumption by inverter (Wdc) */ = PSeuil
	*Psoloss = 0.0; // Self-consumption during operation
	*Ppar = 0.0;
	*Pntloss = 0.0;
	if (Pdc_eff <= PSeuil)
	{
		*Pac = -Night_Loss;
		*Ppar = Night_Loss;
		*Pntloss = Night_Loss;
	}
	else
	{
		// Power consumption during operation only occurs
		// when inverter is operating during the day 
		// calculate by setting B to zero (ie. Pso = 0 );
		double PacNoPso = *Pac + Aux_Loss;
		*Psoloss = PacNoPso - *Pac;
	}

	// Final calculations and returning true
	*Plr = Pdc_eff / PNomDC;
	return true;
}