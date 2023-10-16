#include "include.h"

/** \brief Constructor takes run name and associated simulation context pointer as parameters
 *
 * \param runName a string representing the run name associated with this RunStats object
 * \param simContext a pointer to the SimContext representing the inputs associated with this RunStats object*/
RunStats::RunStats(string runName, SimContext *simContext) {

	statsFileName = runName;
	statsFileName.append(CepacUtil::FILE_EXTENSION_FOR_OUTPUT);

	orphanFileName = runName;
	orphanFileName.append(CepacUtil::FILE_EXTENSION_FOR_ORPHAN_OUTPUT);
	this->simContext = simContext;

	initRunStats(false);
} /* end Constructor */

/** \brief Destructor clears vectors and frees the allocated TimeSummary objects */
RunStats::~RunStats(void) {
	for (vector<TimeSummary *>::iterator s = timeSummaries.begin(); s != timeSummaries.end(); s++) {
		TimeSummary *summary = *s;
		delete summary;
	}
	for (vector<OrphanStats *>::iterator s = orphanStats.begin(); s != orphanStats.end(); s++) {
		OrphanStats *summary = *s;
		delete summary;
	}
	timeSummaries.clear();
	orphanStats.clear();
	patients.clear();
} /* end Destructor */


/** \brief initRunStats Initializes all the run stats
 *
* if isDynamic is True the time summary is not fully cleared; the dynamic transmission incidence outputs from the warmup run are kept to use in calculating monthly transmission rate multipliers
*/
void RunStats::initRunStats(bool isDynamic) {
	/** Clear the vectors for patient and time summaries */
		patients.clear();
		if (!isDynamic)
			timeSummaries.clear();
		else{
			for (vector<TimeSummary *>::iterator s = timeSummaries.begin(); s != timeSummaries.end(); s++) {
				TimeSummary *summary = *s;
				initTimeSummary(summary, isDynamic);
			}
		}
		orphanStats.clear();

		/** Initialize the statistics subclasses */
		initPopulationSummary();
		initHIVScreening();
		initSurvivalStats();
		initInitialDistributions();
		initCHRMsStats();
		initOIStats();
		initDeathStats();
		initOverallSurvival();
		initOverallCosts();
		initTBStats();
		initLTFUStats();
		initProphStats();
		initARTStats();
}; /* end initRunStats */



/** \brief finalizeStats calculate all aggregate statistics and values to be outputted
 *
 * The finalize functions calculate all averages and standard deviations.
 *
 * Calls:
 * - RunStats::finalizePopulationSummary();
 * - RunStats::finalizeHIVScreening();
 * - RunStats::finalizeSurvivalStats();
 * - RunStats::finalizeInitialDistributions();
 * - RunStats::finalizeCHRMsStats();
 * - RunStats::finalizeOIStats();
 * - RunStats::finalizeDeathStats();
 * - RunStats::finalizeOverallSurvival();
 * - RunStats::finalizeOverallCosts();
 * - RunStats::finalizeTBStats();
 * - RunStats::finalizeLTFUStats();
 * - RunStats::finalizeProphStats();
 * - RunStats::finalizeARTStats();
 * - RunStats::finalizeTimeSummaries();
*/
void RunStats::finalizeStats() {
	finalizePopulationSummary();
	finalizeHIVScreening();
	finalizeSurvivalStats();
	finalizeInitialDistributions();
	finalizeCHRMsStats();
	finalizeOIStats();
	finalizeDeathStats();
	finalizeOverallSurvival();
	finalizeOverallCosts();
	finalizeTBStats();
	finalizeLTFUStats();
	finalizeProphStats();
	finalizeARTStats();
	finalizeTimeSummaries();
}; /* end finalizeStats */

/** \brief writeStatsFile outputs all statistics to the stats file
 *
 *  * Calls:
 * - RunStats::writePopulationSummary();
 * - RunStats::writeHIVScreening();
 * - RunStats::writeSurvivalStats();
 * - RunStats::writeInitialDistributions();
 * - RunStats::writeCHRMsStats();
 * - RunStats::writeOIStats();
 * - RunStats::writeDeathStats();
 * - RunStats::writeOverallSurvival();
 * - RunStats::writeOverallCosts();
 * - RunStats::writeTBStats();
 * - RunStats::writeLTFUStats();
 * - RunStats::writeProphStats();
 * - RunStats::writeARTStats();
 * - RunStats::writeTimeSummaries();
* - RunStats::writeOrphanStats();*/
void RunStats::writeStatsFile() {
	CepacUtil::changeDirectoryToResults();
	statsFile = CepacUtil::openFile(statsFileName.c_str(), "w");
	if (statsFile == NULL) {
		statsFileName.append("-tmp");
		statsFile = CepacUtil::openFile(statsFileName.c_str(), "w");
		if (statsFile == NULL) {
			string errorString = "   ERROR - Could not write stats or temporary stats file";
			throw errorString;
		}
	}

	writePopulationSummary();
	writeHIVScreening();
	writeSurvivalStats();
	writeInitialDistributions();
	writeCHRMsStats();
	writeOIStats();
	writeDeathStats();
	writeOverallSurvival();
	writeOverallCosts();
	if (simContext->getTBInputs()->enableTB)
		writeTBStats();
	writeLTFUStats();
	writeProphStats();
	writeARTStats();
	writeTimeSummaries();

	CepacUtil::closeFile(statsFile);

	/** write orphans file if required */
	if(simContext->getCHRMsInputs()->enableOrphans && simContext->getCHRMsInputs()->showOrphansOutput){
		CepacUtil::changeDirectoryToResults();
		orphanFile = CepacUtil::openFile(orphanFileName.c_str(), "w");
		if (orphanFile == NULL) {
			orphanFileName.append("-tmp");
			orphanFile = CepacUtil::openFile(orphanFileName.c_str(), "w");
			if (orphanFile == NULL) {
				string errorString = "   ERROR - Could not write orphan output or temporary stats file";
				throw errorString;
			}
		}

		writeOrphanStats();
		CepacUtil::closeFile(orphanFile);
	}
} /* end writeStatsFile */

/** \brief initPopulationSummary initializes the PopulationSummary object */
void RunStats::initPopulationSummary() {
	popSummary.numCohorts = 0;
	popSummary.numCohortsHIVPositive = 0;
	popSummary.costsSum = 0;
	popSummary.costsAverage = 0;
	popSummary.costsSumSquares = 0;
	popSummary.costsStdDev = 0;
	popSummary.costsLowerBound = DBL_MAX;
	popSummary.costsUpperBound = DBL_MIN;
	popSummary.LMsSum = 0;
	popSummary.LMsAverage = 0;
	popSummary.LMsSumSquares = 0;
	popSummary.LMsStdDev = 0;
	popSummary.LMsLowerBound = DBL_MAX;
	popSummary.LMsUpperBound = DBL_MIN;
	popSummary.QALMsSum = 0;
	popSummary.QALMsAverage = 0;
	popSummary.QALMsSumSquares = 0;
	popSummary.QALMsStdDev = 0;
	popSummary.QALMsLowerBound = DBL_MAX;
	popSummary.QALMsUpperBound = DBL_MIN;

	for(int i = 0; i<SimContext::MAX_NUM_SUBCOHORTS; i++){
		popSummary.costsSumCohortParsing[i] = 0;
		popSummary.costsAverageCohortParsing[i] = 0;
		popSummary.costsSumSquaresCohortParsing[i] = 0;
		popSummary.costsStdDevCohortParsing[i] = 0;
		popSummary.LMsSumCohortParsing[i] = 0;
		popSummary.LMsAverageCohortParsing[i] = 0;
		popSummary.LMsSumSquaresCohortParsing[i] = 0;
		popSummary.LMsStdDevCohortParsing[i] = 0;
		popSummary.QALMsSumCohortParsing[i] = 0;
		popSummary.QALMsAverageCohortParsing[i] = 0;
		popSummary.QALMsSumSquaresCohortParsing[i] = 0;
		popSummary.QALMsStdDevCohortParsing[i] = 0;
	}

	for(int i = 0; i<SimContext::NUM_DISCOUNT_RATES; i++){
		popSummary.multDiscCostsSum[i] = 0;
		popSummary.multDiscCostsAverage[i] = 0;
		popSummary.multDiscCostsSumSquares[i] = 0;
		popSummary.multDiscLMsSum[i] = 0;
		popSummary.multDiscLMsAverage[i] = 0;
		popSummary.multDiscLMsSumSquares[i] = 0;
		popSummary.multDiscQALMsSum[i] = 0;
		popSummary.multDiscQALMsAverage[i] = 0;
		popSummary.multDiscQALMsSumSquares[i] = 0;
	}

	for (int i = 0; i <= SimContext::ART_NUM_LINES; i++) {
		popSummary.numFailART[i] = 0;
		popSummary.costsFailARTSum[i] = 0;
		popSummary.costsFailARTAverage[i] = 0;
		popSummary.LMsFailARTSum[i] = 0;
		popSummary.LMsFailARTAverage[i] = 0;
		popSummary.QALMsFailARTSum[i] = 0;
		popSummary.QALMsFailARTAverage[i] = 0;
	}
	popSummary.totalClinicVisits = 0;
	popSummary.costsHIVPositiveSum = 0;
	popSummary.costsHIVPositiveAverage = 0;
	popSummary.LMsHIVPositiveSum = 0;
	popSummary.LMsHIVPositiveAverage = 0;
	popSummary.QALMsHIVPositiveSum = 0;
	popSummary.QALMsHIVPositiveAverage = 0;
} /* end initPopulationSummary */

/** \brief initHIVScreening initializes the HIVScreening object */
void RunStats::initHIVScreening() {
	hivScreening.numPrevalentCases = 0;
	hivScreening.numIncidentCases = 0;
	for(int i = 0; i < SimContext::HIV_POS_PREP_STATES_NUM; i++)
		hivScreening.numIncidentCasesByPrEPState[i] = 0;
	for( int i = 0; i < 3; i++)
		hivScreening.numHIVExposed[i] = 0;
	hivScreening.numNeverHIVExposed = 0;	
	hivScreening.numHIVPositiveTotal = 0;
	hivScreening.numHIVNegativeAtInit = 0;
	for(int i = 0; i < SimContext::EVER_PREP_NUM; i++){
		hivScreening.numNeverHIVPositive[i] = 0;
	}
	for (int i = 0; i < SimContext::EVER_PREP_NUM; i++) {
		hivScreening.LMsHIVNegativeSum[i] = 0;
		hivScreening.LMsHIVNegativeAverage[i] = 0;
		hivScreening.QALMsHIVNegativeSum[i] = 0;
		hivScreening.QALMsHIVNegativeAverage[i] = 0;
	}
	for (int i = 0; i < SimContext::HIV_EXT_INF_NUM; i++) {
		hivScreening.numPatientsInitialHIVState[i] = 0;
		hivScreening.numTestsHIVState[i] = 0;
	}
	hivScreening.numAtDetectionPrevalent = 0;
	hivScreening.numAtDetectionIncident = 0;
	for(int i = 0; i < SimContext::PEDS_CD4_AGE_CAT_NUM; i++){
		hivScreening.numAtDetectionPrevalentCD4Metric[i] = 0;
		hivScreening.numAtDetectionIncidentCD4Metric[i] = 0;
		hivScreening.numAtLinkageCD4Metric[i] = 0;
	}
	hivScreening.numAtLinkage = 0;
	for (int j = 0; j < SimContext::HIV_POS_NUM; j++) {
		hivScreening.numAtDetectionPrevalentHIV[j] = 0;
		hivScreening.numAtDetectionIncidentHIV[j] = 0;
		hivScreening.numAtLinkageHIV[j] = 0;
		hivScreening.numLinkedAtInit[j] = 0;
		for(int i = 0; i < SimContext::PEDS_CD4_AGE_CAT_NUM; i++){
			hivScreening.numAtDetectionPrevalentHIVCD4Metric[j][i] = 0;
			hivScreening.numAtDetectionIncidentHIVCD4Metric[j][i] = 0;
			hivScreening.numAtLinkageHIVCD4Metric[j][i] = 0;
		}
		for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
			hivScreening.numAtDetectionPrevalentCD4[i] = 0;
			hivScreening.numAtDetectionPrevalentCD4HIV[i][j] = 0;
			hivScreening.numAtDetectionIncidentCD4[i] = 0;
			hivScreening.percentAtDetectionIncidentCD4[i] = 0;
			hivScreening.numAtDetectionIncidentCD4HIV[i][j] = 0;
			hivScreening.numAtLinkageCD4[i] = 0;
			hivScreening.numAtLinkageCD4HIV[i][j] = 0;
		}
		for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
			hivScreening.numAtDetectionPrevalentHVL[i] = 0;
			hivScreening.numAtDetectionPrevalentHVLHIV[i][j] = 0;
			hivScreening.numAtDetectionIncidentHVL[i] = 0;
			hivScreening.percentAtDetectionIncidentHVL[i] = 0;
			hivScreening.numAtDetectionIncidentHVLHIV[i][j] = 0;
			hivScreening.numAtLinkageHVL[i] = 0;
			hivScreening.numAtLinkageHVLHIV[i][j] = 0;
		}
	}
	hivScreening.CD4AtDetectionPrevalentSum = 0;
	hivScreening.CD4AtDetectionPrevalentAverage = 0;
	hivScreening.CD4AtDetectionIncidentSum = 0;
	hivScreening.CD4AtDetectionIncidentAverage = 0;
	hivScreening.CD4AtLinkageSum = 0;
	hivScreening.CD4AtLinkageAverage = 0;
	for (int i = 0; i < SimContext::HIV_POS_NUM; i++) {
		hivScreening.CD4AtDetectionPrevalentSumHIV[i] = 0;
		hivScreening.CD4AtDetectionPrevalentAverageHIV[i] = 0;
		hivScreening.CD4AtDetectionIncidentSumHIV[i] = 0;
		hivScreening.CD4AtDetectionIncidentAverageHIV[i] = 0;
		hivScreening.CD4AtLinkageSumHIV[i] = 0;
		hivScreening.CD4AtLinkageAverageHIV[i] = 0;
	}
	hivScreening.monthsToInfectionSum = 0;
	hivScreening.monthsToInfectionAverage = 0;
	hivScreening.monthsToInfectionSumSquares = 0;
	hivScreening.monthsToInfectionStdDev = 0;
	hivScreening.monthsAfterInfectionToDetectionSum = 0;
	hivScreening.monthsAfterInfectionToDetectionAverage = 0;
	hivScreening.monthsAfterInfectionToDetectionSumSquares = 0;
	hivScreening.monthsAfterInfectionToDetectionStdDev = 0;
	hivScreening.monthsToDetectionPrevalentSum = 0;
	hivScreening.monthsToDetectionPrevalentAverage = 0;
	hivScreening.monthsToDetectionPrevalentSumSquares = 0;
	hivScreening.monthsToDetectionPrevalentStdDev = 0;
	hivScreening.monthsToDetectionIncidentSum = 0;
	hivScreening.monthsToDetectionIncidentAverage = 0;
	hivScreening.monthsToDetectionIncidentSumSquares = 0;
	hivScreening.monthsToDetectionIncidentStdDev = 0;
	hivScreening.monthsToLinkageSum = 0;
	hivScreening.monthsToLinkageAverage = 0;
	hivScreening.monthsToLinkageSumSquares = 0;
	hivScreening.monthsToLinkageStdDev = 0;
	hivScreening.ageMonthsAtDetectionPrevalentSum = 0;
	hivScreening.ageMonthsAtDetectionPrevalentAverage = 0;
	hivScreening.ageMonthsAtDetectionPrevalentSumSquares = 0;
	hivScreening.ageMonthsAtDetectionPrevalentStdDev = 0;
	hivScreening.ageMonthsAtDetectionIncidentSum = 0;
	hivScreening.ageMonthsAtDetectionIncidentAverage = 0;
	hivScreening.ageMonthsAtDetectionIncidentSumSquares = 0;
	hivScreening.ageMonthsAtDetectionIncidentStdDev = 0;
	hivScreening.ageMonthsAtLinkageSum = 0;
	hivScreening.ageMonthsAtLinkageAverage = 0;
	hivScreening.ageMonthsAtLinkageSumSquares = 0;
	hivScreening.ageMonthsAtLinkageStdDev = 0;
	hivScreening.numDetectedGender[SimContext::GENDER_MALE] = 0;
	hivScreening.numDetectedGender[SimContext::GENDER_FEMALE] = 0;
	for (int i = 0; i < SimContext::HIV_DET_NUM; i++) {
		hivScreening.numDetectedPrevalentMeans[i] = 0;
		hivScreening.numDetectedIncidentMeans[i] = 0;
		hivScreening.numLinkedMeans[i] = 0;
		hivScreening.monthsToLinkageSumMeans[i]=0;
		hivScreening.monthsToLinkageAverageMeans[i]=0;
		hivScreening.monthsToLinkageSumSquaresMeans[i]=0;
		hivScreening.monthsToLinkageStdDevMeans[i]=0;
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		hivScreening.numDetectedByOIs[i] = 0;
		hivScreening.numDetectedByOIsPrevDetected[i] = 0;
	}
	for (int i = 0; i < SimContext::TEST_ACCEPT_NUM; i++) {
		for (int j = 0; j < SimContext::HIV_EXT_INF_NUM; j++) {
			hivScreening.numTestingAcceptProb[i][j] = 0;
		}
	}
	for (int i = 0; i < SimContext::HIV_TEST_FREQ_NUM; i++) {
		hivScreening.numTestingInterval[i] = 0;
	}
	hivScreening.numAcceptTest = 0;
	hivScreening.numRefuseTest = 0;
	hivScreening.numReturnForResults = 0;
	hivScreening.numNoReturnForResults = 0;

	hivScreening.numAcceptLabStaging = 0;
	hivScreening.numRefuseLabStaging = 0;
	hivScreening.numReturnForResultsLabStaging = 0;
	hivScreening.numNoReturnForResultsLabStaging = 0;
	hivScreening.numLinkLabStaging = 0;
	hivScreening.numNoLinkLabStaging = 0;
	for (int i=0; i< SimContext::HIV_POS_NUM;i++){
		hivScreening.numAcceptLabStagingHIVState[i] = 0;
		hivScreening.numReturnLabStagingHIVState[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++){
		hivScreening.numReturnLabStagingObsvCD4[i] = 0;
		hivScreening.numReturnLabStagingTrueCD4[i] = 0;
		hivScreening.numLinkLabStagingObsvCD4[i] = 0;
		hivScreening.numLinkLabStagingTrueCD4[i] = 0;
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++){
			hivScreening.numLinkLabStagingObsvTrueCD4[i][j] = 0;
			hivScreening.numReturnLabStagingObsvTrueCD4[i][j] = 0;
		}
	}

	hivScreening.numTestResultsPrevalent = 0;
	hivScreening.numTestResultsIncident = 0;
	hivScreening.numTestResultsHIVNegative = 0;
	for (int i = 0; i < SimContext::TEST_RESULT_NUM; i++) {
		hivScreening.numTestResultsPrevalentType[i] = 0;
		hivScreening.numTestResultsIncidentType[i] = 0;
		hivScreening.numTestResultsHIVNegativeType[i] = 0;
	}

	hivScreening.numEverPrEP = 0;
	hivScreening.numDropoutPrEP = 0;

	hivScreening.LMsFalsePositive = 0;
	hivScreening.LMsFalsePositiveLinked = 0;
} /* end initHIVScreening */

/** \brief initSurvivalStats initializes the SurvivalStats objects for each of the subgroups */
void RunStats::initSurvivalStats() {
	for (int i = 0; i < NUM_SURVIVAL_GROUPS; i++) {
		survivalStats[i].LMsHistogram.clear();
		survivalStats[i].LMsMin = DBL_MAX;
		survivalStats[i].LMsMax = DBL_MIN;
		survivalStats[i].LMsMedian = 0;
		survivalStats[i].LMsSumDeviationMedian = 0;
		survivalStats[i].LMsAverageDeviationMedian = 0;
		survivalStats[i].LMsSum = 0;
		survivalStats[i].LMsMean = 0;
		survivalStats[i].LMsSumDeviation = 0;
		survivalStats[i].LMsAverageDeviation = 0;
		survivalStats[i].LMsSumDeviationSquares = 0;
		survivalStats[i].LMsStdDev = 0;
		survivalStats[i].LMsVariance = 0;
		survivalStats[i].LMsSumDeviationCubes = 0;
		survivalStats[i].LMsSkew = 0;
		survivalStats[i].LMsSumDeviationQuads = 0;
		survivalStats[i].LMsKurtosis = 0;
		survivalStats[i].costsSum = 0;
		survivalStats[i].costsMean = 0;
		survivalStats[i].costsSumSquares = 0;
		survivalStats[i].costsStdDev = 0;
		survivalStats[i].QALMsSum = 0;
		survivalStats[i].QALMsMean = 0;
		survivalStats[i].QALMsSumSquares = 0;
		survivalStats[i].QALMsStdDev = 0;
	}
} /* end initSurvivalStats */

/** \brief initInitialDistributions initializes the InititalDistribution object */
void RunStats::initInitialDistributions() {
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		initialDistributions.numPatientsCD4Level[i] = 0;
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		initialDistributions.numPatientsHVLLevel[i] = 0;
		initialDistributions.numPatientsHVLSetpointLevel[i] = 0;
	}
	initialDistributions.sumInitialAgeMonths = 0;
	initialDistributions.averageInitialAgeMonths = 0;
	initialDistributions.numMalePatients = 0;
	initialDistributions.numFemalePatients = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		initialDistributions.numPriorOIHistories[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_RESPONSE_NUM_TYPES; i++) {
		initialDistributions.numARTResposneTypes[i] = 0;
	}
	for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
		initialDistributions.numRiskFactors[i] = 0;
	}
	for (int i = 0; i < SimContext::PEDS_HIV_NUM; i++) {
		for (int j = 0; j < SimContext::PEDS_MATERNAL_STATUS_NUM; j++) {
			initialDistributions.numInitialPediatrics[i][j] = 0;
		}
	}
} /* end initInitialDistributions */

/** \brief initCHRMsStats initializes the CHRMsStats object */
void RunStats::initCHRMsStats() {
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		chrmsStats.numPatientsWithCHRM[i]=0;
		chrmsStats.numPatientsWithCHRMHIVPos[i] = 0;
		chrmsStats.numPatientsWithCHRMHIVNeg[i] = 0;
		chrmsStats.numPrevalentCHRM[i] = 0;
		chrmsStats.numHIVNegWithPrevalentCHRMs = 0;
		chrmsStats.numPrevalentCHRMHIVNeg[i] = 0;
		chrmsStats.numIncidentCHRM[i] = 0;
		chrmsStats.numHIVNegWithIncidentCHRMs = 0;
		chrmsStats.numIncidentCHRMHIVneg[i] = 0;
		chrmsStats.numDeathsCHRM[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		chrmsStats.numPrevalentCD4[i] = 0;
		chrmsStats.numIncidentCD4[i] = 0;
		chrmsStats.numDeathsCD4[i] = 0;
	}
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			chrmsStats.numPrevalentCHRMCD4[i][j] = 0;
			chrmsStats.numIncidentCHRMCD4[i][j] = 0;
			chrmsStats.numDeathsCHRMCD4[i][j] = 0;
		}
	}
} /* end initCHRMsStats */

/** \brief initOIStats initializes the OIStats object */
void RunStats::initOIStats() {
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		oiStats.numPrimaryOIsOI[i] = 0;
		oiStats.numSecondaryOIsOI[i] = 0;
		oiStats.numDetectedOIsOI[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		oiStats.numPrimaryOIsCD4[i] = 0;
		oiStats.numSecondaryOIsCD4[i] = 0;
		oiStats.numDetectedOIsCD4[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		for (int j = 0; j < SimContext::OI_NUM; j++) {
			oiStats.numPrimaryOIsCD4OI[i][j] = 0;
			oiStats.numSecondaryOIsCD4OI[i][j] = 0;
			oiStats.numDetectedOIsCD4OI[i][j] = 0;
		}
	}
	oiStats.numOIEventsTotal = 0;
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		oiStats.numPatientsHVL[i] = 0;
		oiStats.numMonthsHVL[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		oiStats.numPatientsCD4[i] = 0;
		oiStats.numMonthsCD4[i] = 0;
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			oiStats.numPatientsHVLCD4[i][j] = 0;
			oiStats.numMonthsHVLCD4[i][j] = 0;
		}
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			oiStats.numPatientsOIHistoryHVL[i][j] = 0;
			oiStats.probPatientsOIHistoryHVL[i][j] = 0;
			oiStats.numMonthsOIHistoryHVL[i][j] = 0;
			oiStats.probMonthsOIHistoryHVL[i][j] = 0;
		}
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			oiStats.numPatientsOIHistoryCD4[i][j] = 0;
			oiStats.probPatientsOIHistoryCD4[i][j] = 0;
			oiStats.numMonthsOIHistoryCD4[i][j] = 0;
			oiStats.probMonthsOIHistoryCD4[i][j] = 0;
		}
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			for (int k = 0; k < SimContext::CD4_NUM_STRATA; k++) {
				oiStats.numPatientsOIHistoryHVLCD4[i][j][k] = 0;
				oiStats.probPatientsOIHistoryHVLCD4[i][j][k] = 0;
				oiStats.numMonthsOIHistoryHVLCD4[i][j][k] = 0;
				oiStats.probMonthsOIHistoryHVLCD4[i][j][k] = 0;
			}
		}
	}
} /* end initOIStats */

/** \brief initDeathStats initializes the DeathStats object */
void RunStats::initDeathStats() {
	deathStats.numDeathsUninfected = 0;
	deathStats.numARTToxDeaths = 0;
	for(int i = 0; i < SimContext::PEDS_CD4_AGE_CAT_NUM; i++){
		deathStats.numARTToxDeathsCD4Metric[i] = 0;
	}

	for (int i = 0; i < SimContext::DTH_NUM_CAUSES; i++) {
		deathStats.numDeathsHIVPosType[i] = 0;
		deathStats.numDeathsType[i] = 0;
        for(int j = 0; j < SimContext::OUTPUT_AGE_CAT_NUM; j++){
            deathStats.numDeathsTypeAge[i][j] = 0;
        }
	}

	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		deathStats.numDeathsCD4[i] = 0;
		for (int j = 0; j < SimContext::DTH_NUM_CAUSES; j++) {
			deathStats.numDeathsCD4Type[i][j] = 0;
		}
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		deathStats.numDeathsHVL[i] = 0;
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			deathStats.numDeathsHVLCD4[i][j] = 0;
		}
	}

	for (int i = 0; i < SimContext::HIV_CARE_NUM; i++){
		deathStats.numDeathsCare[i] = 0;
		for (int j = 0; j < SimContext::DTH_NUM_CAUSES; j++){
			deathStats.numDeathsCareType[i][j]=0;
		}
	}

	for (int i=0;i<SimContext::CD4_NUM_STRATA;i++){
		deathStats.numARTToxDeathsCD4[i] = 0;
		for (int j=0;j<SimContext::HVL_NUM_STRATA;j++){
			deathStats.numARTToxDeathsCD4HVL[i][j]=0;
			for (int k=0;k<SimContext::OI_NUM;k++){
				deathStats.numARTToxDeathsCD4HVLOIHist[i][j][k]=0;
			}
		}
	}

	deathStats.numHIVDeathsNoOIHistory = 0;
	deathStats.numHIVDeathsOIHistory = 0;
	deathStats.numBackgroundMortDeathsNoOIHistory = 0;
	deathStats.numBackgroundMortDeathsOIHistory = 0;
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		deathStats.numHIVDeathsNoOIHistoryCD4[i] = 0;
		deathStats.numHIVDeathsOIHistoryCD4[i] = 0;
		deathStats.numBackgroundMortDeathsNoOIHistoryCD4[i] = 0;
		deathStats.numBackgroundMortDeathsOIHistoryCD4[i] = 0;
	}
	deathStats.ARTToxDeathsCD4Sum=0;
	deathStats.ARTToxDeathsCD4SumSquares=0;
} /* end initDeathStats */

/** \brief initOverallSurvival initializes the OverallSurvival object */
void RunStats::initOverallSurvival() {
	overallSurvival.LMsNoOIHistory = 0;
	overallSurvival.LMsOIHistory = 0;
	overallSurvival.LMsTotal = 0;
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		overallSurvival.LMsNoOIHistoryCD4[i] = 0;
		overallSurvival.LMsOIHistoryCD4[i] = 0;
		overallSurvival.LMsTotalCD4[i] = 0;
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		overallSurvival.LMsHVL[i] = 0;
		overallSurvival.LMsHVLSetpoint[i] = 0;
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		overallSurvival.LMsNoOIHistoryOIs[i] = 0;
		overallSurvival.LMsOIHistoryOIs[i] = 0;
	}
	for (int i=0;i<SimContext::CHRM_NUM;i++){
		overallSurvival.LMsCHRMHistoryCHRMsHIVPos[i] = 0;
		overallSurvival.LMsCHRMHistoryCHRMsHIVNeg[i] = 0;
	}
	overallSurvival.LMsHIVPositive = 0;
	overallSurvival.QALMsHIVPositive = 0;
	for (int i = 0; i < SimContext::HIV_ID_NUM; i++) {
		overallSurvival.LMsHIVState[i] = 0;
		overallSurvival.QALMsHIVState[i] = 0;
	}
	overallSurvival.LMsInScreening = 0;
	overallSurvival.QALMsInScreening = 0;
	overallSurvival.LMsHIVNegativeOnPrEP = 0;
	overallSurvival.LMsInRegularCEPAC = 0;
	
	for (int i = 0; i < SimContext::GENDER_NUM; i++) {
		overallSurvival.LMsGender[i] = 0;
		overallSurvival.QALMsGender[i] = 0;
	}
} /* end initOverallSurvival */

/** \brief initOverallCosts initializes the OverallCosts object */
void RunStats::initOverallCosts() {
	overallCosts.costsNoOIHistory = 0;
	overallCosts.costsOIHistory = 0;
	overallCosts.costsTotal = 0;
	overallCosts.costsTBTotal = 0;
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		overallCosts.costsNoOIHistoryCD4[i] = 0;
		overallCosts.costsOIHistoryCD4[i] = 0;
		overallCosts.costsTotalCD4[i] = 0;
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		overallCosts.costsHVL[i] = 0;
		overallCosts.costsHVLSetpoint[i] = 0;
	}
	overallCosts.directCostsProph = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		overallCosts.directCostsProphOIs[i] = 0;
		for (int j = 0; j < SimContext::PROPH_NUM; j++) {
			overallCosts.directCostsProphOIsProph[i][j] = 0;
		}
	}
	overallCosts.directCostsART = 0;
	overallCosts.costsARTInit = 0;
	overallCosts.costsARTMonthly = 0;
	for (int j = 0; j < SimContext::NUM_DISCOUNT_RATES; j++){
		overallCosts.directCostsARTMultDisc[j] = 0;
		overallCosts.costsARTInitMultDisc[j] = 0;
		overallCosts.costsARTMonthlyMultDisc[j] = 0;
	}
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		overallCosts.directCostsARTLine[i] = 0;
		overallCosts.costsARTInitLine[i] = 0;
		overallCosts.costsARTMonthlyLine[i] = 0;
		for (int j = 0; j < SimContext::NUM_DISCOUNT_RATES; j++){
			overallCosts.directCostsARTLineMultDisc[i][j] = 0;
			overallCosts.costsARTInitLineMultDisc[i][j] = 0;
			overallCosts.costsARTMonthlyLineMultDisc[i][j] = 0;
		}
	}
	overallCosts.costsHIVPositive = 0;
	for (int i = 0; i < SimContext::HIV_ID_NUM; i++) {
		overallCosts.costsHIVState[i] = 0;
	}
	overallCosts.costsCD4Testing = 0;
	overallCosts.costsHVLTesting = 0;

	for (int j = 0; j < SimContext::NUM_DISCOUNT_RATES; j++){
		overallCosts.costsCD4TestingMultDisc[j] = 0;
		overallCosts.costsHVLTestingMultDisc[j] = 0;
	}

	for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
        overallCosts.costsTBProviderVisits[j] = 0;
        overallCosts.costsTBMedicationVisits[j] = 0;
	}

	overallCosts.costsTBTests = 0;

	for (int j = 0; j < SimContext::TB_NUM_TESTS; j++){
        overallCosts.costsTBTestsInit[j] = 0;
        overallCosts.costsTBTestsDST[j] = 0;
	}

	overallCosts.costsTBTreatment = 0;

	for (int j = 0; j < SimContext::TB_NUM_TREATMENTS; j++){
        overallCosts.costsTBTreatmentByLine[j] = 0;
        overallCosts.costsTBTreatmentToxByLine[j] = 0;
	}

	overallCosts.costsClinicVisits = 0;
	overallCosts.costsEIDVisits = 0;
	overallCosts.costsPrEP = 0;
	overallCosts.costsPrEPNeverHIV = 0;
	for(int j = 0; j < SimContext::HIV_POS_NEVER_PREP; j++)
		overallCosts.costsPrEPHIVPos[j] = 0;
	overallCosts.costsHIVScreeningTests = 0;
	overallCosts.costsHIVScreeningMisc = 0;
	overallCosts.costsLabStagingTests = 0;
	overallCosts.costsEIDTests = 0;
	overallCosts.costsInfantHIVProph = 0;
	overallCosts.costsLabStagingMisc = 0;
	overallCosts.costsEIDMisc = 0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		overallCosts.totalUndiscountedCosts[i] = 0;
		overallCosts.totalDiscountedCosts[i] = 0;
	}
	overallCosts.totalUndiscountedCostsUnclassified = 0;
	overallCosts.totalDiscountedCostsUnclassified = 0;


	overallCosts.costsDrugs = 0;
	overallCosts.costsInterventionStartup=0;
	overallCosts.costsInterventionMonthly=0;
	overallCosts.costsToxicity = 0;

	overallCosts.costsDrugsDiscounted = 0;
	overallCosts.costsToxicityDiscounted = 0;

	for (int i=0;i<SimContext::CHRM_NUM;i++){
		overallCosts.costsCHRMs[i]=0;
	}

	for (int i = 0; i < SimContext::GENDER_NUM; i++) {
		overallCosts.costsGender[i] = 0;
	}
} /* end initOverallCost */

/** \brief initTBStats initializes the TBStats object */
void RunStats::initTBStats() {
	for (int i = 0; i < SimContext::TB_NUM_STATES; i++)
		for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++)
			tbStats.numInfections[i][j] = 0;

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++) {
		tbStats.numReactivationsLatent[i] = 0;
		tbStats.numReactivationsPulmLatent[i] = 0;
		tbStats.numReactivationsExtraPulmLatent[i] = 0;

		tbStats.numRelapses[i] = 0;
		tbStats.numRelapsesPulm[i] = 0;
		tbStats.numRelapsesExtraPulm[i] = 0;

		tbStats.numTBSelfCures[i] = 0;
		tbStats.numDeaths[i] = 0;
		tbStats.numDeathsHIVPos[i] = 0;
		tbStats.numDeathsHIVNeg[i] = 0;
	}
	for (int i = 0; i < SimContext::TB_NUM_TREATMENTS; i++) {
		tbStats.numTreatmentMinorToxicity[i] = 0;
		tbStats.numTreatmentMajorToxicity[i] = 0;
	}
	for (int i = 0; i < SimContext::TB_NUM_PROPHS; i++) {
		tbStats.numProphMinorToxicity[i] = 0;
		tbStats.numProphMajorToxicity[i] = 0;
	}
	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++) {
		for (int j = 0; j < SimContext::TB_NUM_STATES; j++) {
			tbStats.numInStateAtEntryStrain[i][j] = 0;
		}
	}
	tbStats.numUninfectedTBAtEntry = 0;
	for(int i =0; i <2; i++){
		for(int j = 0; j < 2; j++){
			for (int k = 0; k < 2; k++){
				for (int l = 0; l < 2; l++){
					tbStats.numWithUnfavorableOutcome[i][j][k][l] = 0;
				}
			}
		}
	}				
	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++) {
		for (int j = 0; j < SimContext::TB_NUM_TREATMENTS; j++) {
			tbStats.numStartOnTreatment[i][j] = 0;
			tbStats.numDropoutTreatment[i][j] = 0;
			tbStats.numTransitionsToTBTreatmentDefault[i][j] = 0;
			tbStats.numFinishTreatment[i][j] = 0;
			tbStats.numCuredAtTreatmentFinish[i][j] = 0;
			tbStats.numIncreaseResistanceAtTreatmentStop[i][j] = 0;
		}
	}

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++){
		tbStats.numDSTTestResultsUninfectedTB[i] = 0;
		for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++)
			tbStats.numDSTTestResultsByTrueTBStrain[i][j] = 0;
	}
} /* end initTBStats */

/** \brief initLTFUStats initializes the LTFUStats object */
void RunStats::initLTFUStats() {
	ltfuStats.numPatientsLost = 0;
	ltfuStats.numPatientsReturned = 0;
	ltfuStats.numDeathsWhileLost = 0;
	ltfuStats.numLostToFollowUp = 0;
	ltfuStats.numReturnToCare = 0;
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		ltfuStats.numLostToFollowUpCD4[i] = 0;
		ltfuStats.numReturnToCareCD4[i] = 0;
		ltfuStats.numDeathsWhileLostCD4[i] = 0;
	}
	ltfuStats.monthsLostBeforeReturnSum = 0;
	ltfuStats.monthsLostBeforeReturnMean = 0;
	ltfuStats.monthsLostBeforeReturnSumSquares = 0;
	ltfuStats.monthsLostBeforeReturnStdDev = 0;
	ltfuStats.numLostToFollowUpPreART = 0;
	ltfuStats.numLostToFollowUpPostART = 0;
	ltfuStats.numReturnToCarePreART = 0;
	ltfuStats.numReturnToCarePostART = 0;
	ltfuStats.numDeathsWhileLostPreART = 0;
	ltfuStats.numDeathsWhileLostPostART = 0;
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		ltfuStats.numLostToFollowUpART[i] = 0;
		ltfuStats.numReturnOnPrevART[i] = 0;
		ltfuStats.numReturnOnNextART[i] = 0;
		ltfuStats.numDeathsWhileLostART[i] = 0;
	}
} /* end initLTFUStats */

/** \brief initProphStats initializes the ProphStats object */
void RunStats::initProphStats() {
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		prophStats.numMinorToxicityTotal[i] = 0;
		prophStats.numMajorToxicityTotal[i] = 0;
		for (int j = 0; j < SimContext::PROPH_NUM; j++) {
			prophStats.numMinorToxicity[i][j] = 0;
			prophStats.numMajorToxicity[i][j] = 0;
			for (int k = 0; k < SimContext::PROPH_NUM_TYPES; k++) {
				prophStats.trueCD4InitProphSum[k][i][j] = 0;
				prophStats.trueCD4InitProphMean[k][i][j] = 0;
				prophStats.observedCD4InitProphSum[k][i][j] = 0;
				prophStats.observedCD4InitProphMean[k][i][j] = 0;
				prophStats.numTimesInitProph[k][i][j] = 0;
				prophStats.numTimesInitProphWithObservedCD4[k][i][j] = 0;
				for(int l = 0; l < SimContext::PEDS_CD4_AGE_CAT_NUM; l++)	
					prophStats.numTimesInitProphCD4Metric[k][i][j][l] = 0;
			}
		}
	}
} /* end initProphStats */

/** \brief initARTStats initializes the ARTStats object */
void RunStats::initARTStats() {

	artStats.monthsSuppressed = 0;
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		artStats.monthsFailedHVL[i] = 0;
	}

	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		artStats.monthsSuppressedLine[i] = 0;
		artStats.monthsFailedLine[i] = 0;
		artStats.numOnARTAtInit[i] = 0;
		for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
			artStats.numOnARTAtInitCD4Metric[i][j] = 0;
			artStats.trueCD4AtInitSum[i][j] = 0;
			artStats.trueCD4AtInitMean[i][j] = 0;
		}
		artStats.observedCD4AtInitSum[i] = 0;
		artStats.numWithObservedCD4AtInit[i] = 0;
		artStats.observedCD4AtInitMean[i] = 0;
		for (int j = 0; j < SimContext::ART_EFF_NUM_TYPES; j++) {
			artStats.numDrawEfficacyAtInit[i][j] = 0;
		}
		for (int j = 0; j < SimContext::CD4_RESPONSE_NUM_TYPES; j++) {
			artStats.numCD4ResponseTypeAtInit[i][j] = 0;
		}
		for (int j = 0; j < SimContext::RISK_FACT_NUM; j++) {
			artStats.numWithRiskFactorAtInit[i][j] = 0;
		}
		artStats.numTrueFailure[i] = 0;
		for (int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++) {
			artStats.numTrueFailureCD4Metric[i][j] = 0;
			artStats.trueCD4AtTrueFailureSum[i][j] = 0;
			artStats.trueCD4AtTrueFailureMean[i][j] = 0;
		}	
		artStats.observedCD4AtTrueFailureSum[i] = 0;
		artStats.numWithObservedCD4AtTrueFailure[i] = 0;
		artStats.observedCD4AtTrueFailureMean[i] = 0;
		artStats.monthsToTrueFailureSum[i] = 0;
		artStats.monthsToTrueFailureMean[i] = 0;
		artStats.monthsToTrueFailureSumSquares[i] = 0;
		artStats.monthsToTrueFailureStdDev[i] = 0;

		for (int j = 0; j < SimContext::RESP_NUM_TYPES; j++) {
			for(int k=0;k<SimContext::HET_NUM_OUTCOMES;k++){
				artStats.numOnARTAtInitResp[i][k][j] = 0;
				artStats.trueCD4AtInitSumResp[i][k][j] = 0;
				artStats.trueCD4AtInitMeanResp[i][k][j] = 0;
				artStats.numWithObservedCD4AtInitResp[i][k][j] = 0;
				artStats.observedCD4AtInitSumResp[i][k][j] = 0;
				artStats.observedCD4AtInitMeanResp[i][k][j] = 0;
			}

			for (int k = 0; k < SimContext::ART_EFF_NUM_TYPES; k++) {
				for(int l=0;l<SimContext::HET_NUM_OUTCOMES;l++){
					artStats.numDrawEfficacyAtInitResp[i][k][l][j] = 0;
				}
			}

			for (int k = 0; k < SimContext::CD4_RESPONSE_NUM_TYPES; k++) {
				for (int l=0;l<SimContext::HET_NUM_OUTCOMES;l++){
					artStats.numCD4ResponseTypeAtInitResp[i][k][l][j] = 0;
				}
			}
			for (int k = 0; k < SimContext::RISK_FACT_NUM; k++) {
				for (int l=0;l <SimContext::HET_NUM_OUTCOMES;l++){
					artStats.numWithRiskFactorAtInitResp[i][k][l][j] = 0;
				}
			}

			for(int k=0;k<SimContext::HET_NUM_OUTCOMES;k++){
				artStats.numTrueFailureResp[i][k][j] = 0;
				artStats.trueCD4AtTrueFailureSumResp[i][k][j] = 0;
				artStats.trueCD4AtTrueFailureMeanResp[i][k][j] = 0;
				artStats.numWithObservedCD4AtTrueFailureResp[i][k][j] = 0;
				artStats.observedCD4AtTrueFailureSumResp[i][k][j] = 0;
				artStats.observedCD4AtTrueFailureMeanResp[i][k][j] = 0;
				artStats.monthsToTrueFailureSumResp[i][k][j] = 0;
				artStats.monthsToTrueFailureMeanResp[i][k][j] = 0;
				artStats.monthsToTrueFailureSumSquaresResp[i][k][j] = 0;
				artStats.monthsToTrueFailureStdDevResp[i][k][j] = 0;
			}
		}

		artStats.numSTIInterruptionsSum[i] = 0;
		artStats.numSTIInterruptionsMean[i] = 0;
		artStats.monthsOnSTIInterruptionSum[i] = 0;
		artStats.monthsOnSTIInterruptionMean[i] = 0;
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			artStats.monthsFailedLineHVL[i][j] = 0;
		}
		artStats.numObservedFailure[i] = 0;
		for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
			artStats.numObservedFailureCD4Metric[i][j] = 0;
			artStats.trueCD4AtObservedFailureSum[i][j] = 0;
			artStats.trueCD4AtObservedFailureMean[i][j] = 0;
		}	
		artStats.numObservedFailureAfterTrue[i] = 0;
		artStats.numNeverObservedFailure[i] = 0;
		artStats.numObservedCD4atObservedFailure[i] = 0;
		artStats.observedCD4AtObservedFailureSum[i] = 0;
		artStats.observedCD4AtObservedFailureMean[i] = 0;
		artStats.monthsToObservedFailureSum[i] = 0;
		artStats.monthsToObservedFailureMean[i] = 0;
		artStats.monthsToObservedFailureSumSquares[i] = 0;
		artStats.monthsToObservedFailureStdDev[i] = 0;
		for (int j = 0; j < SimContext::ART_NUM_FAIL_TYPES; j++) {
			artStats.numObservedFailureType[i][j] = 0;
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
				artStats.numObservedFailureTypeCD4Metric[i][j][k] = 0;
				artStats.trueCD4AtObservedFailureSumType[i][j][k] = 0;
				artStats.trueCD4AtObservedFailureMeanType[i][j][k] = 0;
			}	
			artStats.numObservedFailureAfterTrueType[i][j] = 0;
			artStats.numObservedCD4atObservedFailureType[i][j] = 0;
			artStats.observedCD4AtObservedFailureSumType[i][j] = 0;
			artStats.observedCD4AtObservedFailureMeanType[i][j] = 0;
			artStats.monthsToObservedFailureSumType[i][j] = 0;
			artStats.monthsToObservedFailureMeanType[i][j] = 0;
			artStats.monthsToObservedFailureSumSquaresType[i][j] = 0;
			artStats.monthsToObservedFailureStdDevType[i][j] = 0;
		}
		artStats.numStop[i] = 0;
		artStats.numStopAfterTrueFailure[i] = 0;
		for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
			artStats.numStopCD4Metric[i][j] = 0;
			artStats.trueCD4AtStopSum[i][j] = 0;
			artStats.trueCD4AtStopMean[i][j] = 0;
		}	
		artStats.numNeverStop[i] = 0;	
		artStats.numWithObservedCD4Stop[i] = 0;	
		artStats.observedCD4AtStopSum[i] = 0;
		artStats.observedCD4AtStopMean[i] = 0;
		artStats.monthsToStopSum[i] = 0;
		artStats.monthsToStopMean[i] = 0;
		artStats.monthsToStopSumSquares[i] = 0;
		artStats.monthsToStopStdDev[i] = 0;
		for (int j = 0; j < SimContext::ART_NUM_STOP_TYPES; j++) {
			artStats.numStopType[i][j] = 0;
			artStats.numStopAfterTrueFailureType[i][j] = 0;
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
				artStats.numStopTypeCD4Metric[i][j][k] = 0;
				artStats.trueCD4AtStopSumType[i][j][k] = 0;
				artStats.trueCD4AtStopMeanType[i][j][k] = 0;
			}
			artStats.numWithObservedCD4StopType[i][j] = 0;
			artStats.observedCD4AtStopSumType[i][j] = 0;
			artStats.observedCD4AtStopMeanType[i][j] = 0;
			artStats.monthsToStopSumType[i][j] = 0;
			artStats.monthsToStopMeanType[i][j] = 0;
			artStats.monthsToStopSumSquaresType[i][j] = 0;
			artStats.monthsToStopStdDevType[i][j] = 0;
		}
		for (int j = 0; j < SimContext::ART_NUM_MTHS_RECORD; j++) {
			artStats.numOnARTAtMonth[i][j] = 0;
			artStats.numSuppressedAtMonth[i][j] = 0;
			artStats.HVLDropsAtMonthSum[i][j] = 0;
			artStats.HVLDropsAtMonthMean[i][j] = 0;
			artStats.HVLDropsAtMonthSumSquares[i][j] = 0;
			artStats.HVLDropsAtMonthStdDev[i][j] = 0;
		}

		artStats.numARTDeath[i] = 0;
		for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
			artStats.numARTDeathCD4Metric[i][j] = 0;
			artStats.trueCD4AtARTDeathMean[i][j] = 0;
			artStats.trueCD4AtARTDeathSum[i][j] = 0;
		}	
		artStats.observedCD4AtARTDeathMean[i] = 0;
		artStats.numWithObservedCD4AtARTDeath[i] = 0;
		artStats.observedCD4AtARTDeathSum[i] = 0;
		artStats.propensityAtARTDeathMean[i] = 0;
		artStats.propensityAtARTDeathSum[i] = 0;

		for (int j = 0; j < SimContext::DTH_NUM_CAUSES; j++){
			artStats.numARTDeathCause[i][j] = 0;
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
				artStats.numARTDeathCauseCD4Metric[i][j][k] = 0;
				artStats.trueCD4AtARTDeathCauseMean[i][j][k] = 0;
				artStats.trueCD4AtARTDeathCauseSum[i][j][k] = 0;
			}	
			artStats.observedCD4AtARTDeathCauseMean[i][j] = 0;
			artStats.numWithObservedCD4AtARTDeathCause[i][j] = 0;
			artStats.observedCD4AtARTDeathCauseSum[i][j] = 0;
			artStats.propensityAtARTDeathCauseMean[i][j] = 0;
			artStats.propensityAtARTDeathCauseSum[i][j] = 0;
		}

		for (int j = 0; j < SimContext::OI_NUM; j++){
			artStats.numARTOI[i][j] = 0;
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
				artStats.numARTOICD4Metric[i][j][k] = 0;
				artStats.trueCD4AtARTOIMean[i][j][k] = 0;
				artStats.trueCD4AtARTOISum[i][j][k] = 0;
			}	
			artStats.observedCD4AtARTOIMean[i][j] = 0;
			artStats.numWithObservedCD4AtARTOI[i][j] = 0;
			artStats.observedCD4AtARTOISum[i][j] = 0;
			artStats.propensityAtARTOIMean[i][j] = 0;
			artStats.propensityAtARTOISum[i][j] = 0;
		}

		artStats.numARTEverInit[i] = 0;
		for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
			artStats.numARTEverInitCD4Metric[i][j] = 0;
			artStats.trueCD4AtARTEverInitMean[i][j] = 0;
			artStats.trueCD4AtARTEverInitSum[i][j] = 0;
		}	
		artStats.observedCD4AtARTEverInitMean[i] = 0;
		artStats.numWithObservedCD4AtARTEverInit[i] = 0;
		artStats.observedCD4AtARTEverInitSum[i] = 0;
		artStats.propensityAtARTEverInitMean[i] = 0;
		artStats.propensityAtARTEverInitSum[i] = 0;

		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			for (int k = 0; k < SimContext::HVL_NUM_STRATA; k++) {
				artStats.distributionAtInit[i][j][k] = 0;
			}
		}
		for (int j = 0; j < SimContext::ART_NUM_TOX_SEVERITY; j++) {
			for (int k = 0; k < SimContext::HVL_NUM_STRATA; k++) {
				artStats.numToxicityCases[i][j][k] = 0;
			}
		}
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			artStats.numToxicityDeaths[i][j] = 0;
		}
		for (int j = 0; j < SimContext::STI_NUM_TRACKED; j++) {
			artStats.numSTIInterruptions[i][j] = 0;
			artStats.numSTIRestarts[i][j] = 0;
			artStats.numSTIEndpoints[i][j] = 0;
			artStats.numPatientsWithSTIInterruptions[i][j] = 0;
		}
	}
} /* end initARTStats */

/** \brief initTimeSummary initializes the given TimeSummary object */
void RunStats::initTimeSummary(TimeSummary *currTime, bool isDynamic) {
	// Initialize all the time summary values
	if (!isDynamic)
		currTime->timePeriod = 0;
	currTime->numAlive = 0;

	//TB longitudinal outputs
	for (int i = 0; i < SimContext::TB_NUM_STATES; i++)
		currTime->numAliveTB[i] = 0;

	for (int i = 0; i < SimContext::TB_NUM_TRACKER; i++){
		for (int j = 0; j < SimContext::HIV_CARE_NUM; j++){
			currTime->numAliveTBTrackerCare[i][j] = 0;
		}
	}		

	currTime->numTBLTFU = 0;

	for (int i = 0; i < SimContext::TB_NUM_PROPHS; i++){
		currTime->numTBProphMinorTox[i] = 0;
		currTime->numTBProphMajorTox[i] = 0;
		for(int j =0; j < SimContext::TB_NUM_STRAINS; j++){
			currTime->numIncreaseResistanceDueToProph[j][i] = 0;
		}
		for( int j = 0; j < SimContext::TB_NUM_STATES; j++){
			currTime->numOnTBProph[i][j] = 0;
			currTime->numCompletedTBProph[i][j] = 0;
		}	
	}		

	for (int i = 0; i < SimContext::TB_NUM_TREATMENTS; i++){
		currTime->numOnTBTreatmentTotal[i] = 0;
		currTime->numOnEmpiricTBTreatmentTotal[i] = 0;
		for (int j = 0; j < SimContext::TB_NUM_STATES; j++){
			currTime->numOnTBTreatmentByState[j][i] = 0;
			currTime->numOnEmpiricTBTreatmentByState[j][i] = 0;
		}
	}

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++)
		currTime->numTBStrain[i] = 0;

	for (int i = 0; i < SimContext::TB_NUM_TRACKER; i++){
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++){
			currTime->numHIVTBTrackerCD4[i][j] = 0;
		}	
	}

	for (int i = 0; i < SimContext::TB_NUM_STATES; i++){
		for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++){
			currTime->numTBInfections[i][j] = 0;
		}
	}		

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++) {
		currTime->numTBReactivationsLatent[i] = 0;
		currTime->numTBReactivationsPulmLatentHIVNegative[i] = 0;
		currTime->numTBReactivationsExtraPulmLatentHIVNegative[i] = 0;

		currTime->numTBRelapses[i] = 0;
		currTime->numTBRelapsesPulm[i] = 0;
		currTime->numTBRelapsesExtraPulm[i] = 0;

		for(int j=0;j<SimContext::CD4_NUM_STRATA;j++){
			currTime->numTBReactivationsPulmLatentHIVPositive[j][i] = 0;
			currTime->numTBReactivationsExtraPulmLatentHIVPositive[j][i] = 0;
		}
	}

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++){
		currTime->numObservedTBUninfectedTB[i] = 0;
		for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++)
			currTime->numObservedTBByTrueTBStrain[i][j] = 0;
	}

	for (int i = 0; i < SimContext::TB_NUM_STATES; i++){
		for (int j = 0; j < SimContext::TB_DIAG_STATUS_NUM; j++){
			for (int k = 0; k < SimContext::TB_NUM_TESTS; k++)
				currTime->numTBTestResults[k][i][j] = 0;
			currTime->numTBDiagnosticResults[i][j] = 0;
		}
	}

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++){
		currTime->numDSTTestResultsUninfectedTB[i] = 0;
		for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++)
			currTime->numDSTTestResultsByTrueTBStrain[i][j] = 0;
	}

	for(int i =0; i <2; i++){
		for(int j = 0; j < 2; j++){
			for (int k = 0; k < 2; k++){
				for (int l = 0; l < 2; l++){
					currTime->numTBUnfavorableOutcome[i][j][k][l] = 0;
					currTime->numDeathsTBUnfavorableOutcome[i][j][k][l]= 0;
				}
			}	
		}
	}	
	for (int i = 0; i < SimContext::TB_NUM_TREATMENTS; i++){
		currTime->numOnSuccessfulTBTreatment[i] = 0;
		currTime->numOnSuccessfulTBTreatmentPulm[i] = 0;
		currTime->numOnSuccessfulTBTreatmentExtraPulm[i] = 0;

		currTime->numOnFailedTBTreatment[i] = 0;
		currTime->numOnFailedTBTreatmentPulm[i] = 0;
		currTime->numOnFailedTBTreatmentExtraPulm[i] = 0;

		currTime->numDefaultTBTreatment[i] = 0;
		currTime->numDefaultTBTreatmentPulm[i] = 0;
		currTime->numDefaultTBTreatmentExtraPulm[i] = 0;
	}

	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++){
		for (int j = 0; j < SimContext::TB_NUM_TREATMENTS; j++){
			currTime->numDropoutTBTreatment[i][j] = 0;
		}	
	}
			
	currTime->numDeathsTB = 0;
	currTime->numDeathsTBPulmHIVNegative = 0;
	currTime->numDeathsTBExtraPulmHIVNegative = 0;
	currTime->numDeathsTBLTFUHIVNegative = 0;
	for(int j=0;j<SimContext::CD4_NUM_STRATA;j++){
		currTime->numDeathsTBPulmHIVPositive[j] = 0;
		currTime->numDeathsTBExtraPulmHIVPositive[j] = 0;
		currTime->numDeathsTBLTFUHIVPositive[j] = 0;
	}
	currTime->numDeathsTBWhileFailedTBTreatment = 0;
	currTime->numAllDeathsWhileFailedTBTreatment = 0;




	for(int i = 0; i < SimContext::HIST_EXT_NUM; i++){
		currTime->numWithOIHistExt[i]=0;
	}
	for(int i = 0; i < SimContext::GENDER_NUM; i++){
		currTime->numGender[i]=0;
	}

	currTime->numDeaths=0;

	currTime->numAliveWithCHRMs=0;
	currTime->numAliveWithoutCHRMs=0;

	for(int i=0;i<SimContext::CHRM_NUM;i++){
		currTime->numAliveCHRM[i]=0;
	}

	for (int j=0;j<SimContext::CHRM_AGE_CAT_NUM;j++){
		currTime->numCHRMsAgeTotal[j]=0;
	}
	for (int j=0;j<SimContext::GENDER_NUM;j++){
		currTime->numCHRMsGenderTotal[j]=0;
	}
	for(int j=0;j<SimContext::CD4_NUM_STRATA;j++){
		currTime->numCHRMsCD4Total[j]=0;
	}

	for (int i = 0; i < SimContext::HIV_ID_NUM; i++) {
		currTime->numAliveWithCHRMsDetState[i]=0;
		currTime->numAliveWithoutCHRMsDetState[i]=0;
		for(int j=0;j<SimContext::CHRM_NUM;j++){
			currTime->numAliveTypeCHRMs[i][j]=0;
		}
	}
	for (int i = 0; i < SimContext::HIV_CARE_NUM; i++){
		currTime->numAliveCare[i] = 0;
		currTime->numDeathsCare[i] = 0;

		for(int j = 0; j < SimContext::HIST_EXT_NUM; j++){
			currTime->numWithOIHistExtCare[j][i]=0;
		}
		for(int j = 0; j < SimContext::GENDER_NUM; j++){
			currTime->numGenderCare[j][i]=0;
		}
		currTime->trueCD4SumCare[i] = 0;
		currTime->trueCD4MeanCare[i] = 0;
		currTime->trueCD4SumSquaresCare[i] = 0;
		currTime->trueCD4StdDevCare[i] = 0;
		currTime->observedCD4SumCare[i] = 0;
		currTime->observedCD4MeanCare[i] = 0;
		currTime->observedCD4SumSquaresCare[i] = 0;
		currTime->observedCD4StdDevCare[i] = 0;

		currTime->propRespSumCare[i] = 0;
		currTime->propRespMeanCare[i] = 0;
		currTime->propRespSumSquaresCare[i] = 0;
		currTime->propRespStdDevCare[i] = 0;

		currTime->ageSumCare[i] = 0;
		currTime->ageMeanCare[i] = 0;
		currTime->ageSumSquaresCare[i] = 0;
		currTime->ageStdDevCare[i] = 0;

		for (int j = 0; j < SimContext::OUTPUT_AGE_CAT_NUM; j++){
			currTime->numAgeBracketCare[i][j] = 0;
			currTime->numDeathsAgeBracketCare[i][j] = 0;
		}		
	}

	for (int j = 0; j < SimContext::OUTPUT_AGE_CAT_NUM; j++){
		currTime->numAgeBracketOnART[j] = 0;
		currTime->numAgeBracketInCareOffART[j] = 0;
		currTime->numAgeBracketHIVPositive[j] = 0;
		currTime->numAgeBracketAlive[j] = 0;
		currTime->numDeathsAgeBracketOnART[j] = 0;
		currTime->numDeathsAgeBracketInCareOffART[j] = 0;
		currTime->numDeathsAgeBracketHIVPositive[j] = 0;
		currTime->numDeathsAgeBracket[j] = 0;
	}
	currTime->totalAliveOnART = 0;
	currTime->totalDeathsOnART = 0;
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++){
		for (int j = 0; j < SimContext::ART_EFF_NUM_TYPES; j++){
			currTime->numAliveOnART[i][j] = 0;
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++)
				currTime->numAliveOnARTCD4Metric[i][j][k] = 0;
			currTime->numDeathsOnART[i][j] = 0;
			for(int k = 0; k < SimContext::HIST_EXT_NUM; k++){
				currTime->numWithOIHistExtOnART[k][i][j]=0;
			}
			for(int k = 0; k < SimContext::GENDER_NUM; k++){
				currTime->numGenderOnART[k][i][j]=0;
			}
			currTime->trueCD4SumOnART[i][j] = 0;
			currTime->trueCD4MeanOnART[i][j] = 0;
			currTime->trueCD4SumSquaresOnART[i][j] = 0;
			currTime->trueCD4StdDevOnART[i][j] = 0;
			currTime->observedCD4SumOnART[i][j] = 0;
			currTime->observedCD4MeanOnART[i][j] = 0;
			currTime->observedCD4SumSquaresOnART[i][j] = 0;
			currTime->observedCD4StdDevOnART[i][j] = 0;

			currTime->propRespSumOnART[i][j] = 0;
			currTime->propRespMeanOnART[i][j] = 0;
			currTime->propRespSumSquaresOnART[i][j] = 0;
			currTime->propRespStdDevOnART[i][j] = 0;

			currTime->ageSumOnART[i][j] = 0;
			currTime->ageMeanOnART[i][j] = 0;
			currTime->ageSumSquaresOnART[i][j] = 0;
			currTime->ageStdDevOnART[i][j] = 0;
		}
	}
	currTime->numAlivePositive = 0;
	currTime->numAliveInCareOffART = 0;
	for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
		currTime->numAlivePositiveCD4Metric[j] = 0;
		currTime->numAliveInCareOffARTCD4Metric[j] = 0;	
		for(int i = 0; i < SimContext::HIV_CARE_NUM; i++){
			currTime->numAliveCareCD4Metric[i][j] = 0;
		}
	}
	currTime->numDeathsInCareOffART = 0;
	currTime->numDeathsPositive = 0;

	for(int i = 0; i < SimContext::HIST_EXT_NUM; i++){
		currTime->numWithOIHistExtInCareOffART[i]=0;
		currTime->numWithOIHistExtPositive[i]=0;
	}
	for(int i = 0; i < SimContext::GENDER_NUM; i++){
		currTime->numGenderInCareOffART[i]=0;
		currTime->numGenderPositive[i]=0;
	}
	currTime->trueCD4SumInCareOffART = 0;
	currTime->trueCD4MeanInCareOffART = 0;
	currTime->trueCD4SumSquaresInCareOffART = 0;
	currTime->trueCD4StdDevInCareOffART = 0;
	currTime->observedCD4SumInCareOffART = 0;
	currTime->observedCD4MeanInCareOffART = 0;
	currTime->observedCD4SumSquaresInCareOffART = 0;
	currTime->observedCD4StdDevInCareOffART = 0;

	currTime->propRespSumInCareOffART = 0;
	currTime->propRespMeanInCareOffART = 0;
	currTime->propRespSumSquaresInCareOffART = 0;
	currTime->propRespStdDevInCareOffART = 0;

	currTime->ageSumInCareOffART = 0;
	currTime->ageMeanInCareOffART = 0;
	currTime->ageSumSquaresInCareOffART = 0;
	currTime->ageStdDevInCareOffART = 0;

	currTime->propRespSumPositive = 0;
	currTime->propRespMeanPositive = 0;
	currTime->propRespSumSquaresPositive = 0;
	currTime->propRespStdDevPositive = 0;

	currTime->ageSumPositive = 0;
	currTime->ageMeanPositive = 0;
	currTime->ageSumSquaresPositive = 0;
	currTime->ageStdDevPositive = 0;

	for (int i = 0; i < SimContext::PEDS_HIV_NUM; i++) {
		currTime->numAlivePediatrics[i] = 0;
	}
	for (int i = 0; i < SimContext::PEDS_MATERNAL_STATUS_NUM; i++)
		currTime->numAlivePediatricsMotherAlive[i] = 0;
	currTime->numAlivePediatricsMotherDead = 0;
	currTime->numNewlyDetectedPediatricsMotherStatusUnknown = 0;
	for(int i = 0; i < SimContext::PEDS_EXPOSED_BREASTFEEDING_NUM; i++)
		currTime->numHIVExposedUninf[i] = 0;	
	currTime->numNeverHIVExposed = 0;	

	for (int i = 0; i < SimContext::EID_TEST_TYPE_NUM;i++){
		currTime->numEIDTestsGivenType[i] = 0;
		currTime->numTruePositiveEIDTestResultsType[i] = 0;
		currTime->numTrueNegativeEIDTestResultsType[i] = 0;
		currTime->numFalsePositiveEIDTestResultsType[i] = 0;
		currTime->numFalseNegativeEIDTestResultsType[i] = 0;
	}
	for (int i = 0; i < SimContext::EID_NUM_TESTS;i++){
		currTime->numEIDTestsGivenTest[i] = 0;
		currTime->numTruePositiveEIDTestResultsTest[i] = 0;
		currTime->numTrueNegativeEIDTestResultsTest[i] = 0;
		currTime->numFalsePositiveEIDTestResultsTest[i] = 0;
		currTime->numFalseNegativeEIDTestResultsTest[i] = 0;
	}

	currTime->numAliveFalsePositive = 0;
	currTime->numAliveFalsePositiveLinked = 0;
	currTime->numIncidentPPInfections = 0;

	for(int i=0;i<SimContext::CHRM_NUM;i++){
		currTime->numIncidentCHRMs[i]=0;
	}
	currTime->numIncidentHIVInfections = 0;
	if (!isDynamic){
		currTime->dynamicNumIncidentHIVInfections = 0;
		currTime->dynamicNumHIVNegAtStartMonth = 0;
	}
	currTime->debugNumHIVNegAtStartMonth = 0;
	currTime->dynamicSelfTransmissionMult = 0;
	for(int i = 0; i< SimContext::HIV_BEHAV_NUM; i++){
		currTime->numAliveNegRisk[i] = 0;
		currTime->probPrepUptake[i] = 0;
	}	
	for(int i = 0; i < SimContext::HIV_DET_NUM; i++){
		currTime->numHIVDetections[i] = 0;
		currTime->cumulativeNumHIVDetections[i] = 0;
	}
	currTime->numHIVTestsPerformed = 0;
	currTime->numHIVTestsPerformedAtInitOffer = 0;
	currTime->numHIVTestsPerformedPostStartup = 0;
	currTime->cumulativeNumHIVTests = 0;
	currTime->cumulativeNumHIVTestsAtInitOffer = 0;
	currTime->cumulativeNumHIVTestsPostStartup = 0;
	

	for(int i=0;i<SimContext::CHRM_NUM;i++){
		currTime->numIncidentCHRMs[i]=0;
	}
	currTime->sumQOLModifiers = 0;
	currTime->trueCD4Sum = 0;
	currTime->trueCD4Mean = 0;
	currTime->trueCD4SumSquares = 0;
	currTime->trueCD4StdDev = 0;
	currTime->observedCD4Sum = 0;
	currTime->observedCD4Mean = 0;
	currTime->observedCD4SumSquares = 0;
	currTime->observedCD4StdDev = 0;

	currTime->propRespSum = 0;
	currTime->propRespMean = 0;
	currTime->propRespSumSquares = 0;
	currTime->propRespStdDev = 0;

	currTime->ageSum = 0;
	currTime->ageMean = 0;
	currTime->ageSumSquares = 0;
	currTime->ageStdDev = 0;

	currTime->trueCD4PercentageSum = 0;
	currTime->trueCD4PercentageMean = 0;
	currTime->trueCD4PercentageSumSquares = 0;
	currTime->trueCD4PercentageStdDev = 0;
	currTime->trueHVLSum = 0;
	currTime->trueHVLMean = 0;
	currTime->trueHVLSumSquares = 0;
	currTime->trueHVLStdDev = 0;
	currTime->observedHVLSum = 0;
	currTime->observedHVLMean = 0;
	currTime->observedHVLSumSquares = 0;
	currTime->observedHVLStdDev = 0;
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		for (int j = 0; j < SimContext::ART_NUM_STATES; j++) {
			currTime->trueCD4ARTDistribution[j][i] = 0;
		}
		currTime->observedCD4Distribution[i] = 0;
		for (int j = 0; j < SimContext::HIV_CARE_NUM; j++){
			currTime->observedCD4DistributionCare[i][j] = 0;
		}
	}

	currTime->numWithObservedCD4InCareOffART = 0;
	currTime->numHIVPosWithObservedCD4 = 0;
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++){
		for (int j = 0; j < SimContext::ART_EFF_NUM_TYPES; j++){
			currTime->observedCD4DistributionOnART[i][j] = 0;
		}
	}

	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		currTime->trueHVLDistribution[i] = 0;
		currTime->observedHVLDistribution[i] = 0;
	}
	for (int i = 0; i < SimContext::ART_NUM_STATES; i++) {
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			for (int k = 0; k < SimContext::CD4_NUM_STRATA; k++) {
				currTime->trueCD4HVLARTDistribution[i][j][k] = 0;
			}
		}
	}

	currTime->numTransmissions = 0;
	for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
		currTime->numTransmissionsHVL[j] = 0;
	}
	for (int j = 0; j < SimContext::TRANSM_RISK_NUM; j++) {
		currTime->numTransmissionsRisk[j] = 0;
	}

	for (int i = 0; i < SimContext::ART_EFF_NUM_TYPES; i++) {
		currTime->numARTEfficacyState[i] = 0;
	}
	currTime->numWithoutOIHistory = 0;
	currTime->numPrimaryOIsTotal = 0;
	currTime->numSecondaryOIsTotal = 0;
	currTime->numOIsTotal = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		currTime->numPrimaryOIs[i] = 0;
		currTime->numSecondaryOIs[i] = 0;
		currTime->numWithOIHistory[i] = 0;
		currTime->numWithFirstOI[i] = 0;
		currTime->numDeathsFromFirstOI[i] = 0;
	}
	for (int i=0; i<SimContext::CHRM_NUM;i++){
		for (int j=0;j<SimContext::CHRM_AGE_CAT_NUM;j++){
			currTime->numCHRMsAge[i][j]=0;
		}
		for (int j=0;j<SimContext::GENDER_NUM;j++){
			currTime->numCHRMsGender[i][j]=0;
		}
		for(int j=0;j<SimContext::CD4_NUM_STRATA;j++){
			currTime->numCHRMsCD4[i][j]=0;
		}
	}
	currTime->numDeathsWithoutCHRMs=0;
	for (int i = 0; i < SimContext::DTH_NUM_CAUSES; i++) {
		currTime->numDeathsType[i] = 0;
		currTime->numDeathsWithoutCHRMsType[i]=0;
		for(int j=0;j<SimContext::CHRM_NUM;j++){
			currTime->numDeathsWithCHRMsTypeCHRM[i][j]=0;
		}
		for (int j = 0; j < SimContext::HIV_CARE_NUM; j++)
			currTime->numDeathsTypeCare[i][j]=0;
	}
	currTime->costsCD4Testing = 0;
	currTime->costsHVLTesting = 0;
	currTime->costsClinicVisits = 0;
	currTime->costsEIDVisits = 0;
	currTime->costsPrEP = 0;
	currTime->costsHIVTests = 0;
	currTime->costsHIVMisc = 0;
	currTime->costsLabStagingTests = 0;
	currTime->costsEIDTests = 0;
	currTime->costsInfantHIVProphDirect = 0;
    currTime->costsInfantHIVProphTox = 0;
	currTime->costsLabStagingMisc = 0;
	currTime->costsEIDMisc = 0;
	currTime->costsInterventionStartup = 0;
	currTime->costsInterventionMonthly = 0;



	currTime->totalMonthlyCohortCosts = 0;
	currTime->totalMonthlyTBCohortCosts = 0;

	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		currTime->totalMonthlyCohortCostsType[i] = 0;
	}

	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::PROPH_NUM; j++) {
			currTime->costsProph[i][j] = 0;
		}
	}

	currTime->cumulativeCohortCosts = 0;

	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		currTime->cumulativeCohortCostsType[i] = 0;
	}

	currTime->cumulativeARTCosts = 0;
	currTime->cumulativeCD4TestingCosts = 0;
	currTime->cumulativeHVLTestingCosts = 0;
	currTime->cumulativeHIVTestingCosts = 0;
	currTime->cumulativeHIVMiscCosts = 0;

    for (int i = 0; i < SimContext::HIV_BEHAV_NUM; i++) {
        currTime->numOnPrEP[i] = 0;
    }
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		currTime->costsART[i] = 0;
		currTime->numOnART[i] = 0;
		currTime->numStartingART[i] = 0;
		currTime->numLostToFollowUpART[i] = 0;
		currTime->numStartingLostToFollowUpART[i] = 0;
		currTime->numReturnOnPrevART[i] = 0;
		currTime->numReturnOnNextART[i] = 0;
		currTime->numDeathsWhileLostART[i] = 0;
        for (int j = 0; j < SimContext::ART_NUM_SUBREGIMENS; j++){
            for (int k = 0; k < SimContext::ART_NUM_TOX_SEVERITY; k++){
                for (int m = 0; m < SimContext::ART_NUM_TOX_PER_SEVERITY; m++){
                    currTime->incidentToxicities[i][j][k][m] = 0;
                }
            }
            for (int m = 0; i < SimContext::ART_NUM_TOX_PER_SEVERITY; i++){
                currTime->prevalentChronicToxicities[i][j][m] = 0;
            }
        }
	}
	currTime->numInCarePostART = 0;
	currTime->numInCarePreART = 0;
	currTime->numStartingPreART = 0;
	currTime->numStartingPostART = 0;
	for (int i=0;i<SimContext::CHRM_NUM;i++){
		currTime->costsCHRMs[i]=0;
	}
	currTime->numLostToFollowUpPreART = 0;
	currTime->numLostToFollowUpPostART = 0;
	currTime->numStartingLostToFollowUpPreART = 0;
	currTime->numStartingLostToFollowUpPostART = 0;
	currTime->numReturnToCarePreART = 0;
	currTime->numReturnToCarePostART = 0;
	currTime->numDeathsWhileLostPreART = 0;
	currTime->numDeathsWhileLostPostART = 0;
	currTime->numDeathsHIVPosNeverVisitedClinic = 0;
	currTime->numDeathsHIVPosHadClinicVisit = 0;
	currTime->numDeathsUninfected = 0;
} /* end initTimeSummary */

/** \brief initOrphanStats initializes the orphanStats object */
void RunStats::initOrphanStats(OrphanStats *currTime) {
	// Initialize all the Orphan Stats values
	currTime->timePeriod = 0;
	currTime->numOrphans = 0;
	for (int i = 0; i < SimContext::CHRM_ORPHANS_OUTPUT_AGE_CAT_NUM; i++){
		currTime->numOrphansAge[i] = 0;
	}
} /* end initOrphanStats */

/** \brief finalizePopulationSummary calculates aggregate statistics for the PopulationSummary object */
void RunStats::finalizePopulationSummary() {
	char tmpbuf[256];

	// Copy the run/set name and get the time/date of completion
	const SimContext::RunSpecsInputs *runSpecs = simContext->getRunSpecsInputs();
	popSummary.runSetName = runSpecs->runSetName;
	popSummary.runName = runSpecs->runName;
	CepacUtil::getDateString(tmpbuf, 255);
	popSummary.runDate = tmpbuf;
	CepacUtil::getTimeString(tmpbuf, 255);
	popSummary.runTime = tmpbuf;

	/** Calculate the total costs and life month statistics from all patients */
	popSummary.costsAverage = popSummary.costsSum / popSummary.numCohorts;
	popSummary.costsStdDev = sqrt(popSummary.costsSumSquares / popSummary.numCohorts - popSummary.costsAverage * popSummary.costsAverage);
	popSummary.costsLowerBound = popSummary.costsAverage - (1.96 * popSummary.costsStdDev / sqrt((double)popSummary.numCohorts));
	popSummary.costsUpperBound = popSummary.costsAverage + (1.96 * popSummary.costsStdDev / sqrt((double)popSummary.numCohorts));
	popSummary.LMsAverage = popSummary.LMsSum / popSummary.numCohorts;
	popSummary.LMsStdDev = sqrt(popSummary.LMsSumSquares / popSummary.numCohorts - popSummary.LMsAverage * popSummary.LMsAverage);
	popSummary.LMsLowerBound = popSummary.LMsAverage - (1.96 * popSummary.LMsStdDev / sqrt((double)popSummary.numCohorts));
	popSummary.LMsUpperBound = popSummary.LMsAverage + (1.96 * popSummary.LMsStdDev / sqrt((double)popSummary.numCohorts));
	popSummary.QALMsAverage = popSummary.QALMsSum / popSummary.numCohorts;
	popSummary.QALMsStdDev = sqrt(popSummary.QALMsSumSquares / popSummary.numCohorts - popSummary.QALMsAverage * popSummary.QALMsAverage);
	popSummary.QALMsLowerBound = popSummary.QALMsAverage - (1.96 * popSummary.QALMsStdDev / sqrt((double)popSummary.numCohorts));
	popSummary.QALMsUpperBound = popSummary.QALMsAverage + (1.96 * popSummary.QALMsStdDev / sqrt((double)popSummary.numCohorts));

	if (simContext->getOutputInputs()->enableSubCohorts){
		for (int i = 0; i < SimContext::MAX_NUM_SUBCOHORTS; i++){
			int numInSubCohort = simContext->getOutputInputs()->subCohorts[i];
			if(numInSubCohort>0){
				popSummary.costsAverageCohortParsing[i] = popSummary.costsSumCohortParsing[i] / numInSubCohort;
				popSummary.costsStdDevCohortParsing[i] = sqrt(popSummary.costsSumSquaresCohortParsing[i] / numInSubCohort - popSummary.costsAverageCohortParsing[i] * popSummary.costsAverageCohortParsing[i]);
				popSummary.LMsAverageCohortParsing[i] = popSummary.LMsSumCohortParsing[i] / numInSubCohort;
				popSummary.LMsStdDevCohortParsing[i] = sqrt(popSummary.LMsSumSquaresCohortParsing[i] / numInSubCohort - popSummary.LMsAverageCohortParsing[i] * popSummary.LMsAverageCohortParsing[i]);
				popSummary.QALMsAverageCohortParsing[i] = popSummary.QALMsSumCohortParsing[i] / numInSubCohort;
				popSummary.QALMsStdDevCohortParsing[i] = sqrt(popSummary.QALMsSumSquaresCohortParsing[i] / numInSubCohort - popSummary.QALMsAverageCohortParsing[i] * popSummary.QALMsAverageCohortParsing[i]);
			}
			else{
				popSummary.costsAverageCohortParsing[i] = 0;
				popSummary.costsStdDevCohortParsing[i] = 0;
				popSummary.LMsAverageCohortParsing[i] = 0;
				popSummary.LMsStdDevCohortParsing[i] = 0;
				popSummary.QALMsAverageCohortParsing[i] = 0;
				popSummary.QALMsStdDevCohortParsing[i] = 0;
			}
		}
	}
	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		popSummary.multDiscCostsAverage[i] = popSummary.multDiscCostsSum[i]/popSummary.numCohorts;
		popSummary.multDiscCostsStdDev[i] = sqrt(popSummary.multDiscCostsSumSquares[i] / popSummary.numCohorts - popSummary.multDiscCostsAverage[i] * popSummary.multDiscCostsAverage[i]);
		popSummary.multDiscLMsAverage[i] = popSummary.multDiscLMsSum[i]/popSummary.numCohorts;
		popSummary.multDiscLMsStdDev[i] = sqrt(popSummary.multDiscLMsSumSquares[i] / popSummary.numCohorts - popSummary.multDiscLMsAverage[i] * popSummary.multDiscLMsAverage[i]);
		popSummary.multDiscQALMsAverage[i] = popSummary.multDiscQALMsSum[i]/popSummary.numCohorts;
		popSummary.multDiscQALMsStdDev[i] = sqrt(popSummary.multDiscQALMsSumSquares[i] / popSummary.numCohorts - popSummary.multDiscQALMsAverage[i] * popSummary.multDiscQALMsAverage[i]);
	}
	/** Calculate the total costs and life month statistics from patients with X number of ART failures */
	for (int i = 0; i <= SimContext::ART_NUM_LINES; i++) {
		if (popSummary.numFailART[i] > 0) {
			popSummary.costsFailARTAverage[i] = popSummary.costsFailARTSum[i] / popSummary.numFailART[i];
			popSummary.LMsFailARTAverage[i] = popSummary.LMsFailARTSum[i] / popSummary.numFailART[i];
			popSummary.QALMsFailARTAverage[i] = popSummary.QALMsFailARTSum[i] / popSummary.numFailART[i];
		}
	}

	/** Calculate the total costs and life month statistics from the HIV positive patients */
	if (popSummary.numCohortsHIVPositive > 0) {
		popSummary.costsHIVPositiveAverage = popSummary.costsHIVPositiveSum / popSummary.numCohortsHIVPositive;
		popSummary.LMsHIVPositiveAverage = popSummary.LMsHIVPositiveSum / popSummary.numCohortsHIVPositive;
		popSummary.QALMsHIVPositiveAverage = popSummary.QALMsHIVPositiveSum / popSummary.numCohortsHIVPositive;
	}
} /* end finalizePopulationSummary */

/** \brief finalizeHIVScreening calculates aggregate statistics for the HIVScreening object */
void RunStats::finalizeHIVScreening() {
	hivScreening.numHIVPositiveTotal = hivScreening.numPrevalentCases + hivScreening.numIncidentCases;
	for(int i = 0; i < SimContext::EVER_PREP_NUM; i++){
		if(hivScreening.numNeverHIVPositive[i] > 0){
			hivScreening.LMsHIVNegativeAverage[i] = hivScreening.LMsHIVNegativeSum[i] / hivScreening.numNeverHIVPositive[i];
			hivScreening.QALMsHIVNegativeAverage[i] = hivScreening.QALMsHIVNegativeSum[i] / hivScreening.numNeverHIVPositive[i];
		}	
	}

	/** Calculate sums for number at detection by CD4, HVL, HIV state */
	for (int j = 0; j < SimContext::HIV_POS_NUM; j++) {
		for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
			hivScreening.numAtDetectionPrevalent += hivScreening.numAtDetectionPrevalentCD4HIV[i][j];
			hivScreening.numAtDetectionIncident += hivScreening.numAtDetectionIncidentCD4HIV[i][j];
			hivScreening.numAtDetectionPrevalentHIV[j] += hivScreening.numAtDetectionPrevalentCD4HIV[i][j];
			hivScreening.numAtDetectionIncidentHIV[j] += hivScreening.numAtDetectionIncidentCD4HIV[i][j];
			hivScreening.numAtDetectionPrevalentCD4[i] += hivScreening.numAtDetectionPrevalentCD4HIV[i][j];
			hivScreening.numAtDetectionIncidentCD4[i] += hivScreening.numAtDetectionIncidentCD4HIV[i][j];
			hivScreening.numAtLinkage += hivScreening.numAtLinkageCD4HIV[i][j];
			hivScreening.numAtLinkageHIV[j]+=hivScreening.numAtLinkageCD4HIV[i][j];
			hivScreening.numAtLinkageCD4[i] += hivScreening.numAtLinkageCD4HIV[i][j];
		}
		for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
			hivScreening.numAtDetectionPrevalentHVL[i] += hivScreening.numAtDetectionPrevalentHVLHIV[i][j];
			hivScreening.numAtDetectionIncidentHVL[i] += hivScreening.numAtDetectionIncidentHVLHIV[i][j];
			hivScreening.numAtLinkageHVL[i] += hivScreening.numAtLinkageHVLHIV[i][j];
		}
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		if (hivScreening.numAtDetectionIncident > 0)
			hivScreening.percentAtDetectionIncidentCD4[i] = 100.0 * hivScreening.numAtDetectionIncidentCD4[i] / hivScreening.numAtDetectionIncident;
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		if (hivScreening.numAtDetectionIncident > 0)
			hivScreening.percentAtDetectionIncidentHVL[i] = 100.0 * hivScreening.numAtDetectionIncidentHVL[i] / hivScreening.numAtDetectionIncident;
	}

	/** Calculate average CD4s at time of detection */
	for (int i = 0; i < SimContext::HIV_POS_NUM; i++) {
		hivScreening.CD4AtDetectionPrevalentSum += hivScreening.CD4AtDetectionPrevalentSumHIV[i];
		hivScreening.CD4AtDetectionIncidentSum += hivScreening.CD4AtDetectionIncidentSumHIV[i];
		hivScreening.CD4AtLinkageSum += hivScreening.CD4AtLinkageSumHIV[i];
		if (hivScreening.numAtDetectionPrevalentHIVCD4Metric[i][SimContext::CD4_ABSOLUTE] > 0)
			hivScreening.CD4AtDetectionPrevalentAverageHIV[i] = hivScreening.CD4AtDetectionPrevalentSumHIV[i] / hivScreening.numAtDetectionPrevalentHIVCD4Metric[i][SimContext::CD4_ABSOLUTE];
		if (hivScreening.numAtDetectionIncidentHIVCD4Metric[i][SimContext::CD4_ABSOLUTE] > 0)
			hivScreening.CD4AtDetectionIncidentAverageHIV[i] = hivScreening.CD4AtDetectionIncidentSumHIV[i] / hivScreening.numAtDetectionIncidentHIVCD4Metric[i][SimContext::CD4_ABSOLUTE];
		if (hivScreening.numAtLinkageHIVCD4Metric[i][SimContext::CD4_ABSOLUTE] > 0)
			hivScreening.CD4AtLinkageAverageHIV[i] = hivScreening.CD4AtLinkageSumHIV[i] / hivScreening.numAtLinkageHIVCD4Metric[i][SimContext::CD4_ABSOLUTE];
	}
	if (hivScreening.numAtDetectionPrevalentCD4Metric[SimContext::CD4_ABSOLUTE] > 0)
		hivScreening.CD4AtDetectionPrevalentAverage = hivScreening.CD4AtDetectionPrevalentSum / hivScreening.numAtDetectionPrevalentCD4Metric[SimContext::CD4_ABSOLUTE];
	if (hivScreening.numAtDetectionIncidentCD4Metric[SimContext::CD4_ABSOLUTE] > 0)
		hivScreening.CD4AtDetectionIncidentAverage = hivScreening.CD4AtDetectionIncidentSum / hivScreening.numAtDetectionIncidentCD4Metric[SimContext::CD4_ABSOLUTE];
	if (hivScreening.numAtLinkageCD4Metric[SimContext::CD4_ABSOLUTE] > 0)
		hivScreening.CD4AtLinkageAverage = hivScreening.CD4AtLinkageSum / hivScreening.numAtLinkageCD4Metric[SimContext::CD4_ABSOLUTE];
	/** Calculate averages and standard deviations for time to infection and detection */
	if (hivScreening.numIncidentCases > 0) {
		hivScreening.monthsToInfectionAverage = hivScreening.monthsToInfectionSum / hivScreening.numIncidentCases;
		hivScreening.monthsToInfectionStdDev = sqrt(hivScreening.monthsToInfectionSumSquares / hivScreening.numIncidentCases - hivScreening.monthsToInfectionAverage * hivScreening.monthsToInfectionAverage);
	}
	if (hivScreening.numAtDetectionPrevalent > 0) {
		hivScreening.monthsToDetectionPrevalentAverage = hivScreening.monthsToDetectionPrevalentSum / hivScreening.numAtDetectionPrevalent;
		hivScreening.monthsToDetectionPrevalentStdDev = sqrt(hivScreening.monthsToDetectionPrevalentSumSquares / hivScreening.numAtDetectionPrevalent - hivScreening.monthsToDetectionPrevalentAverage * hivScreening.monthsToDetectionPrevalentAverage);
		hivScreening.ageMonthsAtDetectionPrevalentAverage = hivScreening.ageMonthsAtDetectionPrevalentSum / hivScreening.numAtDetectionPrevalent;
		hivScreening.ageMonthsAtDetectionPrevalentStdDev = sqrt(hivScreening.ageMonthsAtDetectionPrevalentSumSquares / hivScreening.numAtDetectionPrevalent - hivScreening.ageMonthsAtDetectionPrevalentAverage * hivScreening.ageMonthsAtDetectionPrevalentAverage);
	}
	if (hivScreening.numAtDetectionIncident > 0) {
		hivScreening.monthsAfterInfectionToDetectionAverage = hivScreening.monthsAfterInfectionToDetectionSum / hivScreening.numAtDetectionIncident;
		hivScreening.monthsAfterInfectionToDetectionStdDev = sqrt(hivScreening.monthsAfterInfectionToDetectionSumSquares / hivScreening.numAtDetectionIncident - hivScreening.monthsAfterInfectionToDetectionAverage * hivScreening.monthsAfterInfectionToDetectionAverage);
		hivScreening.monthsToDetectionIncidentAverage = hivScreening.monthsToDetectionIncidentSum / hivScreening.numAtDetectionIncident;
		hivScreening.monthsToDetectionIncidentStdDev = sqrt(hivScreening.monthsToDetectionIncidentSumSquares / hivScreening.numAtDetectionIncident - hivScreening.monthsToDetectionIncidentAverage * hivScreening.monthsToDetectionIncidentAverage);
		hivScreening.ageMonthsAtDetectionIncidentAverage = hivScreening.ageMonthsAtDetectionIncidentSum / hivScreening.numAtDetectionIncident;
		hivScreening.ageMonthsAtDetectionIncidentStdDev = sqrt(hivScreening.ageMonthsAtDetectionIncidentSumSquares / hivScreening.numAtDetectionIncident - hivScreening.ageMonthsAtDetectionIncidentAverage * hivScreening.ageMonthsAtDetectionIncidentAverage);
	}
	if (hivScreening.numAtLinkage > 0){
		hivScreening.monthsToLinkageAverage = hivScreening.monthsToLinkageSum / hivScreening.numAtLinkage;
		hivScreening.monthsToLinkageStdDev = sqrt(hivScreening.monthsToLinkageSumSquares / hivScreening.numAtLinkage - hivScreening.monthsToLinkageAverage*hivScreening.monthsToLinkageAverage);
		hivScreening.ageMonthsAtLinkageAverage = hivScreening.ageMonthsAtLinkageSum / hivScreening.numAtLinkage;
		hivScreening.ageMonthsAtLinkageStdDev = sqrt(hivScreening.ageMonthsAtLinkageSumSquares / hivScreening.numAtLinkage - hivScreening.ageMonthsAtLinkageAverage * hivScreening.ageMonthsAtLinkageAverage);
	}
	for (int i = 0; i < SimContext::HIV_DET_NUM; i++){
		if (hivScreening.numLinkedMeans[i] > 0){
			hivScreening.monthsToLinkageAverageMeans[i] = hivScreening.monthsToLinkageSumMeans[i] / hivScreening.numLinkedMeans[i];
			hivScreening.monthsToLinkageStdDevMeans[i] = sqrt(hivScreening.monthsToLinkageSumSquaresMeans[i] / hivScreening.numLinkedMeans[i] - hivScreening.monthsToLinkageAverageMeans[i]*hivScreening.monthsToLinkageAverageMeans[i]);
		}
	}
	/** Calculate sums for test results by infection type */
	for (int i = 0; i < SimContext::TEST_RESULT_NUM; i++) {
		hivScreening.numTestResultsPrevalent += hivScreening.numTestResultsPrevalentType[i];
		hivScreening.numTestResultsIncident += hivScreening.numTestResultsIncidentType[i];
		hivScreening.numTestResultsHIVNegative += hivScreening.numTestResultsHIVNegativeType[i];
	}
} /* end finalizeHIVScreening */

/** \brief finalizeSurvivalStats calculates aggregate statistics for the SurvivalStats objects */
void RunStats::finalizeSurvivalStats() {
	// if all patients are HIV negative, skip - these statistics are for HIV+ only (see StateUpdater::addPatientSummary())
	if (patients.size() == 0)
		return;

	/** Sort the patient summaries by life months */
	sort(patients.begin(), patients.end(), PatientSummary::compareLMs());

	/** Calculate the upper bound, lower bound, and median of the survival groups,
	//	also initial histogram starting point and bucket size */
	int numTruncate = patients.size() * TRUNC_HISTOGRAM_PERC / 100;
	int lowerBoundNum[NUM_SURVIVAL_GROUPS];
	int upperBoundNum[NUM_SURVIVAL_GROUPS];
	int medianNum[NUM_SURVIVAL_GROUPS];
	int numCohorts[NUM_SURVIVAL_GROUPS];
	int currBucket[NUM_SURVIVAL_GROUPS];
	int bucketSize[NUM_SURVIVAL_GROUPS];
	for (int i = 0; i < NUM_SURVIVAL_GROUPS; i++) {
		if ((i == SURVIVAL_EXCL_SHORT) || (i == SURVIVAL_EXCL_LONG_AND_SHORT))
			lowerBoundNum[i] = numTruncate;
		else
			lowerBoundNum[i] = 0;
		if ((i == SURVIVAL_EXCL_LONG) || (i == SURVIVAL_EXCL_LONG_AND_SHORT))
			upperBoundNum[i] = patients.size() - numTruncate - 1;
		else
			upperBoundNum[i] = patients.size() - 1;
		numCohorts[i] = upperBoundNum[i] - lowerBoundNum[i] + 1;
		medianNum[i] = (upperBoundNum[i] + lowerBoundNum[i]) / 2;
		survivalStats[i].LMsMin = patients[lowerBoundNum[i]].LMs;
		survivalStats[i].LMsMax = patients[upperBoundNum[i]].LMs;
		survivalStats[i].LMsMedian = patients[medianNum[i]].LMs;
		currBucket[i] = (int) floor(survivalStats[i].LMsMin);
		bucketSize[i] = (int) floor((survivalStats[i].LMsMax - survivalStats[i].LMsMin) / MAX_NUM_HISTOGRAM_BUCKETS) + 1;
	}

	/** Initital loop over the patient summaries only calculates mean values and histogram */
	int patientNum = 0;
	for (vector<PatientSummary>::iterator i = patients.begin(); i != patients.end(); i++) {
		const PatientSummary &currPatient = *i;
		for (int j = 0; j < NUM_SURVIVAL_GROUPS; j++) {
			if ((patientNum < lowerBoundNum[j]) || (patientNum > upperBoundNum[j]))
				continue;
			survivalStats[j].LMsSum += currPatient.LMs;
			survivalStats[j].costsSum += currPatient.costs;
			survivalStats[j].QALMsSum += currPatient.QALMs;
			// Remember that LMs are a double and that before discounting, the patient accrues 0.5 LMs in the month of their death, so the upper bound is excluded from each bin here
			while (currPatient.LMs > currBucket[j]) {
				currBucket[j] += bucketSize[j];
				survivalStats[j].LMsHistogram[currBucket[j]] = 0;
			}
			survivalStats[j].LMsHistogram[currBucket[j]]++;
		}
		patientNum++;
	}
	for (int i = 0; i < NUM_SURVIVAL_GROUPS; i++) {
		survivalStats[i].LMsMean = survivalStats[i].LMsSum / numCohorts[i];
		survivalStats[i].costsMean = survivalStats[i].costsSum / numCohorts[i];
		survivalStats[i].QALMsMean = survivalStats[i].QALMsSum / numCohorts[i];
	}

	/** Second loop calculates all other statistics, need means for skew and kurtosis */
	patientNum = 0;
	for (vector<PatientSummary>::iterator i = patients.begin(); i != patients.end(); i++) {
		const PatientSummary &currPatient = *i;
		for (int j = 0; j < NUM_SURVIVAL_GROUPS; j++) {
			if ((patientNum < lowerBoundNum[j]) || (patientNum > upperBoundNum[j]))
				continue;
			double devLMMedian = survivalStats[j].LMsMedian - currPatient.LMs;
			devLMMedian = (devLMMedian < 0.0) ? -devLMMedian : devLMMedian;
			survivalStats[j].LMsSumDeviationMedian += devLMMedian;
			double devLM = currPatient.LMs - survivalStats[j].LMsMean;
			survivalStats[j].LMsSumDeviation += (devLM < 0.0) ? -devLM : devLM;
			double devLMAccum = devLM * devLM;
			survivalStats[j].LMsSumDeviationSquares += devLMAccum;
			devLMAccum *= devLM;
			survivalStats[j].LMsSumDeviationCubes += devLMAccum;
			devLMAccum *= devLM;
			survivalStats[j].LMsSumDeviationQuads += devLMAccum;
			survivalStats[j].costsSumSquares += currPatient.costs * currPatient.costs;
			survivalStats[j].QALMsSumSquares += currPatient.QALMs * currPatient.QALMs;
		}
		patientNum++;
	}
	for (int i = 0; i < NUM_SURVIVAL_GROUPS; i++) {
		survivalStats[i].LMsAverageDeviationMedian = survivalStats[i].LMsSumDeviationMedian / numCohorts[i];
		survivalStats[i].LMsAverageDeviation = survivalStats[i].LMsSumDeviation / numCohorts[i];
		survivalStats[i].LMsVariance = survivalStats[i].LMsSumDeviationSquares / numCohorts[i];
		survivalStats[i].LMsStdDev = sqrt(survivalStats[i].LMsVariance);
		survivalStats[i].LMsSkew = survivalStats[i].LMsSumDeviationCubes / (numCohorts[i] * survivalStats[i].LMsVariance * survivalStats[i].LMsStdDev);
		survivalStats[i].LMsKurtosis = survivalStats[i].LMsSumDeviationQuads / (numCohorts[i] * survivalStats[i].LMsVariance * survivalStats[i].LMsVariance) - 3;
		survivalStats[i].costsStdDev = sqrt(survivalStats[i].costsSumSquares / numCohorts[i] - survivalStats[i].costsMean * survivalStats[i].costsMean);
		survivalStats[i].QALMsStdDev = sqrt(survivalStats[i].QALMsSumSquares / numCohorts[i] - survivalStats[i].QALMsMean * survivalStats[i].QALMsMean);
	}
} /* end finalizeSurvivalStats */

/** \brief finalizeInitialDistributions calculates aggregate statistics for the InitialDistributions object */
void RunStats::finalizeInitialDistributions() {
	/** Finalize initial distribution stats */
	if (popSummary.numCohortsHIVPositive > 0)
		initialDistributions.averageInitialAgeMonths = initialDistributions.sumInitialAgeMonths / popSummary.numCohortsHIVPositive;
} /* end finalizeInitialDistributions */

/** \brief finalizeCHRMsStats calculates aggregate statistics for the CHRMsStats object */
void RunStats::finalizeCHRMsStats() {
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			chrmsStats.numPrevalentCHRM[i] +=  chrmsStats.numPrevalentCHRMCD4[i][j];
			chrmsStats.numPrevalentCD4[j] +=  chrmsStats.numPrevalentCHRMCD4[i][j];
			chrmsStats.numIncidentCHRM[i] +=  chrmsStats.numIncidentCHRMCD4[i][j];
			chrmsStats.numIncidentCD4[j] +=  chrmsStats.numIncidentCHRMCD4[i][j];
			chrmsStats.numDeathsCHRM[i] +=  chrmsStats.numDeathsCHRMCD4[i][j];
			chrmsStats.numDeathsCD4[j] +=  chrmsStats.numDeathsCHRMCD4[i][j];
		}
		chrmsStats.numHIVNegWithPrevalentCHRMs += chrmsStats.numPrevalentCHRMHIVNeg[i];
		chrmsStats.numHIVNegWithIncidentCHRMs += chrmsStats.numIncidentCHRMHIVneg[i];
	}
} /* end finalizeCHRMsStats */

/** \brief finalizeOIStats calculates aggregate statistics for the OIStats object */
void RunStats::finalizeOIStats() {
	/** Accumulate the OI occurrence numbers */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			oiStats.numPrimaryOIsCD4[j] += oiStats.numPrimaryOIsCD4OI[j][i];
			oiStats.numPrimaryOIsOI[i] += oiStats.numPrimaryOIsCD4OI[j][i];
			oiStats.numSecondaryOIsCD4[j] += oiStats.numSecondaryOIsCD4OI[j][i];
			oiStats.numSecondaryOIsOI[i] += oiStats.numSecondaryOIsCD4OI[j][i];
			oiStats.numDetectedOIsCD4[j] += oiStats.numDetectedOIsCD4OI[j][i];
			oiStats.numDetectedOIsOI[i] += oiStats.numDetectedOIsCD4OI[j][i];
		}
		oiStats.numOIEventsTotal += (oiStats.numPrimaryOIsOI[i] + oiStats.numSecondaryOIsOI[i]);
	}

	/** Accumulate the OI history numbers */
	for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
		for (int k = 0; k < SimContext::CD4_NUM_STRATA; k++) {
			oiStats.numMonthsCD4[k] += oiStats.numMonthsHVLCD4[j][k];
			oiStats.numMonthsHVL[j] += oiStats.numMonthsHVLCD4[j][k];
			oiStats.numPatientsCD4[k] += oiStats.numPatientsHVLCD4[j][k];
			oiStats.numPatientsHVL[j] += oiStats.numPatientsHVLCD4[j][k];
			for (int i = 0; i < SimContext::OI_NUM; i++) {
				oiStats.numMonthsOIHistoryCD4[i][k] += oiStats.numMonthsOIHistoryHVLCD4[i][j][k];
				oiStats.numMonthsOIHistoryHVL[i][j] += oiStats.numMonthsOIHistoryHVLCD4[i][j][k];
				oiStats.numPatientsOIHistoryCD4[i][k] += oiStats.numPatientsOIHistoryHVLCD4[i][j][k];
				oiStats.numPatientsOIHistoryHVL[i][j] += oiStats.numPatientsOIHistoryHVLCD4[i][j][k];
			}
		}
	}

	/** Calculate the OI history probabilities */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			if (oiStats.numPatientsCD4[j] > 0)
				oiStats.probPatientsOIHistoryCD4[i][j] = 1.0 * oiStats.numPatientsOIHistoryCD4[i][j] / oiStats.numPatientsCD4[j];
			if (oiStats.numMonthsCD4[j] > 0)
				oiStats.probMonthsOIHistoryCD4[i][j] = 1.0 * oiStats.numMonthsOIHistoryCD4[i][j] / oiStats.numMonthsCD4[j];
		}
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			if (oiStats.numPatientsHVL[j] > 0)
				oiStats.probPatientsOIHistoryHVL[i][j] = 1.0 * oiStats.numPatientsOIHistoryHVL[i][j] / oiStats.numPatientsHVL[j];
			if (oiStats.numMonthsHVL[j] > 0)
				oiStats.probMonthsOIHistoryHVL[i][j] = 1.0 * oiStats.numMonthsOIHistoryHVL[i][j] / oiStats.numMonthsHVL[j];
		}
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			for (int k = 0; k < SimContext::CD4_NUM_STRATA; k++) {
				if (oiStats.numPatientsHVLCD4[j][k] > 0)
					oiStats.probPatientsOIHistoryHVLCD4[i][j][k] = 1.0 * oiStats.numPatientsOIHistoryHVLCD4[i][j][k] / oiStats.numPatientsHVLCD4[j][k];
				if (oiStats.numMonthsHVLCD4[j][k] > 0)
					oiStats.probMonthsOIHistoryHVLCD4[i][j][k] = 1.0 * oiStats.numMonthsOIHistoryHVLCD4[i][j][k] / oiStats.numMonthsHVLCD4[j][k];
			}
		}
	}
} /* end finalizeOIStats */

/** \brief finalizeDeathStats calculates aggregate statistics for the DeathStats object */
void RunStats::finalizeDeathStats() {
	/** Finalize death stats */
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		for (int j = 0; j < SimContext::DTH_NUM_CAUSES; j++) {
			deathStats.numDeathsCD4[i] += deathStats.numDeathsCD4Type[i][j];
			deathStats.numDeathsHIVPosType[j] += deathStats.numDeathsCD4Type[i][j];
		}
	}
	for (int i = 0; i < SimContext::HIV_CARE_NUM; i++){
		for (int j = 0; j < SimContext::DTH_NUM_CAUSES; j++){
			deathStats.numDeathsCare[i] += deathStats.numDeathsCareType[i][j];
		}
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			deathStats.numDeathsHVL[i] += deathStats.numDeathsHVLCD4[i][j];
		}
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		deathStats.numHIVDeathsNoOIHistory += deathStats.numHIVDeathsNoOIHistoryCD4[i];
		deathStats.numHIVDeathsOIHistory += deathStats.numHIVDeathsOIHistoryCD4[i];
		deathStats.numBackgroundMortDeathsNoOIHistory += deathStats.numBackgroundMortDeathsNoOIHistoryCD4[i];
		deathStats.numBackgroundMortDeathsOIHistory += deathStats.numBackgroundMortDeathsOIHistoryCD4[i];
	}

	for (int i=0;i<SimContext::CD4_NUM_STRATA;i++){
		for (int j=0;j<SimContext::HVL_NUM_STRATA;j++){
			if(deathStats.numARTToxDeathsCD4[i]==0){
				deathStats.hvlDistribToxDeathCD4HVL[i][j]=0;
			}
			else{
				deathStats.hvlDistribToxDeathCD4HVL[i][j]=(double)deathStats.numARTToxDeathsCD4HVL[i][j]/deathStats.numARTToxDeathsCD4[i];
			}
		}
	}
	for (int i=0;i<SimContext::OI_NUM;i++){
		for (int j=0;j<SimContext::CD4_NUM_STRATA;j++){
			for (int k=0;k<SimContext::HVL_NUM_STRATA;k++){
				if(deathStats.numARTToxDeathsCD4HVL[j][k]==0){
					deathStats.probOIHistARTToxDeathsCD4HVL[j][k][i]=0;
				}
				else{
					deathStats.probOIHistARTToxDeathsCD4HVL[j][k][i]=(double)deathStats.numARTToxDeathsCD4HVLOIHist[j][k][i]/deathStats.numARTToxDeathsCD4HVL[j][k];
				}
			}
		}
	}

	if(deathStats.numARTToxDeaths > 0){
		deathStats.ARTToxDeathsCD4Mean=deathStats.ARTToxDeathsCD4Sum/deathStats.numARTToxDeaths;
		deathStats.ARTToxDeathsCD4StdDev=sqrt
		
		(deathStats.ARTToxDeathsCD4SumSquares/deathStats.numARTToxDeaths-deathStats.ARTToxDeathsCD4Mean*deathStats.ARTToxDeathsCD4Mean);
	}
} /* end finalizeDeathStats */

/** \brief finalizeOverallSurvival calculates aggregate statistics for the OverallSurvival object */
void RunStats::finalizeOverallSurvival() {
	/** Finalize the overall survival stats */
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		overallSurvival.LMsTotalCD4[i] = overallSurvival.LMsNoOIHistoryCD4[i] + overallSurvival.LMsOIHistoryCD4[i];
		overallSurvival.LMsNoOIHistory += overallSurvival.LMsNoOIHistoryCD4[i];
		overallSurvival.LMsOIHistory += overallSurvival.LMsOIHistoryCD4[i];
		overallSurvival.LMsTotal += overallSurvival.LMsTotalCD4[i];
	}
	overallSurvival.LMsHIVPositive = overallSurvival.LMsHIVState[SimContext::HIV_ID_IDEN] +
		overallSurvival.LMsHIVState[SimContext::HIV_ID_UNID];
	overallSurvival.QALMsHIVPositive = overallSurvival.QALMsHIVState[SimContext::HIV_ID_IDEN] +
		overallSurvival.QALMsHIVState[SimContext::HIV_ID_UNID];
} /* end finalizeOverallSurvival */

/** \brief finalizeOverallCosts calculates aggregate statistics for the OverallCosts object */
void RunStats::finalizeOverallCosts() {
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		overallCosts.costsTotalCD4[i] = overallCosts.costsOIHistoryCD4[i] + overallCosts.costsNoOIHistoryCD4[i];
		overallCosts.costsTotal += overallCosts.costsTotalCD4[i];
		overallCosts.costsOIHistory += overallCosts.costsOIHistoryCD4[i];
		overallCosts.costsNoOIHistory += overallCosts.costsNoOIHistoryCD4[i];
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::PROPH_NUM; j++) {
			overallCosts.directCostsProph += overallCosts.directCostsProphOIsProph[i][j];
			overallCosts.directCostsProphOIs[i] += overallCosts.directCostsProphOIsProph[i][j];
		}
	}
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		overallCosts.costsARTInit += overallCosts.costsARTInitLine[i];
		overallCosts.costsARTMonthly += overallCosts.costsARTMonthlyLine[i];
		for (int j = 0; j < SimContext::NUM_DISCOUNT_RATES; j++){
			overallCosts.directCostsARTMultDisc[j] += overallCosts.directCostsARTLineMultDisc[i][j];
			overallCosts.costsARTInitMultDisc[j] += overallCosts.costsARTInitLineMultDisc[i][j];
			overallCosts.costsARTMonthlyMultDisc[j] += overallCosts.costsARTMonthlyLineMultDisc[i][j];
		}
	}
	overallCosts.costsHIVPositive = overallCosts.costsHIVState[SimContext::HIV_ID_UNID] + overallCosts.costsHIVState[SimContext::HIV_ID_IDEN];
} /* end finalizeOverallCosts */

/* finalizeTBStats calculates aggregate statistics for the TBStats object */
void RunStats::finalizeTBStats() {
	for (int i = 0; i < SimContext::TB_NUM_STRAINS; i++)
		tbStats.numDeaths[i] = tbStats.numDeathsHIVPos[i] + tbStats.numDeathsHIVNeg[i];
} /* end finalizeTBStats */

/** \brief finalizeLTFUStats calculates aggregate statistics for the LTFUStats object */
void RunStats::finalizeLTFUStats() {
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++) {
		ltfuStats.numLostToFollowUp += ltfuStats.numLostToFollowUpCD4[i];
		ltfuStats.numReturnToCare += ltfuStats.numReturnToCareCD4[i];
		ltfuStats.numDeathsWhileLost += ltfuStats.numDeathsWhileLostCD4[i];
	}
	if (ltfuStats.numReturnToCare > 0) {
		ltfuStats.monthsLostBeforeReturnMean = ltfuStats.monthsLostBeforeReturnSum / ltfuStats.numReturnToCare;
		ltfuStats.monthsLostBeforeReturnStdDev = sqrt(ltfuStats.monthsLostBeforeReturnSumSquares / ltfuStats.numReturnToCare - ltfuStats.monthsLostBeforeReturnMean * ltfuStats.monthsLostBeforeReturnMean);
	}
} /* end finalizeLTFUStats */

/** \brief finalizeProphStats calculates aggregate statistics for the ProphStats object */
void RunStats::finalizeProphStats() {
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		for (int j = 0; j < SimContext::PROPH_NUM_TYPES; j++) {
			prophStats.numMajorToxicityTotal[i] += prophStats.numMajorToxicity[i][j];
			prophStats.numMinorToxicityTotal[i] += prophStats.numMinorToxicity[i][j];
			for (int k = 0; k < SimContext::PROPH_NUM_TYPES; k++) {
				if (prophStats.numTimesInitProphCD4Metric[k][i][j][SimContext::CD4_ABSOLUTE] > 0) {
					prophStats.trueCD4InitProphMean[k][i][j] = prophStats.trueCD4InitProphSum[k][i][j] / prophStats.numTimesInitProphCD4Metric[k][i][j][SimContext::CD4_ABSOLUTE];
					if(prophStats.numTimesInitProphWithObservedCD4[k][i][j] > 0)
						prophStats.observedCD4InitProphMean[k][i][j] = prophStats.observedCD4InitProphSum[k][i][j] / prophStats.numTimesInitProphWithObservedCD4[k][i][j];
				}
			}
		}
	}
} /* end finalizeProphStats */

/** \brief finalizeARTStats calculates aggregate statistics for the ARTStats object */
void RunStats::finalizeARTStats() {
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		/** Finalize the months of ART suppression stats */
		artStats.monthsSuppressed += artStats.monthsSuppressedLine[i];
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			artStats.monthsFailedLine[i] += artStats.monthsFailedLineHVL[i][j];
			artStats.monthsFailedHVL[j] += artStats.monthsFailedLineHVL[i][j];
		}

		/** Finalize stats for beginning ART */
		for (int j = 0; j < SimContext::RESP_NUM_TYPES; j++) {
			for(int k=0;k<SimContext::HET_NUM_OUTCOMES;k++){
				if (artStats.numOnARTAtInitResp[i][k][j] > 0) {
					artStats.trueCD4AtInitMeanResp[i][k][j] = artStats.trueCD4AtInitSumResp[i][k][j] / artStats.numOnARTAtInitResp[i][k][j];
					if(artStats.numWithObservedCD4AtInitResp[i][k][j] > 0)
						artStats.observedCD4AtInitMeanResp[i][k][j] = artStats.observedCD4AtInitSumResp[i][k][j] / artStats.numWithObservedCD4AtInitResp[i][k][j];
				}
			}
		}
		if (artStats.numOnARTAtInit[i] > 0) {
			for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
				if(artStats.numOnARTAtInitCD4Metric[i][j] > 0)
					artStats.trueCD4AtInitMean[i][j] = artStats.trueCD4AtInitSum[i][j] / artStats.numOnARTAtInitCD4Metric[i][j];
			}	
			if(artStats.numWithObservedCD4AtInit[i] > 0)
				artStats.observedCD4AtInitMean[i] = artStats.observedCD4AtInitSum[i] / artStats.numWithObservedCD4AtInit[i];
		}

		/** Finalize stats for true ART failures */
		for (int j = 0; j < SimContext::RESP_NUM_TYPES; j++) {
			for(int k=0;k<SimContext::HET_NUM_OUTCOMES;k++){
				if (artStats.numTrueFailureResp[i][k][j] > 0) {
					artStats.trueCD4AtTrueFailureMeanResp[i][k][j] = artStats.trueCD4AtTrueFailureSumResp[i][k][j] / artStats.numTrueFailureResp[i][k][j];
					if(artStats.numWithObservedCD4AtTrueFailureResp[i][j][k] > 0)
						artStats.observedCD4AtTrueFailureMeanResp[i][k][j] = artStats.observedCD4AtTrueFailureSumResp[i][k][j] / artStats.numWithObservedCD4AtTrueFailureResp[i][j][k];
					artStats.monthsToTrueFailureMeanResp[i][k][j] = artStats.monthsToTrueFailureSumResp[i][k][j] / artStats.numTrueFailureResp[i][k][j];
					artStats.monthsToTrueFailureStdDevResp[i][k][j] = sqrt(artStats.monthsToTrueFailureSumSquaresResp[i][k][j] / artStats.numTrueFailureResp[i][k][j] - artStats.monthsToTrueFailureMeanResp[i][k][j] * artStats.monthsToTrueFailureMeanResp[i][k][j]);
				}
			}
		}
		if (artStats.numTrueFailure[i] > 0) {
			for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
				if(artStats.numTrueFailureCD4Metric[i][j] > 0)
					artStats.trueCD4AtTrueFailureMean[i][j] = artStats.trueCD4AtTrueFailureSum[i][j] / artStats.numTrueFailureCD4Metric[i][j];
			}		
			if(artStats.numWithObservedCD4AtTrueFailure[i] > 0)
				artStats.observedCD4AtTrueFailureMean[i] = artStats.observedCD4AtTrueFailureSum[i] / artStats.numWithObservedCD4AtTrueFailure[i];
			artStats.monthsToTrueFailureMean[i] = artStats.monthsToTrueFailureSum[i] / artStats.numTrueFailure[i];
			artStats.monthsToTrueFailureStdDev[i] = sqrt(artStats.monthsToTrueFailureSumSquares[i] / artStats.numTrueFailure[i] - artStats.monthsToTrueFailureMean[i] * artStats.monthsToTrueFailureMean[i]);
		}

		/** Finalize stats for observed ART failures */
		for (int j = 0; j < SimContext::ART_NUM_FAIL_TYPES; j++) {
			artStats.numObservedFailure[i] += artStats.numObservedFailureType[i][j];
			artStats.numObservedFailureAfterTrue[i] += artStats.numObservedFailureAfterTrueType[i][j];
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
				artStats.trueCD4AtObservedFailureSum[i][k] += artStats.trueCD4AtObservedFailureSumType[i][j][k];
			}	
			artStats.numObservedCD4atObservedFailure[i] += artStats.numObservedCD4atObservedFailureType[i][j];
			artStats.observedCD4AtObservedFailureSum[i] += artStats.observedCD4AtObservedFailureSumType[i][j];
			artStats.monthsToObservedFailureSum[i] += artStats.monthsToObservedFailureSumType[i][j];
			artStats.monthsToObservedFailureSumSquares[i] += artStats.monthsToObservedFailureSumSquaresType[i][j];
			if (artStats.numObservedFailureType[i][j] > 0) {
				for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
					if(artStats.numObservedFailureTypeCD4Metric[i][j][k] > 0)
						artStats.trueCD4AtObservedFailureMeanType[i][j][k] = artStats.trueCD4AtObservedFailureSumType[i][j][k] / artStats.numObservedFailureTypeCD4Metric[i][j][k];
				}		
				if(artStats.numObservedCD4atObservedFailureType[i][j] > 0)
					artStats.observedCD4AtObservedFailureMeanType[i][j] = artStats.observedCD4AtObservedFailureSumType[i][j] / artStats.numObservedCD4atObservedFailureType[i][j];
				artStats.monthsToObservedFailureMeanType[i][j] = artStats.monthsToObservedFailureSumType[i][j] / artStats.numObservedFailureType[i][j];
				artStats.monthsToObservedFailureStdDevType[i][j] = sqrt(artStats.monthsToObservedFailureSumSquaresType[i][j] / artStats.numObservedFailureType[i][j] - artStats.monthsToObservedFailureMeanType[i][j] * artStats.monthsToObservedFailureMeanType[i][j]);
			}
		}
		artStats.numNeverObservedFailure[i] = artStats.numOnARTAtInit[i] - artStats.numObservedFailure[i];
		if (artStats.numObservedFailure[i] > 0) {
			for(int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++){
				if(artStats.numObservedFailureCD4Metric[i][j] > 0)
					artStats.trueCD4AtObservedFailureMean[i][j] = artStats.trueCD4AtObservedFailureSum[i][j] / artStats.numObservedFailureCD4Metric[i][j];
			}
			if(artStats.numObservedCD4atObservedFailure[i] > 0)
				artStats.observedCD4AtObservedFailureMean[i] = artStats.observedCD4AtObservedFailureSum[i] / artStats.numObservedCD4atObservedFailure[i];
			artStats.monthsToObservedFailureMean[i] = artStats.monthsToObservedFailureSum[i] / artStats.numObservedFailure[i];
			artStats.monthsToObservedFailureStdDev[i] = sqrt(artStats.monthsToObservedFailureSumSquares[i] / artStats.numObservedFailure[i] - artStats.monthsToObservedFailureMean[i] * artStats.monthsToObservedFailureMean[i]);
		}

		/** Finalize stats for stopping ART */
		for (int j = 0; j < SimContext::ART_NUM_STOP_TYPES; j++) {
			artStats.numStop[i] += artStats.numStopType[i][j];
			artStats.numStopAfterTrueFailure[i] += artStats.numStopAfterTrueFailureType[i][j];
			for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
				artStats.numStopCD4Metric[i][k] += artStats.numStopTypeCD4Metric[i][j][k];
				artStats.trueCD4AtStopSum[i][k] += artStats.trueCD4AtStopSumType[i][j][k];
			}	
			artStats.observedCD4AtStopSum[i] += artStats.observedCD4AtStopSumType[i][j];
			artStats.numWithObservedCD4Stop[i] += artStats.numWithObservedCD4StopType[i][j];
			artStats.monthsToStopSum[i] += artStats.monthsToStopSumType[i][j];
			artStats.monthsToStopSumSquares[i] += artStats.monthsToStopSumSquaresType[i][j];
			if (artStats.numStopType[i][j] > 0) {
				for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
					if(artStats.numStopTypeCD4Metric[i][j][k] > 0)
						artStats.trueCD4AtStopMeanType[i][j][k] = artStats.trueCD4AtStopSumType[i][j][k] / artStats.numStopTypeCD4Metric[i][j][k];
				}		
				if(artStats.numWithObservedCD4StopType[i][j] > 0)
					artStats.observedCD4AtStopMeanType[i][j] = artStats.observedCD4AtStopSumType[i][j] / artStats.numWithObservedCD4StopType[i][j];
				artStats.monthsToStopMeanType[i][j] = artStats.monthsToStopSumType[i][j] / artStats.numStopType[i][j];
				artStats.monthsToStopStdDevType[i][j] = sqrt(artStats.monthsToStopSumSquaresType[i][j] / artStats.numStopType[i][j] - artStats.monthsToStopMeanType[i][j] * artStats.monthsToStopMeanType[i][j]);
			}
		}
		artStats.numNeverStop[i] = artStats.numOnARTAtInit[i] - artStats.numStop[i];
		if (artStats.numStop[i] > 0) {
			for (int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++) {
				if (artStats.numStopCD4Metric[i][j] > 0){
					artStats.trueCD4AtStopMean[i][j] = artStats.trueCD4AtStopSum[i][j] / artStats.numStopCD4Metric[i][j];
				}	
			}		
			if(artStats.numWithObservedCD4Stop[i] > 0)
				artStats.observedCD4AtStopMean[i] = artStats.observedCD4AtStopSum[i] / artStats.numWithObservedCD4Stop[i];
			artStats.monthsToStopMean[i] = artStats.monthsToStopSum[i] / artStats.numStop[i];
			artStats.monthsToStopStdDev[i] = sqrt(artStats.monthsToStopSumSquares[i] / artStats.numStop[i] - artStats.monthsToStopMean[i] * artStats.monthsToStopMean[i]);
		}

		/** finalize stats for deaths on ART */
		if (artStats.numARTDeath[i] >0){
			for (int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++) {
				if (artStats.numARTDeathCD4Metric[i][j] >0)
					artStats.trueCD4AtARTDeathMean[i][j] = artStats.trueCD4AtARTDeathSum[i][j]/artStats.numARTDeathCD4Metric[i][j];
			}
			
			double propSum = artStats.propensityAtARTDeathSum[i];

			artStats.propensityAtARTDeathMean[i] = artStats.propensityAtARTDeathSum[i]/artStats.numARTDeath[i];
		}
		if (artStats.numWithObservedCD4AtARTDeath[i] > 0)
			artStats.observedCD4AtARTDeathMean[i] = artStats.observedCD4AtARTDeathSum[i]/artStats.numWithObservedCD4AtARTDeath[i];

		/** finalize stats for DeathCauses on ART by cause*/
		for(int j = 0; j < SimContext::DTH_NUM_CAUSES;j++){
			if (artStats.numARTDeathCause[i][j] >0){
				for (int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++) {
					if(artStats.numARTDeathCauseCD4Metric[i][j][k] > 0)
						artStats.trueCD4AtARTDeathCauseMean[i][j][k] = artStats.trueCD4AtARTDeathCauseSum[i][j][k]/artStats.numARTDeathCauseCD4Metric[i][j][k];
				}	
				artStats.propensityAtARTDeathCauseMean[i][j] = artStats.propensityAtARTDeathCauseSum[i][j]/artStats.numARTDeathCause[i][j];
			}
			if (artStats.numWithObservedCD4AtARTDeathCause[i][j] > 0)
				artStats.observedCD4AtARTDeathCauseMean[i][j] = artStats.observedCD4AtARTDeathCauseSum[i][j]/artStats.numWithObservedCD4AtARTDeathCause[i][j];
		}

		/** finalize stats for OI on ART by OI*/
		for(int j = 0; j < SimContext::OI_NUM;j++){
			if(artStats.numARTOI[i][j] > 0){
				for(int k = 0; k < SimContext::PEDS_CD4_AGE_CAT_NUM; k++){
					if (artStats.numARTOICD4Metric[i][j][k] > 0){
						artStats.trueCD4AtARTOIMean[i][j][k] = artStats.trueCD4AtARTOISum[i][j][k]/artStats.numARTOICD4Metric[i][j][k];
					}	
				}	
				if (artStats.numWithObservedCD4AtARTOI[i][j] > 0)
					artStats.observedCD4AtARTOIMean[i][j] = artStats.observedCD4AtARTOISum[i][j]/artStats.numWithObservedCD4AtARTOI[i][j];
				artStats.propensityAtARTOIMean[i][j] = artStats.propensityAtARTOISum[i][j]/artStats.numARTOI[i][j];	
			}	
		}

		/** finalize stats for ever Init ART */
		if(artStats.numARTEverInit[i] > 0){
			artStats.propensityAtARTEverInitMean[i] = artStats.propensityAtARTEverInitSum[i]/artStats.numARTEverInit[i];
			for (int j = 0; j < SimContext::PEDS_CD4_AGE_CAT_NUM; j++) {
				if (artStats.numARTEverInitCD4Metric[i][j] > 0){
					artStats.trueCD4AtARTEverInitMean[i][j] = artStats.trueCD4AtARTEverInitSum[i][j]/artStats.numARTEverInitCD4Metric[i][j];	
				}	
			}
			if (artStats.numWithObservedCD4AtARTEverInit[i] > 0)
				artStats.observedCD4AtARTEverInitMean[i] = artStats.observedCD4AtARTEverInitSum[i]/artStats.numWithObservedCD4AtARTEverInit[i];
		}

		/** finalize stats for selected month efficacy totals */
		for (int j = 0; j < SimContext::ART_NUM_MTHS_RECORD; j++) {
			if (artStats.numOnARTAtMonth[i][j] > 0) {
				artStats.HVLDropsAtMonthMean[i][j] = artStats.HVLDropsAtMonthSum[i][j] / artStats.numOnARTAtMonth[i][j];
				artStats.HVLDropsAtMonthStdDev[i][j] = sqrt(artStats.HVLDropsAtMonthSumSquares[i][j] / artStats.numOnARTAtMonth[i][j] - artStats.HVLDropsAtMonthMean[i][j] * artStats.HVLDropsAtMonthMean[i][j]);
			}
		}

		/** finalize stats for STI */
		for (int j = 0; j < SimContext::STI_NUM_CYCLES; j++) {
			artStats.numSTIInterruptionsSum[i] += artStats.numSTIInterruptions[i][j];
		}
		if (artStats.numOnARTAtInit[i] > 0) {
			artStats.numSTIInterruptionsMean[i] = artStats.numSTIInterruptionsSum[i] / artStats.numOnARTAtInit[i];
		}
		if (artStats.numSTIInterruptionsSum[i] > 0) {
			artStats.monthsOnSTIInterruptionMean[i] = artStats.monthsOnSTIInterruptionSum[i] / artStats.numSTIInterruptionsSum[i];
		}
	}
} /* end finalizeARTStats */

/** \brief finalizeTimeSummaries calculates aggregate statistics for the TimeSummaries object */
void RunStats::finalizeTimeSummaries() {
	/** Finalize the TimeSummary objects stats */
	for (vector<TimeSummary *>::iterator i = timeSummaries.begin(); i != timeSummaries.end(); i++) {
		TimeSummary *currTime = *i;
		for (int k = 0; k < SimContext::HIV_CARE_NUM; k++){
			for (int j = 0; j < SimContext::DTH_NUM_CAUSES; j++){
				if (k != SimContext::HIV_CARE_NEG)
					currTime->numHIVPosDeathsType[j] += currTime->numDeathsTypeCare[j][k];
			}
		}
		for (int j = 0; j < SimContext::HIV_ID_NUM; j++) {
			currTime->numAliveWithCHRMs+=currTime->numAliveWithCHRMsDetState[j];
			currTime->numAliveWithoutCHRMs+=currTime->numAliveWithoutCHRMsDetState[j];
			for(int k=0;k<SimContext::CHRM_NUM;k++){
				currTime->numAliveCHRM[k]+=currTime->numAliveTypeCHRMs[j][k];
			}
		}

		if (currTime->numAlive > 0){
			// HIV-negative patients draw a baseline PTR but PTR is not used unless they become HIV-positive
			currTime->propRespMean = currTime->propRespSum / currTime->numAlive;
			currTime->propRespStdDev = sqrt(currTime->propRespSumSquares / currTime->numAlive - currTime->propRespMean * currTime->propRespMean);

			currTime->ageMean = currTime->ageSum / currTime->numAlive;
			currTime->ageStdDev = sqrt(currTime->ageSumSquares / currTime->numAlive - currTime->ageMean * currTime->ageMean);
		}
		if (currTime->numAlivePositive > 0){
			currTime->propRespMeanPositive = currTime->propRespSumPositive / currTime->numAlivePositive;
			currTime->propRespStdDevPositive = sqrt(currTime->propRespSumSquaresPositive / currTime->numAlivePositive - currTime->propRespMeanPositive * currTime->propRespMeanPositive);

			currTime->ageMeanPositive = currTime->ageSumPositive / currTime->numAlivePositive;
			currTime->ageStdDevPositive = sqrt(currTime->ageSumSquaresPositive / currTime->numAlivePositive - currTime->ageMeanPositive * currTime->ageMeanPositive);

			if (currTime->numAlivePositiveCD4Metric[SimContext::CD4_ABSOLUTE] > 0) {
				currTime->trueCD4Mean = currTime->trueCD4Sum / currTime->numAlivePositiveCD4Metric[SimContext::CD4_ABSOLUTE] ;
				currTime->trueCD4StdDev = sqrt(currTime->trueCD4SumSquares /  currTime->numAlivePositiveCD4Metric[SimContext::CD4_ABSOLUTE]  - currTime->trueCD4Mean * currTime->trueCD4Mean);
			}	
			if (currTime->numAlivePositiveCD4Metric[SimContext::CD4_PERC]  > 0) {
				currTime->trueCD4PercentageMean = currTime->trueCD4PercentageSum / currTime->numAlivePositiveCD4Metric[SimContext::CD4_PERC] ;
				currTime->trueCD4PercentageStdDev = sqrt(currTime->trueCD4PercentageSumSquares / currTime->numAlivePositiveCD4Metric[SimContext::CD4_PERC]  - currTime->trueCD4PercentageMean * currTime->trueCD4PercentageMean);
			}
			currTime->trueHVLMean = currTime->trueHVLSum /  currTime->numAlivePositive ;
			currTime->trueHVLStdDev = sqrt(currTime->trueHVLSumSquares /  currTime->numAlivePositive  - currTime->trueHVLMean * currTime->trueHVLMean);
		}

		if (currTime->numHIVPosWithObservedCD4 > 0) {
			currTime->observedCD4Mean = currTime->observedCD4Sum / currTime->numHIVPosWithObservedCD4;
			currTime->observedCD4StdDev = sqrt(currTime->observedCD4SumSquares / currTime->numHIVPosWithObservedCD4 - currTime->observedCD4Mean * currTime->observedCD4Mean);
		}
		int numObservedHVL = 0;
		for (int j = 0; j < SimContext::HVL_NUM_STRATA; j++) {
			numObservedHVL += currTime->observedHVLDistribution[j];
		}

		if (numObservedHVL > 0) {
			currTime->observedHVLMean = currTime->observedHVLSum / numObservedHVL;
			currTime->observedHVLStdDev = sqrt(currTime->observedHVLSumSquares / numObservedHVL - currTime->observedHVLMean * currTime->observedHVLMean);
		}

		int numObservedCD4Care[SimContext::HIV_CARE_NUM];

		for (int k = 0; k < SimContext::HIV_CARE_NUM; k++){
			numObservedCD4Care[k] = 0;
		}
		for (int j = 0; j < SimContext::CD4_NUM_STRATA; j++) {
			// start at 1 to skip HIVneg - if enum changes this needs to change too 
			for (int k = 1; k < SimContext::HIV_CARE_NUM; k++){
				numObservedCD4Care[k] += currTime->observedCD4DistributionCare[j][k];
			}
		}
		
		for (int j = 0; j < SimContext::HIV_CARE_NUM; j++){
			if(currTime->numAliveCare[j] > 0){
				currTime->propRespMeanCare[j] = currTime->propRespSumCare[j] / currTime->numAliveCare[j];
				currTime->propRespStdDevCare[j] = sqrt(currTime->propRespSumSquaresCare[j] / currTime->numAliveCare[j] - currTime->propRespMeanCare[j] * currTime->propRespMeanCare[j]);

				currTime->ageMeanCare[j] = currTime->ageSumCare[j] / currTime->numAliveCare[j];
				currTime->ageStdDevCare[j] = sqrt(currTime->ageSumSquaresCare[j] / currTime->numAliveCare[j] - currTime->ageMeanCare[j] * currTime->ageMeanCare[j]);
			}
			if(currTime->numAliveCareCD4Metric[j][SimContext::CD4_ABSOLUTE] > 0){
				currTime->trueCD4MeanCare[j] = currTime->trueCD4SumCare[j] / currTime->numAliveCareCD4Metric[j][SimContext::CD4_ABSOLUTE] ;
				currTime->trueCD4StdDevCare[j] = sqrt(currTime->trueCD4SumSquaresCare[j] / currTime->numAliveCareCD4Metric[j][SimContext::CD4_ABSOLUTE] - currTime->trueCD4MeanCare[j] * currTime->trueCD4MeanCare[j]);
			}
			if(numObservedCD4Care[j] > 0){
				currTime->observedCD4MeanCare[j] = currTime->observedCD4SumCare[j] / numObservedCD4Care[j];
				currTime->observedCD4StdDevCare[j] = sqrt(currTime->observedCD4SumSquaresCare[j] / numObservedCD4Care[j] - currTime->observedCD4MeanCare[j] * currTime->observedCD4MeanCare[j]);
			}
		}

		if (currTime->numAliveInCareOffART > 0){
			currTime->propRespMeanInCareOffART = currTime->propRespSumInCareOffART / currTime->numAliveInCareOffART;
			currTime->propRespStdDevInCareOffART = sqrt(currTime->propRespSumSquaresInCareOffART / currTime->numAliveInCareOffART - currTime->propRespMeanInCareOffART * currTime->propRespMeanInCareOffART);

			currTime->ageMeanInCareOffART = currTime->ageSumInCareOffART / currTime->numAliveInCareOffART;
			currTime->ageStdDevInCareOffART = sqrt(currTime->ageSumSquaresInCareOffART / currTime->numAliveInCareOffART - currTime->ageMeanInCareOffART * currTime->ageMeanInCareOffART);

			if (currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_ABSOLUTE] > 0){
				currTime->trueCD4MeanInCareOffART = currTime->trueCD4SumInCareOffART / currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_ABSOLUTE];
				currTime->trueCD4StdDevInCareOffART = sqrt(currTime->trueCD4SumSquaresInCareOffART / currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_ABSOLUTE] - currTime->trueCD4MeanInCareOffART * currTime->trueCD4MeanInCareOffART);
			}	
		}
		if (currTime->numWithObservedCD4InCareOffART > 0){
			currTime->observedCD4MeanInCareOffART = currTime->observedCD4SumInCareOffART / currTime->numWithObservedCD4InCareOffART;
			currTime->observedCD4StdDevInCareOffART = sqrt(currTime->observedCD4SumSquaresInCareOffART / currTime->numWithObservedCD4InCareOffART - currTime->observedCD4MeanInCareOffART * currTime->observedCD4MeanInCareOffART);
		}

		for (int i = 0; i < SimContext::ART_NUM_LINES; i++){
			for (int j = 0; j < SimContext::ART_EFF_NUM_TYPES; j++){
				if (currTime->numAliveOnART[i][j] > 0){
					currTime->propRespMeanOnART[i][j] = currTime->propRespSumOnART[i][j] / currTime->numAliveOnART[i][j];
					currTime->propRespStdDevOnART[i][j] = sqrt(currTime->propRespSumSquaresOnART[i][j] / currTime->numAliveOnART[i][j] - currTime->propRespMeanOnART[i][j] * currTime->propRespMeanOnART[i][j]);

					currTime->ageMeanOnART[i][j] = currTime->ageSumOnART[i][j] / currTime->numAliveOnART[i][j];
					currTime->ageStdDevOnART[i][j] = sqrt(currTime->ageSumSquaresOnART[i][j] / currTime->numAliveOnART[i][j] - currTime->ageMeanOnART[i][j] * currTime->ageMeanOnART[i][j]);
					if (currTime->numAliveOnARTCD4Metric[i][j][SimContext::CD4_ABSOLUTE] > 0){
						currTime->trueCD4MeanOnART[i][j] = currTime->trueCD4SumOnART[i][j] / currTime->numAliveOnARTCD4Metric[i][j][SimContext::CD4_ABSOLUTE];
						currTime->trueCD4StdDevOnART[i][j] = sqrt(currTime->trueCD4SumSquaresOnART[i][j] / currTime->numAliveOnARTCD4Metric[i][j][SimContext::CD4_ABSOLUTE] - currTime->trueCD4MeanOnART[i][j] * currTime->trueCD4MeanOnART[i][j]);
					}
				}
				if (currTime->observedCD4DistributionOnART[i][j] > 0){
					currTime->observedCD4MeanOnART[i][j] = currTime->observedCD4SumOnART[i][j] / currTime->observedCD4DistributionOnART[i][j];
					currTime->observedCD4StdDevOnART[i][j] = sqrt(currTime->observedCD4SumSquaresOnART[i][j] / currTime->observedCD4DistributionOnART[i][j] - currTime->observedCD4MeanOnART[i][j] * currTime->observedCD4MeanOnART[i][j]);
				}
			}
		}
		for (int j = 0; j < SimContext::ART_NUM_STATES; j++) {
			for (int k = 0; k < SimContext::CD4_NUM_STRATA; k++) {
				for (int l = 0; l < SimContext::HVL_NUM_STRATA; l++) {
					currTime->trueHVLDistribution[l] += currTime->trueCD4HVLARTDistribution[j][k][l];
					currTime->trueCD4ARTDistribution[j][k] += currTime->trueCD4HVLARTDistribution[j][k][l];
				}
			}
		}
	}
} /* end finalizeTimeSummaries */

/** \brief writePopulationSummary outputs the PopulationSummary statistics to the stats file */
void RunStats::writePopulationSummary() {
	int i;

	// Print out the section header
	fprintf(statsFile, "POPULATION SUMMARY MEASURES (run completed %s,", popSummary.runDate.c_str() );
	fprintf(statsFile, "%s)\n[Program version %s, build %s]", popSummary.runTime.c_str(),
		CepacUtil::CEPAC_VERSION_STRING, CepacUtil::CEPAC_EXECUTABLE_COMPILED_DATE);



	//Print out costs and LMs for multiple discount factors
	if (simContext->getRunSpecsInputs()->enableMultipleDiscountRates){
		for (i = 0; i < simContext->NUM_DISCOUNT_RATES; i++){
			double annualRateCost = pow(simContext->getRunSpecsInputs()->multDiscountRatesCost[i],12.0)-1;
			double annualRateBenefit = pow(simContext->getRunSpecsInputs()->multDiscountRatesBenefit[i],12.0)-1;

			fprintf(statsFile,"\n\tOutcome/Measure \tDiscount Rate \tAverage \tStd Dev");
			fprintf(statsFile,"\n\tCosts \t%1.2lf \t%1.0lf \t%1.0lf",
				annualRateCost, popSummary.multDiscCostsAverage[i], popSummary.multDiscCostsStdDev[i]);
			fprintf(statsFile,"\n\tLife Months \t%1.2lf \t%1.4lf \t%1.4lf",
				annualRateBenefit, popSummary.multDiscLMsAverage[i], popSummary.multDiscLMsStdDev[i]);
			fprintf(statsFile,"\n\tQuality-Adj Life Mths \t%1.2lf \t%1.4lf \t%1.4lf",
				annualRateBenefit, popSummary.multDiscQALMsAverage[i], popSummary.multDiscQALMsStdDev[i]);
		}
	}
	else{
		// Print out costs and LM totals
		fprintf(statsFile,"\n\tOutcome/Measure \tAverage \tStd Dev \tLB \tUB");
		fprintf(statsFile,"\n\tCosts \t%1.0lf \t%1.0lf \t%1.0lf \t%1.0lf",
			popSummary.costsAverage, popSummary.costsStdDev,
			popSummary.costsLowerBound, popSummary.costsUpperBound);
		fprintf(statsFile,"\n\tLife Months \t%1.4lf \t%1.4lf \t%1.4lf \t%1.4lf",
			popSummary.LMsAverage, popSummary.LMsStdDev,
			popSummary.LMsLowerBound, popSummary.LMsUpperBound);
		fprintf(statsFile,"\n\tQuality-Adj Life Mths \t%1.4lf \t%1.4lf \t%1.4lf \t%1.4lf",
			popSummary.QALMsAverage, popSummary.QALMsStdDev,
			popSummary.QALMsLowerBound, popSummary.QALMsUpperBound);
	}

	//Output averages by subcohorts
	if (simContext->getOutputInputs()->enableSubCohorts){
		fprintf(statsFile, "\n\tSub-Cohort Size\tAverage Costs\tStd Dev of Costs\tAverage LMs\tStd Dev of LMs\tAvg QALMs\tStd Dev of QALMs");
		for(int i = 0; i < SimContext::MAX_NUM_SUBCOHORTS; i++){
			if(simContext->getOutputInputs()->subCohorts[i]>0){
				fprintf(statsFile, "\n\t%d\t%1.0lf\t%1.0lf\t%1.4lf\t%1.4lf\t%1.4lf\t%1.4lf",
						simContext->getOutputInputs()->subCohorts[i],
						popSummary.costsAverageCohortParsing[i], popSummary.costsStdDevCohortParsing[i],
						popSummary.LMsAverageCohortParsing[i], popSummary.LMsStdDevCohortParsing[i],
						popSummary.QALMsAverageCohortParsing[i], popSummary.QALMsStdDevCohortParsing[i]);
			}
		}
	}
	// output avg costs, LMs, QALMs by line of ARTs
	fprintf(statsFile,"\n\t \tAvg Costs \tAvg LMs \tAvg QALMs");
	for (i = 0; i < SimContext::ART_NUM_LINES+1; ++i) {
		fprintf(statsFile,"\n\tUp to %d ART(s) obsv fail \t%1.0lf \t%1.4lf \t%1.4lf", i,
			popSummary.costsFailARTAverage[i], popSummary.LMsFailARTAverage[i],
			popSummary.QALMsFailARTAverage[i]);
	}
	fprintf(statsFile,"\n\tOnly HIV+ patients \t%1.0lf \t%1.4lf \t%1.4lf",
		popSummary.costsHIVPositiveAverage, popSummary.LMsHIVPositiveAverage,
		popSummary.QALMsHIVPositiveAverage);
	fprintf(statsFile,"\n\tTotal Clinic Visits \t%1lu", popSummary.totalClinicVisits);

} /* end writePopulationSummary */

/** \brief writeHIVScreening outputs the HIVScreening statistics to the stats file */
void RunStats::writeHIVScreening() {
	int i, j;

	fprintf(statsFile, "\nHIV SCREENING MODULE");
	fprintf(statsFile, "\n\tOverall Prevalence");
	fprintf(statsFile, "\n\tHIV+ Cases (Preval):\t%lu", hivScreening.numPrevalentCases);
	fprintf(statsFile, "\t\tHIV- Cases (Preval):\t%lu", hivScreening.numHIVNegativeAtInit);
	fprintf(statsFile, "\n\tIncidence");
	fprintf(statsFile, "\n\t\tTotal\tNever PrEP\tPrEP Dropout When Infected\tOn PrEP When Infected");
	fprintf(statsFile, "\n\tHIV+ Cases (Incident):\t%lu\t%lu\t%lu\t%lu", hivScreening.numIncidentCases, hivScreening.numIncidentCasesByPrEPState[SimContext::HIV_POS_NEVER_PREP], hivScreening.numIncidentCasesByPrEPState[SimContext::HIV_POS_PREP_DROPOUT], hivScreening.numIncidentCasesByPrEPState[SimContext::HIV_POS_ON_PREP]);
	fprintf(statsFile, "\n\t\tMean\tSD");
	fprintf(statsFile, "\n\tMths to Inf (Incid):\t%1.2lf\t%1.2lf",
		hivScreening.monthsToInfectionAverage, hivScreening.monthsToInfectionStdDev);
	fprintf(statsFile, "\n\n\tTotal HIV+ Cases:\t%lu", hivScreening.numHIVPositiveTotal);
	fprintf(statsFile,"\n\n\t# Ever on PrEP (all patients)\t%lu", hivScreening.numEverPrEP);
	fprintf(statsFile,"\n\t# Drop out of PrEP (all patients)\t%lu", hivScreening.numDropoutPrEP);
	fprintf(statsFile,"\n\tOnly HIV- patients (never infected)");
	fprintf(statsFile,"\n\t\tNum Patients\tAvg LMs \tAvg QALMs");
	fprintf(statsFile,"\n\tEver on PrEP \t%lu \t%1.4lf \t%1.4lf ",
		hivScreening.numNeverHIVPositive[SimContext::EVER_PREP], hivScreening.LMsHIVNegativeAverage[SimContext::EVER_PREP],
		hivScreening.QALMsHIVNegativeAverage[SimContext::EVER_PREP] );
	fprintf(statsFile,"\n\tNever on PrEP \t%lu \t%1.4lf \t%1.4lf ",
		hivScreening.numNeverHIVPositive[SimContext::NEVER_PREP], hivScreening.LMsHIVNegativeAverage[SimContext::NEVER_PREP],
		hivScreening.QALMsHIVNegativeAverage[SimContext::NEVER_PREP]);	
	fprintf(statsFile, "\n\tTotal HIV Exposed (Pediatric)");
	fprintf(statsFile, "\n\t\tMother Chronic HIV (Preganancy)\tMother Acute HIV (Pregnancy)\t\t\tMother Acute HIV (Breastfeeding)\tHIV- Never Exposed");
	fprintf(statsFile, "\n\tBefore Birth");
	for (i = 0; i < SimContext::MOM_ACUTE_BREASTFEEDING; ++i) 
		fprintf(statsFile,"\t%lu", hivScreening.numHIVExposed[i]);
	fprintf(statsFile, "\t\tAfter Birth");	
	fprintf(statsFile, "\t%lu\t%lu", hivScreening.numHIVExposed[SimContext::MOM_ACUTE_BREASTFEEDING], hivScreening.numNeverHIVExposed);

	fprintf(statsFile, "\n\t\tFalse Positive\tFalse Positive Linked");
	fprintf(statsFile, "\n\tLife Months\t%1lu\t%1lu", hivScreening.LMsFalsePositive, hivScreening.LMsFalsePositiveLinked);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::HIV_EXT_INF_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::HIV_EXT_INF_STRS[(i+1)%SimContext::HIV_EXT_INF_NUM ]);
	fprintf(statsFile, "\n\tInit States (Prevalence):");
	for (i = 0; i < SimContext::HIV_EXT_INF_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numPatientsInitialHIVState[(i+1)%SimContext::HIV_EXT_INF_NUM ]);
	fprintf(statsFile, "\n\tLinked to HIV Care at Init:");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%lu", hivScreening.numLinkedAtInit[i]);
	fprintf(statsFile, "\n\n\tCD4 at Detection (Preval):");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%s", SimContext::HIV_POS_STRS[i]);
	fprintf(statsFile, "\tTotal");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
			fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentCD4HIV[j][i]);
		}
		fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentCD4[j]);
	}
	fprintf(statsFile, "\n\tTotal");	
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentHIV[i]);
	fprintf(statsFile, "\n\tMean CD4 Count at Det (Preval):");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
		fprintf(statsFile, "\t%1.2lf", hivScreening.CD4AtDetectionPrevalentAverageHIV[i]);
	}
	fprintf(statsFile, "\t%1.2lf", hivScreening.CD4AtDetectionPrevalentAverage);
	if(simContext->getPedsInputs()->enablePediatricsModel){
		fprintf(statsFile, "\n\tNum w.Absolute CD4 Metric at Detection (Preval):");
		for (i = 0; i  < SimContext::HIV_POS_NUM; i++)
			fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentHIVCD4Metric[i][SimContext::CD4_ABSOLUTE]);
		fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentCD4Metric[SimContext::CD4_ABSOLUTE]);	
	}
	else{
		fprintf(statsFile, "\n");
	}

	fprintf(statsFile, "\n\tHVL at Detection (Preval):");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%s", SimContext::HIV_POS_STRS[i]);
	fprintf(statsFile, "\tTotal");
	for (j = SimContext::HVL_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile, "\n\t%s", SimContext::HVL_STRATA_STRS[j]);
		for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
			fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentHVLHIV[j][i]);
		}
		fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionPrevalentHVL[j]);
	}
	fprintf(statsFile, "\n\t\tMean\tSD");
	fprintf(statsFile, "\n\tMths from model start to det (Preval):\t%1.2lf\t%1.2lf", hivScreening.monthsToDetectionPrevalentAverage, hivScreening.monthsToDetectionPrevalentStdDev);
	fprintf(statsFile, "\n\tAge in mths at det (Preval):\t%1.2lf\t%1.2lf",
		hivScreening.ageMonthsAtDetectionPrevalentAverage, hivScreening.ageMonthsAtDetectionPrevalentStdDev);

	fprintf(statsFile, "\n\n\tCD4 at Detection (Incid):");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%s", SimContext::HIV_POS_STRS[i]);
	fprintf(statsFile, "\tTotal\tTotal %%");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
			fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionIncidentCD4HIV[j][i]);
		}
		fprintf(statsFile, "\t%lu\t%1.2lf", hivScreening.numAtDetectionIncidentCD4[j],
			hivScreening.percentAtDetectionIncidentCD4[j]);
	}
	fprintf(statsFile, "\n\tTotal");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionIncidentHIV[i]);
	fprintf(statsFile, "\n\tMean CD4 Count at Det (Incid):");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
		fprintf(statsFile, "\t%1.2lf", hivScreening.CD4AtDetectionIncidentAverageHIV[i]);
	}
	fprintf(statsFile, "\t%1.2lf", hivScreening.CD4AtDetectionIncidentAverage);
	if(simContext->getPedsInputs()->enablePediatricsModel){
		fprintf(statsFile, "\n\tNum w.Absolute CD4 Metric at Detection (Incid):");
		for (i = 0; i  < SimContext::HIV_POS_NUM; i++)
			fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionIncidentHIVCD4Metric[i][SimContext::CD4_ABSOLUTE]);
		fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionIncidentCD4Metric[SimContext::CD4_ABSOLUTE]);	
	}
	else{
		fprintf(statsFile, "\n");
	}

	fprintf(statsFile, "\n\tHVL at Detection (Incid):");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%s", SimContext::HIV_POS_STRS[i]);
	fprintf(statsFile, "\tTotal\tTotal %%");
	for (j = SimContext::HVL_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile, "\n\t%s", SimContext::HVL_STRATA_STRS[j]);
		for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
			fprintf(statsFile, "\t%lu", hivScreening.numAtDetectionIncidentHVLHIV[j][i]);
		}
		fprintf(statsFile, "\t%lu\t%1.2lf", hivScreening.numAtDetectionIncidentHVL[j],
			hivScreening.percentAtDetectionIncidentHVL[j]);
	}
	fprintf(statsFile, "\n\t\tMean\tSD");
	fprintf(statsFile, "\n\tMths from model start to det (Incid):\t%1.2lf\t%1.2lf", hivScreening.monthsToDetectionIncidentAverage, hivScreening.monthsToDetectionIncidentStdDev);
	fprintf(statsFile, "\n\tMths from HIV inf to det (Incid):\t%1.2lf\t%1.2lf", 
	hivScreening.monthsAfterInfectionToDetectionAverage, hivScreening.monthsAfterInfectionToDetectionStdDev);
	fprintf(statsFile, "\n\tAge in mths at det (Incid):\t%1.2lf\t%1.2lf",
		hivScreening.ageMonthsAtDetectionIncidentAverage, hivScreening.ageMonthsAtDetectionIncidentStdDev);

	fprintf(statsFile, "\n\n\tCD4 at Linkage:");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  // don't start at HIV neg; make sure mod this if HIVneg #def chgs
		fprintf(statsFile, "\t%s", SimContext::HIV_POS_STRS[i]);
	fprintf(statsFile, "\tTotal");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
			fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageCD4HIV[j][i]);
		}
		fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageCD4[j]);
	}
	fprintf(statsFile, "\n\tTotal");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageHIV[i]);
	fprintf(statsFile, "\n\tMean CD4 Count at Linkage:");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
		fprintf(statsFile, "\t%1.2lf", hivScreening.CD4AtLinkageAverageHIV[i]);
	}
	fprintf(statsFile, "\t%1.2lf", hivScreening.CD4AtLinkageAverage);
	if(simContext->getPedsInputs()->enablePediatricsModel){
		fprintf(statsFile, "\n\tNum w.Absolute CD4 Metric at Linkage:");
		for (i = 0; i  < SimContext::HIV_POS_NUM; i++)
			fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageHIVCD4Metric[i][SimContext::CD4_ABSOLUTE]);
		fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageCD4Metric[SimContext::CD4_ABSOLUTE]);	
	}
	else{
		fprintf(statsFile, "\n");
	}

	fprintf(statsFile, "\n\tHVL at Linkage:");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)  
		fprintf(statsFile, "\t%s", SimContext::HIV_POS_STRS[i]);
	fprintf(statsFile, "\tTotal");
	for (j = SimContext::HVL_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile, "\n\t%s", SimContext::HVL_STRATA_STRS[j]);
		for (i = 0; i < SimContext::HIV_POS_NUM; ++i) {
			fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageHVLHIV[j][i]);
		}
		fprintf(statsFile, "\t%lu", hivScreening.numAtLinkageHVL[j]);
	}
	fprintf(statsFile, "\n\t\tMean\tSD");
	fprintf(statsFile, "\n\tMths from model start to linkage:\t%1.2lf\t%1.2lf", hivScreening.monthsToLinkageAverage, hivScreening.monthsToLinkageStdDev);
	fprintf(statsFile, "\n\tAge in mths at linkage:\t%1.2lf\t%1.2lf",
		hivScreening.ageMonthsAtLinkageAverage, hivScreening.ageMonthsAtLinkageStdDev);

	fprintf(statsFile, "\n\t\tMale\tFemale");
	fprintf(statsFile, "\n\tGender at Detection:\t%lu\t%lu",
		hivScreening.numDetectedGender[SimContext::GENDER_MALE], hivScreening.numDetectedGender[SimContext::GENDER_FEMALE]);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::HIV_DET_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::HIV_DET_STRS[i]);
	fprintf(statsFile, "\n\tMethod of Detection (Preval):");
	for (i = 0; i < SimContext::HIV_DET_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numDetectedPrevalentMeans[i]);
	fprintf(statsFile, "\n\tMethod of Detection (Incid):");
	for (i = 0; i < SimContext::HIV_DET_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numDetectedIncidentMeans[i]);
	fprintf(statsFile,"\n\tMethod of Linkage:");
	for (i = 0; i < SimContext::HIV_DET_NUM; i++)
		fprintf(statsFile, "\t%lu", hivScreening.numLinkedMeans[i]);
	fprintf(statsFile,"\n\tMean months to linkage:");
	for (i = 0 ; i < SimContext::HIV_DET_NUM; i++)
		fprintf(statsFile,"\t%1.2lf", hivScreening.monthsToLinkageAverageMeans[i]);
	fprintf(statsFile,"\n\tSD(mo to linkage):");
	for (i = 0 ; i < SimContext::HIV_DET_NUM; i++)
		fprintf(statsFile,"\t%1.2lf", hivScreening.monthsToLinkageStdDevMeans[i]);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::OI_STRS[i]);
	fprintf(statsFile, "\n\tPresenting OI:");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numDetectedByOIs[i]);
	fprintf(statsFile, "\n\tPresenting OI (Previously Detected):");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numDetectedByOIsPrevDetected[i]);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::HIV_EXT_INF_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::HIV_EXT_INF_STRS[(i+1)%SimContext::HIV_EXT_INF_NUM ]);
	for (j = 0; j < SimContext::TEST_ACCEPT_NUM; ++j) {
		fprintf(statsFile, "\n\tPatient Test Accept Distrib %d:", j+1);
		for (i = 0; i < SimContext::HIV_EXT_INF_NUM; ++i)
			fprintf(statsFile, "\t%lu", hivScreening.numTestingAcceptProb[j][(i+1)%SimContext::HIV_EXT_INF_NUM]);
	}
	fprintf(statsFile, "\n\t\tAccept\tRefuse");
	fprintf(statsFile, "\n\tHIV Test accepts:\t%lu\t%lu", hivScreening.numAcceptTest, hivScreening.numRefuseTest);
	fprintf(statsFile, "\n\tHIV Test returns:\t%lu\t%lu", hivScreening.numReturnForResults, hivScreening.numNoReturnForResults);
	fprintf(statsFile, "\n\tLab Staging accepts:\t%lu\t%lu", hivScreening.numAcceptLabStaging, hivScreening.numRefuseLabStaging);
	fprintf(statsFile, "\n\tLab Staging returns:\t%lu\t%lu", hivScreening.numReturnForResultsLabStaging, hivScreening.numNoReturnForResultsLabStaging);
	fprintf(statsFile, "\n\tLinkage To Care (Lab Staging):\t%lu\t%lu", hivScreening.numLinkLabStaging, hivScreening.numNoLinkLabStaging);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::HIV_TEST_FREQ_NUM; ++i)
		fprintf(statsFile, "\t[/%dmths]", simContext->getHIVTestInputs()->HIVTestingInterval[i]);
	fprintf(statsFile, "\n\tPatient Testing Freq in Prog:");
	for (i = 0; i < SimContext::HIV_TEST_FREQ_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numTestingInterval[i]);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::TEST_RESULT_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::TEST_RESULT_STRS[i]);
	fprintf(statsFile, "\tTotal");
	fprintf(statsFile, "\n\tNum Test Results (Preval):\t%lu\tN/A\tN/A\t%lu\t%lu",
		hivScreening.numTestResultsPrevalentType[SimContext::TEST_TRUE_POS],
		hivScreening.numTestResultsPrevalentType[SimContext::TEST_FALSE_NEG],
		hivScreening.numTestResultsPrevalent);
	fprintf(statsFile, "\n\tNum Test Results (Incid):\t%lu\tN/A\tN/A\t%lu\t%lu",
		hivScreening.numTestResultsIncidentType[SimContext::TEST_TRUE_POS],
		hivScreening.numTestResultsIncidentType[SimContext::TEST_FALSE_NEG],
		hivScreening.numTestResultsIncident);
	fprintf(statsFile, "\n\tNum Test Results (HIVneg):\tN/A\t%lu\t%lu\tN/A\t%lu",
		hivScreening.numTestResultsHIVNegativeType[SimContext::TEST_FALSE_POS],
		hivScreening.numTestResultsHIVNegativeType[SimContext::TEST_TRUE_NEG],
		hivScreening.numTestResultsHIVNegative);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::HIV_EXT_INF_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::HIV_EXT_INF_STRS[(i+1)%SimContext::HIV_EXT_INF_NUM]);
	fprintf(statsFile, "\n\tNum Tests Done by State:");
	for (i = 0; i < SimContext::HIV_EXT_INF_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numTestsHIVState[(i+1)%SimContext::HIV_EXT_INF_NUM]);

	fprintf(statsFile, "\n\tNum Lab Staging Accepts by State:");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numAcceptLabStagingHIVState[i]);
	fprintf(statsFile, "\n\tNum Lab Staging Return for Results by State:");
	for (i = 0; i < SimContext::HIV_POS_NUM; ++i)
		fprintf(statsFile, "\t%lu", hivScreening.numReturnLabStagingHIVState[i]);

	fprintf(statsFile, "\n\tNum Lab Staging Return for Results\t");
	for (i = 0; i < SimContext::CD4_NUM_STRATA; i++)
		fprintf(statsFile, "\tTRUE");
	fprintf(statsFile, "\tTOTAL");
	fprintf(statsFile, "\n\t\t");
	for (i = 0; i < SimContext::CD4_NUM_STRATA; i++)
		fprintf(statsFile, "\t%s", SimContext::CD4_STRATA_STRS[i]);

	for (i = 0; i < SimContext::CD4_NUM_STRATA; i++){
		fprintf(statsFile, "\n\tOBSV\t%s", SimContext::CD4_STRATA_STRS[i]);
		for (j = 0; j < SimContext::CD4_NUM_STRATA; j++){
			fprintf(statsFile, "\t%lu", hivScreening.numReturnLabStagingObsvTrueCD4[i][j]);
		}
		fprintf(statsFile, "\t%lu", hivScreening.numReturnLabStagingObsvCD4[i]);
	}
	fprintf(statsFile, "\n\t\tTOTAL");
	for (j = 0 ; j < SimContext::CD4_NUM_STRATA; j++){
		fprintf(statsFile, "\t%lu", hivScreening.numReturnLabStagingTrueCD4[j]);
	}
	fprintf(statsFile, "\t%lu", hivScreening.numReturnForResultsLabStaging);

	fprintf(statsFile, "\n\tNum Lab Staging Link to Care\t");
	for (i = 0; i < SimContext::CD4_NUM_STRATA; i++)
		fprintf(statsFile, "\tTRUE");
	fprintf(statsFile, "\tTOTAL");
	fprintf(statsFile, "\n\t\t");
	for (i = 0; i < SimContext::CD4_NUM_STRATA; i++)
		fprintf(statsFile, "\t%s", SimContext::CD4_STRATA_STRS[i]);

	for (i = 0; i < SimContext::CD4_NUM_STRATA; i++){
		fprintf(statsFile, "\n\tOBSV\t%s", SimContext::CD4_STRATA_STRS[i]);
		for (j = 0; j < SimContext::CD4_NUM_STRATA; j++){
			fprintf(statsFile, "\t%lu", hivScreening.numLinkLabStagingObsvTrueCD4[i][j]);
		}
		fprintf(statsFile, "\t%lu", hivScreening.numLinkLabStagingObsvCD4[i]);
	}
	fprintf(statsFile, "\n\t\tTOTAL");
	for (j = 0 ; j < SimContext::CD4_NUM_STRATA; j++){
		fprintf(statsFile, "\t%lu", hivScreening.numLinkLabStagingTrueCD4[j]);
	}
	fprintf(statsFile, "\t%lu", hivScreening.numLinkLabStaging);
} /* end HIVScreening */

/** \brief writeSurvivalStats outputs the SurvivalStats statistics to the stats file */
void RunStats::writeSurvivalStats() {
	int numTruncate = patients.size() * TRUNC_HISTOGRAM_PERC / 100;
	for (int i = 0; i < NUM_SURVIVAL_GROUPS; i++) {
		SurvivalStats &currSurvival = survivalStats[i];

		// Print out the section headers
		switch (i) {
			case SURVIVAL_ALL:
				fprintf(statsFile, "\nLIFE MONTH SURVIVAL OF ENTIRE HIV+ COHORT"); break;
			case SURVIVAL_EXCL_LONG:
				fprintf(statsFile, "\nLIFE MONTH SURVIVAL EXCLUDING LONGEST %d%% (%d HIV+ patients) LMs",
					TRUNC_HISTOGRAM_PERC, numTruncate); break;
			case SURVIVAL_EXCL_SHORT:
				fprintf(statsFile, "\nLIFE MONTH SURVIVAL EXCLUDING SHORTEST %d%% (%d HIV+ patients) LMs",
					TRUNC_HISTOGRAM_PERC, numTruncate); break;
			case SURVIVAL_EXCL_LONG_AND_SHORT:
				fprintf(statsFile, "\nLIFE MONTH SURVIVAL EXCLUDING LONGEST & SHORTEST %d%% (%d HIV+ patients) LMs",
					TRUNC_HISTOGRAM_PERC, numTruncate*2);break;
		}

		// Print out the histogram information
		fprintf(statsFile, "\n\tLM bucket (UB, excl):");
		for (map<int,int>::iterator j = currSurvival.LMsHistogram.begin();
			j != currSurvival.LMsHistogram.end(); j++) {
				fprintf(statsFile, "\t%1ld", j->first);
		}
		fprintf(statsFile, "\n\t# Patients:");
		for (map<int,int>::iterator j = currSurvival.LMsHistogram.begin();
			j != currSurvival.LMsHistogram.end(); j++) {
				fprintf(statsFile, "\t%1ld", j->second);
		}

		// output min, max, median, & mean values
		fprintf(statsFile, "\n\tLM Min:\t%1.4lf\t\tLM Max:\t%1.4lf",
			currSurvival.LMsMin, currSurvival.LMsMax);
		fprintf(statsFile, "\n\tLM Median:\t%1.4lf\t\tLM AvgDev:\t%1.4lf",
			currSurvival.LMsMedian, currSurvival.LMsAverageDeviationMedian);
		fprintf(statsFile, "\n\tLM Mean:\t%1.4lf\t\tLM StdDev:\t%1.4lf\t\tLM AvgDev:\t%1.4lf",
			currSurvival.LMsMean, currSurvival.LMsStdDev, currSurvival.LMsAverageDeviation);
		fprintf(statsFile, "\n\tLM Variance:\t%1.4lf", currSurvival.LMsVariance);
		fprintf(statsFile, "\t\tLM Skew:\t%1.4lf\t\tLM Kurtosis:\t%1.4lf",
			currSurvival.LMsSkew, currSurvival.LMsKurtosis);

		// output cost and QALM values
		fprintf(statsFile, "\n\tCost Mean:\t%1.2lf\t\tCost StdDev:\t%1.2lf",
			currSurvival.costsMean, currSurvival.costsStdDev);
		fprintf(statsFile, "\n\tQALM Mean:\t%1.4lf\t\tQALM StdDev:\t%1.4lf",
			currSurvival.QALMsMean, currSurvival.QALMsStdDev);
	}
} /* end writeSurvivalStats */

/** \brief writeInitialDistributions outputs the InitialDistributions statistics to the stats file */
void RunStats::writeInitialDistributions() {
	int i, j;
	fprintf(statsFile,"\nINITIAL DISTRIBUTIONS");
    fprintf(statsFile,"\n\tCD4 Count Level \t# Patients \t\tHVL Setpt Lvl \t# Patients \t\tCurr HVL Lvl \t# Patients");
	fprintf(statsFile,"\n\tVHI (>500) \t%1ld \t\tVHI (>100k) \t%1ld \t\tVHI \t%1ld",
		initialDistributions.numPatientsCD4Level[SimContext::CD4_VHI],
		initialDistributions.numPatientsHVLLevel[SimContext::HVL_VHI],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL_VHI] );
	fprintf(statsFile,"\n\tHI (300-500) \t%1ld \t\tHI (30k-100k) \t%1ld \t\tHI \t%1ld",
		initialDistributions.numPatientsCD4Level[SimContext::CD4__HI],
		initialDistributions.numPatientsHVLLevel[SimContext::HVL__HI],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL__HI] );
	fprintf(statsFile,"\n\tMHI (200-300) \t%1ld \t\tMHI (10k-30k) \t%1ld \t\tMHI \t%1ld",
		initialDistributions.numPatientsCD4Level[SimContext::CD4_MHI],
		initialDistributions.numPatientsHVLLevel[SimContext::HVL_MHI],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL_MHI] );
	fprintf(statsFile,"\n\tMLO (100-200) \t%1ld \t\tMED (3k-10k) \t%1ld \t\tMED \t%1ld",
		initialDistributions.numPatientsCD4Level[SimContext::CD4_MLO],
		initialDistributions.numPatientsHVLLevel[SimContext::HVL_MED],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL_MED] );
	fprintf(statsFile,"\n\tLO (50-100) \t%1ld \t\tMLO (500-3k) \t%1ld \t\tMLO \t%1ld",
		initialDistributions.numPatientsCD4Level[SimContext::CD4__LO],
		initialDistributions.numPatientsHVLLevel[SimContext::HVL_MLO],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL_MLO] );
	fprintf(statsFile,"\n\tVLO (0-50) \t%1ld \t\tLO (20-500) \t%1ld \t\tLO \t%1ld",
		initialDistributions.numPatientsCD4Level[SimContext::CD4_VLO],
		initialDistributions.numPatientsHVLLevel[SimContext::HVL__LO],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL__LO] );
	fprintf(statsFile,"\n\t \t \t\tVLO (0-20) \t%1ld \t\tVLO \t%1ld",
		initialDistributions.numPatientsHVLLevel[SimContext::HVL_VLO],
		initialDistributions.numPatientsHVLSetpointLevel[SimContext::HVL_VLO] );

	fprintf(statsFile,"\n\tAvg Init Age(Mths): \t%1.0lf \n\tMale Patients: \t%1ld \t\tFemale Patients: \t%1ld\n\t",
		initialDistributions.averageInitialAgeMonths,
		initialDistributions.numMalePatients, initialDistributions.numFemalePatients);
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
	fprintf(statsFile,"\n\tPrior OI Histories Distrib:");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%ld", initialDistributions.numPriorOIHistories[i]);
	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::CD4_RESPONSE_NUM_TYPES; i++)
		fprintf(statsFile, "\t%s", SimContext::CD4_RESPONSE_STRS[i]);
	fprintf(statsFile, "\n\tART Response Types");
	for (i = 0; i < SimContext::CD4_RESPONSE_NUM_TYPES; i++)
		fprintf(statsFile, "\t%ld", initialDistributions.numARTResposneTypes[i]);

	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::RISK_FACT_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::RISK_FACT_STRS[i]);
	}
	fprintf(statsFile, "\n\tRisk Factors");
	for (i = 0; i < SimContext::RISK_FACT_NUM; i++) {
		fprintf(statsFile, "\t%ld", initialDistributions.numRiskFactors[i]);
	}
	fprintf(statsFile, "\n\tPeds/Maternal");
	for (i = 0; i < SimContext::PEDS_MATERNAL_STATUS_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::PEDS_MATERNAL_STATUS_STRS[i]);
	}
	for (i = 0; i < SimContext::PEDS_MATERNAL_STATUS_NUM; i++) {
		if (i == SimContext::PEDS_HIV_POS_PP)
			continue;
		fprintf(statsFile, "\n\t%s", SimContext::PEDS_HIV_STATE_STRS[i]);
		for (j = 0; j < SimContext::PEDS_MATERNAL_STATUS_NUM; j++) {
			fprintf(statsFile, "\t%ld", initialDistributions.numInitialPediatrics[i][j]);
		}
	}
} /* end writeInitialDistributions */

/** \brief writeCHRMsStats outputs the CHRMsStats statistics to the stats file */
void RunStats::writeCHRMsStats() {
	fprintf(statsFile, "\nCHRMs SUMMARIES");
	fprintf(statsFile, "\n\tTotal Patients with CHRM\n\t");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile, "\n\tHIV+");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1lu", chrmsStats.numPatientsWithCHRMHIVPos[i]);
	}
	fprintf(statsFile, "\n\tHIV-");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1lu", chrmsStats.numPatientsWithCHRMHIVNeg[i]);
	}
	fprintf(statsFile, "\n\tTotal");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1lu", chrmsStats.numPatientsWithCHRM[i]);
	}
	fprintf(statsFile, "\n\tAvg Mths with CHRM\n\t");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile, "\n\tHIV+");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1.0lf", chrmsStats.numPatientsWithCHRMHIVPos[i]==0?0:overallSurvival.LMsCHRMHistoryCHRMsHIVPos[i]/chrmsStats.numPatientsWithCHRMHIVPos[i]);
	}	
	fprintf(statsFile, "\n\tHIV-");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1.0lf", chrmsStats.numPatientsWithCHRMHIVNeg[i]==0?0:overallSurvival.LMsCHRMHistoryCHRMsHIVNeg[i]/chrmsStats.numPatientsWithCHRMHIVNeg[i]);
	}	
	fprintf(statsFile, "\n\tHIV+ Patients");
	fprintf(statsFile, "\n\tPrevalent");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile, "\tTotal");
	for (int j = SimContext::CD4_NUM_STRATA - 1; j >= 0; j--) {
		fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (int i = 0; i < SimContext::CHRM_NUM; i++) {
			fprintf(statsFile, "\t%ld", chrmsStats.numPrevalentCHRMCD4[i][j]);
		}
		fprintf(statsFile, "\t%ld", chrmsStats.numPrevalentCD4[j]);
	}
	fprintf(statsFile, "\n\tTotal");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%ld", chrmsStats.numPrevalentCHRM[i]);
	}

	fprintf(statsFile, "\n\tIncident");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile, "\tTotal");
	for (int j = SimContext::CD4_NUM_STRATA - 1; j >= 0; j--) {
		fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (int i = 0; i < SimContext::CHRM_NUM; i++) {
			fprintf(statsFile, "\t%ld", chrmsStats.numIncidentCHRMCD4[i][j]);
		}
		fprintf(statsFile, "\t%ld", chrmsStats.numIncidentCD4[j]);
	}
	fprintf(statsFile, "\n\tTotal");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%ld", chrmsStats.numIncidentCHRM[i]);
	}

	fprintf(statsFile, "\n\tDeaths");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile, "\tTotal");
	for (int j = SimContext::CD4_NUM_STRATA - 1; j >= 0; j--) {
		fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (int i = 0; i < SimContext::CHRM_NUM; i++) {
			fprintf(statsFile, "\t%ld", chrmsStats.numDeathsCHRMCD4[i][j]);
		}
		fprintf(statsFile, "\t%ld", chrmsStats.numDeathsCD4[j]);
	}
	fprintf(statsFile, "\n\tTotal");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%ld", chrmsStats.numDeathsCHRM[i]);
	}
	fprintf(statsFile, "\n\tHIV- Patients");
	fprintf(statsFile, "\n\t");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile, "\tTotal");
	fprintf(statsFile, "\n\tPrevalent");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1lu", chrmsStats.numPrevalentCHRMHIVNeg[i]);
	}
	fprintf(statsFile, "\t%1lu", chrmsStats.numHIVNegWithPrevalentCHRMs);
	fprintf(statsFile, "\n\tIncident");
	for (int i = 0; i < SimContext::CHRM_NUM; i++) {
		fprintf(statsFile, "\t%1lu", chrmsStats.numIncidentCHRMHIVneg[i]);
	}	
	fprintf(statsFile, "\t%1lu", chrmsStats.numHIVNegWithIncidentCHRMs);
} /* end writeCHRMsStats */

/** \brief writeOIStats outputs the OIStats statistics to the stats file */
void RunStats::writeOIStats() {
	int i, j, k;
	fprintf(statsFile,"\nOI SUMMARIES");
	fprintf(statsFile,"\n\tType/OI");

	// Write out total OI summaries
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i] );
	fprintf(statsFile,"\n\t# Primary OIs");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%1lu", oiStats.numPrimaryOIsOI[i]);
	fprintf(statsFile,"\n\t# Secondary OIs");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%1lu", oiStats.numSecondaryOIsOI[i]);
	fprintf(statsFile, "\t\t# OIs Total\t%1lu", oiStats.numOIEventsTotal);
	// Write out number of primary, secondary, and detected OIs
	fprintf(statsFile,"\n\tPrimary OIs");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
	fprintf(statsFile," \tTotal");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile,"\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::OI_NUM; ++i)
			fprintf(statsFile," \t%1lu", oiStats.numPrimaryOIsCD4OI[j][i]);
		fprintf(statsFile," \t%1lu", oiStats.numPrimaryOIsCD4[j]);
	}
	fprintf(statsFile,"\n\tSecondary OIs");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
	fprintf(statsFile," \tTotal");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile,"\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::OI_NUM; ++i)
			fprintf(statsFile,"\t%1lu", oiStats.numSecondaryOIsCD4OI[j][i]);
		fprintf(statsFile,"\t%1lu", oiStats.numSecondaryOIsCD4[j]);
	}
	fprintf(statsFile,"\n\tDetected OIs");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
	fprintf(statsFile," \tTotal");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile,"\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::OI_NUM; ++i) {
			fprintf(statsFile," \t%1lu", oiStats.numDetectedOIsCD4OI[j][i]);
		}
		fprintf(statsFile," \t%1lu", oiStats.numDetectedOIsCD4[j]);
	}
	fprintf(statsFile,"\n\tTotal");
	for (i = 0; i < SimContext::OI_NUM; ++i) {
		fprintf(statsFile," \t%1lu", oiStats.numDetectedOIsOI[i]);
	}

	// Print out prior OI history logging information
	fprintf(statsFile,"\nPRIOR OI HIST PROB AS PROPORTION OF PATIENTS (LOGGED)");
	for (i = 0; i < SimContext::OI_NUM; ++i) {
		fprintf(statsFile,"\n\tOI: %s", SimContext::OI_STRS[i]);
		for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
			fprintf(statsFile,"\t%s", SimContext::CD4_STRATA_STRS[j]);
		fprintf(statsFile,"\tTotal");
		for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k ) {
			fprintf(statsFile,"\n\t%s", SimContext::HVL_STRATA_STRS[k]);
			for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
				if (oiStats.numPatientsHVLCD4[k][j] == 0)
					fprintf(statsFile,"\tN/A");
				else {
					fprintf(statsFile,"\t%1.4lf", oiStats.probPatientsOIHistoryHVLCD4[i][k][j]);
				}  // else
			}  // for j
			if (oiStats.numPatientsHVL[k] == 0)
				fprintf(statsFile,"\tN/A");
			else
				fprintf(statsFile,"\t%1.4lf", oiStats.probPatientsOIHistoryHVL[i][k]);
		}  // for k
		fprintf(statsFile,"\n\tTotal");
		for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
			if (oiStats.numPatientsCD4[j] == 0)
				fprintf(statsFile,"\tN/A");
			else
				fprintf(statsFile,"\t%1.4lf", oiStats.probPatientsOIHistoryCD4[i][j]);
		}  // for j
	}  // for i
	fprintf(statsFile,"\nPRIOR OI HIST PROB BY PATIENT MTHS (LOGGED)");
	for (i = 0; i < SimContext::OI_NUM; ++i) {
		fprintf(statsFile,"\n\tOI: %s", SimContext::OI_STRS[i]);
		for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
			fprintf(statsFile,"\t%s", SimContext::CD4_STRATA_STRS[j]);
		fprintf(statsFile,"\tTotal");
		for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k ) {
			fprintf(statsFile,"\n\t%s", SimContext::HVL_STRATA_STRS[k]);
			for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
				if (oiStats.numMonthsHVLCD4[k][j] == 0)
					fprintf(statsFile,"\tN/A");
				else {
					fprintf(statsFile,"\t%1.4lf", oiStats.probMonthsOIHistoryHVLCD4[i][k][j]);
				}  // else
			}  // for j
			if (oiStats.numMonthsHVL[k] == 0)
				fprintf(statsFile,"\tN/A");
			else
				fprintf(statsFile,"\t%1.4lf", oiStats.probMonthsOIHistoryHVL[i][k]);
		}  // for k
		fprintf(statsFile,"\n\tTotal");
		for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
			if (oiStats.numMonthsCD4[j] == 0)
				fprintf(statsFile,"\tN/A");
			else
				fprintf(statsFile,"\t%1.4lf", oiStats.probMonthsOIHistoryCD4[i][j]);
		}  // for j
	}  // for i
} /* end writeOIStats */

/** \brief writeDeathStats outputs the DeathStats statistics to the stats file */
void RunStats::writeDeathStats() {
	int i, j, k;
	fprintf(statsFile,"\nCAUSES OF DEATH");

	// Print out OI and other causes of death statistics
	fprintf(statsFile,"\n\tCD4 Count Level");
	for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
		fprintf(statsFile," \t%s", SimContext::DTH_CAUSES_STRS[i] );
	fprintf(statsFile," \tTotal");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j) {
		fprintf(statsFile,"\n\t%s", SimContext::CD4_STRATA_STRS[j]);
		for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
			fprintf(statsFile," \t%1lu", deathStats.numDeathsCD4Type[j][i]);
		fprintf(statsFile," \t%1lu", deathStats.numDeathsCD4[j]);
	}
	fprintf(statsFile,"\n\tTotal HIV+ Deaths");
	for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
		fprintf(statsFile," \t%1lu", deathStats.numDeathsHIVPosType[i]);
	fprintf(statsFile," \t%1ld", popSummary.numCohortsHIVPositive);

	fprintf(statsFile,"\n\n\tStatus");
	for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
		fprintf(statsFile," \t%s", SimContext::DTH_CAUSES_STRS[i] );
	fprintf(statsFile," \tTotal");
	for (j = 0; j < SimContext::HIV_CARE_NUM; j++) {
		fprintf(statsFile,"\n\t%s", SimContext::HIV_CARE_STRS[j]);
		for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
			fprintf(statsFile," \t%1lu", deathStats.numDeathsCareType[j][i]);
		fprintf(statsFile," \t%1lu", deathStats.numDeathsCare[j]);
	}
	fprintf(statsFile,"\n\tTotal HIV+ Deaths");
	for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
		fprintf(statsFile," \t%1lu", deathStats.numDeathsHIVPosType[i]);
	fprintf(statsFile," \t%1ld", popSummary.numCohortsHIVPositive);

	fprintf(statsFile,"\n\tTotal Deaths");
	for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
		fprintf(statsFile," \t%1lu", deathStats.numDeathsType[i]);
	fprintf(statsFile," \t%1ld", popSummary.numCohorts);

    // Deaths by age and cause
    fprintf(statsFile,"\n\n\tAge Bracket");
    for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
        fprintf(statsFile," \t%s", SimContext::DTH_CAUSES_STRS[j]);
    for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++){
        fprintf(statsFile,"\n\t%s", SimContext::OUTPUT_AGE_CAT_STRS[k]);
        for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j)
            fprintf(statsFile," \t%1lu", deathStats.numDeathsTypeAge[j][k]);
    }

	// Print out CD4/HVL death distribution
	fprintf(statsFile,"\n\tDeath Distrib");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(statsFile," \t%s", SimContext::CD4_STRATA_STRS[j]);
	fprintf(statsFile," \tTotal");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i) {
		fprintf(statsFile,"\n\t%s", SimContext::HVL_STRATA_STRS[i]);
		for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
			fprintf(statsFile," \t%1lu", deathStats.numDeathsHVLCD4[i][j]);
		fprintf(statsFile," \t%1lu", deathStats.numDeathsHVL[i]);
	}

	// Print HIV and background mortality deaths by CD4 and OI history
	fprintf(statsFile,"\n\tCD4/OIhist");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(statsFile," \t%s", SimContext::CD4_STRATA_STRS[j]);
	fprintf(statsFile," \tTotal");
	fprintf(statsFile," \n\tHIV [woOIHist]");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(statsFile," \t%1lu", deathStats.numHIVDeathsNoOIHistoryCD4[j]);
	fprintf(statsFile," \t%1lu", deathStats.numHIVDeathsNoOIHistory);
	fprintf(statsFile," \n\tHIV [w.OIHist]");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(statsFile," \t%1lu", deathStats.numHIVDeathsOIHistoryCD4[j]);
	fprintf(statsFile," \t%1lu", deathStats.numHIVDeathsOIHistory);
	fprintf(statsFile," \n\tBackground Mortality [woOIHist]");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(statsFile," \t%1lu", deathStats.numBackgroundMortDeathsNoOIHistoryCD4[j]);
	fprintf(statsFile," \t%1lu", deathStats.numBackgroundMortDeathsNoOIHistory);
	fprintf(statsFile," \n\tBackground Mortality [w.OIHist]");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(statsFile," \t%1lu", deathStats.numBackgroundMortDeathsOIHistoryCD4[j]);
	fprintf(statsFile," \t%1lu", deathStats.numBackgroundMortDeathsOIHistory);

	//Print death by tox
	fprintf(statsFile,"\n\tART Tox Deaths");

	for(int i=0;i<SimContext::CD4_NUM_STRATA;i++){
		fprintf(statsFile,"\t%s",SimContext::CD4_STRATA_STRS[i]);
	}

	for(int j=0;j<SimContext::HVL_NUM_STRATA;j++){
		fprintf(statsFile,"\n\t%s",SimContext::HVL_STRATA_STRS[j]);
		for(int i=0;i<SimContext::CD4_NUM_STRATA;i++){
			fprintf(statsFile,"\t%1lu",deathStats.numARTToxDeathsCD4HVL[i][j]);
		}
	}

	//Print death by tox with oi hist
	fprintf(statsFile,"\n\tDeaths From ART Toxicity");

	fprintf(statsFile,"\n\t\tDistribution of CD4 at ART Tox Death");
	if(deathStats.numARTToxDeathsCD4Metric[SimContext::CD4_ABSOLUTE] > 0){
		fprintf(statsFile,"\n\t\tMean\t%1.0lf",deathStats.ARTToxDeathsCD4Mean);
		fprintf(statsFile,"\n\t\tStd Dev\t%1.0lf",deathStats.ARTToxDeathsCD4StdDev);
		if(simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile, "\n\t\tNum w.Absolute CD4 Metric at ART Tox Death\t%lu", deathStats.numARTToxDeathsCD4Metric[SimContext::CD4_ABSOLUTE]);	
		}
		else{
			fprintf(statsFile, "\n");
		}
	}
	else{
		fprintf(statsFile,"\n\t\tMean\tN/A");
		fprintf(statsFile,"\n\t\tStd Dev\tN/A");
	}

	fprintf(statsFile,"\n\t\tDistribution of HVL at ART Tox Death\n\t\t");
	for(int i=0;i<SimContext::CD4_NUM_STRATA;i++){
		fprintf(statsFile,"\t%s",SimContext::CD4_STRATA_STRS[i]);
	}

	for(int j=0;j<SimContext::HVL_NUM_STRATA;j++){
		fprintf(statsFile,"\n\t\t%s",SimContext::HVL_STRATA_STRS[j]);
		for(int i=0;i<SimContext::CD4_NUM_STRATA;i++){
			fprintf(statsFile,"\t%1.4lf",deathStats.hvlDistribToxDeathCD4HVL[i][j]);
		}
	}

	fprintf(statsFile,"\n\n\t\tProbability of OI History at ART Tox Death");
	for (k=0;k<SimContext::OI_NUM;k++){
		fprintf(statsFile,"\n\t\t%s",SimContext::OI_STRS[k]);
		for(int i=0;i<SimContext::CD4_NUM_STRATA;i++){
			fprintf(statsFile,"\t%s",SimContext::CD4_STRATA_STRS[i]);
		}
		for(int j=0;j<SimContext::HVL_NUM_STRATA;j++){
			fprintf(statsFile,"\n\t\t%s",SimContext::HVL_STRATA_STRS[j]);
			for(int i=0;i<SimContext::CD4_NUM_STRATA;i++){
				fprintf(statsFile,"\t%1.4lf",deathStats.probOIHistARTToxDeathsCD4HVL[i][j][k]);
			}
		}
	}
} /* end writeDeathStats */

/** \brief writeOverallSurvival outputs the OverallSurvival statistics to the stats file */
void RunStats::writeOverallSurvival() {
	int i;
    fprintf(statsFile,"\nOVERALL SURVIVAL");

	// output total LMs by CD4 strata (w/ and w/o history of any OI)
    fprintf(statsFile,"\n\tCD4 Strata: ");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i)
		fprintf(statsFile,"\t%s", SimContext::CD4_STRATA_STRS[i]);
    fprintf(statsFile,"\tTotal");
    fprintf(statsFile,"\n\tLife Months [woOIHist]");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
		fprintf(statsFile,"\t%1.0lf", overallSurvival.LMsNoOIHistoryCD4[i]);
	}
	fprintf(statsFile,"\t%1.0lf", overallSurvival.LMsNoOIHistory);
    fprintf(statsFile,"\n\tLife Months [w.OIHist]");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
		fprintf(statsFile,"\t%1.0lf", overallSurvival.LMsOIHistoryCD4[i]);
	}
	fprintf(statsFile,"\t%1.0lf", overallSurvival.LMsOIHistory);
    fprintf(statsFile,"\n\tLife Months [Total]");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
		fprintf(statsFile,"\t%1.0lf", overallSurvival.LMsTotalCD4[i]);
	}
	fprintf(statsFile,"\t%1.0lf", overallSurvival.LMsTotal);

	// output total LMs by curr HVL and HVL setpoint
    fprintf(statsFile,"\n\tHVL Strata: ");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i)
		fprintf(statsFile,"\t%s", SimContext::HVL_STRATA_STRS[i]);
    fprintf(statsFile,"\n\tLife Mths, HVL Setpt ");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i)
	    fprintf(statsFile," \t%1.0lf", overallSurvival.LMsHVLSetpoint[i]);
    fprintf(statsFile,"\n\tLife Mths, Curr HVL ");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i)
	    fprintf(statsFile," \t%1.0lf", overallSurvival.LMsHVL[i]);

	// output total LMs by history or no history of each indiv OI
    fprintf(statsFile,"\n\tOI: ");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile,"\t%s", SimContext::OI_STRS[i]);
    fprintf(statsFile,"\n\tLife Mths, no OI hist");
	for (i = 0; i < SimContext::OI_NUM; ++i)
	    fprintf(statsFile," \t%1.0lf", overallSurvival.LMsNoOIHistoryOIs[i]);
    fprintf(statsFile,"\n\tLife Mths, with OI hist");
	for (i = 0; i < SimContext::OI_NUM; ++i)
	    fprintf(statsFile," \t%1.0lf", overallSurvival.LMsOIHistoryOIs[i]);

	// output total LMs by history of individual chrms
    fprintf(statsFile,"\n\tCHRMs: ");
	for (i = 0; i < SimContext::CHRM_NUM; ++i)
		fprintf(statsFile,"\t%s", SimContext::CHRM_STRS[i]);
    fprintf(statsFile,"\n\tLife Mths, with CHRM hist (HIV+)");
	for (i = 0; i < SimContext::CHRM_NUM; ++i)
	    fprintf(statsFile," \t%1.0lf", overallSurvival.LMsCHRMHistoryCHRMsHIVPos[i]);


	// output LMs by HIV screening states
	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::HIV_ID_NUM; ++i)
		fprintf(statsFile, "\t%s", SimContext::HIV_ID_STRS[i]);
	fprintf(statsFile, "\tHIVpos (All)");
	fprintf(statsFile, "\n\tLMs by HIV State:");
	for (i = 0; i < SimContext::HIV_ID_NUM; ++i)
		fprintf(statsFile, "\t%1.0lf", overallSurvival.LMsHIVState[i]);
	fprintf(statsFile, "\t%1.0lf", overallSurvival.LMsHIVPositive);

	// output other misc LM stats
	fprintf(statsFile, "\n\t\tLMs\tQALMs");
	fprintf(statsFile, "\n\tLMs in HIV Scr Module:\t%1.0lf\t%1.0lf",
		overallSurvival.LMsInScreening, overallSurvival.QALMsInScreening);
	fprintf(statsFile, "\t\tLMs in \"Reg CEPAC\":\t%1.0lf", overallSurvival.LMsInRegularCEPAC);
	fprintf(statsFile, "\n\tLMs on PrEP (HIV-):\t%1.0lf", overallSurvival.LMsHIVNegativeOnPrEP);

	// output survival by gender
	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::GENDER_NUM; i++)
		fprintf(statsFile, "\t%s", SimContext::GENDER_STRS[i]);
	fprintf(statsFile, "\n\tLMs Gender");
	for (i = 0; i < SimContext::GENDER_NUM; i++)
		fprintf(statsFile, "\t%1.0lf", overallSurvival.LMsGender[i]);
	fprintf(statsFile, "\n\tQALMs Gender");
	for (i = 0; i < SimContext::GENDER_NUM; i++)
		fprintf(statsFile, "\t%1.0lf", overallSurvival.QALMsGender[i]);
} /* end writeOverallSurvival */

/** \brief writeOverallCosts outputs the OverallCosts statistics to the stats file */
void RunStats::writeOverallCosts() {
	int i, j;
    fprintf(statsFile,"\nOVERALL COSTS");

	//Print out costs for multiple discount factors
	if (simContext->getRunSpecsInputs()->enableMultipleDiscountRates){
		fprintf(statsFile,"\n\t\tDiscount Rate\tCost");
		for (i = 0; i < simContext->NUM_DISCOUNT_RATES; i++){
			double annualRateCost = pow(simContext->getRunSpecsInputs()->multDiscountRatesCost[i],12.0)-1;
			fprintf(statsFile,"\n\tOverall Discounted Cost\t%1.2lf\t%1.0lf",
				annualRateCost, popSummary.multDiscCostsSum[i]);
		}
	}
	else{
		fprintf(statsFile, "\n\tOverall Discounted Cost\t%1.0lf", overallCosts.costsTotal);
	}


	// output total costs by CD4 strata (w/ and w/o history of any OI)
    fprintf(statsFile,"\n\tCD4 Strata: ");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i)
		fprintf(statsFile,"\t%s", SimContext::CD4_STRATA_STRS[i]);
    fprintf(statsFile,"\tTotal");
    fprintf(statsFile,"\n\tCosts [woOIHist]");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
	    fprintf(statsFile," \t%1.0lf", overallCosts.costsNoOIHistoryCD4[i]);
	}
	fprintf(statsFile,"\t%1.0lf", overallCosts.costsNoOIHistory);
    fprintf(statsFile,"\n\tCosts [w.OIHist]");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
		fprintf(statsFile," \t%1.0lf", overallCosts.costsOIHistoryCD4[i]);
	}
	fprintf(statsFile,"\t%1.0lf", overallCosts.costsOIHistory);
    fprintf(statsFile,"\n\tCosts [Total]");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
	    fprintf(statsFile," \t%1.0lf", overallCosts.costsTotalCD4[i]);
	}
	fprintf(statsFile,"\t%1.0lf", overallCosts.costsTotal);

	//Chrms costs
	fprintf(statsFile, "\n\t");
	for(int i=0;i<SimContext::CHRM_NUM;i++){
		fprintf(statsFile,"\t%s", SimContext::CHRM_STRS[i]);
	}
	fprintf(statsFile,"\n\tCHRMs Costs:");
	for(int i=0;i<SimContext::CHRM_NUM;i++){
		fprintf(statsFile,"\t%1.0lf",overallCosts.costsCHRMs[i]);
	}

	// output costs by HVL and HVL setpoint
    fprintf(statsFile,"\n\tHVL Strata: ");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i)
		fprintf(statsFile,"\t%s", SimContext::HVL_STRATA_STRS[i]);
    fprintf(statsFile,"\n\tCosts, HVL Setpt ");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i)
	    fprintf(statsFile," \t%1.0lf", overallCosts.costsHVLSetpoint[i]);
    fprintf(statsFile,"\n\tCosts, Curr HVL ");
	for (i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i)
	    fprintf(statsFile," \t%1.0lf", overallCosts.costsHVL[i]);

	// output proph and art costs
    fprintf(statsFile,"\n\tDirect Proph Costs");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
    fprintf(statsFile," \tAll");
	for (j = 0; j < SimContext::PROPH_NUM; ++j) {
	    fprintf(statsFile,"\n\tProph %d", j + 1);
		for (i = 0; i < SimContext::OI_NUM; ++i)
		    fprintf(statsFile," \t%1.0lf", overallCosts.directCostsProphOIsProph[i][j]);
	}
    fprintf(statsFile,"\n\tTotal Proph Costs");
	for (i = 0; i < SimContext::OI_NUM; ++i) {
	    fprintf(statsFile," \t%1.0lf", overallCosts.directCostsProphOIs[i]);
	}
    fprintf(statsFile," \t%1.0lf\n\t", overallCosts.directCostsProph);


	if (simContext->getRunSpecsInputs()->enableMultipleDiscountRates){
		fprintf(statsFile, " \tDiscount Factor");
		for (i = 0; i < SimContext::ART_NUM_LINES; ++i)
		    fprintf(statsFile," \tART %d", i+1);
		fprintf(statsFile,"\tTotal");
		for (j = 0; j < simContext->NUM_DISCOUNT_RATES; j++){
		    fprintf(statsFile,"\n\tDirect ART Costs:");
		    fprintf(statsFile,"\t%1.2lf", pow(simContext->getRunSpecsInputs()->multDiscountRatesCost[j],12.0)-1);
			for (i = 0; i < SimContext::ART_NUM_LINES; ++i) {
			    fprintf(statsFile," \t%1.0lf", overallCosts.directCostsARTLineMultDisc[i][j]);
			}
			fprintf(statsFile,"\t%1.0lf", overallCosts.directCostsARTMultDisc[j]);
		}

		for (j = 0; j < simContext->NUM_DISCOUNT_RATES; j++){
		    fprintf(statsFile,"\n\tInit ART Costs:");
		    fprintf(statsFile,"\t%1.2lf", pow(simContext->getRunSpecsInputs()->multDiscountRatesCost[j],12.0)-1);
			for (i = 0; i < SimContext::ART_NUM_LINES; ++i) {
			    fprintf(statsFile," \t%1.0lf", overallCosts.costsARTInitLineMultDisc[i][j]);
			}
			fprintf(statsFile,"\t%1.0lf", overallCosts.costsARTInitMultDisc[j]);
		}

		for (j = 0; j < simContext->NUM_DISCOUNT_RATES; j++){
		    fprintf(statsFile,"\n\tMonthly ART Costs:");
		    fprintf(statsFile,"\t%1.2lf", pow(simContext->getRunSpecsInputs()->multDiscountRatesCost[j],12.0)-1);
			for (i = 0; i < SimContext::ART_NUM_LINES; ++i) {
			    fprintf(statsFile," \t%1.0lf", overallCosts.costsARTMonthlyLineMultDisc[i][j]);
			}
			fprintf(statsFile,"\t%1.0lf", overallCosts.costsARTMonthlyMultDisc[j]);
		}
	}
	else{
		for (i = 0; i < SimContext::ART_NUM_LINES; ++i)
		    fprintf(statsFile," \tART %d", i+1);
		fprintf(statsFile,"\tTotal");
	    fprintf(statsFile,"\n\tDirect ART Costs:");
		for (i = 0; i < SimContext::ART_NUM_LINES; ++i) {
		    fprintf(statsFile," \t%1.0lf", overallCosts.directCostsARTLine[i]);
		}
	    fprintf(statsFile,"\t%1.0lf", overallCosts.directCostsART);

	    fprintf(statsFile,"\n\tInit ART Costs:");
		for (i = 0; i < SimContext::ART_NUM_LINES; ++i) {
		    fprintf(statsFile," \t%1.0lf", overallCosts.costsARTInitLine[i]);
		}
	    fprintf(statsFile,"\t%1.0lf", overallCosts.costsARTInit);

	    fprintf(statsFile,"\n\tMonthly ART Costs:");
		for (i = 0; i < SimContext::ART_NUM_LINES; ++i) {
		    fprintf(statsFile," \t%1.0lf", overallCosts.costsARTMonthlyLine[i]);
		}
	    fprintf(statsFile,"\t%1.0lf", overallCosts.costsARTMonthly);

	}

	// Other misc costs
	if (simContext->getRunSpecsInputs()->enableMultipleDiscountRates){
		fprintf(statsFile,"\n\t\tDiscountFactor\tCD4 Tests \tHVL Tests");
		for (j = 0; j < simContext->NUM_DISCOUNT_RATES; j++){
		    fprintf(statsFile,"\n\tTesting Costs:\t%1.2lf\t%1.0lf \t%1.0lf",
		    		pow(simContext->getRunSpecsInputs()->multDiscountRatesCost[j],12.0)-1,overallCosts.costsCD4TestingMultDisc[j], overallCosts.costsHVLTestingMultDisc[j]);
		}
	}
	else{
		fprintf(statsFile,"\n\t\tCD4 Tests \tHVL Tests");
	    fprintf(statsFile,"\n\tTesting Costs: \t%1.0lf \t%1.0lf",
			overallCosts.costsCD4Testing, overallCosts.costsHVLTesting);
	}

	fprintf(statsFile,"\n\tClinic Visit Costs: \t%1.0lf", overallCosts.costsClinicVisits);
	fprintf(statsFile, "\n\t\tTests\tMisc");
	fprintf(statsFile, "\n\tHIV Screening Costs:\t%1.0lf\t%1.0lf",
		overallCosts.costsHIVScreeningTests, overallCosts.costsHIVScreeningMisc);

	fprintf(statsFile, "\n\tLab Staging Costs:\t%1.0lf\t%1.0lf",
		overallCosts.costsLabStagingTests, overallCosts.costsLabStagingMisc);	

	fprintf(statsFile, "\n\n\t\tNever HIV\tOn PrEP When Infected\tPrEP Dropout When Infected\tTotal");
	fprintf(statsFile, "\n\tPrEP Costs:\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf", overallCosts.costsPrEPNeverHIV,
			overallCosts.costsPrEPHIVPos[SimContext::HIV_POS_ON_PREP], overallCosts.costsPrEPHIVPos[SimContext::HIV_POS_PREP_DROPOUT], overallCosts.costsPrEP);
	fprintf(statsFile, "\n\t\tStartup\tMonthly");
	fprintf(statsFile, "\n\tIntervention Costs:\t%1.0lf\t%1.0lf",
		overallCosts.costsInterventionStartup, overallCosts.costsInterventionMonthly);

	if (simContext->getPedsInputs()->enablePediatricsModel && simContext->getEIDInputs()->enableHIVTestingEID){
        fprintf(statsFile, "\n\t\tTests\tMisc");
		fprintf(statsFile, "\n\tEID Screening Costs:\t%1.0lf\t%1.0lf",
				overallCosts.costsEIDTests, overallCosts.costsEIDMisc);
        fprintf(statsFile, "\n\t\tTotal");
        fprintf(statsFile, "\n\tInfant HIV Proph Costs:\t%1.0f", overallCosts.costsInfantHIVProph);
	}
	fprintf(statsFile, "\n\t\tDirectMedical\tDirectNonMedical\tTimeCosts\tIndirect\tUnclassified\tDrugCosts\tToxicity");
	fprintf(statsFile, "\n\tTotal Undiscounted Costs:\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf",
		overallCosts.totalUndiscountedCosts[0], overallCosts.totalUndiscountedCosts[1],
		overallCosts.totalUndiscountedCosts[2], overallCosts.totalUndiscountedCosts[3],
		overallCosts.totalUndiscountedCostsUnclassified, overallCosts.costsDrugs, overallCosts.costsToxicity);

	fprintf(statsFile, "\n\tTotal Discounted Costs:\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf",
		overallCosts.totalDiscountedCosts[0], overallCosts.totalDiscountedCosts[1],
		overallCosts.totalDiscountedCosts[2], overallCosts.totalDiscountedCosts[3],
		overallCosts.totalDiscountedCostsUnclassified, overallCosts.costsDrugsDiscounted, overallCosts.costsToxicityDiscounted);


	// output costs by gender
	fprintf(statsFile, "\n\t");
	for (i = 0; i < SimContext::GENDER_NUM; i++)
		fprintf(statsFile, "\t%s", SimContext::GENDER_STRS[i]);
	fprintf(statsFile, "\n\tCosts Gender");
	for (i = 0; i < SimContext::GENDER_NUM; i++)
		fprintf(statsFile, "\t%1.0lf", overallCosts.costsGender[i]);

    // TB Costs

    /** Total TB Cost*/
	fprintf(statsFile, "\n\tTotal TB Costs");
	fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTotal);

	/** Visit Costs */
	fprintf(statsFile, "\n\t");
    for (i = 0; i < SimContext::COST_NUM_TYPES; i++)
        fprintf(statsFile, "\t%s", SimContext::COST_TYPES_STRS[i]);
    fprintf(statsFile, "\n\tTB Provider Visit Costs");
    for (i = 0; i < SimContext::COST_NUM_TYPES; i++)
        fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBProviderVisits[i]);
    fprintf(statsFile, "\n\tTB Med Pick-Up Costs");
    for (i = 0; i < SimContext::COST_NUM_TYPES; i++)
        fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBMedicationVisits[i]);

    /** Diagnostic Testing Costs */
    fprintf(statsFile, "\n\tTB Diagnostic Number");
    for (i = 0; i < SimContext::TB_NUM_TESTS; i++)
        fprintf(statsFile, "\t%d", i);
    fprintf(statsFile, "\n\tInitial Costs");
    for (i = 0; i < SimContext::TB_NUM_TESTS; i++)
        fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTestsInit[i]);
    fprintf(statsFile, "\n\tDST Costs");
    for (i = 0; i < SimContext::TB_NUM_TESTS; i++)
        fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTestsDST[i]);
    fprintf(statsFile, "\n\tTotal TB Diagnostic Costs");
	fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTests);

    /** Treatment Costs */
    fprintf(statsFile, "\n\tTB Treatment Number");
    for (i = 0; i < SimContext::TB_NUM_TREATMENTS; i++)
        fprintf(statsFile, "\t%d", i);
    fprintf(statsFile, "\n\tDirect Costs");
    for (i = 0; i < SimContext::TB_NUM_TREATMENTS; i++)
        fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTreatmentByLine[i]);
    fprintf(statsFile, "\n\tToxicity Costs");
    for (i = 0; i < SimContext::TB_NUM_TREATMENTS; i++)
        fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTreatmentToxByLine[i]);
    fprintf(statsFile, "\n\tTB Treatment Total Direct Costs");
	fprintf(statsFile, "\t%1.0lf", overallCosts.costsTBTreatment);
} /* end writeOverallCosts */

/** \brief writeTBStats outputs the TBStats statistics to the stats file */
void RunStats::writeTBStats() {
	int i, j;
	fprintf(statsFile, "\nTB SUMMARY EVENTS");

	fprintf(statsFile, "\n\tTB State on Entry:");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[i]);
	fprintf(statsFile, "\n\tUninfected\t%lu", tbStats.numUninfectedTBAtEntry);
	for ( j = 1; j < SimContext::TB_NUM_STATES; ++j ){
		fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[j]);
		for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
			fprintf(statsFile, "\t%lu", tbStats.numInStateAtEntryStrain[i][j]);
	}
	fprintf(statsFile, "\n\tTB Care Outcomes");
	fprintf(statsFile, "\n\tStart on TB Treatment:");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%d", j);
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i ) {
		fprintf(statsFile, "\n\tObsv %s", SimContext::TB_STRAIN_STRS[i]);
		for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
			fprintf(statsFile, "\t%lu", tbStats.numStartOnTreatment[i][j]);
	}
	fprintf(statsFile, "\n\tDropout TB Treatment:");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%d", j);
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i ) {
		fprintf(statsFile, "\n\tObsv %s", SimContext::TB_STRAIN_STRS[i]);
		for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
			fprintf(statsFile, "\t%lu", tbStats.numDropoutTreatment[i][j]);
	}
	fprintf(statsFile, "\n\tTransitions to TB Treatment Default after TB LTFU:");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%d", j);
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i ) {
		fprintf(statsFile, "\n\tObsv %s", SimContext::TB_STRAIN_STRS[i]);
		for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
			fprintf(statsFile, "\t%lu", tbStats.numTransitionsToTBTreatmentDefault[i][j]);
	}
	fprintf(statsFile, "\n\tComplete TB Treatment:");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%d", j);
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i ) {
		fprintf(statsFile, "\n\tObsv %s", SimContext::TB_STRAIN_STRS[i]);
		for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
			fprintf(statsFile, "\t%lu", tbStats.numFinishTreatment[i][j]);
	}
	fprintf(statsFile, "\n\tTB Treatment Efficacy Outcomes (All TB Care States)");
	fprintf(statsFile, "\n\tTB Cure at Treatment Completion:");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%d", j);
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i ) {
		fprintf(statsFile, "\n\t%s", SimContext::TB_STRAIN_STRS[i]);
		for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
			fprintf(statsFile, "\t%lu", tbStats.numCuredAtTreatmentFinish[i][j]);
	}
	fprintf(statsFile, "\n\tTB Increase Resistance at Treatment Stop:");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%d", j);
	fprintf(statsFile, "\n\tdsTB to mdrTB");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%lu", tbStats.numIncreaseResistanceAtTreatmentStop[SimContext::TB_STRAIN_DS][j]);
	fprintf(statsFile, "\n\tmdrTB to xdrTB");
	for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
		fprintf(statsFile, "\t%lu", tbStats.numIncreaseResistanceAtTreatmentStop[SimContext::TB_STRAIN_MDR][j]);
	fprintf(statsFile, "\n\n\tTotal TB Incident Infections/Reinfections");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[i]);
	for(i = 0; i < SimContext::TB_NUM_STATES; ++i ){
		fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[i]);
		for ( j = 0; j < SimContext::TB_NUM_STRAINS; j++)
			fprintf(statsFile, "\t%lu", tbStats.numInfections[i][j]);
	}

	fprintf(statsFile, "\n\t");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[i]);

	fprintf(statsFile, "\n\tTotal Activations (Pulm) from Latent TB");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numReactivationsPulmLatent[i]);
	fprintf(statsFile, "\n\tTotal Activations (Extrapulm) from Latent TB");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numReactivationsExtraPulmLatent[i]);
	fprintf(statsFile, "\n\tTotal Activations (All) from Latent TB");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numReactivationsLatent[i]);

	fprintf(statsFile, "\n\tTotal Relapses (Pulm)");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numRelapsesPulm[i]);
	fprintf(statsFile, "\n\tTotal Relapses (Extrapulm)");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numRelapsesExtraPulm[i]);
	fprintf(statsFile, "\n\tTotal Relapses (All)");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numRelapses[i]);
	fprintf(statsFile, "\n\tTotal TB Self Cures");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numTBSelfCures[i]);
	
	fprintf(statsFile, "\n\tTB Deaths");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[i]);
	fprintf(statsFile, "\n\tHIV Negative Deaths");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numDeathsHIVNeg[i]);	
	fprintf(statsFile, "\n\tHIV Positive Deaths");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numDeathsHIVPos[i]);	
	fprintf(statsFile, "\n\tTotal Deaths from TB");
	for ( i = 0; i < SimContext::TB_NUM_STRAINS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numDeaths[i]);
	fprintf(statsFile, "\n\t");
	
	for ( i = 0; i < SimContext::TB_NUM_TREATMENTS; ++i )
		fprintf(statsFile, "\t%d", i);
	fprintf(statsFile, "\n\tTotal TB Treatment Minor Toxicity");
	for ( i = 0; i < SimContext::TB_NUM_TREATMENTS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numTreatmentMinorToxicity[i]);
	fprintf(statsFile, "\n\tTotal TB Treatment Major Toxicity");
	for ( i = 0; i < SimContext::TB_NUM_TREATMENTS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numTreatmentMajorToxicity[i]);
	fprintf(statsFile,"\n\t");
	for ( i = 0; i < SimContext::TB_NUM_PROPHS; ++i )
		fprintf(statsFile, "\tProph%d", i + 1);
	fprintf(statsFile, "\n\tTotal TB Proph Minor Toxicity");
	for ( i = 0; i < SimContext::TB_NUM_PROPHS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numProphMinorToxicity[i]);
	fprintf(statsFile, "\n\tTotal TB Proph Major Toxicity");
	for ( i = 0; i < SimContext::TB_NUM_PROPHS; ++i )
		fprintf(statsFile, "\t%lu", tbStats.numProphMajorToxicity[i]);
	fprintf(statsFile, "\n\tNumber with Unfavorable Outcome");
	fprintf(statsFile, "\n");
	for ( i = 0; i < SimContext::TB_NUM_UNFAVORABLE; i++)
		fprintf(statsFile,"\t%s",simContext->TB_UNFAVORABLE_STRS[i]);
	for( i = 0; i < 2; i++){
		for( j = 0; j < 2; j++){
			for(int k = 0; k < 2; k++){
				for(int l = 0; l < 2; l++){
					fprintf(statsFile, "\n\t%s\t%s\t%s\t%s",i?"yes":"no",j?"yes":"no",k?"yes":"no",l?"yes":"no");
					fprintf(statsFile, "\t%lu",tbStats.numWithUnfavorableOutcome[i][j][k][l]);
				}
			}
		}
	}			

	fprintf(statsFile,"\n\tDST Test Result");
	fprintf(statsFile,"\n\tTrue Strain");
	for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
		fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[j]);
	fprintf(statsFile, "\tTB Uninfected");
	for (int k = 0; k < SimContext::TB_NUM_STRAINS; k++){
		fprintf(statsFile, "\n\tObserved %s", SimContext::TB_STRAIN_STRS[k]);
		for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
			fprintf(statsFile, "\t%lu", tbStats.numDSTTestResultsByTrueTBStrain[k][j]);
		fprintf(statsFile, "\t%lu", tbStats.numDSTTestResultsUninfectedTB[k]);
	}
} /* end writeTBStats */

/** \brief writeLTFUStats outputs the LTFUStats statistics to the stats file */
void RunStats::writeLTFUStats() {
	int i, j;

	//print out LTFU/RTC stats
	fprintf(statsFile, "\nLOST TO FOLLOW UP AND RETURN TO CARE STATS\n");
	fprintf(statsFile, "\tTotal persons experiencing LTFU:\t%d\n", ltfuStats.numPatientsLost);
	fprintf(statsFile, "\tTotal persons experiencing RTC:\t%d\n", ltfuStats.numPatientsReturned);
	fprintf(statsFile, "\tTotal deaths while LTFU:\t%d\n\t", ltfuStats.numDeathsWhileLost);

	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i)
		fprintf(statsFile,"\t%s", SimContext::CD4_STRATA_STRS[i]);
    fprintf(statsFile,"\tTotal");
    fprintf(statsFile,"\n\tTotal LTFU by CD4");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
	    fprintf(statsFile," \t%d", ltfuStats.numLostToFollowUpCD4[i]);
	}
	fprintf(statsFile,"\t%d", ltfuStats.numLostToFollowUp);
    fprintf(statsFile,"\n\tTotal RTC by CD4");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
	    fprintf(statsFile," \t%d", ltfuStats.numReturnToCareCD4[i]);
	}
	fprintf(statsFile,"\t%d", ltfuStats.numReturnToCare);
    fprintf(statsFile,"\n\tTotal Deaths while Lost by CD4");
	for (i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i) {
	    fprintf(statsFile," \t%d", ltfuStats.numDeathsWhileLostCD4[i]);
	}
	fprintf(statsFile,"\t%d\n\t", ltfuStats.numDeathsWhileLost);

	fprintf(statsFile,"\tMean \tStdDev \n\t");
	fprintf(statsFile,"Months Lost Before Returning:\t%1.2lf\t%1.2lf\n\t",
			ltfuStats.monthsLostBeforeReturnMean, ltfuStats.monthsLostBeforeReturnStdDev);

	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j )
		fprintf(statsFile, "\tART%d", j+1);
	fprintf(statsFile, "\tPre-ART\tPost-ART(Off ART)\n");
	fprintf(statsFile, "\tTotal LTFU by ART:");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j )
		fprintf(statsFile, "\t%d", ltfuStats.numLostToFollowUpART[j]);
	fprintf(statsFile, "\t%d", ltfuStats.numLostToFollowUpPreART);
	fprintf(statsFile, "\t%d", ltfuStats.numLostToFollowUpPostART);
	fprintf(statsFile, "\n\tTotal RTC by ART continuing on previous ART:");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j )
		fprintf(statsFile, "\t%d", ltfuStats.numReturnOnPrevART[j]);
	fprintf(statsFile, "\t%d", ltfuStats.numReturnToCarePreART);
	fprintf(statsFile, "\t%d", ltfuStats.numReturnToCarePostART);
	fprintf(statsFile, "\n\tTotal RTC by ART switching to next ART:");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j )
		fprintf(statsFile, "\t%d", ltfuStats.numReturnOnNextART[j]);
	fprintf(statsFile, "\n\tTotal Death while Lost by ART:");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j )
		fprintf(statsFile, "\t%d", ltfuStats.numDeathsWhileLostART[j]);
	fprintf(statsFile, "\t%d", ltfuStats.numDeathsWhileLostPreART);
	fprintf(statsFile, "\t%d", ltfuStats.numDeathsWhileLostPostART);
} /* end writeLTFUStats */

/** \brief writeProphStats outputs the ProphStats statistics to the stats file */
void RunStats::writeProphStats() {
	int i,j;

	// output proph toxicities stats
	fprintf(statsFile,"\nOI PROPH TOXICITY EVENTS");
	fprintf(statsFile,"\n\tMinor Tox Events");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
	for (i = 0; i < SimContext::PROPH_NUM; ++i) {
		fprintf(statsFile,"\n\tProph %d", i+1);
		for (j = 0; j < SimContext::OI_NUM; ++j)
			fprintf(statsFile," \t%1lu", prophStats.numMinorToxicity[j][i]);
	}
	fprintf(statsFile,"\n\tTotal");
	for (j = 0; j < SimContext::OI_NUM; ++j)
		fprintf(statsFile," \t%1lu", prophStats.numMinorToxicityTotal[j]);
	fprintf(statsFile,"\n\tMajor Tox Events");
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(statsFile," \t%s", SimContext::OI_STRS[i]);
	for (i = 0; i < SimContext::PROPH_NUM; ++i) {
		fprintf(statsFile,"\n\tProph %d", i+1);
		for (j = 0; j < SimContext::OI_NUM; ++j)
			fprintf(statsFile," \t%1lu", prophStats.numMajorToxicity[j][i]);
	}
	fprintf(statsFile,"\n\tTotal");
	for (j = 0; j < SimContext::OI_NUM; ++j)
		fprintf(statsFile," \t%1lu", prophStats.numMajorToxicityTotal[j]);

	// print out all primary proph stats
	fprintf(statsFile, "\nPRIMARY OI PROPH MEAN CD4 AT INIT\n\t");
	for ( i = 0; i < SimContext::PROPH_NUM; ++i )
		fprintf(statsFile, "\tProph%d True\tObsv CD4\tTimes Init'd w.Absolute CD4 Metric\tTotal Times Init'd", i+1, i+1);
	for ( j = 0; j < SimContext::OI_NUM; ++j ) {
		fprintf(statsFile, "\n\t%s", SimContext::OI_STRS[j]);
		for ( i = 0; i < SimContext::PROPH_NUM; ++i ) {
			fprintf(statsFile, "\t%1.0lf \t%1.0lf \t%d \t%d",
				prophStats.trueCD4InitProphMean[SimContext::PROPH_PRIMARY][j][i],
				prophStats.observedCD4InitProphMean[SimContext::PROPH_PRIMARY][j][i], prophStats.numTimesInitProphCD4Metric[SimContext::PROPH_PRIMARY][j][i][SimContext::CD4_ABSOLUTE],
				prophStats.numTimesInitProph[SimContext::PROPH_PRIMARY][j][i]);
		}
	}

	// print out all secondary proph stats
	fprintf(statsFile, "\nSECONDARY OI PROPH MEAN CD4 AT INIT\n\t");
	for ( i = 0; i < SimContext::PROPH_NUM; ++i )
		fprintf(statsFile, "\tProph%d True\tObsv CD4\tTimes Init'd w.Absolute CD4 Metric\tTotal Times Init'd", i+1, i+1);
	for ( j = 0; j < SimContext::OI_NUM; ++j ) {
		fprintf(statsFile, "\n\t%s", SimContext::OI_STRS[j]);
		for ( i = 0; i < SimContext::PROPH_NUM; ++i ) {
			fprintf(statsFile, "\t%1.0lf \t%1.0lf \t%d \t%d",
				prophStats.trueCD4InitProphMean[SimContext::PROPH_SECONDARY][j][i],
				prophStats.observedCD4InitProphMean[SimContext::PROPH_SECONDARY][j][i],prophStats.numTimesInitProphCD4Metric[SimContext::PROPH_SECONDARY][j][i][SimContext::CD4_ABSOLUTE],
				prophStats.numTimesInitProph[SimContext::PROPH_SECONDARY][j][i]);
		}
	}
} /* end ProphStats */

/** \brief writeARTStats outputs the ARTStats statistics to the stats file */
void RunStats::writeARTStats() {
	int i, j, k;
	const SimContext::RunSpecsInputs *runSpecs = simContext->getRunSpecsInputs();

	// print out months on suppressed or failed ART
	fprintf(statsFile, "\nMONTHS IN SUPPRESSED/FAILED STATES ON ART\n\t");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j )
		fprintf(statsFile, "\tART%d", j+1);
	fprintf(statsFile, "\tTotal");
	fprintf(statsFile, "\nMonths suppressed\t");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j ) {
		fprintf(statsFile, "\t%1lu", artStats.monthsSuppressedLine[j]);
	}
	fprintf(statsFile, "\t%1lu", artStats.monthsSuppressed);

	fprintf(statsFile, "\nMonths failed");
	for ( i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i ) {
		fprintf(statsFile, "\n\t%s", SimContext::HVL_STRATA_STRS[i]);
		for ( j = 0; j < SimContext::ART_NUM_LINES; ++j ) {
			fprintf(statsFile, "\t%1lu", artStats.monthsFailedLineHVL[j][i]);
		}
		fprintf(statsFile, "\t%1lu", artStats.monthsFailedHVL[i]);
	}
	fprintf(statsFile, "\n\tTotal");
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j ) {
		fprintf(statsFile, "\t%1lu", artStats.monthsFailedLine[j]);
	}

	// print out all ART stats
	for ( j = 0; j < SimContext::ART_NUM_LINES; ++j ) {
		fprintf(statsFile, "\nART %d STATS", j+1);
		// print out ART initiation stats
		fprintf(statsFile, "\n\tAt Init(First time): \t# Total \tAvgCD4Perc (Peds <5) \tAvgTrueCD4 \tAvgObsvCD4 \tAvgPropensity \tNum w.Perc CD4 Metric \tNum w.Absolute CD4 Metric ");
		fprintf(statsFile, "\n\tAny Response: \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1.2lf \t%1lu \t%1lu ",
			artStats.numARTEverInit[j], artStats.trueCD4AtARTEverInitMean[j][SimContext::CD4_PERC], artStats.trueCD4AtARTEverInitMean[j][SimContext::CD4_ABSOLUTE], artStats.observedCD4AtARTEverInitMean[j], artStats.propensityAtARTEverInitMean[j], artStats.numARTEverInitCD4Metric[j][SimContext::CD4_PERC], artStats.numARTEverInitCD4Metric[j][SimContext::CD4_ABSOLUTE]);
		fprintf(statsFile, "\n\tAt Init(Any): \t# Total \tAvgCD4Perc (Peds <5) \tAvgTrueCD4 \tAvgObsvCD4 \tNum w.Perc CD4 Metric \tNum w.Absolute CD4 Metric \t#Drawn Supp \t#Drawn Fail");
		for (k = 0; k < SimContext::CD4_RESPONSE_NUM_TYPES; k++) {
			fprintf(statsFile, "\tCD4 Type %1lu", k + 1);
		}
		for (k = 0; k < SimContext::RISK_FACT_NUM; k++) {
			fprintf(statsFile, "\t%s", SimContext::RISK_FACT_STRS[k]);
		}
		fprintf(statsFile, "\n\tAny response: \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1lu \t%1lu ",
			artStats.numOnARTAtInit[j], artStats.trueCD4AtInitMean[j][SimContext::CD4_PERC], artStats.trueCD4AtInitMean[j][SimContext::CD4_ABSOLUTE], artStats.observedCD4AtInitMean[j], artStats.numOnARTAtInitCD4Metric[j][SimContext::CD4_PERC], artStats.numOnARTAtInitCD4Metric[j][SimContext::CD4_ABSOLUTE]);
		for (k = 0; k < SimContext::ART_EFF_NUM_TYPES; k++) {
			fprintf(statsFile, "\t%1lu", artStats.numDrawEfficacyAtInit[j][k]);
		}
		for (k = 0; k < SimContext::CD4_RESPONSE_NUM_TYPES; k++) {
			fprintf(statsFile, "\t%1lu", artStats.numCD4ResponseTypeAtInit[j][k]);
		}
		for (k = 0; k < SimContext::RISK_FACT_NUM; k++) {
			fprintf(statsFile, "\t%1lu", artStats.numWithRiskFactorAtInit[j][k]);
		}
		/**
		 * outputs for heterogeneity resp types need to fix before outputting (stratify by het_num_outcomes, accommodate for Peds Perc CD4 metric and check mean observed CD4 denominators)
		 *
		for (i = 0; i < SimContext::RESP_NUM_TYPES; i++) {
			fprintf(statsFile, "\n\t%s \t%1lu \t%1.2lf \t%1.2lf", SimContext::RESP_TYPE_STRS[i],
				artStats.numOnARTAtInitResp[j][i], artStats.trueCD4AtInitMeanResp[j][i], artStats.observedCD4AtInitMeanResp[j][i]);
			for (k = 0; k < SimContext::ART_EFF_NUM_TYPES; k++) {
				fprintf(statsFile, "\t%1lu", artStats.numDrawEfficacyAtInitResp[j][k][i]);
			}
			for (k = 0; k < SimContext::CD4_RESPONSE_NUM_TYPES; k++) {
				fprintf(statsFile, "\t%1lu", artStats.numCD4ResponseTypeAtInitResp[j][k][i]);
			}
			for (k = 0; k < SimContext::RISK_FACT_NUM; k++) {
				fprintf(statsFile, "\t%1lu", artStats.numWithRiskFactorAtInitResp[j][k][i]);
			}
		}
		**/
		// print out ART true failure stats
		fprintf(statsFile, "\n\tAt ART true fail: \t# Total \tAvgCD4Perc (Peds <5) \tAvgTrueCD4 \tAvgObsvCD4 \tNum w.Perc CD4 Metric \tNum w.Absolute CD4 Metric \tMthsToFail(Mean) \tMthsToFail(StdDev) ");
		fprintf(statsFile, "\n\tAny response: \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1lu \t%1lu \t%1.2lf \t%1.2lf ",
			artStats.numTrueFailure[j], artStats.trueCD4AtTrueFailureMean[j][SimContext::CD4_PERC],  artStats.trueCD4AtTrueFailureMean[j][SimContext::CD4_ABSOLUTE], 
			artStats.observedCD4AtTrueFailureMean[j], artStats.numTrueFailureCD4Metric[j][SimContext::CD4_PERC], artStats.numTrueFailureCD4Metric[j][SimContext::CD4_ABSOLUTE], artStats.monthsToTrueFailureMean[j],	artStats.monthsToTrueFailureStdDev[j] );
		// het response outputs need some work, including for Peds CD4 metrics and mean observed CD4 denominators, not sure they will be kept
		/**
		for (i = 0; i < SimContext::RESP_NUM_TYPES; i++) {
			fprintf(statsFile, "\n\t%s \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1.2lf", SimContext::RESP_TYPE_STRS[i],
				artStats.numTrueFailureResp[j][i], artStats.trueCD4AtTrueFailureMeanResp[j][i],
				artStats.observedCD4AtTrueFailureMeanResp[j][i], artStats.monthsToTrueFailureMeanResp[j][i],
				artStats.monthsToTrueFailureStdDevResp[j][i]);
		}
		**/

		// print ART observed failure statistics
		fprintf(statsFile, "\n\tAt ART obsv fail: \t# Total \t# with true fail \tAvgCD4Perc (Peds <5) \tAvgTrueCD4 \tAvgObsvCD4 \tNum w.Perc CD4 Metric \tNum w.Absolute CD4 Metric \tMthsToObsvFail(Mean) \tMthsToObsvFail(StdDev) ");
		fprintf(statsFile, "\n\tAny fail diagnosis \t%1lu \t%1lu \t%1.2lf  \t%1.2lf \t%1.2lf \t%1lu \t%1lu \t%1.2lf \t%1.2lf ",
			artStats.numObservedFailure[j], artStats.numObservedFailureAfterTrue[j],
			artStats.trueCD4AtObservedFailureMean[j][SimContext::CD4_PERC],  artStats.trueCD4AtObservedFailureMean[j][SimContext::CD4_ABSOLUTE], artStats.observedCD4AtObservedFailureMean[j], artStats.numObservedFailureCD4Metric[j][SimContext::CD4_PERC],  artStats.numObservedFailureCD4Metric[j][SimContext::CD4_ABSOLUTE], 
			artStats.monthsToObservedFailureMean[j], artStats.monthsToObservedFailureStdDev[j] ) ;
		for (i = 0; i < SimContext::ART_NUM_FAIL_TYPES; ++i) {
			fprintf(statsFile, "\n\t%s \t%1lu \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1lu \t%1lu \t%1.2lf \t%1.2lf ",
				SimContext::ART_FAIL_TYPE_STRS[i],
				artStats.numObservedFailureType[j][i], artStats.numObservedFailureAfterTrueType[j][i],
				artStats.trueCD4AtObservedFailureMeanType[j][i][SimContext::CD4_PERC], artStats.trueCD4AtObservedFailureMeanType[j][i][SimContext::CD4_ABSOLUTE], artStats.observedCD4AtObservedFailureMeanType[j][i], 
				artStats.numObservedFailureTypeCD4Metric[j][i][SimContext::CD4_PERC], artStats.numObservedFailureTypeCD4Metric[j][i][SimContext::CD4_ABSOLUTE],
				artStats.monthsToObservedFailureMeanType[j][i], artStats.monthsToObservedFailureStdDevType[j][i] ) ;
		}
		fprintf(statsFile, "\n\tNo fail diagnoses \t%1lu", artStats.numNeverObservedFailure[j]);

		// print ART stop statistics
		fprintf(statsFile, "\n\tAt ART stop: \t# Total \t# with true fail \tAvgCD4Perc (Peds <5) \tAvgTrueCD4 \tAvgObsvCD4 \tNum w.Perc CD4 Metric \tNum w.Absolute CD4 Metric \tMthsToStop(Mean) \tMthsToStop(StdDev) ");
		fprintf(statsFile, "\n\tAll \t%1lu \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1lu \t%1lu \t%1.2lf \t%1.2lf ",
			artStats.numStop[j], artStats.numStopAfterTrueFailure[j],
			artStats.trueCD4AtStopMean[j][SimContext::CD4_PERC], artStats.trueCD4AtStopMean[j][SimContext::CD4_ABSOLUTE], artStats.observedCD4AtStopMean[j], 
			artStats.numStopCD4Metric[j][SimContext::CD4_PERC], artStats.numStopCD4Metric[j][SimContext::CD4_ABSOLUTE], 
			artStats.monthsToStopMean[j], artStats.monthsToStopStdDev[j] );
		for (i = 0; i < SimContext::ART_NUM_STOP_TYPES; ++i) {
			fprintf(statsFile, "\n\t%s \t%1lu \t%1lu \t%1.2lf \t%1.2lf \t%1.2lf \t%1lu \t%1lu \t%1.2lf \t%1.2lf",
				SimContext::ART_STOP_TYPE_STRS[i],
				artStats.numStopType[j][i], artStats.numStopAfterTrueFailureType[j][i],
				artStats.trueCD4AtStopMeanType[j][i][SimContext::CD4_PERC], artStats.trueCD4AtStopMeanType[j][i][SimContext::CD4_ABSOLUTE], artStats.observedCD4AtStopMeanType[j][i], 
				artStats.numStopTypeCD4Metric[j][i][SimContext::CD4_PERC], artStats.numStopTypeCD4Metric[j][i][SimContext::CD4_ABSOLUTE], 
				artStats.monthsToStopMeanType[j][i], artStats.monthsToStopStdDevType[j][i]);
		}
		fprintf(statsFile, "\n\tNever stopped \t%1lu", artStats.numNeverStop[j]);

		//print ART death statistics
		fprintf(statsFile, "\n\tAt Death on ART Line: \t# Total");
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++){
			fprintf(statsFile,"\t%s", SimContext::DTH_CAUSES_STRS[i]);
		}

		fprintf(statsFile,"\n\tNum Deaths (Total)\t%1lu",artStats.numARTDeath[j]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1lu",artStats.numARTDeathCause[j][i]);
		fprintf(statsFile,"\n\tAvgCD4Perc (Peds <5) at Death\t%1.2lf",artStats.trueCD4AtARTDeathMean[j][SimContext::CD4_PERC]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.trueCD4AtARTDeathCauseMean[j][i][SimContext::CD4_PERC]);
		fprintf(statsFile,"\n\tAvgTrueCD4 at Death\t%1.2lf",artStats.trueCD4AtARTDeathMean[j][SimContext::CD4_ABSOLUTE]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.trueCD4AtARTDeathCauseMean[j][i][SimContext::CD4_ABSOLUTE]);	
		fprintf(statsFile,"\n\tAvgObsvCD4\t%1.2lf",artStats.observedCD4AtARTDeathMean[j]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.observedCD4AtARTDeathCauseMean[j][i]);
		fprintf(statsFile,"\n\tNum w.Perc CD4 Metric at Death\t%1lu",artStats.numARTDeathCD4Metric[j][SimContext::CD4_PERC]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1lu",artStats.numARTDeathCauseCD4Metric[j][i][SimContext::CD4_PERC]);		
		fprintf(statsFile,"\n\tNum w.Absolute CD4 Metric at Death\t%1lu",artStats.numARTDeathCD4Metric[j][SimContext::CD4_ABSOLUTE]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1lu",artStats.numARTDeathCauseCD4Metric[j][i][SimContext::CD4_ABSOLUTE]);			
		fprintf(statsFile,"\n\tAvgPropensity\t%1.2lf",artStats.propensityAtARTDeathMean[j]);
		for (i =0; i < SimContext::DTH_NUM_CAUSES; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.propensityAtARTDeathCauseMean[j][i]);

		//print ART OI statistics
		fprintf(statsFile, "\n\tOI on ART Line:");
		for (i =0; i < SimContext::OI_NUM; i++){
			fprintf(statsFile,"\t%s", SimContext::OI_STRS[i]);
		}
		fprintf(statsFile,"\n\tNum OI");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1lu",artStats.numARTOI[j][i]);
		fprintf(statsFile,"\n\tAvgCD4Perc (Peds <5)");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.trueCD4AtARTOIMean[j][i][SimContext::CD4_PERC]);	
		fprintf(statsFile,"\n\tAvgTrueCD4");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.trueCD4AtARTOIMean[j][i][SimContext::CD4_ABSOLUTE]);
		fprintf(statsFile,"\n\tAvgObsvCD4");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.observedCD4AtARTOIMean[j][i]);
		fprintf(statsFile, "\n\tNum w.Perc CD4 Metric at OI event");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1lu",artStats.numARTOICD4Metric[j][i][SimContext::CD4_PERC]);	
		fprintf(statsFile, "\n\tNum w.Absolute CD4 Metric at OI event");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1lu",artStats.numARTOICD4Metric[j][i][SimContext::CD4_ABSOLUTE]);	
		fprintf(statsFile,"\n\tAvgPropensity");
		for (i =0; i < SimContext::OI_NUM; i++)
			fprintf(statsFile,"\t%1.2lf",artStats.propensityAtARTOIMean[j][i]);

		// print number of patients on ART at intervals
		fprintf(statsFile, "\n\t \t# at Mth %d \t# Supp, Mth %d \t# at Mth %d \t# Supp, Mth %d \t# at Mth %d \t# Supp, Mth %d",
			runSpecs->monthRecordARTEfficacy[0], runSpecs->monthRecordARTEfficacy[0],
			runSpecs->monthRecordARTEfficacy[1], runSpecs->monthRecordARTEfficacy[1],
			runSpecs->monthRecordARTEfficacy[2], runSpecs->monthRecordARTEfficacy[2]);
		fprintf(statsFile, "\n\tNumber Patients: \t%1lu \t%1lu \t%1lu \t%1lu \t%1lu \t%1lu",
			artStats.numOnARTAtMonth[j][0], artStats.numSuppressedAtMonth[j][0],
			artStats.numOnARTAtMonth[j][1], artStats.numSuppressedAtMonth[j][1],
			artStats.numOnARTAtMonth[j][2], artStats.numSuppressedAtMonth[j][2]);
		fprintf(statsFile, "\n\t \tMean, Mth %d \tSD, Mth %d \tMean, Mth %d \tSD, Mth %d \tMean, Mth %d \tSD, Mth %d",
			runSpecs->monthRecordARTEfficacy[0], runSpecs->monthRecordARTEfficacy[0],
			runSpecs->monthRecordARTEfficacy[1], runSpecs->monthRecordARTEfficacy[1],
			runSpecs->monthRecordARTEfficacy[2], runSpecs->monthRecordARTEfficacy[2]);
		fprintf(statsFile, "\n\tHVL Drops: \t%1.4lf \t%1.4lf \t%1.4lf \t%1.4lf \t%1.4lf \t%1.4lf",
			artStats.HVLDropsAtMonthMean[j][0], artStats.HVLDropsAtMonthStdDev[j][0],
			artStats.HVLDropsAtMonthMean[j][1], artStats.HVLDropsAtMonthStdDev[j][1],
			artStats.HVLDropsAtMonthMean[j][2], artStats.HVLDropsAtMonthStdDev[j][2]);

		// print out initial art patient distributions and toxicities
		fprintf(statsFile,"\n\tPat Distrib at Init:");
		for ( i = SimContext::HVL_NUM_STRATA - 1; i >= 0; --i )
			fprintf(statsFile," \t%s", SimContext::HVL_STRATA_STRS[i]);
		for ( i = SimContext::CD4_NUM_STRATA - 1; i >= 0; --i ) {
			fprintf(statsFile,"\n\t%s", SimContext::CD4_STRATA_STRS[i]);
			for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k )
				fprintf(statsFile, " \t%1lu", artStats.distributionAtInit[j][i][k]);
		}
		fprintf(statsFile, "\n\tMinor Tox Cases:");
		for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k )
			fprintf(statsFile, " \t%1lu", artStats.numToxicityCases[j][SimContext::ART_TOX_MINOR][k]);
		fprintf(statsFile, "\n\tChronic Tox Cases:");
		for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k )
			fprintf(statsFile, " \t%1lu", artStats.numToxicityCases[j][SimContext::ART_TOX_CHRONIC][k]);
		fprintf(statsFile, "\n\tMajor Tox Cases:");
		for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k )
			fprintf(statsFile, " \t%1lu", artStats.numToxicityCases[j][SimContext::ART_TOX_MAJOR][k]);
		fprintf(statsFile, "\n\tDeath Tox Cases:");
		for ( k = SimContext::HVL_NUM_STRATA - 1; k >= 0; --k )
			fprintf(statsFile, " \t%1lu", artStats.numToxicityDeaths[j][k]);

		// print out ART STI stats
		fprintf(statsFile, "\n\t");
		for ( k = 1; k <= SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\tCycle %d", k);
		fprintf(statsFile, "+\t\t");
		for ( k = 1; k <= SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\tCycle %d", k);
		fprintf(statsFile, "+\t\t");
		for ( k = 1; k <= SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\tCycle %d", k);
		fprintf(statsFile, "+");
		fprintf(statsFile, "\n\tInterruptions:");
		for ( k = 0; k < SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\t%lu", artStats.numSTIInterruptions[j][k]);
		fprintf(statsFile, "\t\tRestarts:");
		for ( k = 0; k < SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\t%lu", artStats.numSTIRestarts[j][k]);
		fprintf(statsFile, "\t\tSTI Endpoint:");
		for ( k = 0; k < SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\t%lu", artStats.numSTIEndpoints[j][k]);
		fprintf(statsFile,"\n\t");
		for ( k = 0; k < SimContext::STI_NUM_TRACKED; k++ ) {
			fprintf(statsFile, "\t%d Interrupts", k+1);
		}
		fprintf(statsFile, "+\n\t#People who have had n interrupts");
		for ( k = 0; k < SimContext::STI_NUM_TRACKED; ++k )
			fprintf(statsFile, "\t%u", artStats.numPatientsWithSTIInterruptions[j][k]);
		fprintf(statsFile, "\n\tMean Interruptions:");
		fprintf(statsFile, "\t%lf", artStats.numSTIInterruptionsMean[j]);
		fprintf(statsFile, "\n\tMean Interruption Duration:");
		fprintf(statsFile, "\t%lf", artStats.monthsOnSTIInterruptionMean[j]);
	}
} /* end writeARTStats */

/** \brief writeTimeSummaries outputs the vector of TimeSummary objects statistics to the stats file */
void RunStats::writeTimeSummaries() {
	int j, k;
	const SimContext::RunSpecsInputs *runSpecs = simContext->getRunSpecsInputs();

	if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_NONE)
		return;

	for (vector<TimeSummary *>::iterator t = timeSummaries.begin(); t != timeSummaries.end(); t++) {
		TimeSummary *currTime = *t;

		if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_YR_DET)
			fprintf(statsFile,"\nCOHORT SUMMARY FOR YEAR %d END", currTime->timePeriod);
		else
			fprintf(statsFile,"\nCOHORT SUMMARY FOR MONTH %d", currTime->timePeriod);

		//output cohort characteristics by care state
		fprintf(statsFile,"\n\tCare State\t#Alive\t#Deaths\tTrue CD4 Mean\tTrue CD4 SD\tObserved CD4 Mean\tObserved CD4 SD\tProp Resp Mean\tProp Resp SD\tAge Mean\tAge SD\tNum Mild OI Hist\tNum Sevr OI Hist\tNum Male");
		if(simContext->getPedsInputs()->enablePediatricsModel)
			fprintf(statsFile,"\tNum w.Perc CD4 Metric (Peds < 5)\tNum w.Absolute CD4 Metric");
		for (j = 0; j < SimContext::HIV_CARE_NUM; j++){
			fprintf(statsFile, "\n\t%s", SimContext::HIV_CARE_STRS[j]);
			fprintf(statsFile, "\t%1lu\t%1lu", currTime->numAliveCare[j], currTime->numDeathsCare[j]);
			fprintf(statsFile,  "\t%1.2lf\t%1.2lf\t%1.2lf\t%1.2lf",currTime->trueCD4MeanCare[j], currTime->trueCD4StdDevCare[j], currTime->observedCD4MeanCare[j], currTime->observedCD4StdDevCare[j]);
			fprintf(statsFile, "\t%1.4lf\t%1.4lf", currTime->propRespMeanCare[j], currTime->propRespStdDevCare[j]);
			fprintf(statsFile, "\t%1.2lf\t%1.2lf", currTime->ageMeanCare[j], currTime->ageStdDevCare[j]);
			fprintf(statsFile, "\t%1lu\t%1lu\t%1lu",currTime->numWithOIHistExtCare[SimContext::HIST_EXT_MILD][j],currTime->numWithOIHistExtCare[SimContext::HIST_EXT_SEVR][j], currTime->numGenderCare[SimContext::GENDER_MALE][j]);
			if(simContext->getPedsInputs()->enablePediatricsModel){
				fprintf(statsFile, "\t%lu\t%lu", currTime->numAliveCareCD4Metric[j][SimContext::CD4_PERC], currTime->numAliveCareCD4Metric[j][SimContext::CD4_ABSOLUTE]);
			}
		}
		SimContext::ART_EFF_TYPE effToUse;
		for (j = 0; j < SimContext::ART_NUM_LINES; j++){
			for (k = 0; k < 2; k++){
				if (k==0){
					effToUse = SimContext::ART_EFF_SUCCESS;
					fprintf(statsFile, "\n\tHIV+onART%d_supp", j+1);
				}
				else{
					effToUse = SimContext::ART_EFF_FAILURE;
					fprintf(statsFile, "\n\tHIV+onART%d_fail", j+1);
				}
				fprintf(statsFile, "\t%1lu\t%1lu", currTime->numAliveOnART[j][effToUse], currTime->numDeathsOnART[j][effToUse]);
				fprintf(statsFile,  "\t%1.2lf\t%1.2lf\t%1.2lf\t%1.2lf",currTime->trueCD4MeanOnART[j][effToUse], currTime->trueCD4StdDevOnART[j][effToUse], currTime->observedCD4MeanOnART[j][effToUse], currTime->observedCD4StdDevOnART[j][effToUse]);
				fprintf(statsFile, "\t%1.4lf\t%1.4lf", currTime->propRespMeanOnART[j][effToUse], currTime->propRespStdDevOnART[j][effToUse]);
				fprintf(statsFile, "\t%1.2lf\t%1.2lf", currTime->ageMeanOnART[j][effToUse], currTime->ageStdDevOnART[j][effToUse]);
				fprintf(statsFile, "\t%1lu\t%1lu\t%1lu",currTime->numWithOIHistExtOnART[SimContext::HIST_EXT_MILD][j][effToUse],currTime->numWithOIHistExtOnART[SimContext::HIST_EXT_SEVR][j][effToUse], currTime->numGenderOnART[SimContext::GENDER_MALE][j][effToUse]);
				if(simContext->getPedsInputs()->enablePediatricsModel){
					fprintf(statsFile, "\t%lu\t%lu", currTime->numAliveOnARTCD4Metric[j][effToUse][SimContext::CD4_PERC], currTime->numAliveOnARTCD4Metric[j][effToUse][SimContext::CD4_ABSOLUTE]);
				}
			}
		}
		fprintf(statsFile, "\n\tHIV+in_care_offART");
		fprintf(statsFile, "\t%1lu\t%1lu", currTime->numAliveInCareOffART, currTime->numDeathsInCareOffART);
		fprintf(statsFile,  "\t%1.2lf\t%1.2lf\t%1.2lf\t%1.2lf",currTime->trueCD4MeanInCareOffART, currTime->trueCD4StdDevInCareOffART, currTime->observedCD4MeanInCareOffART, currTime->observedCD4StdDevInCareOffART);
		fprintf(statsFile, "\t%1.4lf\t%1.4lf", currTime->propRespMeanInCareOffART, currTime->propRespStdDevInCareOffART);
		fprintf(statsFile, "\t%1.2lf\t%1.2lf", currTime->ageMeanInCareOffART, currTime->ageStdDevInCareOffART);
		fprintf(statsFile, "\t%1lu\t%1lu\t%1lu",currTime->numWithOIHistExtInCareOffART[SimContext::HIST_EXT_MILD],currTime->numWithOIHistExtInCareOffART[SimContext::HIST_EXT_SEVR], currTime->numGenderInCareOffART[SimContext::GENDER_MALE]);
		if(simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile, "\t%lu\t%lu", currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_PERC], currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_ABSOLUTE]);
		}

		fprintf(statsFile, "\n\tTotal HIV+");
		fprintf(statsFile, "\t%1lu\t%1lu", currTime->numAlivePositive, currTime->numDeathsPositive);
		fprintf(statsFile,  "\t%1.2lf\t%1.2lf\t%1.2lf\t%1.2lf",currTime->trueCD4Mean, currTime->trueCD4StdDev, currTime->observedCD4Mean, currTime->observedCD4StdDev);
		fprintf(statsFile, "\t%1.4lf\t%1.4lf", currTime->propRespMeanPositive, currTime->propRespStdDevPositive);
		fprintf(statsFile, "\t%1.2lf\t%1.2lf", currTime->ageMeanPositive, currTime->ageStdDevPositive);
		fprintf(statsFile, "\t%1lu\t%1lu\t%1lu",currTime->numWithOIHistExtPositive[SimContext::HIST_EXT_MILD],currTime->numWithOIHistExtPositive[SimContext::HIST_EXT_SEVR], currTime->numGenderPositive[SimContext::GENDER_MALE]);
		if(simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile, "\t%lu\t%lu", currTime->numAlivePositiveCD4Metric[SimContext::CD4_PERC], currTime->numAlivePositiveCD4Metric[SimContext::CD4_ABSOLUTE]);
		}

		fprintf(statsFile, "\n\tTotal");
		fprintf(statsFile, "\t%1lu\t%1lu", currTime->numAlive, currTime->numDeaths);
		fprintf(statsFile,  "\t%1.2lf\t%1.2lf\t%1.2lf\t%1.2lf",currTime->trueCD4Mean, currTime->trueCD4StdDev, currTime->observedCD4Mean, currTime->observedCD4StdDev);
		fprintf(statsFile, "\t%1.4lf\t%1.4lf", currTime->propRespMean, currTime->propRespStdDev);
		fprintf(statsFile, "\t%1.2lf\t%1.2lf", currTime->ageMean, currTime->ageStdDev);
		fprintf(statsFile, "\t%1lu\t%1lu\t%1lu",currTime->numWithOIHistExt[SimContext::HIST_EXT_MILD],currTime->numWithOIHistExt[SimContext::HIST_EXT_SEVR], currTime->numGender[SimContext::GENDER_MALE]);
		if(simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile, "\t%lu\t%lu", currTime->numAlivePositiveCD4Metric[SimContext::CD4_PERC]+currTime->numAliveCareCD4Metric[SimContext::HIV_CARE_NEG][SimContext::CD4_PERC], currTime->numAlivePositiveCD4Metric[SimContext::CD4_ABSOLUTE]+currTime->numAliveCareCD4Metric[SimContext::HIV_CARE_NEG][SimContext::CD4_ABSOLUTE]);
		}

		fprintf(statsFile, "\n\t\tPediatric HIV+ (< 5 y old)\tNon-Pediatric HIV+");
		fprintf(statsFile, "\n\tNum Patients:\t%lu\t%lu", currTime->numAlivePositiveCD4Metric[SimContext::CD4_PERC],currTime->numAlivePositiveCD4Metric[SimContext::CD4_ABSOLUTE]);
		fprintf(statsFile, "\n\t\tLowRisk HIVneg\tHighRisk HIVneg");
		fprintf(statsFile, "\n\tNum Alive\t%lu\t%lu", currTime->numAliveNegRisk[SimContext::HIV_BEHAV_LO], currTime->numAliveNegRisk[SimContext::HIV_BEHAV_HI]);
        fprintf(statsFile, "\n\tProb PrEP Uptake\t%1.5lf\t%1.5lf", currTime->probPrepUptake[SimContext::HIV_BEHAV_LO], currTime->probPrepUptake[SimContext::HIV_BEHAV_HI]);

		// Output number alive and number of deaths, stratified by age bracket and state, as well as the totals for each state
		fprintf(statsFile,"\n\t\tAge Bracket (Years)");
		fprintf(statsFile, "\n\tAge-Stratified Outputs");
		for (j = 0 ; j < SimContext::OUTPUT_AGE_CAT_NUM; j++)
			fprintf(statsFile, "\t%s", SimContext::OUTPUT_AGE_CAT_STRS[j]);
		fprintf(statsFile, "\tTotal");	
		for (j = 0; j < SimContext::HIV_CARE_NUM; j++){
			fprintf(statsFile, "\n\t%s", SimContext::HIV_CARE_STRS[j]);
			for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
				fprintf(statsFile, "\t%1lu", currTime->numAgeBracketCare[j][k]);
			fprintf(statsFile, "\t%1lu",currTime->numAliveCare[j]);	
			fprintf(statsFile, "\n\tNum Deaths %s", SimContext::HIV_CARE_STRS[j]);
			for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
				fprintf(statsFile, "\t%1lu", currTime->numDeathsAgeBracketCare[j][k]);
			fprintf(statsFile, "\t%1lu", currTime->numDeathsCare[j]);	
		}

		fprintf(statsFile, "\n\t%s", "HIV+onART");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numAgeBracketOnART[k]);
		fprintf(statsFile, "\t%1lu", currTime->totalAliveOnART);	
		fprintf(statsFile, "\n\t%s", "Num Deaths HIV+onART");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numDeathsAgeBracketOnART[k]);
		fprintf(statsFile, "\t%1lu", currTime->totalDeathsOnART);	
		fprintf(statsFile, "\n\t%s", "HIV+in_care_offART");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numAgeBracketInCareOffART[k]);
		fprintf(statsFile, "\t%1lu", currTime->numAliveInCareOffART);	
		fprintf(statsFile, "\n\t%s", "Num Deaths HIV+in_care_offART");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numDeathsAgeBracketInCareOffART[k]);	
		fprintf(statsFile, "\t%1lu", currTime->numDeathsInCareOffART);	
		fprintf(statsFile, "\n\t%s", "Total HIV+");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numAgeBracketHIVPositive[k]);
		fprintf(statsFile, "\t%1lu", currTime->numAlivePositive);	
		fprintf(statsFile, "\n\t%s", "Num Deaths HIV+");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numDeathsAgeBracketHIVPositive[k]);	
		fprintf(statsFile, "\t%1lu", currTime->numDeathsPositive);	
		fprintf(statsFile, "\n\t%s", "Total Alive");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numAgeBracketAlive[k]);
		fprintf(statsFile, "\t%1lu", currTime->numAlive);	
		fprintf(statsFile, "\n\t%s", "Num Deaths");
		for (k = 0; k < SimContext::OUTPUT_AGE_CAT_NUM; k++)
			fprintf(statsFile, "\t%1lu", currTime->numDeathsAgeBracket[k]);
		fprintf(statsFile, "\t%1lu", currTime->numDeaths);		

		fprintf(statsFile, "\n\tIncident HIV+");
		if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_MTH_DET){
			fprintf(statsFile,"\t\tTotal QOL applied");
		}
		fprintf(statsFile, "\n\t%1lu", currTime->numIncidentHIVInfections);
		if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_MTH_DET){
			fprintf(statsFile, "\t\t%1.2f", currTime->sumQOLModifiers);
		}
		fprintf(statsFile,"\n\t\tHighRisk\tLowRisk");
		fprintf(statsFile,"\n\tNum on PrEP:");
		for (j = 0; j < SimContext::HIV_BEHAV_NUM; ++j)
            fprintf(statsFile, "\t%lu", currTime->numOnPrEP[j]);
		fprintf(statsFile, "\n\t");
		for (int i = 0; i < SimContext::HIV_DET_NUM; ++i)
			fprintf(statsFile, "\t%s", SimContext::HIV_DET_STRS[i]);
		fprintf(statsFile, "\n\tHIV Detections");
		for(int i = 0; i < SimContext::HIV_DET_NUM; ++i)
			fprintf(statsFile, "\t%1lu", currTime->numHIVDetections[i]);
		fprintf(statsFile, "\n\tCumulative HIV Detections");
		for(int i = 0; i < SimContext::HIV_DET_NUM; ++i)
			fprintf(statsFile, "\t%1lu", currTime->cumulativeNumHIVDetections[i]);
		fprintf(statsFile, "\n\tHIV Tests Performed");
		fprintf(statsFile, "\n\t\tInitial Offers (Screening Startup Month)\tRepeat Offers (Post-Startup Month)\tTotal");
		fprintf(statsFile, "\n\tCurrent\t%1lu\t%1lu\t%1lu", currTime->numHIVTestsPerformedAtInitOffer, currTime->numHIVTestsPerformedPostStartup, currTime->numHIVTestsPerformed);
		fprintf(statsFile, "\n\tCumulative\t%1lu\t%1lu\t%1lu", currTime->cumulativeNumHIVTestsAtInitOffer, currTime->cumulativeNumHIVTestsPostStartup, currTime->cumulativeNumHIVTests);

		//Output peds information if enabled
		if (simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile, "\n\t");
			for (j = 0; j < SimContext::PEDS_HIV_NUM; ++j)
				fprintf(statsFile, "\t%s", SimContext::PEDS_HIV_STATE_STRS[j]);
			fprintf(statsFile,"\n\t# Alive Pediatrics:");
			for (j = 0; j < SimContext::PEDS_HIV_NUM; ++j) {
				fprintf(statsFile, "\t%1lu", currTime->numAlivePediatrics[j]);
			}
			fprintf(statsFile, "\n\tPediatric HIV Exposure");	
			fprintf(statsFile, "\tMother Chronic HIV (Breastfeeding)\tMother Acute HIV (Breastfeeding)\tHIV- Unexposed");
			fprintf(statsFile,"\n\t# Exposed");
			for (j = 0; j < SimContext::PEDS_EXPOSED_BREASTFEEDING_NUM; ++j)
				fprintf(statsFile, "\t%1lu", currTime->numHIVExposedUninf[j]);
			fprintf(statsFile, "\t%1lu", currTime->numNeverHIVExposed);		
			fprintf(statsFile, "\n\tMaternal Status");
			for (j = 0; j < SimContext::PEDS_MATERNAL_STATUS_NUM; ++j)
				fprintf(statsFile, "\t%s", SimContext::PEDS_MATERNAL_STATUS_STRS[j]);
			fprintf(statsFile, "\tMother Dead");
			fprintf(statsFile,"\n\t# Alive Pediatrics:");
			for (j = 0; j < SimContext::PEDS_HIV_NUM; ++j) {
				fprintf(statsFile, "\t%1lu", currTime->numAlivePediatricsMotherAlive[j]);
			}
			fprintf(statsFile, "\t%1lu", currTime->numAlivePediatricsMotherDead);

			fprintf(statsFile, "\n\t\tFalse Positive\tFalse Positive Linked");
			fprintf(statsFile, "\n\t#Alive\t%1lu\t%1lu", currTime->numAliveFalsePositive, currTime->numAliveFalsePositiveLinked);

			fprintf(statsFile, "\n\t# Incident PP Infections\t%1lu", currTime->numIncidentPPInfections);
			fprintf(statsFile, "\n\t# Newly Detected Pediatrics with Maternal Status Unknown\t%1lu", currTime->numNewlyDetectedPediatricsMotherStatusUnknown);

			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::EID_TEST_TYPE_NUM; j++)
				fprintf(statsFile, "\t%s", SimContext::EID_TEST_TYPE_STRS[j]);
			fprintf(statsFile,"\t");
			for(j = 0; j < SimContext::EID_NUM_TESTS; j++)
				fprintf(statsFile, "\tHIV Test %d", j);

			fprintf(statsFile, "\n\t#EID Tests Given");
			for (j = 0; j < SimContext::EID_TEST_TYPE_NUM; j++)
				fprintf(statsFile, "\t%1lu", currTime->numEIDTestsGivenType[j]);
			fprintf(statsFile,"\t");
			for (j = 0; j < SimContext::EID_NUM_TESTS; j++)
				fprintf(statsFile, "\t%1lu", currTime->numEIDTestsGivenTest[j]);

			fprintf(statsFile, "\n\t#True Positive EID Test Results");
			for (j = 0; j < SimContext::EID_TEST_TYPE_NUM; j++)
				fprintf(statsFile, "\t%1lu", currTime->numTruePositiveEIDTestResultsType[j]);
			fprintf(statsFile,"\t");
			for (j = 0; j < SimContext::EID_NUM_TESTS; j++)
				fprintf(statsFile, "\t%1lu", currTime->numTruePositiveEIDTestResultsTest[j]);

			fprintf(statsFile, "\n\t#True Negative EID Test Results");
			for (j = 0; j < SimContext::EID_TEST_TYPE_NUM; j++)
				fprintf(statsFile, "\t%1lu", currTime->numTrueNegativeEIDTestResultsType[j]);
			fprintf(statsFile,"\t");
			for (j = 0; j < SimContext::EID_NUM_TESTS; j++)
				fprintf(statsFile, "\t%1lu", currTime->numTrueNegativeEIDTestResultsTest[j]);

			fprintf(statsFile, "\n\t#False Positive EID Test Results");
			for (j = 0; j < SimContext::EID_TEST_TYPE_NUM; j++)
				fprintf(statsFile, "\t%1lu", currTime->numFalsePositiveEIDTestResultsType[j]);
			fprintf(statsFile,"\t");
			for (j = 0; j < SimContext::EID_NUM_TESTS; j++)
				fprintf(statsFile, "\t%1lu", currTime->numFalsePositiveEIDTestResultsTest[j]);

			fprintf(statsFile, "\n\t#False Negative EID Test Results");
			for (j = 0; j < SimContext::EID_TEST_TYPE_NUM; j++)
				fprintf(statsFile, "\t%1lu", currTime->numFalseNegativeEIDTestResultsType[j]);
			fprintf(statsFile,"\t");
			for (j = 0; j < SimContext::EID_NUM_TESTS; j++)
				fprintf(statsFile, "\t%1lu", currTime->numFalseNegativeEIDTestResultsTest[j]);
		} // end peds outputs

		if (simContext->getCHRMsInputs()->showCHRMsOutput){
			fprintf(statsFile,"\n\n\t\tDetected State\t\t\t\tAge");
			for (j = 0; j < SimContext::CHRM_AGE_CAT_NUM; ++j)
				fprintf(statsFile, "\t");
			fprintf(statsFile,"Gender");
			for (j = 0; j < SimContext::GENDER_NUM; ++j)
				fprintf(statsFile, "\t");
			fprintf(statsFile,"CD4(HIV+)");
			for (j = 0; j < SimContext::CD4_NUM_STRATA; ++j)
				fprintf(statsFile, "\t");

			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::HIV_ID_NUM; ++j)
				fprintf(statsFile, "\t%s", SimContext::HIV_ID_STRS[j]);
			fprintf(statsFile,"\tTotal");
			for (j = 0; j < SimContext::CHRM_AGE_CAT_NUM; ++j)
				fprintf(statsFile, "\t%s", SimContext::CHRM_AGE_CAT_STRS[j]);
			for (j = 0; j < SimContext::GENDER_NUM; ++j)
				fprintf(statsFile, "\t%s", SimContext::GENDER_STRS[j]);
			for (j = 0; j < SimContext::CD4_NUM_STRATA; ++j)
				fprintf(statsFile, "\t%s", SimContext::CD4_STRATA_STRS[j]);

			fprintf(statsFile,"\n\t# Alive with CHRMs");
			for (j = 0; j < SimContext::HIV_ID_NUM; ++j) {
				fprintf(statsFile, "\t%1lu", currTime->numAliveWithCHRMsDetState[j]);
			}
			fprintf(statsFile, "\t%1lu", currTime->numAliveWithCHRMs);
			for(j=0;j<SimContext::CHRM_AGE_CAT_NUM;j++){
				fprintf(statsFile, "\t%1lu", currTime->numCHRMsAgeTotal[j]);
			}
			for(j=0;j<SimContext::GENDER_NUM;j++){
				fprintf(statsFile, "\t%1lu", currTime->numCHRMsGenderTotal[j]);
			}
			for(j=0;j<SimContext::CD4_NUM_STRATA;j++){
				fprintf(statsFile, "\t%1lu", currTime->numCHRMsCD4Total[j]);
			}

			for(int i=0;i<SimContext::CHRM_NUM;i++){
				fprintf(statsFile,"\n\t%s",SimContext::CHRM_STRS[i]);
				for (j = 0; j < SimContext::HIV_ID_NUM; ++j) {
					fprintf(statsFile, "\t%1lu", currTime->numAliveTypeCHRMs[j][i]);
				}
				fprintf(statsFile, "\t%1lu", currTime->numAliveCHRM[i]);

				for(int j=0;j<SimContext::CHRM_AGE_CAT_NUM;j++){
					fprintf(statsFile, "\t%1lu", currTime->numCHRMsAge[i][j]);
				}
				for(int j=0;j<SimContext::GENDER_NUM;j++){
					fprintf(statsFile, "\t%1lu", currTime->numCHRMsGender[i][j]);
				}
				for(int j=0;j<SimContext::CD4_NUM_STRATA;j++){
					fprintf(statsFile, "\t%1lu", currTime->numCHRMsCD4[i][j]);
				}
			}
			fprintf(statsFile,"\n\t# Alive without CHRMs");
			for (j = 0; j < SimContext::HIV_ID_NUM; ++j) {
				fprintf(statsFile, "\t%1lu", currTime->numAliveWithoutCHRMsDetState[j]);
			}
			fprintf(statsFile, "\t%1lu", currTime->numAliveWithoutCHRMs);
			//output chrms incidence for this month
			fprintf(statsFile,"\n\t");
			for(j=0;j<SimContext::CHRM_NUM;j++){
				fprintf(statsFile," \t%s",SimContext::CHRM_STRS[j]);
			}
			fprintf(statsFile,"\n\tIncident CHRMs evts");
			for(j=0;j<SimContext::CHRM_NUM;j++){
				fprintf(statsFile," \t%1lu",currTime->numIncidentCHRMs[j]);
			}
		}// end if CHRMs outputs are enabled 
		// output true and observed CD4 and HVL
		if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_MTH_BRF) {
			fprintf(statsFile,"\n\tMeanTrueCD4 (HIV+): \t%1.0lf", currTime->trueCD4Mean );
			fprintf(statsFile,"\t\tMeanTrueHVL (HIV+): \t%1.0lf", currTime->trueHVLMean );
		}
		else {
			if (simContext->getHIVTestInputs()->enableHIVTesting || simContext->getPedsInputs()->enablePediatricsModel){
				fprintf(statsFile,"\n\t\tMean True CD4\tSD\t\tMean Observed CD4\tSD");
				for (int i = 1; i < SimContext::HIV_CARE_NUM; i++){
					fprintf(statsFile,"\n\t%s\t%1.0lf\t%1.0lf\t\t%1.0lf\t%1.0lf",simContext->HIV_CARE_STRS[i],currTime->trueCD4MeanCare[i],currTime->trueCD4StdDevCare[i],currTime->observedCD4MeanCare[i],currTime->observedCD4StdDevCare[i]);
				}
			}
			else{
				fprintf(statsFile,"\n\t\tMean True CD4\tSD\t\tMean Observed CD4\tSD");
				fprintf(statsFile,"\n\tHIV+\t%1.0lf\t%1.0lf\t\t%1.0lf\t%1.0lf",
					currTime->trueCD4Mean, currTime->trueCD4StdDev, 
					currTime->observedCD4Mean, currTime->observedCD4StdDev );
			}
			fprintf(statsFile,"\n\t\tMean CD4 Perc\tSD");
			fprintf(statsFile, "\n\tPeds < 5 y old HIV+\t%1.2lf \t%1.2lf",
				currTime->trueCD4PercentageMean, currTime->trueCD4PercentageStdDev );
			fprintf(statsFile,"\n\t\tMean\tSD\t\t\tMean\tSD");
			fprintf(statsFile,"\n\tTrue HVL (HIV+)\t%1.0lf \t%1.0lf SD",
				currTime->trueHVLMean, currTime->trueHVLStdDev );
			fprintf(statsFile,"\t\tObsv HVL (HIV+)\t%1.0lf \t%1.0lf SD",
				currTime->observedHVLMean, currTime->observedHVLStdDev );

			// output CD4 and HVL distribs
			fprintf(statsFile,"\n\tCD4 Strata Distrib");
			for ( j = 0; j < SimContext::CD4_NUM_STRATA; ++j )
				fprintf(statsFile," \t%s", SimContext::CD4_STRATA_STRS[j]);
			fprintf(statsFile,"\n\tObsv CD4:");
			for ( j = 0; j < SimContext::CD4_NUM_STRATA; ++j )
				fprintf(statsFile," \t%1lu", currTime->observedCD4Distribution[j]);
			fprintf(statsFile,"\n\tTrue CD4/HVL Strata Distrib");
			for ( j = 0; j < SimContext::HVL_NUM_STRATA; ++j )
				fprintf(statsFile," \t%s", SimContext::HVL_STRATA_STRS[j]);
			fprintf(statsFile," \tTotal");
			for ( j = 0; j < SimContext::CD4_NUM_STRATA; ++j ) {
				fprintf(statsFile,"\n\tOffART:%s", SimContext::CD4_STRATA_STRS[j]);
				for ( k = 0; k < SimContext::HVL_NUM_STRATA; ++k ) {
					fprintf(statsFile," \t%1lu", currTime->trueCD4HVLARTDistribution[SimContext::ART_OFF_STATE][j][k] );
				}
				fprintf(statsFile," \t%1lu", currTime->trueCD4ARTDistribution[SimContext::ART_OFF_STATE][j]);
			}
			for ( j = 0; j < SimContext::CD4_NUM_STRATA; ++j ) {
				fprintf(statsFile,"\n\tOnART:%s", SimContext::CD4_STRATA_STRS[j]);
				for ( k = 0; k < SimContext::HVL_NUM_STRATA; ++k ) {
					fprintf(statsFile," \t%1lu", currTime->trueCD4HVLARTDistribution[SimContext::ART_ON_STATE][j][k] );
				}
				fprintf(statsFile," \t%1lu", currTime->trueCD4ARTDistribution[SimContext::ART_ON_STATE][j]);
				switch (j) {
					case 0:
						fprintf(statsFile, " \t\tSuppressed State on ART");
						break;
					case 1:
						fprintf(statsFile, " \t\tSuppressed \t%1lu",
							currTime->numARTEfficacyState[SimContext::ART_EFF_SUCCESS]);
						break;
					case 2:
						fprintf(statsFile, " \t\tFailure \t%1lu",
							currTime->numARTEfficacyState[SimContext::ART_EFF_FAILURE]);
						break;
				}
			}
			fprintf(statsFile,"\n\tTotal by True HVL");
			for ( k = 0; k < SimContext::HVL_NUM_STRATA; ++k ) {
				fprintf(statsFile," \t%1lu", currTime->trueHVLDistribution[k]);
			}
			fprintf(statsFile,"\n\tObsv HVL Strata Distrib");
			for ( j = 0; j < SimContext::HVL_NUM_STRATA; ++j )
				fprintf(statsFile," \t%1lu", currTime->observedHVLDistribution[j]);
			//Output ART Tox information
        	// Incident Toxicities
			fprintf(statsFile, "\n\tIncident ART Toxicities");
			fprintf(statsFile, "\n\t");
			for (j = 0; j < SimContext::ART_NUM_LINES; ++j) {
				for (k = 0; k < SimContext::ART_NUM_SUBREGIMENS; ++k) {
					fprintf(statsFile, "\tART %d.%d", j+1, k);
				}
			}
			for (int n = 0; n < SimContext::ART_NUM_TOX_SEVERITY; ++n){
				for (int m = 0; m < SimContext::ART_NUM_TOX_PER_SEVERITY; ++m){
					fprintf(statsFile, "\n\t%s Tox %d", SimContext::ART_TOX_SEVERITY_STRS[n], m);
					for (j = 0; j < SimContext::ART_NUM_LINES; ++j) {
						for (k = 0; k < SimContext::ART_NUM_SUBREGIMENS; ++k) {
							fprintf(statsFile, "\t%d", currTime->incidentToxicities[j][k][n][m]);
						}
					}
				}
			}
			// Prevalent Chronic Toxicities
			fprintf(statsFile, "\n\tPrevalent Chronic ART Toxicities");
			fprintf(statsFile, "\n\t");
			for (j = 0; j < SimContext::ART_NUM_LINES; ++j) {
				for (k = 0; k < SimContext::ART_NUM_SUBREGIMENS; ++k) {
					fprintf(statsFile, "\tART %d.%d", j+1, k);
				}
			}
			for (int m = 0; m < SimContext::ART_NUM_TOX_PER_SEVERITY; ++m){
				fprintf(statsFile, "\n\tChr Tox %d",  m);
				for (j = 0; j < SimContext::ART_NUM_LINES; ++j) {
					for (k = 0; k < SimContext::ART_NUM_SUBREGIMENS; ++k) {
						fprintf(statsFile, "\t%d", currTime->prevalentChronicToxicities[j][k][m]);
					}
				}
			}	
			
			if (simContext->getCohortInputs()->showTransmissionOutput){
                fprintf(statsFile, "\n\t\tWarm up Run\tPrimary Cohort");
                fprintf(statsFile, "\n\tIncident Infections");
                fprintf(statsFile, "\t%1lu", currTime->dynamicNumIncidentHIVInfections);
                fprintf(statsFile, "\t%1lu", currTime->numIncidentHIVInfections);
                fprintf(statsFile, "\n\tHIV- at Month Start");
                fprintf(statsFile, "\t%1lu", currTime->dynamicNumHIVNegAtStartMonth);
                fprintf(statsFile, "\t%1lu", currTime->debugNumHIVNegAtStartMonth);
                fprintf(statsFile,"\n\tSelf Transmission Rate Multiplier");
                fprintf(statsFile,"\t%1.5lf", currTime->dynamicSelfTransmissionMult);
				fprintf(statsFile, "\n\t");
				for ( k = 0; k < SimContext::HVL_NUM_STRATA; ++k )
					fprintf(statsFile," \t%s", SimContext::HVL_STRATA_STRS[k]);
				fprintf(statsFile,"\tTotal");
				fprintf(statsFile,"\n\tPrimary Transmissions by True HVL");
				for ( k = 0; k < SimContext::HVL_NUM_STRATA; ++k )
					fprintf(statsFile," \t%1.3lf", currTime->numTransmissionsHVL[k]);
				fprintf(statsFile," \t%1.3lf", currTime->numTransmissions);

				fprintf(statsFile, "\n\t");
				for (k = 0; k < SimContext::TRANSM_RISK_NUM; k++)
					fprintf(statsFile, "\t%s", SimContext::TRANSM_RISK_STRS[k]);
				fprintf(statsFile, "\n\tPrimary Transmissions by Risk");
				for (k = 0; k < SimContext::TRANSM_RISK_NUM; k++)
					fprintf(statsFile," \t%1.3lf", currTime->numTransmissionsRisk[k]);
			}
		} // end detailed CD4 and HVL outputs, ART toxicity outputs and transmission outputs if logging level is not SimContext::LONGIT_SUMM_MTH_BRF

		//output monthly TB stats (if tb module is enabled)
		if (simContext->getTBInputs()->enableTB){
			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::TB_NUM_STATES; j++)
				fprintf(statsFile, "\t%s", SimContext::TB_STATE_STRS[j]);
			fprintf(statsFile,"\n\tTB State");
			for (j = 0; j < SimContext::TB_NUM_STATES; j++)
				fprintf(statsFile, "\t%lu", currTime->numAliveTB[j]);

			fprintf (statsFile, "\n\tNum TB LTFU");
			fprintf(statsFile, "\t%1lu", currTime->numTBLTFU);

			fprintf(statsFile, "\n");
			for (int i = 0; i < SimContext::TB_NUM_UNFAVORABLE; i++)
				fprintf(statsFile,"\t%s",simContext->TB_UNFAVORABLE_STRS[i]);
			fprintf(statsFile, "\tNum with TB Unfavorable Outcome\tNum Deaths with TB Unfavorable Outcome");
			for(int i = 0; i < 2; i++){
				for( j = 0; j < 2; j++){
					for(int k = 0; k < 2; k++){
						for(int l = 0; l < 2; l++){
							fprintf(statsFile, "\n\t%s\t%s\t%s\t%s",i?"yes":"no",j?"yes":"no",k?"yes":"no",l?"yes":"no");
							fprintf(statsFile, "\t%1lu\t%1lu",currTime->numTBUnfavorableOutcome[i][j][k][l], currTime->numDeathsTBUnfavorableOutcome[i][j][k][l]);
						}
					}
				}
			}

			fprintf(statsFile, "\n\tTB Tracker");
			for (j = 0; j < SimContext::TB_NUM_TRACKER; j++)
				fprintf(statsFile, "\t%s", SimContext::TB_TRACKER_STRS[j]);

			for (k = 0; k < SimContext::HIV_CARE_NUM; k++){
				fprintf(statsFile, "\n\t%s", SimContext::HIV_CARE_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_TRACKER; j++){
					fprintf(statsFile, "\t%1lu", currTime->numAliveTBTrackerCare[j][k]);
				}
			}


			for (k = 0; k < SimContext::CD4_NUM_STRATA; k++){
				fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_TRACKER; j++){
					fprintf(statsFile, "\t%1lu", currTime->numHIVTBTrackerCD4[j][k]);
				}
			}

			fprintf(statsFile,"\n\t");
			fprintf(statsFile,"\n\tNum On TB Prophylaxis");
			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::TB_NUM_PROPHS; j++)
				fprintf(statsFile, "\tProph %d", j + 1);
			for( j = 0; j < SimContext::TB_NUM_STATES; ++j){
				fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[j]);
				for ( k = 0; k < SimContext::TB_NUM_PROPHS; ++k )
					fprintf(statsFile, "\t%lu", currTime->numOnTBProph[k][j]);
			}
			fprintf(statsFile, "\n\tTB Proph Minor Tox");
			for ( j = 0; j < SimContext::TB_NUM_PROPHS; ++j )
				fprintf(statsFile, "\t%lu", currTime->numTBProphMinorTox[j]);
			fprintf(statsFile, "\n\tTB Proph Major Tox");
			for ( j = 0; j < SimContext::TB_NUM_PROPHS; ++j )
				fprintf(statsFile, "\t%lu", currTime->numTBProphMajorTox[j]);
			fprintf(statsFile, "\n\tNum Increase Resistance from TB Proph");
			fprintf(statsFile, "\n\t");
			for (j = 0; j < SimContext::TB_NUM_PROPHS; ++j){
				fprintf(statsFile, "\tProph %d", j + 1);
			}	
			fprintf(statsFile, "\n\tdsTB to mdrTB");
			for ( int j = 0; j < SimContext::TB_NUM_PROPHS; ++j ){
				fprintf(statsFile, "\t%lu", currTime->numIncreaseResistanceDueToProph[SimContext::TB_STRAIN_DS][j]);
			}	
			fprintf(statsFile, "\n\tmdrTB to xdrTB");
			for (int j = 0; j < SimContext::TB_NUM_PROPHS; ++j ){
				fprintf(statsFile, "\t%lu",currTime->numIncreaseResistanceDueToProph[SimContext::TB_STRAIN_MDR][j]);
			}
			fprintf(statsFile, "\n\tNum TB Proph Line Completions");
			fprintf(statsFile, "\n\t");
			for (j = 0; j < SimContext::TB_NUM_PROPHS; j++)
				fprintf(statsFile, "\tProph %d", j + 1);
			for( j = 0; j < SimContext::TB_NUM_STATES; ++j){
				fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[j]);
				for ( k = 0; k < SimContext::TB_NUM_PROPHS; ++k )
					fprintf(statsFile, "\t%lu", currTime->numCompletedTBProph[k][j]);
			}
					
			fprintf(statsFile,"\n\tNum On TB Treatment");
			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\tTreatment %d", j);
			for(int i = 0; i < SimContext::TB_NUM_STATES; i++){
				fprintf(statsFile,"\n\t%s", SimContext::TB_STATE_STRS[i]);
				for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++){
					fprintf(statsFile, "\t%lu", currTime->numOnTBTreatmentByState[i][j]);
				}
			}	
			fprintf(statsFile, "\n\tTotal");	
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnTBTreatmentTotal[j]);
			fprintf(statsFile,"\n\tNum On Empiric TB Treatment");
			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\tTreatment %d", j);

			for(int i = 0; i < SimContext::TB_NUM_STATES; i++){
				fprintf(statsFile,"\n\t%s", SimContext::TB_STATE_STRS[i]);
				for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++){
					fprintf(statsFile, "\t%lu", currTime->numOnEmpiricTBTreatmentByState[i][j]);
				}
			}	
			fprintf(statsFile,"\n\tTotal");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnEmpiricTBTreatmentTotal[j]);
			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
				fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[j]);
			fprintf(statsFile, "\n\tNum With TB Strain");
			for ( k = 0; k < SimContext::TB_NUM_STRAINS; k++)
				fprintf(statsFile, "\t%lu", currTime->numTBStrain[k]);

			fprintf(statsFile, "\n\tTotal TB Incident Infections/Reinfections");
			for(j = 0; j < SimContext::TB_NUM_STATES; ++j ){
				fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[j]);
				for ( k = 0; k < SimContext::TB_NUM_STRAINS; k++)
					fprintf(statsFile, "\t%lu", currTime->numTBInfections[j][k]);
			}

			fprintf(statsFile, "\n\t");
			for ( j = 0; j < SimContext::TB_NUM_STRAINS; ++j )
				fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[j]);

			fprintf(statsFile, "\n\tTotal Activations (All) from Latent TB");
			for ( j = 0; j < SimContext::TB_NUM_STRAINS; ++j )
				fprintf(statsFile, "\t%lu", currTime->numTBReactivationsLatent[j]);

			fprintf(statsFile, "\n\tTotal Activations (Pulm) from Latent TB");
			fprintf(statsFile, "\n\tHIV Negative");
			for (j = 0; j < SimContext::TB_NUM_STRAINS; j++){
				fprintf(statsFile, "\t%lu", currTime->numTBReactivationsPulmLatentHIVNegative[j]);
			}
			for (k = 0; k < SimContext::CD4_NUM_STRATA; k++){
				fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_STRAINS; j++){
					fprintf(statsFile, "\t%lu", currTime->numTBReactivationsPulmLatentHIVPositive[k][j]);
				}
			}
			fprintf(statsFile, "\n\tTotal Activations (Extrapulm) from Latent TB");
			fprintf(statsFile, "\n\tHIV Negative");
			for (j = 0; j < SimContext::TB_NUM_STRAINS; j++){
				fprintf(statsFile, "\t%lu", currTime->numTBReactivationsExtraPulmLatentHIVNegative[j]);
			}
			for (k = 0; k < SimContext::CD4_NUM_STRATA; k++){
				fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_STRAINS; j++){
					fprintf(statsFile, "\t%lu", currTime->numTBReactivationsExtraPulmLatentHIVPositive[k][j]);
				}
			}

			fprintf(statsFile, "\n\tTotal Relapses (All)");
			for ( j = 0; j < SimContext::TB_NUM_STRAINS; ++j )
				fprintf(statsFile, "\t%lu", currTime->numTBRelapses[j]);
			fprintf(statsFile, "\n\tTotal Relapses (Pulm)");
			for ( j = 0; j < SimContext::TB_NUM_STRAINS; ++j )
				fprintf(statsFile, "\t%lu", currTime->numTBRelapsesPulm[j]);
			fprintf(statsFile, "\n\tTotal Relapses (Extrapulm)");
			for ( j = 0; j < SimContext::TB_NUM_STRAINS; ++j )
				fprintf(statsFile, "\t%lu", currTime->numTBRelapsesExtraPulm[j]);

			fprintf(statsFile,"\n\tNum With Observed TB Strain");
			fprintf(statsFile,"\n\tTrue Strain");
			for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
				fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[j]);
			fprintf(statsFile, "\tTB Uninfected");
			for (k = 0; k < SimContext::TB_NUM_STRAINS; k++){
				fprintf(statsFile, "\n\tObserved %s", SimContext::TB_STRAIN_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
					fprintf(statsFile, "\t%lu", currTime->numObservedTBByTrueTBStrain[k][j]);
				fprintf(statsFile, "\t%lu", currTime->numObservedTBUninfectedTB[k]);
			}

			fprintf(statsFile, "\n\tTB Pos Test Result");
			for (j = 0; j < SimContext::TB_NUM_TESTS; j++)
				fprintf(statsFile, "\tTest %d", j);
			for (k = 0; k < SimContext::TB_NUM_STATES; k++){
				fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_TESTS; j++){
					fprintf(statsFile, "\t%lu", currTime->numTBTestResults[j][k][SimContext::TB_DIAG_STATUS_POS]);
				}
			}

			fprintf(statsFile, "\n\tTB Neg Test Result");
			for (j = 0; j < SimContext::TB_NUM_TESTS; j++)
				fprintf(statsFile, "\tTest %d", j);
			for (k = 0; k < SimContext::TB_NUM_STATES; k++){
				fprintf(statsFile, "\n\t%s", SimContext::TB_STATE_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_TESTS; j++){
					fprintf(statsFile, "\t%lu", currTime->numTBTestResults[j][k][SimContext::TB_DIAG_STATUS_NEG]);
				}
			}

			fprintf(statsFile,"\n\tDST Test Result");
			fprintf(statsFile,"\n\tTrue Strain");
			for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
				fprintf(statsFile, "\t%s", SimContext::TB_STRAIN_STRS[j]);
			fprintf(statsFile, "\tTB Uninfected");
			for (k = 0; k < SimContext::TB_NUM_STRAINS; k++){
				fprintf(statsFile, "\n\tObserved %s", SimContext::TB_STRAIN_STRS[k]);
				for (j = 0; j < SimContext::TB_NUM_STRAINS; j++)
					fprintf(statsFile, "\t%lu", currTime->numDSTTestResultsByTrueTBStrain[k][j]);
				fprintf(statsFile, "\t%lu", currTime->numDSTTestResultsUninfectedTB[k]);
			}


			fprintf(statsFile, "\n\t");
			for (j = 0; j < SimContext::TB_NUM_STATES; j++)
				fprintf(statsFile, "\t%s", SimContext::TB_STATE_STRS[j]);
			fprintf(statsFile, "\n\tTB Diagnostic Result Pos");
			for (j = 0; j < SimContext::TB_NUM_STATES; j++){
				fprintf(statsFile, "\t%lu", currTime->numTBDiagnosticResults[j][1]);
			}
			fprintf(statsFile, "\n\tTB Diagnostic Result Neg");
			for (j = 0; j < SimContext::TB_NUM_STATES; j++){
				fprintf(statsFile, "\t%lu", currTime->numTBDiagnosticResults[j][0]);
			}


			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\tTreatment %d", j);

			fprintf(statsFile, "\n\tNum On Successful Treatment (All)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnSuccessfulTBTreatment[j]);
			fprintf(statsFile, "\n\tNum On Successful Treatment (Pulm)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnSuccessfulTBTreatmentPulm[j]);
			fprintf(statsFile, "\n\tNum On Successful Treatment (Extrapulm)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnSuccessfulTBTreatmentExtraPulm[j]);

			fprintf(statsFile, "\n\tNum On Failed Treatment (All)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnFailedTBTreatment[j]);
			fprintf(statsFile, "\n\tNum On Failed Treatment (Pulm)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnFailedTBTreatmentPulm[j]);
			fprintf(statsFile, "\n\tNum On Failed Treatment (Extrapulm)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numOnFailedTBTreatmentExtraPulm[j]);

			fprintf(statsFile, "\n\tNum Treatment Defaults (All)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numDefaultTBTreatment[j]);
			fprintf(statsFile, "\n\tNum Treatment Defaults (Pulm)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numDefaultTBTreatmentPulm[j]);
			fprintf(statsFile, "\n\tNum Treatment Defaults (Extrapulm)");
			for (j = 0; j < SimContext::TB_NUM_TREATMENTS; j++)
				fprintf(statsFile, "\t%lu", currTime->numDefaultTBTreatmentExtraPulm[j]);

			fprintf(statsFile, "\n\tNum Dropout TB Treatment:");

			for (int i = 0; i < SimContext::TB_NUM_STRAINS; ++i ) {
				fprintf(statsFile, "\n\tObsv %s", SimContext::TB_STRAIN_STRS[i]);
				for ( j = 0; j < SimContext::TB_NUM_TREATMENTS; ++j )
					fprintf(statsFile, "\t%lu", currTime->numDropoutTBTreatment[i][j]);
			}


			fprintf(statsFile,"\n\tTB Deaths (All)");
			fprintf(statsFile, "\t%lu", currTime->numDeathsTB);

			fprintf(statsFile,"\n\t\tPulm TB Deaths\tExtrapulm TB Deaths\tTB Deaths While TB LTFU");
			fprintf(statsFile, "\n\tHIV Negative");
			fprintf(statsFile, "\t%lu\t%lu\t%lu", 
				currTime->numDeathsTBPulmHIVNegative,currTime->numDeathsTBExtraPulmHIVNegative, currTime->numDeathsTBLTFUHIVNegative);
			for (k = 0; k < SimContext::CD4_NUM_STRATA; k++){
				fprintf(statsFile, "\n\t%s", SimContext::CD4_STRATA_STRS[k]);
				fprintf(statsFile, "\t%lu\t%lu\t%lu", currTime->numDeathsTBPulmHIVPositive[k], currTime->numDeathsTBExtraPulmHIVPositive[k], currTime->numDeathsTBLTFUHIVPositive[k]);
			}

			fprintf(statsFile,"\n\tAll Cause Deaths while on failed TB Treatment");
			fprintf(statsFile, "\t%lu", currTime->numAllDeathsWhileFailedTBTreatment);
			fprintf(statsFile,"\n\tTB Deaths while on failed TB Treatment");
			fprintf(statsFile, "\t%lu", currTime->numDeathsTBWhileFailedTBTreatment);

			fprintf(statsFile,"\n\tTB Monthly Costs");
			fprintf(statsFile, "\t%1.0lf", currTime->totalMonthlyTBCohortCosts);
		} //end if TB is enabled

		// output OI distribs for only this month (not cumulative)
		fprintf(statsFile,"\n\n\tOIs Distrib");
		for ( j = 0; j < SimContext::OI_NUM; ++j )
			fprintf(statsFile," \t%s", SimContext::OI_STRS[j]);
		fprintf(statsFile, "\tTotal");
		if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_MTH_BRF) {
			fprintf(statsFile,"\n\tTot OI evts:");
			for ( j = 0; j < SimContext::OI_NUM; ++j )
				fprintf(statsFile," \t%1lu", currTime->numPrimaryOIs[j] + currTime->numSecondaryOIs[j]);
			fprintf(statsFile,"\t%1lu", currTime->numOIsTotal);
		}
		else {
			fprintf(statsFile,"\n\tPrim OI evts:");
			for ( j = 0; j < SimContext::OI_NUM; ++j )
				fprintf(statsFile," \t%1lu", currTime->numPrimaryOIs[j]);
			fprintf(statsFile,"\t%1lu", currTime->numPrimaryOIsTotal);
			fprintf(statsFile,"\n\tSec OI evts:");
			for ( j = 0; j < SimContext::OI_NUM; ++j )
				fprintf(statsFile," \t%1lu", currTime->numSecondaryOIs[j]);
			fprintf(statsFile,"\t%1lu", currTime->numSecondaryOIsTotal);
			fprintf(statsFile,"\t# Tot OI evts:\t%1lu", currTime->numOIsTotal);
		}

		// output #patients with OI hists (is cumulative), and # w/o hist of any OI
		fprintf(statsFile,"\n\t# w.OIhist:");
		for ( j = 0; j < SimContext::OI_NUM; ++j )
			fprintf(statsFile," \t%1lu", currTime->numWithOIHistory[j]);
		fprintf(statsFile,"\t\t# No OI hist:\t%1lu", currTime->numWithoutOIHistory);
		if (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_MTH_BRF)
			continue;

		// output #patients with "first" OI during this period
		fprintf(statsFile,"\n\t# w.first OI:");
		for ( j = 0; j < SimContext::OI_NUM; ++j )
			fprintf(statsFile," \t%1lu", currTime->numWithFirstOI[j]);
		fprintf(statsFile,"\n\tDths from first OI:");
		for ( j = 0; j < SimContext::OI_NUM; ++j )
			fprintf(statsFile," \t%1lu", currTime->numDeathsFromFirstOI[j]);

		// output dth distribs for only this month (not cumulative)
		fprintf(statsFile,"\n\tDths Distrib");
		for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
			fprintf(statsFile," \t%s", SimContext::DTH_CAUSES_STRS[j]);
		if (simContext->getHIVTestInputs()->enableHIVTesting || simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile,"\n\tDth events:");
			for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
				fprintf(statsFile," \t%1lu", currTime->numDeathsType[j]);
			fprintf(statsFile,"\n\tTotal Dth Evts of HIV+:");
			for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
				fprintf(statsFile," \t%1lu", currTime->numHIVPosDeathsType[j]);
			for (int k = 0; k < SimContext::HIV_CARE_NUM; k++){
				fprintf(statsFile,"\n\tDeath evts while %s:",SimContext::HIV_CARE_STRS[k]);
				for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
					fprintf(statsFile," \t%1lu", currTime->numDeathsTypeCare[j][k]);
			}

		}
		else{
			fprintf(statsFile,"\n\tDth events:");
			for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
				fprintf(statsFile," \t%1lu", currTime->numDeathsType[j]);
			fprintf(statsFile,"\n\tDth evts while LTFU:");
			for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
				fprintf(statsFile," \t%1lu", currTime->numDeathsTypeCare[j][SimContext::HIV_CARE_LTFU]);
		}

		if (simContext->getCHRMsInputs()->showCHRMsOutput){
			//output dth distribs for those without CHRMs when they died (not cumulative)
			fprintf(statsFile,"\n\tPatients without CHRMs");
			fprintf(statsFile,"\n\tDths Distrib");
			for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
				fprintf(statsFile," \t%s", SimContext::DTH_CAUSES_STRS[j]);
			fprintf(statsFile," \t%s","Total");
			fprintf(statsFile,"\n\tNum Dths:");
			for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
				fprintf(statsFile," \t%1lu", currTime->numDeathsWithoutCHRMsType[j]);
			fprintf(statsFile," \t%1lu",currTime->numDeathsWithoutCHRMs);

			//output dth distribs for those with CHRMs when they died (not cumulative)
			fprintf(statsFile,"\n\tPatients with CHRMs");
			for(int i=0;i<SimContext::CHRM_NUM;i++){
				fprintf(statsFile,"\n\t%s",SimContext::CHRM_STRS[i]);
				fprintf(statsFile,"\n\tDths Distrib");
				for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
					fprintf(statsFile," \t%s", SimContext::DTH_CAUSES_STRS[j]);
				fprintf(statsFile," \t%s","Total");
				fprintf(statsFile,"\n\tNum Dths:");
				for ( j = 0; j < SimContext::DTH_NUM_CAUSES; ++j )
					fprintf(statsFile," \t%1lu", currTime->numDeathsWithCHRMsTypeCHRM[j][i]);
				fprintf(statsFile," \t%1lu",currTime->numDeathsWithCHRMsCHRM[i]);
			}
			fprintf(statsFile,"\n\t");
			for (j = 0; j < SimContext::CHRM_NUM; ++j)
				fprintf(statsFile, " \t%s",SimContext::CHRM_STRS[j]);
			fprintf(statsFile,"\n\tCHRMs Costs:");
			for (j = 0; j < SimContext::CHRM_NUM; ++j)
				fprintf(statsFile," \t%1.0lf", currTime->costsCHRMs[j]);
		} //end if CHRMs outputs are enabled

		// output cost distribs 
		fprintf(statsFile,"\n\tHIV Screening Costs\n\t\tHIVtests\tHIVmisc\tLab Stage Tests\tLab Stage Misc");

		fprintf(statsFile,"\n\tCurrent costs:\t%1.0lf \t%1.0lf \t%1.0lf\t%1.0lf", currTime->costsHIVTests, currTime->costsHIVMisc, currTime->costsLabStagingTests, currTime->costsLabStagingMisc);
		fprintf(statsFile, "\t\tPrEP Costs:\t%1.0lf", currTime->costsPrEP);	
		fprintf(statsFile,"\n\tCumulative costs:\t%1.0lf \t%1.0lf", currTime->cumulativeHIVTestingCosts, currTime->cumulativeHIVMiscCosts);

		if (simContext->getPedsInputs()->enablePediatricsModel && simContext->getEIDInputs()->enableHIVTestingEID){
			fprintf(statsFile, "\n\t\tEID Tests\tEID Misc");
			fprintf(statsFile,"\n\tEID costs:\t%1.0lf \t%1.0lf",
					currTime->costsEIDTests, currTime->costsEIDMisc);
			fprintf(statsFile, "\n\t\tDirect\tTox");
			fprintf(statsFile,"\n\tInfant HIV Proph Cost:\t%1.0lf \t%1.0lf",
					currTime->costsInfantHIVProphDirect, currTime->costsInfantHIVProphTox);
		}	

		fprintf(statsFile,"\n\t\tStartup\tMonthly");
		fprintf(statsFile,"\n\tIntervention costs:\t%1.0lf \t%1.0lf",
				currTime->costsInterventionStartup, currTime->costsInterventionMonthly);

		fprintf(statsFile, "\n\t\tCD4 Test Costs (All)\tHVL Test Costs (All)");
		fprintf(statsFile, "\n\tCurrent Cost Totals\t%1.0lf \t%1.0lf", currTime->costsCD4Testing, currTime->costsHVLTesting);

		fprintf(statsFile, "\n\tCumulative Cost Totals\t%1.0lf \t%1.0lf ",currTime->cumulativeCD4TestingCosts,
				currTime->cumulativeHVLTestingCosts);

		fprintf(statsFile,"\n\tClinic Visit Costs (Current %s)\t%1.2lf", (runSpecs->longitLoggingLevel == SimContext::LONGIT_SUMM_YR_DET)?"Year":"Month", currTime->costsClinicVisits);
		fprintf(statsFile, "\n\tCategorized Cost Totals\n\t\tDirectMedical\tDirectNonMedical\tTimeCosts\tIndirect");
		fprintf(statsFile, "\n\tCurrent Categorized Costs\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf",
			currTime->totalMonthlyCohortCostsType[0], currTime->totalMonthlyCohortCostsType[1],
			currTime->totalMonthlyCohortCostsType[2], currTime->totalMonthlyCohortCostsType[3]);
		fprintf(statsFile, "\n\tCumulative Categorized Costs\t%1.0lf\t%1.0lf\t%1.0lf\t%1.0lf",
			currTime->cumulativeCohortCostsType[0], currTime->cumulativeCohortCostsType[1],
			currTime->cumulativeCohortCostsType[2], currTime->cumulativeCohortCostsType[3]);	

		fprintf(statsFile,"\n\tProph Costs");
		for (j = 0; j < SimContext::OI_NUM; ++j)
			fprintf(statsFile," \t%s", SimContext::OI_STRS[j]);
		for (j = 0; j < SimContext::PROPH_NUM; ++j) {
			fprintf(statsFile,"\n\tProph %d", j + 1);
			for (k = 0; k < SimContext::OI_NUM; ++k)
				fprintf(statsFile," \t%1.0lf", currTime->costsProph[k][j]);
		}

		fprintf(statsFile,"\n\t");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile, " \tART %d", j + 1);
		fprintf(statsFile, " \tPre-ART \tPost-ART (Off ART)");
		fprintf(statsFile,"\n\tART Costs:");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%1.0lf", currTime->costsART[j]);
		fprintf(statsFile,"\n\tNum HIV+in_care (Starting this month):");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numStartingART[j]);
		fprintf(statsFile,"\t%lu",currTime->numStartingPreART);
		fprintf(statsFile, "\t%lu", currTime->numStartingPostART);
		fprintf(statsFile,"\n\tNum HIV+in_care (Total):");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numOnART[j]);
		fprintf(statsFile,"\t%lu",currTime->numInCarePreART);
		fprintf(statsFile, "\t%lu", currTime->numInCarePostART);
		fprintf(statsFile,"\n\tNum HIV+LTFU (Starting this month):");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numStartingLostToFollowUpART[j]);
		fprintf(statsFile," \t%lu", currTime->numStartingLostToFollowUpPreART);
		fprintf(statsFile," \t%lu", currTime->numStartingLostToFollowUpPostART);
		fprintf(statsFile,"\n\tNum HIV+LTFU (Total):");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numLostToFollowUpART[j]);
		fprintf(statsFile," \t%lu", currTime->numLostToFollowUpPreART);
		fprintf(statsFile," \t%lu", currTime->numLostToFollowUpPostART);
		fprintf(statsFile,"\n\tNum RTC (continue previous regimen):");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numReturnOnPrevART[j]);
		fprintf(statsFile," \t%lu", currTime->numReturnToCarePreART);
		fprintf(statsFile," \t%lu", currTime->numReturnToCarePostART);
		fprintf(statsFile,"\n\tNum RTC (switch regimen):");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numReturnOnNextART[j]);
		fprintf(statsFile,"\n\tNum Deaths while Lost:");
		for (j = 0; j < SimContext::ART_NUM_LINES; ++j)
			fprintf(statsFile," \t%lu", currTime->numDeathsWhileLostART[j]);
		fprintf(statsFile," \t%lu", currTime->numDeathsWhileLostPreART);
		fprintf(statsFile," \t%lu", currTime->numDeathsWhileLostPostART);

		if (simContext->getHIVTestInputs()->enableHIVTesting || simContext->getPedsInputs()->enablePediatricsModel){
			fprintf(statsFile, "\n\n\tNum Deaths HIV+ (at least one clinic visit):\t%lu", currTime->numDeathsHIVPosHadClinicVisit);
			fprintf(statsFile, "\n\tNum Deaths HIV+ (never visited clinic):\t%lu", currTime->numDeathsHIVPosNeverVisitedClinic);
			fprintf(statsFile, "\n\tNum Deaths HIV-:\t%lu", currTime->numDeathsUninfected);
		}

		//output current and cumulative total cohort costs
		fprintf(statsFile, "\n\n\t\tCumulative ART Costs");
		fprintf(statsFile, "\n\tTotal (up to this month)\t%1.0lf", currTime->cumulativeARTCosts);
		fprintf(statsFile,"\n\t\tTotal Cohort Costs");
		fprintf(statsFile, "\n\tCurrent Total\t%1.2lf", currTime->totalMonthlyCohortCosts);
		fprintf(statsFile, "\n\tCumulative Total\t%1.2lf", currTime->cumulativeCohortCosts);
	}
} /* end writeTimeSummaries */

/** \brief writeOrphanStats outputs the orphan statistics to the orphan output file */
void RunStats::writeOrphanStats() {
	int i, j, k;

	for (vector<OrphanStats *>::iterator t = orphanStats.begin(); t != orphanStats.end(); t++) {
		OrphanStats *currTime = *t;
		fprintf(orphanFile,"\nORPHAN SUMMARY FOR MONTH %d", currTime->timePeriod);
		fprintf(orphanFile,"\nOrphan Age (Years)\t# Incident Orphans");
		for (i = 0; i < SimContext::CHRM_ORPHANS_OUTPUT_AGE_CAT_NUM; i++)
			fprintf(orphanFile,"\n%d\t%d", i, currTime->numOrphansAge[i]);
		fprintf(orphanFile,"\nTotal\t%d", currTime->numOrphans);
	}


} /* end writeOrphanStats */

