#include "include.h"

/** \brief Constructor takes in the patient as a pointer */
HIVTestingUpdater::HIVTestingUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
HIVTestingUpdater::~HIVTestingUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void HIVTestingUpdater::performInitialUpdates() {
	/** First calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	//initialize variables
	resetNumMissedVisitsEID();
	setFalsePositiveStatus(false, false);
	setCanReceiveEID(true);
	setMostRecentPositiveEIDTest(false, -1, SimContext::EID_TEST_TYPE_BASE);
	setMostRecentNegativeEIDTest(false);
	setMissedEIDTest(false, -1);

	bool useHIVTesting = false;
	bool useEID = false;

	//Use either the HIVTest inputs or the EID inputs for initial states, depending on whether the Peds Module is enabled 
	if(simContext->getHIVTestInputs()->enableHIVTesting)
		useHIVTesting = true;
	if(simContext->getPedsInputs()->enablePediatricsModel && simContext->getEIDInputs()->enableHIVTestingEID){
		useHIVTesting = true;
		useEID = true;
	}

	/** If not using the testing module, set all HIV positive patients to detected and return */
	if (!useHIVTesting) {
		//Pediatric patients can be HIV negative if EID is not enabled; adult patients are all initailly chronic HIV+ and in care if HIVTest is not enabled
		
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
			setDetectedHIVState(false);
			setLinkedState(false);
		}
		else{
			setDetectedHIVState(true, SimContext::HIV_DET_INITIAL);
			setLinkedState(true, SimContext::HIV_DET_INITIAL);
		}
		return;
	}

	/** Start with adult HIV test inputs */
	if(!useEID){
		/** If using the adult testing module for initial states, determine if patient should enter the model detected */
		if (patient->getDiseaseState()->infectedHIVState != SimContext::HIV_INF_NEG){
			SimContext::HIV_POS hivPosState = patient->getDiseaseState()->infectedHIVPosState;
			double randNum = CepacUtil::getRandomDouble(100010, patient);
			if(randNum < simContext->getHIVTestInputs()->probHIVDetectionInitial[hivPosState]) {
				setDetectedHIVState(true, SimContext::HIV_DET_INITIAL);
				setLinkedState(true, SimContext::HIV_DET_INITIAL);
			}
			else{
				setDetectedHIVState(false);
				setLinkedState(false);	
			}
		}
		else {
			setDetectedHIVState(false);
			setLinkedState(false);
		}
		/** If adult patient is not detected as positive and linked, initialize the HIV testing freq and acceptance probability */
		if (!patient->getMonitoringState()->isLinked) {
			if(simContext->getHIVTestInputs()->HIVTestAvailable && patient->getGeneralState()->ageMonths >= simContext->getHIVTestInputs()->HIVRegularTestingStartAge){
				initRegularScreening();
			}
			else{
				scheduleHIVTest(false, SimContext::NOT_APPL);
			}
		} // end if patient is not yet linked to HIV care
	}
	// If Peds and EID modules are enabled, patients enter the model undetected 
	else{
		setDetectedHIVState(false);
		setLinkedState(false);
		// patients may transition to the adult HIV Testing module at the starting age for user-defined tests
		if(simContext->getHIVTestInputs()->enableHIVTesting){
			if(simContext->getHIVTestInputs()->HIVTestAvailable && patient->getGeneralState()->ageMonths >= simContext->getHIVTestInputs()->HIVRegularTestingStartAge){
				initRegularScreening();
			}
			else{
				scheduleHIVTest(false, SimContext::NOT_APPL);
			}	
		}	
	}
} /* end performInitialUpdates */

/** \brief setSimContext changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model
 * changes the testing frequency and acceptance probability based on the testing strategy of the new simContext
 *
 * \param newSimContext a pointer to a SimContext object that patient should switch calling inputs from
 **/
void HIVTestingUpdater::setSimContext(SimContext *newSimContext){
	/** First calls the parent function to switch the simContext */
	StateUpdater::setSimContext(newSimContext);

	/** Only update the testing strategy if there is a strategy to update and the patient is undetected. Since this function is no longer being called it has not been updated to check for a starting age or call initRegularScreening - would need to add that in if it ever were revived */
	if (this->simContext->getHIVTestInputs()->enableHIVTesting && !patient->getMonitoringState()->isDetectedHIVPositive){
		// Default HIV testing parameters to NOT_APPL
		int intervalTestIndex = 0;
		int acceptanceProbIndex = 0;
		bool hasNextTest = patient->getMonitoringState()->hasScheduledHIVTest;
		int monthNextTest = patient->getMonitoringState()->monthOfScheduledHIVTest;
		if (simContext->getHIVTestInputs()->HIVTestAvailable) {
			/** Set the HIV testing interval and time for next test */
			double randNum = CepacUtil::getRandomDouble(100020, patient);
			for (int i = 0; i < SimContext::HIV_TEST_FREQ_NUM; i++) {
				if ((randNum < simContext->getHIVTestInputs()->HIVTestingProbability[i]) &&
					(simContext->getHIVTestInputs()->HIVTestingProbability[i] > 0)) {
						if (simContext->getHIVTestInputs()->HIVTestingInterval[i] > 0) {
							hasNextTest = true;
							intervalTestIndex = i;
							/** If we're turning testing on for this patient, set monthNextTest to be randomly determined between current month and currMonth+testingInterval
							//0.5 is added because casting as int just takes the floor... adding 0.5 causes correct rounding */
							monthNextTest = patient->getGeneralState()->monthNum + ((int) (CepacUtil::getRandomDouble(100025, patient) * simContext->getHIVTestInputs()->HIVTestingInterval[i] + 0.5));
						}
						break;
				}
				randNum -= simContext->getHIVTestInputs()->HIVTestingProbability[i];
			}

			/** Set the HIV testing acceptance probability */
			randNum = CepacUtil::getRandomDouble(100030, patient);
			SimContext::HIV_EXT_INF extInfectedState;
			if (!patient->getMonitoringState()->isHighRiskForHIV && (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)) {
				extInfectedState = SimContext::HIV_EXT_INF_NEG_LO;
			}
			else {
				extInfectedState = (SimContext::HIV_EXT_INF) patient->getDiseaseState()->infectedHIVState;
			}
			for (int i = 0; i < SimContext::TEST_ACCEPT_NUM; i++) {
				if ((randNum < simContext->getHIVTestInputs()->HIVTestAcceptDistribution[extInfectedState][i]) &&
					(simContext->getHIVTestInputs()->HIVTestAcceptDistribution[extInfectedState][i] > 0)) {
						acceptanceProbIndex = i;
						break;
				}
				randNum -= simContext->getHIVTestInputs()->HIVTestAcceptDistribution[extInfectedState][i];
			}
		}
		setHIVTestingParams(intervalTestIndex, acceptanceProbIndex);
		scheduleHIVTest(hasNextTest, monthNextTest);
	}
}

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void HIVTestingUpdater::performMonthlyUpdates() {

	if (patient->getMonitoringState()->isLinked)
		return;
	if (patient->getPedsState()->isFalsePositiveLinked)
		return;

	bool useHIVTesting = false;
	bool useEID = false;

	//Use either the HIVTest inputs or the EID inputs, depending on whether the Peds Module is enabled
	if(simContext->getHIVTestInputs()->enableHIVTesting)
		useHIVTesting = true;
	if (simContext->getPedsInputs()->enablePediatricsModel){
		if(simContext->getEIDInputs()->enableHIVTestingEID){
			useHIVTesting = true;
			if(!simContext->getHIVTestInputs()->enableHIVTesting || patient->getGeneralState()->ageMonths < simContext->getHIVTestInputs()->HIVRegularTestingStartAge)
				useEID = true;
		}	
	}

	if (!useHIVTesting)
		return;

	if(patient->getGeneralState()->ageMonths == simContext->getHIVTestInputs()->HIVRegularTestingStartAge && !patient->getMonitoringState()->isLinked &&simContext->getHIVTestInputs()->HIVTestAvailable){
		// Regular screening should be initiated immediately before offering the first test to avoid giving user-defined tests to people who are underage
		initRegularScreening(false);
	}

	if (useEID){
		//This ordering of tests reduces the likelihood of people getting a base test when they need a confirmatory test due to a previous positive result
		performEIDResultReturnUpdates();
		performEIDFirstConfirmatoryTests();
		performEIDResultReturnUpdates();
		performEIDSecondConfirmatoryTests();
		performEIDResultReturnUpdates();

		performEIDScreeningUpdates();
		
		performEIDResultReturnUpdates();
		performEIDFirstConfirmatoryTests();
		performEIDResultReturnUpdates();
		performEIDSecondConfirmatoryTests();
		performEIDResultReturnUpdates();
	}
	else{
		/** perform the regular and background HIV screenings if patient is not already linked to care by calling HIVTestingUpdater::performRegularScreeningUpdates() and then if they are still not linked, call HIVTestingUpdater::performBackgroundScreeningUpdates() */
		performRegularScreeningUpdates();
		if (!patient->getMonitoringState()->isLinked) {
			performBackgroundScreeningUpdates();
		}
	}
} /* end performMonthlyUpdates */

/** \brief initRegularScreening sets an unlinked patient's user-defined testing interval and acceptance probability for the run if tests are available 
 * 	\param atInit a bool defaulting to true indicating whether this is called at initiation or later in the run when the patient reaches the starting age for user-defined tests
*/
void HIVTestingUpdater::initRegularScreening(bool atInit){
	// Default HIV testing month to NOT_APPL
	bool hasNextTest = false;
	int monthNextTest = SimContext::NOT_APPL;
	int intervalTestIndex = 0;
	int acceptanceProbIndex = 0;

	/** Set the HIV testing interval and time for next test */
	double randNum = CepacUtil::getRandomDouble(100020, patient);
	for (int i = 0; i < SimContext::HIV_TEST_FREQ_NUM; i++) {
		if ((randNum < simContext->getHIVTestInputs()->HIVTestingProbability[i]) &&
		(simContext->getHIVTestInputs()->HIVTestingProbability[i] > 0)) {
			if (simContext->getHIVTestInputs()->HIVTestingInterval[i] > 0) {
				hasNextTest = true;
				intervalTestIndex = i;
				if(atInit){
					monthNextTest = 0;
				}	
				else{
					monthNextTest = patient->getGeneralState()->monthNum;
				}
				break;
			}	
		}
		randNum -= simContext->getHIVTestInputs()->HIVTestingProbability[i];

	}	
	/** Determine their HIV testing acceptance probability for the run now */
	randNum = CepacUtil::getRandomDouble(100030, patient);
	SimContext::HIV_EXT_INF extInfectedState;
	if (!patient->getMonitoringState()->isHighRiskForHIV && (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)) {
		extInfectedState = SimContext::HIV_EXT_INF_NEG_LO;
	}
	else {
		extInfectedState = (SimContext::HIV_EXT_INF) patient->getDiseaseState()->infectedHIVState;
	}
	for (int i = 0; i < SimContext::TEST_ACCEPT_NUM; i++) {
		if ((randNum < simContext->getHIVTestInputs()->HIVTestAcceptDistribution[extInfectedState][i]) &&
		(simContext->getHIVTestInputs()->HIVTestAcceptDistribution[extInfectedState][i] > 0)) {
			acceptanceProbIndex = i;
			break;
		}
		randNum -= simContext->getHIVTestInputs()->HIVTestAcceptDistribution[extInfectedState][i];
	}
	setHIVTestingParams(intervalTestIndex, acceptanceProbIndex);
	scheduleHIVTest(hasNextTest, monthNextTest);
}

/** \brief performRegularScreeningUpdates determines if a screening occurs, if its accepted, if
	they are detected, and updates the associated state and statistics */
void HIVTestingUpdater::performRegularScreeningUpdates() {
	/** Return if this is not the month of the next test */
	if (!patient->getMonitoringState()->hasScheduledHIVTest ||
		(patient->getGeneralState()->monthNum < patient->getMonitoringState()->monthOfScheduledHIVTest))
			return;
	/** Return if patient is too old to get regular testing */
	if(simContext->getHIVTestInputs()->HIVRegularTestingStopAge >= 0 &&
			patient->getGeneralState()->ageMonths >= simContext->getHIVTestInputs()->HIVRegularTestingStopAge)
		return;

	SimContext::HIV_INF infectedState = patient->getDiseaseState()->infectedHIVState;
	SimContext::HIV_EXT_INF extInfectedState;
	if (!patient->getMonitoringState()->isHighRiskForHIV && (infectedState == SimContext::HIV_INF_NEG)) {
		extInfectedState = SimContext::HIV_EXT_INF_NEG_LO;
	}
	else {
		extInfectedState = (SimContext::HIV_EXT_INF) patient->getDiseaseState()->infectedHIVState;
	}

	/** Accrue the initial startup cost for all patients, regardless of test acceptance */
	if ((patient->getGeneralState()->monthNum == patient->getGeneralState()->initialMonthNum) || (patient->getGeneralState()->ageMonths == simContext->getHIVTestInputs()->HIVRegularTestingStartAge)) {
		double cost = simContext->getHIVTestInputs()->HIVTestInitialCost[extInfectedState];
		incrementCostsHIVTest(cost);
		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d HIV SCREENING STARTUP, $ %1.0lf\n",
				patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
		}
	}

	/** Check if scheduled testing is accepted by patient */
	double randNum = CepacUtil::getRandomDouble(100040, patient);
	double probAccept;
	if (patient->getMonitoringState()->hasPrEP){
		SimContext::HIV_BEHAV risk = patient->getMonitoringState()->isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
		probAccept = simContext->getHIVTestInputs()->PrEPHIVTestAcceptProb[risk];
	}
	else
		probAccept = patient->getMonitoringState()->acceptanceProbHIVTest;

	if (randNum < probAccept) {
		/** If test is accepted, accrue the cost of performing the test */
		double cost = simContext->getHIVTestInputs()->HIVTestCost[extInfectedState];
		incrementCostsHIVTest(cost);
		incrementNumHIVTests();

		/** Check if patient returns for test result; if so, roll for that result */
		randNum = CepacUtil::getRandomDouble(100050, patient);
		if (randNum < simContext->getHIVTestInputs()->HIVTestReturnProb[infectedState]) {
			/** Determine the test result */
			randNum = CepacUtil::getRandomDouble(100060, patient);
			if (randNum < simContext->getHIVTestInputs()->HIVTestPositiveProb[infectedState]) {
				/** Increment cost, set QOL, and update identified state for a positive test result */
				cost = simContext->getHIVTestInputs()->HIVTestPositiveCost[infectedState];
				incrementCostsHIVMisc(cost);
				accumulateQOLModifier(simContext->getHIVTestInputs()->HIVTestPositiveQOLModifier[infectedState]);
				/** Update statistics for the accepted and positive HIV test result */
				updateHIVTestingStats(true, true, true);

				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d HIV TEST ACCEPT, RETURN, %s POSITIVE, $ %1.0lf\n",
						patient->getGeneralState()->monthNum,
						(infectedState != SimContext::HIV_INF_NEG) ? "TRUE" : "FALSE",
						patient->getGeneralState()->costsDiscounted);
				}

				if (infectedState != SimContext::HIV_INF_NEG) {
					cost = simContext->getHIVTestInputs()->HIVTestDetectionCost[patient->getDiseaseState()->infectedHIVPosState];
					incrementCostsHIVMisc(cost);
					bool wasPrevDetected= patient->getMonitoringState()->isDetectedHIVPositive;
					if (wasPrevDetected)
						setDetectedHIVState(true, SimContext::HIV_DET_SCREENING_PREV_DET);
					else
						setDetectedHIVState(true, SimContext::HIV_DET_SCREENING);


					if (simContext->getHIVTestInputs()->CD4TestAvailable)
						performLabStagingUpdates(wasPrevDetected);
					else{
						if (wasPrevDetected)
							setLinkedState(true,SimContext::HIV_DET_SCREENING_PREV_DET);
						else
							setLinkedState(true, SimContext::HIV_DET_SCREENING);
						scheduleInitialClinicVisit();
					}
					return;
				}
			}
			else {
				/** Increment cost and set QOL for a negative test result */
				cost = simContext->getHIVTestInputs()->HIVTestNegativeCost[infectedState];
				incrementCostsHIVMisc(cost);
				accumulateQOLModifier(simContext->getHIVTestInputs()->HIVTestNegativeQOLModifier[infectedState]);

				/** Update statistics for the accepted and negative HIV test result */
				updateHIVTestingStats(true, true, false);

				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d HIV TEST ACCEPT, RETURN, %s NEGATIVE, $ %1.0lf\n",
						patient->getGeneralState()->monthNum,
						(infectedState == SimContext::HIV_INF_NEG) ? "TRUE" : "FALSE",
						patient->getGeneralState()->costsDiscounted);
				}
			}
		}  // end if patient returns for the test result
		else {
			/** Increment cost for patient not returning for test results */
			cost = simContext->getHIVTestInputs()->HIVTestNonReturnCost[extInfectedState];
			incrementCostsHIVMisc(cost);

			/** Update statistics for the accepted HIV test with no return for results */
			updateHIVTestingStats(true, false, false);

			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d HIV TEST ACCEPT, NON-RETURN, $ %1.0lf\n",
					patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
			}
		}
	}
	else {
		/** Update statistics for the refused HIV test */
		updateHIVTestingStats(false, false, false);

		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d HIV TEST NOT ACCEPTED, $ %1.0lf\n",
				patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
		}
	}

	/** Schedule the next HIV test */
	scheduleHIVTest(true, patient->getGeneralState()->monthNum + patient->getMonitoringState()->intervalHIVTest);

} /* end performRegularScreeningUpdates */

/** \brief performBackgroundScreeningUpdates handles whether patient is detected by background
	screening and updates the associated state and statistics */
void HIVTestingUpdater::performBackgroundScreeningUpdates() {
	/** Return if patient is not old enough for background testing */
	if (patient->getGeneralState()->ageMonths < simContext->getHIVTestInputs()->HIVBackgroundStartAge)
		return;

	SimContext::HIV_INF infectedState = patient->getDiseaseState()->infectedHIVState;
	SimContext::HIV_EXT_INF extInfectedState;
	if (!patient->getMonitoringState()->isHighRiskForHIV && (infectedState == SimContext::HIV_INF_NEG)) {
		extInfectedState = SimContext::HIV_EXT_INF_NEG_LO;
	}
	else {
		extInfectedState = (SimContext::HIV_EXT_INF) patient->getDiseaseState()->infectedHIVState;
	}

	/** Check if background testing is accepted by patient */
	double randNum = CepacUtil::getRandomDouble(100040, patient);
	if (randNum < simContext->getHIVTestInputs()->HIVBackgroundAcceptProb[extInfectedState]) {
		/** If test is accepted, accrue the cost of performing the test */
		double cost = simContext->getHIVTestInputs()->HIVBackgroundTestCost[extInfectedState];
		incrementCostsHIVMisc(cost);

		/** Check if patient returns for test result */
		randNum = CepacUtil::getRandomDouble(100050, patient);
		if (randNum < simContext->getHIVTestInputs()->HIVBackgroundReturnProb[extInfectedState]) {
			/** Determine the test result */
			randNum = CepacUtil::getRandomDouble(100060, patient);
			if (randNum < simContext->getHIVTestInputs()->HIVBackgroundPositiveProb[extInfectedState]) {
				/** Increment cost, , and update identified state for a positive test result */
				cost = simContext->getHIVTestInputs()->HIVBackgroundPositiveCost[extInfectedState];
				incrementCostsHIVMisc(cost);

				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d HIV BACKGROUND TEST ACCEPT, RETURN, %s POSITIVE, $ %1.0lf\n",
						patient->getGeneralState()->monthNum,
						(infectedState != SimContext::HIV_INF_NEG) ? "TRUE" : "FALSE",
						patient->getGeneralState()->costsDiscounted);
				}

				if (infectedState != SimContext::HIV_INF_NEG) {
					cost = simContext->getHIVTestInputs()->HIVTestDetectionCost[patient->getDiseaseState()->infectedHIVPosState];
					incrementCostsHIVMisc(cost);
					bool wasPrevDetected= patient->getMonitoringState()->isDetectedHIVPositive;
					if (wasPrevDetected)
						setDetectedHIVState(true, SimContext::HIV_DET_BACKGROUND_PREV_DET);
					else
						setDetectedHIVState(true, SimContext::HIV_DET_BACKGROUND);

					//Roll for linkage
					randNum = CepacUtil::getRandomDouble(100065, patient);
					if (randNum < simContext->getHIVTestInputs()->HIVBackgroundProbLinkage) {
						if (wasPrevDetected)
							setLinkedState(true,SimContext::HIV_DET_BACKGROUND_PREV_DET);
						else
							setLinkedState(true, SimContext::HIV_DET_BACKGROUND);
						scheduleInitialClinicVisit();

					}
					return;
				}
			}
			else {
				/** Increment cost for a negative test result */
				cost = simContext->getHIVTestInputs()->HIVBackgroundNegativeCost[extInfectedState];
				incrementCostsHIVMisc(cost);

				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d HIV BACKGROUND TEST ACCEPT, RETURN, %s NEGATIVE, $ %1.0lf\n",
						patient->getGeneralState()->monthNum,
						(infectedState == SimContext::HIV_INF_NEG) ? "TRUE" : "FALSE",
						patient->getGeneralState()->costsDiscounted);
				}
			}
		}
		else {
			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d HIV BACKGROUND TEST ACCEPT, NON-RETURN, $ %1.0lf\n",
					patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
			}
		}
	}
} /* end performBackgroundScreeningUpdates */

/** \brief performLabStagingUpdates handles whether a patient is given a cd4 test, if they accept and updates the associated state and statistics*/
void HIVTestingUpdater::performLabStagingUpdates(bool wasPrevDetected){
	// return if they are HIV negative or too young to be eligible - absolute CD4 counts only
	if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG || patient->getPedsState()->ageCategoryPediatrics < SimContext::PEDS_AGE_LATE)
		return;

	SimContext::HIV_POS hivPosState = patient->getDiseaseState()->infectedHIVPosState;

	/** Accrue the initial startup cost for all patients, regardless of test acceptance */
	if ((patient->getGeneralState()->monthNum == patient->getGeneralState()->initialMonthNum) || (patient->getGeneralState()->ageMonths == simContext->getHIVTestInputs()->HIVRegularTestingStartAge)) {
		double cost = simContext->getHIVTestInputs()->CD4TestInitialCost[hivPosState];
		incrementCostsLabStagingTest(cost);
		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d LAB STAGING STARTUP, $ %1.0lf\n",
				patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
		}
	}

	/** Check if scheduled testing is accepted by patient */
	double randNum = CepacUtil::getRandomDouble(100080, patient);
	if (randNum < simContext->getHIVTestInputs()->CD4TestAcceptProb[hivPosState]) {
		/** If test is accepted, accrue the cost of performing the test */
		double cost = simContext->getHIVTestInputs()->CD4TestCost[hivPosState];
		incrementCostsLabStagingTest(cost);
		incrementNumLabStagingTests();

		/** Check if patient returns for test result */
		randNum = CepacUtil::getRandomDouble(100090, patient);
		if (randNum < simContext->getHIVTestInputs()->CD4TestReturnProb[hivPosState]) {
			/** If performing a test, calculate the observed CD4 level using the standard deviation of testing error */
			double stdDevPerc = CepacUtil::getRandomGaussian(0,  simContext->getHIVTestInputs()->CD4TestStdDevPercentage, 50010, patient);
			double biasStdDevPerc = CepacUtil::getRandomGaussian(0 , simContext->getHIVTestInputs()->CD4TestBiasStdDevPercentage, 50013, patient);
			double cd4Value = patient->getDiseaseState()->currTrueCD4*(1+stdDevPerc+simContext->getHIVTestInputs()->CD4TestBiasMean)*(1+biasStdDevPerc);

			setObservedCD4(true, cd4Value, true);

			/**Increment cost for return for result**/
			double cost = simContext->getHIVTestInputs()->CD4TestReturnCost[hivPosState];
			incrementCostsLabStagingMisc(cost);

			/** Determine Linkage to Care */
			randNum = CepacUtil::getRandomDouble(100110, patient);
			if (randNum < simContext->getHIVTestInputs()->CD4TestLinkageProb[patient->getMonitoringState()->currObservedCD4Strata]){
				//patient linked to care
				updateLabStagingStats(true,true,true);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d LAB STAGING ACCEPT, RETURN, LINK: obsv CD4 %1.0lf, $ %1.0lf\n",
							patient->getGeneralState()->monthNum, patient->getMonitoringState()->currObservedCD4, patient->getGeneralState()->costsDiscounted);
				}
				if (wasPrevDetected)
					setLinkedState(true, SimContext::HIV_DET_SCREENING_PREV_DET);
				else
					setLinkedState(true, SimContext::HIV_DET_SCREENING);
				scheduleInitialClinicVisit();
				return;
			}
			else{
				updateLabStagingStats(true,true,false);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d LAB STAGING ACCEPT, RETURN, NON-LINK: obsv CD4 %1.0lf, $ %1.0lf\n",
							patient->getGeneralState()->monthNum, patient->getMonitoringState()->currObservedCD4, patient->getGeneralState()->costsDiscounted);
				}
			}
		}
		else {
			/** Increment cost for patient not returning for test results */
			double cost = simContext->getHIVTestInputs()->CD4TestNonReturnCost[hivPosState];
			incrementCostsLabStagingMisc(cost);
			updateLabStagingStats(true,false,false);
			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d LAB STAGING ACCEPT, NON-RETURN, $ %1.0lf\n",
					patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
			}
		}
	}
	else {
		updateLabStagingStats(false,false,false);
		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d LAB STAGING NOT ACCEPTED, $ %1.0lf\n",
				patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
		}
	}

	/** Schedule the next HIV test */
	scheduleHIVTest(true, patient->getGeneralState()->monthNum + patient->getMonitoringState()->intervalHIVTest);
}

/** \brief performEIDScreeningUpdates determines if a EID screening occurs, if its accepted, if
	they are detected, and updates the associated state and statistics */
void HIVTestingUpdater::performEIDScreeningUpdates() {
	if (patient->getMonitoringState()->isLinked)
		return;
	if (patient->getPedsState()->isFalsePositiveLinked)
		return;

	//determine if mother is known positive or not known to be positive
	SimContext::PEDS_MATERNAL_KNOWLEDGE maternalKnowledge;
	if (patient->getPedsState()->maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG && patient->getPedsState()->maternalStatusKnown)
		maternalKnowledge = SimContext::PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE;
	else
		maternalKnowledge = SimContext::PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE;

	int patientAge = patient->getGeneralState()->ageMonths;
	bool hasVisit = false;
	int assayNum;
	double probPresent;
	double probNonMaternal;
	bool isEIDVisit;
	bool reofferTest;

	//check if it is time for a EID test visit
	for (int i = 0; i < SimContext::EID_NUM_ASSAYS; i++){
		if (patientAge == simContext->getEIDInputs()->testingAdminAge[maternalKnowledge][i]){
			hasVisit = true;
			assayNum = simContext->getEIDInputs()->testingAdminAssay[maternalKnowledge][i];
			probPresent = simContext->getEIDInputs()->testingAdminProbPresent[maternalKnowledge][i];
			if (patient->getPedsState()->isMotherAlive)
				probNonMaternal = simContext->getEIDInputs()->testingAdminProbNonMaternalCaregiver[maternalKnowledge][i];
			else
				probNonMaternal = 1.0;
			isEIDVisit = simContext->getEIDInputs()->testingAdminIsEIDVisit[maternalKnowledge][i];
			reofferTest = simContext->getEIDInputs()->testingAdminReofferTestIfMissed[maternalKnowledge][i];

			if (patient->getGeneralState()->tracingEnabled)
				tracer->printTrace(1, "**%d %s visit scheduled\n", patient->getGeneralState()->monthNum, isEIDVisit?"EID":"Well Child");

			break;
		}
	}

	if (!hasVisit)
		return;

	//If this is an EID visit don't let people in who have exited EID system
	if (isEIDVisit && !patient->getPedsState()->canReceiveEID){
		if (assayNum != SimContext::NOT_APPL)
			setMissedEIDTest(true, assayNum);
		return;
	}

	/**patient has scheduled visit*/

	//Check if patient presents to visit
	//multiply prob of presenting by number of previous missed visits
	probPresent = probPresent * pow(simContext->getEIDInputs()->probMultMissedVisit,patient->getPedsState()->numMissedVistsEID);
	double randNum = CepacUtil::getRandomDouble(100120, patient);
	if (randNum >= probPresent){
		//patient does not present
		incrementNumMissedVisitsEID();
		if (patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d EID failed to present to visit\n", patient->getGeneralState()->monthNum);
		if (assayNum != SimContext::NOT_APPL)
			setMissedEIDTest(true, assayNum);
		return;
	}

	/** patient presented to visit */
	//Add cost of visit
	if(patientAge < SimContext::EID_COST_VISIT_NUM)
		incrementCostsEIDVisit(simContext->getEIDInputs()->costVisit[patientAge]);

	//Roll for non maternal caregiver
	bool isNonMaternal = false;
	randNum = CepacUtil::getRandomDouble(100123, patient);
	if (randNum < probNonMaternal)
		isNonMaternal = true;

	if (patient->getGeneralState()->tracingEnabled)
		tracer->printTrace(1, "**%d EID presented to visit, %s care giver\n", patient->getGeneralState()->monthNum, isNonMaternal?"non-maternal":"maternal");



	//if no test scheduled this visit, check if can make up previous visit tests
	if ( assayNum == SimContext::NOT_APPL){
		if (patient->getPedsState()->hasMissedEIDTest && reofferTest)
			performEIDTest(patient->getPedsState()->missedEIDTestBaseAssay, patient->getPedsState()->missedEIDTestBaseAssay, SimContext::EID_TEST_TYPE_BASE, SimContext::OI_NONE, isNonMaternal);
		return;
	}

	//If provider has knowledge of previous positive result continue the cascade from where they left off
	randNum = CepacUtil::getRandomDouble(100125, patient);
	if (randNum < simContext->getEIDInputs()->probKnowedgePriorResult && patient->getPedsState()->hasMostRecentPositiveEIDTest){
		int mostRecentAssay = patient->getPedsState()->mostRecentPositiveEIDTestBaseAssay;
		SimContext::EID_TEST_TYPE mostRecentType = patient->getPedsState()->mostRecentPositiveEIDTestType;
		SimContext::EIDInputs::EIDTest mostRecentTest = simContext->getEIDInputs()->EIDTests[mostRecentAssay];
		if (patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d EID provider used knowledge of previous test %d\n", patient->getGeneralState()->monthNum, mostRecentAssay);

		bool testScheduled = false;
		if (mostRecentType == SimContext::EID_TEST_TYPE_BASE && mostRecentTest.EIDFirstConfirmatoryTestAssay != SimContext::NOT_APPL){
			if(patient->getPedsState()->eidScheduledConfirmatoryTests.size() > 0) {
				for (vector<SimContext::EIDTestState>::const_iterator i = patient->getPedsState()->eidScheduledConfirmatoryTests.begin(); i != patient->getPedsState()->eidScheduledConfirmatoryTests.end(); i++){
					if(i->baseAssay == mostRecentAssay && i->testType == SimContext::EID_TEST_TYPE_FIRST_CONF){
						testScheduled = true;
						if (patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d Patient has a first confirmatory test scheduled for month %d\n", patient->getGeneralState()->monthNum, i->monthToReturn);
						break;
					}
				}
			}
			if(!testScheduled)
				performEIDTest(mostRecentAssay, mostRecentTest.EIDFirstConfirmatoryTestAssay, SimContext::EID_TEST_TYPE_FIRST_CONF, SimContext::OI_NONE, isNonMaternal);
		}
		else if (mostRecentType == SimContext::EID_TEST_TYPE_FIRST_CONF && mostRecentTest.EIDSecondConfirmatoryTestAssay != SimContext::NOT_APPL){
			if(patient->getPedsState()->eidScheduledConfirmatoryTests.size() > 0) {
				for (vector<SimContext::EIDTestState>::const_iterator i = patient->getPedsState()->eidScheduledConfirmatoryTests.begin(); i != patient->getPedsState()->eidScheduledConfirmatoryTests.end(); i++){
					if(i->baseAssay == mostRecentAssay && i->testType == SimContext::EID_TEST_TYPE_SECOND_CONF){
						testScheduled = true;
						if (patient->getGeneralState()->tracingEnabled)
			tracer->printTrace(1, "**%d Patient has a second confirmatory test scheduled for month %d\n", patient->getGeneralState()->monthNum, i->monthToReturn);
						break;
					}
				}
			}
			if(!testScheduled)
				performEIDTest(mostRecentAssay, mostRecentTest.EIDSecondConfirmatoryTestAssay, SimContext::EID_TEST_TYPE_SECOND_CONF, SimContext::OI_NONE, isNonMaternal);
		}
		else{
			//roll for linkage
			rollForEIDLinkage(mostRecentAssay, SimContext::OI_NONE);
		}
	}
	else{
		//do regular scheduled test
		performEIDTest(assayNum, assayNum, SimContext::EID_TEST_TYPE_BASE, SimContext::OI_NONE, isNonMaternal);
	}
}

