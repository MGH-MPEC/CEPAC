#pragma once

/**
	\brief include.h is the main include file for all CEPAC simulation classes
*/

// avoid deprecated warnings for many string functions in VC++ 2005
#pragma warning(disable:4996)

/** include all the necessary standard C and C++ libraries */
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <float.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <vector>
#include <map>
#include <algorithm>
#include <list>
using namespace std;

/** Predefine classes that have circular dependencies */
class SimContext;
class Tracer;
class RunStats;
class CostStats;
class SummaryStats;
class Patient;

/** Include all the class definitions */
#include "SimContext.h"
#include "Tracer.h"
#include "RunStats.h"
#include "CostStats.h"
#include "SummaryStats.h"
#include "StateUpdater.h"
#include "BeginMonthUpdater.h"
#include "HIVInfectionUpdater.h"
#include "HIVTestingUpdater.h"
#include "BehaviorUpdater.h"
#include "DrugEfficacyUpdater.h"
#include "CD4TestUpdater.h"
#include "HVLTestUpdater.h"
#include "ClinicVisitUpdater.h"
#include "DrugToxicityUpdater.h"
#include "MortalityUpdater.h"
#include "CHRMsUpdater.h"
#include "TBDiseaseUpdater.h"
#include "TBClinicalUpdater.h"
#include "AcuteOIUpdater.h"
#include "CD4HVLUpdater.h"
#include "EndMonthUpdater.h"
#include "Patient.h"
#include "mtrand.h"
#include "CepacUtil.h"


/** Include platform specific header files */
#if defined(_LINUX)
	#include <sys/io.h>
#endif
#if defined(_WIN32)
	#include <io.h>
	#include <direct.h>	//for _mkdir and _CHDIR
#else
	#include <sys/types.h>
	#include <sys/stat.h>
	//used to find files in a directory
	#include <glob.h>
#endif

