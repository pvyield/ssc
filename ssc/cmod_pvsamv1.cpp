/*******************************************************************************************************
*  Copyright 2017 Alliance for Sustainable Energy, LLC
*
*  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
<<<<<<< HEAD
*  (�Alliance�) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
=======
*  ("Alliance") under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
>>>>>>> pr/11
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
<<<<<<< HEAD
*  the underlying software originally provided by Alliance as �System Advisor Model� or �SAM�. Except
*  to comply with the foregoing, the terms �System Advisor Model�, �SAM�, or any confusingly similar
=======
*  the underlying software originally provided by Alliance as "System Advisor Model" or "SAM". Except
*  to comply with the foregoing, the terms "System Advisor Model", "SAM", or any confusingly similar
>>>>>>> pr/11
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

#include "cmod_pvsamv1.h"
#include "lib_pv_io_manager.h"

// comment following define if do not want shading database validation outputs
//#define SHADE_DB_OUTPUTS

static var_info _cm_vtab_pvsamv1[] = {
/*   VARTYPE           DATATYPE         NAME                                            LABEL                                                   UNITS      META                             GROUP                  REQUIRED_IF                 CONSTRAINTS                      UI_HINTS*/
	{ SSC_INPUT,        SSC_STRING,      "solar_resource_file",                         "Weather file in TMY2, TMY3, EPW, or SAM CSV.",         "",         "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_TABLE,       "solar_resource_data",                         "Weather data",                                         "",         "lat,lon,tz,elev,year,month,hour,minute,gh,dn,df,poa,tdry,twet,tdew,rhum,pres,snow,alb,aod,wspd,wdir",    "pvsamv1",              "?",                        "",                              "" },

	// transformer model percent of rated ac output
	{ SSC_INPUT,		SSC_NUMBER,		 "transformer_no_load_loss",					"Power transformer no load loss",						"%",		"",								 "pvsamv1",				 "?=0",						 "",							 "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "transformer_load_loss",						"Power transformer load loss",							"%",		"",								 "pvsamv1",				 "?=0",						 "",							 "" },

	// optional for lifetime analysis
	{ SSC_INPUT,        SSC_NUMBER,      "system_use_lifetime_output",                  "PV lifetime simulation",                               "0/1",      "",                              "pvsamv1",             "?=0",                        "INTEGER,MIN=0,MAX=1",          "" },
	{ SSC_INPUT,        SSC_NUMBER,      "analysis_period",                             "Lifetime analysis period",                             "years",    "",                              "pvsamv1",             "system_use_lifetime_output=1",   "",                             "" },
	{ SSC_INPUT,        SSC_ARRAY,       "dc_degradation",                              "Annual module degradation",                            "%/year",   "",                              "pvsamv1",             "system_use_lifetime_output=1",   "",                             "" },
//	{ SSC_INPUT,        SSC_ARRAY,       "ac_degradation",                              "Annual AC degradation",                                "%/year",   "",                              "pvsamv1",             "system_use_lifetime_output=1",   "",                             "" },
	{ SSC_OUTPUT,       SSC_ARRAY,       "dc_degrade_factor",                           "Annual module degrade factor",                         "",         "",                              "Annual",             "system_use_lifetime_output=1",   "",                             "" },
//	{ SSC_OUTPUT,       SSC_ARRAY,       "ac_degrade_factor",                           "Annual AC degrade factor",                             "",         "",                              "pvsamv1",             "system_use_lifetime_output=1",   "",                             "" },
	{ SSC_INPUT,        SSC_NUMBER,      "en_dc_lifetime_losses",                       "Enable lifetime daily DC losses",                      "0/1",      "",                              "pvsamv1",             "?=0",                        "INTEGER,MIN=0,MAX=1",          "" },
	{ SSC_INPUT,        SSC_ARRAY,       "dc_lifetime_losses",                          "Lifetime daily DC losses",                             "%",        "",                              "pvsamv1",             "en_dc_lifetime_losses=1",    "",                             "" },
	{ SSC_INPUT,        SSC_NUMBER,      "en_ac_lifetime_losses",                       "Enable lifetime daily AC losses",                      "0/1",      "",                              "pvsamv1",             "?=0",                        "INTEGER,MIN=0,MAX=1",          "" },
	{ SSC_INPUT,        SSC_ARRAY,       "ac_lifetime_losses",                          "Lifetime daily AC losses",                             "%",        "",                              "pvsamv1",             "en_ac_lifetime_losses=1",    "",                             "" },

	//SEV: Activating the snow model
	{ SSC_INPUT,        SSC_NUMBER,      "en_snow_model",                               "Toggle snow loss estimation",                          "0/1",      "",                              "snowmodel",            "?=0",                       "BOOLEAN",                      "" },
	{ SSC_INPUT,        SSC_NUMBER,      "system_capacity",                             "DC Nameplate capacity",                                "kWdc",       "",                            "pvsamv1",              "*",                         "",                             "" },

	{ SSC_INPUT,        SSC_NUMBER,      "use_wf_albedo",                               "Use albedo in weather file if provided",               "0/1",      "",                              "pvsamv1",              "?=1",                      "BOOLEAN",                       "" },
	{ SSC_INPUT,        SSC_ARRAY,       "albedo",                                      "User specified ground albedo",                         "0..1",     "",                              "pvsamv1",              "*",						  "LENGTH=12",					  "" },
	{ SSC_INPUT,        SSC_NUMBER,      "irrad_mode",                                  "Irradiance input translation mode",                     "",        "0=beam&diffuse,1=total&beam,2=total&diffuse,3=poa_reference,4=poa_pyranometer", "pvsamv1", "?=0", "INTEGER,MIN=0,MAX=4", "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sky_model",                                   "Diffuse sky model",                                     "",        "0=isotropic,1=hkdr,2=perez",    "pvsamv1",              "?=2",                      "INTEGER,MIN=0,MAX=2",           "" },

	{ SSC_INPUT,        SSC_NUMBER,      "inverter_count",                              "Number of inverters",                                   "",        "",                              "pvsamv1",              "*",                        "INTEGER,POSITIVE",              "" },

	{ SSC_INPUT,        SSC_NUMBER,      "enable_mismatch_vmax_calc",                   "Enable mismatched subarray Vmax calculation",           "",        "",                              "pvsamv1",              "?=0",                      "BOOLEAN",                       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_nstrings",                          "Sub-array 1 Number of parallel strings",                "",        "",                              "pvsamv1",              "",						 "INTEGER",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_modules_per_string",                "Sub-array 1 Modules per string",                        "",        "",                              "pvsamv1",              "*",                        "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_mppt_input",                        "Sub-array 1 Inverter MPPT input number",                "",        "",                              "pvsamv1",              "?=1",                      "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_tilt",                              "Sub-array 1 Tilt",                                      "deg",     "0=horizontal,90=vertical",      "pvsamv1",              "",                         "MIN=0,MAX=90",                "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_tilt_eq_lat",                       "Sub-array 1 Tilt=latitude override",                    "0/1",     "",                              "pvsamv1",              "",                         "BOOLEAN",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_azimuth",                           "Sub-array 1 Azimuth",                                   "deg",     "0=N,90=E,180=S,270=W",          "pvsamv1",              "",                         "MIN=0,MAX=359.9",               "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_track_mode",                        "Sub-array 1 Tracking mode",                             "",        "0=fixed,1=1axis,2=2axis,3=azi,4=monthly", "pvsamv1",    "*",                        "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_rotlim",                            "Sub-array 1 Tracker rotation limit",                    "deg",     "",                              "pvsamv1",              "?=45",                     "MIN=0,MAX=85",                  "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "subarray1_shade_mode",				     	"Sub-array 1 shading mode (fixed tilt or 1x tracking)",	 "0/1/2",   "0=none,1=standard(non-linear),2=thin film(linear)", "pvsamv1", "*",                 "INTEGER,MIN=0,MAX=2",		      "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_gcr",                               "Sub-array 1 Ground coverage ratio",                     "0..1",    "",                              "pvsamv1",              "?=0.3",                    "MIN=0,MAX=3",               "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray1_monthly_tilt",                      "Sub-array 1 monthly tilt input",                        "deg",     "",                              "pvsamv1",              "subarray1_track_mode=4",   "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_shading:string_option",             "Sub-array 1 shading string option",                     "",        "0=shadingdb,1=shadingdb_notc,2=average,3=maximum,4=minimum", "pvsamv1", "?=-1",     "INTEGER,MIN=-1,MAX=4", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray1_shading:timestep",                  "Sub-array 1 timestep beam shading losses",              "%",       "",                              "pvsamv1",              "?",                        "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray1_shading:mxh",                       "Sub-array 1 Month x Hour beam shading losses",          "%",       "",                              "pvsamv1",              "?",                        "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray1_shading:azal",                      "Sub-array 1 Azimuth x altitude beam shading losses",    "%",       "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_shading:diff",                      "Sub-array 1 Diffuse shading loss",                      "%",       "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray1_soiling",                           "Sub-array 1 Monthly soiling loss",                      "%",       "",                              "pvsamv1",              "*",                        "LENGTH=12",                      "" },

	// loss diagram outputs, also used to calculate total dc derate
	{ SSC_INPUT, SSC_NUMBER, "subarray1_rear_irradiance_loss", "Sub-array 1 rear irradiance loss", "%", "", "pvsamv1", "*", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray1_mismatch_loss", "Sub-array 1 DC mismatch loss", "%", "", "pvsamv1", "*", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray1_diodeconn_loss", "Sub-array 1 DC diodes and connections loss", "%", "", "pvsamv1", "*", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray1_dcwiring_loss", "Sub-array 1 DC wiring loss", "%", "", "pvsamv1", "*", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray1_tracking_loss", "Sub-array 1 DC tracking error loss", "%", "", "pvsamv1", "*", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray1_nameplate_loss", "Sub-array 1 DC nameplate loss", "%", "", "pvsamv1", "*", "MIN=-5,MAX=100", "" },

	{ SSC_INPUT, SSC_NUMBER, "subarray2_rear_irradiance_loss", "Sub-array 2 rear irradiance loss", "%", "", "pvsamv1", "subarray2_enable=1", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray2_mismatch_loss", "Sub-array 2 DC mismatch loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray2_diodeconn_loss", "Sub-array 2 DC diodes and connections loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray2_dcwiring_loss", "Sub-array 2 DC wiring loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray2_tracking_loss", "Sub-array 2 DC tracking error loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray2_nameplate_loss", "Sub-array 2 DC nameplate loss", "%", "", "pvsamv1", "?", "MIN=-5,MAX=100", "" },

	{ SSC_INPUT, SSC_NUMBER, "subarray3_rear_irradiance_loss", "Sub-array 3 rear irradiance loss", "%", "", "pvsamv1", "subarray3_enable=1", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray3_mismatch_loss", "Sub-array 3 DC mismatch loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray3_diodeconn_loss", "Sub-array 3 DC diodes and connections loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray3_dcwiring_loss", "Sub-array 3 DC wiring loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray3_tracking_loss", "Sub-array 3 DC tracking error loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray3_nameplate_loss", "Sub-array 3 DC nameplate loss", "%", "", "pvsamv1", "?", "MIN=-5,MAX=100", "" },

	{ SSC_INPUT, SSC_NUMBER, "subarray4_rear_irradiance_loss", "Sub-array 4 rear irradiance loss", "%", "", "pvsamv1", "subarray4_enable=1", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray4_mismatch_loss", "Sub-array 4 DC mismatch loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray4_diodeconn_loss", "Sub-array 4 DC diodes and connections loss", "%", "?", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray4_dcwiring_loss", "Sub-array 4 DC wiring loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray4_tracking_loss", "Sub-array 4 DC tracking error loss", "%", "", "pvsamv1", "?", "MIN=0,MAX=100", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray4_nameplate_loss", "Sub-array 4 DC nameplate loss", "%", "", "pvsamv1", "?", "MIN=-5,MAX=100", "" },

	//this is a DC loss that is applied uniformly to all subarrays
	{ SSC_INPUT,		SSC_NUMBER,		"dcoptimizer_loss",								"DC power optimizer loss",		"%", "",	"pvsamv1", "*", "MIN=0,MAX=100", "" },
	//AC losses are also applied uniformly to all subarrays
	{ SSC_INPUT,		SSC_NUMBER,		"acwiring_loss",								"AC wiring loss",				"%", "",	"pvsamv1", "*", "MIN=0,MAX=100", "" },
//	{ SSC_INPUT, SSC_NUMBER, "transformer_loss", "AC step-up transformer loss", "%", "", "pvsamv1", "*", "MIN=0,MAX=100", "" },
	{ SSC_INPUT,		SSC_NUMBER,		"transmission_loss",							"Transmission loss",			"%", "",	"pvsamv1", "*", "MIN=0,MAX=100", "" },

	//

	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_mod_orient",                        "Sub-array 1 Module orientation",                          "0/1",    "0=portrait,1=landscape",        "pvsamv1",              "*",                        "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_nmodx",                             "Sub-array 1 Number of modules along bottom of row",          "",       "",                              "pvsamv1",              "*",                        "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_nmody",                             "Sub-array 1 Number of modules along side of row",            "",       "",                              "pvsamv1",              "*",                        "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray1_backtrack",                         "Sub-array 1 Backtracking enabled",                        "",       "0=no backtracking,1=backtrack", "pvsamv1",              "subarray1_track_mode=1",   "BOOLEAN",                       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_enable",                            "Sub-array 2 Enable",                                      "0/1",    "0=disabled,1=enabled",          "pvsamv1",              "?=0",                      "BOOLEAN",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_modules_per_string",                "Sub-array 2 Modules per string",                          "",       "",                              "pvsamv1",              "subarray2_enable=1",                        "INTEGER,MIN=1",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_nstrings",                          "Sub-array 2 Number of parallel strings",                  "",       "",                              "pvsamv1",              "subarray2_enable=1",       "INTEGER,MIN=1",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_mppt_input",                        "Sub-array 2 Inverter MPPT input number",                  "",       "",                              "pvsamv1",              "?=1",                      "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_tilt",                              "Sub-array 2 Tilt",                                        "deg",    "0=horizontal,90=vertical",      "pvsamv1",              "",                         "MIN=0,MAX=90",                "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_tilt_eq_lat",                       "Sub-array 2 Tilt=latitude override",                      "0/1",    "",                              "pvsamv1",              "",                         "BOOLEAN",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_azimuth",                           "Sub-array 2 Azimuth",                                     "deg",    "0=N,90=E,180=S,270=W",          "pvsamv1",              "",                         "MIN=0,MAX=359.9",               "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_track_mode",                        "Sub-array 2 Tracking mode",                               "",       "0=fixed,1=1axis,2=2axis,3=azi,4=monthly", "pvsamv1",    "subarray2_enable=1",       "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_rotlim",                            "Sub-array 2 Tracker rotation limit",                      "deg",    "",                              "pvsamv1",              "?=45",                     "MIN=0,MAX=85",                  "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "subarray2_shade_mode",				     	"Sub-array 2 Shading mode (fixed tilt or 1x tracking)",	   "0/1/2",   "0=none,1=standard(non-linear),2=thin film(linear)", "pvsamv1", "subarray2_enable=1", "INTEGER,MIN=0,MAX=2",		   "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_gcr",                               "Sub-array 2 Ground coverage ratio",                       "0..1",   "",                              "pvsamv1",              "?=0.3",                    "MIN=0,MAX=3",               "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray2_monthly_tilt",                      "Sub-array 2 Monthly tilt input",                          "deg",    "",                              "pvsamv1",              "",                         "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_shading:string_option",             "Sub-array 2 Shading string option",                       "", "0=shadingdb,1=shadingdb_notc,2=average,3=maximum,4=minimum", "pvsamv1", "?=-1", "INTEGER,MIN=-1,MAX=4", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray2_shading:timestep",                  "Sub-array 2 Timestep beam shading losses",                "%", "", "pvsamv1", "?", "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray2_shading:mxh",                        "Sub-array 2 Month x Hour beam shading losses",           "%", "", "pvsamv1", "?", "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray2_shading:azal",                      "Sub-array 2 Azimuth x altitude beam shading losses",      "%",      "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_shading:diff",                      "Sub-array 2 Diffuse shading loss",                        "%",      "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray2_soiling",                           "Sub-array 2 Monthly soiling loss",                        "%",      "",                              "pvsamv1",              "subarray2_enable=1",       "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_mod_orient",                        "Sub-array 2 Module orientation",                          "0/1",    "0=portrait,1=landscape",        "pvsamv1",              "subarray2_enable=1",       "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_nmodx",                             "Sub-array 2 Number of modules along bottom of row",          "",       "",                              "pvsamv1",              "subarray2_enable=1",       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_nmody",                             "Sub-array 2 Number of modules along side of row",            "",       "",                              "pvsamv1",              "subarray2_enable=1",       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray2_backtrack",                         "Sub-array 2 Backtracking enabled",                        "",       "0=no backtracking,1=backtrack", "pvsamv1",              "",                         "BOOLEAN",                       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_enable",                            "Sub-array 3 Enable",                                      "0/1",    "0=disabled,1=enabled",          "pvsamv1",              "?=0",                      "BOOLEAN",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_modules_per_string",                "Sub-array 3 Modules per string",                          "",       "",                              "pvsamv1",              "subarray3_enable=1",       "INTEGER,MIN=1",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_nstrings",                          "Sub-array 3 Number of parallel strings",                  "",       "",                              "pvsamv1",              "subarray3_enable=1",       "INTEGER,MIN=1",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_mppt_input",                        "Sub-array 3 Inverter MPPT input number",                  "",       "",                              "pvsamv1",              "?=1",                      "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_tilt",                              "Sub-array 3 Tilt",                                        "deg",    "0=horizontal,90=vertical",      "pvsamv1",              "",                         "MIN=0,MAX=90",                "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_tilt_eq_lat",                       "Sub-array 3 Tilt=latitude override",                      "0/1",    "",                              "pvsamv1",              "",                         "BOOLEAN",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_azimuth",                           "Sub-array 3 Azimuth",                                     "deg",    "0=N,90=E,180=S,270=W",          "pvsamv1",              "",                         "MIN=0,MAX=359.9",               "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_track_mode",                        "Sub-array 3 Tracking mode",                               "",       "0=fixed,1=1axis,2=2axis,3=azi,4=monthly", "pvsamv1",    "subarray3_enable=1",       "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_rotlim",                            "Sub-array 3 Tracker rotation limit",                      "deg",    "",                              "pvsamv1",              "?=45",                     "MIN=0,MAX=85",                  "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "subarray3_shade_mode",				     	"Sub-array 3 Shading mode (fixed tilt or 1x tracking)",	   "0/1/2",   "0=none,1=standard(non-linear),2=thin film(linear)", "pvsamv1", "subarray3_enable=1", "INTEGER,MIN=0,MAX=2",		   "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_gcr",                               "Sub-array 3 Ground coverage ratio",                       "0..1",   "",                              "pvsamv1",              "?=0.3",                    "MIN=0,MAX=3",               "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray3_monthly_tilt",                      "Sub-array 3 Monthly tilt input",                          "deg",    "",                              "pvsamv1",              "",                         "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_shading:string_option",             "Sub-array 3 Shading string option",                       "",       "0=shadingdb,1=shadingdb_notc,2=average,3=maximum,4=minimum", "pvsamv1", "?=-1",     "INTEGER,MIN=-1,MAX=4", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray3_shading:timestep",                  "Sub-array 3 Timestep beam shading losses",                "%",      "",                              "pvsamv1",              "?",                        "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray3_shading:mxh",                       "Sub-array 3 Month x Hour beam shading losses",            "%",      "",                              "pvsamv1",              "?",                        "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray3_shading:azal",                      "Sub-array 3 Azimuth x altitude beam shading losses",      "%",      "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_shading:diff",                      "Sub-array 3 Diffuse shading loss",                        "%",      "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray3_soiling",                           "Sub-array 3 Monthly soiling loss",                        "%",      "",                              "pvsamv1",              "subarray3_enable=1",       "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_mod_orient",                        "Sub-array 3 Module orientation",                          "0/1",    "0=portrait,1=landscape",        "pvsamv1",              "subarray3_enable=1",       "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_nmodx",                             "Sub-array 3 Number of modules along bottom of row",          "",       "",                              "pvsamv1",              "subarray3_enable=1",       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_nmody",                             "Sub-array 3 Number of modules along side of row",            "",       "",                              "pvsamv1",              "subarray3_enable=1",       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray3_backtrack",                         "Sub-array 3 Backtracking enabled",                        "",       "0=no backtracking,1=backtrack", "pvsamv1",              "",                         "BOOLEAN",                       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_enable",                            "Sub-array 4 Enable",                                      "0/1",    "0=disabled,1=enabled",          "pvsamv1",              "?=0",                      "BOOLEAN",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_modules_per_string",                "Sub-array 4 Modules per string",                          "",       "",                              "pvsamv1",              "subarray4_enable=1",       "INTEGER,MIN=1",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_nstrings",                          "Sub-array 4 Number of parallel strings",                  "",       "",                              "pvsamv1",              "subarray4_enable=1",       "INTEGER,MIN=1",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_mppt_input",                        "Sub-array 4 Inverter MPPT input number",                  "",       "",                              "pvsamv1",              "?=1",                       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_tilt",                              "Sub-array 4 Tilt",                                        "deg",    "0=horizontal,90=vertical",      "pvsamv1",              "",                         "MIN=0,MAX=90",                "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_tilt_eq_lat",                       "Sub-array 4 Tilt=latitude override",                      "0/1",    "",                              "pvsamv1",              "",                         "BOOLEAN",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_azimuth",                           "Sub-array 4 Azimuth",                                     "deg",    "0=N,90=E,180=S,270=W",          "pvsamv1",              "",                         "MIN=0,MAX=359.9",               "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_track_mode",                        "Sub-array 4 Tracking mode",                               "",       "0=fixed,1=1axis,2=2axis,3=azi,4=monthly", "pvsamv1",    "subarray4_enable=1",       "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_rotlim",                            "Sub-array 4 Tracker rotation limit",                      "deg",    "",                              "pvsamv1",              "?=45",                     "MIN=0,MAX=85",                  "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "subarray4_shade_mode",				     	"Sub-array 4 shading mode (fixed tilt or 1x tracking)",	   "0/1/2",  "0=none,1=standard(non-linear),2=thin film(linear)", "pvsamv1", "subarray4_enable=1", "INTEGER,MIN=0,MAX=2",		   "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_gcr",                               "Sub-array 4 Ground coverage ratio",                       "0..1",   "",                              "pvsamv1",              "?=0.3",                    "MIN=0,MAX=3",               "" },
<<<<<<< HEAD
	{ SSC_INPUT,        SSC_ARRAY,       "subarray4_monthly_tilt",                      "Sub-array 4 monthly tilt input",                          "deg",    "",                              "pvsamv1",              "subarray4_track_mode=4",   "LENGTH=12",                     "" },
//	{ SSC_INPUT,        SSC_ARRAY,       "subarray4_shading:hourly",                    "Sub-array 4 Hourly beam shading losses",                 "%",       "",                              "pvsamv1",              "?",                        "",                              "" },
//	{ SSC_INPUT, SSC_NUMBER, "subarray4_shading:shading_db_lookup", "Sub-array 4 enable shading database lookup", "", "", "pvsamv1", "?=0", "BOOLEAN", "" },
//	{ SSC_INPUT, SSC_NUMBER, "subarray4_shading:string_option", "Sub-array 4 shading string option", "", "0=shadingdb,1=average,2=maximum,3=minimum", "pvsamv1", "?=-1", "INTEGER,MIN=-1,MAX=3", "" },
	{ SSC_INPUT, SSC_NUMBER, "subarray4_shading:string_option", "Sub-array 4 shading string option", "", "0=shadingdb,1=shadingdb_notc,2=average,3=maximum,4=minimum", "pvsamv1", "?=-1", "INTEGER,MIN=-1,MAX=4", "" },
	{ SSC_INPUT, SSC_MATRIX, "subarray4_shading:timestep", "Sub-array 4 timestep beam shading losses", "%", "", "pvsamv1", "?", "", "" },
	{ SSC_INPUT, SSC_MATRIX, "subarray4_shading:mxh", "Sub-array 4 Month x Hour beam shading losses", "%", "", "pvsamv1", "?", "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray4_shading:azal",                      "Sub-array 4 Azimuth x altitude beam shading losses",     "%",       "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_shading:diff",                      "Sub-array 4 Diffuse shading loss",                       "%",       "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray4_soiling",                           "Sub-array 4 Monthly soiling loss",                       "%",   "",                              "pvsamv1",              "subarray4_enable=1",       "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_mod_orient",                        "Sub-array 4 Module orientation for self-shading",         "0/1",    "0=portrait,1=landscape",        "pvsamv1",              "subarray4_shade_mode>0", "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_nmodx",                             "Sub-array 4 no. of modules along bottom for self-shading","",       "",                              "pvsamv1",              "subarray4_shade_mode>0", "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_nmody",                             "Sub-array 4 no. of modules along side for self-shading",  "",       "",                              "pvsamv1",              "subarray4_shade_mode>0", "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_backtrack",                         "Sub-array 4 Backtracking enabled",                        "",       "0=no backtracking,1=backtrack", "pvsamv1",              "subarray4_track_mode=1",   "BOOLEAN",                       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "module_model",                                "Photovoltaic module model specifier",                     "",       "0=spe,1=cec,2=6par_user,3=snl,4=sd11-iec61853", "pvsamv1",              "*",                        "INTEGER,MIN=0,MAX=5",           "" },
=======
	{ SSC_INPUT,        SSC_ARRAY,       "subarray4_monthly_tilt",                      "Sub-array 4 Monthly tilt input",                          "deg",    "",                              "pvsamv1",              "",                         "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_shading:string_option",             "Sub-array 4 Shading string option",                       "",       "0=shadingdb,1=shadingdb_notc,2=average,3=maximum,4=minimum", "pvsamv1", "?=-1", "INTEGER,MIN=-1,MAX=4", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray4_shading:timestep",                  "Sub-array 4 Timestep beam shading losses",                "%",      "",                              "pvsamv1",              "?",                        "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray4_shading:mxh",                       "Sub-array 4 Month x Hour beam shading losses",            "%",      "",                              "pvsamv1",              "?",                        "", "" },
	{ SSC_INPUT,        SSC_MATRIX,      "subarray4_shading:azal",                      "Sub-array 4 Azimuth x altitude beam shading losses",      "%",      "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_shading:diff",                      "Sub-array 4 Diffuse shading loss",                        "%",      "",                              "pvsamv1",              "?",                        "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "subarray4_soiling",                           "Sub-array 4 Monthly soiling loss",                        "%",      "",                              "pvsamv1",              "subarray4_enable=1",       "LENGTH=12",                     "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_mod_orient",                        "Sub-array 4 Module orientation",                          "0/1",    "0=portrait,1=landscape",        "pvsamv1",              "subarray4_enable=1",       "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_nmodx",                             "Sub-array 4 Number of modules along bottom of row",          "",       "",                              "pvsamv1",              "subarray4_enable=1",       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_nmody",                             "Sub-array 4 Number of modules along side of row",            "",       "",                              "pvsamv1",              "subarray4_enable=1",       "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "subarray4_backtrack",                         "Sub-array 4 Backtracking enabled",                        "",       "0=no backtracking,1=backtrack", "pvsamv1",              "",                         "BOOLEAN",                       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "module_model",                                "Photovoltaic module model specifier",                     "",       "0=spe,1=cec,2=6par_user,3=snl,4=sd11-iec61853,5=PVYield", "pvsamv1",              "*",                        "INTEGER,MIN=0,MAX=5",           "" },
>>>>>>> pr/11
	{ SSC_INPUT,        SSC_NUMBER,      "module_aspect_ratio",                         "Module aspect ratio",                                     "",       "",                              "pvsamv1",              "?=1.7",                    "",                              "POSITIVE" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_area",                                    "Module area",                                             "m2",     "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_rad0",                                    "Irradiance level 0",                                      "W/m2",   "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_rad1",                                    "Irradiance level 1",                                      "W/m2",   "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_rad2",                                    "Irradiance level 2",                                      "W/m2",   "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_rad3",                                    "Irradiance level 3",                                      "W/m2",   "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_rad4",                                    "Irradiance level 4",                                      "W/m2",   "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_eff0",                                    "Efficiency at irradiance level 0",                        "%",      "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_eff1",                                    "Efficiency at irradiance level 1",                        "%",      "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_eff2",                                    "Efficiency at irradiance level 2",                        "%",      "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_eff3",                                    "Efficiency at irradiance level 3",                        "%",      "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_eff4",                                    "Efficiency at irradiance level 4",                        "%",      "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_reference",                               "Reference irradiance level",                              "",       "",                              "pvsamv1",              "module_model=0",           "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_module_structure",                        "Mounting and module structure",                           "",       "0=glass/cell/polymer sheet - open rack,1=glass/cell/glass - open rack,2=polymer/thin film/steel - open rack,3=Insulated back, building-integrated PV,4=close roof mount,5=user-defined",                      "pvsamv1",       "module_model=0",                    "INTEGER,MIN=0,MAX=5",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_a",                                       "Cell temp parameter a",                                   "",       "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_b",                                       "Cell temp parameter b",                                   "",       "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_dT",                                      "Cell temp parameter dT",                                  "",       "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_temp_coeff",                              "Temperature coefficient",                                 "%/C",    "",                              "pvsamv1",              "module_model=0",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_fd",                                      "Diffuse fraction",                                        "0..1",   "",                              "pvsamv1",              "module_model=0",           "MIN=0,MAX=1",                   "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_vmp",                                     "Nominal max power voltage",                               "V",      "",                              "pvsamv1",              "module_model=0",           "POSITIVE",                      "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_voc",                                     "Nominal open circuit voltage",                            "V",      "",                              "pvsamv1",              "module_model=0",           "POSITIVE",                      "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_is_bifacial",                             "Modules are bifacial",                                     "0/1",     "",                            "pvsamv1",              "module_model=0",            "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_bifacial_transmission_factor",            "Bifacial transmission factor",                             "0-1",     "",                            "pvsamv1",              "module_model=0",            "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_bifaciality",                             "Bifaciality factor",                                       "%",       "",                            "pvsamv1",              "module_model=0",            "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "spe_bifacial_ground_clearance_height",        "Module ground clearance height",                           "m",       "",                            "pvsamv1",              "module_model=0",            "",                              "" },


	{ SSC_INPUT,        SSC_NUMBER,      "cec_area",                                    "Module area",                                             "m2",     "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_a_ref",                                   "Nonideality factor a",                                    "",       "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_adjust",                                  "Temperature coefficient adjustment",                      "%",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_alpha_sc",                                "Short circuit current temperature coefficient",           "A/C",    "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_beta_oc",                                 "Open circuit voltage temperature coefficient",            "V/C",    "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_gamma_r",                                 "Maximum power point temperature coefficient",             "%/C",    "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_i_l_ref",                                 "Light current",                                           "A",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_i_mp_ref",                                "Maximum power point current",                             "A",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_i_o_ref",                                 "Saturation current",                                      "A",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_i_sc_ref",                                "Short circuit current",                                   "A",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_n_s",                                     "Number of cells in series",                               "",       "",                              "pvsamv1",              "module_model=1",           "POSITIVE",                      "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_r_s",                                     "Series resistance",                                       "ohm",    "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_r_sh_ref",                                "Shunt resistance",                                        "ohm",    "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_t_noct",                                  "Nominal operating cell temperature",                      "C",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_v_mp_ref",                                "Maximum power point voltage",                             "V",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_v_oc_ref",                                "Open circuit voltage",                                    "V",      "",                              "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_temp_corr_mode",                          "Cell temperature model selection",                        "",       "0=noct,1=mc",                   "pvsamv1",              "module_model=1",           "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_is_bifacial",                             "Modules are bifacial",                                     "0/1",     "",                            "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_bifacial_transmission_factor",            "Bifacial transmission factor",                             "0-1",     "",                            "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_bifaciality",                             "Bifaciality factor",                                       "%",       "",                            "pvsamv1",              "module_model=1",           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_bifacial_ground_clearance_height",        "Module ground clearance height",                           "m",       "",                            "pvsamv1",              "module_model=1",           "",                              "" },


	{ SSC_INPUT,        SSC_NUMBER,      "cec_standoff",                                "Standoff mode",                                           "",       "0=bipv,1=>3.5in,2=2.5-3.5in,3=1.5-2.5in,4=0.5-1.5in,5=<0.5in,6=ground/rack",  "pvsamv1",       "module_model=1",                           "INTEGER,MIN=0,MAX=6",       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_height",                                  "Array mounting height",                                   "",       "0=one story,1=two story",                                           "pvsamv1",       "module_model=1",                           "INTEGER,MIN=0,MAX=1",       "" },

	{ SSC_INPUT,        SSC_NUMBER,      "cec_mounting_config",                         "Mounting configuration",                                  "",       "0=rack,1=flush,2=integrated,3=gap",                                 "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "INTEGER,MIN=0,MAX=3",       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_heat_transfer",                           "Heat transfer dimensions",                                "",       "0=module,1=array",                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "INTEGER,MIN=0,MAX=1",       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_mounting_orientation",                    "Mounting structure orientation",                          "",       "0=do not impede flow,1=vertical supports,2=horizontal supports",    "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "INTEGER,MIN=0,MAX=2",       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_gap_spacing",                             "Gap spacing",                                             "m",      "",                                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "",                          "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_module_width",                            "Module width",                                            "m",      "",                                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "",                          "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_module_length",                           "Module height",                                           "m",      "",                                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "",                          "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_array_rows",                              "Rows of modules in array",                                "",       "",                                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "",                          "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_array_cols",                              "Columns of modules in array",                             "",       "",                                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "",                          "" },
	{ SSC_INPUT,        SSC_NUMBER,      "cec_backside_temp",                           "Module backside temperature",                             "C",      "",                                                                  "pvsamv1",       "module_model=1&cec_temp_corr_mode=1",      "POSITIVE",                  "" },

	{ SSC_INPUT,        SSC_NUMBER,      "6par_celltech",                               "Solar cell technology type",                              "",       "monoSi=0,multiSi=1,CdTe=2,CIS=3,CIGS=4,Amorphous=5",                "pvsamv1",       "module_model=2",                           "INTEGER,MIN=0,MAX=5",       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_vmp",                                    "Maximum power point voltage",                             "V",      "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_imp",                                    "Imp",                                                     "A",      "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_voc",                                    "Voc",                                                     "V",      "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_isc",                                    "Isc",                                                     "A",      "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_bvoc",                                   "Open circuit voltage temperature coefficient",            "V/C",    "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_aisc",                                   "Short circuit current temperature coefficient",           "A/C",    "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_gpmp",                                   "Maximum power point temperature coefficient",             "%/C",    "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_nser",                                   "Nseries",                                                 "",       "",                                                                  "pvsamv1",       "module_model=2",                           "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_area",                                   "Module area",                                             "m2",     "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_tnoct",                                  "Nominal operating cell temperature",                      "C",      "",                                                                  "pvsamv1",       "module_model=2",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_standoff",                               "Standoff mode",                                           "",       "0=bipv,1=>3.5in,2=2.5-3.5in,3=1.5-2.5in,4=0.5-1.5in,5=<0.5in,6=ground/rack",  "pvsamv1",       "module_model=2",                           "INTEGER,MIN=0,MAX=6",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_mounting",                               "Array mounting height",                                   "",       "0=one story,1=two story",                                           "pvsamv1",       "module_model=2",                           "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_is_bifacial",                            "Modules are bifacial",                                     "0/1",     "",                                                                "pvsamv1",       "module_model=2",                          "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_bifacial_transmission_factor",           "Bifacial transmission factor",                             "0-1",     "",                                                                "pvsamv1",       "module_model=2",                          "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_bifaciality",                            "Bifaciality factor",                                       "%",       "",                                                                "pvsamv1",       "module_model=2",                          "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "6par_bifacial_ground_clearance_height",        "Module ground clearance height",                           "m",       "",                                                               "pvsamv1",       "module_model=2",                          "",                              "" },


	{ SSC_INPUT,        SSC_NUMBER,      "snl_module_structure",                        "Module and mounting structure configuration",             "",       "0=Use Database Values,1=glass/cell/polymer sheet - open rack,2=glass/cell/glass - open rack,3=polymer/thin film/steel - open rack,4=Insulated back building-integrated PV,5=close roof mount,6=user-defined",                      "pvsamv1",       "module_model=3",                    "INTEGER,MIN=0,MAX=6",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_a",                                       "Temperature coefficient a",                               "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b",                                       "Temperature coefficient b",                               "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_dtc",                                     "Temperature coefficient dT",                              "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_ref_a",                                   "User-specified a",                                        "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_ref_b",                                   "User-specified b",                                        "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_ref_dT",                                  "User-specified dT",                                       "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_fd",                                      "Diffuse fraction",                                        "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_a0",                                      "Air mass polynomial coeff 0",                             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_a1",                                      "Air mass polynomial coeff 1",                             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_a2",                                      "Air mass polynomial coeff 2",                             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_a3",                                      "Air mass polynomial coeff 3",                             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_a4",                                      "Air mass polynomial coeff 4",                             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_aimp",                                    "Max power point current temperature coefficient",         "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_aisc",                                    "Short circuit current temperature coefficient",           "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_area",                                    "Module area",                                             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b0",                                      "Incidence angle modifier polynomial coeff 0",             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b1",                                      "Incidence angle modifier polynomial coeff 1",             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b2",                                      "Incidence angle modifier polynomial coeff 2",             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b3",                                      "Incidence angle modifier polynomial coeff 3",             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b4",                                      "Incidence angle modifier polynomial coeff 4",             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_b5",                                      "Incidence angle modifier polynomial coeff 5",             "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_bvmpo",                                   "Max power point voltage temperature coefficient",         "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_bvoco",                                   "Open circuit voltage temperature coefficient",            "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c0",                                      "C0",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c1",                                      "C1",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c2",                                      "C2",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c3",                                      "C3",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c4",                                      "C4",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c5",                                      "C5",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c6",                                      "C6",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_c7",                                      "C7",                                                      "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_impo",                                    "Max power point current",                                 "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_isco",                                    "Short circuit current",                                   "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_ixo",                                     "Ix midpoint current",                                     "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_ixxo",                                    "Ixx midpoint current",                                    "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_mbvmp",                                   "Irradiance dependence of Vmp temperature coefficient",    "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_mbvoc",                                   "Irradiance dependence of Voc temperature coefficient",    "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_n",                                       "Diode factor",                                            "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_series_cells",                            "Number of cells in series",                               "",       "",                      "pvsamv1",       "module_model=3",                    "INTEGER",                       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_vmpo",                                    "Max power point voltage",                                 "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "snl_voco",                                    "Open circuit voltage",                                    "",       "",                      "pvsamv1",       "module_model=3",                    "",                              "" },
	
	//{ SSC_INPUT,        SSC_NUMBER,      "sd11par_type",                                "Cell technology type",                                    "",       "monoSi=0,multiSi=1,CdTe=2,CIS=3,CIGS=4,Amorphous=5",                "pvsamv1",       "module_model=4",                           "INTEGER,MIN=0,MAX=5",       "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_nser",                                "Nseries",                                                 "",       "",                                                                  "pvsamv1",       "module_model=4",                           "INTEGER,POSITIVE",              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_area",                                "Module area",                                             "m2",     "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_AMa0",                                "Air mass modifier coeff 0",                               "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_AMa1",                                "Air mass modifier coeff 1",                               "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_AMa2",                                "Air mass modifier coeff 2",                               "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_AMa3",                                "Air mass modifier coeff 3",                               "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_AMa4",                                "Air mass modifier coeff 4",                               "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_glass",                               "Module cover glass type",                                        "",       "0=normal,1=AR glass",                                               "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_tnoct",                               "Nominal operating cell temperature",                      "C",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_standoff",                            "Standoff mode",                                           "",       "0=bipv,1=>3.5in,2=2.5-3.5in,3=1.5-2.5in,4=0.5-1.5in,5=<0.5in,6=ground/rack",  "pvsamv1",       "module_model=4",                 "INTEGER,MIN=0,MAX=6",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_mounting",                            "Array mounting height",                                   "",       "0=one story,1=two story",                                           "pvsamv1",       "module_model=4",                           "INTEGER,MIN=0,MAX=1",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Vmp0",                                "Vmp (STC)",                                               "V",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Imp0",                                "Imp (STC)",                                               "A",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Voc0",                                "Voc (STC)",                                               "V",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Isc0",                                "Isc (STC)",                                               "A",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_alphaIsc",                            "Short curcuit current temperature coefficient",            "A/C",    "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_n",                                   "Diode nonideality factor",                                "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Il",                                  "Light current",                                           "A",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Io",                                  "Saturation current",                                      "A",      "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_Egref",                               "Bandgap voltage",                                         "eV",     "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_d1",                                  "Rs fit parameter 1",                                      "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_d2",                                  "Rs fit parameter 2",                                      "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_d3",                                  "Rs fit parameter 3",                                      "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_c1",                                  "Rsh fit parameter 1",                                     "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_c2",                                  "Rsh fit parameter 2",                                     "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "sd11par_c3",                                  "Rsh fit parameter 3",                                     "",       "",                                                                  "pvsamv1",       "module_model=4",                           "",                              "" },
	

	{ SSC_INPUT,        SSC_NUMBER,      "mlm_N_series",                                "Number of cells in series",                               "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_N_parallel",                              "Number of cells in parallel",						       "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_N_diodes",                                "Number of diodes",									       "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_Width",                                   "Module width (short side)",						       "m",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_Length",                                  "Module length (long side)",							   "m",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_V_mp_ref",                                "V_mp at STC",                                             "V",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_I_mp_ref",                                "I_mp at STC",                                             "A",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_V_oc_ref",                                "V_oc at STC",                                             "V",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_I_sc_ref",                                "I_sc at STC",                                             "A",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_S_ref",                                   "Reference irradiance (Typically 1000W/m²)",               "W/m²",    "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_ref",                                   "Reference temperature (Typically 25°C)",                  "°C",      "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_R_shref",                                 "Reference shunt resistance",                              "V/A",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_R_sh0",                                   "Rsh,0",                                                   "V/A",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_R_shexp",                                 "Rsh exponential coefficient",                             "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_R_s",                                     "Series resistance",                                       "V/A",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_alpha_isc",                               "Temperature coefficient for I_sc",                        "A/K",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_beta_voc_spec",                           "Temperature coefficient for V_oc",                        "V/K",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_E_g",                                     "Reference bandgap energy",                                "eV",      "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_n_0",                                     "Gamma",                                                   "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_mu_n",                                    "Temperature coefficient of gamma",                        "1/K",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_D2MuTau",                                 "Coefficient for recombination losses",                    "V",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_mode",                                  "Cell temperature model mode",                             "-",       "1: NOCT",                                                           "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_c_no_tnoct",                            "NOCT cell temperature",                                   "°C",      "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_c_no_mounting",                         "NOCT Array mounting height",                              "-",       "0=one story,1=two story",                                           "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_c_no_standoff",                         "NOCT standoff mode",                                      "-",       "0=bipv,1=>3.5in,2=2.5-3.5in,3=1.5-2.5in,4=0.5-1.5in,5=<0.5in,6=ground/rack",  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_c_fa_alpha",                            "Extended Faiman model absorptivity",                      "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_c_fa_U0",                               "Extended Faiman model U_0",                               "W/m²K",   "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_T_c_fa_U1",                               "Extended Faiman model U_1",                               "W/m³sK",  "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_mode",                                 "Air-mass modifier mode",                                  "-",       "1: Do not consider AM effects, 2: Use Sandia polynomial [corr=f(AM)], 3: Use standard coefficients from DeSoto model [corr=f(AM)], 4: Use First Solar polynomial [corr=f(AM, p_wat)]",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_sa0",                                "Coefficient 0 for Sandia Air Mass Modifier",              "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_sa1",                                "Coefficient 1 for Sandia Air Mass Modifier",              "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_sa2",                                "Coefficient 2 for Sandia Air Mass Modifier",              "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_sa3",                                "Coefficient 3 for Sandia Air Mass Modifier",              "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_sa4",                                "Coefficient 4 for Sandia Air Mass Modifier",              "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_lp0",                                "Coefficient 0 for Lee/Panchula Air Mass Modifier",        "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_lp1",                                "Coefficient 1 for Lee/Panchula Air Mass Modifier",        "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_lp2",                                "Coefficient 2 for Lee/Panchula Air Mass Modifier",        "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_lp3",                                "Coefficient 3 for Lee/Panchula Air Mass Modifier",        "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_lp4",                                "Coefficient 4 for Lee/Panchula Air Mass Modifier",        "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_AM_c_lp5",                                "Coefficient 5 for Lee/Panchula Air Mass Modifier",        "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_mode",                                "Incidence Angle Modifier mode",                           "-",       "1: Use ASHRAE formula, 2: Use Sandia polynomial, 3: Use cubic spline with user-supplied data",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_as",                                "ASHRAE incidence modifier coefficient b_0",               "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_sa0",                               "Sandia IAM coefficient 0",                                "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_sa1",                               "Sandia IAM coefficient 1",                                "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_sa2",                               "Sandia IAM coefficient 2",                                "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_sa3",                               "Sandia IAM coefficient 3",                                "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_sa4",                               "Sandia IAM coefficient 4",                                "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_IAM_c_sa5",                               "Sandia IAM coefficient 5",                                "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "mlm_IAM_c_cs_incAngle",                       "Spline IAM - Incidence angles",                           "deg",     "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "mlm_IAM_c_cs_iamValue",                       "Spline IAM - IAM values",                                 "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mlm_groundRelfectionFraction",                "Ground reflection fraction",                              "-",       "",                                                                  "pvsamv1",       "module_model=5",                           "",                              "" },

// inverter model
<<<<<<< HEAD
	{ SSC_INPUT,        SSC_NUMBER,      "inverter_model",                              "Inverter model specifier",                                "",        "0=cec,1=datasheet,2=partload,3=coefficientgenerator",        "pvsamv1",               "*",                         "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mppt_low_inverter",                           "Minimum inverter MPPT voltage window",                    "Vdc",     "",                     "pvsamv1",       "",                    "?=0",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mppt_hi_inverter",                            "Maximum inverter MPPT voltage window",                    "Vdc",     "",                     "pvsamv1",       "",                    "?=0",                              "" },
=======
	{ SSC_INPUT,        SSC_NUMBER,      "inverter_model",                              "Inverter model specifier",                                "",        "0=cec,1=datasheet,2=partload,3=coefficientgenerator,4=PVYield",        "pvsamv1",               "*",                         "INTEGER,MIN=0,MAX=4",           "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mppt_low_inverter",                           "Minimum inverter MPPT voltage window",                    "Vdc",     "",                     "pvsamv1",       "?=0",                                 "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "mppt_hi_inverter",                            "Maximum inverter MPPT voltage window",                    "Vdc",     "",                     "pvsamv1",       "?=0",                                 "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_num_mppt",                                "Number of MPPT inputs",                                   "",        "",                     "pvsamv1",       "?=1",                                 "INTEGER,MIN=0,MAX=4",           "" },
>>>>>>> pr/11

	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_c0",                                  "Curvature between AC power and DC power at ref",          "1/W",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_c1",                                  "Coefficient of Pdco variation with DC input voltage",     "1/V",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_c2",                                  "Coefficient of Pso variation with DC input voltage",      "1/V",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_c3",                                  "Coefficient of Co variation with DC input voltage",       "1/V",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_paco",                                "AC maximum power rating",                                 "Wac",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_pdco",                                "DC input power at which AC power rating is achieved",     "Wdc",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_pnt",                                 "AC power consumed by inverter at night",                  "Wac",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_pso",                                 "DC power required to enable the inversion process",       "Wdc",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_vdco",                                "DC input voltage for the rated AC power rating",          "Vdc",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_snl_vdcmax",                              "Maximum DC input operating voltage",                      "Vdc",     "",                     "pvsamv1",       "inverter_model=0",                    "",                              "" },

<<<<<<< HEAD
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_c0", "Curvature between AC power and DC power at ref", "1/W", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_c1", "Coefficient of Pdco variation with DC input voltage", "1/V", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_c2", "Coefficient of Pso variation with DC input voltage", "1/V", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_c3", "Coefficient of Co variation with DC input voltage", "1/V", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_paco", "AC maximum power rating", "Wac", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_pdco", "DC input power at which AC power rating is achieved", "Wdc", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_pnt", "AC power consumed by inverter at night", "Wac", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_psco", "DC power required to enable the inversion process", "Wdc", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_vdco", "DC input voltage for the rated AC power rating", "Vdc", "", "pvsamv1", "inverter_model=3", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "inv_cec_cg_vdcmax", "Maximum DC input operating voltage", "Vdc", "", "pvsamv1", "inverter_model=3", "", "" },

	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_paco",                                "AC maximum power rating",                                 "Wac",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_eff",                                 "Weighted or Peak or Nominal Efficiency",     "Wdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_pnt",                                 "AC power consumed by inverter at night",                  "Wac",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_pso",                                 "DC power required to enable the inversion process",       "Wdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_vdco",                                "DC input voltage for the rated AC power rating",          "Vdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_vdcmax",                              "Maximum DC input operating voltage",                      "Vdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },

	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_paco",                                "AC maximum power rating",                                 "Wac",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_pdco",                                "DC input power at which AC power rating is achieved",     "Wdc",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "inv_pd_partload",                            "Partload curve partload values",                          "%",       "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "inv_pd_efficiency",                          "Partload curve efficiency values",                        "%",       "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_pnt",                                 "AC power consumed by inverter at night",                  "Wac",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_vdco",                                "DC input voltage for the rated AC power rating",          "Vdc",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_vdcmax",                              "Maximum DC input operating voltage",                      "Vdc",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	
=======
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_c0",								"Curvature between AC power and DC power at ref",		   "1/W",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_c1",								"Coefficient of Pdco variation with DC input voltage",	   "1/V",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_c2",								"Coefficient of Pso variation with DC input voltage",	   "1/V",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_c3",								"Coefficient of Co variation with DC input voltage",	   "1/V",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_paco",								"AC maximum power rating",								   "Wac",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_pdco",								"DC input power at which AC power rating is achieved",	   "Wdc",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_pnt",								"AC power consumed by inverter at night",				   "Wac",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_psco",								"DC power required to enable the inversion process",	   "Wdc",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_vdco",								"DC input voltage for the rated AC power rating",		   "Vdc",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },
	{ SSC_INPUT,		SSC_NUMBER,		 "inv_cec_cg_vdcmax",							"Maximum DC input operating voltage",					   "Vdc",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",                              "" },

	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_paco",                                 "AC maximum power rating",                                 "Wac",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_eff",                                  "Weighted or Peak or Nominal Efficiency",				   "Wdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_pnt",                                  "AC power consumed by inverter at night",                  "Wac",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_pso",                                  "DC power required to enable the inversion process",       "Wdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_vdco",                                 "DC input voltage for the rated AC power rating",          "Vdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_ds_vdcmax",                               "Maximum DC input operating voltage",                      "Vdc",     "",                     "pvsamv1",       "inverter_model=1",                    "",                              "" },

	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_paco",                                 "AC maximum power rating",                                 "Wac",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_pdco",                                 "DC input power at which AC power rating is achieved",     "Wdc",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "inv_pd_partload",                             "Partload curve partload values",                          "%",       "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "inv_pd_efficiency",                           "Partload curve efficiency values",                        "%",       "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_pnt",                                  "AC power consumed by inverter at night",                  "Wac",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_vdco",                                 "DC input voltage for the rated AC power rating",          "Vdc",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },
	{ SSC_INPUT,        SSC_NUMBER,      "inv_pd_vdcmax",                               "Maximum DC input operating voltage",                      "Vdc",     "",                     "pvsamv1",       "inverter_model=2",                    "",                              "" },

>>>>>>> pr/11
	{ SSC_INPUT, SSC_NUMBER, "ond_PNomConv", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_PMaxOUT", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_VOutConv", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_VMppMin", "", "V", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_VMPPMax", "", "V", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_VAbsMax", "", "V", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_PSeuil", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_STRING, "ond_ModeOper", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_STRING, "ond_CompPMax", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_STRING, "ond_CompVMax", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_STRING, "ond_ModeAffEnum", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_PNomDC", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_PMaxDC", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_IMaxDC", "", "A", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_INomDC", "", "A", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_INomAC", "", "A", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_IMaxAC", "", "A", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_TPNom", "", "°C", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_TPMax", "", "°C", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_TPLim1", "", "°C", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_TPLimAbs", "", "°C", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_PLim1", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_PLimAbs", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_ARRAY,  "ond_VNomEff", "", "V", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_NbInputs", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_NbMPPT", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_Aux_Loss", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_Night_Loss", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_lossRDc", "", "V/A", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_lossRAc", "", "A", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_effCurve_elements", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_MATRIX, "ond_effCurve_Pdc", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_MATRIX, "ond_effCurve_Pac", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_MATRIX, "ond_effCurve_eta", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_Aux_Loss", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_Aux_Loss", "", "W", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_doAllowOverpower", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
	{ SSC_INPUT, SSC_NUMBER, "ond_doUseTemperatureLimit", "", "-", "", "pvsamv1", "inverter_model=4", "", "" },
<<<<<<< HEAD
=======
	{ SSC_INPUT,		SSC_MATRIX,		 "inv_tdc_cec_db",							   "Temperature derate curves for CEC Database",			   "Vdc",	  "",					  "pvsamv1",	   "inverter_model=0",					  "",							   "" },
	{ SSC_INPUT,		SSC_MATRIX,		 "inv_tdc_cec_cg",							   "Temperature derate curves for CEC Coef Gen",			   "Vdc",	  "",					  "pvsamv1",	   "inverter_model=3",					  "",							   "" },
	{ SSC_INPUT,		SSC_MATRIX,		 "inv_tdc_ds",								   "Temperature derate curves for Inv Datasheet",			   "Vdc",	  "",					  "pvsamv1",	   "inverter_model=1",					  "",							   "" },
	{ SSC_INPUT,		SSC_MATRIX,		 "inv_tdc_plc",								   "Temperature derate curves for Part Load Curve",			   "C",		  "",					  "pvsamv1",	   "inverter_model=2",					  "",							   "" },
>>>>>>> pr/11

	// battery storage and dispatch
	{ SSC_INPUT,        SSC_NUMBER,      "en_batt",                                    "Enable battery storage model",                             "0/1",     "",                     "Battery",       "?=0",                                 "",                              "" },
	{ SSC_INPUT,        SSC_ARRAY,       "load",                                       "Electricity load (year 1)",                                "kW", "", "Battery", "?", "", "" },

	// NOTE:  other battery storage model inputs and outputs are defined in batt_common.h/batt_common.cpp

	// outputs

/* environmental conditions */
	// irradiance data from weather file
	{ SSC_OUTPUT,        SSC_ARRAY,      "gh",                                         "Irradiance GHI from weather file",                                     "W/m2",   "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "dn",                                         "Irradiance DNI from weather file",                                     "W/m2",   "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "df",                                         "Irradiance DHI from weather file",                                     "W/m2",   "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "wfpoa",                                      "Irradiance POA from weather file",                                     "W/m2",   "",                      "Time Series",       "",                     "",                              "" },

	//not all of these three calculated values will be reported, based on irrad_mode selection
	{ SSC_OUTPUT,        SSC_ARRAY,      "gh_calc",                                    "Irradiance GHI calculated",                                       "W/m2",   "",                      "Time Series",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "dn_calc",                                    "Irradiance DNI calculated",                                       "W/m2",   "",                      "Time Series",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "df_calc",                                    "Irradiance DHI calculated",                                       "W/m2",   "",                      "Time Series",       "",                     "",                              "" },

	// non-irradiance data from weather file
	{ SSC_OUTPUT,        SSC_ARRAY,      "wspd",                                       "Weather file wind speed",                                                        "m/s",    "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "tdry",                                       "Weather file ambient temperature",                                               "C",      "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "alb",                                        "Weather file albedo",							                                 "",       "",                     "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "snowdepth",                                  "Weather file snow depth",							                            "cm",       "",                    "Time Series",       "",                    "",                              "" },

	// calculated sun position data
	{ SSC_OUTPUT,        SSC_ARRAY,      "sol_zen",                                    "Sun zenith angle",                                                  "deg",    "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "sol_alt",                                    "Sun altitude angle",                                                "deg",    "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "sol_azi",                                    "Sun azimuth angle",                                                 "deg",    "",                      "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "sunup",                                      "Sun up over horizon",                                               "0/1/2/3", "",                     "Time Series",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "sunpos_hour",                                "Sun position time",                                     "hour",   "",                      "Time Series",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "airmass",                                    "Absolute air mass",                                                 "",       "",                      "Time Series",       "*",                    "",                              "" },

	/* sub-array level outputs */
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_surf_tilt",                  "Subarray 1 Surface tilt",                                              "deg",    "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_surf_azi",                   "Subarray 1 Surface azimuth",                                           "deg",    "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_aoi",                        "Subarray 1 Angle of incidence",                                        "deg",    "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_aoi_modifier",               "Subarray 1 Angle of incidence Modifier",                               "0-1",    "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_axisrot",                    "Subarray 1 Axis rotation for 1 axis trackers",                         "deg",    "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_idealrot",                   "Subarray 1 Axis rotation ideal for 1 axis trackers",                   "deg",    "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_eff_beam",               "Subarray 1 POA front beam irradiance after shading and soiling",       "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_eff_diff",               "Subarray 1 POA front diffuse irradiance after shading and soiling",    "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_nom",                    "Subarray 1 POA front total irradiance nominal",                        "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_shaded",                 "Subarray 1 POA front total irradiance after shading only",             "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_shaded_soiled",          "Subarray 1 POA front total irradiance after shading and soiling",      "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_front",                  "Subarray 1 POA front total irradiance after reflection (IAM)",              "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_rear",                   "Subarray 1 POA rear total irradiance after reflection (IAM)",              "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_poa_eff",                    "Subarray 1 POA total irradiance after reflection (IAM)",                   "W/m2",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_soiling_derate",             "Subarray 1 Soiling beam irradiance factor",                            "frac",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_beam_shading_factor",        "Subarray 1 External shading and soiling beam irradiance factor",       "frac",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_linear_derate",              "Subarray 1 Self-shading linear beam irradiance factor",                "frac",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_ss_diffuse_derate",          "Subarray 1 Self-shading non-linear sky diffuse irradiance factor",     "frac",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_ss_reflected_derate",        "Subarray 1 Self-shading non-linear ground diffuse irradiance factor",  "frac",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_ss_derate",                  "Subarray 1 Self-shading non-linear DC factor",                         "frac",   "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "shadedb_subarray1_shade_frac",         "Subarray 1 Partial external shading DC factor",                        "frac",   "", "Time Series (Subarray 1)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_snow_coverage",              "Subarray 1 Snow cover",                                                "0..1",   "", "Time Series (Subarray 1)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_snow_loss",                  "Subarray 1 Snow cover DC power loss",                                  "kW",     "", "Time Series (Subarray 1)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_modeff",                     "Subarray 1 Module efficiency",                                         "%",      "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_celltemp",                   "Subarray 1 Cell temperature",                                          "C",      "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_dc_voltage",                 "Subarray 1 Operating DC voltage",                                         "V",      "", "Time Series (Subarray 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_voc",                        "Subarray 1 Open circuit DC voltage",                                      "V",      "", "Time Series (Subarray 1)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray1_isc",                        "Subarray 1 Short circuit DC current",                                     "A",      "", "Time Series (Subarray 1)",       "",                     "",                              "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_surf_tilt",                  "Subarray 2 Surface tilt",                                              "deg",    "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_surf_azi",                   "Subarray 2 Surface azimuth",                                           "deg",    "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_aoi",                        "Subarray 2 Angle of incidence",                                        "deg",    "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_aoi_modifier",               "Subarray 2 Angle of incidence Modifier",                               "0-1",    "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_axisrot",                    "Subarray 2 Axis rotation for 1 axis trackers",                         "deg",    "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_idealrot",                   "Subarray 2 Axis rotation ideal for 1 axis trackers",                   "deg",    "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_eff_beam",               "Subarray 2 POA front beam irradiance after shading and soiling",       "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_eff_diff",               "Subarray 2 POA front diffuse irradiance after shading and soiling",    "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_nom",                    "Subarray 2 POA front total irradiance nominal",                        "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_shaded",                 "Subarray 2 POA front total irradiance after shading only",             "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_shaded_soiled",          "Subarray 2 POA front total irradiance after shading and soiling",      "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_front",                  "Subarray 2 POA front total irradiance after reflection (IAM)",                     "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_rear",                   "Subarray 2 POA rear irradiance after reflection (IAM)",                            "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_poa_eff",                    "Subarray 2 POA total irradiance after module reflection (IAM)",                   "W/m2",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_soiling_derate",             "Subarray 2 Soiling beam irradiance factor",                            "frac",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_beam_shading_factor",        "Subarray 2 External shading and soiling beam irradiance factor",       "frac",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_linear_derate",              "Subarray 2 Self-shading linear beam irradiance factor",                "frac",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_ss_diffuse_derate",          "Subarray 2 Self-shading non-linear sky diffuse irradiance factor",     "frac",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_ss_reflected_derate",        "Subarray 2 Self-shading non-linear ground diffuse irradiance factor",  "frac",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_ss_derate",                  "Subarray 2 Self-shading non-linear DC factor",                         "frac",   "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "shadedb_subarray2_shade_frac",         "Subarray 2 Partial shading DC factor",                                 "frac",   "", "Time Series (Subarray 2)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_snow_coverage",				 "Subarray 2 Snow cover",                                                "0..1",   "", "Time Series (Subarray 2)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_snow_loss",					 "Subarray 2 Snow cover DC power loss",                                  "kW",     "", "Time Series (Subarray 2)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_modeff",                     "Subarray 2 Module efficiency",                                         "%",      "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_celltemp",                   "Subarray 2 Cell temperature",                                          "C",      "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_dc_voltage",                 "Subarray 2 Operating DC voltage",                                         "V",      "", "Time Series (Subarray 2)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_voc",                        "Subarray 2 Open circuit DC voltage",                                      "V",      "", "Time Series (Subarray 2)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray2_isc",                        "Subarray 2 Short circuit DC current",                                     "A",      "", "Time Series (Subarray 2)",       "",                     "",                              "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_surf_tilt",                  "Subarray 3 Surface tilt",                                              "deg",    "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_surf_azi",                   "Subarray 3 Surface azimuth",                                           "deg",    "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_aoi",                        "Subarray 3 Angle of incidence",                                        "deg",    "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_aoi_modifier",               "Subarray 3 Angle of incidence Modifier",                               "0-1",    "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_axisrot",                    "Subarray 3 Axis rotation for 1 axis trackers",                         "deg",    "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_idealrot",                   "Subarray 3 Axis rotation ideal for 1 axis trackers",                   "deg",    "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_eff_beam",               "Subarray 3 POA front beam irradiance after shading and soiling",       "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_eff_diff",               "Subarray 3 POA front diffuse irradiance after shading and soiling",    "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_nom",                    "Subarray 3 POA font total irradiance nominal",                        "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_shaded",                 "Subarray 3 POA front total irradiance after shading only",             "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_shaded_soiled",          "Subarray 3 POA front total irradiance after shading and soiling",      "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_front",                  "Subarray 3 POA front total irradiance after reflection (IAM)",					 "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_rear",                   "Subarray 3 POA rear irradiance after reflection (IAM)",                            "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_poa_eff",                    "Subarray 3 POA total irradiance after reflection (IAM)",                          "W/m2",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_soiling_derate",             "Subarray 3 Soiling beam irradiance factor",                            "frac",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_beam_shading_factor",        "Subarray 3 External shading and soiling beam irradiance factor",       "frac",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_linear_derate",              "Subarray 3 Self-shading linear beam irradiance factor",                "frac",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_ss_diffuse_derate",          "Subarray 3 Self-shading non-linear sky diffuse irradiance factor",     "frac",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_ss_reflected_derate",        "Subarray 3 Self-shading non-linear ground diffuse irradiance factor",  "frac",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_ss_derate",                  "Subarray 3 Self-shading non-linear DC factor",                         "frac",   "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "shadedb_subarray3_shade_frac",         "Subarray 3 Partial external shading DC factor",                        "frac",   "", "Time Series (Subarray 3)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_snow_coverage",				 "Subarray 3 Snow cover",                                                "0..1",   "", "Time Series (Subarray 3)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_snow_loss",					 "Subarray 3 Snow cover DC power loss",			                         "kW",     "", "Time Series (Subarray 3)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_modeff",                     "Subarray 3 Module efficiency",                                         "%",      "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_celltemp",                   "Subarray 3 Cell temperature",                                          "C",      "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_dc_voltage",                 "Subarray 3 Operating DC voltage",                                         "V",      "", "Time Series (Subarray 3)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_voc",                        "Subarray 3 Open circuit DC voltage",                                      "V",      "", "Time Series (Subarray 3)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray3_isc",                        "Subarray 3 Short circuit DC current",                                     "A",      "", "Time Series (Subarray 3)",       "",                     "",                              "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_surf_tilt",                  "Subarray 4 Surface tilt",                                              "deg",    "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_surf_azi",                   "Subarray 4 Surface azimuth",                                           "deg",    "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_aoi",                        "Subarray 4 Angle of incidence",                                        "deg",    "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_aoi_modifier",               "Subarray 4 Angle of incidence Modifier",                               "0-1",    "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_axisrot",                    "Subarray 4 Axis rotation for 1 axis trackers",                         "deg",    "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_idealrot",                   "Subarray 4 Axis rotation ideal for 1 axis trackers",                   "deg",    "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_eff_beam",               "Subarray 4 POA front beam irradiance after shading and soiling",       "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_eff_diff",               "Subarray 4 POA front diffuse irradiance after shading and soiling",    "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_nom",                    "Subarray 4 POA front total irradiance nominal",                        "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_shaded",                 "Subarray 4 POA front total irradiance after shading only",             "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_shaded_soiled",          "Subarray 4 POA front total irradiance after shading and soiling",      "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_front",                  "Subarray 4 POA front total irradiance after reflection (IAM)",                     "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_rear",                   "Subarray 4 POA rear irradiance after reflection (IAM)",                            "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_poa_eff",                    "Subarray 4 POA total irradiance after reflection (IAM)",                          "W/m2",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_soiling_derate",             "Subarray 4 Soiling beam irradiance factor",                            "frac",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_beam_shading_factor",        "Subarray 4 External shading and soiling beam irradiance factor",       "frac",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_linear_derate",              "Subarray 4 Self-shading linear beam irradiance factor",                "frac",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_ss_diffuse_derate",          "Subarray 4 Self-shading non-linear sky diffuse irradiance factor",     "frac",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_ss_reflected_derate",        "Subarray 4 Self-shading non-linear ground diffuse irradiance factor",  "frac",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_ss_derate",                  "Subarray 4 Self-shading non-linear DC factor",                         "frac",   "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "shadedb_subarray4_shade_frac",         "Subarray 4 Partial external shading DC factor",                        "frac",   "", "Time Series (Subarray 4)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_snow_coverage",				 "Subarray 4 Snow cover",                                                "0..1",   "", "Time Series (Subarray 4)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_snow_loss",					 "Subarray 4 Snow cover DC power loss",                                  "kW",     "", "Time Series (Subarray 4)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_modeff",                     "Subarray 4 Module efficiency",                                         "%",      "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_celltemp",                   "Subarray 4 Cell temperature",                                          "C",      "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_dc_voltage",                 "Subarray 4 Operating DC voltage",                                         "V",      "", "Time Series (Subarray 4)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_voc",                        "Subarray 4 Open circuit DC voltage",                                      "V",      "", "Time Series (Subarray 4)",       "",                     "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "subarray4_isc",                        "Subarray 4 Short circuit DC current",                                     "A",      "", "Time Series (Subarray 4)",       "",                     "",                              "" },

/* aggregate array level outputs */
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_nom",                              "Array POA front-side total radiation nominal",                    "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_beam_nom",                         "Array POA front-side beam radiation nominal",                     "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_beam_eff",                         "Array POA beam radiation after shading and soiling",              "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_shaded",                           "Array POA front-side total radiation after shading only",         "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_shaded_soiled",                    "Array POA front-side total radiation after shading and soiling",  "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_front",                            "Array POA front-side total radiation after reflection (IAM)",                "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_rear",                             "Array POA rear-side total radiation after reflection (IAM)",                 "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "poa_eff",                              "Array POA radiation total after reflection (IAM)",                           "kW",   "",  "Time Series (Array)",       "*",                    "",                              "" },

	//SEV: total dc snow loss time series (not a required output) 
	{ SSC_OUTPUT,        SSC_ARRAY,      "dc_snow_loss",                         "Array DC power loss due to snow",						 "kW",   "",   "Time Series (Array)",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "dc_net",                               "Array DC power",                                       "kW",   "",   "Time Series (Array)",       "*",                    "",                              "" },
	
	//mppt outputs
	{ SSC_OUTPUT,        SSC_ARRAY,      "inverterMPPT1_DCVoltage",              "Inverter MPPT 1 Nominal DC voltage",                  "V",    "",  "Time Series (MPPT)",           "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inverterMPPT2_DCVoltage",              "Inverter MPPT 2 Nominal DC voltage",                  "V",    "",  "Time Series (MPPT)",           "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inverterMPPT3_DCVoltage",              "Inverter MPPT 3 Nominal DC voltage",                  "V",    "",  "Time Series (MPPT)",           "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inverterMPPT4_DCVoltage",              "Inverter MPPT 4 Nominal DC voltage",                  "V",    "",  "Time Series (MPPT)",           "",                    "",                              "" },

	//inverter outputs
	{ SSC_OUTPUT,        SSC_ARRAY,      "inv_eff",                              "Inverter efficiency",                                  "%",    "",  "Time Series (Inverter)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "dc_invmppt_loss",                      "Inverter clipping loss DC MPPT voltage limits",         "kW",  "",  "Time Series (Inverter)",       "*",                    "",                              "" },
    { SSC_OUTPUT,        SSC_ARRAY,      "inv_cliploss",                         "Inverter clipping loss AC power limit",                "kW",   "",  "Time Series (Inverter)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inv_psoloss",                          "Inverter power consumption loss",                      "kW",   "",  "Time Series (Inverter)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inv_pntloss",                          "Inverter night time loss",                             "kW",   "",  "Time Series (Inverter)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inv_tdcloss",                       	 "Inverter thermal derate loss",                         "kW",   "",   "Time Series (Inverter)",      "*",             "",                   "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "inv_total_loss",                       "Inverter total power loss",                            "kW",   "",   "Time Series (Inverter)",      "*",             "",                   "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "ac_wiring_loss",                       "AC wiring loss",                                       "kW",   "",   "Time Series (Inverter)",      "*",                        "",                   "" },

	// transformer model outputs
	{ SSC_OUTPUT,        SSC_ARRAY,      "xfmr_nll_ts",                          "Transformer no load loss",                              "kW", "",    "Time Series (Transformer)", "", "", "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "xfmr_ll_ts",                           "Transformer load loss",                                 "kW", "",    "Time Series (Transformer)", "", "", "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "xfmr_loss_ts",                         "Transformer total loss",                                "kW", "",    "Time Series (Transformer)", "", "", "" },

	{ SSC_OUTPUT,        SSC_ARRAY,     "ac_transmission_loss",                   "Transmission loss",                                     "kW", "",    "Time Series (Transmission)",                 "",                     "",                   "" },

	//total losses- not part of loss diagram but now outputs instead of inputs JMF 11/25/15
	{ SSC_OUTPUT,        SSC_NUMBER,     "ac_loss",                              "AC wiring loss",                                       "%",   "",    "Annual (Year 1)",              "*",                        "",                   "" },
	// monthly and annual outputs

	{ SSC_OUTPUT,		 SSC_NUMBER,     "annual_energy",						 "Annual AC energy",                                       "kWh",       "",                      "Annual (Year 1)", "*", "", "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_dc_invmppt_loss",               "Inverter clipping loss DC MPPT voltage limits",          "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_inv_cliploss",                  "Inverter clipping loss AC power limit",                  "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_inv_psoloss",                   "Inverter power consumption loss",                        "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_inv_pntloss",                   "Inverter night time loss",                               "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_inv_tdcloss",                   "Inverter thermal derate loss",				                   "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "subarray1_dcloss",                     "Subarray 1 Total DC power loss",                                       "%",      "", "Annual (Year 1)",              "*",                        "",                   "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "subarray2_dcloss",                     "Subarray 2 Total DC power loss",                                       "%",      "", "Annual (Year 1)",              "",                        "",                   "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "subarray3_dcloss",                     "Subarray 3 Total DC power loss",                                       "%",      "", "Annual (Year 1)",              "",                        "",                   "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "subarray4_dcloss",                     "Subarray 4 Total DC power loss",                                       "%",      "", "Annual (Year 1)",              "",                        "",                   "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "xfmr_nll_year1",                              "Transformer no load loss",                               "kWh/yr", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "xfmr_ll_year1",                               "Transformer load loss",                                  "kWh/yr", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "xfmr_loss_year1",                             "Transformer total loss",                                 "kWh/yr", "", "Annual (Year 1)", "", "", "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_poa_nom",                             "POA front-side irradiance total nominal",                          "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_poa_beam_nom",                        "POA front-side irradiance beam nominal",                           "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_poa_front",                           "POA front-side irradiance total",                       "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_poa_rear",                            "POA rear-side irradiance total",                       "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_poa_eff",                             "POA irradiance total after shading and soiling",          "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_poa_beam_eff",                        "POA front-side irradiance beam after shading and soiling",           "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },

	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_dc",                                  "PV array DC energy",                                   "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_energy",                              "System AC energy",                                     "kWh/mo",    "",                      "Monthly",       "*",                    "LENGTH=12",                              "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_gh",                                   "Annual GHI",                                                    "Wh/m2/yr",  "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_nom",                              "POA front-side irradiance total nominal",                       "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_beam_nom",                         "POA front-side irradiance beam nominal",                        "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_shaded",                           "POA front-side irradiance total after shading",                 "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_shaded_soiled",                    "POA front-side irradiance total after shading and soiling",     "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_front",                            "POA front-side irradiance total after reflection (IAM)",                   "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_rear",                             "POA rear-side irradiance total after reflection (IAM)",                    "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_eff",                              "POA irradiance total after reflection (IAM)",                              "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_poa_beam_eff",                         "POA front-side irradiance beam after shading and soiling",      "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_dc_nominal",                           "Annual DC energy nominal",                           "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_dc_gross",                             "Annual DC energy gross",                             "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_dc_net",                               "Annual DC energy",                                   "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_ac_gross",                             "Annual AC energy gross",                               "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },

	// OND inverter model AC and DC loss reporting
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_dc_loss_ond",                          "Annual DC loss OND model",                           "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_ac_loss_ond",                          "Annual AC loss OND model",                           "kWh/yr",    "",                      "Annual (Year 1)",       "*",                    "",                              "" },

	//SEV: total dc snow loss monthy array and annual value (not a required output) 
	{ SSC_OUTPUT,        SSC_ARRAY,      "monthly_snow_loss",                    "Snow DC energy loss",					       "kWh/mo",    "",                       "Monthly",       "",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "annual_snow_loss",                     "Snow DC energy loss",						   "kWh/yr",    "",                       "Annual (Year 1)",       "",                    "",                              "" },

	// loss diagram - order applied
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray1_dc_gross", "Subarray 1 Gross DC energy", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray1_dc_mismatch_loss", "Subarray 1 DC mismatch loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray1_dc_diodes_loss", "Subarray 1 DC diodes and connections loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray1_dc_wiring_loss", "Subarray 1 DC wiring loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray1_dc_tracking_loss", "Subarray 1 DC tracking loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray1_dc_nameplate_loss", "Subarray 1 DC nameplate loss", "kWh", "", "Annual (Year 1)", "*", "", "" },

	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray2_dc_gross", "Subarray 2 Gross DC energy", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray2_dc_mismatch_loss", "Subarray 2 DC mismatch loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray2_dc_diodes_loss", "Subarray 2 DC diodes and connections loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray2_dc_wiring_loss", "Subarray 2 DC wiring loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray2_dc_tracking_loss", "Subarray 2 DC tracking loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray2_dc_nameplate_loss", "Subarray 2 DC nameplate loss", "kWh", "", "Annual (Year 1)", "", "", "" },

	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray3_dc_gross", "Subarray 3 Gross DC energy", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray3_dc_mismatch_loss", "Subarray 3 DC mismatch loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray3_dc_diodes_loss", "Subarray 3 DC diodes and connections loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray3_dc_wiring_loss", "Subarray 3 DC wiring loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray3_dc_tracking_loss", "Subarray 3 DC tracking loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray3_dc_nameplate_loss", "Subarray 3 DC nameplate loss", "kWh", "", "Annual (Year 1)", "", "", "" },

	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray4_dc_gross", "Subarray 4 Gross DC energy", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray4_dc_mismatch_loss", "Subarray 4 DC mismatch loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray4_dc_diodes_loss", "Subarray 4 DC diodes and connections loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray4_dc_wiring_loss", "Subarray 4 DC wiring loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray4_dc_tracking_loss", "Subarray 4 DC tracking loss", "kWh", "", "Annual (Year 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_subarray4_dc_nameplate_loss", "Subarray 4 DC nameplate loss", "kWh", "", "Annual (Year 1)", "", "", "" },

	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_mismatch_loss", "DC mismatch loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_diodes_loss", "DC diodes and connections loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_wiring_loss", "DC wiring loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_tracking_loss", "DC tracking loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_nameplate_loss", "DC nameplate loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_optimizer_loss", "DC power optimizer loss", "kWh", "", "Annual (Year 1)", "*", "", "" },

	// loss diagram energy outputs nominal poa, nominal array at STC, net dc, net ac, system output
	// annual_poa_nom, annual_dc_nominal, annual_dc_net, annual_ac_net, annual_energy
	// loss diagram % losses
	// annual_poa_nom
	{ SSC_OUTPUT, SSC_NUMBER, "annual_poa_shading_loss_percent", "POA front-side shading loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_poa_soiling_loss_percent", "POA front-side soiling loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_poa_cover_loss_percent",   "POA front-side reflection (IAM) loss",   "%", "", "Loss", "*", "", "" }, 
	{ SSC_OUTPUT, SSC_NUMBER, "annual_poa_rear_gain_percent",    "POA rear-side bifacial gain", "%", "", "Loss", "*", "", "" },

	// annual_dc_nominal
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_module_loss_percent", "DC module modeled loss", "%", "", "Loss", "*", "", "" },
	// annual_dc_gross
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_snow_loss_percent", "DC snow loss", "%", "", "Loss", "*", "", "" },


	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_mppt_clip_loss_percent", "DC inverter MPPT clipping loss", "%", "", "Loss", "*", "", "" },


	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_mismatch_loss_percent", "DC mismatch loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_diodes_loss_percent", "DC diodes and connections loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_wiring_loss_percent", "DC wiring loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_tracking_loss_percent", "DC tracking loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_nameplate_loss_percent", "DC nameplate loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_optimizer_loss_percent", "DC power optimizer loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_perf_adj_loss_percent", "DC performance adjustment loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_lifetime_loss_percent", "Lifetime daily DC loss- year 1", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_battery_loss_percent", "DC connected battery loss- year 1", "%", "", "Loss", "*", "", "" },

	//annual_dc_net
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_inv_clip_loss_percent", "AC inverter power clipping loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_inv_pso_loss_percent", "AC inverter power consumption loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_inv_pnt_loss_percent", "AC inverter night tare loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_inv_tdc_loss_percent", "AC inverter thermal derate loss", "%", "", "Loss", "*", "", "" },
	// annual_ac_gross
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_inv_eff_loss_percent", "AC inverter efficiency loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_wiring_loss_percent", "AC wiring loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_transmission_loss_percent", "Transmission loss", "%", "", "Loss", "*", "", "" },
//	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_transformer_loss_percent", "AC step-up transformer loss", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_lifetime_loss_percent", "Lifetime daily AC loss- year 1", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_battery_loss_percent", "AC connected battery loss- year 1", "%", "", "Loss", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_xfmr_loss_percent", "Transformer loss percent", "%", "", "Loss", "", "", "" },


	// annual_ac_net
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_perf_adj_loss_percent", "AC performance adjustment loss", "%", "", "Loss", "*", "", "" },
	// annual_energy

	/*
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_after_mismatch_loss", "DC output after mismatch loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_after_diodes_loss", "DC output after diodes and connections loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_after_wiring_loss", "DC output after wiring loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_after_tracking_loss", "DC output after tracking loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_after_nameplate_loss", "DC output after nameplate loss", "kWh", "", "Annual (Year 1)", "*", "", "" },

	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_after_inv_cliploss", "AC output after inverter clipping loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_after_inv_psoloss", "AC output after inverter power consumption loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_after_inv_pntloss", "AC output after inverter night tare loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	*/
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_wiring_loss", "AC wiring loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_transmission_loss", "Transmission loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
//	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_transformer_loss", "AC step-up transformer loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_dc_optimizer_loss", "DC power optimizer loss", "kWh", "", "Annual (Year 1)", "*", "", "" },

	/*
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_after_wiring_loss", "AC output after wiring loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT, SSC_NUMBER, "annual_ac_after_transformer_loss", "AC output after step-up transformer loss", "kWh", "", "Annual (Year 1)", "*", "", "" },
	*/

	//

	{ SSC_OUTPUT,        SSC_NUMBER,     "6par_a",                                      "CEC 6-parameter: a",        "",       "", "Module CEC 6-parameter model parameters",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "6par_Io",                                     "CEC 6-parameter: Io",       "",       "", "Module CEC 6-parameter model parameters",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "6par_Il",                                     "CEC 6-parameter: Il",       "",       "", "Module CEC 6-parameter model parameters",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "6par_Rs",                                     "CEC 6-parameter: Rs",       "",       "", "Module CEC 6-parameter model parameters",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "6par_Rsh",                                    "CEC 6-parameter: Rsh",      "",       "", "Module CEC 6-parameter model parameters",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "6par_Adj",                                    "CEC 6-parameter: Adj",      "",       "", "Module CEC 6-parameter model parameters",       "*",                    "",                              "" },

	{ SSC_OUTPUT,        SSC_NUMBER,     "performance_ratio",                           "Performance ratio",         "",       "",  "Annual (Year 1)",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "capacity_factor",                             "Capacity factor",           "%",      "",  "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "capacity_factor_ac",                          "Capacity factor based on AC system capacity",           "%",      "",  "Annual (Year 1)", "*", "", "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "kwh_per_kw",                                  "First year kWh(AC)/kW(DC)", "kWh/kW", "",	"Annual (Year 1)", "*", "", "" },

	//miscellaneous outputs
	{ SSC_OUTPUT,        SSC_NUMBER,     "ts_shift_hours",                            "Sun position time offset",   "hours",  "",  "Miscellaneous", "*",                       "",                          "" },
	{ SSC_OUTPUT,        SSC_NUMBER,     "nameplate_dc_rating",                        "System nameplate DC rating", "kW",     "",  "Miscellaneous",       "*",                    "",                              "" },


// test outputs
#ifdef SHADE_DB_OUTPUTS
	// ShadeDB validation

	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_gpoa", "ShadeDB subarray 1 global poa input", "W/m2", "", "Time Series (Subarray 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_dpoa", "ShadeDB subarray 1 diffuse poa input", "W/m2", "", "Time Series (Subarray 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_pv_cell_temp", "ShadeDB subarray 1 pv cell temp input", "C", "", "Time Series (Subarray 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_mods_per_str", "ShadeDB subarray 1 modules per string input", "", "", "Time Series (Subarray 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_str_vmp_stc", "ShadeDB subarray 1 string Vmp at STC input", "V", "", "Time Series (Subarray 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_mppt_lo", "ShadeDB subarray 1 MPPT low input", "V", "", "Time Series (Subarray 1)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray1_mppt_hi", "ShadeDB subarray 1 MPPT high input", "V", "", "Time Series (Subarray 1)", "", "", "" },

	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_gpoa", "ShadeDB subarray 2 global poa input", "W/m2", "", "Time Series (Subarray 2)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_dpoa", "ShadeDB subarray 2 diffuse poa input", "W/m2", "", "Time Series (Subarray 2)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_pv_cell_temp", "ShadeDB subarray 2 pv cell temp input", "C", "", "Time Series (Subarray 2)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_mods_per_str", "ShadeDB subarray 2 modules per string input", "", "", "Time Series (Subarray 2)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_str_vmp_stc", "ShadeDB subarray 2 string Vmp at STC input", "V", "", "Time Series (Subarray 2)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_mppt_lo", "ShadeDB subarray 2 MPPT low input", "V", "", "Time Series (Subarray 2)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray2_mppt_hi", "ShadeDB subarray 2 MPPT high input", "V", "", "Time Series (Subarray 2)", "", "", "" },

	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_gpoa", "ShadeDB subarray 3 global poa input", "W/m2", "", "Time Series (Subarray 3)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_dpoa", "ShadeDB subarray 3 diffuse poa input", "W/m2", "", "Time Series (Subarray 3)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_pv_cell_temp", "ShadeDB subarray 3 pv cell temp input", "C", "", "Time Series (Subarray 3)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_mods_per_str", "ShadeDB subarray 3 modules per string input", "", "", "Time Series (Subarray 3)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_str_vmp_stc", "ShadeDB subarray 3 string Vmp at STC input", "V", "", "Time Series (Subarray 3)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_mppt_lo", "ShadeDB subarray 3 MPPT low input", "V", "", "Time Series (Subarray 3)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray3_mppt_hi", "ShadeDB subarray 3 MPPT high input", "V", "", "Time Series (Subarray 3)", "", "", "" },


	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_gpoa", "ShadeDB subarray 4 global poa input", "W/m2", "", "Time Series (Subarray 4)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_dpoa", "ShadeDB subarray 4 diffuse poa input", "W/m2", "", "Time Series (Subarray 4)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_pv_cell_temp", "ShadeDB subarray 4 pv cell temp input", "C", "", "Time Series (Subarray 4)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_mods_per_str", "ShadeDB subarray 4 modules per string input", "", "", "Time Series (Subarray 4)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_str_vmp_stc", "ShadeDB subarray 4 string Vmp at STC input", "V", "", "Time Series (Subarray 4)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_mppt_lo", "ShadeDB subarray 4 MPPT low input", "V", "", "Time Series (Subarray 4)", "", "", "" },
	{ SSC_OUTPUT, SSC_ARRAY, "shadedb_subarray4_mppt_hi", "ShadeDB subarray 4 MPPT high input", "V", "", "Time Series (Subarray 4)", "", "", "" },

#endif

	// a couple debugging outputs
	/*
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_dc_derate0",                      "SS1x dc derate",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_derate_X",                        "SS1x X",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_derate_S",                        "SS1x S",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_shad1xf",                         "SS1x shade fraction",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_Ee_ratio",                        "SS1x Ee ratio",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_skyd1xf",                         "SS1x skydiff derate",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	{ SSC_OUTPUT,        SSC_ARRAY,      "p_nonlinear_gndd1xf",                         "SS1x gnddiff derate",                                          "",    "",                      "pvsamv1",       "*",                    "",                              "" },
	*/



var_info_invalid };

cm_pvsamv1::cm_pvsamv1()
{
	add_var_info( _cm_vtab_pvsamv1 );
	add_var_info(vtab_adjustment_factors);
	add_var_info(vtab_dc_adjustment_factors);
	add_var_info(vtab_technology_outputs);
	add_var_info(vtab_battery_inputs);
	add_var_info(vtab_battery_outputs);
}

	
<<<<<<< HEAD
void cm_pvsamv1::exec() throw (compute_module::general_error)
{
	smart_ptr<weather_data_provider>::ptr wdprov;
	if (is_assigned("solar_resource_file"))
	{
		const char *file = as_string("solar_resource_file");
		wdprov = smart_ptr<weather_data_provider>::ptr(new weatherfile(file));

		weatherfile *wfile = dynamic_cast<weatherfile*>(wdprov.get());
		if (!wfile->ok()) throw exec_error("pvsamv1", wfile->message());
		if (wfile->has_message()) log(wfile->message(), SSC_WARNING);
	}
	else if (is_assigned("solar_resource_data"))
	{
		wdprov = smart_ptr<weather_data_provider>::ptr(new weatherdata(lookup("solar_resource_data")));
		if (wdprov->has_message()) log(wdprov->message(), SSC_WARNING);
	}
	else
		throw exec_error("pvsamv1", "no weather data supplied");


	// assumes instantaneous values, unless hourly file with no minute column specified
	double ts_shift_hours = 0.0;
	bool instantaneous = true;
	if (wdprov->has_data_column(weather_data_provider::MINUTE))
	{
		// if we have an file with a minute column, then
		// the starting time offset equals the time 
		// of the first record (for correct plotting)
		// this holds true even for hourly data with a minute column
		weather_record rec;
		if (wdprov->read(&rec))
			ts_shift_hours = rec.minute / 60.0;

		wdprov->rewind();
	}
	else if (wdprov->nrecords() == 8760)
	{
		// hourly file with no minute data column.  assume
		// integrated/averaged values and use mid point convention for interpreting results
		instantaneous = false;
		ts_shift_hours = 0.5;
	}
	else
		throw exec_error("pvsamv1", "subhourly weather files must specify the minute for each record");

	assign("ts_shift_hours", var_data((ssc_number_t)ts_shift_hours));

	weather_header hdr;
	wdprov->header(&hdr);

	weather_record wf;

	//total number of records in the weather file (i.e. 8760 * timestep)
	size_t nrec = wdprov->nrecords();
	size_t step_per_hour = nrec / 8760;
	if (step_per_hour < 1 || step_per_hour > 60 || step_per_hour * 8760 != nrec)
		throw exec_error("pvsamv1", util::format("invalid number of data records (%d): must be an integer multiple of 8760", (int)nrec));

	double ts_hour = 1.0 / step_per_hour;
	// shading database if necessary
	smart_ptr<ShadeDB8_mpp>::ptr  p_shade_db; // (new ShadeDB8_mpp());

	bool en_snow_model = (as_integer("en_snow_model") > 0); // snow model activation
	double annual_snow_loss = 0;

	int modules_per_string = as_integer("modules_per_string");
	int strings_in_parallel = as_integer("strings_in_parallel");
	int num_inverters = as_integer("inverter_count");
	//		double ac_derate = (1 - as_double("acwiring_loss") / 100) * (1 - as_double("transformer_loss") / 100);	//calculate using ac wiring and step up transformer losses
	double ac_derate = 1 - as_double("acwiring_loss") / 100;	//calculate using ac wiring 
	double ac_loss_percent = (1 - ac_derate) * 100;
	assign("ac_loss", var_data((ssc_number_t)ac_loss_percent));

	size_t alb_len = 0;
	ssc_number_t *alb_array = as_array("albedo", &alb_len); // monthly albedo array

	bool use_wf_alb = (as_integer("use_wf_albedo") > 0); // weather file albedo

	int radmode = as_integer("irrad_mode"); // 0=B&D, 1=G&B, 2=G&D, 3=POA-Ref, 4=POA-Pyra
	int skymodel = as_integer("sky_model"); // 0=isotropic, 1=hdkr, 2=perez

	// load the subarray parameter information
	subarray sa[4];
	int num_subarrays = 1;


	// check to see if shading database needs to be created; updated 4/19/16
	bool create_shade_db = false;
	// loop over subarrays
	for (size_t nn = 0; nn < 4; nn++)
	{
		sa[nn].enable = true;
		sa[nn].nstrings = strings_in_parallel;
		std::string prefix = "subarray" + util::to_string((int)(nn + 1)) + "_";

		if (nn > 0)
		{
			sa[nn].nstrings = 0;
			sa[nn].enable = as_boolean(prefix + "enable");
			if (sa[nn].enable) sa[nn].nstrings = as_integer(prefix + "nstrings");

			sa[0].nstrings -= sa[nn].nstrings;

			if (sa[nn].nstrings > 0 && sa[nn].enable)
				num_subarrays++;
		}

		// don't read in all the other variables
		// if the subarrays are disabled.
		if (!sa[nn].enable)
			continue;

		size_t soil_len = 0;
		ssc_number_t *soiling = as_array(prefix + "soiling", &soil_len); // monthly soiling array
		if (soil_len != 12) throw exec_error("pvsamv1", "soiling loss array must have 12 values: subarray " + util::to_string((int)(nn + 1)));
		for (int k = 0; k < 12; k++)
			sa[nn].soiling[k] = 1 - (double)soiling[k] / 100; //convert from % to derate

		sa[nn].derate =  /* combine all input losses into one derate, not a percentage */
			(1 - as_double(prefix + "mismatch_loss") / 100) *
			(1 - as_double(prefix + "diodeconn_loss") / 100) *
			(1 - as_double(prefix + "dcwiring_loss") / 100) *
			(1 - as_double(prefix + "tracking_loss") / 100) *
			(1 - as_double(prefix + "nameplate_loss") / 100) *
			(1 - as_double("dcoptimizer_loss") / 100);
		//assign output dc loss since we just calculated it
		double temploss = (1 - sa[nn].derate) * 100;
		assign(prefix + "dcloss", var_data((ssc_number_t)temploss));

		sa[nn].track_mode = as_integer(prefix + "track_mode"); // 0=fixed, 1=1axis, 2=2axis, 3=aziaxis, 4=timeseries

		sa[nn].tilt = fabs(hdr.lat);
		if (sa[nn].track_mode == 4) //timeseries tilt input
		{
			size_t monthly_tilt_count = 0;
			sa[nn].monthly_tilt = as_array(prefix + "monthly_tilt", &monthly_tilt_count);
		}

		if (!lookup(prefix + "tilt_eq_lat") || !as_boolean(prefix + "tilt_eq_lat"))
			sa[nn].tilt = fabs(as_double(prefix + "tilt"));

		sa[nn].azimuth = as_double(prefix + "azimuth");
		sa[nn].rotlim = as_double(prefix + "rotlim");

		sa[nn].gcr = as_double(prefix + "gcr");
		if (sa[nn].gcr < 0.01)
			throw exec_error("pvsamv1", "array ground coverage ratio must obey 0.01 < gcr");

		if (!sa[nn].shad.setup(this, prefix))
			throw exec_error("pvsamv1", prefix + "_shading: " + sa[nn].shad.get_error());

		create_shade_db = (create_shade_db || sa[nn].shad.use_shade_db());

		// backtracking- only required if one-axis tracker
		if (sa[nn].track_mode == 1)
		{
			sa[nn].backtrack = as_boolean(prefix + "backtrack");
		}

		// Initialize snow model if activated
		if (en_snow_model)
		{
			if (sa[nn].track_mode == 4) //timeseries tilt input
				throw exec_error("pvsamv1", "Time-series tilt input may not be used with the snow model at this time: subarray " + util::to_string((int)(nn + 1)));
			if (!sa[nn].sm.setup(as_integer(prefix + "nmody"), (float)sa[nn].tilt)) {
				if (sa[nn].sm.good)log(sa[nn].sm.msg, SSC_WARNING);
				else {
					log(sa[nn].sm.msg, SSC_ERROR);
					return;
				}
			}
		}

		sa[nn].poa.usePOAFromWF = false;
	}

	// create single instance of shading database if necessary
	if (create_shade_db)
	{
		p_shade_db = smart_ptr<ShadeDB8_mpp>::ptr(new ShadeDB8_mpp());
		p_shade_db->init();
	}


	// loop over subarrays AGAIN to calculate shading inputs because nstrings in subarray 1 isn't correct until AFTER the previous loop
	for (size_t nn = 0; nn < 4; nn++)
	{
		std::string prefix = "subarray" + util::to_string((int)(nn + 1)) + "_";

		// shading mode- only required for fixed tilt/timeseries tilt or one-axis, not backtracking systems
		if ((sa[nn].track_mode == 0 || sa[nn].track_mode == 4) || (sa[nn].track_mode == 1 && sa[nn].backtrack == 0))
		{
			sa[nn].shade_mode = as_integer(prefix + "shade_mode");
			if (!sa[nn].enable) continue; //skip disabled subarrays


			// shading inputs only required if shade mode is self-shaded
			if (sa[nn].shade_mode == 1 || sa[nn].shade_mode == 2)
			{
				sa[nn].sscalc.mod_orient = as_integer(prefix + "mod_orient");
				sa[nn].sscalc.nmody = as_integer(prefix + "nmody");
				sa[nn].sscalc.nmodx = as_integer(prefix + "nmodx");
				sa[nn].sscalc.nstrx = sa[nn].sscalc.nmodx / modules_per_string;

				// SELF-SHADING ASSUMPTIONS

				// Calculate the number of rows given the module dimensions of each row.
				sa[nn].sscalc.nrows = (int)floor((sa[nn].nstrings * modules_per_string) / (sa[nn].sscalc.nmodx * sa[nn].sscalc.nmody));
				//if nrows comes out to be zero, this will cause a divide by zero error. Give an error in this case.
				if (sa[nn].sscalc.nrows == 0 && sa[nn].nstrings != 0) //no need to give an error if the subarray has 0 strings
					throw exec_error("pvsamv1", "Self shading: Number of rows calculated for subarray " + util::to_string(to_double((double)nn + 1)) + " was zero. Please check your inputs.");
				// Otherwise, if self-shading configuration does not have equal number of modules as specified on system design page for that subarray,
				// compute dc derate using the self-shading configuration and apply it to the whole subarray. Give warning.
				if ((sa[nn].sscalc.nmodx * sa[nn].sscalc.nmody * sa[nn].sscalc.nrows) != (sa[nn].nstrings * modules_per_string))
					log(util::format("The product of number of modules along side and bottom for subarray %d is not equal to the number of modules in the subarray. Check your inputs for self shading.",
					(nn + 1)), SSC_WARNING);
				// assume aspect ratio of 1.7 (see variable "aspect_ratio" below to change this assumption)
				sa[nn].sscalc.str_orient = 1;	//assume horizontal wiring
				sa[nn].sscalc.mask_angle_calc_method = 0; //assume worst case mask angle calc method
				sa[nn].sscalc.ndiode = 3;	//assume 3 diodes- maybe update this assumption based on number of cells in the module?
			}
		}
	}

	double aspect_ratio = as_double("module_aspect_ratio");

	if (sa[0].nstrings < 0)
		throw exec_error("pvsamv1", "invalid string allocation between subarrays.  all subarrays must have zero or positive number of strings.");

	// run some preliminary checks on inputs

	int mod_type = as_integer("module_model");

	spe_module_t spe;
	sandia_celltemp_t spe_tc;

	cec6par_module_t cec;
	noct_celltemp_t noct_tc;
	mcsp_celltemp_t mcsp_tc;

	sandia_module_t snl;
	sandia_celltemp_t snl_tc;

	iec61853_module_t sd11; // 11 parameter single diode, uses noct_tc from above

	mlmodel_module_t mlm;
	mock_celltemp_t mock_tc;

	pvcelltemp_t *celltemp_model = 0;
	pvmodule_t *module_model = 0;

	double ref_area_m2 = 0;

	double self_shading_fill_factor = 0;
	double ssVmp = 0;

	double module_watts_stc = -1.0;

	//"0=spe,1=cec,2=6par_user,3=snl,4=sd11-iec61853"
	bool enable_mismatch_vmax_calc = as_boolean("enable_mismatch_vmax_calc");
	if (enable_mismatch_vmax_calc
		&& mod_type != 1 && mod_type != 2 && mod_type != 4)
		throw exec_error("pvsamv1", "String level subarray mismatch can only be calculated using a single-diode based module model.");


	bool speForceNoPOA = false;		// SEV 151002 - Set these flags to avoid calling as_integer(...) repeatedly later on
	bool mcspForceNoPOA = false;    //   These flags are used to ensure that the usePOAFromWF flag for each sub array will be force
									//   to false

	if (mod_type == 0)
	{
		spe.VmpNominal = as_double("spe_vmp");
		spe.VocNominal = as_double("spe_voc");
		spe.Area = as_double("spe_area");
		ref_area_m2 = spe.Area;
		for (int i = 0; i < 5; i++)
		{
			spe.Rad[i] = as_double(util::format("spe_rad%d", i));
			spe.Eff[i] = 0.01*as_double(util::format("spe_eff%d", i));
			if (i > 0 && spe.Rad[i] <= spe.Rad[i - 1])
				throw exec_error("pvsamv1", "SPE model radiation levels must increase monotonically");
		}

		spe.Gamma = as_double("spe_temp_coeff");
		spe.Reference = as_integer("spe_reference");

		switch (as_integer("spe_module_structure"))
		{
		case 0: //glass/cell/polymer sheet - open rack
			spe_tc.a = -3.56;
			spe_tc.b = -0.0750;
			spe_tc.DT0 = 3;
			break;
		case 1: //glass/cell/glass - open rack
			spe_tc.a = -3.47;
			spe_tc.b = -0.0594;
			spe_tc.DT0 = 3;
			break;
		case 2: //polymer/thin film/steel - open rack
			spe_tc.a = -3.58;
			spe_tc.b = -0.113;
			spe_tc.DT0 = 3;
			break;
		case 3: //Insulated back (building-integrated PV)
			spe_tc.a = -2.81;
			spe_tc.b = -0.0455;
			spe_tc.DT0 = 0;
			break;
		case 4: //close roof mount
			spe_tc.a = -2.98;
			spe_tc.b = -0.0471;
			spe_tc.DT0 = 1;
			break;
		case 5: //user defined
			spe_tc.a = as_double("spe_a");
			spe_tc.b = as_double("spe_b");
			spe_tc.DT0 = as_double("spe_dT");
			break;
		default:
			throw exec_error("pvsamv1", "invalid spe module structure and mounting");
		}

		spe.fd = as_double("spe_fd");
		spe_tc.fd = spe.fd;

		if (spe.fd < 1.0)
			speForceNoPOA = true;

		celltemp_model = &spe_tc;
		module_model = &spe;
		module_watts_stc = spe.WattsStc();
		ssVmp = spe.VmpNominal;
	}
	else if (mod_type == 1)
	{
		cec.Area = as_double("cec_area");
		ref_area_m2 = cec.Area;
		cec.Vmp = as_double("cec_v_mp_ref");
		cec.Imp = as_double("cec_i_mp_ref");
		cec.Voc = as_double("cec_v_oc_ref");
		cec.Isc = as_double("cec_i_sc_ref");
		cec.alpha_isc = as_double("cec_alpha_sc");
		cec.beta_voc = as_double("cec_beta_oc");
		cec.a = as_double("cec_a_ref");
		cec.Il = as_double("cec_i_l_ref");
		cec.Io = as_double("cec_i_o_ref");
		cec.Rs = as_double("cec_r_s");
		cec.Rsh = as_double("cec_r_sh_ref");
		cec.Adj = as_double("cec_adjust");

		self_shading_fill_factor = cec.Vmp * cec.Imp / cec.Voc / cec.Isc;
		ssVmp = cec.Vmp;

		if (as_integer("cec_temp_corr_mode") == 0)
		{
			noct_tc.Tnoct = as_double("cec_t_noct");
			int standoff = as_integer("cec_standoff");
			noct_tc.standoff_tnoct_adj = 0;
			switch (standoff)
			{
			case 2: noct_tc.standoff_tnoct_adj = 2; break; // between 2.5 and 3.5 inches
			case 3: noct_tc.standoff_tnoct_adj = 6; break; // between 1.5 and 2.5 inches
			case 4: noct_tc.standoff_tnoct_adj = 11; break; // between 0.5 and 1.5 inches
			case 5: noct_tc.standoff_tnoct_adj = 18; break; // less than 0.5 inches
				// note: all others, standoff_tnoct_adj = 0;
			}

			int height = as_integer("cec_height");
			noct_tc.ffv_wind = 0.51;
			if (height == 1)
				noct_tc.ffv_wind = 0.61;

			celltemp_model = &noct_tc;
		}
		else
		{
			/*	int MC; // Mounting configuration (1=rack,2=flush,3=integrated,4=gap)
				int HTD; // Heat transfer dimension (1=Module,2=Array)
				int MSO; // Mounting structure orientation (1=does not impede flow beneath, 2=vertical supports, 3=horizontal supports)
				int Nrows, Ncols; // number of modules in rows and columns, when using array heat transfer dimensions
				double Length; // module length, along horizontal dimension, (m)
				double Width; // module width, along vertical dimension, (m)
				double Wgap;  // gap width spacing (m)
				double TbackInteg; */

			mcsp_tc.DcDerate = sa[0].derate;  // TODO dc_derate needs to updated for each subarray
			mcsp_tc.MC = as_integer("cec_mounting_config") + 1;
			mcsp_tc.HTD = as_integer("cec_heat_transfer") + 1;
			mcsp_tc.MSO = as_integer("cec_mounting_orientation") + 1;
			mcsp_tc.Wgap = as_double("cec_gap_spacing");
			mcsp_tc.Length = as_double("cec_module_length");
			mcsp_tc.Width = as_double("cec_module_width");
			mcsp_tc.Nrows = (int)as_integer("cec_array_rows");
			mcsp_tc.Ncols = (int)as_integer("cec_array_cols");
			mcsp_tc.TbackInteg = as_double("cec_backside_temp");

			celltemp_model = &mcsp_tc;
			mcspForceNoPOA = true;
		}

		module_model = &cec;
		module_watts_stc = cec.Vmp * cec.Imp;
	}
	else if (mod_type == 3)
	{
		snl.A0 = as_double("snl_a0");
		snl.A1 = as_double("snl_a1");
		snl.A2 = as_double("snl_a2");
		snl.A3 = as_double("snl_a3");
		snl.A4 = as_double("snl_a4");
		snl.aImp = as_double("snl_aimp");
		snl.aIsc = as_double("snl_aisc");
		snl.Area = as_double("snl_area");
		ref_area_m2 = snl.Area;
		snl.B0 = as_double("snl_b0");
		snl.B1 = as_double("snl_b1");
		snl.B2 = as_double("snl_b2");
		snl.B3 = as_double("snl_b3");
		snl.B4 = as_double("snl_b4");
		snl.B5 = as_double("snl_b5");
		snl.BVmp0 = as_double("snl_bvmpo");
		snl.BVoc0 = as_double("snl_bvoco");
		snl.C0 = as_double("snl_c0");
		snl.C1 = as_double("snl_c1");
		snl.C2 = as_double("snl_c2");
		snl.C3 = as_double("snl_c3");
		snl.C4 = as_double("snl_c4");
		snl.C5 = as_double("snl_c5");
		snl.C6 = as_double("snl_c6");
		snl.C7 = as_double("snl_c7");
		snl.fd = as_double("snl_fd");
		snl.Imp0 = as_double("snl_impo");
		snl.Isc0 = as_double("snl_isco");
		snl.Ix0 = as_double("snl_ixo");
		snl.Ixx0 = as_double("snl_ixxo");
		snl.mBVmp = as_double("snl_mbvmp");
		snl.mBVoc = as_double("snl_mbvoc");
		snl.DiodeFactor = as_double("snl_n");
		snl.NcellSer = as_integer("snl_series_cells");
		snl.Vmp0 = as_double("snl_vmpo");
		snl.Voc0 = as_double("snl_voco");

		self_shading_fill_factor = snl.Vmp0 * snl.Imp0 / snl.Voc0 / snl.Isc0;
		ssVmp = snl.Vmp0;

		// by default, use database values
		double A = as_double("snl_a");
		double B = as_double("snl_b");
		double DT = as_double("snl_dtc");

		switch (as_integer("snl_module_structure"))
		{
		case 1: //glass/cell/polymer sheet - open rack
			A = -3.56;
			B = -0.0750;
			DT = 3;
			break;
		case 2: //glass/cell/glass - open rack
			A = -3.47;
			B = -0.0594;
			DT = 3;
			break;
		case 3: //polymer/thin film/steel - open rack
			A = -3.58;
			B = -0.113;
			DT = 3;
			break;
		case 4: //Insulated back (building-integrated PV)
			A = -2.81;
			B = -0.0455;
			DT = 0;
			break;
		case 5: //close roof mount
			A = -2.98;
			B = -0.0471;
			DT = 1;
			break;
		case 6: //user defined
			A = as_double("snl_ref_a");
			B = as_double("snl_ref_b");
			DT = as_double("snl_ref_dT");
			break;

		default:
			break;
		}

		snl_tc.a = A;
		snl_tc.b = B;
		snl_tc.DT0 = DT;
		snl_tc.fd = snl.fd;

		celltemp_model = &snl_tc;
		module_model = &snl;
		module_watts_stc = snl.Vmp0 * snl.Imp0;
	}
	else if (mod_type == 2)
	{
		// calculate the 6 parameters
		// adjust TNOCT and FFV_wind

		int tech_id = module6par::monoSi;
		int type = as_integer("6par_celltech"); // "monoSi,multiSi,CdTe,CIS,CIGS,Amorphous"
		switch (type)
		{
		case 0: tech_id = module6par::monoSi; break;
		case 1: tech_id = module6par::multiSi; break;
		case 2: tech_id = module6par::CdTe; break;
		case 3: tech_id = module6par::CIS; break;
		case 4: tech_id = module6par::CIGS; break;
		case 5: tech_id = module6par::Amorphous; break;
		}

		double Vmp = as_double("6par_vmp");
		double Imp = as_double("6par_imp");
		double Voc = as_double("6par_voc");
		double Isc = as_double("6par_isc");
		double alpha = as_double("6par_aisc");
		double beta = as_double("6par_bvoc");
		double gamma = as_double("6par_gpmp");
		int nser = as_integer("6par_nser");

		module6par m(tech_id, Vmp, Imp, Voc, Isc, beta, alpha, gamma, nser, 298.15);
		int err = m.solve_with_sanity_and_heuristics<double>(300, 1e-7);

		if (err != 0)
			throw exec_error("pvsamv1", "CEC 6 parameter model:  Could not solve for normalized coefficients.  Please check your inputs.");

		cec.Area = as_double("6par_area");
		ref_area_m2 = cec.Area;
		cec.Vmp = Vmp;
		cec.Imp = Imp;
		cec.Voc = Voc;
		cec.Isc = Isc;
		cec.alpha_isc = alpha;
		cec.beta_voc = beta;
		cec.a = m.a;
		cec.Il = m.Il;
		cec.Io = m.Io;
		cec.Rs = m.Rs;
		cec.Rsh = m.Rsh;
		cec.Adj = m.Adj;

		self_shading_fill_factor = cec.Vmp * cec.Imp / cec.Voc / cec.Isc;
		ssVmp = cec.Vmp;

		setup_noct_model("6par", noct_tc);

		celltemp_model = &noct_tc;
		module_model = &cec;
		module_watts_stc = cec.Vmp * cec.Imp;
	}
	else if (mod_type == 4)
	{
		// IEC 61853 model
		sd11.NcellSer = as_integer("sd11par_nser");
		sd11.Area = as_double("sd11par_area");
		sd11.AMA[0] = as_double("sd11par_AMa0");
		sd11.AMA[1] = as_double("sd11par_AMa1");
		sd11.AMA[2] = as_double("sd11par_AMa2");
		sd11.AMA[3] = as_double("sd11par_AMa3");
		sd11.AMA[4] = as_double("sd11par_AMa4");
		sd11.GlassAR = as_boolean("sd11par_glass");

		setup_noct_model("sd11par", noct_tc);

		sd11.Vmp0 = as_double("sd11par_Vmp0");
		sd11.Imp0 = as_double("sd11par_Imp0");
		sd11.Voc0 = as_double("sd11par_Voc0");
		sd11.Isc0 = as_double("sd11par_Isc0");
		sd11.alphaIsc = as_double("sd11par_alphaIsc");
		sd11.n = as_double("sd11par_n");
		sd11.Il = as_double("sd11par_Il");
		sd11.Io = as_double("sd11par_Io");
		sd11.Egref = as_double("sd11par_Egref");
		sd11.D1 = as_double("sd11par_d1");
		sd11.D2 = as_double("sd11par_d2");
		sd11.D3 = as_double("sd11par_d3");
		sd11.C1 = as_double("sd11par_c1");
		sd11.C2 = as_double("sd11par_c2");
		sd11.C3 = as_double("sd11par_c3");

		celltemp_model = &noct_tc;
		module_model = &sd11;
		module_watts_stc = sd11.Vmp0 * sd11.Imp0;
		ref_area_m2 = sd11.Area;
		self_shading_fill_factor = sd11.Vmp0 * sd11.Imp0 / sd11.Voc0 / sd11.Isc0;
		ssVmp = sd11.Vmp0;
	}
	else if (mod_type == 5)
	{
		// Mermoud/Lejeune single-diode model
		size_t elementCount1 = 0;
		size_t elementCount2 = 0;
		ssc_number_t *arrayIncAngle = 0;
		ssc_number_t *arrayIamValue = 0;

		mlm.N_series = as_integer("mlm_N_series");
		mlm.N_parallel = as_integer("mlm_N_parallel");
		mlm.N_diodes = as_integer("mlm_N_diodes");
		mlm.Width = as_double("mlm_Width");
		mlm.Length = as_double("mlm_Length");
		mlm.V_mp_ref = as_double("mlm_V_mp_ref");
		mlm.I_mp_ref = as_double("mlm_I_mp_ref");
		mlm.V_oc_ref = as_double("mlm_V_oc_ref");
		mlm.I_sc_ref = as_double("mlm_I_sc_ref");
		mlm.S_ref = as_double("mlm_S_ref");
		mlm.T_ref = as_double("mlm_T_ref");
		mlm.R_shref = as_double("mlm_R_shref");
		mlm.R_sh0 = as_double("mlm_R_sh0");
		mlm.R_shexp = as_double("mlm_R_shexp");
		mlm.R_s = as_double("mlm_R_s");
		mlm.alpha_isc = as_double("mlm_alpha_isc");
		mlm.beta_voc_spec = as_double("mlm_beta_voc_spec");
		mlm.E_g = as_double("mlm_E_g");
		mlm.n_0 = as_double("mlm_n_0");
		mlm.mu_n = as_double("mlm_mu_n");
		mlm.D2MuTau = as_double("mlm_D2MuTau");
		mlm.T_mode = as_integer("mlm_T_mode");
		mlm.T_c_no_tnoct = as_double("mlm_T_c_no_tnoct");
		mlm.T_c_no_mounting = as_integer("mlm_T_c_no_mounting");
		mlm.T_c_no_standoff = as_integer("mlm_T_c_no_standoff");
		mlm.T_c_fa_alpha = as_double("mlm_T_c_fa_alpha");
		mlm.T_c_fa_U0 = as_double("mlm_T_c_fa_U0");
		mlm.T_c_fa_U1 = as_double("mlm_T_c_fa_U1");
		mlm.AM_mode = as_integer("mlm_AM_mode");
		mlm.AM_c_sa[0] = as_double("mlm_AM_c_sa0");
		mlm.AM_c_sa[1] = as_double("mlm_AM_c_sa1");
		mlm.AM_c_sa[2] = as_double("mlm_AM_c_sa2");
		mlm.AM_c_sa[3] = as_double("mlm_AM_c_sa3");
		mlm.AM_c_sa[4] = as_double("mlm_AM_c_sa4");
		mlm.AM_c_lp[0] = as_double("mlm_AM_c_lp0");
		mlm.AM_c_lp[1] = as_double("mlm_AM_c_lp0");
		mlm.AM_c_lp[2] = as_double("mlm_AM_c_lp0");
		mlm.AM_c_lp[3] = as_double("mlm_AM_c_lp0");
		mlm.AM_c_lp[4] = as_double("mlm_AM_c_lp0");
		mlm.AM_c_lp[5] = as_double("mlm_AM_c_lp0");
		mlm.IAM_mode = as_integer("mlm_IAM_mode");
		mlm.IAM_c_as = as_double("mlm_IAM_c_as");
		mlm.IAM_c_sa[0] = as_double("mlm_IAM_c_sa0");
		mlm.IAM_c_sa[1] = as_double("mlm_IAM_c_sa1");
		mlm.IAM_c_sa[2] = as_double("mlm_IAM_c_sa2");
		mlm.IAM_c_sa[3] = as_double("mlm_IAM_c_sa3");
		mlm.IAM_c_sa[4] = as_double("mlm_IAM_c_sa4");
		mlm.IAM_c_sa[5] = as_double("mlm_IAM_c_sa5");
		mlm.groundRelfectionFraction = as_double("mlm_groundRelfectionFraction");

		arrayIncAngle = as_array("mlm_IAM_c_cs_incAngle", &elementCount1);
		arrayIamValue = as_array("mlm_IAM_c_cs_iamValue", &elementCount2);
		mlm.IAM_c_cs_elements = elementCount1; // as_integer("mlm_IAM_c_cs_elements");

		if (mlm.IAM_mode == 3)
		{
			if (elementCount1 != elementCount2)
			{
				exec_error("pvsamv1", "Spline IAM: Number of entries for incidence angle and IAM value different.");
			}
			for (int i = 0; i <= mlm.IAM_c_cs_elements - 1; i = i + 1) {
				mlm.IAM_c_cs_incAngle[i] = arrayIncAngle[i];
				mlm.IAM_c_cs_iamValue[i] = arrayIamValue[i];
			}
		}
		if (mlm.T_mode == 1) {
			setup_noct_model("mlm_T_c_no", noct_tc);
			celltemp_model = &noct_tc;
		}
		else if (mlm.T_mode == 2) {
			celltemp_model = &mock_tc;
		}
		else {
			throw exec_error("pvsamv1", "invalid temperature model type");
		}

		mlm.initializeManual();

		module_model = &mlm;
		module_watts_stc = mlm.V_oc_ref * mlm.I_mp_ref;
		ref_area_m2 = mlm.Width * mlm.Length;
		self_shading_fill_factor = mlm.V_mp_ref * mlm.I_mp_ref / mlm.V_oc_ref / mlm.I_sc_ref;
		ssVmp = mlm.V_mp_ref;
	}
	else
		throw exec_error("pvsamv1", "invalid pv module model type");

	//boolean to determine if the sandia model is being used for CPV
	bool is_cpv = false;

	if (as_integer("module_model") == 3 // sandia model 
		&& as_double("snl_fd") == 0)
		is_cpv = true;

=======
void cm_pvsamv1::exec( ) throw (compute_module::general_error)
{

	/// Underlying class which parses the compute module structure and sets up model inputs and outputs
	std::unique_ptr<PVIOManager> IOManager(new PVIOManager(this, "pvsamv1"));
	Simulation_IO * Simulation = IOManager->getSimulationIO();
	Irradiance_IO * Irradiance = IOManager->getIrradianceIO();
	std::vector<Subarray_IO *> Subarrays = IOManager->getSubarrays();
	PVSystem_IO * PVSystem = IOManager->getPVSystemIO();
	ShadeDB8_mpp * shadeDatabase = IOManager->getShadeDatabase();
	
	size_t nrec = Simulation->numberOfWeatherFileRecords;
	size_t nlifetime = Simulation->numberOfSteps;
	size_t nyears = Simulation->numberOfYears;
	double ts_hour = Simulation->dtHour;
	size_t step_per_hour = Simulation->stepsPerHour;
	bool system_use_lifetime_output = Simulation->useLifetimeOutput;

	// Get Irradiance Inputs for now (eventually models can use these directly)
	weather_header hdr = Irradiance->weatherHeader;
	weather_data_provider * wdprov = Irradiance->weatherDataProvider.get();
	int radmode = Irradiance->radiationMode;

	// Get System or Subarray Inputs
	double aspect_ratio = Subarrays[0]->moduleAspectRatio;
	size_t num_subarrays = PVSystem->numberOfSubarrays;
	int mod_type = Subarrays[0]->Module->modulePowerModel;
	double ref_area_m2 = Subarrays[0]->Module->referenceArea;
	double module_watts_stc = Subarrays[0]->Module->moduleWattsSTC;
	SharedInverter * sharedInverter = PVSystem->m_sharedInverter.get();

	double annual_snow_loss = 0;
	
>>>>>>> pr/11
	// SELF-SHADING MODULE INFORMATION
	double width = sqrt((ref_area_m2 / aspect_ratio));
	for (size_t nn = 0; nn < num_subarrays; nn++)
	{
		Subarrays[nn]->selfShadingInputs.width = width;
		Subarrays[nn]->selfShadingInputs.length = width * aspect_ratio;
		Subarrays[nn]->selfShadingInputs.FF0 = Subarrays[nn]->Module->selfShadingFillFactor;
		Subarrays[nn]->selfShadingInputs.Vmp = Subarrays[nn]->Module->voltageMaxPower;
		double b = 0;
		if (Subarrays[nn]->selfShadingInputs.mod_orient == 0)
			b = Subarrays[nn]->selfShadingInputs.nmody * Subarrays[nn]->selfShadingInputs.length;
		else
			b = Subarrays[nn]->selfShadingInputs.nmody * Subarrays[nn]->selfShadingInputs.width;
		Subarrays[nn]->selfShadingInputs.row_space = b / Subarrays[nn]->groundCoverageRatio;
	}
<<<<<<< HEAD

	double nameplate_kw = modules_per_string * strings_in_parallel * module_watts_stc * util::watt_to_kilowatt;

	::sandia_inverter_t snlinv;
	::ond_inverter ondinv;
	::partload_inverter_t plinv;

	int inv_type = as_integer("inverter_model");
	double V_mppt_lo_1module;
	double V_mppt_hi_1module;
	if (inv_type == 4) {
		V_mppt_lo_1module = as_double("ond_VMppMin") / modules_per_string;
		V_mppt_hi_1module = as_double("ond_VMppMax") / modules_per_string;
	}
	else {
		V_mppt_lo_1module = as_double("mppt_low_inverter") / modules_per_string;
		V_mppt_hi_1module = as_double("mppt_hi_inverter") / modules_per_string;
	}

	bool clip_mppt_window = false;
	double ratedACOutput = 0;
=======
>>>>>>> pr/11

	double nameplate_kw = 0;
	for (size_t nn = 0; nn < num_subarrays; nn++)
	{
<<<<<<< HEAD
		if ( mod_type == 1     // cec with database
			|| mod_type == 2   // cec with user specs
			|| mod_type == 5   // ml single diode model
			|| mod_type == 4 ) // iec61853 single diode
		{
			clip_mppt_window = true;
		}
		else
		{
			log( "The simple efficiency and Sandia module models do not allow limiting module voltage to the MPPT tracking range of the inverter.", SSC_NOTICE );
		}
	}
	else
	{
		log( "Inverter MPPT voltage tracking window not defined - modules always operate at MPPT.", SSC_NOTICE );
	}

	if (inv_type == 0) // cec database
	{
		snlinv.Paco = as_double("inv_snl_paco");
		snlinv.Pdco = as_double("inv_snl_pdco");
		snlinv.Vdco = as_double("inv_snl_vdco");
		snlinv.Pso = as_double("inv_snl_pso");
		snlinv.Pntare = as_double("inv_snl_pnt");
		snlinv.C0 = as_double("inv_snl_c0");
		snlinv.C1 = as_double("inv_snl_c1");
		snlinv.C2 = as_double("inv_snl_c2");
		snlinv.C3 = as_double("inv_snl_c3");
		ratedACOutput = snlinv.Paco;
	}
	else if (inv_type == 1) // datasheet data
	{
		double eff_ds = as_double("inv_ds_eff") / 100.0;
		snlinv.Paco = as_double("inv_ds_paco");
		if (eff_ds != 0)
			snlinv.Pdco = snlinv.Paco / eff_ds;
		else
			snlinv.Pdco = 0;
		snlinv.Vdco = as_double("inv_ds_vdco");
		snlinv.Pso = as_double("inv_ds_pso");
		snlinv.Pntare = as_double("inv_ds_pnt");
		snlinv.C0 = 0;
		snlinv.C1 = 0;
		snlinv.C2 = 0;
		snlinv.C3 = 0;
		ratedACOutput = snlinv.Paco;
	}
	else if (inv_type == 2) // partload curve
	{
		plinv.Paco = as_double("inv_pd_paco");
		plinv.Pdco = as_double("inv_pd_pdco");
		plinv.Pntare = as_double("inv_pd_pnt");

		std::vector<double> pl_pd = as_doublevec("inv_pd_partload");
		std::vector<double> eff_pd = as_doublevec("inv_pd_efficiency");

		plinv.Partload = pl_pd;
		plinv.Efficiency = eff_pd;
		ratedACOutput = plinv.Paco;
	}
	else if (inv_type == 3) // coefficient generator
	{
		snlinv.Paco = as_double("inv_cec_cg_paco");
		snlinv.Pdco = as_double("inv_cec_cg_pdco");
		snlinv.Vdco = as_double("inv_cec_cg_vdco");
		snlinv.Pso = as_double("inv_cec_cg_psco");
		snlinv.Pntare = as_double("inv_cec_cg_pnt");
		snlinv.C0 = as_double("inv_cec_cg_c0");
		snlinv.C1 = as_double("inv_cec_cg_c1");
		snlinv.C2 = as_double("inv_cec_cg_c2");
		snlinv.C3 = as_double("inv_cec_cg_c3");
		ratedACOutput = snlinv.Paco;
	}
	else if (inv_type == 4) // ond inverter
	{
		size_t elementCount = 0;
		size_t rows = 0;
		size_t cols = 0;
		ssc_number_t *VNomEffArray;
		ssc_number_t *effCurve_PdcArray;
		ssc_number_t *effCurve_PacArray;
		ssc_number_t *effCurve_etaArray;
		
		ondinv.PNomConv = as_double("ond_PNomConv");
		ondinv.PMaxOUT = as_double("ond_PMaxOUT");
		ondinv.VOutConv = as_double("ond_VOutConv");
		ondinv.VMppMin = as_double("ond_VMppMin");
		ondinv.VMPPMax = as_double("ond_VMPPMax");
		ondinv.VAbsMax = as_double("ond_VAbsMax");
		ondinv.PSeuil = as_double("ond_PSeuil");
		ondinv.ModeOper = as_string("ond_ModeOper");
		ondinv.CompPMax = as_string("ond_CompPMax");
		ondinv.CompVMax = as_string("ond_CompVMax");
		ondinv.ModeAffEnum = as_string("ond_ModeAffEnum");
		ondinv.PNomDC = as_double("ond_PNomDC");
		ondinv.PMaxDC = as_double("ond_PMaxDC");
		ondinv.IMaxDC = as_double("ond_IMaxDC");
		ondinv.INomDC = as_double("ond_INomDC");
		ondinv.INomAC = as_double("ond_INomAC");
		ondinv.IMaxAC = as_double("ond_IMaxAC");
		ondinv.TPNom = as_double("ond_TPNom");
		ondinv.TPMax = as_double("ond_TPMax");
		ondinv.TPLim1 = as_double("ond_TPLim1");
		ondinv.TPLimAbs = as_double("ond_TPLimAbs");
		ondinv.PLim1 = as_double("ond_PLim1");
		ondinv.PLimAbs = as_double("ond_PLimAbs");
		VNomEffArray = as_array("ond_VNomEff", &elementCount);
		ondinv.NbInputs = as_integer("ond_NbInputs");
		ondinv.NbMPPT = as_integer("ond_NbMPPT");
		ondinv.Aux_Loss = as_double("ond_Aux_Loss");
		ondinv.Night_Loss = as_double("ond_Night_Loss");
		ondinv.lossRDc = as_double("ond_lossRDc");
		ondinv.lossRAc = as_double("ond_lossRAc");
		ondinv.effCurve_elements = as_integer("ond_effCurve_elements");
		effCurve_PdcArray = as_matrix("ond_effCurve_Pdc", &rows, &cols);
		effCurve_PacArray = as_matrix("ond_effCurve_Pac", &rows, &cols);
		effCurve_etaArray = as_matrix("ond_effCurve_eta", &rows, &cols);
		ondinv.doAllowOverpower = as_integer("ond_doAllowOverpower");
		ondinv.doUseTemperatureLimit = as_integer("ond_doUseTemperatureLimit");

		int matrixIndex;
		const int MAX_ELEMENTS = 100;
		for (int i = 0; i <= 2; i = i + 1) {
			ondinv.VNomEff[i] = VNomEffArray[i];
			for (int j = 0; j <= MAX_ELEMENTS - 1; j = j + 1) {
				matrixIndex = i * MAX_ELEMENTS + j;
				ondinv.effCurve_Pdc[i][j] = effCurve_PdcArray[matrixIndex];
				ondinv.effCurve_Pac[i][j] = effCurve_PacArray[matrixIndex];
				ondinv.effCurve_eta[i][j] = effCurve_etaArray[matrixIndex];
			}
		}

		ondinv.initializeManual();
		ratedACOutput = ondinv.PNomConv;
	}
	else
	{
		throw exec_error("pvsamv1", "invalid inverter model type");
	}
	ratedACOutput *= num_inverters;

	// lifetime control variables - used to set array sizes
	int system_use_lifetime_output = as_integer("system_use_lifetime_output");
	size_t nyears = 1;
	if (system_use_lifetime_output == 1)
		nyears = as_integer("analysis_period");

=======
		nameplate_kw += Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings * module_watts_stc * util::watt_to_kilowatt;
	}

>>>>>>> pr/11
	// Warning workaround
	static bool is32BitLifetime = (__ARCHBITS__ == 32 && system_use_lifetime_output);
	if (is32BitLifetime)
		throw exec_error( "pvsamv1", "Lifetime simulation of PV systems is only available in the 64 bit version of SAM.");

	// lifetime outputs
	std::vector<ssc_number_t> p_load_full; p_load_full.reserve(nlifetime);

	//dc hourly adjustment factors
	adjustment_factors dc_haf(this, "dc_adjust");
	if (!dc_haf.setup())
		throw exec_error("pvsamv1", "failed to setup DC adjustment factors: " + dc_haf.error());

	// hourly adjustment factors
	adjustment_factors haf(this, "adjust");
	if (!haf.setup())
		throw exec_error("pvsamv1", "failed to setup adjustment factors: " + haf.error());

	// setup battery model
	bool en_batt = as_boolean("en_batt");
	battstor batt(*this, en_batt, nrec, ts_hour);
	batt.setSharedInverter(sharedInverter);
	int batt_topology = (en_batt == true ? batt.batt_vars->batt_topology : 0);
	std::vector<ssc_number_t> p_invcliploss_full;
	p_invcliploss_full.reserve(nlifetime);

	// Multiple MPPT inverters not enabled with DC-connected batteries
	if (PVSystem->Inverter->nMpptInputs > 1 && en_batt && batt_topology == ChargeController::DC_CONNECTED)
		throw exec_error("pvsamv1", "A DC-connected battery cannot be modeled with multiple MPPT inverters at this time.");

	// Multiple MPPT inverters not enabled with PVyield inverter model
	if (PVSystem->Inverter->nMpptInputs > 1 && PVSystem->Inverter->inverterType == INVERTER_PVYIELD)
		throw exec_error("pvsamv1", "The PVYield inverter model does not currently accept multiple MPPT inputs.");

	std::vector<ssc_number_t> p_pv_clipping_forecast;
	std::vector<ssc_number_t> p_pv_dc_forecast;
	std::vector<ssc_number_t> p_pv_dc_use;

	if (is_assigned("batt_pv_clipping_forecast")) {
		p_pv_clipping_forecast = as_vector_ssc_number_t("batt_pv_clipping_forecast");
	}
	if (is_assigned("batt_pv_dc_forecast")) {
		p_pv_dc_forecast = as_vector_ssc_number_t("batt_pv_dc_forecast");
	}

	// electric load - lifetime load data?
	double cur_load = 0.0;
	size_t nload = 0;
	std::vector<ssc_number_t> p_load_in;
	if ( is_assigned( "load" ) )
	{
		p_load_in = as_vector_ssc_number_t("load");
		nload = p_load_in.size();
		if ( nload != nrec && nload != 8760 )
			throw exec_error("pvsamv1", "electric load profile must have same number of values as weather file, or 8760");
	}

	// for reporting status updates
	float percent_baseline = 0.;
	float percent_complete = 0.;
	size_t nreports = 50 * nyears;
	size_t ireport = 0;
	size_t ireplast = 0;
	size_t insteps = 3 * nyears * 8760;
	size_t irepfreq = insteps/nreports;

	size_t idx = 0;
	size_t hour = 0;

	// variables used to calculate loss diagram
<<<<<<< HEAD
	double annual_energy = 0, annual_ac_gross = 0, annual_ac_pre_avail = 0, dc_gross[4] = { 0, 0, 0, 0 }, annual_mppt_window_clipping = 0, annual_dc_adjust_loss = 0, annual_dc_lifetime_loss = 0, annual_ac_lifetime_loss = 0, annual_ac_battery_loss = 0, annual_xfmr_nll = 0, annual_xfmr_ll = 0, annual_xfmr_loss = 0;
	double annual_dc_loss = 0; // Added to account for DC losses (TR)

	// Check if a POA model is used, if so load all POA data into the poaData struct
	if (radmode == POA_R || radmode == POA_P ){
		for (int nn = 0; nn < 4; nn++){
			if (!sa[nn].enable) continue;
				
			sa[nn].poa.poaAll.elev = hdr.elev;
=======
	double annual_energy = 0, annual_ac_gross = 0, annual_ac_pre_avail = 0, dc_gross[4] = { 0, 0, 0, 0 }, annualMpptVoltageClipping = 0, annual_dc_adjust_loss = 0, annual_dc_lifetime_loss = 0, annual_ac_lifetime_loss = 0, annual_ac_battery_loss = 0, annual_xfmr_nll = 0, annual_xfmr_ll = 0, annual_xfmr_loss = 0;
>>>>>>> pr/11

	/* *********************************************************************************************
	PV DC calculation
	*********************************************************************************************** */
	std::vector<double> dcPowerNetPerMppt_kW; //Vector of Net DC power in kW for each MPPT input on the system for THIS TIMESTEP ONLY
	std::vector<double> dcPowerNetPerSubarray; //Net DC power in W for each subarray for THIS TIMESTEP ONLY
	std::vector<double> dcVoltagePerMppt; //Voltage in V at each MPPT input on the system for THIS TIMESTEP ONLY	
	std::vector<std::vector<double>> dcStringVoltage; // Voltage of string for each subarray
	double dcPowerNetTotalSystem = 0; //Net DC power in W for the entire system (sum of all subarrays)

	for (size_t mpptInput = 0; mpptInput < PVSystem->Inverter->nMpptInputs; mpptInput++)
	{
		dcPowerNetPerMppt_kW.push_back(0);
		dcVoltagePerMppt.push_back(0);
		PVSystem->p_dcPowerNetPerMppt[mpptInput][idx] = 0;		
	}
	for (size_t nn = 0; nn < PVSystem->numberOfSubarrays; nn++) {
		dcPowerNetPerSubarray.push_back(0);
		std::vector<double> tmp;
		dcStringVoltage.push_back(tmp);
	}
	for (size_t iyear = 0; iyear < nyears; iyear++)
	{
		for (hour = 0; hour < 8760; hour++)
		{
			// report progress updates to the caller	
			ireport++;
			if (ireport - ireplast > irepfreq)
			{
				percent_complete = percent_baseline + 100.0f *(float)(hour + iyear * 8760) / (float)(insteps);
				if (!update("", percent_complete))
					throw exec_error("pvsamv1", "simulation canceled at hour " + util::to_string(hour + 1.0) + " in year " + util::to_string((int)iyear + 1) + "in dc loop");
				ireplast = ireport;
			}

			// only hourly electric load, even
			// if PV simulation is subhourly.  load is assumed constant over the hour.
			// if no load profile supplied, load = 0
			if (nload == 8760)
				cur_load = p_load_in[hour];

			for (size_t jj = 0; jj < step_per_hour; jj++)
			{
				// Reset dcPower calculation for new timestep
				dcPowerNetTotalSystem = 0; 

				// electric load is subhourly
				// if no load profile supplied, load = 0
				if (nload == nrec)
					cur_load = p_load_in[hour*step_per_hour + jj];

				// log cur_load to check both hourly and sub hourly load data
				// load data over entrie lifetime period not currently supported.
				//					log(util::format("year=%d, hour=%d, step per hour=%d, load=%g",
				//						iyear, hour, jj, cur_load), SSC_WARNING, (float)idx);
				p_load_full.push_back((ssc_number_t)cur_load);

				if (!wdprov->read(&Irradiance->weatherRecord))
					throw exec_error("pvsamv1", "could not read data line " + util::to_string((int)(idx + 1)) + " in weather file");

				weather_record wf = Irradiance->weatherRecord;

				//update POA data structure indicies if radmode is POA model is enabled
				if (radmode == irrad::POA_R || radmode == irrad::POA_P){
					for (size_t nn = 0; nn < num_subarrays; nn++){
						if (!Subarrays[nn]->enable) continue;

						Subarrays[nn]->poa.poaAll->tDew = wf.tdew;
						Subarrays[nn]->poa.poaAll->i = idx;
						if (jj == 0 && wf.hour == 0) {
							Subarrays[nn]->poa.poaAll->dayStart = idx;
							Subarrays[nn]->poa.poaAll->doy += 1;
						}
					}
				}
				
				double solazi = 0, solzen = 0, solalt = 0;
				int sunup = 0;

				// accumulators for radiation power (W) over this 
				// timestep from each subarray
				double ts_accum_poa_front_nom = 0.0;
				double ts_accum_poa_front_beam_nom = 0.0;
				double ts_accum_poa_front_shaded = 0.0;
				double ts_accum_poa_front_shaded_soiled = 0.0;
				double ts_accum_poa_front_total = 0.0;
				double ts_accum_poa_rear = 0.0;
				double ts_accum_poa_rear_after_losses = 0.0;
				double ts_accum_poa_total_eff = 0.0;
				double ts_accum_poa_front_beam_eff = 0.0;

				// calculate incident irradiance on each subarray
				std::vector<double> ipoa_rear, ipoa_rear_after_losses, ipoa_front, ipoa;
				double alb;
				alb = 0;

				for (size_t nn = 0; nn < num_subarrays; nn++)
				{
					ipoa_rear.push_back(0);
					ipoa_rear_after_losses.push_back(0);
					ipoa_front.push_back(0);
					ipoa.push_back(0);

					if (!Subarrays[nn]->enable
						|| Subarrays[nn]->nStrings < 1)
						continue; // skip disabled subarrays

					irrad irr(Irradiance->weatherRecord, Irradiance->weatherHeader,
						Irradiance->skyModel, Irradiance->radiationMode, Subarrays[nn]->trackMode,
						Irradiance->useWeatherFileAlbedo, Irradiance->instantaneous, Subarrays[nn]->backtrackingEnabled,
						Irradiance->dtHour, Subarrays[nn]->tiltDegrees, Subarrays[nn]->azimuthDegrees, Subarrays[nn]->trackerRotationLimitDegrees, Subarrays[nn]->groundCoverageRatio,
						Subarrays[nn]->monthlyTiltDegrees, Irradiance->userSpecifiedMonthlyAlbedo,
						Subarrays[nn]->poa.poaAll.get());
											
					int code = irr.calc();

					if (code < 0) //jmf updated 11/30/18 so that negative numbers are errors, positive numbers are warnings, 0 is everything correct. implemented in patch for POA model only, will be added to develop for other irrad models as well
						throw exec_error("pvsamv1",
						util::format("failed to calculate irradiance incident on surface (POA) %d (code: %d) [y:%d m:%d d:%d h:%d]",
						nn + 1, code, wf.year, wf.month, wf.day, wf.hour));

					if (code == 40)
						log(util::format("SAM calculated negative direct normal irradiance in the POA decomposition algorithm at time [y:%d m:%d d:%d h:%d], set to zero.",
							wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
					else if (code == 41)
						log(util::format("SAM calculated negative diffuse horizontal irradiance in the POA decomposition algorithm at time [y:%d m:%d d:%d h:%d], set to zero.",
							wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
					else if (code == 42)
						log(util::format("SAM calculated negative global horizontal irradiance in the POA decomposition algorithm at time [y:%d m:%d d:%d h:%d], set to zero.",
							wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
									   					 
					// p_irrad_calc is only weather file records long...
					if (iyear == 0)
					{
						if (radmode == irrad::POA_R || radmode == irrad::POA_P) {
							double gh_temp, df_temp, dn_temp;
							gh_temp = df_temp = dn_temp = 0;
							irr.get_irrad(&gh_temp, &dn_temp, &df_temp);
							Irradiance->p_IrradianceCalculated[1][idx] = (ssc_number_t)df_temp;
							Irradiance->p_IrradianceCalculated[2][idx] = (ssc_number_t)dn_temp;
						}
					}
					// beam, skydiff, and grounddiff IN THE PLANE OF ARRAY (W/m2)
					double ibeam, iskydiff, ignddiff;
					double aoi, stilt, sazi, rot, btd;

					// Ensure that the usePOAFromWF flag is false unless a reference cell has been used. 
					//  This will later get forced to false if any shading has been applied (in any scenario)
					//  also this will also be forced to false if using the cec mcsp thermal model OR if using the spe module model with a diffuse util. factor < 1.0
					Subarrays[nn]->poa.usePOAFromWF = false;
					if (radmode == irrad::POA_R){
						ipoa[nn] = wf.poa;
						Subarrays[nn]->poa.usePOAFromWF = true;
					}
					else if (radmode == irrad::POA_P){
						ipoa[nn] = wf.poa;
					}

					if (Subarrays[nn]->Module->simpleEfficiencyForceNoPOA && (radmode == irrad::POA_R || radmode == irrad::POA_P)){  // only will be true if using a poa model AND spe module model AND spe_fp is < 1
						Subarrays[nn]->poa.usePOAFromWF = false;
						if (idx == 0)
							log("The combination of POA irradiance as in input, single point efficiency module model, and module diffuse utilization factor less than one means that SAM must use a POA decomposition model to calculate the incident diffuse irradiance", SSC_WARNING);
					}

					if (Subarrays[nn]->Module->mountingSpecificCellTemperatureForceNoPOA && (radmode == irrad::POA_R || radmode == irrad::POA_P)){
						Subarrays[nn]->poa.usePOAFromWF = false;
						if (idx == 0)
							log("The combination of POA irradiance as input and heat transfer method for cell temperature means that SAM must use a POA decomposition model to calculate the beam irradiance required by the cell temperature model", SSC_WARNING);
					}


					// Get Incident angles and irradiances
					irr.get_sun(&solazi, &solzen, &solalt, 0, 0, 0, &sunup, 0, 0, 0);
					irr.get_angles(&aoi, &stilt, &sazi, &rot, &btd);
					irr.get_poa(&ibeam, &iskydiff, &ignddiff, 0, 0, 0);
					alb = irr.getAlbedo();

					if (iyear == 0)
						Irradiance->p_sunPositionTime[idx] = (ssc_number_t)irr.get_sunpos_calc_hour();

					// save weather file beam, diffuse, and global for output and for use later in pvsamv1- year 1 only
					/*jmf 2016: these calculations are currently redundant with calculations in irrad.calc() because ibeam and idiff in that function are DNI and DHI, **NOT** in the plane of array
					we'll have to fix this redundancy in the pvsamv1 rewrite. it will require allowing irradproc to report the errors below
					and deciding what to do if the weather file DOES contain the third component but it's not being used in the calculations.*/
					if (iyear == 0)
					{
						// Apply all irradiance component data from weather file (if it exists)
						Irradiance->p_weatherFilePOA[0][idx] = (ssc_number_t)wf.poa;
						Irradiance->p_weatherFileDNI[idx] = (ssc_number_t)wf.dn;
						Irradiance->p_weatherFileGHI[idx] = (ssc_number_t)(wf.gh);
						Irradiance->p_weatherFileDHI[idx] = (ssc_number_t)(wf.df);

						// calculate beam if global & diffuse are selected as inputs
						if (radmode == irrad::GH_DF)
						{
							Irradiance->p_IrradianceCalculated[2][idx] = (ssc_number_t)((wf.gh - wf.df) / cos(solzen*3.1415926 / 180));
							if (Irradiance->p_IrradianceCalculated[2][idx] < -1)
							{
								log(util::format("SAM calculated negative direct normal irradiance %lg W/m2 at time [y:%d m:%d d:%d h:%d], set to zero.",
									Irradiance->p_IrradianceCalculated[2][idx], wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
								Irradiance->p_IrradianceCalculated[2][idx] = 0;
							}
						}

						// calculate global if beam & diffuse are selected as inputs
						if (radmode == irrad::DN_DF)
						{
							Irradiance->p_IrradianceCalculated[0][idx] = (ssc_number_t)(wf.df + wf.dn * cos(solzen*3.1415926 / 180));
							if (Irradiance->p_IrradianceCalculated[0][idx] < -1)
							{
								log(util::format("SAM calculated negative global horizontal irradiance %lg W/m2 at time [y:%d m:%d d:%d h:%d], set to zero.",
									Irradiance->p_IrradianceCalculated[0][idx], wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
								Irradiance->p_IrradianceCalculated[0][idx] = 0;
							}
						}

						// calculate diffuse if total & beam are selected as inputs
						if (radmode == irrad::DN_GH)
						{
							Irradiance->p_IrradianceCalculated[1][idx] = (ssc_number_t)(wf.gh - wf.dn * cos(solzen*3.1415926 / 180));
							if (Irradiance->p_IrradianceCalculated[1][idx] < -1)
							{
								log(util::format("SAM calculated negative diffuse horizontal irradiance %lg W/m2 at time [y:%d m:%d d:%d h:%d], set to zero.",
									Irradiance->p_IrradianceCalculated[1][idx], wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
								Irradiance->p_IrradianceCalculated[1][idx] = 0;
							}
						}
					}

					// record sub-array plane of array output before computing shading and soiling
					if (iyear == 0)
					{
						if (radmode != irrad::POA_R)
							PVSystem->p_poaNominalFront[nn][idx] = (ssc_number_t)((ibeam + iskydiff + ignddiff));
						else
							PVSystem->p_poaNominalFront[nn][idx] = (ssc_number_t)((ipoa[nn]));
					}


					// record sub-array contribution to total POA power for this time step  (W)
					if (radmode != irrad::POA_R)
						ts_accum_poa_front_nom += (ibeam + iskydiff + ignddiff) * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;
					else
						ts_accum_poa_front_nom += (ipoa[nn])* ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;

					// record sub-array contribution to total POA beam power for this time step (W)
					ts_accum_poa_front_beam_nom += ibeam * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;

					// for non-linear shading from shading database
					if (Subarrays[nn]->shadeCalculator.use_shade_db())
					{
						double shadedb_gpoa = ibeam + iskydiff + ignddiff;
						double shadedb_dpoa = iskydiff + ignddiff;

						// update cell temperature - unshaded value per Sara 1/25/16
						double tcell = wf.tdry;
						if (sunup > 0)
						{
							// calculate cell temperature using selected temperature model
							pvinput_t in(ibeam, iskydiff, ignddiff, 0, ipoa[nn],
								wf.tdry, wf.tdew, wf.wspd, wf.wdir, wf.pres,
								solzen, aoi, hdr.elev,
								stilt, sazi,
								((double)wf.hour) + wf.minute / 60.0,
								radmode, Subarrays[nn]->poa.usePOAFromWF);
							// voltage set to -1 for max power
							(*Subarrays[nn]->Module->cellTempModel)(in, *Subarrays[nn]->Module->moduleModel, -1.0, tcell);
						}
						double shadedb_str_vmp_stc = Subarrays[nn]->nModulesPerString * Subarrays[nn]->Module->voltageMaxPower;
						double shadedb_mppt_lo = PVSystem->Inverter->mpptLowVoltage;
						double shadedb_mppt_hi = PVSystem->Inverter->mpptHiVoltage;
						 
						// shading database if necessary
						if (!Subarrays[nn]->shadeCalculator.fbeam_shade_db(shadeDatabase, hour, solalt, solazi, jj, step_per_hour, shadedb_gpoa, shadedb_dpoa, tcell, Subarrays[nn]->nModulesPerString, shadedb_str_vmp_stc, shadedb_mppt_lo, shadedb_mppt_hi))
						{
							throw exec_error("pvsamv1", util::format("Error calculating shading factor for subarray %d", nn));
						}
						if (iyear == 0)
						{
#ifdef SHADE_DB_OUTPUTS
							p_shadedb_gpoa[nn][idx] = (ssc_number_t)shadedb_gpoa;
							p_shadedb_dpoa[nn][idx] = (ssc_number_t)shadedb_dpoa;
							p_shadedb_pv_cell_temp[nn][idx] = (ssc_number_t)tcell;
							p_shadedb_mods_per_str[nn][idx] = (ssc_number_t)Subarrays[nn]->nModulesPerString;
							p_shadedb_str_vmp_stc[nn][idx] = (ssc_number_t)shadedb_str_vmp_stc;
							p_shadedb_mppt_lo[nn][idx] = (ssc_number_t)shadedb_mppt_lo;
							p_shadedb_mppt_hi[nn][idx] = (ssc_number_t)shadedb_mppt_hi;
							log("shade db hour " + util::to_string((int)hour) +"\n" + shadeCalculator->get_warning());
#endif
							// fraction shaded for comparison
							PVSystem->p_shadeDBShadeFraction[nn][idx] = (ssc_number_t)(Subarrays[nn]->shadeCalculator.dc_shade_factor());
						} 
					}
					else
					{
						if (!Subarrays[nn]->shadeCalculator.fbeam(hour, solalt, solazi, jj, step_per_hour))
						{
							throw exec_error("pvsamv1", util::format("Error calculating shading factor for subarray %d", nn));
						}
					}

					// apply hourly shading factors to beam (if none enabled, factors are 1.0) 
					// shj 3/21/16 - update to handle negative shading loss
					if (Subarrays[nn]->shadeCalculator.beam_shade_factor() != 1.0){
						//							if (sa[nn].shad.beam_shade_factor() < 1.0){
						// Sara 1/25/16 - shading database derate applied to dc only
						// shading loss applied to beam if not from shading database
						ibeam *= Subarrays[nn]->shadeCalculator.beam_shade_factor();
						if (radmode == irrad::POA_R || radmode == irrad::POA_P){
							Subarrays[nn]->poa.usePOAFromWF = false;
							if (Subarrays[nn]->poa.poaShadWarningCount == 0){
								log(util::format("Combining POA irradiance as input with the beam shading losses at time [y:%d m:%d d:%d h:%d] forces SAM to use a POA decomposition model to calculate incident beam irradiance",
									wf.year, wf.month, wf.day, wf.hour), SSC_WARNING, (float)idx);
							}
							else{
								log(util::format("Combining POA irradiance as input with the beam shading losses at time [y:%d m:%d d:%d h:%d] forces SAM to use a POA decomposition model to calculate incident beam irradiance",
									wf.year, wf.month, wf.day, wf.hour), SSC_NOTICE, (float)idx);
							}
							Subarrays[nn]->poa.poaShadWarningCount++;
						}
					}

					// apply sky diffuse shading factor (specified as constant, nominally 1.0 if disabled in UI)
					if (Subarrays[nn]->shadeCalculator.fdiff() < 1.0){
						iskydiff *= Subarrays[nn]->shadeCalculator.fdiff();
						if (radmode == irrad::POA_R || radmode == irrad::POA_P){
							if (idx == 0)
								log("Combining POA irradiance as input with the diffuse shading losses forces SAM to use a POA decomposition model to calculate incident diffuse irradiance", SSC_WARNING);
							Subarrays[nn]->poa.usePOAFromWF = false;
						}
					}

					double beam_shading_factor = Subarrays[nn]->shadeCalculator.beam_shade_factor();

					//self-shading calculations
					if (((Subarrays[nn]->trackMode == 0 || Subarrays[nn]->trackMode == 4) && (Subarrays[nn]->shadeMode == 1 || Subarrays[nn]->shadeMode == 2)) //fixed tilt or timeseries tilt, self-shading (linear or non-linear) OR
						|| (Subarrays[nn]->trackMode == 1 && (Subarrays[nn]->shadeMode == 1 || Subarrays[nn]->shadeMode == 2) && Subarrays[nn]->backtrackingEnabled == 0)) //one-axis tracking, self-shading, not backtracking
					{

						if (radmode == irrad::POA_R || radmode == irrad::POA_P){
							if (idx == 0)
								log("Combining POA irradiance as input with self shading forces SAM to employ a POA decomposition model to calculate incident beam irradiance", SSC_WARNING);
							Subarrays[nn]->poa.usePOAFromWF = false;
						}

						// info to be passed to self-shading function
						bool trackbool = (Subarrays[nn]->trackMode == 1);	// 0 for fixed tilt and timeseries tilt, 1 for one-axis
						bool linear = (Subarrays[nn]->shadeMode == 2); //0 for full self-shading, 1 for linear self-shading

						//geometric fraction of the array that is shaded for one-axis trackers.
						//USES A DIFFERENT FUNCTION THAN THE SELF-SHADING BECAUSE SS IS MEANT FOR FIXED ONLY. shadeFraction1x IS FOR ONE-AXIS TRACKERS ONLY.
						//used in the non-linear self-shading calculator for one-axis tracking only
						double shad1xf = 0;
						if (trackbool)
							shad1xf = shadeFraction1x(solazi, solzen, Subarrays[nn]->tiltDegrees, Subarrays[nn]->azimuthDegrees, Subarrays[nn]->groundCoverageRatio, rot);

						//execute self-shading calculations
						ssc_number_t beam_to_use; //some self-shading calculations require DNI, NOT ibeam (beam in POA). Need to know whether to use DNI from wf or calculated, depending on radmode
						if (radmode == irrad::DN_DF || radmode == irrad::DN_GH) beam_to_use = (ssc_number_t)wf.dn;
						else beam_to_use = Irradiance->p_IrradianceCalculated[2][hour * step_per_hour]; // top of hour in first year

						if (linear && trackbool) //one-axis linear
						{
							ibeam *= (1 - shad1xf); //derate beam irradiance linearly by the geometric shading fraction calculated above per Chris Deline 2/10/16
							beam_shading_factor *= (1 - shad1xf);
							if (iyear == 0)
							{
								PVSystem->p_derateSelfShading[nn][idx] = (ssc_number_t)1;
								PVSystem->p_derateLinear[nn][idx] = (ssc_number_t)(1 - shad1xf);
								PVSystem->p_derateSelfShadingDiffuse[nn][idx] = (ssc_number_t)1; //no diffuse derate for linear shading
								PVSystem->p_derateSelfShadingReflected[nn][idx] = (ssc_number_t)1; //no reflected derate for linear shading
							}
						}

						else if (ss_exec(Subarrays[nn]->selfShadingInputs, stilt, sazi, solzen, solazi, beam_to_use, ibeam, (iskydiff + ignddiff), alb, trackbool, linear, shad1xf, Subarrays[nn]->selfShadingOutputs))
						{
							if (linear) //fixed tilt linear
							{
								ibeam *= (1 - Subarrays[nn]->selfShadingOutputs.m_shade_frac_fixed);
								beam_shading_factor *= (1 - Subarrays[nn]->selfShadingOutputs.m_shade_frac_fixed);
								if (iyear == 0)
								{
									PVSystem->p_derateSelfShading[nn][idx] = (ssc_number_t)1;
									PVSystem->p_derateLinear[nn][idx] = (ssc_number_t)(1 - Subarrays[nn]->selfShadingOutputs.m_shade_frac_fixed);
									PVSystem->p_derateSelfShadingDiffuse[nn][idx] = (ssc_number_t)1; //no diffuse derate for linear shading
									PVSystem->p_derateSelfShadingReflected[nn][idx] = (ssc_number_t)1; //no reflected derate for linear shading
								}
							}
							else //non-linear: fixed tilt AND one-axis
							{
								if (iyear == 0)
								{
									PVSystem->p_derateSelfShadingDiffuse[nn][idx] = (ssc_number_t)Subarrays[nn]->selfShadingOutputs.m_diffuse_derate;
									PVSystem->p_derateSelfShadingReflected[nn][idx] = (ssc_number_t)Subarrays[nn]->selfShadingOutputs.m_reflected_derate;
									PVSystem->p_derateSelfShading[nn][idx] = (ssc_number_t)Subarrays[nn]->selfShadingOutputs.m_dc_derate;
									PVSystem->p_derateLinear[nn][idx] = (ssc_number_t)1;
								}

								// Sky diffuse and ground-reflected diffuse are derated according to C. Deline's algorithm
								iskydiff *= Subarrays[nn]->selfShadingOutputs.m_diffuse_derate;
								ignddiff *= Subarrays[nn]->selfShadingOutputs.m_reflected_derate;
								// Beam is not derated- all beam derate effects (linear and non-linear) are taken into account in the nonlinear_dc_shading_derate
								Subarrays[nn]->poa.nonlinearDCShadingDerate = Subarrays[nn]->selfShadingOutputs.m_dc_derate;
							}
						}
						else
							throw exec_error("pvsamv1", util::format("Self-shading calculation failed at %d", (int)idx));
					}

					double poashad = (radmode == irrad::POA_R) ? ipoa[nn] : (ibeam + iskydiff + ignddiff);

					// determine sub-array contribution to total shaded plane of array for this hour
					ts_accum_poa_front_shaded += poashad * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;

					// apply soiling derate to all components of irradiance
					double soiling_factor = 1.0;
					int month_idx = wf.month - 1;
					if (month_idx >= 0 && month_idx < 12)
					{
						soiling_factor = Subarrays[nn]->monthlySoiling[month_idx];
						ibeam *= soiling_factor;
						iskydiff *= soiling_factor;
						ignddiff *= soiling_factor;
						if (radmode == irrad::POA_R || radmode == irrad::POA_P){
							ipoa[nn] *= soiling_factor;
							if (soiling_factor < 1 && idx == 0)
								log("Soiling may already be accounted for in the input POA data. Please confirm that the input data does not contain soiling effects, or remove the additional losses on the Losses page.", SSC_WARNING);
						}
						beam_shading_factor *= soiling_factor;
					}

					// Calculate total front irradiation after soiling added to shading
					ipoa_front[nn] = ibeam + iskydiff + ignddiff;
					ts_accum_poa_front_shaded_soiled += ipoa_front[nn] * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;
					
					// Calculate rear-side irradiance for bifacial modules
					if (Subarrays[0]->Module->isBifacial)
					{
<<<<<<< HEAD
						// save sub-array level outputs			
						p_poashaded[nn][idx] = (ssc_number_t)poashad;
						p_poaeffbeam[nn][idx] = (ssc_number_t)ibeam;
						p_poaeffdiff[nn][idx] = (ssc_number_t)(iskydiff + ignddiff);
						p_poaeff[nn][idx] = (radmode == POA_R) ? (ssc_number_t)ipoa : (ssc_number_t)(ibeam + iskydiff + ignddiff);
						p_shad[nn][idx] = (ssc_number_t)beam_shading_factor;
						p_rot[nn][idx] = (ssc_number_t)rot;
						p_idealrot[nn][idx] = (ssc_number_t)(rot - btd);
						p_aoi[nn][idx] = (ssc_number_t)aoi;
						p_surftilt[nn][idx] = (ssc_number_t)stilt;
						p_surfazi[nn][idx] = (ssc_number_t)sazi;
						p_soiling[nn][idx] = (ssc_number_t)soiling_factor;
=======
						double slopeLength = Subarrays[nn]->selfShadingInputs.length * Subarrays[nn]->selfShadingInputs.nmody;
						if (Subarrays[nn]->selfShadingInputs.mod_orient == 1) {
							slopeLength = Subarrays[nn]->selfShadingInputs.width * Subarrays[nn]->selfShadingInputs.nmody;
						}
						irr.calc_rear_side(Subarrays[0]->Module->bifacialTransmissionFactor, Subarrays[0]->Module->bifaciality, Subarrays[0]->Module->groundClearanceHeight, slopeLength);
						ipoa_rear[nn] = irr.get_poa_rear();
						ipoa_rear_after_losses[nn] = ipoa_rear[nn] * (1 - Subarrays[nn]->rearIrradianceLossPercent);
					}

					ts_accum_poa_rear += ipoa_rear[nn] * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;
					ts_accum_poa_rear_after_losses = ts_accum_poa_rear * (1 - Subarrays[nn]->rearIrradianceLossPercent);

					if (iyear == 0) 
					{
						// save sub-array level outputs			
						PVSystem->p_poaShadedFront[nn][idx] = (ssc_number_t)poashad;
						PVSystem->p_poaShadedSoiledFront[nn][idx] = (ssc_number_t)ipoa_front[nn];
						PVSystem->p_poaBeamFront[nn][idx] = (ssc_number_t)ibeam;
						PVSystem->p_poaDiffuseFront[nn][idx] = (ssc_number_t)(iskydiff + ignddiff);
						PVSystem->p_poaRear[nn][idx] = (ssc_number_t)(ipoa_rear_after_losses[nn]);
						PVSystem->p_beamShadingFactor[nn][idx] = (ssc_number_t)beam_shading_factor;
						PVSystem->p_axisRotation[nn][idx] = (ssc_number_t)rot;
						PVSystem->p_idealRotation[nn][idx] = (ssc_number_t)(rot - btd);
						PVSystem->p_angleOfIncidence[nn][idx] = (ssc_number_t)aoi;
						PVSystem->p_surfaceTilt[nn][idx] = (ssc_number_t)stilt;
						PVSystem->p_surfaceAzimuth[nn][idx] = (ssc_number_t)sazi;
						PVSystem->p_derateSoiling[nn][idx] = (ssc_number_t)soiling_factor;
>>>>>>> pr/11
					}

					// accumulate incident total radiation (W) in this timestep (all subarrays)
					ts_accum_poa_front_beam_eff += ibeam * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;

					// save the required irradiance inputs on array plane for the module output calculations.
					Subarrays[nn]->poa.poaBeamFront = ibeam;
					Subarrays[nn]->poa.poaDiffuseFront = iskydiff;
					Subarrays[nn]->poa.poaGroundFront = ignddiff;
					Subarrays[nn]->poa.poaRear = ipoa_rear_after_losses[nn];
					Subarrays[nn]->poa.poaTotal = (radmode == irrad::POA_R) ? ipoa[nn] :(ipoa_front[nn] + ipoa_rear_after_losses[nn]);
					Subarrays[nn]->poa.angleOfIncidenceDegrees = aoi;
					Subarrays[nn]->poa.sunUp = sunup;
					Subarrays[nn]->poa.surfaceTiltDegrees = stilt;
					Subarrays[nn]->poa.surfaceAzimuthDegrees = sazi;
				}

				std::vector<double> mpptVoltageClipping; //a vector to store power that is clipped due to the inverter MPPT low & high voltage limits for each subarray
				for (size_t nn = 0; nn < PVSystem->numberOfSubarrays; nn++) {
					mpptVoltageClipping.push_back(0.0);
				}

				//Calculate power of each MPPT input
				for (size_t mpptInput = 0; mpptInput < PVSystem->Inverter->nMpptInputs; mpptInput++) //remember that actual named mppt inputs are 1-indexed, and these are 0-indexed
				{
					int nSubarraysOnMpptInput = (int)(PVSystem->mpptMapping[mpptInput].size()); //number of subarrays attached to this MPPT input
					std::vector<int> SubarraysOnMpptInput = PVSystem->mpptMapping[mpptInput]; //vector of which subarrays are attached to this MPPT input

					//string voltage for this MPPT input- if 1 subarray, this will be the string voltage. if >1 subarray and mismatch enabled, this
					//will be the string voltage found by the mismatch calculation. if >1 subarray and mismatch not enabled, this will be the average
					//voltage of the strings from all the subarrays on this mppt input.
					//initialize it as -1 and check for that later
					double stringVoltage = -1;

					//mismatch calculations assume that the inverter MPPT operates all strings on that MPPT input at the same voltage.
					//this algorithm sweeps across a range of string voltages, calculating total power for all strings on this MPPT input at each voltage.
					//it finds the maximum total power of all string voltages swept, then uses that in subsequent power calculations for each subarray. 
					if (PVSystem->enableMismatchVoltageCalc)
					{
						double vmax = PVSystem->Inverter->mpptHiVoltage; //the upper MPPT range of the inverter is the high end for string voltages that it will control
						double vmin = PVSystem->Inverter->mpptLowVoltage; //the lower MPPT range of the inverter is the low end for string voltages that it will control
						const int NP = 100; //number of points in between max and min voltage to sweep
						double Pmax = 0; //variable to store the maximum power for comparison between different points along the voltage sweep
						// sweep voltage, calculating current for each subarray, add all subarray currents together at each voltage
						for (int i = 0; i < NP; i++)
						{
							double stringV = vmin + (vmax - vmin)*i / ((double)NP); //voltage of a string at this point in the voltage sweep							

							//if the voltage is ok, continue to calculate total power on this MPPT input at this voltage
							double P = 0; //temporary variable to store the total power on this MPPT input at this voltage
							for (int nSubarray = 0; nSubarray < nSubarraysOnMpptInput; nSubarray++) //sweep across all subarrays connected to this MPPT input
							{
								int nn = SubarraysOnMpptInput[nSubarray]; //get the index of the subarray we're checking here
								double V = stringV / (double)Subarrays[nn]->nModulesPerString; //voltage of an individual module on a string on this subarray

								//initalize pvinput and pvoutput structures for the model
								pvinput_t in(Subarrays[nn]->poa.poaBeamFront, Subarrays[nn]->poa.poaDiffuseFront, Subarrays[nn]->poa.poaGroundFront, Subarrays[nn]->poa.poaRear, Subarrays[nn]->poa.poaTotal,
									wf.tdry, wf.tdew, wf.wspd, wf.wdir, wf.pres,
									solzen, Subarrays[nn]->poa.angleOfIncidenceDegrees, hdr.elev,
									Subarrays[nn]->poa.surfaceTiltDegrees, Subarrays[nn]->poa.surfaceAzimuthDegrees,
									((double)wf.hour) + wf.minute / 60.0,
									radmode, Subarrays[nn]->poa.usePOAFromWF);
								pvoutput_t out(0, 0, 0, 0, 0, 0, 0, 0);

								//calculate the output power for one module in this subarray at this voltage
								if (Subarrays[nn]->poa.sunUp)
								{
									double tcell = wf.tdry;
									// calculate cell temperature using selected temperature model
									(*Subarrays[nn]->Module->cellTempModel)(in, *Subarrays[nn]->Module->moduleModel, V, tcell);
									// calculate module power output using conversion model previously specified
									(*Subarrays[nn]->Module->moduleModel)(in, tcell, V, out);
								}
								//add the power from this subarray to the total power
								P += V * out.Current * (double)Subarrays[nn]->nModulesPerString * (double)Subarrays[nn]->nStrings;
							}

							//check if the total power at this voltage is higher than the power values we've calculated before, if so, set it as the new max
							if (P > Pmax)
							{
								Pmax = P;
								stringVoltage = stringV;
							}
						}

					} //now we have the string voltage at which the MPPT input will produce max power, to be used in subsequent calcs

					//now calculate power for each subarray on this mppt input. stringVoltage will still be -1 if mismatch calcs aren't enabled, or the value decided by mismatch calcs if they are enabled
					std::vector<pvinput_t> in{ num_subarrays }; //create arrays for the pv input and output structures because we have to deal with them in multiple loops to check for MPPT clipping
					std::vector<pvoutput_t> out{ num_subarrays };
					double tcell = wf.tdry;
					for (int nSubarray = 0; nSubarray < nSubarraysOnMpptInput; nSubarray++) //sweep across all subarrays connected to this MPPT input
					{
						int nn = SubarraysOnMpptInput[nSubarray]; //get the index of the subarray we're checking here
						//initalize pvinput and pvoutput structures for the model
						pvinput_t in_temp(Subarrays[nn]->poa.poaBeamFront, Subarrays[nn]->poa.poaDiffuseFront, Subarrays[nn]->poa.poaGroundFront, Subarrays[nn]->poa.poaRear, Subarrays[nn]->poa.poaTotal,
							wf.tdry, wf.tdew, wf.wspd, wf.wdir, wf.pres,
							solzen, Subarrays[nn]->poa.angleOfIncidenceDegrees, hdr.elev,
							Subarrays[nn]->poa.surfaceTiltDegrees, Subarrays[nn]->poa.surfaceAzimuthDegrees,
							((double)wf.hour) + wf.minute / 60.0,
							radmode, Subarrays[nn]->poa.usePOAFromWF);
						pvoutput_t out_temp(0, 0, 0, 0, 0, 0, 0, 0);
						in[nn] = in_temp;
						out[nn] = out_temp;					
						
						if (Subarrays[nn]->poa.sunUp)
						{
							//module voltage value to be passed into module power function. 
							//if -1 is passed in, power will be calculated at max power point. 
							//if a voltage value is passed in, power will be calculated at the specified voltage for all single-diode module models
							double module_voltage = -1;
							if (stringVoltage != -1) module_voltage = stringVoltage / (double)Subarrays[nn]->nModulesPerString;
							// calculate cell temperature using selected temperature model
							// calculate module power output using conversion model previously specified
							(*Subarrays[nn]->Module->cellTempModel)(in[nn], *Subarrays[nn]->Module->moduleModel, module_voltage, tcell);
							(*Subarrays[nn]->Module->moduleModel)(in[nn], tcell, module_voltage, out[nn]);
						}
					}

					//assign input voltage at this MPPT input
					//if mismatch was enabled, the voltage already was clipped to the inverter MPPT range as needed and  
					//the string voltage is the same for all subarrays, so the voltage at the MPPT input is the same as the string voltage of any subarray
					if (PVSystem->enableMismatchVoltageCalc) {
						PVSystem->p_mpptVoltage[mpptInput][idx] = (ssc_number_t)out[SubarraysOnMpptInput[0]].Voltage * Subarrays[SubarraysOnMpptInput[0]]->nModulesPerString;
					}
					//if mismatch wasn't enabled, we assume the MPPT input voltage is a weighted average of the string voltages on this MPPT input,
					//and still need to check that average against the inverter MPPT bounds
					else
					{
						//create temporary values to calculate the weighted average string voltage
						double nStrings = 0;
						double avgVoltage = 0;
						for (int nSubarray = 0; nSubarray < nSubarraysOnMpptInput; nSubarray++)
						{
							int nn = SubarraysOnMpptInput[nSubarray]; //get the index of the subarray itself
							nStrings += Subarrays[nn]->nStrings;
							avgVoltage += out[nn].Voltage * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;
						}
						avgVoltage /= nStrings;
						PVSystem->p_mpptVoltage[mpptInput][idx] = (ssc_number_t)avgVoltage;

						//check the weighted average string voltage against the inverter MPPT bounds
						bool recalculatePower = false;
						if (PVSystem->clipMpptWindow)
						{
							if (avgVoltage < PVSystem->Inverter->mpptLowVoltage)
							{
								avgVoltage = PVSystem->Inverter->mpptLowVoltage;
								recalculatePower = true;
							}
							else if (avgVoltage > PVSystem->Inverter->mpptHiVoltage)
							{
								avgVoltage = PVSystem->Inverter->mpptHiVoltage;
								recalculatePower = true;
							}
							
							//if MPPT clipping occurs, we need to recalculate the module power for each subarray
							if (recalculatePower)
							{
								for (int nSubarray = 0; nSubarray < nSubarraysOnMpptInput; nSubarray++) //sweep across all subarrays connected to this MPPT input
								{
									int nn = SubarraysOnMpptInput[nSubarray]; //get the index of the subarray we're checking here

									if (iyear == 0) mpptVoltageClipping[nn] = out[nn].Power; //initialize the voltage clipping loss with the power at module MPP, subtract from this later for the actual MPPT clipping loss

									//recalculate power at the correct voltage
									double module_voltage = avgVoltage / (double)Subarrays[nn]->nModulesPerString;
									(*Subarrays[nn]->Module->cellTempModel)(in[nn], *Subarrays[nn]->Module->moduleModel, module_voltage, tcell);
									(*Subarrays[nn]->Module->moduleModel)(in[nn], tcell, module_voltage, out[nn]);

									if (iyear == 0)	mpptVoltageClipping[nn] -= out[nn].Power; //subtract the power that remains after voltage clipping in order to get the total loss. if no power was lost, all the power will be subtracted away again.
								}
							}
						}
					}

					//now that we have the correct power for all subarrays, subject to inverter MPPT clipping, save outputs 
					for (int nSubarray = 0; nSubarray < nSubarraysOnMpptInput; nSubarray++) //sweep across all subarrays connected to this MPPT input
					{
						int nn = SubarraysOnMpptInput[nSubarray]; //get the index of the subarray we're checking here

						//check for weird results
						if (out[nn].Voltage > Subarrays[nn]->Module->moduleModel->VocRef()*1.3)
							log(util::format("Module voltage is unrealistically high (exceeds 1.3*VocRef) at [mdhm: %d %d %d %lg]: %lg V\n", wf.month, wf.day, wf.hour, wf.minute, out[nn].Voltage), SSC_NOTICE);
						if (!std::isfinite(out[nn].Power))
						{
							out[nn].Power = 0;
							out[nn].Voltage = 0;
							out[nn].Current = 0;
							out[nn].Efficiency = 0;
							out[nn].CellTemp = tcell;
							log(util::format("Non-finite power output calculated at [mdhm: %d %d %d %lg], set to zero.\n"
								"could be due to anomolous equation behavior at very low irradiances (poa: %lg W/m2)",
								wf.month, wf.day, wf.hour, wf.minute, Subarrays[nn]->poa.poaTotal), SSC_NOTICE);
						}

						// save DC module outputs for this subarray
						Subarrays[nn]->Module->dcPowerW = out[nn].Power;
						Subarrays[nn]->Module->dcEfficiency = out[nn].Efficiency * 100;
						Subarrays[nn]->Module->dcVoltage = out[nn].Voltage;
						Subarrays[nn]->Module->temperatureCellCelcius = out[nn].CellTemp;
						Subarrays[nn]->Module->currentShortCircuit = out[nn].Isc_oper;
						Subarrays[nn]->Module->voltageOpenCircuit = out[nn].Voc_oper;
						Subarrays[nn]->Module->angleOfIncidenceModifier = out[nn].AOIModifier;
						
						// Lifetime dcStringVoltage
						dcStringVoltage[nn].push_back(Subarrays[nn]->Module->dcVoltage * Subarrays[nn]->nModulesPerString);

						// Output front-side irradiance after the reflection (IAM) loss - needs to be after the module model for now because reflection effects are part of the module model
						if (iyear == 0)
						{
							ipoa_front[nn] *= out[nn].AOIModifier;
							PVSystem->p_poaFront[nn][idx] = (radmode == irrad::POA_R) ? (ssc_number_t)ipoa[nn] : (ssc_number_t)(ipoa_front[nn]);
							PVSystem->p_poaTotal[nn][idx] = (radmode == irrad::POA_R) ? (ssc_number_t)ipoa[nn] : (ssc_number_t)(ipoa_front[nn] + ipoa_rear[nn]);

							ts_accum_poa_front_total += ipoa_front[nn] * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;
							ts_accum_poa_total_eff += ((radmode == irrad::POA_R) ? ipoa[nn] : (ipoa_front[nn] + ipoa_rear_after_losses[nn])) * ref_area_m2 * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;

							//assign final string voltage output
							PVSystem->p_dcStringVoltage[nn][idx] = (ssc_number_t)Subarrays[nn]->Module->dcVoltage * Subarrays[nn]->nModulesPerString;
						}
					}
				}

				// sum up all DC power from the whole array
				PVSystem->p_systemDCPower[idx] = 0;
				for (size_t nn = 0; nn < num_subarrays; nn++)
				{
					// DC derates for snow and shading must be applied first
					// these can't be applied before the power calculation because they are POWER derates

					// self-shading derate (by default it is 1.0 if disbled)
					Subarrays[nn]->Module->dcPowerW *= Subarrays[nn]->poa.nonlinearDCShadingDerate;
					if (iyear == 0) mpptVoltageClipping[nn] *= Subarrays[nn]->poa.nonlinearDCShadingDerate;

					// Sara 1/25/16 - shading database derate applied to dc only
					// shading loss applied to beam if not from shading database
					Subarrays[nn]->Module->dcPowerW *= Subarrays[nn]->shadeCalculator.dc_shade_factor();

					// Calculate and apply snow coverage losses if activated
					if (PVSystem->enableSnowModel)
					{
						float smLoss = 0.0f;

						if (Subarrays[nn]->snowModel.getLoss((float)(Subarrays[nn]->poa.poaBeamFront + Subarrays[nn]->poa.poaDiffuseFront + Subarrays[nn]->poa.poaGroundFront),
							(float)Subarrays[nn]->poa.surfaceTiltDegrees, (float)wf.wspd, (float)wf.tdry, (float)wf.snow, sunup, 1.0f / step_per_hour, smLoss))
						{
							if (!Subarrays[nn]->snowModel.good)
								throw exec_error("pvsamv1", Subarrays[nn]->snowModel.msg);
						}

						if (iyear == 0)
						{
							PVSystem->p_snowLoss[nn][idx] = (ssc_number_t)(util::watt_to_kilowatt*Subarrays[nn]->Module->dcPowerW*smLoss);
							PVSystem->p_snowLossTotal[idx] += (ssc_number_t)(util::watt_to_kilowatt*Subarrays[nn]->Module->dcPowerW*smLoss);
							PVSystem->p_snowCoverage[nn][idx] = (ssc_number_t)(Subarrays[nn]->snowModel.coverage);
							annual_snow_loss += (ssc_number_t)(util::watt_to_kilowatt*Subarrays[nn]->Module->dcPowerW*smLoss);
						}

						Subarrays[nn]->Module->dcPowerW *= (1 - smLoss);
						if (iyear == 0) mpptVoltageClipping[nn] *= (1 - smLoss);
					}

					// scale power and mppt voltage clipping to subarray dimensions
					Subarrays[nn]->dcPowerSubarray = Subarrays[nn]->Module->dcPowerW * Subarrays[nn]->nModulesPerString * Subarrays[nn]->nStrings;
					if (iyear == 0) mpptVoltageClipping[nn] *= Subarrays[nn]->nModulesPerString* Subarrays[nn]->nStrings;

					//assign gross outputs per subarray at this point
					if (iyear == 0)
					{
						//Gross DC power
						dc_gross[nn] += Subarrays[nn]->dcPowerSubarray*util::watt_to_kilowatt*ts_hour; //power W to	energy kWh
						PVSystem->p_dcPowerGross[nn][idx] = (ssc_number_t)dc_gross[nn];
						//Add to annual MPPT clipping
						annualMpptVoltageClipping += mpptVoltageClipping[nn]*util::watt_to_kilowatt*ts_hour; //power W to energy kWh
						// save to SSC output arrays
						PVSystem->p_temperatureCell[nn][idx] = (ssc_number_t)Subarrays[nn]->Module->temperatureCellCelcius;
						PVSystem->p_moduleEfficiency[nn][idx] = (ssc_number_t)Subarrays[nn]->Module->dcEfficiency;					
						PVSystem->p_voltageOpenCircuit[nn][idx] = (ssc_number_t)(Subarrays[nn]->Module->voltageOpenCircuit * (double)Subarrays[nn]->nModulesPerString);
						PVSystem->p_currentShortCircuit[nn][idx] = (ssc_number_t)Subarrays[nn]->Module->currentShortCircuit;
						PVSystem->p_angleOfIncidenceModifier[nn][idx] = (ssc_number_t)(Subarrays[nn]->Module->angleOfIncidenceModifier);

					}
					
					//calculate net power for each subarray

					// apply pre-inverter power derate
					dcPowerNetPerSubarray[nn] = Subarrays[nn]->dcPowerSubarray * (1 - Subarrays[nn]->dcLossTotalPercent);

					//module degradation and lifetime DC losses apply to all subarrays
					if (system_use_lifetime_output == 1)
						dcPowerNetPerSubarray[nn] *= PVSystem->dcDegradationFactor[iyear + 1];

					//dc adjustment factors apply to all subarrays
					if (iyear == 0) annual_dc_adjust_loss += dcPowerNetPerSubarray[nn] * (1 - dc_haf(hour)) * util::watt_to_kilowatt * ts_hour; //only keep track of this loss for year 0, convert from power W to energy kWh
					dcPowerNetPerSubarray[nn] *= dc_haf(hour);

					//lifetime daily DC losses apply to all subarrays and should be applied last. Only applied if they are enabled.
					if (system_use_lifetime_output == 1 && PVSystem->enableDCLifetimeLosses)
					{
						//current index of the lifetime daily DC losses is the number of years that have passed (iyear, because it is 0-indexed) * the number of days + the number of complete days that have passed
						int dc_loss_index = (int)iyear * 365 + (int)floor(hour / 24); //in units of days
						if (iyear == 0) annual_dc_lifetime_loss += dcPowerNetPerSubarray[nn] * (PVSystem->p_dcLifetimeLosses[dc_loss_index] / 100) * util::watt_to_kilowatt * ts_hour; //this loss is still in percent, only keep track of it for year 0, convert from power W to energy kWh
						dcPowerNetPerSubarray[nn] *= (100 - PVSystem->p_dcLifetimeLosses[dc_loss_index]) / 100;
					}

					//assign net DC power output
					PVSystem->p_systemDCPower[idx] += (ssc_number_t)(dcPowerNetPerSubarray[nn] * util::watt_to_kilowatt);

					//add this subarray's net DC power to the appropriate MPPT input and to the total system DC power
					PVSystem->p_dcPowerNetPerMppt[Subarrays[nn]->mpptInput - 1][idx] += (ssc_number_t)(dcPowerNetPerSubarray[nn]); //need to subtract 1 from mppt input number because those are 1-indexed
					dcPowerNetTotalSystem += dcPowerNetPerSubarray[nn];	
				}								

				// save other array-level environmental and irradiance outputs	- year 1 only outputs
				if (iyear == 0)
				{
					Irradiance->p_weatherFileWindSpeed[idx] = (ssc_number_t)wf.wspd;
					Irradiance->p_weatherFileAmbientTemp[idx] = (ssc_number_t)wf.tdry;
					Irradiance->p_weatherFileAlbedo[idx] = (ssc_number_t)alb;
					Irradiance->p_weatherFileSnowDepth[idx] = (ssc_number_t)wf.snow;

					Irradiance->p_sunZenithAngle[idx] = (ssc_number_t)solzen;
					Irradiance->p_sunAltitudeAngle[idx] = (ssc_number_t)solalt;
					Irradiance->p_sunAzimuthAngle[idx] = (ssc_number_t)solazi;

					// absolute relative airmass calculation as f(zenith angle, site elevation)
					Irradiance->p_absoluteAirmass[idx] = sunup > 0 ? (ssc_number_t)(exp(-0.0001184 * hdr.elev) / (cos(solzen*3.1415926 / 180) + 0.5057*pow(96.080 - solzen, -1.634))) : 0.0f;
					Irradiance->p_sunUpOverHorizon[idx] = (ssc_number_t)sunup;

					// Sum of radiation power on each subarray for the current timestep [kW]
					PVSystem->p_poaFrontNominalTotal[idx] = (ssc_number_t)(ts_accum_poa_front_nom * util::watt_to_kilowatt); 
					PVSystem->p_poaFrontBeamNominalTotal[idx] = (ssc_number_t)(ts_accum_poa_front_beam_nom * util::watt_to_kilowatt); 
					PVSystem->p_poaFrontShadedTotal[idx] = (ssc_number_t)(ts_accum_poa_front_shaded * util::watt_to_kilowatt); 
					PVSystem->p_poaFrontShadedSoiledTotal[idx] = (ssc_number_t)(ts_accum_poa_front_shaded_soiled * util::watt_to_kilowatt);
					PVSystem->p_poaFrontTotal[idx] = (ssc_number_t)(ts_accum_poa_front_total * util::watt_to_kilowatt);
					PVSystem->p_poaRearTotal[idx] = (ssc_number_t)(ts_accum_poa_rear_after_losses * util::watt_to_kilowatt);
					PVSystem->p_poaTotalAllSubarrays[idx] = (ssc_number_t)(ts_accum_poa_total_eff * util::watt_to_kilowatt); 
					PVSystem->p_poaFrontBeamTotal[idx] = (ssc_number_t)(ts_accum_poa_front_beam_eff * util::watt_to_kilowatt);
					PVSystem->p_inverterMPPTLoss[idx] = 0;
					for (size_t nn = 0; nn < num_subarrays; nn++) {
						PVSystem->p_inverterMPPTLoss[idx] = (ssc_number_t)(mpptVoltageClipping[nn] * util::watt_to_kilowatt);
					}
				}

				// Predict clipping for DC battery controller
				if (en_batt)
				{
					double cliploss = 0;
					double dcpwr_kw = PVSystem->p_systemDCPower[idx];

					if (p_pv_dc_forecast.size() > 1 && p_pv_dc_forecast.size() > idx % (8760 * step_per_hour)) {
						dcpwr_kw = p_pv_dc_forecast[idx % (8760 * step_per_hour)];
					}
					p_pv_dc_use.push_back(static_cast<ssc_number_t>(dcpwr_kw));

					if (p_pv_clipping_forecast.size() > 1 && p_pv_clipping_forecast.size() > idx % (8760 * step_per_hour)) {
						cliploss = p_pv_clipping_forecast[idx % (8760 * step_per_hour)] * util::kilowatt_to_watt;
					}
					else {
						//DC batteries not allowed with multiple MPPT, so can just use MPPT 1's voltage
						sharedInverter->calculateACPower(dcpwr_kw, PVSystem->p_mpptVoltage[0][idx], 0.0);
						cliploss = sharedInverter->powerClipLoss_kW;
					}

					p_invcliploss_full.push_back(static_cast<ssc_number_t>(cliploss));
				}

				idx++;
			}
		}
		// using single weather file initially - so rewind to use for next year
		wdprov->rewind();

		// Assign annual lifetime DC outputs
		if (system_use_lifetime_output) {
			PVSystem->p_dcDegradationFactor[iyear] = (ssc_number_t)(PVSystem->dcDegradationFactor[iyear]);
		}
	}

	// Initialize DC battery predictive controller
	if (en_batt && (batt_topology == ChargeController::DC_CONNECTED))
		batt.initialize_automated_dispatch(util::array_to_vector<ssc_number_t>(PVSystem->p_systemDCPower, nlifetime), p_load_full, p_invcliploss_full);

	/* *********************************************************************************************
	PV AC calculation
	*********************************************************************************************** */
	idx = 0; ireport = 0; ireplast = 0; percent_baseline = percent_complete;
<<<<<<< HEAD
	double annual_dc_power_before_battery = 0, annual_dc_power_after_battery = 0;

	double annual_dc_loss_ond = 0, annual_ac_loss_ond = 0; // (TR)

=======
	double annual_battery_loss = 0;
	wdprov->rewind();

	double annual_dc_loss_ond = 0, annual_ac_loss_ond = 0; // (TR)


>>>>>>> pr/11
	for (size_t iyear = 0; iyear < nyears; iyear++)
	{
		for (hour = 0; hour < 8760; hour++)
		{
			// report progress updates to the caller	
			ireport++;
			if (ireport - ireplast > irepfreq)
			{
				percent_complete = percent_baseline + 100.0f *(float)(hour + iyear * 8760) / (float)(insteps);
				if (!update("", percent_complete))
					throw exec_error("pvsamv1", "simulation canceled at hour " + util::to_string(hour + 1.0) + " in year " + util::to_string((int)iyear + 1) + "in ac loop");
				ireplast = ireport;
			}

			for (size_t jj = 0; jj < step_per_hour; jj++)
			{
				double dcPower_kW = PVSystem->p_systemDCPower[idx];

				// Battery replacement
				if (en_batt && (batt_topology == ChargeController::DC_CONNECTED))
				{
					batt.initialize_time(iyear, hour, jj);
					batt.check_replacement_schedule();
				}

<<<<<<< HEAD
				// Iterative loop over DC battery
				size_t dc_count = 0; bool iterate_dc = false;
				double dcpwr_net = 0, acpwr_gross = 0, aceff = 0, pntloss = 0, psoloss = 0, cliploss = 0, ac_wiringloss = 0;
				double dc_wiringloss = 0; // Added by TR for OND model

				do {
					cur_load = p_load_full[idx];
					dcpwr_net = util::kilowatt_to_watt * p_dcpwr[idx];
					double dc_string_voltage = p_inv_dc_voltage[idx];

					// DC Connected Battery
					bool battery_charging = false;
					if (en_batt && (ac_or_dc == charge_controller::DC_CONNECTED))
					{
						if (iyear == 0 && dc_count == 0)
							annual_dc_power_before_battery += p_dcpwr[idx] * ts_hour;
=======
				double acpwr_gross = 0, ac_wiringloss = 0, transmissionloss = 0;
				cur_load = p_load_full[idx];
				wdprov->read(&Irradiance->weatherRecord);
				weather_record wf = Irradiance->weatherRecord;

				//set DC voltages for use in AC power calculation
				for (size_t m = 0; m < PVSystem->Inverter->nMpptInputs; m++)
				{
					dcVoltagePerMppt[m] = PVSystem->p_mpptVoltage[m][idx];
					dcPowerNetPerMppt_kW[m] = PVSystem->p_dcPowerNetPerMppt[m][idx] * util::watt_to_kilowatt;
				}
>>>>>>> pr/11

				//run AC power calculation
				if (en_batt && (batt_topology == ChargeController::DC_CONNECTED)) // DC-connected battery
				{
					// Compute PV clipping before adding battery
					sharedInverter->calculateACPower(dcPower_kW, dcVoltagePerMppt[0], wf.tdry); //DC batteries not allowed with multiple MPPT, so can just use MPPT 1's voltage

					// Run PV plus battery through sharedInverter, returns AC power
					batt.advance(*this, dcPower_kW, dcVoltagePerMppt[0], cur_load, sharedInverter->powerClipLoss_kW);
					acpwr_gross = batt.outGenPower[idx];
				}
				else if (PVSystem->Inverter->inverterType == INVERTER_PVYIELD) //PVyield inverter model not currently enabled for multiple MPPT
				{
					sharedInverter->calculateACPower(dcPower_kW, dcVoltagePerMppt[0], wf.tdry);
					acpwr_gross = sharedInverter->powerAC_kW;
				}
				else
				{
					// inverter: runs at all hours of the day, even if no DC power.  important
<<<<<<< HEAD
					// for capturing tare losses			
					acpwr_gross = 0, aceff = 0, pntloss = 0, psoloss = 0, cliploss = 0, ac_wiringloss = 0;
					if ((inv_type == 0) || (inv_type == 1) || (inv_type == 3))
					{
						double _par, _plr;
						snlinv.acpower(dcpwr_net / num_inverters, dc_string_voltage,
							&acpwr_gross, &_par, &_plr, &aceff, &cliploss, &psoloss, &pntloss);

						acpwr_gross *= num_inverters;
						cliploss *= num_inverters;
						psoloss *= num_inverters;
						pntloss *= num_inverters;
						aceff *= 100;
					}
					else if (inv_type == 2) // partload
					{
						double _par, _plr;
						plinv.acpower(dcpwr_net / num_inverters, &acpwr_gross, &_par, &_plr, &aceff, &cliploss, &pntloss);
						acpwr_gross *= num_inverters;
						cliploss *= num_inverters;
						psoloss *= num_inverters;
						pntloss *= num_inverters;
						aceff *= 100;
					}
					else if (inv_type == 4) // ond
					{
						double _par, _plr;
						ondinv.acpower(dcpwr_net / num_inverters, dc_string_voltage, p_tdry[idx],
							&acpwr_gross, &_par, &_plr, &aceff, &cliploss, &psoloss, &pntloss, &dc_wiringloss, &ac_wiringloss);
						acpwr_gross *= num_inverters;
						cliploss *= num_inverters;
						psoloss *= num_inverters;
						pntloss *= num_inverters;
						dc_wiringloss *= num_inverters;
						ac_wiringloss *= num_inverters;
						aceff *= 100;
					}

					// if dc connected battery, update post-inverted quantities
					if (en_batt && (ac_or_dc == charge_controller::DC_CONNECTED))
					{
						if (battery_charging)
						{
							// change sign back now that is inverted
							dcpwr_net *= -1;
							acpwr_gross *= -1;
						}
						batt.update_post_inverted(*this, acpwr_gross*util::watt_to_kilowatt);
						iterate_dc = batt.check_iterate(dc_count);
						acpwr_gross = batt.outGenPower[idx] * util::kilowatt_to_watt;
					}
					dc_count++;
				} while (iterate_dc);

				if (inv_type != 4) { // losses calculated above for OND model
					ac_wiringloss = fabs(acpwr_gross) * ac_loss_percent * 0.01;
				}

				// accumulate first year annual energy
				if (iyear == 0)
				{
					if (en_batt && (ac_or_dc == charge_controller::DC_CONNECTED))
							annual_dc_power_after_battery += batt.outGenPower[idx] * ts_hour;

					annual_ac_gross += acpwr_gross * util::watt_to_kilowatt * ts_hour;
					annual_dc_loss_ond += dc_wiringloss * util::watt_to_kilowatt * ts_hour; // (TR)
					annual_ac_loss_ond += ac_wiringloss * util::watt_to_kilowatt * ts_hour; // (TR)
					
					p_inveff[idx] = (ssc_number_t)(aceff);
					p_invcliploss[idx] = (ssc_number_t)(cliploss * util::watt_to_kilowatt);
					p_invpsoloss[idx] = (ssc_number_t)(psoloss * util::watt_to_kilowatt);
					p_invpntloss[idx] = (ssc_number_t)(pntloss * util::watt_to_kilowatt);
					p_ac_wiringloss[idx] = (ssc_number_t)(ac_wiringloss * util::watt_to_kilowatt);
				}
				p_dcpwr[idx] = (ssc_number_t)((dcpwr_net - dc_wiringloss) * util::watt_to_kilowatt); // added dc_wiringloss-term to consider DC losses here (TR)
					
=======
					// for capturing tare losses
					sharedInverter->calculateACPower(dcPowerNetPerMppt_kW, dcVoltagePerMppt, wf.tdry);
					acpwr_gross = sharedInverter->powerAC_kW;
				}		
				
				ac_wiringloss = fabs(acpwr_gross) * PVSystem->acLossPercent * 0.01;
				transmissionloss = fabs(acpwr_gross) * PVSystem->transmissionLossPercent * 0.01;

				// accumulate first year annual energy
				if (iyear == 0)
				{ 
					annual_ac_gross += acpwr_gross * ts_hour;

					annual_dc_loss_ond += sharedInverter->dcWiringLoss_ond_kW * ts_hour; // (TR)
					annual_ac_loss_ond += sharedInverter->dcWiringLoss_ond_kW *  ts_hour; // (TR)

					PVSystem->p_inverterEfficiency[idx] = (ssc_number_t)(sharedInverter->efficiencyAC);
					PVSystem->p_inverterClipLoss[idx] = (ssc_number_t)(sharedInverter->powerClipLoss_kW);
					PVSystem->p_inverterPowerConsumptionLoss[idx] = (ssc_number_t)(sharedInverter->powerConsumptionLoss_kW);
					PVSystem->p_inverterNightTimeLoss[idx] = (ssc_number_t)(sharedInverter->powerNightLoss_kW);
					PVSystem->p_inverterThermalLoss[idx] = (ssc_number_t)(sharedInverter->powerTempLoss_kW);
					PVSystem->p_acWiringLoss[idx] = (ssc_number_t)(ac_wiringloss);
					PVSystem->p_transmissionLoss[idx] = (ssc_number_t)(transmissionloss);
					PVSystem->p_inverterTotalLoss[idx] = (ssc_number_t)(sharedInverter->powerLossTotal_kW);
				}
				PVSystem->p_systemDCPower[idx] = (ssc_number_t)(sharedInverter->powerDC_kW);

>>>>>>> pr/11
				//ac losses should always be subtracted, this means you can't just multiply by the derate because at nighttime it will add power
				PVSystem->p_systemACPower[idx] = (ssc_number_t)(acpwr_gross - ac_wiringloss);

				// Apply transformer loss
				ssc_number_t transformerRatingkW = static_cast<ssc_number_t>(PVSystem->ratedACOutput * util::watt_to_kilowatt);
				ssc_number_t xfmr_ll = PVSystem->transformerLoadLossFraction;
				ssc_number_t xfmr_nll = PVSystem->transformerNoLoadLossFraction * static_cast<ssc_number_t>(ts_hour * transformerRatingkW);

				if (PVSystem->transformerLoadLossFraction != 0 && transformerRatingkW != 0)
				{
					if (PVSystem->p_systemACPower[idx] < transformerRatingkW)
						xfmr_ll *= PVSystem->p_systemACPower[idx] * PVSystem->p_systemACPower[idx] / transformerRatingkW;
					else 
						xfmr_ll *= PVSystem->p_systemACPower[idx];
				} 
				// total load loss
				ssc_number_t xfmr_loss = xfmr_ll + xfmr_nll;
				PVSystem->p_systemACPower[idx] -= xfmr_loss;

				// transmission loss if AC power is produced
				if (PVSystem->p_systemACPower[idx] > 0){
					PVSystem->p_systemACPower[idx] -= (ssc_number_t)(transmissionloss);
				}

				// accumulate first year annual energy
				if (iyear == 0)
				{
					annual_xfmr_nll += PVSystem->transformerNoLoadLossFraction;
					annual_xfmr_ll += xfmr_ll;
					annual_xfmr_loss += xfmr_loss;
					PVSystem->p_transformerNoLoadLoss[idx] = PVSystem->transformerNoLoadLossFraction;
					PVSystem->p_transformerLoadLoss[idx] = xfmr_ll;
					PVSystem->p_transformerLoss[idx] = xfmr_loss;
				}

				idx++;
			}
		}

		if (iyear == 0)
		{
			int year_idx = 0;
			if (system_use_lifetime_output) {
				year_idx = 1;
			}
			// accumulate DC power after the battery
			if (en_batt && (batt_topology == ChargeController::DC_CONNECTED)) {
				annual_battery_loss = batt.outAnnualEnergyLoss[year_idx];
			}
		}
	}

	// Initialize AC connected battery predictive control
	if (en_batt && batt_topology == ChargeController::AC_CONNECTED)
		batt.initialize_automated_dispatch(util::array_to_vector<ssc_number_t>(PVSystem->p_systemACPower, nlifetime), p_load_full);

	/* *********************************************************************************************
	Post PV AC 
	*********************************************************************************************** */
	idx = 0; ireport = 0; ireplast = 0; percent_baseline = percent_complete;
	double annual_energy_pre_battery = 0.; 
	for (size_t iyear = 0; iyear < nyears; iyear++)
	{
		for (hour = 0; hour < 8760; hour++)
		{
			// report progress updates to the caller	
			ireport++;
			if (ireport - ireplast > irepfreq)
			{
				percent_complete = percent_baseline + 100.0f *(float)(hour + iyear * 8760) / (float)(insteps);
				if (!update("", percent_complete))
					throw exec_error("pvsamv1", "simulation canceled at hour " + util::to_string(hour + 1.0) + " in year " + util::to_string((int)iyear + 1) + "in post ac loop");
				ireplast = ireport;
			}

			for (size_t jj = 0; jj < step_per_hour; jj++)
			{
				if (iyear == 0)
					annual_energy_pre_battery += PVSystem->p_systemACPower[idx] * ts_hour;

				if (en_batt && batt_topology == ChargeController::AC_CONNECTED)
				{
					batt.initialize_time(iyear, hour, jj);
					batt.check_replacement_schedule();
					batt.advance(*this, PVSystem->p_systemACPower[idx], 0, p_load_full[idx]);
					PVSystem->p_systemACPower[idx] = batt.outGenPower[idx];
				}

				// accumulate system generation before curtailment and availability
				if (iyear == 0)
<<<<<<< HEAD
					annual_ac_pre_avail += p_gen[idx] * ts_hour;
=======
					annual_ac_pre_avail += PVSystem->p_systemACPower[idx] * ts_hour;
		
>>>>>>> pr/11

				//apply availability and curtailment
				PVSystem->p_systemACPower[idx] *= haf(hour);

				//apply lifetime daily AC losses only if they are enabled
				if (system_use_lifetime_output && PVSystem->enableACLifetimeLosses)
				{
					//current index of the lifetime daily AC losses is the number of years that have passed (iyear, because it is 0-indexed) * days in a year + the number of complete days that have passed
					int ac_loss_index = (int)iyear * 365 + (int)floor(hour / 24); //in units of days
					if (iyear == 0) annual_ac_lifetime_loss += PVSystem->p_systemACPower[idx] * (PVSystem->p_acLifetimeLosses[ac_loss_index] / 100) * util::watt_to_kilowatt * ts_hour; //this loss is still in percent, only keep track of it for year 0, convert from power W to energy kWh
					PVSystem->p_systemACPower[idx] *= (100 - PVSystem->p_acLifetimeLosses[ac_loss_index]) / 100;
				}
				// Update battery with final gen to compute grid power
				if (en_batt)
					batt.update_grid_power(*this, PVSystem->p_systemACPower[idx], p_load_full[idx], idx);

				if (iyear == 0)
					annual_energy += (ssc_number_t)(PVSystem->p_systemACPower[idx] * ts_hour);

				idx++;
			}
		} 

	} 
	// Check the snow models and if neccessary report a warning
	//  *This only needs to be done for subarray1 since all of the activated subarrays should 
	//   have the same number of bad values
	//  *Also accumulate monthly and annual loss values 

	if (PVSystem->enableSnowModel){
		if (Subarrays[0]->snowModel.badValues > 0){
			log(util::format("The snow model has detected %d bad snow depth values (less than 0 or greater than 610 cm). These values have been set to zero.", Subarrays[0]->snowModel.badValues), SSC_WARNING);
		}
			
		// scale by ts_hour to convert power -> energy
		accumulate_monthly_for_year( "dc_snow_loss", "monthly_snow_loss", ts_hour , step_per_hour );			
		accumulate_annual_for_year( "dc_snow_loss", "annual_snow_loss", ts_hour, step_per_hour);
	}
		 
	if (hour != 8760)
		throw exec_error("pvsamv1", "failed to simulate all 8760 hours, error in weather file ?");


	accumulate_monthly_for_year("dc_net", "monthly_dc", ts_hour, step_per_hour);
	accumulate_monthly_for_year("gen", "monthly_energy", ts_hour, step_per_hour);
		
	// scale by ts_hour to convert power -> energy
	accumulate_annual_for_year("gh", "annual_gh", ts_hour, step_per_hour);
		
	// scale by ts_hour to convert power -> energy
	double annual_poa_nom = accumulate_annual_for_year("poa_nom", "annual_poa_nom", ts_hour, step_per_hour);
	double annual_poa_beam_nom = accumulate_annual_for_year("poa_beam_nom", "annual_poa_beam_nom", ts_hour, step_per_hour);
	double annual_poa_shaded = accumulate_annual_for_year("poa_shaded", "annual_poa_shaded", ts_hour, step_per_hour);
	double annual_poa_shaded_soiled = accumulate_annual_for_year("poa_shaded_soiled", "annual_poa_shaded_soiled", ts_hour, step_per_hour);
	double annual_poa_front = accumulate_annual_for_year("poa_front", "annual_poa_front", ts_hour, step_per_hour);
	double annual_poa_rear = accumulate_annual_for_year("poa_rear", "annual_poa_rear", ts_hour, step_per_hour);
	double annual_poa_eff = accumulate_annual_for_year("poa_eff", "annual_poa_eff", ts_hour, step_per_hour);
	double annual_poa_beam_eff = accumulate_annual_for_year("poa_beam_eff", "annual_poa_beam_eff", ts_hour, step_per_hour);
		
	accumulate_monthly_for_year( "poa_nom", "monthly_poa_nom", ts_hour, step_per_hour );
	accumulate_monthly_for_year( "poa_beam_nom", "monthly_poa_beam_nom", ts_hour, step_per_hour );
	accumulate_monthly_for_year("poa_front", "monthly_poa_front", ts_hour, step_per_hour);
	accumulate_monthly_for_year( "poa_rear", "monthly_poa_rear", ts_hour, step_per_hour );
	accumulate_monthly_for_year("poa_eff", "monthly_poa_eff", ts_hour, step_per_hour);
	accumulate_monthly_for_year( "poa_beam_eff", "monthly_poa_beam_eff", ts_hour, step_per_hour );

	// scale by ts_hour to convert power -> energy
	double annual_dc_net = accumulate_annual_for_year("dc_net", "annual_dc_net", ts_hour, step_per_hour);
	accumulate_annual_for_year("gen", "annual_ac_net", ts_hour, step_per_hour);
	double annual_inv_cliploss = accumulate_annual_for_year("inv_cliploss", "annual_inv_cliploss", ts_hour, step_per_hour);
	accumulate_annual_for_year("dc_invmppt_loss", "annual_dc_invmppt_loss", ts_hour, step_per_hour);

	double annual_inv_psoloss = accumulate_annual_for_year("inv_psoloss", "annual_inv_psoloss", ts_hour, step_per_hour );
	double annual_inv_pntloss = accumulate_annual_for_year("inv_pntloss", "annual_inv_pntloss", ts_hour, step_per_hour);
	double annual_inv_tdcloss = accumulate_annual_for_year("inv_tdcloss", "annual_inv_tdcloss", ts_hour, step_per_hour);

	double nom_rad = Subarrays[0]->Module->isConcentratingPV ? annual_poa_beam_nom : annual_poa_nom;
	double inp_rad = Subarrays[0]->Module->isConcentratingPV ? annual_poa_beam_eff : annual_poa_eff;
	double ac_net = as_double("annual_ac_net");
	double mod_eff = module_eff( mod_type );

	// calculate system performance factor
	// reference: (http://files.sma.de/dl/7680/Perfratio-UEN100810.pdf)
	// additional reference: (http://www.nrel.gov/docs/fy05osti/37358.pdf)
	// PR = net_ac (kWh) / ( total input radiation (kWh) * stc efficiency (%) )
	// bug fix 6/15/15 jmf: total input radiation for PR should NOT including shading or soiling, hence use Nominal value.
	assign("performance_ratio", var_data((ssc_number_t)(ac_net / (nom_rad * mod_eff / 100.0))));

	// accumulate annual and monthly battery model outputs
	if ( en_batt ) batt.calculate_monthly_and_annual_outputs( *this );
	else assign( "average_battery_roundtrip_efficiency", var_data( 0.0f ) ); // if battery disabled, since it's shown in the metrics table

	// calculate nominal dc input
	double annual_dc_nominal = (inp_rad * mod_eff / 100.0);
	assign( "annual_dc_nominal", var_data( (ssc_number_t) annual_dc_nominal) );

	assign( "nameplate_dc_rating", var_data( (ssc_number_t)nameplate_kw ) );

	inverter_vdcmax_check();
	inverter_size_check();

	assign("annual_energy", var_data((ssc_number_t)annual_energy));


	double annual_mismatch_loss = 0, annual_diode_loss = 0, annual_wiring_loss = 0, annual_tracking_loss = 0, annual_nameplate_loss = 0, annual_dcopt_loss = 0;
	double annual_dc_gross = 0;
<<<<<<< HEAD

	//dc optimizer losses are the same for all four subarrays, assign outside of subarray loop but calculate inside loop
	double dc_opt = as_double("dcoptimizer_loss");
=======
		
>>>>>>> pr/11
	// loop over subarrays
	for (size_t nn = 0; nn < num_subarrays; nn++)
	{
		if ( Subarrays[nn]->enable )
		{
			std::string prefix = "subarray" + util::to_string(static_cast<int>(nn+1)) + "_";

			double mismatch_loss = 0,diode_loss = 0,wiring_loss = 0,tracking_loss = 0, nameplate_loss = 0, dcopt_loss = 0;
			// dc derate for each sub array
			double dc_loss = dc_gross[nn] * Subarrays[nn]->dcLossTotalPercent;
			annual_dc_gross += dc_gross[nn];
			
			if (Subarrays[nn]->dcLossTotalPercent != 0)
			{
				double total_percent = Subarrays[nn]->dcLossTotalPercent;
				mismatch_loss = Subarrays[nn]->mismatchLossPercent / total_percent * dc_loss;
				diode_loss = Subarrays[nn]->diodesLossPercent / total_percent * dc_loss;
				wiring_loss = Subarrays[nn]->dcWiringLossPercent / total_percent * dc_loss;
				tracking_loss = Subarrays[nn]->trackingLossPercent / total_percent * dc_loss;
				nameplate_loss = Subarrays[nn]->nameplateLossPercent / total_percent * dc_loss;
				dcopt_loss = Subarrays[nn]->dcOptimizerLossPercent / total_percent * dc_loss;
			}
			annual_mismatch_loss += mismatch_loss;
			annual_diode_loss += diode_loss;
			annual_wiring_loss += wiring_loss;
			annual_tracking_loss += tracking_loss;
			annual_nameplate_loss += nameplate_loss;
			annual_dcopt_loss += dcopt_loss;
			
			assign("annual_" + prefix + "dc_gross", var_data((ssc_number_t)dc_gross[nn]));
			assign("annual_" + prefix + "dc_mismatch_loss", var_data((ssc_number_t)mismatch_loss));
			assign("annual_" + prefix + "dc_diodes_loss", var_data((ssc_number_t)diode_loss));
			assign("annual_" + prefix + "dc_wiring_loss", var_data((ssc_number_t)wiring_loss));
			assign("annual_" + prefix + "dc_tracking_loss", var_data((ssc_number_t)tracking_loss));
			assign("annual_" + prefix + "dc_nameplate_loss", var_data((ssc_number_t)nameplate_loss));
		}
	}

	assign("annual_dc_gross", var_data((ssc_number_t)annual_dc_gross));
	assign("annual_ac_gross", var_data((ssc_number_t)annual_ac_gross));

	// AC/DC loss reporting OND model
	assign("annual_dc_loss_ond", var_data((ssc_number_t)annual_dc_loss_ond));
	assign("annual_ac_loss_ond", var_data((ssc_number_t)annual_ac_loss_ond));

	assign("xfmr_nll_year1", (ssc_number_t)annual_xfmr_nll);
	assign("xfmr_ll_year1", (ssc_number_t)annual_xfmr_ll);
	assign("xfmr_loss_year1", (ssc_number_t)annual_xfmr_loss);

	assign("annual_dc_mismatch_loss", var_data((ssc_number_t)annual_mismatch_loss));
	assign("annual_dc_diodes_loss", var_data((ssc_number_t)annual_diode_loss));
	assign("annual_dc_wiring_loss", var_data((ssc_number_t)annual_wiring_loss));
	assign("annual_dc_tracking_loss", var_data((ssc_number_t)annual_tracking_loss));
	assign("annual_dc_nameplate_loss", var_data((ssc_number_t)annual_nameplate_loss));
	assign("annual_dc_optimizer_loss", var_data((ssc_number_t)annual_dcopt_loss));

	// dc user input losses
	// order taken from ui - meaningless if out of order - use percentages per 9/18/14 meeting
	double sys_output = annual_dc_gross;
	sys_output -= annual_mismatch_loss;
	assign("annual_dc_after_mismatch_loss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_diode_loss;
	assign("annual_dc_after_diodes_loss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_wiring_loss;
	assign("annual_dc_after_wiring_loss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_tracking_loss;
	assign("annual_dc_after_tracking_loss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_nameplate_loss;
	assign("annual_dc_after_nameplate_loss", var_data((ssc_number_t)sys_output));

//#define WITH_CHECKS

#ifdef WITH_CHECKS
	// check that sys_output=dc_net
	if ( fabs(annual_dc_net - sys_output)/annual_dc_net > 0.00001 )
		log(util::format("Internal discrepancy in calculated output dc_gross: %lg != %lg at DC1.  Please report to SAM support.", annual_dc_net, sys_output), SSC_WARNING);
#endif

	// dc to ac losses
	sys_output -= annual_inv_cliploss;
	assign("annual_ac_after_inv_cliploss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_inv_psoloss;
	assign("annual_ac_after_inv_psoloss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_inv_pntloss;
	assign("annual_ac_after_inv_pntloss", var_data((ssc_number_t)sys_output));
	sys_output -= annual_inv_tdcloss;
	assign("annual_ac_after_inv_tdcloss", var_data((ssc_number_t)sys_output));

	double acwiring = as_double("acwiring_loss");
	double transmission = as_double("transmission_loss");
//		double transformer = as_double("transformer_loss");
	double total_percent = acwiring + transmission; // +transformer;
	double acwiring_loss = 0., transmission_loss = 0.; // , transformer_loss = 0;
	sys_output = annual_ac_gross;
	double ac_loss = sys_output*(1.0 - PVSystem->acDerate);

	if (total_percent != 0)
	{
		acwiring_loss = acwiring / total_percent * ac_loss;
		transmission_loss = transmission / total_percent * ac_loss;
//			transformer_loss = transformer / total_percent * ac_loss;
	}

	assign("annual_ac_wiring_loss", var_data((ssc_number_t)acwiring_loss));
	assign("annual_transmission_loss", var_data((ssc_number_t)transmission_loss));
//		assign("annual_ac_transformer_loss", var_data((ssc_number_t)transformer_loss));

	// ac losses
	sys_output -= acwiring_loss;
	assign("annual_ac_after_wiring_loss", var_data((ssc_number_t)sys_output));
//		sys_output -= transformer_loss;
//		assign("annual_ac_after_transformer_loss", var_data((ssc_number_t)sys_output));

	double percent = 0.;
	if (annual_poa_nom > 0) percent = 100 * (annual_poa_nom - annual_poa_shaded) / annual_poa_nom;
	assign("annual_poa_shading_loss_percent", var_data((ssc_number_t)percent));
	percent = 0.;
	if (annual_poa_shaded > 0) percent = 100 * (annual_poa_shaded - annual_poa_shaded_soiled) / annual_poa_shaded;
	assign("annual_poa_soiling_loss_percent", var_data((ssc_number_t)percent));
	percent = 0.;
	if (annual_poa_shaded > 0) percent = 100 * (annual_poa_shaded_soiled - annual_poa_front) / annual_poa_shaded_soiled;
	assign("annual_poa_cover_loss_percent", var_data((ssc_number_t)percent));
	percent = 0.;
	if (annual_poa_rear > 0) percent = 100 * (annual_poa_rear) / annual_poa_front;
	assign("annual_poa_rear_gain_percent", var_data((ssc_number_t)percent));

	// annual_dc_nominal
	percent = 0.;
	// SEV: Apply Snow loss to loss diagram 
	if (annual_dc_nominal > 0) percent = 100 * annual_snow_loss / annual_dc_nominal;
	assign("annual_dc_snow_loss_percent", var_data((ssc_number_t)percent));

	// apply clipping window loss
	if (annual_dc_nominal > 0) percent = 100 * annualMpptVoltageClipping / annual_dc_nominal;
	assign("annual_dc_mppt_clip_loss_percent", var_data((ssc_number_t)percent));

	// module loss depends on if MPPT clipping enabled.
	percent = 0.;
	if (annual_dc_nominal > 0) percent = 100 * (annual_dc_nominal - (annual_dc_gross + annual_snow_loss + annualMpptVoltageClipping)) / annual_dc_nominal;
	assign("annual_dc_module_loss_percent", var_data((ssc_number_t)percent));


	// annual_dc_gross
	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_mismatch_loss / annual_dc_gross;
	assign("annual_dc_mismatch_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_diode_loss / annual_dc_gross;
	assign("annual_dc_diodes_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_wiring_loss / annual_dc_gross;
	assign("annual_dc_wiring_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_tracking_loss / annual_dc_gross;
	assign("annual_dc_tracking_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_nameplate_loss / annual_dc_gross;
	assign("annual_dc_nameplate_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_dcopt_loss / annual_dc_gross;
	assign("annual_dc_optimizer_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_dc_adjust_loss / annual_dc_gross;
	assign("annual_dc_perf_adj_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_battery_loss / annual_dc_gross;
	assign("annual_dc_battery_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_gross > 0) percent = 100 * annual_dc_lifetime_loss / annual_dc_gross;
	assign("annual_dc_lifetime_loss_percent", var_data((ssc_number_t)percent));


	//annual_dc_net
	percent = 0.;
	if (annual_dc_net > 0) percent = 100 *annual_inv_cliploss / annual_dc_net;
	assign("annual_ac_inv_clip_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_net > 0) percent = 100 * annual_inv_psoloss / annual_dc_net;
	assign("annual_ac_inv_pso_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_net > 0) percent = 100 * annual_inv_pntloss / annual_dc_net;
	assign("annual_ac_inv_pnt_loss_percent", var_data((ssc_number_t)percent));

	percent = 0.;
	if (annual_dc_net > 0) percent = 100 * annual_inv_tdcloss / annual_dc_net;
	assign("annual_ac_inv_tdc_loss_percent", var_data((ssc_number_t)percent));

	sys_output = annual_dc_net;
	sys_output -= (annual_inv_cliploss + annual_inv_pntloss + annual_inv_psoloss);
	percent = 0.;
	if (sys_output > 0) percent = 100 * (sys_output - annual_ac_gross) / sys_output;
	assign("annual_ac_inv_eff_loss_percent", var_data((ssc_number_t)percent));


	// annual_ac_gross
	sys_output *= (1.0 - percent / 100.0);

#ifdef WITH_CHECKS
	// check that ac_gross = sys_output at this point
	if (fabs(annual_ac_gross - sys_output)/ annual_ac_gross > 0.00001)
		log(util::format("Internal discrepancy in calculated output ac_gross: %lg != %lg at AC1.  Please report to SAM support.", annual_ac_gross, sys_output), SSC_WARNING);
#endif

	percent = 0.;
	annual_ac_battery_loss = (annual_energy_pre_battery - annual_ac_pre_avail);
	if (annual_ac_gross > 0) percent = 100.0 * annual_ac_battery_loss / annual_ac_gross;
	assign("annual_ac_battery_loss_percent", var_data((ssc_number_t)percent));
	sys_output -= annual_ac_battery_loss;

	percent = 0.;
	if (annual_ac_gross > 0) percent = 100.0 * acwiring_loss / annual_ac_gross;
	assign("annual_ac_wiring_loss_percent", var_data((ssc_number_t)percent));
	sys_output -= acwiring_loss;

	percent = 0.;
	if (annual_ac_gross > 0) percent = 100.0 * transmission_loss / annual_ac_gross;
	assign("annual_transmission_loss_percent", var_data((ssc_number_t)percent));
	sys_output -= transmission_loss;
//		percent = 0;
//		if (annual_ac_gross > 0) percent = 100.0 * transformer_loss / annual_ac_gross;
//		assign("annual_ac_transformer_loss_percent", var_data((ssc_number_t)percent));
//		sys_output -= transformer_loss;
	// annual_ac_pre_avail

	percent = 0.;
	if (annual_ac_gross > 0) percent = 100 * annual_ac_lifetime_loss / annual_ac_gross;
	assign("annual_ac_lifetime_loss_percent", var_data((ssc_number_t)percent));
	sys_output -= annual_ac_lifetime_loss;

	percent = 0.;
	if (annual_xfmr_loss > 0) percent = 100 * annual_xfmr_loss / annual_ac_gross;
	assign("annual_xfmr_loss_percent", var_data((ssc_number_t)percent));
	sys_output -= annual_ac_lifetime_loss;


#ifdef WITH_CHECKS
	// check that ac_net = sys_output at this point
	if (fabs(annual_ac_pre_avail - sys_output)/ annual_ac_pre_avail > 0.00001)
		log(util::format("Internal discrepancy in calculated output ac_net: %lg != %lg at AC2.  Please report to SAM support.", annual_ac_pre_avail, sys_output), SSC_WARNING);
#endif



	percent = 0.;
	if (annual_ac_pre_avail > 0) percent = 100.0 * (annual_ac_pre_avail - annual_energy) / annual_ac_pre_avail;
	assign("annual_ac_perf_adj_loss_percent", var_data((ssc_number_t)percent));
	sys_output *= (1.0 - percent / 100.0);


	// annual_ac_net = system_output

#ifdef WITH_CHECKS
	// check that ac_net = sys_output at this point
	if (fabs(annual_ac_net - sys_output) / annual_ac_net > 0.00001)
		log(util::format("Internal discrepancy in calculated output ac_net: %lg != %lg at AC3.  Please report to SAM support.", annual_ac_net, sys_output), SSC_WARNING);
#endif


	// DC Capacity Factor
	double kWhACperkWDC = 0.0;
	double nameplate_dc = as_double("system_capacity");
	if (nameplate_dc > 0) {
		kWhACperkWDC = annual_energy / nameplate_dc;
	}
	assign("capacity_factor", var_data((ssc_number_t)(kWhACperkWDC / 87.6)));
	assign("kwh_per_kw", var_data((ssc_number_t)kWhACperkWDC));

	// AC Capacity Factor
	double kWhACperkWAC = 0.0;
	double nameplate_ac = sharedInverter->getACNameplateCapacity();
	if (nameplate_ac > 0) {
		kWhACperkWAC = annual_energy / nameplate_ac;
	}
	assign("capacity_factor_ac", var_data((ssc_number_t)(kWhACperkWAC / 87.6)));

	if (is_assigned("load"))
	{
		p_load_in = as_vector_ssc_number_t("load");
		nload = p_load_in.size();
	}

	Irradiance->AssignOutputs(this);
	Subarrays[0]->AssignOutputs(this);
	PVSystem->AssignOutputs(this);
	
//	_CrtDumpMemoryLeaks();
//	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);

}
	
double cm_pvsamv1::module_eff(int mod_type)
{
	double eff = -1;
	
	switch (mod_type)
	{
	case 0: // SPE
		eff = as_double( util::format("spe_eff%d", as_integer("spe_reference")) );
		break;
	case 1: // CEC
		{
			double a_c = as_double("cec_area");
			double i_noct = 1000; // as_double("cec_i_noct");
			double v_mp_ref = as_double("cec_v_mp_ref");
			double i_mp_ref = as_double("cec_i_mp_ref");

			if (a_c == 0) a_c = -1;
		//	if (i_noct == 0) i_noct = 1000.0;

			eff = 100.0 * (( v_mp_ref * i_mp_ref ) / a_c) / i_noct;
		}
		break;
	case 2: // 6par user entered
		{
			double area = as_double("6par_area");
			double vmp = as_double("6par_vmp");
			double imp = as_double("6par_imp");
			if (area == 0) area = 1;
			eff = 100.0 * ((vmp*imp)/area)/1000.0;
		}
		break;
	case 3: // Sandia
		{
			double area = as_double("snl_area");
			double vmpo = as_double("snl_vmpo");
			double impo = as_double("snl_impo");

			eff = vmpo*impo;
			if (area > 0)
				eff = eff/area;
			eff = eff / 1000.0;
			eff = eff * 100;
		}
		break;
	case 4: // IEC 61853
		{
			double area = as_double("sd11par_area");
			double vmp = as_double("sd11par_Vmp0");
			double imp = as_double("sd11par_Imp0");
			if (area == 0) area = 1;
			eff = 100.0 * ((vmp*imp)/area)/1000.0;
		}
		break;
	case 5: // ML model
		{
			double Width = as_double("mlm_Width");
			double Length = as_double("mlm_Length");
			double vmp = as_double("mlm_V_mp_ref");
			double imp = as_double("mlm_I_mp_ref");
			double Sref = as_double("mlm_S_ref");
			eff = 100.0 * ((vmp*imp) / (Width*Length)) / Sref;
		}
	}

	if (eff == 0.0) eff = -1;
	return eff;
}

void cm_pvsamv1::inverter_vdcmax_check()
{
	// check that no hourly vmp values exceed Vdcmax
	// add max value and number of times > Vdcmax
	int numVmpGTVdcmax = 0;
	double maxVmp=0;
	int maxVmpHour=0;
	int invType = as_integer("inverter_model");
	double vdcmax;
	switch (invType)
	{
		case 0: // cec
			vdcmax = as_double("inv_snl_vdcmax");
			break;
		case 1: // datasheet
			vdcmax = as_double("inv_ds_vdcmax");
			break;
		case 2: // partload curve
			vdcmax = as_double("inv_pd_vdcmax");
			break;
		case 3: // coefficient generator
			vdcmax = as_double("inv_cec_cg_vdcmax");
			break;
<<<<<<< HEAD
		case 4:
=======
		case 4: // ondInverter (PVYield)
>>>>>>> pr/11
			vdcmax = as_double("ond_VAbsMax");
			break;
		default:
			// message
			return;
	}

	// warning on inverter page
	if (vdcmax <=0) return;

	size_t count;
	ssc_number_t *da = as_array("inverterMPPT1_DCVoltage", &count);
	if (count == 8760)
	{
		for (size_t i=0; i < count;i++)
		{
			if (da[i] > vdcmax)
			{
				numVmpGTVdcmax++;
				if (da[i] > maxVmp) 
				{
					maxVmp = da[i];
					maxVmpHour = (int)i;
				}
			}
		}
	}

	if (numVmpGTVdcmax > 0) 
	{
		log( util::format( "Module array voltage Vmp exceeds the Vdcmax (%.2lfV) of inverter %d times.\n"
				"   The maximum Vmp value is %.2lfV at hour %d.\n"
				"   It is recommended that you reduce the number of modules per string.", vdcmax, numVmpGTVdcmax, maxVmp, maxVmpHour ),
				SSC_WARNING );
	}
}

void cm_pvsamv1::inverter_size_check()
{
	// undersized - check that no hourly output exceeds the rated output of the inverter
	// 9/26/10 note that e_net automatically clipped - must look at derated dc power
	// oversized - add max output > 75% of inverter ourput
	ssc_number_t *acPower;
	size_t acCount;
	ssc_number_t *dcPower;
	size_t dcCount;
	int numHoursClipped = 0;
	double maxACOutput=0;
	int invType = as_integer("inverter_model");
	int numInv = as_integer("inverter_count");

	double ratedACOutput = 0;
	double ratedDCOutput = 0;
	switch (invType)
	{
		case 0: // cec
			ratedACOutput = as_double("inv_snl_paco");
			ratedDCOutput = as_double("inv_snl_pdco");
			break;
		case 1: // datasheet
			ratedACOutput = as_double("inv_ds_paco");
			ratedDCOutput = as_double("inv_ds_eff")/100.0;
			if (ratedDCOutput != 0) ratedDCOutput = ratedACOutput/ratedDCOutput;
			break;
		case 2: // partload curve
			ratedACOutput = as_double("inv_pd_paco");
			ratedDCOutput = as_double("inv_pd_pdco");
			break;
		case 3: // coefficient generator
			ratedACOutput = as_double("inv_cec_cg_paco");
			ratedDCOutput = as_double("inv_cec_cg_pdco");
			break;
		case 4: // ond inverter (PVYield) 
			ratedACOutput = as_double("ond_PMaxOUT");
			ratedDCOutput = as_double("ond_PMaxDC");
			break;
		default:
			// message
			return;
	}
	ratedACOutput *= numInv;
	ratedDCOutput *= numInv;

	if ((ratedACOutput <= 0) || (ratedDCOutput <= 0)) return;

	ratedACOutput = ratedACOutput * util::watt_to_kilowatt; // W to kW to compare to hourly output
	ratedDCOutput = ratedDCOutput * util::watt_to_kilowatt; // W to kW to compare to hourly output

	acPower = as_array("gen", &acCount);
	dcPower = as_array("dc_net", &dcCount);
	if ((acCount == 8760) && (dcCount == 8760))
	{
		for (size_t i=0; i < acCount;i++)
		{
			if (dcPower[i] > ratedDCOutput) numHoursClipped++;
			if (acPower[i] > maxACOutput) maxACOutput = acPower[i]; 
		}
	}
	if (numHoursClipped >= 2190) //more than one quarter of the entire year (8760) is clipped
		log( util::format("Inverter undersized: The array output exceeded the inverter rating %.2lf kWdc for %d hours.", 
			ratedDCOutput, numHoursClipped), 
			SSC_WARNING );

	if ((maxACOutput < 0.75 * ratedACOutput) && (maxACOutput > 0))
		log( util::format("Inverter oversized: The maximum inverter output was %.2lf%% of the rated value %lg kWac.", 
			100 * maxACOutput / ratedACOutput, ratedACOutput), 
			SSC_WARNING);
}

DEFINE_MODULE_ENTRY( pvsamv1, "Photovoltaic performance model, SAM component models V.1", 1 )
