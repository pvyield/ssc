/*******************************************************************************************************
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

#include "lib_pvmodel.h"

class mlmodel_module_t : public pvmodule_t
{
public:
	int N_series;

	double Width;
	double Length;

	double V_mp_ref;
	double I_mp_ref;
	double V_oc_ref;
	double I_sc_ref;

	double S_ref;
	double T_ref;

	double R_shref;
	double R_sh0;
	double R_shexp;
	double R_s;
	double alpha_isc;
	double E_g;
	double n_0;
	double mu_n;
	
	int T_mode;
	double T_c_no_tnoct;
	int T_c_no_mounting;
	int T_c_no_standoff;
	double T_c_fa_alpha;
	double T_c_fa_U0;
	double T_c_fa_U1;

	int AM_mode;
	double AM_c_sa[5];
	double AM_c_lp[6];

	int IAM_mode;
	double IAM_c_as;
	double IAM_c_sa[6];
	int IAM_c_cs_elements;
	double IAM_c_cs_incAngle[100];
	double IAM_c_cs_iamValue[100];

	mlmodel_module_t();

	virtual double AreaRef() { return (Width * Length); }
	virtual double VmpRef() { return V_mp_ref; }
	virtual double ImpRef() { return I_mp_ref; }
	virtual double VocRef() { return V_oc_ref; }
	virtual double IscRef() { return I_sc_ref; }
	virtual bool operator() (pvinput_t &input, double TcellC, double opvoltage, pvoutput_t &output);
	virtual void initializeManual();
};

class mock_celltemp_t : public pvcelltemp_t
{
public:
	virtual bool operator() (pvinput_t &input, pvmodule_t &module, double opvoltage, double &Tcell);
};