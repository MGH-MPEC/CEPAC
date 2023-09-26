#include "include.h"

/* Constructor takes run name as parameter */
SimContext::SimContext(string runName) {

	counter=0;
	inputFileName = runName;
	inputFileName.append(".in");
	runSpecsInputs.runName = runName;
}

/* Destructor cleans up the allocated memory for art and proph inputs */
SimContext::~SimContext(void) {
	for (int i = 0; i < ART_NUM_LINES; i++) {
		delete artInputs[i];
		delete pedsARTInputs[i];
		delete adolescentARTInputs[i];
	}
	for (int i = 0; i < PROPH_NUM_TYPES; i++) {
		for (int j = 0; j < OI_NUM; j++) {
			for (int k = 0; k < PROPH_NUM; k++) {
				delete prophsInputs[i][j][k];
				delete pedsProphsInputs[i][j][k];
			}
		}
	}
}

/* Initialize the number of patients to trace*/
int SimContext::numPatientsToTrace;

/* Initialize the constant character strings */
const char *SimContext::CD4_STRATA_STRS[] = {
	"CD4vlo", "CD4_lo", "CD4mlo", "CD4mhi", "CD4_hi", "CD4vhi", "CD4unk"
};
const char *SimContext::HVL_STRATA_STRS[] = {
	"HVLvlo", "HVL_lo", "HVLmlo", "HVLmed", "HVLmhi", "HVL_hi", "HVLvhi", "HVLunk"
};
const double SimContext::HVL_STRATA_MIDPTS[] = {
	10.0, 250.0, 1750.0, 6500.0, 20000.0, 65000.0, 550000.0
};
const char *SimContext::GENDER_STRS[] = {
	"male", "female"
};
const char *SimContext::TRANSM_RISK_STRS[] = {
	"MSM", "IDU", "Other"
};
char SimContext::OI_STRS[SimContext::OI_NUM][32];	// OI_STRS is set during readRunSpecsInputs
char SimContext::DTH_CAUSES_STRS[SimContext::DTH_NUM_CAUSES][32]; 	// DTH_CAUSES_STRS is set during readRunSpecsInputs, readCHRMsInputs, and readCohortInputs
const char *SimContext::HIST_OI_CATS_STRS[] = {
	"NoOIHist", "MildOIHist", "SevrOIHist"
};
char SimContext::CHRM_STRS[SimContext::CHRM_NUM][32];	// CHRM_STRS is set during readCHRMsInputs

char SimContext::RISK_FACT_STRS[SimContext::RISK_FACT_NUM][32];	// RISK_FACT_STRS is set during readCohortInputs

const char *SimContext::CHRM_AGE_CAT_STRS[]={
		"CHRMs Age Bucket 1","CHRMs Age Bucket 2","CHRMs Age Bucket 3","CHRMs Age Bucket 4","CHRMs Age Bucket 5","CHRMs Age Bucket 6","CHRMs Age Bucket 7"
};

const char *SimContext::OUTPUT_AGE_CAT_STRS[]={
		"<20","20-24","25-29","30-34","35-39","40-44","45-49","50-54","55-59","60-64","65-69","70-74","75-79",">=80"
};

const char *SimContext::LINKAGE_STATS_AGE_CAT_STRS[]={
		"<15","15-19","20-24","25-29","30-34","35-39","40-44","45-49",">50"
};

const char *SimContext::COST_CD4_STRATA_STRS[] = {
	"CD4vlo", "CD4_lo", "CD4mlo", "CD4mhi", "CD4_hi", "CD4vhi", "CD4Unknown", "CD4All"
};

const char *SimContext::COST_SUBGROUPS_STRS[] = {
	"All Patients", "HIV-", "HIV+", "HIV+ Pre Linkage", "HIV+ Pre ART In Care", "On ART", "LTFU After ART", "LTFU Never had ART", "RTC", "On ART Never Lost", "On ART first 6 mths", "On 1st Line ART", "On 2nd Line or Higher ART"
};

const char *SimContext::CLINIC_VISITS_STRS[] = {
	"initial", "acute", "sched"
};

const char *SimContext::EMERGENCY_TYPE_STRS[] = {
	"Acute OI", "Testing", "ART Policies", "OI Proph Policies", "Routine"
};

const char *SimContext::ART_EFF_STRS[] = {
	"suppressed", "failure"
};
const char *SimContext::CD4_RESPONSE_STRS[] = {
	"Type_1", "Type_2", "Type_3", "Type_4"
};
const char *SimContext::RESP_TYPE_STRS[] = {
	"Full Responder", "Partial Responder", "Non Responder"
};
const char *SimContext::HET_OUTCOME_STRS[]={
	"Suppression", "Late Failure", "ART Effect OI","ART Effect CHRMs", "ART Effect Mortality",
	"Resistance", "Toxicity", "Cost", "Restart Regimen", "Resuppression"
};
const char *SimContext::ART_TOX_SEVERITY_STRS[] = {
	"Min","Chr","Maj","Dth"
};
const char *SimContext::ART_FAIL_TYPE_STRS[] = {
	"Virologic", "Immunologic", "Clinical", "Not Failed"
};
const char *SimContext::ART_STOP_TYPE_STRS[] = {
	"Max Months on ART", "With Major Toxicity","With Chronic Toxicity", "On Observed Failure", "Fail and CD4", "Fail and Severe OI",
	"Fail and Max Months", "LTFU", "Not Stopped", "STI"
};
const char *SimContext::PROPH_TYPE_STRS[] = {
	"PRIMARY", "SECONDARY"
};
const char *SimContext::COST_TYPES_STRS[] = {
		"Direct Medical", "Direct Nonmedical", "Time", "Indirect"
};
const char *SimContext::TB_STRAIN_STRS[] = {
	"dsTB", "mdrTB", "xdrTB"
};
const char *SimContext::TB_STATE_STRS[] = {
	"Uninfected", "Latent", "Active Pulmonary", "Active Extrapulmonary","Previously Treated", "Treatment Default"
};
const char *SimContext::TB_TRACKER_STRS[] = {
	"Sputum Bacillary Load Hi", "Immune Reactive", "TB Symptoms"
};

const char *SimContext::TB_DIAG_STATUS_STRS[] = {
	"Pos", "Neg"
};

const char *SimContext::TB_UNFAVORABLE_STRS[] = {
	"Failure", "Relapse", "LTFU", "Death"
};

const char *SimContext::HIV_ID_STRS[] = {
	"HIVneg", "unidentHIV+", "identHIV+"
};
const char *SimContext::HIV_POS_STRS[] = {
	"HIVasym", "HIVsymp", "HIVacut"
};
const char *SimContext::HIV_EXT_INF_STRS[] =  {
	"HIVneg_hiRisk", "HIVasym", "HIVsymp", "HIVacut", "HIVneg_loRisk"
};
const char *SimContext::HIV_DET_STRS[] = {
	"initial", "HIVscreening", "HIVscreeningPrevDetected","HIVbackground", "HIVbackgroundPrevDetected", "presentingOI", "presentingOIPrevDetected", "linkingToTBCare", "linkingToTBCarePrevDetected", "unidentified"
};
const char *SimContext::HIV_CARE_STRS[] = {
	"HIV-", "HIV+undetected", "HIV+unlinked(detected not in care)","HIV+in_care","HIV+LTFU","HIV+RTC"
};
const char *SimContext::TEST_RESULT_STRS[] = {
	"truePos", "falsPos", "trueNeg", "falsNeg"
};
const char *SimContext::PEDS_HIV_STATE_STRS[] = {
	"HIVposIU", "HIVposIP", "HIVposPP", "HIVneg"
};
const char *SimContext::PEDS_MATERNAL_STATUS_STRS[] = {
		"negative", "HIVchr CD4 high", "HIVchr CD4 low", "HIVacut"
};
const char *SimContext::EID_TEST_TYPE_STRS[] = {
		"Base Test", "First Confirmatory", "Second Confirmatory"
};

const char *SimContext::PEDS_BF_TYPE_STRS[] = {
	"Exclusive Feeding", "Mixed Feeding", "Complementary Feeding", "Replacement Feeding"
};
const char *SimContext::PEDS_AGE_CAT_STRS[] = {
	"0-2mth", "3-5mth", "6-8mth", "9-11mth", "12-14mth", "15-17mth", "18-23mth", "2yr", "3yr", "4yr", ">4yr"
};
const char *SimContext::PEDS_CD4_PERC_STRS[] = {
	"0-5perc", "5-10perc", "10-15perc", "15-20perc", "20-25perc", "25-30perc", "30-35perc", ">35perc"
};

/* readInputs function reads in all the inputs from the given input file,
	throws exception if there is an error */
void SimContext::readInputs() {
	/* Open the input file for reading */
	CepacUtil::changeDirectoryToInputs();
	inputFile = CepacUtil::openFile(inputFileName.c_str(), "r");
	if (inputFile == NULL) {
		string errorString = "   ERROR - Could not open input file ";
		errorString.append(inputFileName);
		throw errorString;
	}

	/* Read all the input data from the file */
	readRunSpecsInputs();
	readOutputInputs();
	readCohortInputs();
	readLTFUInputs();
	readHeterogeneityInputs();
	readHIVTestInputs();
	readNatHistInputs();
	readCHRMsInputs();
	readQOLInputs();
	readCostInputs();
	readTreatmentInputsPart1();
	readARTInputs();
	readTreatmentInputsPart2();
	readProphInputs();
	readSTIInputs();
	readTBInputs();
	readPedsInputs();
	readPedsProphInputs();
	readPedsARTInputs();
	readPedsCostInputs();
	readEIDInputs();
	readAdolescentInputs();
	readAdolescentARTInputs();

	/* Close the input file */
	CepacUtil::closeFile(inputFile);
} /* end readInputs */

/* readRunSpecsInputs reads data from the RunSpecs tab of the input sheet */
void SimContext::readRunSpecsInputs() {
	char buffer[256];
	int i, tempBool;
	// read in name of set this run belongs to
	readAndSkipPast( "Runset", inputFile );
	fscanf( inputFile, "%299s", buffer );
	runSpecsInputs.runSetName = buffer;
	// read in cohort size
	readAndSkipPast( "CohortSize", inputFile );
	fscanf( inputFile, "%ld", &runSpecsInputs.numCohorts );
	// read in discount rate, convert to monthly rate from yearly
	readAndSkipPast( "DiscFactor", inputFile );
	fscanf( inputFile, "%lf", &runSpecsInputs.originalDiscRate );
	runSpecsInputs.discountFactor = pow(1.0 + runSpecsInputs.originalDiscRate, 1.0 / 12.0);
	// read in max actual CD4 count for patient
	readAndSkipPast( "MaxPatCD4", inputFile );
	fscanf( inputFile, "%lf", &runSpecsInputs.maxPatientCD4 );

	// read in mth times A - C to rec ART eff
	readAndSkipPast( "MthRecARTEffA", inputFile );
	fscanf( inputFile, "%d", &runSpecsInputs.monthRecordARTEfficacy[0] );
	readAndSkipPast( "MthRecARTEffB", inputFile );
	fscanf( inputFile, "%d", &runSpecsInputs.monthRecordARTEfficacy[1] );
	readAndSkipPast( "MthRecARTEffC", inputFile );
	fscanf( inputFile, "%d", &runSpecsInputs.monthRecordARTEfficacy[2] );
	// read in whether to use time to init rand seeds
	readAndSkipPast( "RandSeedByTime", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	runSpecsInputs.randomSeedByTime = (bool) tempBool;
	// read in user locale
	readAndSkipPast( "UserLocale", inputFile );
	fscanf( inputFile, " %32s", buffer );
	runSpecsInputs.userProgramLocale = buffer;

	// read in input's internal version
	readAndSkipPast( "InpVer", inputFile );
	fscanf( inputFile, " %20s", buffer );
	// Verify input version, throw error if it does not match up
	if (strcmp(buffer, CepacUtil::CEPAC_INPUT_VERSION) != 0) {
		string errorString = "   ERROR - Input file version incompatible with the CEPAC executable";
		throw errorString;
	}
	runSpecsInputs.inputVersion = buffer;
	// read in the model version
	readAndSkipPast( "ModelVer", inputFile );
	fscanf( inputFile, " %20s", buffer );
	runSpecsInputs.modelVersion = buffer;

	// read in whether to model TB as an OI when the TB tab is disabled; this allows the program to apply the TB-specific OI death rate ratio
	readAndSkipPast( "IncludeTB_AsOI", inputFile);
	fscanf( inputFile,"%d", &tempBool );
	runSpecsInputs.OIsIncludeTB = (bool) tempBool;
	// read in user defined OI names, and set causes of death names
	readAndSkipPast( "OIstrs", inputFile );
	for ( i = 0; i < OI_NUM; ++i ) {
		fscanf( inputFile, " %32s", OI_STRS[i] );
		strcpy(DTH_CAUSES_STRS[i], OI_STRS[i]);
	}
	strcpy(DTH_CAUSES_STRS[DTH_HIV], "HIV");
	strcpy(DTH_CAUSES_STRS[DTH_BKGD_MORT], "backgroundMort");
	strcpy(DTH_CAUSES_STRS[DTH_ACTIVE_TB], "TB(Active)");
	strcpy(DTH_CAUSES_STRS[DTH_TOX_ART], "toxART");
	strcpy(DTH_CAUSES_STRS[DTH_TOX_PROPH], "toxProph");
	strcpy(DTH_CAUSES_STRS[DTH_TOX_TB_PROPH], "toxTBProph");
	strcpy(DTH_CAUSES_STRS[DTH_TOX_TB_TREATM], "toxTBTreatment");
	strcpy(DTH_CAUSES_STRS[DTH_TOX_INFANT_HIV_PROPH], "toxInfantHIVProph");

	// read in whether to output monthly cohort summaries to file
	readAndSkipPast( "LongitLogCohort", inputFile );
	fscanf( inputFile, "%d", &runSpecsInputs.longitLoggingLevel );
	// read in OIs considered as first OIs in the log
	readAndSkipPast( "LongitLogFirstOIs", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(runSpecsInputs.firstOIsLongitLogging[i]) );

	// read in whether to output CD4 distribution of OI histories
	readAndSkipPast( "LogPriorOIHistProb", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	runSpecsInputs.enableOIHistoryLogging = (bool) tempBool;
	// read in the number of ART failures pat has to log OI hists
	readAndSkipPast( "LogOIHistwithARTfails", inputFile );
	fscanf( inputFile, "%d", &runSpecsInputs.numARTFailuresForOIHistoryLogging );
	// read in CD4 bounds to constrain when pat mths are included in OI hist logging
	readAndSkipPast( "LogOIHistwithCD4", inputFile );
	fscanf( inputFile, "%lf %lf", &(runSpecsInputs.CD4BoundsForOIHistoryLogging[LOWER_BOUND]), &(runSpecsInputs.CD4BoundsForOIHistoryLogging[UPPER_BOUND]) );
	// read in HVL bnds to constrain when pat mths included in OI hist logging
	readAndSkipPast( "LogOIHistwithHVL", inputFile );
	fscanf( inputFile, "%d %d", &(runSpecsInputs.HVLBoundsForOIHistoryLogging[LOWER_BOUND]), &(runSpecsInputs.HVLBoundsForOIHistoryLogging[UPPER_BOUND]) );
	// read in whether to excl pat mths from OI hist log if there's OI hist of given OI
	readAndSkipPast( "LogOIHistExcludeOITypes", inputFile );
	for ( i = 0; i < OI_NUM; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		runSpecsInputs.OIsToExcludeOIHistoryLogging[i] = (bool) tempBool;
	}
	// read in OI Fraction of Benefit
	readAndSkipPast( "FOB_OIs", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(runSpecsInputs.OIsFractionOfBenefit[i]) );
	// read in severe OI classification
	readAndSkipPast( "Severe_OIs", inputFile );
	for ( i = 0; i < OI_NUM; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		runSpecsInputs.severeOIs[i] = (bool) tempBool;
	}
	// read in CD4 count boundaries for strata
	readAndSkipPast( "CD4Bounds", inputFile );
	for ( i = CD4__HI; i >= CD4_VLO; i-- )
		fscanf( inputFile, "%lf", &(runSpecsInputs.CD4StrataUpperBounds[i]) );

	// read in whether to enable Multiple disocunt rates
	readAndSkipPast( "EnableMultDiscountOutput", inputFile );
	fscanf( inputFile, "%d", &tempBool );
	runSpecsInputs.enableMultipleDiscountRates = (bool) tempBool;

	// read in multiple discount rate, convert to monthly rate from yearly
	readAndSkipPast( "DiscountRatesCost", inputFile );
	for (i = 0; i < NUM_DISCOUNT_RATES; i++){
		double annualDiscountRate;
		fscanf( inputFile, "%lf", &annualDiscountRate);
		runSpecsInputs.multDiscountRatesCost[i] = pow(1.0+annualDiscountRate, 1.0/12.0);
	}

	// read in multiple discount rate, convert to monthly rate from yearly
	readAndSkipPast( "DiscountRatesBenefit", inputFile );
	for (i = 0; i < NUM_DISCOUNT_RATES; i++){
		double annualDiscountRate;
		fscanf( inputFile, "%lf", &annualDiscountRate);
		runSpecsInputs.multDiscountRatesBenefit[i] = pow(1.0+annualDiscountRate, 1.0/12.0);
	}


} /* end readRunSpecsInputs */

/* readOutputInputs reads data from the Output tab of the input sheet */
void SimContext::readOutputInputs() {
	int i, tempBool;

	readAndSkipPast("NumPatientsToTrace", inputFile);
	fscanf(inputFile, "%d", &(outputInputs.traceNumSelection));
	numPatientsToTrace = min(outputInputs.traceNumSelection, MAX_NUM_TRACES);
	//read in sub cohort parameters
	readAndSkipPast("EnableSubCohorts", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	outputInputs.enableSubCohorts = (bool) tempBool;
	readAndSkipPast("SubCohortValues", inputFile);
	for (i = 0; i < MAX_NUM_SUBCOHORTS; i++)
		fscanf(inputFile, "%ld", &(outputInputs.subCohorts[i]));

	//read in cost output parameters
	readAndSkipPast("EnableDetailedCosts", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	outputInputs.enableDetailedCostOutputs = (bool) tempBool;
}

/* readCohortInputs reads data from the Cohort tab of the input sheet */
void SimContext::readCohortInputs() {
	int i, j, k, tempInt, tempBool;
	double dTemp;
	char scratch[256];

	// read in popul initial CD4 distrib
	readAndSkipPast( "InitCD4", inputFile );
	fscanf(inputFile,"%lf %lf", &(cohortInputs.initialCD4Mean), &(cohortInputs.initialCD4StdDev));
	readAndSkipPast("UseSqRtTransform", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.enableSquareRootTransform = (bool) tempBool;

	// read in popul initial HVL distrib
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j ) {
		readAndSkipPast( "InitHVL", inputFile );
		readAndSkipPast( CD4_STRATA_STRS[j], inputFile );
		for ( i = HVL_NUM_STRATA - 1; i >= HVL_VLO; --i )
			fscanf( inputFile, "%lf", &(cohortInputs.initialHVLDistribution[j][i]) );
	}
	// read in popul initial age (mths) distrib
	readAndSkipPast( "InitAge", inputFile );
	fscanf(inputFile,"%lf %lf", &cohortInputs.initialAgeMean, &cohortInputs.initialAgeStdDev);

    // read in custom age dist bool
	readAndSkipPast( "InitAgeCustomDist", inputFile );
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.useCustomAgeDist = (bool) tempBool;

    // store age dist strata mins/maxes in same array!
    readAndSkipPast( "AgeStratMins", inputFile );
    for (i = 0; i < INIT_AGE_NUM_STRATA; ++i) {
        fscanf(inputFile, "%lf", &cohortInputs.ageStrata[i] );
    }
    readAndSkipPast( "AgeStratMaxes", inputFile );
    for (i = 0; i < INIT_AGE_NUM_STRATA; ++i) {
        fscanf(inputFile, "%lf", &cohortInputs.ageStrata[INIT_AGE_NUM_STRATA + i] );
    }

    // read in age strata probabilites as CDF
    readAndSkipPast( "AgeStratProbs", inputFile );
    for (i = 0; i < INIT_AGE_NUM_STRATA; ++i) {
        fscanf(inputFile, "%lf", &cohortInputs.ageProbs[i]);
        if (i > 0)
            cohortInputs.ageProbs[i] += cohortInputs.ageProbs[i - 1];
    }

	// read in male percentage of cohort
	readAndSkipPast( "InitGender", inputFile );
	fscanf( inputFile, "%lf", &cohortInputs.maleGenderDistribution );
	// read in OI proph noncompliance prob and degree
	readAndSkipPast( "ProphNonCompliance", inputFile );
	fscanf( inputFile, "%lf %lf", &cohortInputs.OIProphNonComplianceRisk, &cohortInputs.OIProphNonComplianceDegree );

	// read in distribution of clinic visit patient types
	readAndSkipPast( "PatClinicTypes", inputFile );
	dTemp = 1.0;
	for ( i = 0; i < CLINIC_VISITS_NUM - 1; ++i ) {
		fscanf( inputFile, "%lf", &(cohortInputs.clinicVisitTypeDistribution[i]) );
		dTemp -= cohortInputs.clinicVisitTypeDistribution[i];
	}
	cohortInputs.clinicVisitTypeDistribution[CLINIC_VISITS_NUM - 1] = dTemp;
	// read in distribution of proph and ART implement patient types
	readAndSkipPast( "PatTreatmentTypes", inputFile );
	dTemp = 1.0;
	for ( i = 0; i < THERAPY_IMPL_NUM - 1; ++i ) {
		fscanf( inputFile, "%lf", &(cohortInputs.therapyImplementationDistribution[i]) );
		dTemp -= cohortInputs.therapyImplementationDistribution[i];
	}
	cohortInputs.therapyImplementationDistribution[THERAPY_IMPL_NUM - 1] = dTemp;
	// read in distribution of CD4 response types on ART
	readAndSkipPast("PatCD4ResponeTypeOnART", inputFile);
	dTemp = 1.0;
	for (i = 0; i < CD4_RESPONSE_NUM_TYPES - 1; i++) {
		fscanf(inputFile, "%lf", &(cohortInputs.CD4ResponseTypeOnARTDistribution[i]));
		dTemp -= cohortInputs.CD4ResponseTypeOnARTDistribution[i];
	}
	cohortInputs.CD4ResponseTypeOnARTDistribution[CD4_RESPONSE_NUM_TYPES - 1] = dTemp;

	// read in popul prob of prev OI histories
	readAndSkipPast( "PriorOIHistAtEntry", inputFile );
	for ( i = 0; i < OI_NUM; ++i ) {
		readAndSkipPast( OI_STRS[i], inputFile );
		for ( k = HVL_NUM_STRATA - 1; k >= 0; --k ) {
			readAndSkipPast( HVL_STRATA_STRS[k], inputFile );
			for ( j = CD4_NUM_STRATA - 1; j >= 0; --j )
				fscanf( inputFile, "%lf", &(cohortInputs.probOIHistoryAtEntry[j][k][i]) );
		}
	}

	// read in prevalence and incidence of generic risk factors, along with string labels for them 
	readAndSkipPast("ProbRiskFactorPrev", inputFile);
	for (int i = 0; i < RISK_FACT_NUM; i++) {
		fscanf(inputFile, "%lf ", &(cohortInputs.probRiskFactorPrev[i]));
	}
	readAndSkipPast("ProbRiskFactorIncid", inputFile);
	for (int i = 0; i < RISK_FACT_NUM; i++) {
		fscanf(inputFile, "%lf ", &(cohortInputs.probRiskFactorIncid[i]));
	}
	readAndSkipPast( "GenRiskFactorStrs", inputFile);
	for (int i = 0; i < RISK_FACT_NUM; i++) {
		fscanf( inputFile, "%32s", RISK_FACT_STRS[i] );
		strcpy(DTH_CAUSES_STRS[DTH_RISK_1 + i], RISK_FACT_STRS[i]);
	}

	//Read in transmission inputs
	readAndSkipPast("ShowTransmissionOutput", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.showTransmissionOutput = (bool) tempBool;

	for ( k = HVL_NUM_STRATA - 1; k >= 0; --k ) {
		readAndSkipPast2( "TransmissionRateOnART", HVL_STRATA_STRS[k], inputFile );
		for ( j = CD4_NUM_STRATA - 1; j >= 0; --j )
			fscanf( inputFile, "%lf", &(cohortInputs.transmRateOnART[j][k]) );
	}
	readAndSkipPast2( "TransmissionRateOnART", "Acute", inputFile );
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j )
		fscanf( inputFile, "%lf", &(cohortInputs.transmRateOnARTAcute[j]) );

	for ( k = HVL_NUM_STRATA - 1; k >= 0; --k ) {
		readAndSkipPast2( "TransmissionRateOffART", HVL_STRATA_STRS[k], inputFile );
		for ( j = CD4_NUM_STRATA - 1; j >= 0; --j )
			fscanf( inputFile, "%lf", &(cohortInputs.transmRateOffART[j][k]) );
	}
	readAndSkipPast2( "TransmissionRateOffART", "Acute", inputFile );
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j )
		fscanf( inputFile, "%lf", &(cohortInputs.transmRateOffARTAcute[j]) );

	readAndSkipPast("TransmissionUseHIVTestAcuteDef", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.transmUseHIVTestAcuteDefinition = (bool) tempBool;

	readAndSkipPast("TransmissionAcuteDuration", inputFile);
	fscanf(inputFile, "%d", &(cohortInputs.transmAcuteDuration));

	readAndSkipPast("IntvlTransmissionRateMultiplier", inputFile);
	for (i = 0; i < 2; i++)
		fscanf(inputFile, "%d", &(cohortInputs.transmRateMultInterval[i]));

	readAndSkipPast("TransmissionRateMultiplier", inputFile);
	for (i = 0; i < 3; i++)
		fscanf(inputFile, "%lf", &(cohortInputs.transmRateMult[i]));

	for ( i = 0; i < TRANSM_RISK_NUM; i++){
		readAndSkipPast2("TransmissionRiskDistribution", TRANSM_RISK_STRS[i], inputFile);
		for (j = 0; j < TRANSM_RISK_AGE_NUM; j++){
			fscanf(inputFile, "%lf", &(cohortInputs.transmRiskDistrib[GENDER_MALE][j][i]));
		}
		for (j = 0; j < TRANSM_RISK_AGE_NUM; j++){
			fscanf(inputFile, "%lf", &(cohortInputs.transmRiskDistrib[GENDER_FEMALE][j][i]));
		}
	}

	readAndSkipPast("TransmissionRiskMultiplierBounds", inputFile);
	for ( i = 0; i < 2; i++){
		fscanf( inputFile, "%d", &(cohortInputs.transmRiskMultBounds[i]));
	}

	for (i = 0; i < 3; i++){
		sprintf(scratch, "TransmissionRiskMultiplier_T%d", i + 1);
		readAndSkipPast(scratch, inputFile);
		for ( j = 0; j < TRANSM_RISK_NUM; j++){
			fscanf( inputFile, "%lf", &(cohortInputs.transmRiskMult[j][i]));
		}
	}

	readAndSkipPast("UseDynamicTransmission", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.useDynamicTransm = (bool) tempBool;
	cohortInputs.updateDynamicTransmInc = true;

	readAndSkipPast("DynamicTransmissionNumTransmissionsHRG", inputFile);
	fscanf(inputFile, "%lf", &(cohortInputs.dynamicTransmHRGTransmissions));
	readAndSkipPast("DynamicTransmissionPropHRGAttrib", inputFile);
	fscanf(inputFile, "%lf", &(cohortInputs.dynamicTransmPropHRGAttributable));
	readAndSkipPast("DynamicTransmissionNumHIVPosHRG", inputFile);
	fscanf(inputFile, "%lf", &(cohortInputs.dynamicTransmNumHIVPosHRG));
	readAndSkipPast("DynamicTransmissionNumHIVNegHRG", inputFile);
	fscanf(inputFile, "%lf", &(cohortInputs.dynamicTransmNumHIVNegHRG));
	readAndSkipPast("DynamicTransmissionWarmupSize", inputFile);
	fscanf(inputFile, "%d", &(cohortInputs.dynamicTransmWarmupSize));

	readAndSkipPast("DynamicTransmissionKeepPrEPAfterWarmup", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.keepPrEPAfterWarmup = (bool) tempBool;
	readAndSkipPast("DynamicTransmissionUsePrEPDuringWarmup", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.usePrEPDuringWarmup = (bool) tempBool;

	readAndSkipPast("TransmissionUseEndLifeHVLAdjust", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	cohortInputs.useTransmEndLifeHVLAdjustment = (bool) tempBool;

	readAndSkipPast("TransmissionEndLifeAdjustCD4Threshold", inputFile);
	fscanf(inputFile, "%lf", &(cohortInputs.transmEndLifeHVLAdjustmentCD4Threshold));

	readAndSkipPast("TransmissionEndLifeAdjustARTLineThreshold", inputFile);
	fscanf(inputFile, "%d", &tempInt);
	cohortInputs.transmEndLifeHVLAdjustmentARTLineThreshold = tempInt - 1;
} /* end readCohortInputs */

/* readTreatmentInputsPart1 reads data from the Treatment tab of the input sheet, 
	split in the middle for the reading of ARTs tab */
void SimContext::readTreatmentInputsPart1() {
	int i, j, tempBool;

	// read in clinic visit interval
	readAndSkipPast( "IntvlClinicVisit", inputFile );
	fscanf( inputFile, "%d", &(treatmentInputs.clinicVisitInterval) );
	// read in probabilities of detecting patient's prior OI history
	readAndSkipPast( "ProbDetOI_Entry", inputFile );
	for ( j = 0; j < OI_NUM; ++j )
		fscanf( inputFile, "%lf", &(treatmentInputs.probDetectOIAtEntry[j]) );
	readAndSkipPast( "ProbDetOI_LastVst", inputFile );
	for ( j = 0; j < OI_NUM; ++j )
		fscanf( inputFile, "%lf", &(treatmentInputs.probDetectOISinceLastVisit[j]) );
	// read in probabilities of switching to secondary proph at OI event
	readAndSkipPast( "ProbSwitchSecProph", inputFile );
	for ( j = 0; j < OI_NUM; ++j )
		fscanf( inputFile, "%lf", &(treatmentInputs.probSwitchSecondaryProph[j]) );

	// read in CD4 and on ART months thresholds used for testing frequency
	readAndSkipPast( "IntvlCD4Tst_CD4Threshold", inputFile );
	fscanf( inputFile, "%lf", &treatmentInputs.testingIntervalCD4Threshold );
	readAndSkipPast("IntvlCD4Tst_MonthsThreshold", inputFile);
	fscanf( inputFile, "%d %d", &treatmentInputs.testingIntervalARTMonthsThreshold,
			&treatmentInputs.testingIntervalLastARTMonthsThreshold);
	// read in CD4/HVL test intervals
	readAndSkipPast( "IntvlCD4Tst", inputFile );
	fscanf( inputFile, "%d %d %d %d %d %d %d",
		&treatmentInputs.CD4TestingIntervalPreARTHighCD4, &treatmentInputs.CD4TestingIntervalPreARTLowCD4,
		&treatmentInputs.CD4TestingIntervalOnART[0], &treatmentInputs.CD4TestingIntervalOnART[1],
		&treatmentInputs.CD4TestingIntervalOnLastART[0], &treatmentInputs.CD4TestingIntervalOnLastART[1],
		&treatmentInputs.CD4TestingIntervalPostART);
	readAndSkipPast( "IntvlHVLTst", inputFile );
	fscanf( inputFile, "%d %d %d %d %d %d %d",
		&treatmentInputs.HVLTestingIntervalPreARTHighCD4, &treatmentInputs.HVLTestingIntervalPreARTLowCD4,
		&treatmentInputs.HVLTestingIntervalOnART[0], &treatmentInputs.HVLTestingIntervalOnART[1],
		&treatmentInputs.HVLTestingIntervalOnLastART[0], &treatmentInputs.HVLTestingIntervalOnLastART[1],
		&treatmentInputs.HVLTestingIntervalPostART );

	// read in HVL test err prob (lower & higher)
	readAndSkipPast( "HVLtestErrProb", inputFile );
	fscanf( inputFile, "%lf %lf", &treatmentInputs.probHVLTestErrorHigher, &treatmentInputs.probHVLTestErrorLower );
	// read in CD4 test err deviation and bias
	readAndSkipPast( "CD4testErrSDev", inputFile );
	fscanf( inputFile, "%lf", &treatmentInputs.CD4TestStdDevPercentage );
	readAndSkipPast( "CD4testBiasMean", inputFile );
	fscanf( inputFile, "%lf", &treatmentInputs.CD4TestBiasMean );
	readAndSkipPast( "CD4testBiasSdev", inputFile );
	fscanf( inputFile, "%lf", &treatmentInputs.CD4TestBiasStdDevPercentage );

	// read in whether to perform ART obsv fail CD4/HVL tests outside clinic visit
	readAndSkipPast( "ObsvARTFailTestOnRegClinicVst", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	treatmentInputs.ARTFailureOnlyAtRegularVisit = (bool) tempBool;
	// read in numbers of CD4/HVL tests outside clinic visit at ART initiation
	readAndSkipPast( "ARTInitHVLTestsWOClinicVst", inputFile );
	fscanf( inputFile, "%d", &treatmentInputs.numARTInitialHVLTests );
	readAndSkipPast( "ARTInitCD4TestsWOClinicVst", inputFile );
	fscanf( inputFile, "%d", &treatmentInputs.numARTInitialCD4Tests );
	// read in if OI visits are not treated as regular clinic visits
	readAndSkipPast( "OIVstAsNotSchedClinicVst", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	treatmentInputs.emergencyVisitIsNotRegularVisit = (bool) tempBool;

	// read in months to lag of cd4/hvl testing availability
	readAndSkipPast( "LagToCD4Test", inputFile );
	fscanf( inputFile, "%d", &treatmentInputs.CD4TestingLag );
	readAndSkipPast( "LagToHVLTest", inputFile );
	fscanf( inputFile, "%d", &treatmentInputs.HVLTestingLag );

	//read in criteria to end CD4 monitoring for this line of ART
	readAndSkipPast( "StopCD4MonitoringEnable", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	treatmentInputs.cd4MonitoringStopEnable = (bool) tempBool;

	readAndSkipPast( "StopCD4MonitoringThreshold", inputFile );
	fscanf( inputFile, "%lf", &treatmentInputs.cd4MonitoringStopThreshold );
	readAndSkipPast( "StopCD4MonitoringMthsPostARTInit", inputFile );
	fscanf( inputFile, "%d", &treatmentInputs.cd4MonitoringStopMthsPostARTInit );

	//ART starting criteria
	// read in CD4 bounds
	readAndSkipPast2( "ARTstart_CD4", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(treatmentInputs.startART[i].CD4BoundsOnly[UPPER_BOUND]) );
	readAndSkipPast2( "ARTstart_CD4", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(treatmentInputs.startART[i].CD4BoundsOnly[LOWER_BOUND]) );
	// read in HVL bounds to administer ARTs
	readAndSkipPast2( "ARTstart_HVL", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].HVLBoundsOnly[UPPER_BOUND]) );
	readAndSkipPast2( "ARTstart_HVL", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].HVLBoundsOnly[LOWER_BOUND]) );
	// read in CD4 & HVL bounds to administer ARTs
	readAndSkipPast2( "ARTstart_CD4HVL", "CD4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(treatmentInputs.startART[i].CD4BoundsWithHVL[UPPER_BOUND]) );
	readAndSkipPast2( "ARTstart_CD4HVL", "CD4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(treatmentInputs.startART[i].CD4BoundsWithHVL[LOWER_BOUND]) );
	readAndSkipPast2( "ARTstart_CD4HVL", "HVLupp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].HVLBoundsWithCD4[UPPER_BOUND]) );
	readAndSkipPast2( "ARTstart_CD4HVL", "HVLlwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].HVLBoundsWithCD4[LOWER_BOUND]) );
	// read in OI criteria to administer ARTs
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "ARTstart_OIs", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			treatmentInputs.startART[i].OIHistory[j] = (bool) tempBool;
		}
	}
	readAndSkipPast2( "ARTstart_OIs", "numOIs", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].numOIs) );
	// read in CD4 & OI criteria to administer ARTs
	readAndSkipPast2( "ARTstart_CD4OI", "CD4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(treatmentInputs.startART[i].CD4BoundsWithOIs[UPPER_BOUND]) );
	readAndSkipPast2( "ARTstart_CD4OI", "CD4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(treatmentInputs.startART[i].CD4BoundsWithOIs[LOWER_BOUND]) );
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "ARTstart_CD4OI", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			treatmentInputs.startART[i].OIHistoryWithCD4[j] = (bool) tempBool;
		}
	}

	readAndSkipPast2( "ARTstart", "obsvFailFPSuppression", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i){
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.startART[i].ensureSuppFalsePositiveFailure = (bool) tempBool;
	}

	// read in minimum/maximum mth # to start ART
	readAndSkipPast2( "ARTstart", "minMthNum", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].minMonthNum) );

	readAndSkipPast2( "ARTstart", "maxMthNum", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].maxMonthNum) );

	readAndSkipPast2( "ARTstart", "MthsSincePrevRegStop", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(treatmentInputs.startART[i].monthsSincePrevRegimen) );
} /* endReadTreatmentInputsPart1 */

/* readTreatmentInputsPart2 reads data from the second half of the Treatment tab of the input sheet, split in the middle for the reading of ARTs tab */
void SimContext::readTreatmentInputsPart2() {
	int i, j, tempBool, tempInt;
	char buffer[256];

	// read in whether to enable interruption of ART
	readAndSkipPast( "EnableSTIforART", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.enableSTIForART[i] = (bool) tempBool;
	}

	// ART Failure parameters
	// read in # HVL lvls to incr for fail diag
	readAndSkipPast( "ARTfail_hvlNumIncr", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].HVLNumIncrease) );
	// read in absolute HVL counts for fail diag
	readAndSkipPast( "ARTfail_hvlAbsol", inputFile );
	readAndSkipPast( "uppBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].HVLBounds[UPPER_BOUND]) );
	readAndSkipPast( "ARTfail_hvlAbsol", inputFile );
	readAndSkipPast( "lwrBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].HVLBounds[LOWER_BOUND]) );
	// read in true/false use HVL as setpoint for fail diag
	readAndSkipPast( "ARTfail_hvlAtSetptAsFailDiag", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.failART[i].HVLFailAtSetpoint = (bool) tempBool;
	}
	// read in # of months before using HVL criteria
	readAndSkipPast( "ARTfail_hvlMthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].HVLMonthsFromInit) );
	// read in CD4 percentage to decr for fail diag
	readAndSkipPast( "ARTfail_cd4PercDrop", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.failART[i].CD4PercentageDrop) );
	// read in true/false use CD4 as below pre-ART nadir for fail diag
	readAndSkipPast( "ARTfail_cd4BelowPreARTNadir", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.failART[i].CD4BelowPreARTNadir = (bool) tempBool;
	}
	// read in absolute CD4 counts as OR criteria for fail diag
	readAndSkipPast( "ARTfail_cd4AbsolOR", inputFile );
	readAndSkipPast( "uppBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.failART[i].CD4BoundsOR[UPPER_BOUND]) );
	readAndSkipPast( "ARTfail_cd4AbsolOR", inputFile );
	readAndSkipPast( "lwrBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.failART[i].CD4BoundsOR[LOWER_BOUND]) );
	// read in absolute CD4 counts as AND criteria for fail diag
	readAndSkipPast( "ARTfail_cd4AbsolAND", inputFile );
	readAndSkipPast( "uppBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.failART[i].CD4BoundsAND[UPPER_BOUND]) );
	readAndSkipPast( "ARTfail_cd4AbsolAND", inputFile );
	readAndSkipPast( "lwrBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.failART[i].CD4BoundsAND[LOWER_BOUND]) );
	// read in # of months before using CD4 criteria
	readAndSkipPast( "ARTfail_cd4MthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].CD4MonthsFromInit) );
	// read in whether to treat OI event as ART fail diag
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "ARTfail_OIs", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < ART_NUM_LINES; ++i ){
			fscanf( inputFile, "%d", &tempInt);
			treatmentInputs.failART[i].OIsEvent[j] = (ART_FAIL_BY_OI) tempInt;
		}
	}
	readAndSkipPast( "ARTfail_OIsMinNum", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].OIsMinNum) );
	readAndSkipPast( "ARTfail_OIsMthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].OIsMonthsFromInit) );
	// read in ART failure diagnoses criteria parameters
	readAndSkipPast( "ARTfail_diagNumTestsFail", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].diagnoseNumTestsFail) );
	readAndSkipPast( "ARTfail_diagUseHVLTestsConfirm", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.failART[i].diagnoseUseHVLTestsConfirm = (bool) tempBool;
	}
	readAndSkipPast( "ARTfail_diagUseCD4TestsConfirm", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.failART[i].diagnoseUseCD4TestsConfirm = (bool) tempBool;
	}
	readAndSkipPast( "ARTfail_diagNumTestsConfirm", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.failART[i].diagnoseNumTestsConfirm) );

	//read in ART stopping policy
	// read in maximum number of months to be on ART
	readAndSkipPast( "ARTstop_MaxMthsOnART", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopART[i].maxMonthsOnART));
	// read in stop on major toxicity
	readAndSkipPast("ARTstop_MajorToxicity", inputFile);
	for (i = 0; i < ART_NUM_LINES; i++) {
		fscanf(inputFile, "%d", &tempBool);
		treatmentInputs.stopART[i].withMajorToxicty = (bool) tempBool;
	}
	// read in criteria to use after failure has been observed
	readAndSkipPast( "ARTstop_OnFailImmed", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.stopART[i].afterFailImmediate = (bool) tempBool;
	}
	readAndSkipPast( "ARTstop_OnFailBelowCD4", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopART[i].afterFailCD4LowerBound));
	readAndSkipPast( "ARTstop_OnFailSevereOI", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.stopART[i].afterFailWithSevereOI = (bool) tempBool;
	}
	readAndSkipPast( "ARTstop_OnFailMthsAfterObsv", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopART[i].afterFailMonthsFromObserved));
	// read in minimum month number to stop ART
	readAndSkipPast( "ARTstop_OnFailMinMthNum", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopART[i].afterFailMinMonthNum) );
	readAndSkipPast( "ARTstop_OnFailMthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopART[i].afterFailMonthsFromInit) );

	//Read in next line if stopping for major tox
	readAndSkipPast( "ARTStopMajorToxNextLine", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i){
		fscanf( inputFile, "%d", &tempInt);
		if (tempInt != NOT_APPL)
			tempInt--;
		treatmentInputs.stopART[i].nextLineAfterMajorTox = tempInt;
	}

	// read in ART resistance penalty parameters
	for (i = 0; i < ART_NUM_LINES; i++) {
		sprintf(buffer, "reg_%d", i + 1);
		readAndSkipPast2("ARTresistRed_initSucc", buffer, inputFile);
		for (j = 0; j < ART_NUM_LINES; j++) {
			fscanf(inputFile, "%lf", &(treatmentInputs.ARTResistancePriorRegimen[i][j]));
		}
	}
	readAndSkipPast( "ARTresistRed_HVL", inputFile );
	for (i = HVL_NUM_STRATA - 1; i >= 0; --i) {
		fscanf(inputFile, "%lf", &(treatmentInputs.ARTResistanceHVL[i]));
	}

	// read in primary OI proph regimen starting criteria
	readAndSkipPast( "PriProphStart", inputFile );
	readAndSkipPast( "boolFlag", inputFile );
	for ( i = 0; i < OI_NUM; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.startProph[PROPH_PRIMARY][i].useOrEvaluation = (bool) tempBool;
	}
	readAndSkipPast( "PriProphStart", inputFile );
	readAndSkipPast( "curCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_PRIMARY][i].currCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "PriProphStart", inputFile );
	readAndSkipPast( "curCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_PRIMARY][i].currCD4Bounds[LOWER_BOUND]) );
	readAndSkipPast( "PriProphStart", inputFile );
	readAndSkipPast( "minCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_PRIMARY][i].minCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "PriProphStart", inputFile );
	readAndSkipPast( "minCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_PRIMARY][i].minCD4Bounds[LOWER_BOUND]) );
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "PriProphStart", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(treatmentInputs.startProph[PROPH_PRIMARY][i].OIHistory[j]) );
	}
	readAndSkipPast( "PriProphStart", inputFile );
	readAndSkipPast( "minMthNum", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.startProph[PROPH_PRIMARY][i].minMonthNum) );

	// read in primary OI proph regimen stopping criteria
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "boolFlag", inputFile );
	for ( i = 0; i < OI_NUM; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.stopProph[PROPH_PRIMARY][i].useOrEvaluation = (bool) tempBool;
	}
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "curCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].currCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "curCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].currCD4Bounds[LOWER_BOUND]) );
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "minCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].minCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "minCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].minCD4Bounds[LOWER_BOUND]) );
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "PriProphStop", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].OIHistory[j]) );
	}
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "minMthNum", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].minMonthNum) );
	readAndSkipPast( "PriProphStop", inputFile );
	readAndSkipPast( "mthsOnProph", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopProph[PROPH_PRIMARY][i].monthsOnProph) );

	// read in secondart OI proph regimen starting criteria
	readAndSkipPast( "SecProphStart", inputFile );
	readAndSkipPast( "boolFlag", inputFile );
	for ( i = 0; i < OI_NUM; ++i ){
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.startProph[PROPH_SECONDARY][i].useOrEvaluation = (bool) tempBool;
	}	
	readAndSkipPast( "SecProphStart", inputFile );
	readAndSkipPast( "curCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_SECONDARY][i].currCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "SecProphStart", inputFile );
	readAndSkipPast( "curCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_SECONDARY][i].currCD4Bounds[LOWER_BOUND]) );
	readAndSkipPast( "SecProphStart", inputFile );
	readAndSkipPast( "minCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_SECONDARY][i].minCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "SecProphStart", inputFile );
	readAndSkipPast( "minCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.startProph[PROPH_SECONDARY][i].minCD4Bounds[LOWER_BOUND]) );
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "SecProphStart", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(treatmentInputs.startProph[PROPH_SECONDARY][i].OIHistory[j]) );
	}
	readAndSkipPast( "SecProphStart", inputFile );
	readAndSkipPast( "minMthNum", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.startProph[PROPH_SECONDARY][i].minMonthNum) );

	// read in secondary OI proph regimen stopping criteria
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "boolFlag", inputFile );
	for ( i = 0; i < OI_NUM; ++i ){
		fscanf( inputFile, "%d", &tempBool);
		treatmentInputs.stopProph[PROPH_SECONDARY][i].useOrEvaluation = (bool) tempBool;
	}	
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "curCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].currCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "curCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].currCD4Bounds[LOWER_BOUND]) );
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "minCD4upp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].minCD4Bounds[UPPER_BOUND]) );
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "minCD4lwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].minCD4Bounds[LOWER_BOUND]) );
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "SecProphStop", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].OIHistory[j]) );
	}
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "minMthNum", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].minMonthNum) );
	readAndSkipPast( "SecProphStop", inputFile );
	readAndSkipPast( "mthsOnProph", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(treatmentInputs.stopProph[PROPH_SECONDARY][i].monthsOnProph) );
} /* end readTreatmentInputsPart2 */

/* readLTFUInputs reads data from the LTFU tab of the input sheet */
void SimContext::readLTFUInputs() {
	int tempBool;
	char scratch[256];

	// read in LTFU variables
	readAndSkipPast( "UseLTFU", inputFile);
	fscanf( inputFile, "%d", &tempBool);
	ltfuInputs.useLTFU = (bool) tempBool;
	readAndSkipPast( "PropRespLTFUPreART", inputFile);
	fscanf( inputFile, "%lf %lf", &(ltfuInputs.propRespondLTFUPreARTLogitMean),
		&(ltfuInputs.propRespondLTFUPreARTLogitStdDev));

	readAndSkipPast( "UseInterventionHetOutcomes", inputFile);
	fscanf( inputFile, "%d", &tempBool);
	ltfuInputs.useInterventionLTFU = (bool) tempBool;

	readAndSkipPast("HetOutcomes", inputFile);
	readAndSkipPast("LTFU",inputFile);
	fscanf(inputFile, "%lf %lf %lf %lf", &(ltfuInputs.responseThresholdLTFU[0]), &(ltfuInputs.responseThresholdLTFU[1]),&(ltfuInputs.responseValueLTFU[0]),&(ltfuInputs.responseValueLTFU[1]));

	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++){
		sprintf( scratch, "HetOutcomesPeriod%d", i );
		readAndSkipPast( scratch, inputFile );
		readAndSkipPast("LTFU",inputFile);
		fscanf(inputFile, "%lf %lf %lf %lf", &(ltfuInputs.responseThresholdPeriodLTFU[i][0]), &(ltfuInputs.responseThresholdPeriodLTFU[i][1]),
				&(ltfuInputs.responseValuePeriodLTFU[i][0]),&(ltfuInputs.responseValuePeriodLTFU[i][1]));
	}

	readAndSkipPast("HetOutcomesOffIntervention", inputFile);
	readAndSkipPast("LTFU",inputFile);
	fscanf(inputFile, "%lf %lf %lf %lf", &(ltfuInputs.responseThresholdLTFUOffIntervention[0]), &(ltfuInputs.responseThresholdLTFUOffIntervention[1]),
			&(ltfuInputs.responseValueLTFUOffIntervention[0]),&(ltfuInputs.responseValueLTFUOffIntervention[1]));


	readAndSkipPast("PropGenMedCostsByState",inputFile);
	fscanf(inputFile, "%lf %lf %lf %lf", &(ltfuInputs.propGeneralMedicineCost[HIV_CARE_UNDETECTED]), &(ltfuInputs.propGeneralMedicineCost[HIV_CARE_UNLINKED]), &(ltfuInputs.propGeneralMedicineCost[HIV_CARE_IN_CARE]), &(ltfuInputs.propGeneralMedicineCost[HIV_CARE_LTFU]));
	ltfuInputs.propGeneralMedicineCost[HIV_CARE_NEG] = 1.0;
	ltfuInputs.propGeneralMedicineCost[HIV_CARE_RTC] = ltfuInputs.propGeneralMedicineCost[HIV_CARE_IN_CARE];

	readAndSkipPast("PropInterventionCostsByState",inputFile);
	fscanf(inputFile, "%lf %lf %lf %lf", &(ltfuInputs.propInterventionCost[HIV_CARE_UNDETECTED]), &(ltfuInputs.propInterventionCost[HIV_CARE_UNLINKED]),
			&(ltfuInputs.propInterventionCost[HIV_CARE_IN_CARE]), &(ltfuInputs.propInterventionCost[HIV_CARE_LTFU]));
	ltfuInputs.propInterventionCost[HIV_CARE_NEG] = 1.0;
	ltfuInputs.propInterventionCost[HIV_CARE_RTC] = ltfuInputs.propInterventionCost[HIV_CARE_IN_CARE];


	readAndSkipPast( "pLTFUOIProph", inputFile);
	fscanf( inputFile, "%lf", &ltfuInputs.probRemainOnOIProph);
	readAndSkipPast( "pLTFUOITreat", inputFile);
	fscanf( inputFile, "%lf", &ltfuInputs.probRemainOnOITreatment);

	//Return to Care variables...
	readAndSkipPast( "RTCMinMonthsLost", inputFile);
	fscanf( inputFile, "%d", &ltfuInputs.minMonthsRemainLost);
	readAndSkipPast( "RTCBackground", inputFile);
	fscanf( inputFile, "%lf", &(ltfuInputs.regressionCoefficientsRTC[RTC_BACKGROUND]));
	readAndSkipPast( "RTCCD4", inputFile);
	fscanf( inputFile, "%lf", &(ltfuInputs.regressionCoefficientsRTC[RTC_CD4]));
	readAndSkipPast( "RTCAcuteSevereOI", inputFile);
	fscanf( inputFile, "%lf", &(ltfuInputs.regressionCoefficientsRTC[RTC_ACUTESEVEREOI]));
	readAndSkipPast( "RTCAcuteMildOI", inputFile);
	fscanf( inputFile, "%lf", &(ltfuInputs.regressionCoefficientsRTC[RTC_ACUTEMILDOI]));
	readAndSkipPast( "RTCTBPosDiagnosis", inputFile);
	fscanf( inputFile, "%lf", &(ltfuInputs.regressionCoefficientsRTC[RTC_TBPOS]));
	readAndSkipPast( "RTCCD4Threshold", inputFile);
	fscanf( inputFile, "%lf", &ltfuInputs.CD4ThresholdRTC);
	readAndSkipPast("RTCSevereOIType", inputFile);
	for (int i = 0; i < OI_NUM; i++) {
		fscanf(inputFile, "%d ", &tempBool);
		ltfuInputs.severeOIsRTC[i] = (bool) tempBool;
	}
	readAndSkipPast( "RTCMaxTimePrevOnART", inputFile);
	fscanf( inputFile, "%d", &ltfuInputs.maxMonthsAfterObservedFailureToRestartRegimen);
	readAndSkipPast( "RTCProbTakeSameART", inputFile);
	fscanf( inputFile, "%lf", &ltfuInputs.probRestartRegimenWithoutObsvervedFailure);
	readAndSkipPast( "RTCRecheckARTPolicies", inputFile);
	fscanf( inputFile, "%d", &tempBool);
	ltfuInputs.recheckARTStartPoliciesAtRTC = (bool) tempBool;

	readAndSkipPast( "RTCProbSuppByPrevOutcome", inputFile);
	fscanf( inputFile, "%d", &tempBool);
	ltfuInputs.useProbSuppByPrevOutcome = (bool) tempBool;
	readAndSkipPast( "RTCProbSuppPrevFail", inputFile);
	for (int i = 0; i < ART_NUM_LINES; i++) {
		fscanf(inputFile, "%lf", &(ltfuInputs.probSuppressionWhenReturnToFailed[i]));
	}
	readAndSkipPast( "RTCProbSuppPrevSupp", inputFile);
	for (int i = 0; i < ART_NUM_LINES; i++) {
		fscanf(inputFile, "%lf", &(ltfuInputs.probSuppressionWhenReturnToSuppressed[i]));
	}

	readAndSkipPast( "RTCProbResumeIntervention", inputFile);
	fscanf(inputFile, "%lf", &(ltfuInputs.probResumeInterventionRTC));

	readAndSkipPast( "RTCResumeInterventionCost", inputFile);
	fscanf(inputFile, "%lf", &(ltfuInputs.costResumeInterventionRTC));
} /* end readLTFUInputs */

/* readHeterogeneityInputs reads data from the Heterogeneity tab of the input sheet */
void SimContext::readHeterogeneityInputs() {
	int tempBool, tempInt;
	// read in the propensity to respond coefficients
	readAndSkipPast("PropRespBaseline", inputFile);
	fscanf(inputFile, "%lf %lf", &(heterogeneityInputs.propRespondBaselineLogitMean),
			&(heterogeneityInputs.propRespondBaselineLogitStdDev));
	readAndSkipPast("PropRespAge", inputFile);
	for (int i = 0; i < RESP_AGE_CAT_NUM; i++) {
		fscanf(inputFile, "%lf ", &(heterogeneityInputs.propRespondAge[i]));
	}

	readAndSkipPast("PropRespPedsAge", inputFile);
	fscanf(inputFile, "%lf %lf", &(heterogeneityInputs.propRespondAgeEarly),&(heterogeneityInputs.propRespondAgeLate));

	readAndSkipPast("PropRespCD4", inputFile);
	for (int i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		fscanf(inputFile, "%lf ", &(heterogeneityInputs.propRespondCD4[i]));
	}
	readAndSkipPast("PropRespFemale", inputFile);
	fscanf(inputFile, "%lf", &(heterogeneityInputs.propRespondFemale));
	readAndSkipPast("PropRespHistOIs", inputFile);
	fscanf(inputFile, "%lf", &(heterogeneityInputs.propRespondHistoryOIs));
	readAndSkipPast("PropRespPriorARTTox", inputFile);
	fscanf(inputFile, "%lf", &(heterogeneityInputs.propRespondPriorARTToxicity));
	readAndSkipPast("PropRespRiskFactors", inputFile);
	for (int i = 0; i < RISK_FACT_NUM; i++) {
		fscanf(inputFile, "%lf ", &(heterogeneityInputs.propRespondRiskFactor[i]));
	}

	readAndSkipPast( "UseIntervention", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++){
		fscanf( inputFile, "%d", &tempBool);
		heterogeneityInputs.useIntervention[i] = (bool) tempBool;
	}

	readAndSkipPast("InterventionDurationMean", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++)
		fscanf(inputFile, "%lf", &(heterogeneityInputs.interventionDurationMean[i]));
	readAndSkipPast("InterventionDurationSD", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++)
		fscanf(inputFile, "%lf", &(heterogeneityInputs.interventionDurationSD[i]));


	readAndSkipPast("InterventionPropAdjustmentMean", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++)
		fscanf(inputFile, "%lf", &(heterogeneityInputs.interventionAdjustmentMean[i]));
	readAndSkipPast("InterventionPropAdjustmentSD", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++)
		fscanf(inputFile, "%lf", &(heterogeneityInputs.interventionAdjustmentSD[i]));

	readAndSkipPast("InterventionPropAdjustmentDistribution", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++){
		fscanf(inputFile, "%d", &tempInt);
		heterogeneityInputs.interventionAdjustmentDistribution[i] = (HET_ART_LOGIT_DISTRIBUTION) tempInt;
	}

	readAndSkipPast("InterventionCostStartup", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++)
		fscanf(inputFile, "%lf", &(heterogeneityInputs.interventionCostInit[i]));
	readAndSkipPast("InterventionCostMonthly", inputFile);
	for (int i = 0; i < HET_INTV_NUM_PERIODS; i++)
		fscanf(inputFile, "%lf", &(heterogeneityInputs.interventionCostMonthly[i]));
} /* end readHeterogeneityInputs */

/* readSTIInputs reads data from the STI tab of the input sheet */
void SimContext::readSTIInputs() {
	char tmpBuf1[256], tmpBuf2[256];
	int i, j, tempBool;

	// read in STI initiation parameters
	// read in CD4 bounds
	readAndSkipPast2( "STIstart_CD4", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.firstInterruption[i].CD4BoundsOnly[UPPER_BOUND]) );
	readAndSkipPast2( "STIstart_CD4", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.firstInterruption[i].CD4BoundsOnly[LOWER_BOUND]) );
	// read in HVL bounds to begin STI
	readAndSkipPast2( "STIstart_HVL", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].HVLBoundsOnly[UPPER_BOUND]) );
	readAndSkipPast2( "STIstart_HVL", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].HVLBoundsOnly[LOWER_BOUND]) );
	// read in CD4 & HVL bounds to begin STI
	readAndSkipPast2( "STIstart_CD4HVL", "CD4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.firstInterruption[i].CD4BoundsWithHVL[UPPER_BOUND]) );
	readAndSkipPast2( "STIstart_CD4HVL", "CD4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.firstInterruption[i].CD4BoundsWithHVL[LOWER_BOUND]) );
	readAndSkipPast2( "STIstart_CD4HVL", "HVLupp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].HVLBoundsWithCD4[UPPER_BOUND]) );
	readAndSkipPast2( "STIstart_CD4HVL", "HVLlwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].HVLBoundsWithCD4[LOWER_BOUND]) );
	// read in OI criteria to begin STI
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "STIstart_OIs", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			stiInputs.firstInterruption[i].OIHistory[j] = (bool) tempBool;
		}
	}
	readAndSkipPast2( "STIstart_OIs", "numOIs", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].numOIs) );
	// read in CD4 & OI criteria to begin STI
	readAndSkipPast2( "STIstart_CD4OI", "CD4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.firstInterruption[i].CD4BoundsWithOIs[UPPER_BOUND]) );
	readAndSkipPast2( "STIstart_CD4OI", "CD4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.firstInterruption[i].CD4BoundsWithOIs[LOWER_BOUND]) );
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "STIstart_CD4OI", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			stiInputs.firstInterruption[i].OIHistoryWithCD4[j] = (bool) tempBool;
		}
	}
	// read in minimum mth # to beginSTI
	readAndSkipPast2( "STIstart", "minMthNum", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].minMonthNum) );
	readAndSkipPast2( "STIstart", "minMthNum_ARTinit", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.firstInterruption[i].monthsSinceARTStart) );

	// read in STI ART restarting parameters
	readAndSkipPast2( "STI_restartART", "cd4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.ARTRestartCD4Bounds[i][UPPER_BOUND]) );
	readAndSkipPast2( "STI_restartART", "cd4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.ARTRestartCD4Bounds[i][LOWER_BOUND]) );
	readAndSkipPast2( "STI_restartART", "hvlupp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.ARTRestartHVLBounds[i][UPPER_BOUND]) );
	readAndSkipPast2( "STI_restartART", "hvllwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.ARTRestartHVLBounds[i][LOWER_BOUND]) );

	// read in STI ART successive interruption parameters
	readAndSkipPast2( "STI_restopART", "cd4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.ARTRestopCD4Bounds[i][UPPER_BOUND]) );
	readAndSkipPast2( "STI_restopART", "cd4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.ARTRestopCD4Bounds[i][LOWER_BOUND]) );
	readAndSkipPast2( "STI_restopART", "hvlupp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.ARTRestopHVLBounds[i][UPPER_BOUND]) );
	readAndSkipPast2( "STI_restopART", "hvllwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.ARTRestopHVLBounds[i][LOWER_BOUND]) );

	// read in STI endpoint parameters
	// read in CD4 bounds
	readAndSkipPast2( "STIendpt_CD4", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.endpoint[i].CD4BoundsOnly[UPPER_BOUND]) );
	readAndSkipPast2( "STIendpt_CD4", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.endpoint[i].CD4BoundsOnly[LOWER_BOUND]) );
	// read in HVL bounds for STI endpoint
	readAndSkipPast2( "STIendpt_HVL", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.endpoint[i].HVLBoundsOnly[UPPER_BOUND]) );
	readAndSkipPast2( "STIendpt_HVL", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.endpoint[i].HVLBoundsOnly[LOWER_BOUND]) );
	// read in CD4 & HVL bounds for STI endpoint
	readAndSkipPast2( "STIendpt_CD4HVL", "CD4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.endpoint[i].CD4BoundsWithHVL[UPPER_BOUND]) );
	readAndSkipPast2( "STIendpt_CD4HVL", "CD4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.endpoint[i].CD4BoundsWithHVL[LOWER_BOUND]) );
	readAndSkipPast2( "STIendpt_CD4HVL", "HVLupp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.endpoint[i].HVLBoundsWithCD4[UPPER_BOUND]) );
	readAndSkipPast2( "STIendpt_CD4HVL", "HVLlwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.endpoint[i].HVLBoundsWithCD4[LOWER_BOUND]) );
	// read in OI criteria for STI endpoint
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "STIendpt_OIs", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			stiInputs.endpoint[i].OIHistory[j] = (bool) tempBool;
		}
	}
	readAndSkipPast2( "STIendpt_OIs", "numOIs", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.endpoint[i].numOIs) );
	// read in CD4 & OI criteria for STI endpoint
	readAndSkipPast2( "STIendpt_CD4OI", "CD4upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.endpoint[i].CD4BoundsWithOIs[UPPER_BOUND]) );
	readAndSkipPast2( "STIendpt_CD4OI", "CD4lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%lf", &(stiInputs.endpoint[i].CD4BoundsWithOIs[LOWER_BOUND]) );
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "STIendpt_CD4OI", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			stiInputs.endpoint[i].OIHistoryWithCD4[j] = (bool) tempBool;
		}
	}
	// read in minimum mth # for STI endpoint
	readAndSkipPast2( "STIendpt", "minMthNum", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(stiInputs.endpoint[i].monthsSinceSTIStart) );
} /* end readSTIInputs */

/* readProphInputs reads data from the Prophs tab of the input sheet */
void SimContext::readProphInputs() {
	char scratch[256], buffer[256];
	int i, j, k, tempBool;

	for ( k = 0; k < OI_NUM; ++k) {
		for ( i = 0; i < PROPH_NUM; ++i ) {

			// read in OI proph id and name
			sprintf( scratch, "OI%d_PriProph%d", k + 1, i + 1 );
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Id", inputFile );
			int idNum;
			fscanf( inputFile, " %d", &idNum);



			// continue to next proph if this one is unspecified
			if (idNum == NOT_APPL) {
				prophsInputs[PROPH_PRIMARY][k][i] = NULL;

				continue;
			}
			// allocate a proph input structure
			prophsInputs[PROPH_PRIMARY][k][i] = new ProphInputs();

			// read in OI proph efficacy (for primary proph, primary OIs only in LDC model)
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffPriOIs", inputFile );

			for ( j = 0; j < OI_NUM; ++j ){
				fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->primaryOIEfficacy[j]) );

			}
			// read in OI primary proph efficacy on secondary OIs
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffSecOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->secondaryOIEfficacy[j]) );

			// read in proph resist prob, level of proph resistance, time of proph resistance,
			// cost factor of proph resistance, and death rate ratio for proph resistance for the primary proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Resist", inputFile );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->monthlyProbResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->percentResistance) );
			fscanf( inputFile, "%d", &(prophsInputs[PROPH_PRIMARY][k][i]->timeOfResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->costFactorResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->deathRateRatioResistance) );

			// read in min & maj tox for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Tox", inputFile );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->probMinorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->probMajorToxicity) );
			fscanf( inputFile, "%d", &(prophsInputs[PROPH_PRIMARY][k][i]->monthsToToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->deathRateRatioMajorToxicity) );

			// read in costs and QOL for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "CostQOL", inputFile );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->costMonthly) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->costMinorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->QOLMinorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->costMajorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_PRIMARY][k][i]->QOLMajorToxicity) );

			// read in proph switching inputs
			readAndSkipPast(scratch, inputFile);
			readAndSkipPast("Switch", inputFile);
			fscanf( inputFile, "%d", &(prophsInputs[PROPH_PRIMARY][k][i]->monthsToSwitch) );
			fscanf( inputFile, "%d", &tempBool);
			prophsInputs[PROPH_PRIMARY][k][i]->switchOnMinorToxicity = (bool) tempBool;
			fscanf( inputFile, "%d", &tempBool);
			prophsInputs[PROPH_PRIMARY][k][i]->switchOnMajorToxicity = (bool) tempBool;
		}
	}

	for ( k = 0; k < OI_NUM; ++k) {
		for ( i = 0; i < PROPH_NUM; ++i ) {
			// read in OI proph id and name
			sprintf( scratch, "OI%d_SecProph%d", k + 1, i + 1 );
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Id", inputFile );
			int idNum;
			fscanf( inputFile, " %d", &idNum);
			// continue to next proph if this one is unspecified
			if (idNum == NOT_APPL) {
				prophsInputs[PROPH_SECONDARY][k][i] = NULL;
				continue;
			}
			// allocate a proph input structure
			prophsInputs[PROPH_SECONDARY][k][i] = new ProphInputs();

			// read in OI proph efficacy (for primary proph, primary OIs only in LDC model)
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffPriOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->primaryOIEfficacy[j]) );

			// read in OI primary proph efficacy on secondary OIs
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffSecOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->secondaryOIEfficacy[j]) );

			// read in proph resist prob, level of proph resistance, time of proph resistance,
			// cost factor of proph resistance, and death rate ratio for proph resistance for the secondary proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Resist", inputFile );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->monthlyProbResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->percentResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->timeOfResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->costFactorResistance) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->deathRateRatioResistance) );

			// read in min & maj tox for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Tox", inputFile );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->probMinorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->probMajorToxicity) );
			fscanf( inputFile, "%d", &(prophsInputs[PROPH_SECONDARY][k][i]->monthsToToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->deathRateRatioMajorToxicity) );

			// read in costs and QOL for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "CostQOL", inputFile );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->costMonthly) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->costMinorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->QOLMinorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->costMajorToxicity) );
			fscanf( inputFile, "%lf", &(prophsInputs[PROPH_SECONDARY][k][i]->QOLMajorToxicity) );

			// read in proph switching inputs
			readAndSkipPast(scratch, inputFile);
			readAndSkipPast("Switch", inputFile);
			fscanf( inputFile, "%d", &(prophsInputs[PROPH_SECONDARY][k][i]->monthsToSwitch) );
			fscanf( inputFile, "%d", &tempBool);
			prophsInputs[PROPH_SECONDARY][k][i]->switchOnMinorToxicity = (bool) tempBool;
			fscanf( inputFile, "%d", &tempBool);
			prophsInputs[PROPH_SECONDARY][k][i]->switchOnMajorToxicity = (bool) tempBool;
		}
	}
} /* end readProphInputs */

/* readARTInputs reads data from the ARTs tab of the input sheet */
void SimContext::readARTInputs() {
	char tmpBuf[256], buffer[256];
	int i, j, k, tempBool, tempInt;
	double tempCost;
	FILE *file = inputFile;

	for (int artNum = 1; artNum <= ART_NUM_LINES; artNum++) {
		// read in regimen id num and name
		sprintf(tmpBuf, "ART%dId", artNum);
		readAndSkipPast( tmpBuf, file );
		int idNum;
		fscanf( file, " %d", &idNum );
		// skip to next regimen if this one is not specified
		if (idNum == NOT_APPL) {
			artInputs[artNum - 1] = NULL;
			continue;
		}
		// create new regimen input structure
		artInputs[artNum - 1] = new ARTInputs();
		ARTInputs &artInput = *(artInputs[artNum - 1]);

		// read in one-time startup cost
		sprintf(tmpBuf, "ART%dInitCost", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &artInput.costInitial );
		// read in additional cost from the treatment tab and add to initial cost
		sprintf(tmpBuf, "ART%dInitCostAdd", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &tempCost );
		artInput.costInitial += tempCost;
		// read in monthly cost
		sprintf(tmpBuf, "ART%dMthCost", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &artInput.costMonthly );
		// read in additional cost from the treatment tab and add to monthly cost
		sprintf(tmpBuf, "ART%dMthCostAdd", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &tempCost );
		artInput.costMonthly += tempCost;

		// read in efficacy time horizon
		sprintf(tmpBuf, "ART%dEffTimeHorizon", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &artInput.efficacyTimeHorizon );

		sprintf(tmpBuf, "ART%dResuppEffTimeHorizon", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &artInput.efficacyTimeHorizonResuppression );

		// read in mth by which all would fail
		sprintf(tmpBuf, "ART%dMthForceFail", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &artInput.forceFailAtMonth );

		// read in CD4 effect on ART
		sprintf(tmpBuf, "ART%dMthStageCD4Eff_Succ", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d %d", &(artInput.stageBoundsCD4ChangeOnSuppART[0]), &(artInput.stageBoundsCD4ChangeOnSuppART[1]));
		for (j = 0; j < CD4_RESPONSE_NUM_TYPES; j++) {
			sprintf(tmpBuf, "ART%dCD4EffSlope_Succ", artNum);
			readAndSkipPast2( tmpBuf, CD4_RESPONSE_STRS[j], file );
			fscanf( file, "%lf %lf %lf %lf %lf %lf",
				&(artInput.CD4ChangeOnSuppARTMean[j][0]), &(artInput.CD4ChangeOnSuppARTStdDev[j][0]),
				&(artInput.CD4ChangeOnSuppARTMean[j][1]), &(artInput.CD4ChangeOnSuppARTStdDev[j][1]),
				&(artInput.CD4ChangeOnSuppARTMean[j][2]), &(artInput.CD4ChangeOnSuppARTStdDev[j][2]));
		}
		sprintf(tmpBuf, "ART%dMthStageCD4Eff_Fail", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(artInput.stageBoundCD4ChangeOnARTFail));
		for (j = 0; j < CD4_RESPONSE_NUM_TYPES; j++) {
			sprintf(tmpBuf, "ART%dCD4EffMult_Fail", artNum);
			readAndSkipPast2( tmpBuf, CD4_RESPONSE_STRS[j], file);
			fscanf( file, "%lf %lf",
				&(artInput.CD4MultiplierOnFailedART[j][0]),
				&(artInput.CD4MultiplierOnFailedART[j][1]));
		}
		sprintf(tmpBuf, "ART%dMthCD4SecStdDev", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &artInput.secondaryCD4ChangeOnARTStdDev);

		// read in CD4 effect off ART
		sprintf(tmpBuf, "ART%dCD4EffOffART_Succ", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(artInput.monthlyCD4MultiplierOffARTPreSetpoint[ART_EFF_SUCCESS]),
			&(artInput.monthlyCD4MultiplierOffARTPostSetpoint[ART_EFF_SUCCESS]) );
		sprintf(tmpBuf, "ART%dCD4EffOffART_Fail", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(artInput.monthlyCD4MultiplierOffARTPreSetpoint[ART_EFF_FAILURE]),
			&(artInput.monthlyCD4MultiplierOffARTPostSetpoint[ART_EFF_FAILURE]) );

		// read in HVL change probability
		sprintf(tmpBuf, "ART%dHVLChgProb", artNum);
		readAndSkipPast( tmpBuf, file );
		readAndSkipPast( "Supp", file );
		fscanf( file, "%lf %d", &(artInput.monthlyProbHVLChange[ART_EFF_SUCCESS]), &(artInput.monthlyNumStrataHVLChange[ART_EFF_SUCCESS]) );
		readAndSkipPast( tmpBuf, file );
		readAndSkipPast( "Fail", file );
		fscanf( file, "%lf %d", &(artInput.monthlyProbHVLChange[ART_EFF_FAILURE]), &(artInput.monthlyNumStrataHVLChange[ART_EFF_FAILURE]) );

		//read in toxicity structure
		for (i = 0; i < ART_NUM_SUBREGIMENS; i++) {
			for (j = 0; j < ART_NUM_TOX_SEVERITY; j++) {
				for (k = 0; k < ART_NUM_TOX_PER_SEVERITY; k++) {
					sprintf(tmpBuf, "ART%dToxicity1.%d",artNum, i);
					readAndSkipPast( tmpBuf, file );
					//read in name of toxicity
					fscanf( file, "%32s", buffer);
					artInput.toxicity[i][j][k].toxicityName = buffer;
					//these parameters are shared by all toxicities
					int qolDur, costDur;
					fscanf( file, "%lf %lf %lf %lf %d %lf %d %d",
						&(artInput.toxicity[i][j][k].probToxicity),
						&(artInput.toxicity[i][j][k].timeToToxicityMean),
						&(artInput.toxicity[i][j][k].timeToToxicityStdDev),
						&(artInput.toxicity[i][j][k].QOLModifier),
						&(qolDur),
						&(artInput.toxicity[i][j][k].costAmount),
						&(costDur),
						&(artInput.toxicity[i][j][k].switchSubRegimenOnToxicity));
					artInput.toxicity[i][j][k].QOLDuration = (ART_TOX_DUR) qolDur;
					artInput.toxicity[i][j][k].costDuration = (ART_TOX_DUR) costDur;
					//depending in the severity if the toxicity, there might be extra parameters
					if (j == ART_TOX_CHRONIC) {
						int deathDur;
						fscanf( file, "%d %d %lf %d",
                            &(tempInt),
							&(artInput.toxicity[i][j][k].timeToChronicDeathImpact),
							&(artInput.toxicity[i][j][k].chronicToxDeathRateRatio),
							&(deathDur));
						if (tempInt != NOT_APPL)
                            tempInt--;
                        artInput.toxicity[i][j][k].switchARTRegimenOnToxicity = tempInt;
						artInput.toxicity[i][j][k].chronicDeathDuration = (ART_TOX_DUR) deathDur;
					}
					else if (j == ART_TOX_MAJOR) {
						fscanf( file, "%lf %lf",
							 &(artInput.toxicity[i][j][k].acuteMajorToxDeathRateRatio),
							 &(artInput.toxicity[i][j][k].costAcuteDeathMajorToxicity));
					}
				}
			}
			sprintf(tmpBuf, "ART%dToxicity1.%d",artNum, i);
			readAndSkipPast( tmpBuf, file );
			readAndSkipPast("TimeToSwitch", file);
			fscanf( file, "%d",&(artInput.monthsToSwitchSubRegimen[i]));
		}

		//read regimen specific heterogeneity inputs
		sprintf(tmpBuf, "ART%dPropMthCostNonResponders", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf", &(artInput.propMthCostNonResponders));

		sprintf(tmpBuf, "ART%dProbRestartRegimen", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf %lf", &(artInput.probRestartARTRegimenAfterFailure[RESP_TYPE_FULL]),&(artInput.probRestartARTRegimenAfterFailure[RESP_TYPE_PARTIAL]),&(artInput.probRestartARTRegimenAfterFailure[RESP_TYPE_NON]));

		sprintf(tmpBuf, "ART%dMaxResuppAttempts", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%d", &(artInput.maxRestartAttempts));

		sprintf(tmpBuf, "ART%dHetPropRespRegCoeff", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf", &(artInput.propRespondARTRegimenLogitMean), &(artInput.propRespondARTRegimenLogitStdDev));
		sprintf(tmpBuf, "ART%dHetPropRespRegCoeffDistribution", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%d", &(artInput.propRespondARTRegimenLogitDistribution));
		sprintf(tmpBuf, "ART%dHetPropRespRegCoeffUseDuration", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%d", &tempBool);
		artInput.propRespondARTRegimenUseDuration = (bool) tempBool;
		sprintf(tmpBuf, "ART%dHetPropRespRegCoeffDuration", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf", &(artInput.propRespondARTRegimenDurationMean), &(artInput.propRespondARTRegimenDurationStdDev));

		sprintf(tmpBuf, "ART%dHetOutcomes", artNum);
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Supp",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_SUPP][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_SUPP][1]),&(artInput.responseTypeValues[HET_OUTCOME_SUPP][0]),&(artInput.responseTypeValues[HET_OUTCOME_SUPP][1]), &(artInput.responseTypeExponents[HET_OUTCOME_SUPP]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("LateFail",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_LATEFAIL][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_LATEFAIL][1]),&(artInput.responseTypeValues[HET_OUTCOME_LATEFAIL][0]),&(artInput.responseTypeValues[HET_OUTCOME_LATEFAIL][1]), &(artInput.responseTypeExponents[HET_OUTCOME_LATEFAIL]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectOI",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_OI][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_OI][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectCHRMs",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_CHRMS][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_CHRMS][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectMort",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_MORT][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_MORT][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Resist",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_RESIST][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_RESIST][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Tox",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_TOX][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_TOX][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Cost",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_COST][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_COST][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("RestartAfterFail",file);
		fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_RESTART][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_RESTART][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Resuppression",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_RESUPP][0]), &(artInput.responseTypeThresholds[HET_OUTCOME_RESUPP][1]),&(artInput.responseTypeValues[HET_OUTCOME_RESUPP][0]),&(artInput.responseTypeValues[HET_OUTCOME_RESUPP][1]), &(artInput.responseTypeExponents[HET_OUTCOME_RESUPP]));
        sprintf(tmpBuf, "ART%dARTEffectOnFail", artNum);
        readAndSkipPast(tmpBuf, file);
        fscanf(file, "%d", &tempBool);
        artInput.applyARTEffectOnFailed = (bool) tempBool;

	}
} /* end readARTInputs */

/* readNatHistInputs reads data from the NatHist tab of the input sheet */
void SimContext::readNatHistInputs() {
	int i, j;
	// read in death rate ratios for HIV and ART
	readAndSkipPast( "HIVDthRateRatio", inputFile );
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf(inputFile, "%lf", &(natHistInputs.HIVDeathRateRatio[i]));
	readAndSkipPast("ARTDthRateRatio", inputFile);
	fscanf(inputFile, "%lf", &(natHistInputs.ARTDeathRateRatio));
	// read in acute OI prob w/ no OI hist, not on ART
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j ) {
		readAndSkipPast( "OIProb_NoHist_noART", inputFile );
		readAndSkipPast( CD4_STRATA_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%lf", &(natHistInputs.monthlyOIProbOffART[j][i][HIST_N]) );
	}
	// read in acute OI prob w/ OI hist, not on ART
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j ) {
		readAndSkipPast( "OIProb_Hist_noART", inputFile );
		readAndSkipPast( CD4_STRATA_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%lf", &(natHistInputs.monthlyOIProbOffART[j][i][HIST_Y]) );
	}
	// read in acute OI prob on ART multipliers
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j ) {
		readAndSkipPast( "OIProb_onART_Mult", inputFile );
		readAndSkipPast( CD4_STRATA_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%lf", &(natHistInputs.monthlyOIProbOnARTMult[j][i]) );
	}
	// read in death rate ratios for acute cases of severe OIs and TB modeled as a severe OI
	readAndSkipPast( "AcuteOIDthRateRatio", inputFile );
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf( inputFile, "%lf", &(natHistInputs.acuteOIDeathRateRatio[i]));
	readAndSkipPast( "AcuteOIDthRateRatioTB", inputFile );
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf( inputFile, "%lf", &(natHistInputs.acuteOIDeathRateRatioTB[i]));
	// read in death rate ratios for having a history of a severe OI or TB modeled as a severe OI, and their durations
	readAndSkipPast( "SevrOI_HistDthRateRatio", inputFile);
	fscanf( inputFile, "%lf", &(natHistInputs.severeOIHistDeathRateRatio));
	readAndSkipPast( "SevrOI_HistEffectDuration", inputFile);
	fscanf( inputFile, "%d", &(natHistInputs.severeOIHistEffectDuration));
	readAndSkipPast( "TB_OI_HistDthRateRatio", inputFile);
	fscanf( inputFile, "%lf", &(natHistInputs.TB_OIHistDeathRateRatio));
	readAndSkipPast( "TB_OI_HistEffectDuration", inputFile);
	fscanf( inputFile, "%d", &(natHistInputs.TB_OIHistEffectDuration));
	// read in generic risk factor death rate ratios
	readAndSkipPast( "GenRiskDthRateRatio", inputFile);
	for( i = 0; i<RISK_FACT_NUM; ++i)
		fscanf( inputFile, "%lf", &(natHistInputs.genericRiskDeathRateRatio[i]));
	// read in nat hist (Mellors) decl
	for ( j = CD4_NUM_STRATA - 1; j >= 0; --j ) {
		readAndSkipPast( "BslCD4Decl_Mean", inputFile );
		readAndSkipPast( CD4_STRATA_STRS[j], inputFile );
		for ( i = HVL_NUM_STRATA - 1; i >= HVL_VLO; --i )
			fscanf( inputFile, "%lf", &(natHistInputs.monthlyCD4DeclineMean[j][i]) );
		readAndSkipPast( "BslCD4Decl_SDev", inputFile );
		readAndSkipPast( CD4_STRATA_STRS[j], inputFile );
		for ( i = HVL_NUM_STRATA - 1; i >= HVL_VLO; --i )
			fscanf( inputFile, "%lf", &(natHistInputs.monthlyCD4DeclineStdDev[j][i]) );
	}

	readAndSkipPast( "BslCD4Decl_BtwSbjct", inputFile);
	//read in between subject cd4 decline.  This std dev is used to draw between subject cd4 decline for each patient once per run
	fscanf( inputFile, "%lf", &(natHistInputs.monthlyCD4DeclineBtwSubject) );

	// read in background death rates
	readAndSkipPast( "BackgroundDthRate_Male", inputFile );
	for ( i = AGE_STARTING; i < AGE_YRS; ++i )
		fscanf( inputFile, "%lf", &(natHistInputs.monthlyBackgroundDeathRate[GENDER_MALE][i]) );
	readAndSkipPast( "BackgroundDthRate_Female", inputFile );
	for ( i = AGE_STARTING; i < AGE_YRS; ++i )
		fscanf( inputFile, "%lf", &(natHistInputs.monthlyBackgroundDeathRate[GENDER_FEMALE][i]) );
	// read in the background mortality modifier type
	readAndSkipPast( "BackgroundMortModifierType", inputFile) ;
	fscanf(inputFile, "%d", &(natHistInputs.backgroundMortModifierType));
	// read in background mortality modifier
    readAndSkipPast( "BackgroundMortModifier", inputFile) ;
    fscanf( inputFile, "%lf", &(natHistInputs.backgroundMortModifier) );
} /* end readNatHistInputs */

/* readCHRMsInputs reads data from the CHRMs tab of the input sheet */
void SimContext::readCHRMsInputs() {
	char scratch[256];
	int tempBool;

	// Read in CHRMs names
	readAndSkipPast("CHRMstrs", inputFile);
	for (int i = 0; i < CHRM_NUM; i++) {
		fscanf( inputFile, "%32s", CHRM_STRS[i] );
		strcpy(DTH_CAUSES_STRS[DTH_CHRM_1 + i], CHRM_STRS[i]);
	}

	//Output CHRMs output
	readAndSkipPast("ShowCHRMOutput", inputFile);
	fscanf(inputFile,"%d", &tempBool);
	chrmsInputs.showCHRMsOutput = (bool) tempBool;

	//Orphans input
	readAndSkipPast("EnableOrphans", inputFile);
	fscanf(inputFile,"%d", &tempBool);
	chrmsInputs.enableOrphans = (bool) tempBool;

	readAndSkipPast("ShowOrphansOutput", inputFile);
	fscanf(inputFile,"%d", &tempBool);
	chrmsInputs.showOrphansOutput = (bool) tempBool;

	//CHRMs Age Bounds
	for (int i = 0; i < CHRM_NUM; i++) {
		sprintf(scratch, "CHRMAgeCat%s", CHRM_STRS[i]);
		readAndSkipPast(scratch, inputFile);
		for (int l = 0; l < CHRM_AGE_CAT_NUM-1; l++)
			fscanf(inputFile, "%d", &(chrmsInputs.ageBounds[i][l]));
	}
	
	// CHRMs stage durations and std. deviations
	for (int i = 0; i < CHRM_TIME_PER_NUM-1; i++){
        for (int j = 0; j < CHRM_NUM; j++){
            sprintf(scratch, "Duration%dandSD%s", i, CHRM_STRS[j]);
            readAndSkipPast(scratch, inputFile);
            for (int k = 0; k < 2; k++){
                fscanf( inputFile, "%lf", &(chrmsInputs.durationCHRMSstage[i][j][k]));
            }
        }
	}

    // Square root transformation settings
	readAndSkipPast("CHRMDurationUseSqrtTrans", inputFile);
	fscanf(inputFile,"%d", &tempBool);
	chrmsInputs.enableCHRMSDurationSqrtTransform = (bool) tempBool;

	// Read in probability of prevalent CHRMs, modifiers, and months since start
	for (int i = 0; i < CHRM_NUM; i++) {
		sprintf(scratch, "ProbPrevCHRM_%s", CHRM_STRS[i]);
		readAndSkipPast2(scratch, "HIVneg", inputFile);
		for (int k = 0; k < GENDER_NUM; k++) {
			for (int l = 0; l < CHRM_AGE_CAT_NUM; l++) {
				fscanf(inputFile, "%lf", &(chrmsInputs.probPrevalentCHRMsHIVneg[i][k][l]));
			}
		}
		for (int j = CD4_NUM_STRATA - 1; j >= 0; j--) {
			readAndSkipPast2(scratch, CD4_STRATA_STRS[j], inputFile);
			for (int k = 0; k < GENDER_NUM; k++) {
				for (int l = 0; l < CHRM_AGE_CAT_NUM; l++) {
					fscanf(inputFile, "%lf", &(chrmsInputs.probPrevalentCHRMs[i][j][k][l]));
				}
			}
		}
	}
	for (int i = 0; i < CHRM_NUM; i++) {
		readAndSkipPast2("PrevCHRMRiskLogit", CHRM_STRS[i], inputFile);
		for (int j = 0; j < RISK_FACT_NUM; j++) {
			fscanf(inputFile, "%lf", &(chrmsInputs.probPrevalentCHRMsRiskFactorLogit[i][j]));
		}
	}
	for (int i = 0; i < CHRM_NUM; i++) {
		readAndSkipPast2("PrevCHRMNumMonths", CHRM_STRS[i], inputFile);
		fscanf(inputFile, "%lf %lf", &(chrmsInputs.prevalentCHRMsMonthsSinceStartMean[i]),
				&(chrmsInputs.prevalentCHRMsMonthsSinceStartStdDev[i]));
	}

	for (int i = 0; i < CHRM_NUM; i++) {
		readAndSkipPast2("PrevCHRMNumMonthsOrphans", CHRM_STRS[i], inputFile);
		for (int j = 0; j < CHRM_ORPHANS_AGE_CAT_NUM; j++)
			fscanf(inputFile, "%d", &(chrmsInputs.prevalentCHRMsMonthsSinceStartOrphans[i][j]));
	}

	// Read in probability of incident CHRMs and modifiers
	readAndSkipPast("MinMthsSincePrevCHRMSOrphans", inputFile);
	fscanf(inputFile,"%d", &(chrmsInputs.incidentCHRMsMonthsSincePreviousOrphans));

	for (int i = 0; i < CHRM_NUM; i++) {
		sprintf(scratch, "ProbIncidCHRM_%s", CHRM_STRS[i]);
		readAndSkipPast2(scratch, "HIVneg", inputFile);
		for (int k = 0; k < GENDER_NUM; k++) {
			for (int l = 0; l < CHRM_AGE_CAT_NUM; l++) {
				fscanf(inputFile, "%lf", &(chrmsInputs.probIncidentCHRMsHIVneg[i][k][l]));
			}
		}
		for (int j = CD4_NUM_STRATA - 1; j >= 0; j--) {
			readAndSkipPast2(scratch, CD4_STRATA_STRS[j], inputFile);
			for (int k = 0; k < GENDER_NUM; k++) {
				for (int l = 0; l < CHRM_AGE_CAT_NUM; l++) {
					fscanf(inputFile, "%lf", &(chrmsInputs.probIncidentCHRMs[i][j][k][l]));
				}
			}
		}
	}
	for (int i = 0; i < CHRM_NUM; i++) {
		readAndSkipPast2("IncidCHRMOnARTMult", CHRM_STRS[i], inputFile);
		for (int j = CD4_NUM_STRATA - 1; j >= 0; j--) {
			fscanf(inputFile, "%lf", &(chrmsInputs.probIncidentCHRMsOnARTMult[i][j]));
		}
	}
	for (int i = 0; i < CHRM_NUM; i++) {
		readAndSkipPast2("IncidCHRMRiskLogit", CHRM_STRS[i], inputFile);
		for (int j = 0; j < RISK_FACT_NUM; j++) {
			fscanf(inputFile, "%lf", &(chrmsInputs.probIncidentCHRMsRiskFactorLogit[i][j]));
		}
	}
	for (int i = 0; i < CHRM_NUM; i++) {
		readAndSkipPast2("IncidCHRMHistoryLogit", CHRM_STRS[i], inputFile);
		for (int j = 0; j < CHRM_NUM; j++) {
			fscanf(inputFile, "%lf", &(chrmsInputs.probIncidentCHRMsPriorHistoryLogit[i][j]));
		}
	}

	// Read in CHRMs death rate ratios for each stage
	for (int i = 0; i < CHRM_NUM; i++) {
		for (int j = 0; j < CHRM_TIME_PER_NUM; j++) {
			sprintf(scratch, "DthRateRatioCHRM_%s_T%d", CHRM_STRS[i], j + 1);
			readAndSkipPast( scratch, inputFile );
				for (int l = 0; l < GENDER_NUM; l++) {
					for (int m = 0; m < CHRM_AGE_CAT_NUM; m++) {
						fscanf(inputFile, "%lf", &(chrmsInputs.CHRMsDeathRateRatio[i][j][l][m]));
				}
			}
		}
	}

	// Read in cost of CHRMs for each stage
	for (int i = 0; i < CHRM_NUM; i++) {
		for (int j = 0; j < CHRM_TIME_PER_NUM; j++) {
			sprintf(scratch, "CostCHRM_%s", CHRM_STRS[i]);
			readAndSkipPast(scratch, inputFile);
			sprintf(scratch, "T%d", j + 1);
			readAndSkipPast(scratch, inputFile);
			for (int k = 0; k < GENDER_NUM; k++) {
				for (int l = 0; l < CHRM_AGE_CAT_NUM; l++) {
					fscanf(inputFile, "%lf", &(chrmsInputs.costCHRMs[i][j][k][l]));
				}
			}
		}
		sprintf(scratch, "CostDeathCHRM_%s", CHRM_STRS[i]);
		readAndSkipPast(scratch, inputFile);
		fscanf(inputFile, "%lf", &(chrmsInputs.costDeathCHRMs[i]));
	}

	// Read in QOL of CHRMs for each stage
	for (int i = 0; i < CHRM_NUM; i++) {
		for (int j = 0; j < CHRM_TIME_PER_NUM; j++) {
			sprintf(scratch, "QOLCHRM_%s", CHRM_STRS[i]);
			readAndSkipPast(scratch, inputFile);
			sprintf(scratch, "T%d", j + 1);
			readAndSkipPast(scratch, inputFile);
			for (int k = 0; k < GENDER_NUM; k++) {
				for (int l = 0; l < CHRM_AGE_CAT_NUM; l++) {
					fscanf(inputFile, "%lf", &(chrmsInputs.QOLModCHRMs[i][j][k][l]));
				}
			}
		}
		sprintf(scratch, "QOLModDeathCHRM_%s", CHRM_STRS[i]);
		readAndSkipPast(scratch, inputFile);
		fscanf(inputFile, "%lf", &(chrmsInputs.QOLModDeathCHRMs[i]));
	}

	// Read in QOL modifiers for multiple CHRMs
	readAndSkipPast("QOLModMultipleCHRMs",inputFile);
	for (int i = 0; i < CHRM_NUM-1; i++) {
		fscanf(inputFile, "%lf", &(chrmsInputs.QOLModMultipleCHRMs[i]));
	}
} /* end readCHRMsInputs */

/* readCostInputs reads data from the Cost tab of the input sheet */
void SimContext::readCostInputs() {
	char tmpBuf[256];
	int i, j, k;

	// read in cost age category bounds
	readAndSkipPast( "CostAgeBounds", inputFile );
	readAndSkipPast( "EndYr", inputFile );
	for (i = 0; i < COST_AGE_CAT_NUM - 1; ++i) {
		fscanf( inputFile, "%d", &(costInputs.costAgeBounds[i]));
	}

	for (int t=1; t <= COST_AGE_CAT_NUM; t++){
	// read in acute OI costs
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostAcuteOI_noART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.acuteOICostTreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostAcuteOI_noART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.acuteOICostUntreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostAcuteOI_onART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.acuteOICostTreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostAcuteOI_onART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.acuteOICostUntreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
		// read in CD4 / HVL test costs
		sprintf(tmpBuf, "AgeCat%dCostCD4Test", t);
		readAndSkipPast( tmpBuf, inputFile );
		for (i = 0; i < COST_NUM_TYPES; i++) {
			fscanf(inputFile, "%lf", &(costInputs.CD4TestCost[t-1][i]) );
		}
		sprintf(tmpBuf, "AgeCat%dCostHVLTest", t);
		readAndSkipPast( tmpBuf, inputFile );
		for (i = 0; i < COST_NUM_TYPES; i++) {
			fscanf(inputFile, "%lf", &(costInputs.HVLTestCost[t-1][i]) );
		}
		// read in death from OI costs
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostDth_noART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.deathCostTreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostDth_noART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.deathCostUntreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostDth_onART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.deathCostTreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "AgeCat%dCostDth_onART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(costInputs.deathCostUntreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
	}
	
	// read in general medicine costs
	readAndSkipPast("CostGenMed_dmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.generalMedicineCost[i][j][COST_DIR_MED]));
		}
	}
	readAndSkipPast("CostGenMed_nmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.generalMedicineCost[i][j][COST_DIR_NONMED]));
		}
	}
	readAndSkipPast("CostGenMed_time", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.generalMedicineCost[i][j][COST_TIME]));
		}
	}
	readAndSkipPast("CostGenMed_indr", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.generalMedicineCost[i][j][COST_INDIR]));
		}
	}
	// read in routine care costs for HIV positive, not on ART
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("CostRoutine_HIVpos_noART_dmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_DIR_MED]));
			}
		}
		readAndSkipPast2("CostRoutine_HIVpos_noART_nmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_DIR_NONMED]));
			}
		}
		readAndSkipPast2("CostRoutine_HIVpos_noART_time", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_TIME]));
			}
		}
		readAndSkipPast2("CostRoutine_HIVpos_noART_indr", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_INDIR]));
			}
		}
	}
	// read in routine care costs for HIV positive, on ART
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("CostRoutine_HIVpos_onART_dmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_DIR_MED]));
			}
		}
		readAndSkipPast2("CostRoutine_HIVpos_onART_nmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_DIR_NONMED]));
			}
		}
		readAndSkipPast2("CostRoutine_HIVpos_onART_time", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_TIME]));
			}
		}
		readAndSkipPast2("CostRoutine_HIVpos_onART_indr", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_INDIR]));
			}
		}
	}

	//Routine care costs hiv neg and undetected pos
	readAndSkipPast("CostNeg_dmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVNegative[i][j][COST_DIR_MED]));
		}
	}
	readAndSkipPast("CostNeg_nmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVNegative[i][j][COST_DIR_NONMED]));
		}
	}
	readAndSkipPast("CostNeg_time", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVNegative[i][j][COST_TIME]));
		}
	}
	readAndSkipPast("CostNeg_indr", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVNegative[i][j][COST_INDIR]));
		}
	}
	readAndSkipPast("CostNegStopAge", inputFile);
	fscanf(inputFile, "%d", &(costInputs.routineCareCostHIVNegativeStopAge));

	readAndSkipPast("CostUndet_dmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositiveUndetected[i][j][COST_DIR_MED]));
		}
	}
	readAndSkipPast("CostUndet_nmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositiveUndetected[i][j][COST_DIR_NONMED]));
		}
	}
	readAndSkipPast("CostUndet_time", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositiveUndetected[i][j][COST_TIME]));
		}
	}
	readAndSkipPast("CostUndet_indr", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(costInputs.routineCareCostHIVPositiveUndetected[i][j][COST_INDIR]));
		}
	}
	readAndSkipPast("CostUndetStopAge", inputFile);
	fscanf(inputFile, "%d", &(costInputs.routineCareCostHIVPositiveUndetectedStopAge));

	// read in contact/clinic vist costs
	for (int t=1; t <= COST_AGE_CAT_NUM; t++){
		for ( i = 0; i < GENDER_NUM; ++i) {
			for ( j = CD4_NUM_STRATA - 1; j >= 0; j-- ) {
				sprintf(tmpBuf, "AgeCat%dCostVisit_%s_routine", t, GENDER_STRS[i]);
				readAndSkipPast2( tmpBuf, CD4_STRATA_STRS[j], inputFile );
				for (k = 0; k < COST_NUM_TYPES; k++) {
					fscanf(inputFile, "%lf", &(costInputs.clinicVisitCostRoutine[t-1][i][j][k]) );
				}
			}
		}
	}	
} /* end readCostInputs */

/* readTBInputs reads data from the TB tab of the input sheet */
void SimContext::readTBInputs() {
	char tmpBuf[256], tmpBuf2[256];
	int i, j, k, tempBool, tempInt;
	FILE *file = inputFile;

	// read in enable tb toggle
	readAndSkipPast("EnableTB", inputFile);
	fscanf( file, "%d", &tempBool);
	tbInputs.enableTB = (bool) tempBool;

	readAndSkipPast("TBClinicIntegrated", inputFile);
	fscanf( file, "%d", &tempBool);
	tbInputs.isIntegrated = (bool) tempBool;

	// read in TB distribution at entry
	readAndSkipPast2( "ProbTB_Entry", "HIV_Neg", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.distributionTBStateAtEntryHIVNeg[j]));

	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "ProbTB_Entry", CD4_STRATA_STRS[i], file );
		for (j = 0; j < TB_NUM_STATES; j++)
			fscanf( file, "%lf", &(tbInputs.distributionTBStateAtEntryHIVPos[i][j]));
	}

	//Read in tb strain at entry
	readAndSkipPast( "DistTB_Entry_Strain", file );
	for (i = 0; i < TB_NUM_STRAINS; ++i)
		fscanf( file, "%lf", &(tbInputs.distributionTBStrainAtEntry[i]));

	readAndSkipPast( "MthsSinceInitTBTreatStop_Entry", file );
	fscanf( file, "%lf %lf",  &(tbInputs.monthsSinceInitTreatStopMean),  &(tbInputs.monthsSinceInitTreatStopStdDev));

	//Read in tracker variables
	readAndSkipPast2( "ProbSputum_Entry", "HIV_Neg", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.distributionTBTrackerAtEntryHIVNeg[TB_TRACKER_SPUTUM_HI][j]));

	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "ProbSputum_Entry", CD4_STRATA_STRS[i], file );
		for (j = 0; j < TB_NUM_STATES; j++)
			fscanf( file, "%lf", &(tbInputs.distributionTBTrackerAtEntryHIVPos[i][TB_TRACKER_SPUTUM_HI][j]));
	}

	readAndSkipPast2( "ProbImmune_Entry", "HIV_Neg", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.distributionTBTrackerAtEntryHIVNeg[TB_TRACKER_IMMUNE_REACTIVE][j]));

	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "ProbImmune_Entry", CD4_STRATA_STRS[i], file );
		for (j = 0; j < TB_NUM_STATES; j++)
			fscanf( file, "%lf", &(tbInputs.distributionTBTrackerAtEntryHIVPos[i][TB_TRACKER_IMMUNE_REACTIVE][j]));
	}

	readAndSkipPast2( "ProbSymptoms_Entry", "HIV_Neg", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.distributionTBTrackerAtEntryHIVNeg[TB_TRACKER_SYMPTOMS][j]));

	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "ProbSymptoms_Entry", CD4_STRATA_STRS[i], file );
		for (j = 0; j < TB_NUM_STATES; j++)
			fscanf( file, "%lf", &(tbInputs.distributionTBTrackerAtEntryHIVPos[i][TB_TRACKER_SYMPTOMS][j]));
	}

	// nat hist
	//Prob infection or reinfection
	readAndSkipPast2( "ProbMthTBIncid", "HIV_Neg", file );
	for (j = 0; j < TB_INFECT_NUM_AGE_CAT; j++)
		fscanf( file, "%lf", &(tbInputs.probInfectionHIVNeg[j]));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "ProbMthTBIncid", CD4_STRATA_STRS[i], file );
		for (j = 0; j < TB_INFECT_NUM_AGE_CAT; j++)
			fscanf( file, "%lf", &(tbInputs.probInfectionHIVPos[i][j]));
	}

	readAndSkipPast( "TBInfectionMultiplier", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.infectionMultiplier[j]));

	//prob immune reactive on infection
	readAndSkipPast( "ProbImmune_Infection", file );
	fscanf( file, "%lf", &(tbInputs.probImmuneReactiveOnInfectionHIVNeg));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probImmuneReactiveOnInfectionHIVPos[i]));
	}

	//Nat Hist Activation
	readAndSkipPast( "TBActivationMthThreshold", file );
	fscanf( file, "%d", &(tbInputs.probActivateMthThreshold));

	readAndSkipPast( "TBActivationProbActivate1", file );
	fscanf( file, "%lf", &(tbInputs.probActivateHIVNeg[0]));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probActivateHIVPos[0][i]));
	}

	readAndSkipPast( "TBActivationProbActivate2", file );
	fscanf( file, "%lf", &(tbInputs.probActivateHIVNeg[1]));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probActivateHIVPos[1][i]));
	}

	readAndSkipPast( "TBActivationPropPulm", file );
	fscanf( file, "%lf", &(tbInputs.probPulmonaryOnActivationHIVNeg));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probPulmonaryOnActivationHIVPos[i]));
	}

	readAndSkipPast( "ProbSputumHiOnActivation", file );
	fscanf( file, "%lf", &(tbInputs.probSputumHiOnActivationPulmHIVNeg));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probSputumHiOnActivationPulmHIVPos[i]));
	}


	//Nat Hist Relapse
	readAndSkipPast( "TBIncidenceRelapse", file );
	fscanf( file, "%lf %lf %d %d", &(tbInputs.probRelapseTtoARateMultiplier), &(tbInputs.probRelapseTtoAExponent), &(tbInputs.probRelapseTtoAThreshold),&(tbInputs.probRelapseEffHorizon));

	readAndSkipPast( "TBRelapseFCD4", file );
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probRelapseTtoAFCD4[i]));
	}
	readAndSkipPast( "TBTreatDefaultRelapseMult", file);
	fscanf( file, "%lf", &(tbInputs.relapseRateMultTBTreatDefault));

	readAndSkipPast( "TBRelapsePropPulm", file );
	fscanf( file, "%lf", &(tbInputs.probPulmonaryOnRelapseHIVNeg));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probPulmonaryOnRelapseHIVPos[i]));
	}

	readAndSkipPast( "ProbSputumHiOnRelapse", file );
	fscanf( file, "%lf", &(tbInputs.probSputumHiOnRelapsePulmHIVNeg));
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		fscanf( file, "%lf", &(tbInputs.probSputumHiOnRelapsePulmHIVPos[i]));
	}

	readAndSkipPast( "TBRelapseMthUnfavorableOutcome", file );
	fscanf( file, "%d", &(tbInputs.monthRelapseUnfavorableOutcome));

	// read in TB Sypmtoms monthly incidence
	readAndSkipPast2( "TBSymptomsIncidence", "HIV_Neg", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.probTBSymptomsMonthHIVNeg[j]));

	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "TBSymptomsIncidence", CD4_STRATA_STRS[i], file );
		for (j = 0; j < TB_NUM_STATES; j++)
			fscanf( file, "%lf", &(tbInputs.probTBSymptomsMonthHIVPos[i][j]));
	}

	// read in death rate ratios for active TB 
	readAndSkipPast2( "TBDthRateRatio", "HIV_Neg", file );
	fscanf( file, "%lf %lf", &(tbInputs.TBDeathRateRatioActivePulmHIVneg),&(tbInputs.TBDeathRateRatioExtraPulmHIVneg) );
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "TBDthRateRatio", CD4_STRATA_STRS[i], file );
		fscanf( file, "%lf %lf", &(tbInputs.TBDeathRateRatioActivePulmHIVPos[i]), &(tbInputs.TBDeathRateRatioExtraPulmHIVPos[i]));
	}

	// read in death rate ratios and stage bounds for TB treatment success and failure
	readAndSkipPast("TBDthRateRatioTxSuccessBounds", inputFile);
	for ( i = 0; i < 2; i++){
		fscanf( inputFile, "%d", &(tbInputs.TBDeathRateRatioTxSuccessBounds[i]));
	}
	for( i = 0; i < 3; i++ ){
		sprintf(tmpBuf, "TBDthRateRatioTxSuccessHIVNeg_T%d", i + 1);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf( file, "%lf", &(tbInputs.TBDeathRateRatioTreatmentSuccessHIVNeg[i]));
		sprintf(tmpBuf, "TBDthRateRatioTxSuccessHIVPos_T%d", i + 1);	
		readAndSkipPast(tmpBuf, inputFile);
		for ( j = CD4_NUM_STRATA - 1; j >= 0; --j){
			fscanf( inputFile, "%lf", &(tbInputs.TBDeathRateRatioTreatmentSuccessHIVPos[i][j]));
		}
	}
	
	readAndSkipPast("TBDthRateRatioTxFailureBounds", inputFile);
	for ( i = 0; i < 2; i++){
		fscanf( inputFile, "%d", &(tbInputs.TBDeathRateRatioTxFailureBounds[i]));
	}
	for (i = 0; i < 3; i++){
		sprintf(tmpBuf, "TBDthRateRatioTxFailureHIVNeg_T%d", i + 1);
		readAndSkipPast(tmpBuf, inputFile);
		fscanf( file, "%lf", &(tbInputs.TBDeathRateRatioTreatmentFailureHIVNeg[i]));
		sprintf(tmpBuf, "TBDthRateRatioTxFailureHIVPos_T%d", i + 1);
		readAndSkipPast(tmpBuf, inputFile);
		for ( j = CD4_NUM_STRATA - 1; j >= 0; --j){
			fscanf( inputFile, "%lf", &(tbInputs.TBDeathRateRatioTreatmentFailureHIVPos[i][j]));
		}
	}
	
	// read in natural history multipliers by calendar month
	readAndSkipPast( "NatHistMultType", file );
	fscanf( file, "%d", &tempInt);
	tbInputs.natHistMultType = (TB_MULT_TYPE) tempInt;
	
	readAndSkipPast( "NatHistMultTimeBounds", file );
	fscanf( file, "%d %d", &(tbInputs.natHistMultTimeBounds[0]), &(tbInputs.natHistMultTimeBounds[1]));

	readAndSkipPast( "NatHistMultTime", file );
	fscanf( file, "%lf %lf %lf", &(tbInputs.natHistMultTime[0]), &(tbInputs.natHistMultTime[1]),&(tbInputs.natHistMultTime[2]));

	// self cure inputs
	readAndSkipPast( "TBSelfCureEnable", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.enableSelfCure = (bool) tempBool;

	readAndSkipPast( "TBMonthOfSelfCure", file );
	fscanf( file, "%d", &(tbInputs.selfCureTime));

	// prophylaxis policy for TB
	readAndSkipPast( "TBProphOrder", file );
	for(i = 0; i < TB_NUM_PROPHS; i++)
		fscanf( file, "%d", &(tbInputs.prophOrder[i]));

	readAndSkipPast( "TBProphDuration", file );
	for(i = 0; i < TB_NUM_PROPHS; i++)
		fscanf( file, "%d", &(tbInputs.prophDuration[i]));
	readAndSkipPast( "TBProphMaxRestarts", file );
	for(i = 0; i < TB_NUM_PROPHS; i++)
		fscanf( file, "%d", &(tbInputs.maxRestarts[i]));

	readAndSkipPast( "TBProphStartKnownPos", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.startProphUseOrEvaluationKnownPos = (bool) tempBool;
	fscanf( file, "%d", &tempBool);
	tbInputs.startProphAllKnownPos = (bool) tempBool;
	fscanf( file, "%lf %lf", &(tbInputs.startProphObservedCD4Bounds[UPPER_BOUND]),
			&(tbInputs.startProphObservedCD4Bounds[LOWER_BOUND]));
	fscanf( file, "%d", &(tbInputs.startProphARTStatusKnownPos));
	fscanf( file, "%d", &(tbInputs.startProphHistTBDiagKnownPos));
	fscanf( file, "%d", &(tbInputs.startProphHistTreatmentKnownPos));
	fscanf( file, "%d", &(tbInputs.startProphImmuneReactiveKnownPos));

	readAndSkipPast( "TBProphStartNotKnownPos", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.startProphUseOrEvaluationNotKnownPos = (bool) tempBool;
	fscanf( file, "%d", &tempBool);
	tbInputs.startProphAllNotKnownPos = (bool) tempBool;
	fscanf( file, "%d", &(tbInputs.startProphHistTBDiagNotKnownPos));
	fscanf( file, "%d", &(tbInputs.startProphHistTreatmentNotKnownPos));
	fscanf( file, "%d", &(tbInputs.startProphImmuneReactiveNotKnownPos));

	readAndSkipPast( "TBProphStartPropToReceiveUponQual", file);
	fscanf(file, "%lf %lf %lf", &(tbInputs.probReceiveProphNotKnownPos), &(tbInputs.probReceiveProphKnownPosOffART),&(tbInputs.probReceiveProphOnART));
	readAndSkipPast( "TBProphStartLagToStart", file);
	fscanf(file, "%lf %lf", &(tbInputs.monthsLagToStartProphMean), &(tbInputs.monthsLagToStartProphStdDev));
	readAndSkipPast( "TBProphStopMthDropoutProb", file);
	fscanf(file, "%lf", &(tbInputs.probDropoffProph));

	readAndSkipPast( "TBProphStopKnownPos", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.stopProphUseOrEvaluationKnownPos = (bool) tempBool;
	fscanf( file, "%lf %lf", &(tbInputs.stopProphObservedCD4Bounds[UPPER_BOUND]),
			&(tbInputs.stopProphObservedCD4Bounds[LOWER_BOUND]));
	fscanf( file, "%d", &(tbInputs.stopProphNumMonthsKnownPos));
	fscanf( file, "%d", &tempBool);
	tbInputs.stopProphAfterTBDiagKnownPos = (bool) tempBool;
	fscanf( file, "%d", &tempBool);
	tbInputs.stopProphMajorToxKnownPos = (bool) tempBool;

	readAndSkipPast( "TBProphStopNotKnownPos", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.stopProphUseOrEvaluationNotKnownPos = (bool) tempBool;
	fscanf( file, "%d", &(tbInputs.stopProphNumMonthsNotKnownPos));
	fscanf( file, "%d", &tempBool);
	tbInputs.stopProphAfterTBDiagNotKnownPos = (bool) tempBool;
	fscanf( file, "%d", &tempBool);
	tbInputs.stopProphMajorToxNotKnownPos = (bool) tempBool;

	readAndSkipPast( "TBProphMoveToNextAfterTox",file);
	fscanf( file, "%d", &tempBool);
	tbInputs.moveToNextProphAfterTox = (bool) tempBool;

	//prophylaxis efficacy and cost for TB
	for (int prophNum = 0; prophNum < TB_NUM_PROPHS; ++prophNum) {
		TBInputs::TBProph &tbProph = tbInputs.tbProphInputs[prophNum];

		sprintf(tmpBuf, "OnTBProph%dEffInfect", prophNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbProph.efficacyInfectionHIVNeg[0]));

		for (j = CD4_NUM_STRATA - 1; j >= 0; --j){
			fscanf( file, "%lf", &(tbProph.efficacyInfectionHIVPos[j][0]));
		}
		sprintf(tmpBuf, "PostTBProph%dEffInfect", prophNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbProph.efficacyInfectionHIVNeg[1]));

		for (j = CD4_NUM_STRATA - 1; j >= 0; --j){
			fscanf( file, "%lf", &(tbProph.efficacyInfectionHIVPos[j][1]));
		}
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyInfection));
		fscanf( file, "%d", &(tbProph.decayPeriodInfection));

		sprintf(tmpBuf, "OnTBProph%dEffActivate", prophNum);
		readAndSkipPast2( tmpBuf,"DS", file );
		fscanf( file, "%lf", &(tbProph.efficacyActivationHIVNeg[TB_STRAIN_DS][0]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyActivationHIVPos[TB_STRAIN_DS][j][0]));

		sprintf(tmpBuf, "OnTBProph%dEffActivate", prophNum);
		readAndSkipPast2( tmpBuf,"MDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyActivationHIVNeg[TB_STRAIN_MDR][0]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyActivationHIVPos[TB_STRAIN_MDR][j][0]));

		sprintf(tmpBuf, "OnTBProph%dEffActivate", prophNum);
		readAndSkipPast2( tmpBuf,"XDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyActivationHIVNeg[TB_STRAIN_XDR][0]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyActivationHIVPos[TB_STRAIN_XDR][j][0]));

		sprintf(tmpBuf, "PostTBProph%dEffActivate", prophNum);
		readAndSkipPast2( tmpBuf,"DS", file );
		fscanf( file, "%lf", &(tbProph.efficacyActivationHIVNeg[TB_STRAIN_DS][1]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyActivationHIVPos[TB_STRAIN_DS][j][1]));
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyActivation[TB_STRAIN_DS]));
		fscanf( file, "%d", &(tbProph.decayPeriodActivation[TB_STRAIN_DS]));

		sprintf(tmpBuf, "PostTBProph%dEffActivate", prophNum);
		readAndSkipPast2( tmpBuf,"MDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyActivationHIVNeg[TB_STRAIN_MDR][1]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyActivationHIVPos[TB_STRAIN_MDR][j][1]));
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyActivation[TB_STRAIN_MDR]));
		fscanf( file, "%d", &(tbProph.decayPeriodActivation[TB_STRAIN_MDR]));

		sprintf(tmpBuf, "PostTBProph%dEffActivate", prophNum);
		readAndSkipPast2( tmpBuf,"XDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyActivationHIVNeg[TB_STRAIN_XDR][1]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyActivationHIVPos[TB_STRAIN_XDR][j][1]));
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyActivation[TB_STRAIN_XDR]));
		fscanf( file, "%d", &(tbProph.decayPeriodActivation[TB_STRAIN_XDR]));
		
		sprintf(tmpBuf, "OnTBProph%dEffReinfect", prophNum);
		readAndSkipPast2( tmpBuf,"DS", file );
		fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVNeg[TB_STRAIN_DS][0]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVPos[TB_STRAIN_DS][j][0]));
		
		sprintf(tmpBuf, "OnTBProph%dEffReinfect", prophNum);
		readAndSkipPast2( tmpBuf,"MDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVNeg[TB_STRAIN_MDR][0]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVPos[TB_STRAIN_MDR][j][0]));
		
		sprintf(tmpBuf, "OnTBProph%dEffReinfect", prophNum);
		readAndSkipPast2( tmpBuf,"XDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVNeg[TB_STRAIN_XDR][0]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVPos[TB_STRAIN_XDR][j][0]));

		sprintf(tmpBuf, "PostTBProph%dEffReinfect", prophNum);
		readAndSkipPast2( tmpBuf,"DS", file );
		fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVNeg[TB_STRAIN_DS][1]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVPos[TB_STRAIN_DS][j][1]));
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyReinfection[TB_STRAIN_DS]));
		fscanf( file, "%d", &(tbProph.decayPeriodReinfection[TB_STRAIN_DS]));

		sprintf(tmpBuf, "PostTBProph%dEffReinfect", prophNum);
		readAndSkipPast2( tmpBuf,"MDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVNeg[TB_STRAIN_MDR][1]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVPos[TB_STRAIN_MDR][j][1]));
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyReinfection[TB_STRAIN_MDR]));
		fscanf( file, "%d", &(tbProph.decayPeriodReinfection[TB_STRAIN_MDR]));

		sprintf(tmpBuf, "PostTBProph%dEffReinfect", prophNum);
		readAndSkipPast2( tmpBuf,"XDR", file );
		fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVNeg[TB_STRAIN_XDR][1]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbProph.efficacyReinfectionHIVPos[TB_STRAIN_XDR][j][1]));
		fscanf( file, "%d", &(tbProph.monthsOfEfficacyReinfection[TB_STRAIN_XDR]));
		fscanf( file, "%d", &(tbProph.decayPeriodReinfection[TB_STRAIN_XDR]));

		sprintf(tmpBuf, "TBProph%dCostQOL", prophNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf %lf %lf", &(tbProph.costMonthly),
			&(tbProph.costMinorToxicity),
			&(tbProph.QOLModifierMinorToxicity),
			&(tbProph.costMajorToxicity),
			&(tbProph.QOLModifierMajorToxicity));

		sprintf(tmpBuf, "TBProph%dToxicity", prophNum);
		readAndSkipPast2( tmpBuf, "Minor", file );
		fscanf( file, "%lf %lf %lf ", &(tbProph.probMinorToxicityHIVNeg), &(tbProph.probMinorToxicityOffART), &(tbProph.probMinorToxicityOnART));
		
		sprintf(tmpBuf, "TBProph%dToxicity", prophNum);
		readAndSkipPast2( tmpBuf, "Major", file );
		fscanf( file, "%lf %lf %lf %lf", &(tbProph.probMajorToxicityHIVNeg), &(tbProph.probMajorToxicityOffART), &(tbProph.probMajorToxicityOnART),
			&(tbProph.deathRateRatioMajorToxicity));

		sprintf(tmpBuf, "TBProph%dProbResist", prophNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbProph.probResistanceInActiveStates) );
	}

	//TB LTFU
	readAndSkipPast("UseTBLTFU", inputFile);
	fscanf( file, "%d", &tempBool);
	tbInputs.useTBLTFU = (bool) tempBool;

	readAndSkipPast( "MthsToLongTermEffects", file );
	fscanf( file, "%d", &(tbInputs.monthsToLongTermEffectsLTFU));

	readAndSkipPast( "TBMaxMthsLTFU", file );
	fscanf( file, "%d", &(tbInputs.maxMonthsLTFU));

	readAndSkipPast( "TBProbLTFU_Stage1", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.probLTFU[0][j]));
	readAndSkipPast( "TBProbLTFU_Stage2", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.probLTFU[1][j]));

	readAndSkipPast( "TBProbRTCHIVNeg", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.probRTCHIVNeg[j]));

	readAndSkipPast( "TBProbRTCHIVPos", file );
	for (j = 0; j < TB_NUM_STATES; j++)
		fscanf( file, "%lf", &(tbInputs.probRTCHIVPos[j]));

	readAndSkipPast( "TBProphStartWhileHIVLTFU", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.allowTBProphStartWhileHIVLTFU = (bool) tempBool;

	readAndSkipPast( "TBProphStopWhileHIVLTFUProb", file );
	fscanf( file, "%lf", &(tbInputs.probStopTBProphAtHIVLTFU));
	
	readAndSkipPast( "TBRTCRestartReg", file );
	for (j = 0; j < TB_RTC_TIME_CAT_NUM; j++)
		fscanf( file, "%lf", &(tbInputs.rtcProbRestart[j]));

	readAndSkipPast( "TBRTCResumeReg", file );
	for (j = 0; j < TB_RTC_TIME_CAT_NUM; j++)
		fscanf( file, "%lf", &(tbInputs.rtcProbResume[j]));

	readAndSkipPast( "TBRTCRetest", file );
	for (j = 0; j < TB_RTC_TIME_CAT_NUM; j++)
		fscanf( file, "%lf", &(tbInputs.rtcProbRetest[j]));

	readAndSkipPast( "TBRTCNextReg", file );
	for (j = 0; j < TB_RTC_TIME_CAT_NUM; j++)
		fscanf( file, "%lf", &(tbInputs.rtcProbNext[j]));

	//TB Costs
	readAndSkipPast( "CostsTBUntreated", file );
	for (i = 0; i < COST_NUM_TYPES; i++) {
		fscanf( file, "%lf", &(tbInputs.untreatedCosts[i]));
	}

	readAndSkipPast2( "CostsTBTreated", "Visit", file );
	for (i = 0; i < COST_NUM_TYPES; i++) {
		fscanf( file, "%lf", &(tbInputs.treatedCostsVisit[i]));
	}
	fscanf( file, "%d", &(tbInputs.frequencyVisitCosts));

	readAndSkipPast2( "CostsTBTreated", "Medication", file );
	for (i = 0; i < COST_NUM_TYPES; i++) {
		fscanf( file, "%lf", &(tbInputs.treatedCostsMed[i]));
	}
	fscanf( file, "%d", &(tbInputs.frequencyMedCosts));

	readAndSkipPast2( "CostsTBDeath", "Adult", file );
	for (i = 0; i < COST_NUM_TYPES; i++) {
		fscanf( file, "%lf", &(tbInputs.costTBDeath[i]));
	}
	for (i = 1; i <=PEDS_COST_AGE_CAT_NUM; i++){
		// read in acute OI costs
		sprintf(tmpBuf, "Peds%dCostsTBDeath", i);
		readAndSkipPast( tmpBuf, inputFile );
		for (j = 0; j < COST_NUM_TYPES; j++) {
			fscanf(inputFile, "%lf", &(tbInputs.costTBDeathPeds[i-1][j]) );
		}
	}
	readAndSkipPast("QOLActiveTB", file);
	fscanf(file, "%lf", &(tbInputs.QOLModActiveTB));
	readAndSkipPast("QOLDeathActiveTB", file);
	fscanf(file, "%lf", &(tbInputs.QOLModDeathActiveTB));
	
	//TB Diagnostics
	readAndSkipPast("EnableTBDiagnostics", inputFile);
	fscanf( file, "%d", &tempBool );
	tbInputs.enableTBDiagnostics = (bool) tempBool;

	readAndSkipPast("AllowMultipleTestsSameMonth", inputFile);
	fscanf( file, "%d", &tempBool );
	tbInputs.allowMultipleTests = (bool) tempBool;

	//Init Policy
	readAndSkipPast( "TBDiagInitPolicyAndOr", file );
	fscanf( file, "%d", &tempBool );
	tbInputs.TBDiagnosticsInitPoliciesUseOrEvaluation = (bool) tempBool;

	readAndSkipPast( "TBDiagInitPolicy", file );
	for (i = 0; i < TB_DIAG_INIT_POLICY_NUM; i++){
		fscanf( file, "%d", &tempBool );
		tbInputs.TBDiagnosticsInitPolicies[i] = (bool) tempBool;
	}

	readAndSkipPast( "TBDiagInitPolicySymptomsProb", file );
	fscanf( file, "%lf", &(tbInputs.TBDiagnosticsInitSymptomsProb) );

	readAndSkipPast( "TBDiagInitPolicyCD4Bounds", file );
	fscanf( file, "%lf %lf", &(tbInputs.TBDiagnosticsInitCD4Bounds[0]),&(tbInputs.TBDiagnosticsInitCD4Bounds[1]) );
	readAndSkipPast( "TBDiagInitPolicyCalendarMth", file );
	fscanf( file, "%d", &(tbInputs.TBDiagnosticsInitMonth) );


	readAndSkipPast( "TBDiagInitPolicyMthIntvlProb", file );
	fscanf( file, "%lf", &(tbInputs.TBDiagnosticsInitIntervalProb) );
	readAndSkipPast( "TBDiagInitPolicyMthIntvlBounds", file );
	for (i = 0; i < TB_DIAG_INIT_POLICY_INTV_NUM-1; i++)
		fscanf( file, "%d", &(tbInputs.TBDiagnosticsInitIntervalBounds[i]) );

	readAndSkipPast( "TBDiagInitPolicyMthIntvl", file );
	for (i = 0; i < TB_DIAG_INIT_POLICY_INTV_NUM; i++)
		fscanf( file, "%d", &(tbInputs.TBDiagnosticsInitInterval[i]) );
	readAndSkipPast( "TBDiagInitMinMthsPostTreatment", file );
	fscanf( file, "%d", &(tbInputs.TBDiagnosticsInitMinMthsPostTreat) );

	//Diagnostics Test order
	readAndSkipPast( "TBDiagTestOrderNeverTreat", file );
	for (i = 0; i < TB_DIAG_TEST_ORDER_NUM; i++)
		fscanf( file, "%d", &(tbInputs.TBDiagnosticsTestOrder[0][i]));
	readAndSkipPast( "TBDiagTestOrderNeverTreatDST", file );
	for (i = 0; i < TB_DIAG_TEST_ORDER_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticsTestOrderDST[0][i] = (bool) tempBool;
	}

	readAndSkipPast( "TBDiagTestOrderEverTreat", file );
	for (i = 0; i < TB_DIAG_TEST_ORDER_NUM; i++)
		fscanf( file, "%d", &(tbInputs.TBDiagnosticsTestOrder[1][i]));
	readAndSkipPast( "TBDiagTestOrderEverTreatDST", file );
	for (i = 0; i < TB_DIAG_TEST_ORDER_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticsTestOrderDST[1][i] = (bool) tempBool;
	}

	readAndSkipPast2( "TBDiagStartInTreat", "HIVNeg", file );
	for (i = 0; i < TB_NUM_STATES; i++)
		fscanf( file, "%lf", &(tbInputs.TBDiagnosticsInitInTreatmentHIVNeg[i]));

	readAndSkipPast2( "TBDiagStartInTreat", "HIVPos", file );
	for (i = 0; i < TB_NUM_STATES; i++)
		fscanf( file, "%lf", &(tbInputs.TBDiagnosticsInitInTreatmentHIVPos[i]));	

	//Test Sequence Matrix
	readAndSkipPast2( "TBDiagSeq", "2Tests", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix2Tests[i] = (bool) tempBool;
	}

	readAndSkipPast2( "TBDiagSeq", "3Tests1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix3Tests[i][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagSeq", "3Tests1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix3Tests[i][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}

	readAndSkipPast2( "TBDiagSeq", "4Tests2Pos1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix4Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagSeq", "4Tests2Pos1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix4Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagSeq", "4Tests2Neg1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix4Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagSeq", "4Tests2Neg1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticSequenceMatrix4Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}

	//Result Interpretation matrix
	readAndSkipPast( "TBDiagAllowIncomplete", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.TBDiagnosticAllowIncomplete = (bool) tempBool;

	readAndSkipPast( "TBDiagAllowNoDiagnosis", file );
	fscanf( file, "%d", &tempBool);
	tbInputs.TBDiagnosticAllowNoDiagnosis = (bool) tempBool;

	readAndSkipPast2( "TBDiagResultMatrix", "1Test", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix1Test[i] = (bool) tempBool;
	}

	readAndSkipPast2( "TBDiagResultMatrix", "2Tests1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix2Tests[i][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "2Tests1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix2Tests[i][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}

	readAndSkipPast2( "TBDiagResultMatrix", "3Tests2Pos1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix3Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "3Tests2Pos1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix3Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "3Tests2Neg1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix3Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "3Tests2Neg1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix3Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}

	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Pos2Pos1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Pos2Pos1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Pos2Neg1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Pos2Neg1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_POS] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Neg2Pos1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Neg2Pos1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Neg2Neg1Pos", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_POS][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}
	readAndSkipPast2( "TBDiagResultMatrix", "4Tests3Neg2Neg1Neg", file );
	for ( i = 0; i < TB_DIAG_STATUS_NUM; i++){
		fscanf( file, "%d", &tempBool);
		tbInputs.TBDiagnosticResultMatrix4Tests[i][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_NEG][TB_DIAG_STATUS_NEG] = (bool) tempBool;
	}

	//prob of linkage
	readAndSkipPast( "TBDiagProbLinkTBTreatmentIntegrated", file );
	fscanf( file, "%lf", &(tbInputs.probLinkTBTreatmentIntegrated));
	readAndSkipPast( "TBDiagProbLinkTBTreatmentUnintegrated", file );
	fscanf( file, "%lf", &(tbInputs.probLinkTBTreatmentNonIntegrated));
	readAndSkipPast( "TBDiagRTCForHIVUponLinkageIntegrated", file);
	fscanf( file, "%d", &tempBool );
	tbInputs.RTCForHIVUponLinkageIntegrated = (bool) tempBool;

	//read prob of hiv det upon tb linkage
	readAndSkipPast( "TBDiagProbHIVDetUponTBLinkageIntegrated", file );
	fscanf( file, "%lf", &(tbInputs.probHIVDetUponLinkageIntegrated));
	readAndSkipPast( "TBDiagProbHIVDetUponTBLinkageUnintegrated", file );
	fscanf( file, "%lf", &(tbInputs.probHIVDetUponLinkageNonIntegrated));

	for (int testNum = 0; testNum < TB_NUM_TESTS; testNum++) {
		TBInputs::TBTest &tbTest = tbInputs.TBTests[testNum];

		//read in test accuracy
		sprintf(tmpBuf, "TBTest%dPosProb", testNum);
		readAndSkipPast2( tmpBuf, "HIV_Neg", file );
		for (i = 0; i < TB_NUM_STATES; i++)
			fscanf( file, "%lf", &(tbTest.probPositiveHIVNeg[i]));

		sprintf(tmpBuf, "TBTest%dPosProb", testNum);
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j) {
			readAndSkipPast2( tmpBuf, CD4_STRATA_STRS[j], file );
			for (i = 0; i < TB_NUM_STATES; i++)
				fscanf( file, "%lf", &(tbTest.probPositiveHIVPos[i][j]));
		}

		//Prob accept and return
		sprintf(tmpBuf, "TBTest%dProbAccept", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf(file, "%lf", &(tbTest.probAccept));

		sprintf(tmpBuf, "TBTest%dProbPickup", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf(file, "%lf", &(tbTest.probPickup));

		sprintf(tmpBuf, "TBTest%dLagToReset", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf(file, "%d", &(tbTest.monthsToReset));

		sprintf(tmpBuf, "TBTest%dMonthsToPickup", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf(file, "%lf %lf", &(tbTest.timeToPickupMean), &(tbTest.timeToPickupStdDev));

		//DST
		sprintf(tmpBuf, "TBTest%dDSTPosProb", testNum);
		readAndSkipPast2( tmpBuf, "DS", file );
		for(i = 0; i < TB_NUM_STRAINS; i++)
			fscanf(file, "%lf", &(tbTest.DSTMatrixObsvTrue[TB_STRAIN_DS][i]));
		fscanf(file, "%d", &(tbTest.DSTMonthsToResult[TB_STRAIN_DS]));


		sprintf(tmpBuf, "TBTest%dDSTPosProb", testNum);
		readAndSkipPast2( tmpBuf, "MDR", file );
		for(i = 0; i < TB_NUM_STRAINS; i++)
			fscanf(file, "%lf", &(tbTest.DSTMatrixObsvTrue[TB_STRAIN_MDR][i]));
		fscanf(file, "%d", &(tbTest.DSTMonthsToResult[TB_STRAIN_MDR]));


		sprintf(tmpBuf, "TBTest%dDSTPosProb", testNum);
		readAndSkipPast2( tmpBuf, "XDR", file );
		for(i = 0; i < TB_NUM_STRAINS; i++)
			fscanf(file, "%lf", &(tbTest.DSTMatrixObsvTrue[TB_STRAIN_XDR][i]));
		fscanf(file, "%d", &(tbTest.DSTMonthsToResult[TB_STRAIN_XDR]));


		sprintf(tmpBuf, "TBTest%dDSTLinked", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &tempBool);
		tbTest.DSTLinked = (bool) tempBool;

		sprintf(tmpBuf, "TBTest%dDSTProbPickup", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.DSTProbPickup));

		//Prob Empiric
		sprintf(tmpBuf, "TBTest%dProbEmpiricWaitingResultsHIVNegSymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.probEmpiricAwaitingResultsSymptomaticHIVNeg));

		sprintf(tmpBuf, "TBTest%dProbEmpiricWaitingResultsHIVPosSymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
			fscanf( file, "%lf", &(tbTest.probEmpiricAwaitingResultsSymptomaticHIVPos[i]));
		}

		sprintf(tmpBuf, "TBTest%dProbEmpiricWaitingResultsHIVNegAsymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.probEmpiricAwaitingResultsAsymptomaticHIVNeg));

		sprintf(tmpBuf, "TBTest%dProbEmpiricWaitingResultsHIVPosAsymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
			fscanf( file, "%lf", &(tbTest.probEmpiricAwaitingResultsAsymptomaticHIVPos[i]));
		}

		sprintf(tmpBuf, "TBTest%dProbEmpiricPositiveResultHIVNegSymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.probEmpiricPositiveResultSymptomaticHIVNeg));

		sprintf(tmpBuf, "TBTest%dProbEmpiricPositiveResultHIVPosSymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
			fscanf( file, "%lf", &(tbTest.probEmpiricPositiveResultSymptomaticHIVPos[i]));
		}

		sprintf(tmpBuf, "TBTest%dProbEmpiricPositiveResultHIVNegAsymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.probEmpiricPositiveResultAsymptomaticHIVNeg));

		sprintf(tmpBuf, "TBTest%dProbEmpiricPositiveResultHIVPosAsymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
			fscanf( file, "%lf", &(tbTest.probEmpiricPositiveResultAsymptomaticHIVPos[i]));
		}

		sprintf(tmpBuf, "TBTest%dProbEmpiricTestOfferHIVNegSymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.probEmpiricTestOfferSymptomaticHIVNeg));

		sprintf(tmpBuf, "TBTest%dProbEmpiricTestOfferHIVPosSymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
			fscanf( file, "%lf", &(tbTest.probEmpiricTestOfferSymptomaticHIVPos[i]));
		}

		sprintf(tmpBuf, "TBTest%dProbEmpiricTestOfferHIVNegAsymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.probEmpiricTestOfferAsymptomaticHIVNeg));

		sprintf(tmpBuf, "TBTest%dProbEmpiricTestOfferHIVPosAsymptomatic", testNum);
		readAndSkipPast( tmpBuf, file );
		for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
			fscanf( file, "%lf", &(tbTest.probEmpiricTestOfferAsymptomaticHIVPos[i]));
		}

		sprintf(tmpBuf, "TBTest%dProbStopEmpiric", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(tbTest.probStopEmpiricSymptomatic), &(tbTest.probStopEmpiricAsymptomatic));

		//Cost and QOL of test
		sprintf(tmpBuf, "TBTest%dInitialCost", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.initCost));

		sprintf(tmpBuf, "TBTest%dQOLMod", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.QOLMod));

		sprintf(tmpBuf, "TBTest%dCostDST", testNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTest.DSTCost));
	}

	//TB Treatment
	//Order of regimens
	readAndSkipPast2( "TBTreatProbInitialNeverTreat","DS", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbInitialLine[0][TB_STRAIN_DS][i]));
	readAndSkipPast2( "TBTreatProbInitialNeverTreat","MDR", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbInitialLine[0][TB_STRAIN_MDR][i]));
	readAndSkipPast2( "TBTreatProbInitialNeverTreat","XDR", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbInitialLine[0][TB_STRAIN_XDR][i]));

	readAndSkipPast2( "TBTreatProbInitialEverTreat","DS", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbInitialLine[1][TB_STRAIN_DS][i]));
	readAndSkipPast2( "TBTreatProbInitialEverTreat","MDR", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbInitialLine[1][TB_STRAIN_MDR][i]));
	readAndSkipPast2( "TBTreatProbInitialEverTreat","XDR", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbInitialLine[1][TB_STRAIN_XDR][i]));

	readAndSkipPast( "TBTreatProbRepeatLine", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbRepeatLine[i]));
	readAndSkipPast( "TBTreatNumRepeats", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%d", &(tbInputs.TBTreatmentMaxRepeats[i]));
	readAndSkipPast( "TBTreatProbResistAfterFail", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbResistAfterFail[i]));
	readAndSkipPast( "TBTreatProbResistAfterDefault", file );
	for (i = 0; i < TB_NUM_TREATMENTS; i++)
		fscanf( file, "%lf", &(tbInputs.TBTreatmentProbResistAfterDefault[i]));

	readAndSkipPast( "TBTreatProbEmpiricMDR", file );
	fscanf( file, "%lf", &(tbInputs.probEmpiricWithObservedHistMDR));
	readAndSkipPast( "TBTreatProbEmpiricXDR", file );
	fscanf( file, "%lf", &(tbInputs.probEmpiricWithObservedHistXDR));

	readAndSkipPast( "TBTreatEmpiricNum", file );
	for (i = 0; i < TB_NUM_STRAINS; i++)
		fscanf( file, "%d", &(tbInputs.empiricTreatmentNum[i]));

	//TB Treatments
	for (int treatNum = 0; treatNum < TB_NUM_TREATMENTS; treatNum++) {
		TBInputs::TBTreatment &tbTreat = tbInputs.TBTreatments[treatNum];
		int tempInt;

		//treatment duration
		sprintf(tmpBuf, "TBTreat%dStage1Duration", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(tbTreat.stage1Duration));
		sprintf(tmpBuf, "TBTreat%dStage2Duration", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &tempInt);
		tbTreat.totalDuration = tbTreat.stage1Duration + tempInt;

		//cost of treatment
		sprintf(tmpBuf, "TBTreat%dCost", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf", &(tbTreat.costInitial), &(tbTreat.costMonthly[0]), &(tbTreat.costMonthly[1]));

		//prob success
		sprintf(tmpBuf, "TBTreat%dProbSuccess", treatNum);
		readAndSkipPast2( tmpBuf, "HIV_Neg", file );
		for (i = 0; i < TB_NUM_STRAINS; i++)
			fscanf( file, "%lf", &(tbTreat.probSuccessHIVNeg[i]));


		for (j = CD4_NUM_STRATA - 1; j >= 0; --j) {
			sprintf(tmpBuf, "TBTreat%dProbSuccess", treatNum);
			readAndSkipPast2( tmpBuf, CD4_STRATA_STRS[j], file );
			for (i = 0; i < TB_NUM_STRAINS; i++)
				fscanf( file, "%lf", &(tbTreat.probSuccessHIVPos[i][j]));
		}

		//Toxicities
		sprintf(tmpBuf, "TBTreat%dToxProbHIVNeg", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf %lf", &(tbTreat.probMinorToxHIVNeg[0]), &(tbTreat.probMajorToxHIVNeg[0]), &(tbTreat.probMinorToxHIVNeg[1]), &(tbTreat.probMajorToxHIVNeg[1]));
		
		sprintf(tmpBuf, "TBTreat%dToxProbHIVPos_offART", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf %lf", &(tbTreat.probMinorToxOffARTHIVPos[0]), &(tbTreat.probMajorToxOffARTHIVPos[0]), &(tbTreat.probMinorToxOffARTHIVPos[1]), &(tbTreat.probMajorToxOffARTHIVPos[1]));

		sprintf(tmpBuf, "TBTreat%dToxProbHIVPos_onART", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf %lf", &(tbTreat.probMinorToxOnARTHIVPos[0]), &(tbTreat.probMajorToxOnARTHIVPos[0]), &(tbTreat.probMinorToxOnARTHIVPos[1]), &(tbTreat.probMajorToxOnARTHIVPos[1]));

		sprintf(tmpBuf, "TBTreat%dDthRateRatioMajTox", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.deathRateRatioMajorToxicity));

		sprintf(tmpBuf, "TBTreat%dToxCostHIVNeg", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf %lf", &(tbTreat.costMinorToxHIVNeg),&(tbTreat.QOLModMinorToxHIVNeg),
				&(tbTreat.costMajorToxHIVNeg),&(tbTreat.QOLModMajorToxHIVNeg));

		sprintf(tmpBuf, "TBTreat%dToxCostHIVPos", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf %lf %lf", &(tbTreat.costMinorToxHIVPos),&(tbTreat.QOLModMinorToxHIVPos),
				&(tbTreat.costMajorToxHIVPos),&(tbTreat.QOLModMajorToxHIVPos));

		//Obsv early fail
		sprintf(tmpBuf, "TBTreat%dProbObsvEarlyFail", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.probObsvEarlyFail));

		sprintf(tmpBuf, "TBTreat%dProbEarlyFailObsvWithTBTest", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.probEarlyFailObservedWithTBTest));

		sprintf(tmpBuf, "TBTreat%dObsvFailTBTestCost", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.costObsvEarlyFailTBTest));

		sprintf(tmpBuf, "TBTreat%dProbSwitchOnObsvEarlyFail", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.probSwitchEarlyFail));

		sprintf(tmpBuf, "TBTreat%dNextTreatNumOnObsvEarlyFail", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(tbTreat.nextTreatNumEarlyFail));

		sprintf(tmpBuf, "TBTreat%dNextTreatNumOnRegularFail", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(tbTreat.nextTreatNumNormalFail));

		//Efficacy of treatment against infection, activation and reinfection
		sprintf(tmpBuf, "TBTreat%dEffInfect", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.efficacyInfectionHIVNeg));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j){
			fscanf( file, "%lf", &(tbTreat.efficacyInfectionHIVPos[j]));
		}
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyInfection));

		sprintf(tmpBuf, "TBTreat%dEffActivate", treatNum);
		readAndSkipPast2( tmpBuf,"DS", file );
		fscanf( file, "%lf", &(tbTreat.efficacyActivationHIVNeg[TB_STRAIN_DS]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbTreat.efficacyActivationHIVPos[TB_STRAIN_DS][j]));
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyActivation[TB_STRAIN_DS]));

		sprintf(tmpBuf, "TBTreat%dEffActivate", treatNum);
		readAndSkipPast2( tmpBuf,"MDR", file );
		fscanf( file, "%lf", &(tbTreat.efficacyActivationHIVNeg[TB_STRAIN_MDR]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbTreat.efficacyActivationHIVPos[TB_STRAIN_MDR][j]));
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyActivation[TB_STRAIN_MDR]));

		sprintf(tmpBuf, "TBTreat%dEffActivate", treatNum);
		readAndSkipPast2( tmpBuf,"XDR", file );
		fscanf( file, "%lf", &(tbTreat.efficacyActivationHIVNeg[TB_STRAIN_XDR]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbTreat.efficacyActivationHIVPos[TB_STRAIN_XDR][j]));
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyActivation[TB_STRAIN_XDR]));

		sprintf(tmpBuf, "TBTreat%dEffReinfect", treatNum);
		readAndSkipPast2( tmpBuf,"DS", file );
		fscanf( file, "%lf", &(tbTreat.efficacyReinfectionHIVNeg[TB_STRAIN_DS]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbTreat.efficacyReinfectionHIVPos[TB_STRAIN_DS][j]));
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyReinfection[TB_STRAIN_DS]));

		sprintf(tmpBuf, "TBTreat%dEffReinfect", treatNum);
		readAndSkipPast2( tmpBuf,"MDR", file );
		fscanf( file, "%lf", &(tbTreat.efficacyReinfectionHIVNeg[TB_STRAIN_MDR]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbTreat.efficacyReinfectionHIVPos[TB_STRAIN_MDR][j]));
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyReinfection[TB_STRAIN_MDR]));

		sprintf(tmpBuf, "TBTreat%dEffReinfect", treatNum);
		readAndSkipPast2( tmpBuf,"XDR", file );
		fscanf( file, "%lf", &(tbTreat.efficacyReinfectionHIVNeg[TB_STRAIN_XDR]));
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j)
			fscanf( file, "%lf", &(tbTreat.efficacyReinfectionHIVPos[TB_STRAIN_XDR][j]));
		fscanf( file, "%d", &(tbTreat.monthsOfEfficacyReinfection[TB_STRAIN_XDR]));

		sprintf(tmpBuf, "TBTreat%dRelapseMult", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &(tbTreat.relapseMult));
		sprintf(tmpBuf, "TBTreat%dRelapseMultDuration", treatNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(tbTreat.relapseMultDuration));

	}
} /* end readTBInputs */

/* readQOLInputs reads data from the QOL tab of the input sheet */
void SimContext::readQOLInputs() {
	int i, j, tempInt;

	// read in routine QOL, which is the base QoL for HIV+ detected patients 
	for ( j = 0; j < HIST_EXT_NUM; ++j ) {
		if (j == HIST_EXT_MILD){
			for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
				qolInputs.routineCareQOL[i][j] = qolInputs.routineCareQOL[i][HIST_EXT_N];
		}
		else{
		readAndSkipPast( "QOLRoutine", inputFile );
			readAndSkipPast( HIST_OI_CATS_STRS[j], inputFile );
			for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
			fscanf( inputFile, "%lf", &(qolInputs.routineCareQOL[i][j]) );
		}
	}
	readAndSkipPast( "QOLRoutineOIHistEffectDuration", inputFile);
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf( inputFile, "%d", &(qolInputs.routineCareQOLSevereOIHistDuration[i]) );

	// read in acute OI QOL
	readAndSkipPast( "QOLAcuteOI", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(qolInputs.acuteOIQOL[i]) );
	// read in death QOL
	readAndSkipPast( "QOLDeath", inputFile );
	for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i )
		fscanf( inputFile, "%lf", &(qolInputs.deathBasicQOL[i]) );

	// read in background QOL, applied to all patients regardless of HIV status
	readAndSkipPast( "QOLBackgroundMale", inputFile );
	for (i = 0; i < AGE_YRS; i++) {
		fscanf(inputFile, "%lf", &(qolInputs.backgroundQOL[GENDER_MALE][i]));
	}
	readAndSkipPast( "QOLBackgroundFemale", inputFile );
	for (i = 0; i < AGE_YRS; i++) {
		fscanf(inputFile, "%lf", &(qolInputs.backgroundQOL[GENDER_FEMALE][i]));
	}
	readAndSkipPast("QOLCalculationType", inputFile);
	fscanf(inputFile,"%d",&tempInt);
	qolInputs.QOLCalculationType = (QOL_CALC_TYPE) tempInt;
} /* end readQOLInputs */

/* readHIVTestInputs reads data from the HIVTest tab of the input sheet */
void SimContext::readHIVTestInputs() {
	char buffer[256];
	int i, j, k, tempBool;

	// read in enable HIV testing module
	readAndSkipPast( "EnableHIVtest", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	testingInputs.enableHIVTesting = (bool) tempBool;
	// read in HIV test available
	readAndSkipPast( "HIVtestAvail", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	testingInputs.HIVTestAvailable = (bool) tempBool;
	//read in whether to enable PrEP - if Dynamic Transmission is enabled, the warmup run inputs for enabling/disabling PrEP override the HIVTest tab ones; see also ConsoleMain.cpp
	readAndSkipPast( "PrEPEnable", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	if(cohortInputs.useDynamicTransm){
		testingInputs.enablePrEP = cohortInputs.usePrEPDuringWarmup;
	}
	else{
		testingInputs.enablePrEP = (bool) tempBool;
	}

	//read in CD4 test available
	readAndSkipPast( "CD4testAvail", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	testingInputs.CD4TestAvailable = (bool) tempBool;
	// read in whether to use alt HIV+ stopping rule
	readAndSkipPast( "AltStopRuleEnable", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	testingInputs.useAlternateStoppingRule = (bool) tempBool;
	if (testingInputs.enableHIVTesting != true)
		testingInputs.useAlternateStoppingRule = false;
	readAndSkipPast( "AltStopRuleTotHIV", inputFile );
	fscanf( inputFile, "%ld", &testingInputs.totalCohortsWithHIVPositiveLimit );
	readAndSkipPast( "AltStopRuleTotCohort", inputFile );
	fscanf( inputFile, "%ld", &testingInputs.totalCohortsLimit );

    // read in HIV incidence age bins
	readAndSkipPast( "IncidenceAgeBins", inputFile );
	readAndSkipPast( "EndYr", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%d", &(testingInputs.HIVIncAgeBounds[i]));
	}

	// read in distribution of HIV patient states
	readAndSkipPast( "HIVdistNeg", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.initialHIVDistribution[HIV_INF_NEG]) );
	readAndSkipPast( "HIVdistPosAcute", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.initialHIVDistribution[HIV_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVdistPosChr", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.initialHIVDistribution[HIV_INF_ASYMP_CHR_POS]) );
	testingInputs.initialHIVDistribution[HIV_INF_SYMP_CHR_POS] = 0.0;

	readAndSkipPast( "HIVNegRiskDist", inputFile );
	fscanf( inputFile, "%lf %lf", &(testingInputs.initialRiskDistribution[HIV_BEHAV_HI]),&(testingInputs.initialRiskDistribution[HIV_BEHAV_LO]) );

	// read in distribution of inital CD4s and HVLs for acute HIV infections
	readAndSkipPast( "HIVacuteCD4dist", inputFile );
	fscanf( inputFile, "%lf %lf", &(testingInputs.initialAcuteCD4DistributionMean),
		&(testingInputs.initialAcuteCD4DistributionStdDev) );
	readAndSkipPast( "HIVacuteHVLdist", inputFile );
	for ( k = HVL_NUM_STRATA - 1; k >= 0; --k ) {
		fscanf( inputFile, "%lf", &(testingInputs.initialAcuteHVLDistribution[0][k]) );
		for (i = 1; i < CD4_NUM_STRATA; ++i)
			testingInputs.initialAcuteHVLDistribution[i][k] = testingInputs.initialAcuteHVLDistribution[0][k];
	}

	// read in monthly HIV infection rate for HIV neg's
	readAndSkipPast( "HIVmthIncidMale", inputFile );
	readAndSkipPast( "hiRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.probHIVInfection[GENDER_MALE][i][HIV_BEHAV_HI]));
	}
	readAndSkipPast( "HIVmthIncidMale", inputFile );
	readAndSkipPast( "loRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.probHIVInfection[GENDER_MALE][i][HIV_BEHAV_LO]));
	}
	readAndSkipPast( "HIVmthIncidFemale", inputFile );
	readAndSkipPast( "hiRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.probHIVInfection[GENDER_FEMALE][i][HIV_BEHAV_HI]));
	}
	readAndSkipPast( "HIVmthIncidFemale", inputFile );
	readAndSkipPast( "loRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.probHIVInfection[GENDER_FEMALE][i][HIV_BEHAV_LO]));
	}

	readAndSkipPast( "UseHIVIncidReductionMult", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	testingInputs.useHIVIncReductionMult = (bool) tempBool;
	readAndSkipPast("HIVIncidReductionMult",inputFile);
	for (int i = 0; i < INC_REDUC_PERIODS_NUM; i++) {
		fscanf(inputFile, "%lf", &(testingInputs.HIVIncReducMultipliers[i]));
	}
	
	testingInputs.HIVIncReducStageBounds[0] = 1;
	readAndSkipPast("HIVIncidReductionMultBounds",inputFile);
	for (int i = 1; i < INC_REDUC_PERIODS_NUM; i++) {
		fscanf(inputFile, "%d", &(testingInputs.HIVIncReducStageBounds[i]));
	}
	// read in number of mths from acute HIV to chronic HIV state
	readAndSkipPast( "MthsAcuteToChrHIV", inputFile );
	fscanf( inputFile, "%d", &(testingInputs.monthsFromAcuteToChronic) );
	// read in CD4 transitions from HIV acute to chronic
	readAndSkipPast( "MeanCD4ChgAtChrHIVTrans", inputFile );
	for (i = HVL_NUM_STRATA - 1; i >= 0; --i)
		fscanf( inputFile, "%lf", &(testingInputs.CD4ChangeAtChronicHIVMean[i]) );
	readAndSkipPast( "SDevCD4ChgAtChrHIVTrans", inputFile );
	for (i = HVL_NUM_STRATA - 1; i >= 0; --i)
		fscanf( inputFile, "%lf", &(testingInputs.CD4ChangeAtChronicHIVStdDev[i]) );
	// read in HVL transitions from HIV acute to chronic
	for (i = HVL_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast( "HVLDistribAtChrHIVTrans", inputFile );
		readAndSkipPast( HVL_STRATA_STRS[i], inputFile );
		for ( j = HVL_NUM_STRATA - 1; j >= 0; --j )
			fscanf( inputFile, "%lf", &(testingInputs.HVLDistributionAtChronicHIV[i][j]) );
	}
	// read in probability of being initially detected as HIV positive upon model entry
	readAndSkipPast( "HIVdetectAcute", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.probHIVDetectionInitial[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "HIVdetectAsympChr", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.probHIVDetectionInitial[HIV_POS_ASYMP_CHR_POS]));
	readAndSkipPast( "HIVdetectSympChr", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.probHIVDetectionInitial[HIV_POS_SYMP_CHR_POS]));

	// read in probability that someone who hasn't been detected gets detected when they get a particular OI
	readAndSkipPast( "HIVProbDetAtOI", inputFile );
	for (i = 0; i < OI_NUM; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.probHIVDetectionWithOI[i]) );
	}
	// read in probability that someone who has been detected by a particular OI will subsequently link to HIV care
	readAndSkipPast( "HIVProbLinkAtOIDet", inputFile );
	for (i = 0; i < OI_NUM; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.probLinkToCareWithOIDet[i]) );
	}
	// read in monthly costs and QOL for special undet HIV+ states
	readAndSkipPast( "HIVundetCD4Cost", inputFile );
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i)
		fscanf( inputFile, "%lf", &(testingInputs.monthCostHIVUndetected[i]) );
	readAndSkipPast( "HIVundetCD4QOL", inputFile );
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i)
		fscanf( inputFile, "%lf", &(testingInputs.monthQOLHIVUndetected[i]) );	

	// read in death costs for special HIV- and undet HIV+ states
	readAndSkipPast( "HIVnegDthCost", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.deathCostHIVNegative) );
	readAndSkipPast( "HIVundetHIVDthCost", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.HIVDeathCostHIVUndetected) );
	readAndSkipPast( "HIVundetBgMortDthCost", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.backgroundMortDeathCostHIVUndetected) );

	// read in death QOLs for special HIV- and undet HIV+ states
	readAndSkipPast( "HIVnegDthQOL", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.deathQOLHIVNegative) );
	readAndSkipPast( "HIVundetHIVDthQOL", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.HIVDeathQOLHIVUndetected) );
	readAndSkipPast( "HIVundetBgMortDthQOL", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.backgroundMortDeathQOLHIVUndetected) );

	// read in frequency of screening and stop age for user-defined HIV tests
	readAndSkipPast( "HIVtestStartAge", inputFile );
	fscanf( inputFile, "%d", &(testingInputs.HIVRegularTestingStartAge) );
	readAndSkipPast( "HIVtestStopAge", inputFile );
	fscanf( inputFile, "%d", &(testingInputs.HIVRegularTestingStopAge) );
	readAndSkipPast( "HIVtestFreqInterval", inputFile );
	for (i = 0; i < HIV_TEST_FREQ_NUM; ++i)
		fscanf( inputFile, "%d", &(testingInputs.HIVTestingInterval[i]) );
	readAndSkipPast( "HIVtestFreqProb", inputFile );
	for (i = 0; i < HIV_TEST_FREQ_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.HIVTestingProbability[i]) );
	
	// read in enrollment characteristics of user-defined HIV screening program
	for (j = 0; j < TEST_ACCEPT_NUM; ++j) {
		sprintf( buffer, "HIVtestAcceptDist%d", j+1 );
		readAndSkipPast( buffer, inputFile );
		fscanf( inputFile, "%lf %lf %lf %lf %lf",
			&(testingInputs.HIVTestAcceptDistribution[HIV_EXT_INF_NEG_HI][j]),
			&(testingInputs.HIVTestAcceptDistribution[HIV_EXT_INF_NEG_LO][j]),
			&(testingInputs.HIVTestAcceptDistribution[HIV_EXT_INF_ASYMP_CHR_POS][j]),
			&(testingInputs.HIVTestAcceptDistribution[HIV_EXT_INF_SYMP_CHR_POS][j]),
			&(testingInputs.HIVTestAcceptDistribution[HIV_EXT_INF_ACUTE_SYN][j]) );
	}
	for (j = 0; j < TEST_ACCEPT_NUM; ++j) {
		sprintf( buffer, "HIVtestAcceptProb%d", j+1 );
		readAndSkipPast( buffer, inputFile );
		fscanf( inputFile, "%lf %lf %lf %lf %lf",
			&(testingInputs.HIVTestAcceptProb[HIV_EXT_INF_NEG_HI][j]),
			&(testingInputs.HIVTestAcceptProb[HIV_EXT_INF_NEG_LO][j]),
			&(testingInputs.HIVTestAcceptProb[HIV_EXT_INF_ASYMP_CHR_POS][j]),
			&(testingInputs.HIVTestAcceptProb[HIV_EXT_INF_SYMP_CHR_POS][j]),
			&(testingInputs.HIVTestAcceptProb[HIV_EXT_INF_ACUTE_SYN][j]) );
	}
	// read in characteristics of user-defined HIV tests
	readAndSkipPast( "HIVtestRetProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestReturnProb[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVTestReturnProb[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestReturnProb[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestReturnProb[HIV_EXT_INF_ACUTE_SYN]) );
	testingInputs.HIVTestReturnProb[HIV_EXT_INF_NEG_LO] = testingInputs.HIVTestReturnProb[HIV_EXT_INF_NEG_HI];
	readAndSkipPast( "HIVtestPosProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestPositiveProb[HIV_INF_NEG]),
		&(testingInputs.HIVTestPositiveProb[HIV_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestPositiveProb[HIV_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestPositiveProb[HIV_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVtestPosCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestPositiveCost[HIV_INF_NEG]),
		&(testingInputs.HIVTestPositiveCost[HIV_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestPositiveCost[HIV_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestPositiveCost[HIV_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVtestNegCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestNegativeCost[HIV_INF_NEG]),
		&(testingInputs.HIVTestNegativeCost[HIV_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestNegativeCost[HIV_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestNegativeCost[HIV_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVtestPosQOLMod", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestPositiveQOLModifier[HIV_INF_NEG]),
		&(testingInputs.HIVTestPositiveQOLModifier[HIV_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestPositiveQOLModifier[HIV_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestPositiveQOLModifier[HIV_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVtestNegQOLMod", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestNegativeQOLModifier[HIV_INF_NEG]),
		&(testingInputs.HIVTestNegativeQOLModifier[HIV_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestNegativeQOLModifier[HIV_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestNegativeQOLModifier[HIV_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVtestCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf",
		&(testingInputs.HIVTestCost[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVTestCost[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestCost[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestCost[HIV_EXT_INF_ACUTE_SYN]) );
	testingInputs.HIVTestCost[HIV_EXT_INF_NEG_LO] = testingInputs.HIVTestCost[HIV_EXT_INF_NEG_HI];

	// read in costs of user-defined HIV testing program 
	readAndSkipPast( "HIVtestStartupCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVTestInitialCost[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVTestInitialCost[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVTestInitialCost[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestInitialCost[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestInitialCost[HIV_EXT_INF_ACUTE_SYN]) );
	readAndSkipPast( "HIVtestNonRetCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVTestNonReturnCost[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVTestNonReturnCost[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVTestNonReturnCost[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestNonReturnCost[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVTestNonReturnCost[HIV_EXT_INF_ACUTE_SYN]) );

	// read in background HIV testing inputs
	readAndSkipPast( "HIVtestBgAcceptProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVBackgroundAcceptProb[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVBackgroundAcceptProb[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVBackgroundAcceptProb[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundAcceptProb[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundAcceptProb[HIV_EXT_INF_ACUTE_SYN])
	);
	readAndSkipPast( "HIVtestBgReturnProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVBackgroundReturnProb[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVBackgroundReturnProb[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVBackgroundReturnProb[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundReturnProb[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundReturnProb[HIV_EXT_INF_ACUTE_SYN])
	);
	readAndSkipPast( "HIVtestBgPosProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVBackgroundPositiveProb[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVBackgroundPositiveProb[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVBackgroundPositiveProb[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundPositiveProb[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundPositiveProb[HIV_EXT_INF_ACUTE_SYN])
	);
	readAndSkipPast( "HIVtestBgTestCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVBackgroundTestCost[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVBackgroundTestCost[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVBackgroundTestCost[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundTestCost[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundTestCost[HIV_EXT_INF_ACUTE_SYN])
	);
	readAndSkipPast( "HIVtestBgTestPosCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVBackgroundPositiveCost[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVBackgroundPositiveCost[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVBackgroundPositiveCost[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundPositiveCost[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundPositiveCost[HIV_EXT_INF_ACUTE_SYN])
	);
	readAndSkipPast( "HIVtestBgTestNegCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf %lf %lf",
		&(testingInputs.HIVBackgroundNegativeCost[HIV_EXT_INF_NEG_HI]),
		&(testingInputs.HIVBackgroundNegativeCost[HIV_EXT_INF_NEG_LO]),
		&(testingInputs.HIVBackgroundNegativeCost[HIV_EXT_INF_ASYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundNegativeCost[HIV_EXT_INF_SYMP_CHR_POS]),
		&(testingInputs.HIVBackgroundNegativeCost[HIV_EXT_INF_ACUTE_SYN])
	);

	readAndSkipPast( "HIVtestBgStartAge", inputFile );
	fscanf( inputFile, "%d",&(testingInputs.HIVBackgroundStartAge));
	readAndSkipPast( "HIVtestBgProbLink", inputFile );
	fscanf( inputFile, "%lf",&(testingInputs.HIVBackgroundProbLinkage));
	
	// read in inputs common to both user-defined and background HIV tests
	readAndSkipPast( "HIVtestDetectCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
		&(testingInputs.HIVTestDetectionCost[HIV_POS_ASYMP_CHR_POS]),
		&(testingInputs.HIVTestDetectionCost[HIV_POS_SYMP_CHR_POS]),
		&(testingInputs.HIVTestDetectionCost[HIV_POS_ACUTE_SYN]) );

	// read in PrEP inputs
	readAndSkipPast( "PrEPDropoutThreshold", inputFile );
	fscanf( inputFile, "%d", &(testingInputs.PrEPDropoutThreshold));
	readAndSkipPast( "DropoutThresholdRefPrEPStart", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	testingInputs.dropoutThresholdFromPrEPStart = (bool) tempBool;
	readAndSkipPast( "PrEPHIVtestAcceptProb", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPHIVTestAcceptProb[i]));
	readAndSkipPast( "PrEPInitDist", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPInitialDistribution[i]));
	
	readAndSkipPast( "PrEPJoinAfterRollout", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i){
		fscanf( inputFile, "%d", &tempBool);
		testingInputs.PrEPAfterRollout[i] = (bool) tempBool;
	}
	readAndSkipPast( "PrEPDropoutPreThreshold", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPDropoutPreThreshold[i]));
	readAndSkipPast( "PrEPDropoutPostThreshold", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPDropoutPostThreshold[i]));	
	readAndSkipPast( "PrEPCoverage", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPCoverage[i]));
	readAndSkipPast( "PrEPDuration", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%d", &(testingInputs.PrEPRolloutDuration[i]));
	readAndSkipPast( "PrEPShape", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPShape[i]));
	
    readAndSkipPast( "PrEPStartupCost", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.costPrEPInitial[i]));
    readAndSkipPast( "PrEPMonthlyCost", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.costPrEPMonthly[i]));
	readAndSkipPast( "PrEPQoLMod", inputFile );
	for (i = 0; i < HIV_BEHAV_NUM; ++i)
		fscanf( inputFile, "%lf", &(testingInputs.PrEPQOL[i]));

	readAndSkipPast( "PrepIncidMale", inputFile );
	readAndSkipPast( "hiRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.PrEPIncidence[GENDER_MALE][i][HIV_BEHAV_HI]));
	}
	readAndSkipPast( "PrepIncidMale", inputFile );
	readAndSkipPast( "loRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.PrEPIncidence[GENDER_MALE][i][HIV_BEHAV_LO]));
	}
	readAndSkipPast( "PrepIncidFemale", inputFile );
	readAndSkipPast( "hiRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.PrEPIncidence[GENDER_FEMALE][i][HIV_BEHAV_HI]));
	}
	readAndSkipPast( "PrepIncidFemale", inputFile );
	readAndSkipPast( "loRisk", inputFile );
	for (i = 0; i < AGE_CAT_HIV_INC; ++i) {
		fscanf( inputFile, "%lf", &(testingInputs.PrEPIncidence[GENDER_FEMALE][i][HIV_BEHAV_LO]));
	}

	//Read in Lab Staging (CD4) characteristics
	readAndSkipPast( "CD4TestAcceptProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
			&(testingInputs.CD4TestAcceptProb[HIV_POS_ASYMP_CHR_POS]),
			&(testingInputs.CD4TestAcceptProb[HIV_POS_SYMP_CHR_POS]),
			&(testingInputs.CD4TestAcceptProb[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "CD4TestRetProb", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
			&(testingInputs.CD4TestReturnProb[HIV_POS_ASYMP_CHR_POS]),
			&(testingInputs.CD4TestReturnProb[HIV_POS_SYMP_CHR_POS]),
			&(testingInputs.CD4TestReturnProb[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "CD4TestStartupCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
			&(testingInputs.CD4TestInitialCost[HIV_POS_ASYMP_CHR_POS]),
			&(testingInputs.CD4TestInitialCost[HIV_POS_SYMP_CHR_POS]),
			&(testingInputs.CD4TestInitialCost[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "CD4TestCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
			&(testingInputs.CD4TestCost[HIV_POS_ASYMP_CHR_POS]),
			&(testingInputs.CD4TestCost[HIV_POS_SYMP_CHR_POS]),
			&(testingInputs.CD4TestCost[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "CD4TestNonRetCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
			&(testingInputs.CD4TestNonReturnCost[HIV_POS_ASYMP_CHR_POS]),
			&(testingInputs.CD4TestNonReturnCost[HIV_POS_SYMP_CHR_POS]),
			&(testingInputs.CD4TestNonReturnCost[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "CD4TestRetCost", inputFile );
	fscanf( inputFile, "%lf %lf %lf",
			&(testingInputs.CD4TestReturnCost[HIV_POS_ASYMP_CHR_POS]),
			&(testingInputs.CD4TestReturnCost[HIV_POS_SYMP_CHR_POS]),
			&(testingInputs.CD4TestReturnCost[HIV_POS_ACUTE_SYN]));
	readAndSkipPast( "LabStageStdDev", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.CD4TestStdDevPercentage));
	readAndSkipPast( "LabStageBiasMean", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.CD4TestBiasMean));
	readAndSkipPast( "LabStageBiasStdDevPerc", inputFile );
	fscanf( inputFile, "%lf", &(testingInputs.CD4TestBiasStdDevPercentage));

	//Read Lab Staging Linkage probabilities
	readAndSkipPast( "CD4TestLinkProb", inputFile );
	for (i=CD4_NUM_STRATA-1; i >= 0; i--){
		fscanf( inputFile, "%lf", &(testingInputs.CD4TestLinkageProb[i]));
	}
} /* end readHIVTestInputs */

/* readPedsInputs reads data from the Peds tab of the input sheet */
void SimContext::readPedsInputs() {
	int i, j, k,tempBool;
	int tempInt;
	char scratch[256];

	// read in enable pediatrics module, initial HIV state, and BF status
	readAndSkipPast("EnablePeds", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	pedsInputs.enablePediatricsModel = (bool) tempBool;

	// read in initial age (mths) distrib
	readAndSkipPast( "InitAgePeds", inputFile );
	fscanf(inputFile,"%lf %lf", &pedsInputs.initialAgeMean, &pedsInputs.initialAgeStdDev);

	//read in maternal characteristics
	readAndSkipPast( "DistMatStat", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.maternalStatusDistribution[i]);
	readAndSkipPast( "ProbInfectionMom", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherIncidentInfection[i]);
	readAndSkipPast( "ProbMatMort", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMaternalDeath[i]);

	readAndSkipPast( "ProbMatStatKnown", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherStatusKnownPregnancy[i]);
	readAndSkipPast( "ProbMatStatKnownBF", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherStatusBecomeKnown[i]);
	readAndSkipPast( "ProbMomOnARTPreg", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherOnARTInitial[i]);
	readAndSkipPast( "ProbMomSuppressed", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherOnSuppressedART[i]);

	readAndSkipPast( "ProbMomKnownSuppressed", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherKnownSuppressed[i]);
	readAndSkipPast( "ProbMomKnownNotSuppressed", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherKnownNotSuppressed[i]);
	readAndSkipPast( "ProbMomLowHVL", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probMotherLowHVL[i]);

	readAndSkipPast( "DistEarlyVTHIVIU", inputFile );
	fscanf(inputFile,"%lf", &pedsInputs.propEarlyVTHIVIU);

	readAndSkipPast2( "ProbEarlyVTHIVOnART", "Suppressed", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.earlyVTHIVDistributionMotherOnArtSuppressed[i]);
	readAndSkipPast2( "ProbEarlyVTHIVOnART", "NotSuppressedLowHVL", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.earlyVTHIVDistributionMotherOnArtNotSuppressedLowHVL[i]);
	readAndSkipPast2( "ProbEarlyVTHIVOnART", "NotSuppressedHighHVL", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.earlyVTHIVDistributionMotherOnArtNotSuppressedHighHVL[i]);
	readAndSkipPast( "ProbEarlyVTHIVOffART", inputFile );
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.earlyVTHIVDistributionMotherOffArt[i]);

	readAndSkipPast2("ProbPPVTHIVOnART", "Suppressed", inputFile);
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probVTHIVPPMotherOnARTSuppressed[i]);
	readAndSkipPast2("ProbPPVTHIVOnART", "NotSuppressedLowHVL", inputFile);
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probVTHIVPPMotherOnARTNotSuppressedLowHVL[i]);
	readAndSkipPast2("ProbPPVTHIVOnART", "NotSuppressedHighHVL", inputFile);
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
		fscanf(inputFile,"%lf", &pedsInputs.probVTHIVPPMotherOnARTNotSuppressedHighHVL[i]);

	readAndSkipPast2("ProbPPVTHIVOffART", "EBF", inputFile);
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
		fscanf(inputFile,"%lf", &pedsInputs.probVTHIVPPMotherOffART[i][PEDS_BF_EXCL]);
	}
	readAndSkipPast2("ProbPPVTHIVOffART", "MBF", inputFile);
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
		fscanf(inputFile,"%lf", &pedsInputs.probVTHIVPPMotherOffART[i][PEDS_BF_MIXED]);
	}
	readAndSkipPast2("ProbPPVTHIVOffART", "CBF", inputFile);
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
		fscanf(inputFile,"%lf", &pedsInputs.probVTHIVPPMotherOffART[i][PEDS_BF_COMP]);
	}
	for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
		pedsInputs.probVTHIVPPMotherOffART[i][PEDS_BF_REPL] = 0.0;
	}

	readAndSkipPast("BreastfedDist", inputFile);
	fscanf(inputFile, "%lf", &(pedsInputs.initialBFDistribution[PEDS_BF_EXCL]));
	fscanf(inputFile, "%lf", &(pedsInputs.initialBFDistribution[PEDS_BF_MIXED]));
	pedsInputs.initialBFDistribution[PEDS_BF_COMP] = 0.0;
	pedsInputs.initialBFDistribution[PEDS_BF_REPL] = 1 - pedsInputs.initialBFDistribution[PEDS_BF_EXCL] - pedsInputs.initialBFDistribution[PEDS_BF_MIXED];

	readAndSkipPast("BreastfedStopAge", inputFile);
	fscanf(inputFile, " %lf %lf %lf", &(pedsInputs.initialBFStopAgeMean), &(pedsInputs.initialBFStopAgeStdDev), &(pedsInputs.initialBFStopAgeMax));

	readAndSkipPast("ProbStopBFWhenVSKnownNotSuppressedLowHVL", inputFile);
	fscanf(inputFile, "%lf", &(pedsInputs.probStopBFNotSuppressedLowHVL));
	readAndSkipPast("ProbStopBFWhenVSKnownNotSuppressedHighHVL", inputFile);
	fscanf(inputFile, "%lf", &(pedsInputs.probStopBFNotSuppressedHighHVL));

	// read in initial CD4 percentage
	readAndSkipPast("InitCD4PercPrevIU", inputFile);
	fscanf(inputFile, "%lf %lf", &(pedsInputs.initialCD4PercentageIUMean),
			&(pedsInputs.initialCD4PercentageIUStdDev));
	readAndSkipPast("InitCD4PercPrevIP", inputFile);
	fscanf(inputFile, "%lf %lf", &(pedsInputs.initialCD4PercentageIPMean),
			&(pedsInputs.initialCD4PercentageIPStdDev));
	for (i = 0; i < PEDS_AGE_INFANT_NUM-1; i++) {
		readAndSkipPast2("InitCD4PercIncidPP", PEDS_AGE_CAT_STRS[i], inputFile);
		fscanf(inputFile, "%lf %lf", &(pedsInputs.initialCD4PercentagePPMean[i]),
				&(pedsInputs.initialCD4PercentagePPStdDev[i]));
	}
	// last infant age cat covers 18 months and up for PP infections
	readAndSkipPast2("InitCD4PercIncidPP", "18+mth", inputFile);
	fscanf(inputFile, "%lf %lf", &(pedsInputs.initialCD4PercentagePPMean[PEDS_AGE_INFANT_NUM-1]),
	&(pedsInputs.initialCD4PercentagePPStdDev[PEDS_AGE_INFANT_NUM-1]));

	// read in initial HVL strata
	readAndSkipPast("InitHVLPrevIU", inputFile);
	for (j = HVL_NUM_STRATA - 1; j >= 0; j--) {
		fscanf(inputFile, "%lf ", &(pedsInputs.initialHVLDistributionIU[j]));
	}
	readAndSkipPast("InitHVLPrevIP", inputFile);
	for (j = HVL_NUM_STRATA - 1; j >= 0; j--) {
		fscanf(inputFile, "%lf ", &(pedsInputs.initialHVLDistributionIP[j]));
	}
	for (i = 0; i < PEDS_AGE_INFANT_NUM-1; i++) {
		readAndSkipPast2("InitHVLIncidPP", PEDS_AGE_CAT_STRS[i], inputFile);
		for (j = HVL_NUM_STRATA - 1; j >= 0; j--) {
			fscanf(inputFile, "%lf ", &(pedsInputs.initialHVLDistributionPP[i][j]));
		}
	}
	// last infant age cat covers 18 months and up for PP infections
	readAndSkipPast2("InitHVLIncidPP", "18+mth", inputFile);
	for (j = HVL_NUM_STRATA - 1; j >= 0; j--) {
		fscanf(inputFile, "%lf ", &(pedsInputs.initialHVLDistributionPP[PEDS_AGE_INFANT_NUM-1][j]));
	}

	// read in the mapping from age and CD4% to adult CD4 strata
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		readAndSkipPast2("AdultCD4Strata", PEDS_AGE_CAT_STRS[i], inputFile);
		for (j = 0; j < PEDS_CD4_PERC_NUM; j++) {
			fscanf(inputFile, "%d ", &tempInt);
			pedsInputs.adultCD4Strata[i][j] = (CD4_STRATA) tempInt;
		}
	}

	//read in post partum maternal ART distribution
	readAndSkipPast("UsePPMaternalARTStatus", inputFile);
	fscanf(inputFile, "%d", &tempBool);
	pedsInputs.usePPMaternalARTStatus = (bool) tempBool;
	readAndSkipPast2("PPMaternalARTStatusOnART", "Suppressed", inputFile);
	for (i = 0; i < PEDS_PP_MATERNAL_ART_STATUS_NUM; i++)
		fscanf(inputFile, "%lf ", &(pedsInputs.ppMaternalARTStatusProbSuppressed[i]));
	readAndSkipPast2("PPMaternalARTStatusOnART", "NotSuppressedLowHVL", inputFile);
	for (i = 0; i < PEDS_PP_MATERNAL_ART_STATUS_NUM; i++)
		fscanf(inputFile, "%lf ", &(pedsInputs.ppMaternalARTStatusProbNotSuppressedLowHVL[i]));
	readAndSkipPast2("PPMaternalARTStatusOnART", "NotSuppressedHighHVL", inputFile);
	for (i = 0; i < PEDS_PP_MATERNAL_ART_STATUS_NUM; i++)
		fscanf(inputFile, "%lf ", &(pedsInputs.ppMaternalARTStatusProbNotSuppressedHighHVL[i]));
	readAndSkipPast("PPMaternalARTStatusOffART", inputFile);
	for (i = 0; i < PEDS_PP_MATERNAL_ART_STATUS_NUM; i++)
		fscanf(inputFile, "%lf ", &(pedsInputs.ppMaternalARTStatusProbOffART[i]));
	readAndSkipPast("PPMaternalARTStatusVSKnown", inputFile);
	for (i = 0; i < PEDS_PP_MATERNAL_ART_STATUS_NUM; i++)
		fscanf(inputFile, "%lf ", &(pedsInputs.ppMaternalARTStatusProbSuppressionKnown[i]));


	// read in the monthly natural history CD4% decline
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		readAndSkipPast2("CD4PercDeclinePrevIU", PEDS_AGE_CAT_STRS[i], inputFile);
		for (j = 0; j < PEDS_CD4_PERC_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.monthlyCD4PercentDecline[PEDS_HIV_POS_IU][i][j]));
		}
	}
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		readAndSkipPast2("CD4PercDeclinePrevIP", PEDS_AGE_CAT_STRS[i], inputFile);
		for (j = 0; j < PEDS_CD4_PERC_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.monthlyCD4PercentDecline[PEDS_HIV_POS_IP][i][j]));
		}
	}
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		readAndSkipPast2("CD4PercDeclineIncidPP", PEDS_AGE_CAT_STRS[i], inputFile);
		for (j = 0; j < PEDS_CD4_PERC_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.monthlyCD4PercentDecline[PEDS_HIV_POS_PP][i][j]));
		}
	}

	// read in the transition from CD4% to absolute CD4
	for (i = 0; i < PEDS_CD4_PERC_NUM; i++) {
		readAndSkipPast2("CD4PercTransition", PEDS_CD4_PERC_STRS[i], inputFile);
		fscanf(inputFile, "%lf %lf", &(pedsInputs.absoluteCD4TransitionMean[i]),
				&(pedsInputs.absoluteCD4TransitionStdDev[i]));
	}

	// read in the transition from childhood HVL to adult setpoint HVL
	for (i = HVL_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("HVLTransPeds", HVL_STRATA_STRS[i], inputFile);
		for (j = HVL_NUM_STRATA - 1; j >= 0; j--) {
			fscanf(inputFile, "%lf ", &(pedsInputs.setpointHVLTransition[i][j]));
		}
	}
	
	// read in the HIV death rate ratios for early childhood
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		readAndSkipPast2("HIVDthRateRatio_PedsEarly", PEDS_AGE_CAT_STRS[i], inputFile);
		for (j = 0; j < PEDS_CD4_PERC_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.HIVDeathRateRatioPedsEarly[i][j]));
		}
	}
	
	// read in HIV death rate ratios for late childhood
	readAndSkipPast( "HIVDthRateRatio_PedsLate", inputFile );
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf(inputFile, "%lf", &(pedsInputs.HIVDeathRateRatioPedsLate[i]));
	
	//read in the death rate ratios and durations for maternal mortality and replacement fed, for HIV negative and positive patients in early and late childhood
	readAndSkipPast("MatMortDeathRateRatioEarly", inputFile);
	fscanf(inputFile, "%lf %lf ", &(pedsInputs.maternalMortDeathRateRatioEarlyHIVNeg),&(pedsInputs.maternalMortDeathRateRatioEarlyHIVPos));
	readAndSkipPast("MatMortDeathRateRatioDurationEarly", inputFile);
	fscanf(inputFile, "%d ", &(pedsInputs.durationMaternalMortDeathRateRatioEarly));
	readAndSkipPast("ReplFedDeathRateRatioEarly", inputFile);
	fscanf(inputFile, "%lf %lf ", &(pedsInputs.replacementFedDeathRateRatioEarlyHIVNeg), &(pedsInputs.replacementFedDeathRateRatioEarlyHIVPos));
	readAndSkipPast("ReplFedDeathRateRatioDurationEarly", inputFile);
	fscanf(inputFile, "%d ", &(pedsInputs.durationReplacementFedDeathRateRatioEarly));
	readAndSkipPast("MatMortDeathRateRatioLate", inputFile);
	fscanf(inputFile, "%lf %lf ", &(pedsInputs.maternalMortDeathRateRatioLateHIVNeg),&(pedsInputs.maternalMortDeathRateRatioLateHIVPos));
	readAndSkipPast("MatMortDeathRateRatioDurationLate", inputFile);
	fscanf(inputFile, "%d ", &(pedsInputs.durationMaternalMortDeathRateRatioLate));
	
	// read in the death rate ratios for the generic risk factors for all Peds age categories
	for (i = 0; i<RISK_FACT_NUM; i++){
		sprintf(scratch, RISK_FACT_STRS[i]);
		readAndSkipPast2( "GenRiskDthRateRatio_Peds", scratch, inputFile );
		for(j = 0;j<PEDS_AGE_CHILD_NUM;j++){
			fscanf( inputFile, "%lf", &pedsInputs.genericRiskDeathRateRatio[i][j]);
		}
	}
	
	// read in the background death rates and exposed uninfected background death rates for early childhood; late childhood uses adult inputs from the NatHist tab
	readAndSkipPast("BackgroundDthRate_MalePedsEarly", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		fscanf(inputFile, "%lf ", &(pedsInputs.backgroundDeathRateEarly[GENDER_MALE][i]));
	}
	readAndSkipPast("BackgroundDthRate_FemalePedsEarly", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		fscanf(inputFile, "%lf ", &(pedsInputs.backgroundDeathRateEarly[GENDER_FEMALE][i]));
	}
	readAndSkipPast("BackgroundDthRateExposed_MalePedsEarly", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		fscanf(inputFile, "%lf ", &(pedsInputs.backgroundDeathRateExposedUninfectedEarly[GENDER_MALE][i]));
	}
	readAndSkipPast("BackgroundDthRateExposed_FemalePedsEarly", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		fscanf(inputFile, "%lf ", &(pedsInputs.backgroundDeathRateExposedUninfectedEarly[GENDER_FEMALE][i]));
	}
	
	// read in whether to use the exposed uninfected definitions
	readAndSkipPast("UseExposedDefPeds", inputFile);
	fscanf( inputFile, "%d", &tempBool);
	pedsInputs.useExposedUninfectedDefs = (bool) tempBool;
	
	// read in the exposed uninfected definitions for the HEU death rate ratios
	readAndSkipPast("ExposedDefEarly", inputFile);
	for (i = 0; i < PEDS_EXPOSED_CONDITIONS_NUM; i++){
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.exposedUninfectedDefsEarly[i] = (bool) tempBool;
	}
	
	// read in the probability of acute OIs, for early and late childhood
	for (i = 0; i < OI_NUM; i++) {
		sprintf(scratch, "Prob_%s_NoHist", OI_STRS[i]);
		for (j = 0; j < PEDS_AGE_EARLY_NUM; j++) {
			readAndSkipPast2(scratch, PEDS_AGE_CAT_STRS[j], inputFile);
			for (k = 0; k < PEDS_CD4_PERC_NUM; k++) {
				fscanf(inputFile, "%lf ", &(pedsInputs.probAcuteOIEarly[i][j][k][HIST_N]));
			}
		}
		sprintf(scratch, "Prob_%s_WithHist", OI_STRS[i]);
		for (j = 0; j < PEDS_AGE_EARLY_NUM; j++) {
			readAndSkipPast2(scratch, PEDS_AGE_CAT_STRS[j], inputFile);
			for (k = 0; k < PEDS_CD4_PERC_NUM; k++) {
				fscanf(inputFile, "%lf ", &(pedsInputs.probAcuteOIEarly[i][j][k][HIST_Y]));
			}
		}
	}
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("ProbOIsNoHistLate", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < OI_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.probAcuteOILate[j][i][HIST_N]));
		}
	}
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("ProbOIsWithHistLate", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < OI_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.probAcuteOILate[j][i][HIST_Y]));
		}
	}
	
	// read in death rate ratios for acute cases of severe OIs and TB modeled as a severe OI in early childhood
	for (i = 0; i < PEDS_CD4_PERC_NUM; i++) {
		readAndSkipPast2("AcuteOIDthRateRatio_PedsEarly", PEDS_CD4_PERC_STRS[i], inputFile);
		for (j = 0; j < PEDS_AGE_EARLY_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.acuteOIDeathRateRatioPedsEarly[i][j]));
		}
	}
	for (i = 0; i < PEDS_CD4_PERC_NUM; i++) {
		readAndSkipPast2("AcuteOIDthRateRatioTB_PedsEarly", PEDS_CD4_PERC_STRS[i], inputFile);
		for (j = 0; j < PEDS_AGE_EARLY_NUM; j++) {
			fscanf(inputFile, "%lf ", &(pedsInputs.acuteOIDeathRateRatioTBPedsEarly[i][j]));
		}
	}
	// read in death rate ratios for having a history of a severe OI or TB modeled as a severe OI in early childhood, and their durations
	readAndSkipPast("SevrOI_HistDthRateRatio_PedsEarly", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) 
		fscanf(inputFile, "%lf ", &(pedsInputs.severeOIHistDeathRateRatioPedsEarly[i]));
	readAndSkipPast("SevrOI_HistEffectDuration_PedsEarly", inputFile);
	fscanf(inputFile, "%d ", &(pedsInputs.severeOIHistEffectDurationPedsEarly));
	readAndSkipPast("TB_OI_HistDthRateRatio_PedsEarly", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) 
		fscanf(inputFile, "%lf ", &(pedsInputs.TB_OIHistDeathRateRatioPedsEarly[i]));
	readAndSkipPast("TB_OI_HistEffectDuration_PedsEarly", inputFile);
	fscanf(inputFile, "%d ", &(pedsInputs.TB_OIHistEffectDurationPedsEarly));
		
	// read in death rate ratios for acute cases of severe OIs and TB modeled as a severe OI in late childhood
	readAndSkipPast("AcuteOIDthRateRatio_PedsLate", inputFile);
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf( inputFile, "%lf", &(pedsInputs.acuteOIDeathRateRatioPedsLate[i]));
	readAndSkipPast( "AcuteOIDthRateRatioTB_PedsLate", inputFile );
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i )
		fscanf( inputFile, "%lf", &(pedsInputs.acuteOIDeathRateRatioTBPedsLate[i]));

	// read in death rate ratios for having a history of a severe OI or TB modeled as a severe OI in late childhood, and their durations
	readAndSkipPast( "SevrOI_HistDthRateRatio_PedsLate", inputFile);
	fscanf( inputFile, "%lf", &(pedsInputs.severeOIHistDeathRateRatioPedsLate));
	readAndSkipPast( "SevrOI_HistEffectDuration_PedsLate", inputFile);
	fscanf( inputFile, "%d", &(pedsInputs.severeOIHistEffectDurationPedsLate));
	// read in death rate ratios for having a history of a severe OI or TB modeled as a severe OI in late childhood, and their durations
	readAndSkipPast( "TB_OI_HistDthRateRatio_PedsLate", inputFile);
	fscanf( inputFile, "%lf", &(pedsInputs.TB_OIHistDeathRateRatioPedsLate));
	readAndSkipPast( "TB_OI_HistEffectDuration_PedsLate", inputFile);
	fscanf( inputFile, "%d", &(pedsInputs.TB_OIHistEffectDurationPedsLate));

	// read in ART policies inputs
	// read in maximum cd4 percentages and testing intervals
	readAndSkipPast("MaxPedsCD4Perc", inputFile);
	for (i = 0; i < PEDS_AGE_EARLY_NUM; i++) {
		fscanf(inputFile, "%lf", &(pedsInputs.maxCD4Percentage[i]));
	}
	readAndSkipPast("IntvlCD4TstPreARTPeds", inputFile);
	fscanf(inputFile, "%d %d", &(pedsInputs.CD4TestingIntervalPreARTEarly),
			&(pedsInputs.CD4TestingIntervalPreARTLate));
	readAndSkipPast("IntvlHVLTstPreARTPeds", inputFile);
	fscanf(inputFile, "%d %d", &(pedsInputs.HVLTestingIntervalPreARTEarly),
			&(pedsInputs.HVLTestingIntervalPreARTLate));
	
	// read in the ART death rate ratios and their stage bounds for all Peds age categories
	readAndSkipPast("MthStageARTDthRateRatioPeds", inputFile);
	fscanf(inputFile, "%d %d", &(pedsInputs.stageBoundsARTDeathRateRatioPeds[0]),
			&(pedsInputs.stageBoundsARTDeathRateRatioPeds[1]));
	for (i = 0; i < 3; i++) {
		sprintf(scratch, "ARTDthRateRatio_Peds_T%d", i+1 );
		readAndSkipPast(scratch, inputFile);
		for(j = 0; j < PEDS_AGE_CHILD_NUM; j++){
			fscanf(inputFile, "%lf", &(pedsInputs.ARTDeathRateRatio[j][i]));
		}
	}
	// read in ART effect rate multipliers for OIs
	readAndSkipPast("MthStageRateMultOIsPedsEarly", inputFile);
	fscanf(inputFile, "%d %d", &(pedsInputs.stageBoundsMonthlyOIProbOnARTMultEarly[0]),
			&(pedsInputs.stageBoundsMonthlyOIProbOnARTMultEarly[1]));
	for (i = 0; i < PEDS_CD4_PERC_NUM; i++) {
		readAndSkipPast2("RateMultOIsPedsEarly", PEDS_CD4_PERC_STRS[i], inputFile);
		for (j = 0; j < 3; j++) {
			fscanf(inputFile, "%lf", &(pedsInputs.monthlyOIProbOnARTMultEarly[i][j]));
		}
	}
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("RateMultOIsPedsLate", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < OI_NUM; j++) {
			fscanf(inputFile, "%lf", &(pedsInputs.monthlyOIProbOnARTMultLate[i][j]));
		}
	}

	// read in primary OI proph regimen starting criteria
	readAndSkipPast( "PriProphStartPeds", inputFile );
	readAndSkipPast( "agelwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_PRIMARY].ageBounds[LOWER_BOUND][i]) );
	readAndSkipPast( "PriProphStartPeds", inputFile );
	readAndSkipPast( "ageupp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_PRIMARY].ageBounds[UPPER_BOUND][i]) );

	readAndSkipPast( "PriProphStartPeds", inputFile );
	readAndSkipPast( "cd4Percupp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_PRIMARY].currCD4PercBounds[UPPER_BOUND][i]) );
	readAndSkipPast( "PriProphStartPeds", inputFile );
	readAndSkipPast( "cd4Perclwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_PRIMARY].currCD4PercBounds[LOWER_BOUND][i]) );

	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "PriProphStartPeds", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(pedsInputs.startProph[PROPH_PRIMARY].OIHistory[j][i]) );
	}

	readAndSkipPast( "PriProphStartPeds_CondFirst", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.startProph[PROPH_PRIMARY].firstCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "PriProphStartPeds_CondSecond", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.startProph[PROPH_PRIMARY].secondCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "PriProphStartPeds_CondPar", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.startProph[PROPH_PRIMARY].parDirection=(DIRECTIONS_TYPE) tempInt;

	// read in primary OI proph regimen stopping criteria
	readAndSkipPast( "PriProphStopPeds", inputFile );
	readAndSkipPast( "agelwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.stopProph[PROPH_PRIMARY].ageLowerBound[i]) );
	readAndSkipPast( "PriProphStopPeds", inputFile );
	readAndSkipPast( "CD4Perclwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.stopProph[PROPH_PRIMARY].currCD4PercLowerBound[i]) );
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "PriProphStopPeds", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(pedsInputs.stopProph[PROPH_PRIMARY].OIHistory[j][i]) );
	}
	readAndSkipPast( "PriProphStopPeds", inputFile );
	readAndSkipPast( "mthsOnProph", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.stopProph[PROPH_PRIMARY].monthsOnProph[i]) );

	readAndSkipPast( "PriProphStopPeds_CondFirst", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.stopProph[PROPH_PRIMARY].firstCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "PriProphStopPeds_CondSecond", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.stopProph[PROPH_PRIMARY].secondCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "PriProphStopPeds_CondPar", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.stopProph[PROPH_PRIMARY].parDirection=(DIRECTIONS_TYPE) tempInt;

	// read in secondary OI proph regimen starting criteria
	readAndSkipPast( "SecProphStartPeds", inputFile );
	readAndSkipPast( "agelwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_SECONDARY].ageBounds[LOWER_BOUND][i]) );
	readAndSkipPast( "SecProphStartPeds", inputFile );
	readAndSkipPast( "ageupp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_SECONDARY].ageBounds[UPPER_BOUND][i]) );

	readAndSkipPast( "SecProphStartPeds", inputFile );
	readAndSkipPast( "cd4Percupp", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_SECONDARY].currCD4PercBounds[UPPER_BOUND][i]) );
	readAndSkipPast( "SecProphStartPeds", inputFile );
	readAndSkipPast( "cd4Perclwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.startProph[PROPH_SECONDARY].currCD4PercBounds[LOWER_BOUND][i]) );

	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "SecProphStartPeds", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(pedsInputs.startProph[PROPH_SECONDARY].OIHistory[j][i]) );
	}

	readAndSkipPast( "SecProphStartPeds_CondFirst", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.startProph[PROPH_SECONDARY].firstCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "SecProphStartPeds_CondSecond", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.startProph[PROPH_SECONDARY].secondCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "SecProphStartPeds_CondPar", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.startProph[PROPH_SECONDARY].parDirection=(DIRECTIONS_TYPE) tempInt;

	// read in primary OI proph regimen stopping criteria
	readAndSkipPast( "SecProphStopPeds", inputFile );
	readAndSkipPast( "agelwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.stopProph[PROPH_SECONDARY].ageLowerBound[i]) );
	readAndSkipPast( "SecProphStopPeds", inputFile );
	readAndSkipPast( "CD4Perclwr", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.stopProph[PROPH_SECONDARY].currCD4PercLowerBound[i]) );
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "SecProphStopPeds", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < OI_NUM; ++i )
			fscanf( inputFile, "%d", &(pedsInputs.stopProph[PROPH_SECONDARY].OIHistory[j][i]) );
	}
	readAndSkipPast( "SecProphStopPeds", inputFile );
	readAndSkipPast( "mthsOnProph", inputFile );
	for ( i = 0; i < OI_NUM; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.stopProph[PROPH_SECONDARY].monthsOnProph[i]) );

	readAndSkipPast( "SecProphStopPeds_CondFirst", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.stopProph[PROPH_SECONDARY].firstCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "SecProphStopPeds_CondSecond", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.stopProph[PROPH_SECONDARY].secondCondition=(CONDITIONS_TYPE) tempInt;
	readAndSkipPast( "SecProphStopPeds_CondPar", inputFile );
	fscanf( inputFile, "%d", &tempInt);
	pedsInputs.stopProph[PROPH_SECONDARY].parDirection=(DIRECTIONS_TYPE) tempInt;

	//Peds ART starting criteria
	// read in CD4 bounds
	readAndSkipPast( "PedsARTstartMthStage", inputFile );
	for (i=0;i<(NUM_ART_START_CD4PERC_PEDS-1);i++){
		fscanf(inputFile, "%d",&(pedsInputs.startART.CD4PercStageMonths[i]));
	}
	for (i=0;i<NUM_ART_START_CD4PERC_PEDS;i++){
		sprintf(scratch, "PedsARTstart_CD4Perc%d", i);
		readAndSkipPast2( scratch, "upp", inputFile );
		for (j=0;j<ART_NUM_LINES;j++){
			fscanf(inputFile, "%lf", &(pedsInputs.startART.CD4PercBounds[i][j][UPPER_BOUND]));
		}
		readAndSkipPast2( scratch, "lwr", inputFile );
		for (j = 0; j < ART_NUM_LINES; ++j)
			fscanf( inputFile, "%lf", &(pedsInputs.startART.CD4PercBounds[i][j][LOWER_BOUND]) );
	}

	// read in HVL bounds to administer ARTs
	readAndSkipPast2( "PedsARTstart_HVL", "upp", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(pedsInputs.startART.HVLBounds[i][UPPER_BOUND]) );
	readAndSkipPast2( "PedsARTstart_HVL", "lwr", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(pedsInputs.startART.HVLBounds[i][LOWER_BOUND]) );

	// read in OI criteria to administer ARTs
	for (j = 0; j < OI_NUM; ++j) {
		readAndSkipPast2( "PedsARTstart_OIs", OI_STRS[j], inputFile );
		for (i = 0; i < ART_NUM_LINES; ++i) {
			fscanf( inputFile, "%d", &tempBool);
			pedsInputs.startART.OIHistory[i][j] = (bool) tempBool;
		}
	}
	readAndSkipPast2( "PedsARTstart_OIs", "numOIs", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(pedsInputs.startART.numOIs[i]) );

	// read in minimum mth # to start ART
	readAndSkipPast2( "PedsARTstart", "minMthNum", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(pedsInputs.startART.minMonthNum[i]) );
	readAndSkipPast2( "PedsARTstart", "MthsSincePrevRegStop", inputFile );
	for (i = 0; i < ART_NUM_LINES; ++i)
		fscanf( inputFile, "%d", &(pedsInputs.startART.monthsSincePrevRegimen[i]) );

	// ART Failure parameters
	// read in # HVL lvls to incr for fail diag
	readAndSkipPast( "PedsARTfail_hvlNumIncr", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].HVLNumIncrease) );
	// read in absolute HVL counts for fail diag
	readAndSkipPast( "PedsARTfail_hvlAbsol", inputFile );
	readAndSkipPast( "uppBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].HVLBounds[UPPER_BOUND]) );
	readAndSkipPast( "PedsARTfail_hvlAbsol", inputFile );
	readAndSkipPast( "lwrBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].HVLBounds[LOWER_BOUND]) );
	// read in true/false use HVL as setpoint for fail diag
	readAndSkipPast( "PedsARTfail_hvlAtSetptAsFailDiag", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.failART[i].HVLFailAtSetpoint = (bool) tempBool;
	}
	// read in # of months before using HVL criteria
	readAndSkipPast( "PedsARTfail_hvlMthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].HVLMonthsFromInit) );
	// read in CD4 percentage to decr for fail diag
	readAndSkipPast( "PedsARTfail_cd4PercDrop", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.failART[i].CD4PercPercentageDrop) );
	// read in true/false use CD4 as below pre-ART nadir for fail diag
	readAndSkipPast( "PedsARTfail_cd4BelowPreARTNadir", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.failART[i].CD4PercBelowPreARTNadir = (bool) tempBool;
	}
	// read in absolute CD4 counts as OR criteria for fail diag
	readAndSkipPast( "PedsARTfail_cd4AbsolOR", inputFile );
	readAndSkipPast( "uppBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.failART[i].CD4PercBoundsOR[UPPER_BOUND]) );
	readAndSkipPast( "PedsARTfail_cd4AbsolOR", inputFile );
	readAndSkipPast( "lwrBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.failART[i].CD4PercBoundsOR[LOWER_BOUND]) );
	// read in absolute CD4 counts as AND criteria for fail diag
	readAndSkipPast( "PedsARTfail_cd4AbsolAND", inputFile );
	readAndSkipPast( "uppBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.failART[i].CD4PercBoundsAND[UPPER_BOUND]) );
	readAndSkipPast( "PedsARTfail_cd4AbsolAND", inputFile );
	readAndSkipPast( "lwrBnd", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.failART[i].CD4PercBoundsAND[LOWER_BOUND]) );
	// read in # of months before using CD4 criteria
	readAndSkipPast( "PedsARTfail_cd4MthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].CD4PercMonthsFromInit) );
	// read in whether to treat OI event as ART fail diag
	for ( j = 0; j < OI_NUM; ++j ) {
		readAndSkipPast( "PedsARTfail_OIs", inputFile );
		readAndSkipPast( OI_STRS[j], inputFile );
		for ( i = 0; i < ART_NUM_LINES; ++i ){
			fscanf( inputFile, "%d", &tempInt);
			pedsInputs.failART[i].OIsEvent[j] = (ART_FAIL_BY_OI) tempInt;
		}	
	}
	readAndSkipPast( "PedsARTfail_OIsMinNum", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].OIsMinNum) );
	readAndSkipPast( "PedsARTfail_OIsMthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].OIsMonthsFromInit) );
	// read in ART failure diagnoses criteria parameters
	readAndSkipPast( "PedsARTfail_diagNumTestsFail", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].diagnoseNumTestsFail) );
	readAndSkipPast( "PedsARTfail_diagUseHVLTestsConfirm", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.failART[i].diagnoseUseHVLTestsConfirm = (bool) tempBool;
	}
	readAndSkipPast( "PedsARTfail_diagUseCD4TestsConfirm", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.failART[i].diagnoseUseCD4TestsConfirm = (bool) tempBool;
	}
	readAndSkipPast( "PedsARTfail_diagNumTestsConfirm", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.failART[i].diagnoseNumTestsConfirm) );

	//read in ART stopping policy
	// read in maximum number of months to be on ART
	readAndSkipPast( "PedsARTstop_MaxMthsOnART", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.stopART[i].maxMonthsOnART));
	// read in stop on major toxicity
	readAndSkipPast("PedsARTstop_MajorToxicity", inputFile);
	for (i = 0; i < ART_NUM_LINES; i++) {
		fscanf(inputFile, "%d", &tempBool);
		pedsInputs.stopART[i].withMajorToxicty = (bool) tempBool;
	}
	// read in criteria to use after failure has been observed
	readAndSkipPast( "PedsARTstop_OnFailImmed", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.stopART[i].afterFailImmediate = (bool) tempBool;
	}
	readAndSkipPast( "PedsARTstop_OnFailBelowCD4", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%lf", &(pedsInputs.stopART[i].afterFailCD4PercLowerBound));
	readAndSkipPast( "PedsARTstop_OnFailSevereOI", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i ) {
		fscanf( inputFile, "%d", &tempBool);
		pedsInputs.stopART[i].afterFailWithSevereOI = (bool) tempBool;
	}
	readAndSkipPast( "PedsARTstop_OnFailMthsAfterObsv", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.stopART[i].afterFailMonthsFromObserved));
	// read in minimum month number to stop ART
	readAndSkipPast( "PedsARTstop_OnFailMinMthNum", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.stopART[i].afterFailMinMonthNum) );
	readAndSkipPast( "PedsARTstop_OnFailMthsFromInit", inputFile );
	for ( i = 0; i < ART_NUM_LINES; ++i )
		fscanf( inputFile, "%d", &(pedsInputs.stopART[i].afterFailMonthsFromInit) );
} /* end readPedsInputs */


/* readPedsProphInputs reads data from the PedsProphs tab of the input sheet */
void SimContext::readPedsProphInputs() {
	char scratch[256], buffer[256];
	int i, j, k, tempBool;

	for ( k = 0; k < OI_NUM; ++k) {
		for ( i = 0; i < PROPH_NUM; ++i ) {
			// read in OI proph id and name
			sprintf( scratch, "OI%d_PriProph%dPeds", k + 1, i + 1 );
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Id", inputFile );
			int idNum;
			fscanf( inputFile, " %d", &idNum);
			// continue to next proph if this one is unspecified
			if (idNum == NOT_APPL) {
				pedsProphsInputs[PROPH_PRIMARY][k][i] = NULL;
				continue;
			}
			// allocate a proph input structure
			pedsProphsInputs[PROPH_PRIMARY][k][i] = new ProphInputs();

			// read in OI proph efficacy (for primary proph, primary OIs only in LDC model)
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffPriOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->primaryOIEfficacy[j]) );

			// read in OI primary proph efficacy on secondary OIs
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffSecOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->secondaryOIEfficacy[j]) );

			// read in proph resist prob, level of proph resistance, time of proph resistance,
			// cost factor of proph resistance, and death rate ratio for proph resistance for the primary Peds proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Resist", inputFile );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->monthlyProbResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->percentResistance) );
			fscanf( inputFile, "%d", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->timeOfResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->costFactorResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->deathRateRatioResistance) );

			// read in min & maj tox for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Tox", inputFile );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->probMinorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->probMajorToxicity) );
			fscanf( inputFile, "%d", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->monthsToToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->deathRateRatioMajorToxicity) );	

			// read in costs and QOL for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "CostQOL", inputFile );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->costMonthly) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->costMinorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->QOLMinorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->costMajorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->QOLMajorToxicity) );

			// read in proph switching inputs
			readAndSkipPast(scratch, inputFile);
			readAndSkipPast("Switch", inputFile);
			fscanf( inputFile, "%d", &(pedsProphsInputs[PROPH_PRIMARY][k][i]->monthsToSwitch) );
			fscanf( inputFile, "%d", &tempBool);
			pedsProphsInputs[PROPH_PRIMARY][k][i]->switchOnMinorToxicity = (bool) tempBool;
			fscanf( inputFile, "%d", &tempBool);
			pedsProphsInputs[PROPH_PRIMARY][k][i]->switchOnMajorToxicity = (bool) tempBool;
		}
	}

	for ( k = 0; k < OI_NUM; ++k) {
		for ( i = 0; i < PROPH_NUM; ++i ) {
			// read in OI proph id and name
			sprintf( scratch, "OI%d_SecProph%dPeds", k + 1, i + 1 );
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Id", inputFile );
			int idNum;
			fscanf( inputFile, " %d", &idNum);
			// continue to next proph if this one is unspecified
			if (idNum == NOT_APPL) {
				pedsProphsInputs[PROPH_SECONDARY][k][i] = NULL;
				continue;
			}
			// allocate a proph input structure
			pedsProphsInputs[PROPH_SECONDARY][k][i] = new ProphInputs();

			// read in OI proph efficacy (for primary proph, primary OIs only in LDC model)
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffPriOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->primaryOIEfficacy[j]) );

			// read in OI primary proph efficacy on secondary OIs
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "EffSecOIs", inputFile );
			for ( j = 0; j < OI_NUM; ++j )
				fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->secondaryOIEfficacy[j]) );

			// read in proph resist prob, level of proph resistance, time of proph resistance,
			// cost factor of proph resistance, and death rate ratio for proph resistance for the secondary Peds proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Resist", inputFile );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->monthlyProbResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->percentResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->timeOfResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->costFactorResistance) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->deathRateRatioResistance) );

			// read in min & maj tox for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "Tox", inputFile );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->probMinorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->probMajorToxicity) );
			fscanf( inputFile, "%d", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->monthsToToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->deathRateRatioMajorToxicity) );

			// read in costs and QOL for proph
			readAndSkipPast( scratch, inputFile );
			readAndSkipPast( "CostQOL", inputFile );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->costMonthly) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->costMinorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->QOLMinorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->costMajorToxicity) );
			fscanf( inputFile, "%lf", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->QOLMajorToxicity) );

			// read in proph switching inputs
			readAndSkipPast(scratch, inputFile);
			readAndSkipPast("Switch", inputFile);
			fscanf( inputFile, "%d", &(pedsProphsInputs[PROPH_SECONDARY][k][i]->monthsToSwitch) );
			fscanf( inputFile, "%d", &tempBool);
			pedsProphsInputs[PROPH_SECONDARY][k][i]->switchOnMinorToxicity = (bool) tempBool;
			fscanf( inputFile, "%d", &tempBool);
			pedsProphsInputs[PROPH_SECONDARY][k][i]->switchOnMajorToxicity = (bool) tempBool;
		}
	}
} /* end readPedsProphInputs */

/* readPedsARTInputs reads data from the ARTs tab of the input sheet */
void SimContext::readPedsARTInputs() {
	char tmpBuf[256], tmpBuf2[256];
	int i, j, k, tempBool;
	double tempCost;
	FILE *file = inputFile;

	for (int artNum = 1; artNum <= ART_NUM_LINES; artNum++) {
		// read in regimen id num and name
		sprintf(tmpBuf, "ART%dIdPeds", artNum);
		readAndSkipPast( tmpBuf, file );
		int idNum;
		fscanf( file, " %d", &idNum );
		// skip to next regimen if this one is not specified
		if (idNum == NOT_APPL) {
			pedsARTInputs[artNum - 1] = NULL;
			continue;
		}
		// create new regimen input structure
		pedsARTInputs[artNum - 1] = new PedsARTInputs();
		PedsARTInputs &pedsART = *(pedsARTInputs[artNum - 1]);

		// read in one-time startup cost
		sprintf(tmpBuf, "ART%dInitCostPeds", artNum);
		readAndSkipPast( tmpBuf, file );
		for (i = 0; i < PEDS_ART_COST_AGE_CAT_NUM; i++)
			fscanf( file, "%lf", &pedsART.costInitial[i]);
		// read in monthly cost
		sprintf(tmpBuf, "ART%dMthCostPeds", artNum);
		readAndSkipPast( tmpBuf, file );
		for (i = 0; i < PEDS_ART_COST_AGE_CAT_NUM; i++)
			fscanf( file, "%lf", &pedsART.costMonthly[i]);

		// read in efficacy time horizon
		sprintf(tmpBuf, "ART%dEffTimeHorizonPeds", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d %d", &pedsART.efficacyTimeHorizonEarly, &pedsART.efficacyTimeHorizonLate );

		// read in mth by which all would fail
		sprintf(tmpBuf, "ART%dMthForceFailPeds", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d %d", &pedsART.forceFailAtMonthEarly, &pedsART.forceFailAtMonthLate );

		// read in CD4 effect on ART
		sprintf(tmpBuf, "ART%dMthStageCD4Eff_SuccPedsEarly", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d %d", &(pedsART.stageBoundsCD4PercentageChangeOnSuppARTEarly[0]),
				&(pedsART.stageBoundsCD4PercentageChangeOnSuppARTEarly[1]));
		sprintf(tmpBuf, "ART%dCD4EffSlope_SuccPedsEarly", artNum);
		for (j = 0; j < PEDS_AGE_EARLY_NUM; j++) {
			for (k = 0; k < CD4_RESPONSE_NUM_TYPES; k++) {
				sprintf(tmpBuf2, "%s%s", PEDS_AGE_CAT_STRS[j], CD4_RESPONSE_STRS[k]);
				readAndSkipPast2( tmpBuf, tmpBuf2, file );
				fscanf( file, "%lf %lf %lf %lf %lf %lf",
					&(pedsART.CD4PercentageChangeOnSuppARTMeanEarly[j][k][0]), &(pedsART.CD4PercentageChangeOnSuppARTStdDevEarly[j][k][0]),
					&(pedsART.CD4PercentageChangeOnSuppARTMeanEarly[j][k][1]), &(pedsART.CD4PercentageChangeOnSuppARTStdDevEarly[j][k][1]),
					&(pedsART.CD4PercentageChangeOnSuppARTMeanEarly[j][k][2]), &(pedsART.CD4PercentageChangeOnSuppARTStdDevEarly[j][k][2]));
			}
		}
		sprintf(tmpBuf, "ART%dMthStageCD4Eff_SuccPedsLate", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d %d", &(pedsART.stageBoundsCD4ChangeOnSuppARTLate[0]),
				&(pedsART.stageBoundsCD4ChangeOnSuppARTLate[1]));
		sprintf(tmpBuf, "ART%dCD4EffSlope_SuccPedsLate", artNum);
		for (j = 0; j < CD4_RESPONSE_NUM_TYPES; j++) {
			readAndSkipPast2(tmpBuf, CD4_RESPONSE_STRS[j], file);
			fscanf(file, "%lf %lf %lf %lf %lf %lf",
				&(pedsART.CD4ChangeOnSuppARTMeanLate[j][0]), &(pedsART.CD4ChangeOnSuppARTStdDevLate[j][0]),
				&(pedsART.CD4ChangeOnSuppARTMeanLate[j][1]), &(pedsART.CD4ChangeOnSuppARTStdDevLate[j][1]),
				&(pedsART.CD4ChangeOnSuppARTMeanLate[j][2]), &(pedsART.CD4ChangeOnSuppARTStdDevLate[j][2]));
		}
		
		sprintf(tmpBuf, "ART%dMthStageCD4Eff_FailPedsEarly", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(pedsART.stageBoundCD4PercentageChangeOnARTFailEarly));
		sprintf(tmpBuf, "ART%dCD4EffMult_FailPedsEarly", artNum);
		for (j = 0; j < CD4_RESPONSE_NUM_TYPES; j++) {
			readAndSkipPast2(tmpBuf, CD4_RESPONSE_STRS[j], file);
			fscanf( file, "%lf %lf",
				&(pedsART.CD4PercentageMultiplierOnFailedARTEarly[j][0]),
				&(pedsART.CD4PercentageMultiplierOnFailedARTEarly[j][1]));
		}
		sprintf(tmpBuf, "ART%dMthCD4SecStdDevPedsEarly", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &pedsART.secondaryCD4PercentageChangeOnARTStdDevEarly);
		sprintf(tmpBuf, "ART%dMthStageCD4Eff_FailPedsLate", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(pedsART.stageBoundCD4ChangeOnARTFailLate));
		sprintf(tmpBuf, "ART%dCD4EffMult_FailPedsLate", artNum);
		for (j = 0; j < CD4_RESPONSE_NUM_TYPES; j++) {
			readAndSkipPast2(tmpBuf, CD4_RESPONSE_STRS[j], file);
			fscanf( file, "%lf %lf",
				&(pedsART.CD4MultiplierOnFailedARTLate[j][0]),
				&(pedsART.CD4MultiplierOnFailedARTLate[j][1]));
		}
		sprintf(tmpBuf, "ART%dMthCD4SecStdDevPedsLate", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf", &pedsART.secondaryCD4ChangeOnARTStdDevLate);

		// read in CD4 effect off ART
		sprintf(tmpBuf, "ART%dCD4EffOffART_SuccPedsEarly", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(pedsART.monthlyCD4PercentageMultiplierOffARTPreSetpointEarly[ART_EFF_SUCCESS]),
			&(pedsART.monthlyCD4PercentageMultiplierOffARTPostSetpointEarly[ART_EFF_SUCCESS]) );
		sprintf(tmpBuf, "ART%dCD4EffOffART_FailPedsEarly", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(pedsART.monthlyCD4PercentageMultiplierOffARTPreSetpointEarly[ART_EFF_FAILURE]),
			&(pedsART.monthlyCD4PercentageMultiplierOffARTPostSetpointEarly[ART_EFF_FAILURE]) );
		sprintf(tmpBuf, "ART%dCD4EffOffART_SuccPedsLate", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(pedsART.monthlyCD4MultiplierOffARTPreSetpointLate[ART_EFF_SUCCESS]),
			&(pedsART.monthlyCD4MultiplierOffARTPostSetpointLate[ART_EFF_SUCCESS]) );
		sprintf(tmpBuf, "ART%dCD4EffOffART_FailPedsLate", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%lf %lf", &(pedsART.monthlyCD4MultiplierOffARTPreSetpointLate[ART_EFF_FAILURE]),
			&(pedsART.monthlyCD4MultiplierOffARTPostSetpointLate[ART_EFF_FAILURE]) );

		// read in HVL change probability
		sprintf(tmpBuf, "ART%dHVLChgProbPedsEarly", artNum);
		readAndSkipPast2( tmpBuf, "Supp", file );
		fscanf( file, "%lf %d", &(pedsART.monthlyProbHVLChangeEarly[ART_EFF_SUCCESS]),
				&(pedsART.monthlyNumStrataHVLChangeEarly[ART_EFF_SUCCESS]) );
		
		sprintf(tmpBuf, "ART%dHVLChgProbPedsEarly", artNum);
		readAndSkipPast2( tmpBuf, "Fail", file );
		fscanf( file, "%lf %d", &(pedsART.monthlyProbHVLChangeEarly[ART_EFF_FAILURE]),
				&(pedsART.monthlyNumStrataHVLChangeEarly[ART_EFF_FAILURE]) );
		sprintf(tmpBuf, "ART%dHVLChgProbPedsLate", artNum);
		readAndSkipPast2( tmpBuf, "Supp", file );
		fscanf( file, "%lf %d", &(pedsART.monthlyProbHVLChangeLate[ART_EFF_SUCCESS]),
				&(pedsART.monthlyNumStrataHVLChangeLate[ART_EFF_SUCCESS]) );
		sprintf(tmpBuf, "ART%dHVLChgProbPedsLate", artNum);
		readAndSkipPast2( tmpBuf, "Fail", file );
		fscanf( file, "%lf %d", &(pedsART.monthlyProbHVLChangeLate[ART_EFF_FAILURE]),
				&(pedsART.monthlyNumStrataHVLChangeLate[ART_EFF_FAILURE]) );

		//read regimen specific heterogeneity inputs
		sprintf(tmpBuf, "PedsART%dPropMthCostNonRespondersEarly", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf", &(pedsART.propMthCostNonRespondersEarly));

		sprintf(tmpBuf, "PedsART%dProbRestartRegimenEarly", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf %lf", &(pedsART.probRestartARTRegimenAfterFailureEarly[RESP_TYPE_FULL]),&(pedsART.probRestartARTRegimenAfterFailureEarly[RESP_TYPE_PARTIAL]),&(pedsART.probRestartARTRegimenAfterFailureEarly[RESP_TYPE_NON]));

		sprintf(tmpBuf, "PedsART%dHetPropRespRegCoeffEarly", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf", &(pedsART.propRespondARTRegimenLogitMeanEarly), &(pedsART.propRespondARTRegimenLogitStdDevEarly));

		sprintf(tmpBuf, "PedsART%dHetOutcomesEarly", artNum);
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Supp",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_SUPP][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_SUPP][1]),&(pedsART.responseTypeValuesEarly[HET_OUTCOME_SUPP][0]),&(pedsART.responseTypeValuesEarly[HET_OUTCOME_SUPP][1]), &(pedsART.responseTypeExponentsEarly[HET_OUTCOME_SUPP]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("LateFail",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_LATEFAIL][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_LATEFAIL][1]),&(pedsART.responseTypeValuesEarly[HET_OUTCOME_LATEFAIL][0]),&(pedsART.responseTypeValuesEarly[HET_OUTCOME_LATEFAIL][1]), &(pedsART.responseTypeExponentsEarly[HET_OUTCOME_LATEFAIL]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectOI",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_ARTEFFECT_OI][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_ARTEFFECT_OI][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectCHRMs",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_ARTEFFECT_CHRMS][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_ARTEFFECT_CHRMS][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectMort",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_ARTEFFECT_MORT][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_ARTEFFECT_MORT][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Resist",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_RESIST][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_RESIST][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Tox",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_TOX][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_TOX][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Cost",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_COST][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_COST][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("RestartAfterFail",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_RESTART][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_RESTART][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Resuppression",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_RESUPP][0]), &(pedsART.responseTypeThresholdsEarly[HET_OUTCOME_RESUPP][1]),&(pedsART.responseTypeValuesEarly[HET_OUTCOME_RESUPP][0]),&(pedsART.responseTypeValuesEarly[HET_OUTCOME_RESUPP][1]), &(pedsART.responseTypeExponentsEarly[HET_OUTCOME_RESUPP]));

		sprintf(tmpBuf, "PedsART%dARTEffectOnFailedARTEarly", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%d", &tempBool);
        pedsART.applyARTEffectOnFailedARTEarly = (bool) tempBool;
		
		sprintf(tmpBuf, "PedsART%dPropMthCostNonRespondersLate", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf", &(pedsART.propMthCostNonRespondersLate));

		sprintf(tmpBuf, "PedsART%dProbRestartRegimenLate", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf %lf", &(pedsART.probRestartARTRegimenAfterFailureLate[RESP_TYPE_FULL]),&(pedsART.probRestartARTRegimenAfterFailureLate[RESP_TYPE_PARTIAL]),&(pedsART.probRestartARTRegimenAfterFailureLate[RESP_TYPE_NON]));

		sprintf(tmpBuf, "PedsART%dHetPropRespRegCoeffLate", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%lf %lf", &(pedsART.propRespondARTRegimenLogitMeanLate), &(pedsART.propRespondARTRegimenLogitStdDevLate));

		sprintf(tmpBuf, "PedsART%dHetOutcomesLate", artNum);
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Supp",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_SUPP][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_SUPP][1]),&(pedsART.responseTypeValuesLate[HET_OUTCOME_SUPP][0]),&(pedsART.responseTypeValuesLate[HET_OUTCOME_SUPP][1]), &(pedsART.responseTypeExponentsLate[HET_OUTCOME_SUPP]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("LateFail",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_LATEFAIL][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_LATEFAIL][1]),&(pedsART.responseTypeValuesLate[HET_OUTCOME_LATEFAIL][0]),&(pedsART.responseTypeValuesLate[HET_OUTCOME_LATEFAIL][1]), &(pedsART.responseTypeExponentsLate[HET_OUTCOME_LATEFAIL]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectOI",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_ARTEFFECT_OI][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_ARTEFFECT_OI][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectCHRMs",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_ARTEFFECT_CHRMS][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_ARTEFFECT_CHRMS][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("ARTEffectMort",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_ARTEFFECT_MORT][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_ARTEFFECT_MORT][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Resist",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_RESIST][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_RESIST][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Tox",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_TOX][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_TOX][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Cost",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_COST][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_COST][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("RestartAfterFail",file);
		fscanf(file, "%lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_RESTART][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_RESTART][1]));
		readAndSkipPast(tmpBuf, file);
		readAndSkipPast("Resuppression",file);
		fscanf(file, "%lf %lf %lf %lf %lf", &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_RESUPP][0]), &(pedsART.responseTypeThresholdsLate[HET_OUTCOME_RESUPP][1]),&(pedsART.responseTypeValuesLate[HET_OUTCOME_RESUPP][0]),&(pedsART.responseTypeValuesLate[HET_OUTCOME_RESUPP][1]), &(pedsART.responseTypeExponentsLate[HET_OUTCOME_RESUPP]));
		sprintf(tmpBuf, "PedsART%dARTEffectOnFailedARTLate", artNum);
		readAndSkipPast(tmpBuf, file);
		fscanf(file, "%d", &tempBool);
        pedsART.applyARTEffectOnFailedARTLate = (bool) tempBool;

	}
} /* end readPedsARTInputs */


/* readPedsCostInputs reads data from the PedsCost tab of the input sheet */
void SimContext::readPedsCostInputs() {
	char tmpBuf[256];
	int i, j, k;

	for (int t=1; t <=PEDS_COST_AGE_CAT_NUM;t++){
		// read in acute OI costs
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "Peds%dCostAcuteOI_noART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.acuteOICostTreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "Peds%dCostAcuteOI_noART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.acuteOICostUntreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "Peds%dCostAcuteOI_onART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.acuteOICostTreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
		for ( i = 0; i < OI_NUM; ++i ) {
			sprintf(tmpBuf, "Peds%dCostAcuteOI_onART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( OI_STRS[i], inputFile );
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.acuteOICostUntreated[t-1][ART_ON_STATE][i][j]) );
			}
		}

		// read in CD4 / HVL test costs
		sprintf(tmpBuf, "Peds%dCostCD4Test", t);
		readAndSkipPast( tmpBuf, inputFile );
		for (i = 0; i < COST_NUM_TYPES; i++) {
			fscanf(inputFile, "%lf", &(pedsCostInputs.CD4TestCost[t-1][i]) );
		}
		sprintf(tmpBuf, "Peds%dCostHVLTest", t);
		readAndSkipPast( tmpBuf, inputFile );
		for (i = 0; i < COST_NUM_TYPES; i++) {
			fscanf(inputFile, "%lf", &(pedsCostInputs.HVLTestCost[t-1][i]) );
		}

		// read in death from OI costs
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "Peds%dCostDth_noART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.deathCostTreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "Peds%dCostDth_noART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.deathCostUntreated[t-1][ART_OFF_STATE][i][j]) );
			}
		}
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "Peds%dCostDth_onART_treated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.deathCostTreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
		for ( i = 0; i < DTH_NUM_CAUSES_BASIC; ++i ) {
			sprintf(tmpBuf, "Peds%dCostDth_onART_untreated", t);
			readAndSkipPast( tmpBuf, inputFile );
			readAndSkipPast( DTH_CAUSES_STRS[i], inputFile);
			for (j = 0; j < COST_NUM_TYPES; j++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.deathCostUntreated[t-1][ART_ON_STATE][i][j]) );
			}
		}
	}

	// read in routine care costs for HIV-neg
	readAndSkipPast("PedsCostRoutine_HIVneg_dmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < PEDS_COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVNegative[i][j][COST_DIR_MED]));
		}
	}
	readAndSkipPast("PedsCostRoutine_HIVneg_nmed", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < PEDS_COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVNegative[i][j][COST_DIR_NONMED]));
		}
	}
	readAndSkipPast("PedsCostRoutine_HIVneg_time", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < PEDS_COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVNegative[i][j][COST_TIME]));
		}
	}
	readAndSkipPast("PedsCostRoutine_HIVneg_indr", inputFile);
	for (i = 0; i < GENDER_NUM; i++) {
		for (j = 0; j < PEDS_COST_AGE_CAT_NUM; j++) {
			fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVNegative[i][j][COST_INDIR]));
		}
	}
	// read in routine care costs for HIV positive, not on ART
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("PedsCostRoutine_HIVpos_noART_dmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_DIR_MED]));
				}
		}
		readAndSkipPast2("PedsCostRoutine_HIVpos_noART_nmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_DIR_NONMED]));
			}
		}
		readAndSkipPast2("PedsCostRoutine_HIVpos_noART_time", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_TIME]));
			}
		}
		readAndSkipPast2("PedsCostRoutine_HIVpos_noART_indr", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_OFF_STATE][i][j][k][COST_INDIR]));
			}
		}
	}
	// read in routine care costs for HIV positive, on ART
	for (i = CD4_NUM_STRATA - 1; i >= 0; i--) {
		readAndSkipPast2("PedsCostRoutine_HIVpos_onART_dmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_DIR_MED]));

			}
		}
		readAndSkipPast2("PedsCostRoutine_HIVpos_onART_nmed", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_DIR_NONMED]));
			}
		}
		readAndSkipPast2("PedsCostRoutine_HIVpos_onART_time", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_TIME]));
			}
		}
		readAndSkipPast2("PedsCostRoutine_HIVpos_onART_indr", CD4_STRATA_STRS[i], inputFile);
		for (j = 0; j < GENDER_NUM; j++) {
			for (k = 0; k < PEDS_COST_AGE_CAT_NUM; k++) {
				fscanf(inputFile, "%lf", &(pedsCostInputs.routineCareCostHIVPositive[ART_ON_STATE][i][j][k][COST_INDIR]));
			}
		}
	}
} /* end readPedsCostInputs */

/* readEIDInputs reads data from the EID tab of the input sheet */
void SimContext::readEIDInputs() {
	int tempBool, i;
	char tmpBuf[256];

	// read in enable EID HIV testing module
	readAndSkipPast( "EnableHIVtestEID", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	eidInputs.enableHIVTestingEID = (bool) tempBool;
	if (!pedsInputs.enablePediatricsModel)
		eidInputs.enableHIVTestingEID = false;
	// read in whether to use alt HIV+ stopping rule
	readAndSkipPast( "AltStopRuleEnableEID", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	eidInputs.useAlternateStoppingRuleEID = (bool) tempBool;
	if (eidInputs.enableHIVTestingEID != true)
		eidInputs.useAlternateStoppingRuleEID = false;
	readAndSkipPast( "AltStopRuleTotHIVEID", inputFile );
	fscanf( inputFile, "%ld", &eidInputs.totalCohortsWithHIVPositiveLimitEID );
	readAndSkipPast( "AltStopRuleTotCohortEID", inputFile );
	fscanf( inputFile, "%ld", &eidInputs.totalCohortsLimitEID );

	//Read in Testing Administration inputs for EID
	readAndSkipPast( "EIDTestingAdminAssayUnknownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%d", &eidInputs.testingAdminAssay[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminAgeUnknownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%d", &eidInputs.testingAdminAge[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminProbPresentUnknownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%lf", &eidInputs.testingAdminProbPresent[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminProbNonMaternalUnknownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%lf", &eidInputs.testingAdminProbNonMaternalCaregiver[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminEIDVisitUnknownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++){
		fscanf( inputFile, "%d", &tempBool);
		eidInputs.testingAdminIsEIDVisit[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE][i] = (bool) tempBool;
	}
	readAndSkipPast( "EIDTestingAdminReofferUnknownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++){
		fscanf( inputFile, "%d", &tempBool);
		eidInputs.testingAdminReofferTestIfMissed[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE][i] = (bool) tempBool;
	}


	readAndSkipPast( "EIDTestingAdminAssayKnownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%d", &eidInputs.testingAdminAssay[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminAgeKnownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%d", &eidInputs.testingAdminAge[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminProbPresentKnownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%lf", &eidInputs.testingAdminProbPresent[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminProbNonMaternalKnownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++)
		fscanf( inputFile, "%lf", &eidInputs.testingAdminProbNonMaternalCaregiver[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE][i]);
	readAndSkipPast( "EIDTestingAdminEIDVisitKnownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++){
		fscanf( inputFile, "%d", &tempBool);
		eidInputs.testingAdminIsEIDVisit[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE][i] = (bool) tempBool;
	}
	readAndSkipPast( "EIDTestingAdminReofferKnownPos", inputFile );
	for(i = 0; i < EID_NUM_ASSAYS; i++){
		fscanf( inputFile, "%d", &tempBool);
		eidInputs.testingAdminReofferTestIfMissed[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE][i] = (bool) tempBool;
	}

	//Read in age of seroreversion
	readAndSkipPast( "EIDAgeSeroreversion", inputFile );
	fscanf( inputFile, "%d %d", &eidInputs.ageOfSeroreversionMean, &eidInputs.ageOfSeroreversionStdDev);

	//Read in prob multipliers
	readAndSkipPast( "EIDMultProbPresentIfMissedVisit", inputFile );
	fscanf( inputFile, "%lf", &eidInputs.probMultMissedVisit);
	readAndSkipPast( "EIDMultProbOfferIfNonMaternal", inputFile );
	fscanf( inputFile, "%lf", &eidInputs.probMultNonMaternalCaregiver);
	readAndSkipPast( "EIDProbKnowledgePriorResult", inputFile );
	fscanf( inputFile, "%lf", &eidInputs.probKnowedgePriorResult);

	//Read in visit costs
	readAndSkipPast( "EIDCostVisit", inputFile );
	for(i = 0; i < EID_COST_VISIT_NUM; i++)
		fscanf( inputFile, "%lf", &eidInputs.costVisit[i]);

	/** Pediatric HIV TEST Cohort Characteristics */
	readAndSkipPast( "EIDProbDetectionOnOI", inputFile );
	for (i = 0; i < OI_NUM; ++i) {
		fscanf( inputFile, "%lf", &eidInputs.probHIVDetectionWithOI[i] );
	}

	readAndSkipPast( "EIDProbOIDetectionConfirmedLabTest", inputFile );
	fscanf( inputFile, "%lf", &eidInputs.probHIVDetectionWithOIConfirmedByLab );

	readAndSkipPast( "EIDOILabTestMonthsThreshold", inputFile );
	fscanf( inputFile, "%d", &eidInputs.hivDetectionWithOIMonthsThreshold );

	readAndSkipPast( "EIDOIAssayUnknownPos", inputFile );
	fscanf( inputFile, "%d %d", &eidInputs.hivDetectionWithOIAssayBeforeN1[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE],
			&eidInputs.hivDetectionWithOIAssayAfterN1[PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE]);

	readAndSkipPast( "EIDOIAssayKnownPos", inputFile );
	fscanf( inputFile, "%d %d", &eidInputs.hivDetectionWithOIAssayBeforeN1[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE],
			&eidInputs.hivDetectionWithOIAssayAfterN1[PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE]);

	/** Infant HIV Proph */

	for (int prophNum = 0; prophNum < INFANT_HIV_PROPHS_NUM; ++prophNum){
        EIDInputs::InfantHIVProph &infantProph = eidInputs.infantHIVProphs[prophNum];

        // Eligibility
        sprintf(tmpBuf, "InfantHIVProph%dEnable", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &tempBool);
        infantProph.infantProphEnabled = (bool) tempBool;

        sprintf(tmpBuf, "InfantHIVProph%dMaxAge", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphMaxAge);

        sprintf(tmpBuf, "InfantHIVProph%dMaternalHIVStatusKnown", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMaternalStatusKnown);
        sprintf(tmpBuf, "InfantHIVProph%dMaternalHIVStatusPositive", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMaternalStatusPos);
        sprintf(tmpBuf, "InfantHIVProph%dMotherOnART", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMotherOnART);
        sprintf(tmpBuf, "InfantHIVProph%dMaternalVSKnownDelivery", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMaternalVSKnownDelivery);
        sprintf(tmpBuf, "InfantHIVProph%dMotherSuppressedDelivery", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMotherSuppressedDelivery);
        sprintf(tmpBuf, "InfantHIVProph%dMotherHVLHighDelivery", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMotherHVLHighDelivery);
        sprintf(tmpBuf, "InfantHIVProph%dMaternalVSKnown", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMaternalVSKnown);
        sprintf(tmpBuf, "InfantHIVProph%dMotherSuppressed", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMotherSuppressed);
        sprintf(tmpBuf, "InfantHIVProph%dMotherHVLHigh", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEligibilityMotherHVLHigh);
        sprintf(tmpBuf, "InfantHIVProph%dStopOnPosEID", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &tempBool);
        infantProph.infantProphEligibilityStopOnPosEID = (bool) tempBool;

	//Administration
        sprintf(tmpBuf, "InfantHIVProph%dAdminProb", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        for (i = 0; i < INFANT_PROPH_AGES_NUM; ++i) {
            fscanf( inputFile, "%lf", &infantProph.infantProphProb[i] );
        }
        sprintf(tmpBuf, "InfantHIVProph%dAdminEIDNegMonths", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        for (i = 0; i < INFANT_PROPH_AGES_NUM; ++i) {
            fscanf( inputFile, "%d", &infantProph.infantProphNegEIDMonths[i] );
	}

	//Cost
        sprintf(tmpBuf, "InfantHIVProph%dStartupCost", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        for (i = 0; i < INFANT_PROPH_COST_AGES_NUM; ++i) {
            fscanf( inputFile, "%lf", &infantProph.infantProphStartupCost[i] );
        }
        sprintf(tmpBuf, "InfantHIVProph%dDoseCost", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        for (i = 0; i < INFANT_PROPH_COST_AGES_NUM; ++i) {
            fscanf( inputFile, "%lf", &infantProph.infantProphDoseCost[i] );
        }
        sprintf(tmpBuf, "InfantHIVProph%dEffHorizon", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphEffHorizon);
        sprintf(tmpBuf, "InfantHIVProph%dDecayTime", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%d", &infantProph.infantProphDecayTime);
        sprintf(tmpBuf, "InfantHIVProph%dProbSubsequentEff", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%lf", &infantProph.infantProphProbEff);

	//Tox
        sprintf(tmpBuf, "InfantHIVProph%dMajorTox", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%lf", &infantProph.infantProphMajorToxProb);
        fscanf( inputFile, "%lf", &infantProph.infantProphMajorToxQOLMod);
        fscanf( inputFile, "%lf", &infantProph.infantProphMajorToxCost);
        fscanf( inputFile, "%lf", &infantProph.infantProphMajorToxDeathRateRatio);
		fscanf( inputFile, "%d", &infantProph.infantProphMajorToxDeathRateRatioDuration);
        fscanf( inputFile, "%lf", &infantProph.infantProphMajorToxDeathCost);
        fscanf( inputFile, "%d", &tempBool);
        infantProph.infantProphMajorToxStopOnTox = (bool) tempBool;

        sprintf(tmpBuf, "InfantHIVProph%dMinorTox", prophNum);
        readAndSkipPast( tmpBuf, inputFile );
        fscanf( inputFile, "%lf", &infantProph.infantProphMinorToxProb);
        fscanf( inputFile, "%lf", &infantProph.infantProphMinorToxQOLMod);
        fscanf( inputFile, "%lf", &infantProph.infantProphMinorToxCost);


	//PP VTHIV mult
		sprintf(tmpBuf,"InfantHIVProph%dPPVTHIVThreshold", prophNum );
		readAndSkipPast( tmpBuf, inputFile);
		fscanf( inputFile, "%d", &infantProph.infantProphVTHIVPPMultAgeThreshold);

		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOnARTPre", prophNum);
		readAndSkipPast2( tmpBuf, "Suppressed", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOnARTSuppressed[i][0]);

		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOnARTPre", prophNum);
		readAndSkipPast2( tmpBuf, "NotSuppressedLowHVL", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOnARTNotSuppressedLowHVL[i][0]);
		sprintf(tmpBuf,"InfantHIVProph%dPPVTHIVMultOnARTPre", prophNum);
		readAndSkipPast2( tmpBuf, "NotSuppressedHighHVL", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOnARTNotSuppressedHighHVL[i][0]);

		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOffARTPre", prophNum);
		readAndSkipPast2( tmpBuf, "EBF", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_EXCL][0]);
		}
		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOffARTPre", prophNum);
		readAndSkipPast2( tmpBuf, "MBF", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_MIXED][0]);
		}
		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOffARTPre", prophNum);
		readAndSkipPast2( tmpBuf, "CBF",inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_COMP][0]);
		}
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_REPL][0] = 0.0;
		}

		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOnARTPost", prophNum);
		readAndSkipPast2( tmpBuf, "Suppressed", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOnARTSuppressed[i][1]);
		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOnARTPost", prophNum);
		readAndSkipPast2( tmpBuf, "NotSuppressedLowHVL", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOnARTNotSuppressedLowHVL[i][1]);
		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOnARTPost", prophNum);
		readAndSkipPast2( tmpBuf, "NotSuppressedHighHVL", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++)
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOnARTNotSuppressedHighHVL[i][1]);

		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOffARTPost", prophNum);
		readAndSkipPast2( tmpBuf, "EBF", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_EXCL][1]);
		}
		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOffARTPost", prophNum);
		readAndSkipPast2( tmpBuf, "MBF", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_MIXED][1]);
		}
		sprintf(tmpBuf, "InfantHIVProph%dPPVTHIVMultOffARTPost", prophNum);
		readAndSkipPast2( tmpBuf, "CBF", inputFile);
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			fscanf(inputFile,"%lf", &infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_COMP][1]);
		}
		for (int i = 0; i < PEDS_MATERNAL_STATUS_NUM; i++){
			infantProph.infantProphVTHIVPPMultMotherOffART[i][PEDS_BF_REPL][1] = 0.0;
		}
	} // end reading in infant HIV proph inputs


	for (int testNum = 0; testNum < EID_NUM_TESTS; testNum++) {
		EIDInputs::EIDTest &eidTest = eidInputs.EIDTests[testNum];

		//read in whether this tests costs (cost of test, result return etc) should count in the EID costs or EID test costs category. If not they are counted as misc costs
		sprintf(tmpBuf, "EIDHIVTest%dCountTowardEIDCosts", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%d", &tempBool);
		eidTest.includeInEIDCosts = (bool) tempBool;

		//Read in probabilities of being offered and accepting the test
		sprintf(tmpBuf, "EIDHIVTest%dProbOfferedTest", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestOfferProb);

		sprintf(tmpBuf, "EIDHIVTest%dProbAcceptTest", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestAcceptProb);

		//Read in test costs
		sprintf(tmpBuf, "EIDHIVTest%dTestCost", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestCost);

		sprintf(tmpBuf, "EIDHIVTest%dCostResult", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestResultReturnCost);

		sprintf(tmpBuf, "EIDHIVTest%dCostNegResult", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestNegativeResultReturnCost);

		sprintf(tmpBuf, "EIDHIVTest%dCostPosResult", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestPositiveResultReturnCost);

		//read in test characteristics
		sprintf(tmpBuf, "EIDHIVTest%dResultReturnProbLab", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestResultReturnProbLab);

		sprintf(tmpBuf, "EIDHIVTest%dResultReturnProbPatient", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDTestResultReturnProbPatient);

		sprintf(tmpBuf, "EIDHIVTest%dResultReturnTime", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%d %d", &eidTest.EIDTestReturnTimeMean, &eidTest.EIDTestReturnTimeStdDev);

		//Sensitivity
		sprintf(tmpBuf, "EIDHIVTest%dSensitivityIU", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSensitivityIU[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSensitivityIP", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSensitivityIP[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSensitivityPPBeforeSR", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSensitivityPPBeforeSRMotherInfectedPreDelivery[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSensitivityPPAfterSR", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSensitivityPPAfterSRMotherInfectedPreDelivery[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSensitivityPPDuringBF", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSensitivityPPMotherInfectedPostDelivery[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSensitivityMultiplierIfMaternalARTPregnancy", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSensitivityMultiplierMaternalART[i]);

		for (int prophNum = 0; prophNum < INFANT_HIV_PROPHS_NUM; ++prophNum){
            sprintf(tmpBuf, "EIDHIVTest%dSensitivityMultiplierInfantHIVProph%d", testNum, prophNum);
            readAndSkipPast( tmpBuf, inputFile );
            for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
                fscanf( inputFile, "%lf", &eidTest.EIDSensitivityMultiplierInfantHIVProph[prophNum][i]);
        }

		//Specificity
		sprintf(tmpBuf, "EIDHIVTest%dSpecificityHEUBeforeSR", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSpecificityHEUBeforeSRMotherInfectedPreDelivery[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSpecificityHEUAfterSR", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSpecificityHEUAfterSRMotherInfectedPreDelivery[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSpecificityUninfectedMother", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSpecificityMotherUninfected[i]);

		sprintf(tmpBuf, "EIDHIVTest%dSpecificityMotherInfectedBF", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
			fscanf( inputFile, "%lf", &eidTest.EIDSpecificityMotherInfectedPostDelivery[i]);

		for (int prophNum = 0; prophNum < INFANT_HIV_PROPHS_NUM; ++prophNum){
            sprintf(tmpBuf, "EIDHIVTest%dSpecificityMultiplierInfantHIVProph%d", testNum, prophNum);
            readAndSkipPast( tmpBuf, inputFile );
            for(i=0; i<EID_TEST_AGE_CATEGORY_NUM;i++)
                fscanf( inputFile, "%lf", &eidTest.EIDSpecificityMultiplierInfantHIVProph[prophNum][i]);
        }

		//Confirmatory Tests
		sprintf(tmpBuf, "EIDHIVTest%dConfAssay", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%d %d", &eidTest.EIDFirstConfirmatoryTestAssay, &eidTest.EIDSecondConfirmatoryTestAssay);

		sprintf(tmpBuf, "EIDHIVTest%dConfDelay", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%d %d", &eidTest.EIDFirstConfirmatoryTestDelay, &eidTest.EIDSecondConfirmatoryTestDelay);

		sprintf(tmpBuf, "EIDHIVTest%dProbLink", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDProbLinkage);

		sprintf(tmpBuf, "EIDHIVTest%dProbMaternalStatusBecomeKnownUponResult", testNum);
		readAndSkipPast( tmpBuf, inputFile );
		fscanf( inputFile, "%lf", &eidTest.EIDProbMaternalStatusKnownOnPosResult);
	} // end reading in EID HIV test inputs 
} /* end readEIDInputs */

/* readAdolescentInputs reads data from the Adolescent tab of the input sheet */
void SimContext::readAdolescentInputs() {
	int tempBool, i,j,k;
	char tmpBuf[256];

	// read in enable Adolescent and age transitions
	readAndSkipPast( "EnableAdolescent", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	adolescentInputs.enableAdolescent = (bool) tempBool;

	readAndSkipPast( "TransitionToAdult", inputFile );
	fscanf( inputFile, "%d", &tempBool);
	adolescentInputs.transitionToAdult = (bool) tempBool;
	readAndSkipPast( "AgeTransitionToAdult", inputFile );
	fscanf( inputFile, "%d", &adolescentInputs.ageTransitionToAdult );

	readAndSkipPast( "AgeTransitionFromPeds", inputFile );
	fscanf( inputFile, "%d", &adolescentInputs.ageTransitionFromPeds );

	readAndSkipPast( "AdolescentAgeBounds", inputFile );
	for (i=0;i<ADOLESCENT_NUM_AGES-1;i++)
		fscanf( inputFile, "%d", &adolescentInputs.ageBounds[i] );

	//Read in Adolescent Natural History
	for ( i = CD4_NUM_STRATA - 1; i >= 0; --i ) {
		sprintf( tmpBuf, "BslCD4Decl_Mean_Adolescent_%s", CD4_STRATA_STRS[i]);
		for ( j = HVL_NUM_STRATA - 1; j >= HVL_VLO; --j ){
			readAndSkipPast2( tmpBuf, HVL_STRATA_STRS[j], inputFile );
			for (k = 0;k<ADOLESCENT_NUM_AGES; k++)
				fscanf( inputFile, "%lf", &(adolescentInputs.monthlyCD4DeclineMean[i][j][k]) );
		}	
		sprintf( tmpBuf, "BslCD4Decl_StdDev_Adolescent_%s", CD4_STRATA_STRS[i]);
		for ( j = HVL_NUM_STRATA - 1; j >= HVL_VLO; --j ){
			readAndSkipPast2( tmpBuf, HVL_STRATA_STRS[j], inputFile );
			for (k = 0;k<ADOLESCENT_NUM_AGES; k++)
				fscanf( inputFile, "%lf", &(adolescentInputs.monthlyCD4DeclineStdDev[i][j][k]) );
		}	
	}
	
	//Prob OI
	for (i = 0; i < OI_NUM; i++){
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j) {
			sprintf(tmpBuf, "Prob_%s_NoHist_OffART_Adolescent", OI_STRS[i]);
			readAndSkipPast2( tmpBuf,CD4_STRATA_STRS[j], inputFile );
			for (k = 0; k < ADOLESCENT_NUM_AGES; k++)
				fscanf( inputFile, "%lf", &adolescentInputs.monthlyOIProbOffART[j][i][HIST_N][k]);
		}
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j) {
			sprintf(tmpBuf, "Prob_%s_WithHist_OffART_Adolescent", OI_STRS[i]);
			readAndSkipPast2( tmpBuf,CD4_STRATA_STRS[j], inputFile );
			for (k = 0; k < ADOLESCENT_NUM_AGES; k++)
				fscanf( inputFile, "%lf", &adolescentInputs.monthlyOIProbOffART[j][i][HIST_Y][k]);
		}
	}
	for (i = 0; i < OI_NUM; i++){
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j) {
			sprintf(tmpBuf, "Prob_%s_NoHist_OnART_Adolescent", OI_STRS[i]);
			readAndSkipPast2( tmpBuf,CD4_STRATA_STRS[j], inputFile );
			for (k = 0; k < ADOLESCENT_NUM_AGES; k++)
				fscanf( inputFile, "%lf", &adolescentInputs.monthlyOIProbOnART[j][i][HIST_N][k]);

		}
		for (j = CD4_NUM_STRATA - 1; j >= 0; --j) {
			sprintf(tmpBuf, "Prob_%s_WithHist_OnART_Adolescent", OI_STRS[i]);
			readAndSkipPast2( tmpBuf,CD4_STRATA_STRS[j], inputFile );
			for (k = 0; k < ADOLESCENT_NUM_AGES; k++)
				fscanf( inputFile, "%lf", &adolescentInputs.monthlyOIProbOnART[j][i][HIST_Y][k]);
		}
	}
	//HIV and ART death rate ratios
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "HIVDthRateRatio_Adolescent",CD4_STRATA_STRS[i], inputFile );
		for (j = 0;j<ADOLESCENT_NUM_AGES;j++)
			fscanf( inputFile, "%lf", &adolescentInputs.HIVDeathRateRatio[i][j]);
	}
	readAndSkipPast( "ARTDthRateRatio_Adolescent", inputFile );
	for (i = 0;i<ADOLESCENT_NUM_AGES;i++)
			fscanf( inputFile, "%lf", &adolescentInputs.ARTDeathRateRatio[i]);
	//Death rate ratios for generic risk factors 
	for (i = 0; i<RISK_FACT_NUM; i++){
		sprintf(tmpBuf, RISK_FACT_STRS[i]);
		readAndSkipPast2( "GenRiskDthRateRatio_Adolescent", tmpBuf, inputFile );
		for(j = 0;j<ADOLESCENT_NUM_AGES;j++){
			fscanf( inputFile, "%lf", &adolescentInputs.genericRiskDeathRateRatio[i][j]);
		}	
	}

	//Death rate ratios for acute cases of severe OIs and TB modeled as a severe OI
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "AcuteOIDthRateRatio_Adolescent",CD4_STRATA_STRS[i], inputFile );
		for (j = 0;j<ADOLESCENT_NUM_AGES;j++)
			fscanf( inputFile, "%lf", &adolescentInputs.acuteOIDeathRateRatio[i][j]);
	}
	for (i = CD4_NUM_STRATA - 1; i >= 0; --i) {
		readAndSkipPast2( "AcuteOIDthRateRatioTB_Adolescent",CD4_STRATA_STRS[i], inputFile );
		for (j = 0;j<ADOLESCENT_NUM_AGES;j++)
			fscanf( inputFile, "%lf", &adolescentInputs.acuteOIDeathRateRatioTB[i][j]);
	}
	//Death rate ratios for having a history of a severe OI or TB modeled as a severe OI, and their durations
	readAndSkipPast("SevrOI_HistDthRateRatio_Adolescent", inputFile);
	for (i = 0; i<ADOLESCENT_NUM_AGES; i++)
		fscanf( inputFile, "%lf", &(adolescentInputs.severeOIHistDeathRateRatio[i]));
	readAndSkipPast( "SevrOI_HistEffectDuration_Adolescent", inputFile);
	fscanf( inputFile, "%d", &(adolescentInputs.severeOIHistEffectDuration));
	readAndSkipPast("TB_OI_HistDthRateRatio_Adolescent", inputFile);
	for (i = 0; i<ADOLESCENT_NUM_AGES; i++)
		fscanf( inputFile, "%lf", &(adolescentInputs.TB_OIHistDeathRateRatio[i]));
	readAndSkipPast( "TB_OI_HistEffectDuration_Adolescent", inputFile);
	fscanf( inputFile, "%d", &(adolescentInputs.TB_OIHistEffectDuration));
	
	
} /* end readAdolescentInputs */



/* readAdolescentARTInputs reads data from the AdolescentART tab of the input sheet */
void SimContext::readAdolescentARTInputs() {
	char tmpBuf[256], buffer[256];
	int i, j, k, tempBool;
	double tempCost;
	FILE *file = inputFile;

	for (int artNum = 1; artNum <= ART_NUM_LINES; artNum++) {
		// read in regimen id num and name
		sprintf(tmpBuf, "ART%dIdAYA", artNum);
		readAndSkipPast( tmpBuf, file );
		int idNum;
		fscanf( file, " %d", &idNum );
		// skip to next regimen if this one is not specified
		if (idNum == NOT_APPL) {
			adolescentARTInputs[artNum - 1] = NULL;
			continue;
		}
		// create new regimen input structure
		adolescentARTInputs[artNum - 1] = new AdolescentARTInputs();
		AdolescentARTInputs &artInput = *(adolescentARTInputs[artNum - 1]);

		// read in one-time startup cost
		sprintf(tmpBuf, "AYAART%dInitCost", artNum);
		readAndSkipPast( tmpBuf, file );
		for (i = 0; i < ADOLESCENT_NUM_AGES; i++)
			fscanf( file, "%lf", &artInput.costInitial[i] );

		// read in monthly cost
		sprintf(tmpBuf, "AYAART%dMthCost", artNum);
		readAndSkipPast( tmpBuf, file );
		for (i = 0; i < ADOLESCENT_NUM_AGES; i++)
			fscanf( file, "%lf", &artInput.costMonthly[i] );

		// read in efficacy time horizon
		sprintf(tmpBuf, "AYAART%dEffTimeHorizon", artNum);
		readAndSkipPast( tmpBuf, file );
		for (i = 0; i < ADOLESCENT_NUM_AGES; i++)
			fscanf( file, "%d", &artInput.efficacyTimeHorizon[i] );

		// read in mth by which all would fail
		sprintf(tmpBuf, "AYAART%dMthForceFail", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &artInput.forceFailAtMonth );	

		sprintf(tmpBuf, "AYAART%dAgeBounds", artNum);
		readAndSkipPast( tmpBuf, inputFile );
		for (i=0;i<ADOLESCENT_NUM_ART_AGES-1;i++)
			fscanf( inputFile, "%d", &artInput.ageBounds[i]);

		// read in CD4 effect on ART
		sprintf(tmpBuf, "AYAART%dMthStageCD4Eff_Succ", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d %d", &(artInput.stageBoundsCD4ChangeOnSuppART[0]), &(artInput.stageBoundsCD4ChangeOnSuppART[1]));
		for(i=0; i < 3; i++){
			sprintf(tmpBuf, "AYAART%dCD4EffSlope_Succ_Mean%d", artNum,i);
			readAndSkipPast( tmpBuf, file );
			for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
				fscanf( file, "%lf",&(artInput.CD4ChangeOnSuppARTMean[i][j]));

			sprintf(tmpBuf, "AYAART%dCD4EffSlope_Succ_SD%d", artNum,i);
			readAndSkipPast( tmpBuf, file );
			for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
				fscanf( file, "%lf",&(artInput.CD4ChangeOnSuppARTStdDev[i][j]));
		}

		sprintf(tmpBuf, "AYAART%dMthStageCD4Eff_Fail", artNum);
		readAndSkipPast( tmpBuf, file );
		fscanf( file, "%d", &(artInput.stageBoundCD4ChangeOnARTFail));
		for(i=0; i < 2; i++){
			sprintf(tmpBuf, "AYAART%dCD4EffMult_Fail%d", artNum,i);
			readAndSkipPast( tmpBuf,file);
			for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
				fscanf( file, "%lf",&(artInput.CD4MultiplierOnFailedART[i][j]));
		}

		sprintf(tmpBuf, "AYAART%dMthCD4SecStdDev", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &artInput.secondaryCD4ChangeOnARTStdDev[j]);

		// read in CD4 effect off ART
		sprintf(tmpBuf, "AYAART%dCD4EffOffART_Supp_PreSetpoint", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &(artInput.monthlyCD4MultiplierOffARTSuppPreSetpoint[j]));

		sprintf(tmpBuf, "AYAART%dCD4EffOffART_Supp_PostSetpoint", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &(artInput.monthlyCD4MultiplierOffARTSuppPostSetpoint[j]));

		sprintf(tmpBuf, "AYAART%dCD4EffOffART_Fail_PreSetpoint", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &(artInput.monthlyCD4MultiplierOffARTFailPreSetpoint[j]));

		sprintf(tmpBuf, "AYAART%dCD4EffOffART_Fail_PostSetpoint", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &(artInput.monthlyCD4MultiplierOffARTFailPostSetpoint[j]));

		// read in HVL change probability
		sprintf(tmpBuf, "AYAART%dHVLChgProb_Supp", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &(artInput.monthlyProbHVLChangeSupp[j]));

		sprintf(tmpBuf, "AYAART%dHVLChgProb_SuppStrataChg", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%d", &(artInput.monthlyNumStrataHVLChangeSupp[j]));

		sprintf(tmpBuf, "AYAART%dHVLChgProb_Fail", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%lf", &(artInput.monthlyProbHVLChangeFail[j]));

		sprintf(tmpBuf, "AYAART%dHVLChgProb_FailStrataChg", artNum);
		readAndSkipPast( tmpBuf, file );
		for (j = 0; j < ADOLESCENT_NUM_ART_AGES; j++)
			fscanf( file, "%d", &(artInput.monthlyNumStrataHVLChangeFail[j]));


		for (i = 0; i < ADOLESCENT_NUM_ART_AGES; i++){
			sprintf(tmpBuf, "AYAART%dPropMthCostNonResponders_Age%d", artNum, i);
			readAndSkipPast(tmpBuf, file);
			fscanf(file, "%lf", &(artInput.propMthCostNonResponders[i]));

			sprintf(tmpBuf, "AYAART%dProbRestartRegimen_Age%d", artNum, i);
			readAndSkipPast(tmpBuf, file);
			fscanf(file, "%lf %lf %lf", &(artInput.probRestartARTRegimenAfterFailure[RESP_TYPE_FULL][i]),&(artInput.probRestartARTRegimenAfterFailure[RESP_TYPE_PARTIAL][i]),&(artInput.probRestartARTRegimenAfterFailure[RESP_TYPE_NON][i]));

			sprintf(tmpBuf, "AYAART%dHetPropRespRegCoeff_Age%d", artNum, i);
			readAndSkipPast(tmpBuf, file);
			fscanf(file, "%lf %lf", &(artInput.propRespondARTRegimenLogitMean[i]), &(artInput.propRespondARTRegimenLogitStdDev[i]));

			sprintf(tmpBuf, "AYAART%dHetOutcomes", artNum);
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("Supp",file);
			fscanf(file, "%lf %lf %lf %lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_SUPP][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_SUPP][1][i]),&(artInput.responseTypeValues[HET_OUTCOME_SUPP][0][i]),&(artInput.responseTypeValues[HET_OUTCOME_SUPP][1][i]), &(artInput.responseTypeExponents[HET_OUTCOME_SUPP][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("LateFail",file);
			fscanf(file, "%lf %lf %lf %lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_LATEFAIL][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_LATEFAIL][1][i]),&(artInput.responseTypeValues[HET_OUTCOME_LATEFAIL][0][i]),&(artInput.responseTypeValues[HET_OUTCOME_LATEFAIL][1][i]), &(artInput.responseTypeExponents[HET_OUTCOME_LATEFAIL][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("ARTEffectOI",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_OI][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_OI][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("ARTEffectCHRMs",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_CHRMS][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_CHRMS][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("ARTEffectMort",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_MORT][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_ARTEFFECT_MORT][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("Resist",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_RESIST][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_RESIST][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("Tox",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_TOX][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_TOX][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("Cost",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_COST][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_COST][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("RestartAfterFail",file);
			fscanf(file, "%lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_RESTART][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_RESTART][1][i]));
			readAndSkipPast(tmpBuf, file);
			readAndSkipPast("Resuppression",file);
			fscanf(file, "%lf %lf %lf %lf %lf", &(artInput.responseTypeThresholds[HET_OUTCOME_RESUPP][0][i]), &(artInput.responseTypeThresholds[HET_OUTCOME_RESUPP][1][i]),&(artInput.responseTypeValues[HET_OUTCOME_RESUPP][0][i]),&(artInput.responseTypeValues[HET_OUTCOME_RESUPP][1][i]), &(artInput.responseTypeExponents[HET_OUTCOME_RESUPP][i]));

			sprintf(tmpBuf, "AYAART%dARTEffectOnFail_Age%d", artNum, i);
        	readAndSkipPast(tmpBuf, file);
        	fscanf(file, "%d", &tempBool);
        	artInput.applyARTEffectOnFailed[i] = (bool) tempBool;
		}
	}
} /* end readAdolescentARTInputs */

/* readAndSkipPast and readAndSkipPast2 skip over the given text in the input file - in other words, they move to a position from which the model will grab items that come next in the buffer */
bool SimContext::readAndSkipPast(const char* searchStr, FILE* file) {
	char temp[513];

	fscanf(file, "%512s", temp);

	while ( strcmp(temp, searchStr) != 0 ) {

		fscanf(file, "%512s", temp);

		if ( feof(file) ) {
			if(counter<=100){
			printf("\nWARNING: unexpected end of input file. Looking for %s",searchStr);
			counter++;
			}

			return false;
		}

	}
	return true;
}  // readAndSkipPast
/* readAndSkipPast2 is like readAndSkipPast but skips past two strings in the buffer */
bool SimContext::readAndSkipPast2( const char* searchStr1, const char* searchStr2, FILE* file ) {
	bool ret = readAndSkipPast(searchStr1, file);
	if (ret == true)
		ret = readAndSkipPast(searchStr2, file);
	return ret;
}  // readAndSkipPast2
