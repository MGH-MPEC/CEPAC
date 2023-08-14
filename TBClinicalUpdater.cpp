#include "include.h"

/** \brief Constructor takes in the patient object */
TBClinicalUpdater::TBClinicalUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
TBClinicalUpdater::~TBClinicalUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void TBClinicalUpdater::performInitialUpdates() {
	/** First calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	//Set everyone to not in TB Care
	setTBCareState(SimContext::TB_CARE_UNLINKED);

	setInitialTBTreatmentState();
	resetTBTesting(true, true, true);

	/** Set the patient to have no observed TB drug resistance strain or history thereof */
	setObservedTBResistanceStrain(false);
	setObservedHistTBResistanceStrain(false);
	/** Set the initial TB proph state to not be on or scheduled for TB proph */
	setInitialTBProphState();

	/** Identify the first available TB proph line */
	bool hasNext = false;
	int prophIndex = SimContext::NOT_APPL;
	int prophNum =  SimContext::NOT_APPL;
	for (int i = 0; i < SimContext::TB_NUM_PROPHS; i++) {
		if (simContext->getTBInputs()->prophOrder[i]!= SimContext::NOT_APPL) {
			hasNext = true;
			prophIndex = i;
			prophNum = simContext->getTBInputs()->prophOrder[i];
			break;
		}
	}

	setNextTBProph(hasNext, prophIndex, prophNum);
	setTBDSTTestResultPickup(false, SimContext::TB_STRAIN_DS, -1);

	//Roll for eligibility for interval intitation policy
	double randNum = CepacUtil::getRandomDouble(140080, patient);
	if (randNum < simContext->getTBInputs()->TBDiagnosticsInitIntervalProb)
		setTBInitPolicyIntervalEligible(true);
	else
		setTBInitPolicyIntervalEligible(false);

} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month**/
void TBClinicalUpdater::performMonthlyUpdates() {
	if (!simContext->getTBInputs()->enableTB)
		return;

	if (simContext->getTBInputs()->allowMultipleTests){
		bool continueTesting = true;

		while(continueTesting){
			continueTesting = performTBTestingUpdates();
		}
	}
	else
		performTBTestingUpdates();
	checkForStoppingEmpiricTherapy();
	performTBTreatmentUpdates();
	performTBLTFUUpdates();
	checkForTreatmentDefault();
	performTBProphProgramUpdates();
	performTBTreatmentCostsUpdates();


} /* end performMonthlyUpdates */

/** \brief performTBTestingUpdates determines eligibility and performs diagnostics TB tests
 * returns True if testing loop should be continued (only if multiple tests in same month are allowed)**/
bool TBClinicalUpdater::performTBTestingUpdates() {
	if (!simContext->getTBInputs()->enableTBDiagnostics)
		return false;

	bool continueLoop = false;

	//Check to see if month 0 and should start in TB Treatment automatically
	double randNum = CepacUtil::getRandomDouble(100130, patient);
	double probStartInTreat = simContext->getTBInputs()->TBDiagnosticsInitInTreatmentHIVPos[patient->getTBState()->currTrueTBDiseaseState];
	if(patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG){
		probStartInTreat = simContext->getTBInputs()->TBDiagnosticsInitInTreatmentHIVNeg[patient->getTBState()->currTrueTBDiseaseState];
	}
	if (patient->getGeneralState()->monthNum == patient->getGeneralState()->initialMonthNum && randNum < probStartInTreat){
		//Diagnose them with TB and link them to care - I'm not thrilled with this function but it's the best option for managing this right now given the time pressure, may revisit this and some other TB module functions another time
		performTBDiagnosis(SimContext::NOT_APPL, true);
		//Set Observed Strain
		SimContext::TB_STRAIN tbStrain = SimContext::TB_STRAIN_DS;
		if (patient->getTBState()->currTrueTBDiseaseState != SimContext::TB_STATE_UNINFECTED)
			tbStrain = patient->getTBState()->currTrueTBResistanceStrain;
		setObservedTBResistanceStrain(true, tbStrain);
		return false;
	} //end if this is Month 0 and patient should start already in TB care

	if (patient->getTBState()->careState == SimContext::TB_CARE_UNLINKED){
		bool startTesting = false;
		//If patient is unlinked to TB care and they have no ongoing tests check for eligibility to start testing
		if (patient->getTBState()->currTestIndex == SimContext::NOT_APPL &&
			patient->getTBState()->nextTestIndex != SimContext::NOT_APPL)
				startTesting = evaluateStartTBDiagnostics();

		//Start a new testing chain
		if (startTesting){
			startTBTesting();
		}

		//If there are no pending results and they have a curr test do curr test
		if (!patient->getTBState()->hasPendingResult && patient->getTBState()->currTestIndex != SimContext::NOT_APPL){
			//Determine the test num of curr test
			int currTestNum = simContext->getTBInputs()->TBDiagnosticsTestOrder[patient->getTBState()->everOnTreatmentOrEmpiricStartChain][patient->getTBState()->currTestIndex];
			if (currTestNum != SimContext::NOT_APPL){
				bool doDST = simContext->getTBInputs()->TBDiagnosticsTestOrderDST[patient->getTBState()->everOnTreatmentOrEmpiricStartChain][patient->getTBState()->currTestIndex];
				continueLoop = performTBTest(currTestNum, doDST);
			}
		}

		//Check for results
		if (patient->getTBState()->hasPendingResult && patient->getTBState()->monthOfResultPickup <= patient->getGeneralState()->monthNum){
			bool willPickupResult = patient->getTBState()->willPickupResult;
			bool resetTests = patient->getTBState()->resetTestsOnPickup;
			int resultTestNum = patient->getTBState()->currPendingResultTestNum;
			bool willDoDST = patient->getTBState()->willDoDST;
			SimContext::TB_DIAG_STATUS result = patient->getTBState()->currPendingResult;

			//Remove pending result
			setTBTestResultPickup(false, -1, false, SimContext::TB_DIAG_STATUS_NEG, -1, false, false);
			int currIndex = patient->getTBState()->currTestIndex;
			int nextIndex = patient->getTBState()->nextTestIndex;

			//Patient will pickup test
			if (willPickupResult){
				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d TB TESTING PICKING UP RESULTS, index: %d, result: %s, reset tests: %s;\n", patient->getGeneralState()->monthNum,
						currIndex,
						SimContext::TB_DIAG_STATUS_STRS[result],
						resetTests?"yes":"no");
				}

				//if reset test, do not count result and set curr test to first test
				if (resetTests){
					continueLoop = false;
					resetTBTesting(false);
				}
				else{
					//Update result of test
					setTBTestResult(result, currIndex);

					if (result == SimContext::TB_DIAG_STATUS_POS){
						if(resultTestNum >=0){
							//Do DST here if DST not linked
							//Only do DST here if we have no pending DST results
							SimContext::TBInputs::TBTest tbTest = simContext->getTBInputs()->TBTests[resultTestNum];
							if (willDoDST && !tbTest.DSTLinked && !patient->getTBState()->hasPendingDSTResult){
								//Add costs DST test
								incrementCostsTBTest(0.0, tbTest.DSTCost, resultTestNum);

								//Check to see if DST will be picked up
								randNum = CepacUtil::getRandomDouble(100140, patient);
								if (randNum < tbTest.DSTProbPickup){
									//Will Pickup DST
									SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS;
									if (patient->getTBState()->currTrueTBDiseaseState != SimContext::TB_STATE_UNINFECTED){
										/** Draw the observed TB Strain */
										SimContext::TB_STRAIN trueStrain = patient->getTBState()->currTrueTBResistanceStrain;
										randNum = CepacUtil::getRandomDouble(140020, patient);
										for (int j = 0; j < SimContext::TB_NUM_STRAINS; j++) {
											if ((tbTest.DSTMatrixObsvTrue[j][trueStrain] > 0) &&
												(randNum < tbTest.DSTMatrixObsvTrue[j][trueStrain])) {
												obsvStrain = (SimContext::TB_STRAIN) j;
												break;
											}
											randNum -= tbTest.DSTMatrixObsvTrue[j][trueStrain];
										}
									}
									updateDSTResults(obsvStrain);
									int monthOfPickup = patient->getGeneralState()->monthNum + tbTest.DSTMonthsToResult[obsvStrain];
									setTBDSTTestResultPickup(true, obsvStrain, monthOfPickup);
									if (patient->getGeneralState()->tracingEnabled)
										tracer->printTrace(1, "**%d TB DST TEST %d, pickup in month:%d, result:%s;\n", patient->getGeneralState()->monthNum,
												resultTestNum,
												monthOfPickup,
												SimContext::TB_STRAIN_STRS[obsvStrain]);

								}
								else{
									if (patient->getGeneralState()->tracingEnabled)
										tracer->printTrace(1, "**%d TB DST TEST %d, will not pick up;\n", patient->getGeneralState()->monthNum,
												resultTestNum);
								}
							}
						}	

						//Roll for starting empiric therapy on positive result
						if (!patient->getTBState()->isOnEmpiricTreatment && !patient->getTBState()->hadTreatmentMajorTox){
							int currTestNum = simContext->getTBInputs()->TBDiagnosticsTestOrder[patient->getTBState()->everOnTreatmentOrEmpiricStartChain][currIndex];
							SimContext::TBInputs::TBTest tbTest = simContext->getTBInputs()->TBTests[currTestNum];
							double randNum = CepacUtil::getRandomDouble(100140, patient);
							double probEmpiric = 0;
							if (!patient->getMonitoringState()->isDetectedHIVPositive){
								if(patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
									probEmpiric = tbTest.probEmpiricPositiveResultSymptomaticHIVNeg;
								else
									probEmpiric = tbTest.probEmpiricPositiveResultAsymptomaticHIVNeg;
							}
							//Undetected HIV uses same probabilities as HIV-negative but true CD4 is still used so that the decisions will not rely solely on CD4 tests
							else{
								SimContext::CD4_STRATA currCD4Strata = patient->getDiseaseState()->currTrueCD4Strata;
								if(patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
									probEmpiric = tbTest.probEmpiricPositiveResultSymptomaticHIVPos[currCD4Strata];
								else
									probEmpiric = tbTest.probEmpiricPositiveResultAsymptomaticHIVPos[currCD4Strata];
							}
							if (randNum < probEmpiric){
								//Roll for which line to start
								SimContext::TB_STRAIN empiricStrain = SimContext::TB_STRAIN_DS;
								if (patient->getTBState()->hasHistObservedTBResistanceStrain){
									if (patient->getTBState()->observedHistTBResistanceStrain == SimContext::TB_STRAIN_MDR){
										randNum = CepacUtil::getRandomDouble(100140, patient);
										if (randNum < simContext->getTBInputs()->probEmpiricWithObservedHistMDR)
											empiricStrain = SimContext::TB_STRAIN_MDR;
									}
									else if(patient->getTBState()->observedHistTBResistanceStrain == SimContext::TB_STRAIN_XDR){
										randNum = CepacUtil::getRandomDouble(100140, patient);
										if (randNum < simContext->getTBInputs()->probEmpiricWithObservedHistXDR)
											empiricStrain = SimContext::TB_STRAIN_XDR;
									}
								}
								startEmpiricTBTreatment(simContext->getTBInputs()->empiricTreatmentNum[empiricStrain]);
							}
						}
					} //end if positive result is returned
					else{
						//Roll for stopping empiric thrapy on negative result
						if (patient->getTBState()->isOnEmpiricTreatment){
							int currTestNum = simContext->getTBInputs()->TBDiagnosticsTestOrder[patient->getTBState()->everOnTreatmentOrEmpiricStartChain][currIndex];
							SimContext::TBInputs::TBTest tbTest = simContext->getTBInputs()->TBTests[currTestNum];
							double randNum = CepacUtil::getRandomDouble(100140, patient);
							double probStopEmpiric = 0;

							if(patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
								probStopEmpiric = tbTest.probStopEmpiricSymptomatic;
							else
								probStopEmpiric = tbTest.probStopEmpiricAsymptomatic;
							if (randNum < probStopEmpiric)
								stopEmpiricTBTreatment();
						}
					} //end if negative result is returned

					//Determine if next test should be done based on test sequence matrix
					bool doNext = false;
					if (nextIndex != SimContext::NOT_APPL){
						if (currIndex == 0){
							if (simContext->getTBInputs()->TBDiagnosticSequenceMatrix2Tests[patient->getTBState()->testResults[0]])
								doNext = true;
						}
						else if (currIndex == 1){
							if (simContext->getTBInputs()->
							TBDiagnosticSequenceMatrix3Tests[patient->getTBState()->testResults[0]]
							[patient->getTBState()->testResults[1]])
								doNext = true;
						}
						else if (currIndex == 2){
							if (simContext->getTBInputs()->
							TBDiagnosticSequenceMatrix4Tests[patient->getTBState()->testResults[0]]
							[patient->getTBState()->testResults[1]]
							[patient->getTBState()->testResults[2]])
								doNext = true;
						}
					}
					//If we are doing next test update what the next test should be
					if (doNext){
						setCurrTBTestIndex(nextIndex);
						if (nextIndex != SimContext::NOT_APPL && nextIndex + 1 < SimContext::TB_DIAG_TEST_ORDER_NUM &&
								simContext->getTBInputs()->TBDiagnosticsTestOrder[patient->getTBState()->everOnTreatmentOrEmpiricStartChain][nextIndex+1] != SimContext::NOT_APPL)
							setNextTBTestIndex(nextIndex +1, false);
						else{
							setNextTBTestIndex(SimContext::NOT_APPL, false);
						}
					}
					else{
						//End of chain so interpret results
						performTBDiagnosis(currIndex);
						continueLoop = false;
					}
				}
			}//end will pickup result
			else{
				continueLoop = false;
				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d TB TESTING FAILIURE TO PICK UP RESULTS, index: %d, reset tests: %s;\n", patient->getGeneralState()->monthNum,
						currIndex,
						resetTests?"yes":"no");
				}

				//will not pickup result
				if (resetTests){
					resetTBTesting(false);
				}
			}
		}//end has pending result
		else{
			continueLoop = false;
		}
	}

	checkPendingDSTResults();

	return continueLoop;

} /* end performTBTestingUpdates */

/** \brief checkForStoppingEmpiricTherapy stops empiric therapy if duration is exceeded */
void TBClinicalUpdater::checkForStoppingEmpiricTherapy(){
	if (!simContext->getTBInputs()->enableTBDiagnostics)
		return;
	if (patient->getTBState()->careState != SimContext::TB_CARE_UNLINKED)
		return;

	// Evaluate treatment changes for empiric therapy if intended duration has been exceeded
	if (patient->getTBState()->isOnEmpiricTreatment) {
		int monthsTreat = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfEmpiricTreatmentStart + patient->getTBState()->previousEmpiricTreatmentDuration;
		int treatNum = patient->getTBState()->currEmpiricTreatmentNum;
		SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];

		//Check to see if time to stop treatment
		if (monthsTreat >= tbTreat.totalDuration) {
			bool isSuccess = patient->getTBState()->empiricTreatmentSuccess;

			SimContext::TB_STATE tbState = patient->getTBState()->currTrueTBDiseaseState;
			//Patients who complete a full duration on treatment in the Latent state transition to the Uninfected TB state. They are still flagged as having a history of TB. Note that this does not occur with TB prophyalxis, only with TB treatment (regular or empiric).
			if(tbState == SimContext::TB_STATE_LATENT && patient->getTBState()->monthOfTBStateChange <= patient->getTBState()->monthOfEmpiricTreatmentStart){
				setTBDiseaseState(SimContext::TB_STATE_UNINFECTED, false, SimContext::TB_INFECT_INITIAL, tbState);
			}
			//Change TB state if successful.  Those who are active move to the prev treated state.  Those in other states stay where they are.
			if (isSuccess){
				if (tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM || tbState == SimContext::TB_STATE_ACTIVE_PULM)
					setTBDiseaseState(SimContext::TB_STATE_PREV_TREATED, false, SimContext::TB_INFECT_INITIAL, tbState);
				//Remove their observed strain status
				setObservedTBResistanceStrain(false);

				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d TB EMPIRIC TREAT FINISHED SUCCESSFULLY;\n", patient->getGeneralState()->monthNum);
				}
			}
			else{
				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d TB EMPIRIC TREAT FINISHED UNSUCCESSFULLY;\n", patient->getGeneralState()->monthNum);
				}
				if (tbState != SimContext::TB_STATE_UNINFECTED){
					//If failed treatment roll for increased resistance
					SimContext::TB_STRAIN trueStrain = patient->getTBState()->currTrueTBResistanceStrain;
					double randNum = CepacUtil::getRandomDouble(60210, patient);
					if ((randNum < simContext->getTBInputs()->TBTreatmentProbResistAfterFail[treatNum]) &&
						(trueStrain < SimContext::TB_STRAIN_XDR)) {
							increaseTBDrugResistance(true);
							SimContext::TB_STRAIN newTBStrain = patient->getTBState()->currTrueTBResistanceStrain;
							if (patient->getGeneralState()->tracingEnabled) {
								tracer->printTrace(1, "**%d TB EMPIRIC TREAT INCR RESIST TO %s;\n",
									patient->getGeneralState()->monthNum, SimContext::TB_STRAIN_STRS[newTBStrain]);
							}
					}
				}
			}

			stopEmpiricTBTreatment(true);
		} //end if the duration is complete
	} //end if the patient is on empiric treatment
} /* end checkForStoppingEmpiricTherapy */

/** \brief checkPendingDSTResults process the results of any pending DST tests */
void TBClinicalUpdater::checkPendingDSTResults() {
	SimContext::TB_STRAIN prevObsvStrain = SimContext::TB_STRAIN_DS;
	if(patient->getTBState()->hasObservedTBResistanceStrain){
		prevObsvStrain = patient->getTBState()->currObservedTBResistanceStrain;
	}
	//Check for DST results
	if (patient->getTBState()->hasPendingDSTResult && patient->getTBState()->monthOfDSTResultPickup <= patient->getGeneralState()->monthNum){
		setObservedTBResistanceStrain(true, patient->getTBState()->currPendingDSTResult);
		setTBDSTTestResultPickup(false, SimContext::TB_STRAIN_DS, -1);
		// Output tracing if enabled
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d TB DST RESULT PICKUP, Obsv: %s;\n", patient->getGeneralState()->monthNum,
				SimContext::TB_STRAIN_STRS[patient->getTBState()->currObservedTBResistanceStrain]);
		}
		SimContext::TB_STRAIN newObsvStrain = patient->getTBState()->currObservedTBResistanceStrain;
		//perform treatment updates if the observed strain has changed
		if(patient->getTBState()->careState == SimContext::TB_CARE_IN_CARE){
			if(newObsvStrain != prevObsvStrain){
				//If patient is scheduled for regular treatment or already on treatment, update line if necessary
				int lineNum = SimContext::TB_NUM_TREATMENTS - 1;
				double randNum = CepacUtil::getRandomDouble(60195, patient);
				bool treathist = patient->getTBState()->everOnTreatmentOrEmpiric;
				for (int i = 0; i < SimContext::TB_NUM_TREATMENTS; i++) {
					if (randNum < simContext->getTBInputs()->TBTreatmentProbInitialLine[treathist][newObsvStrain][i]) {
						lineNum = i;
						break;
					}
					randNum -= simContext->getTBInputs()->TBTreatmentProbInitialLine[treathist][newObsvStrain][i];
				}
				if(patient->getTBState()->isScheduledForTreatment && lineNum != patient->getTBState()->nextTreatmentNum){
					scheduleNextTBTreatment(lineNum, patient->getTBState()->monthOfTreatmentStart);
				}
				else if(patient->getTBState()->isOnTreatment && lineNum != patient->getTBState()->currTreatmentNum){
					int previousDuration = 0;
					//Treatment for a more resistant strain also treats less resistant strains
					if(newObsvStrain < prevObsvStrain){
						previousDuration = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentStart;
					}
					stopCurrTBTreatment(false, false);
					scheduleNextTBTreatment(lineNum, patient->getGeneralState()->monthNum, false, previousDuration);
					startNextTBTreatment();
				}
			}
		}	

		//If on empiric treatment, ensure that they are on the correct line for their new observed strain. Even if the observed strain has not changed, they no longer need to face a probability to get the correct line. 
		if (patient->getTBState()->isOnEmpiricTreatment){
			int currTreatNum = patient->getTBState()->currEmpiricTreatmentNum;
			int treatmentStrainIndex;

			for (treatmentStrainIndex = SimContext::TB_STRAIN_DS; treatmentStrainIndex < SimContext::TB_NUM_STRAINS; treatmentStrainIndex++){
				if (currTreatNum == simContext->getTBInputs()->empiricTreatmentNum[treatmentStrainIndex])
					break;
			}

			int correctTreatNum = simContext->getTBInputs()->empiricTreatmentNum[newObsvStrain];
			if(correctTreatNum != currTreatNum){
				//if the incorrect regimen was for a more resistant strain, time spent on the incorrect regimen counts towards the duration on the correct regimen; otherwise, they must complete the full duration
				int previousDuration = 0;
				
				if (treatmentStrainIndex > (int) newObsvStrain)
					previousDuration = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfEmpiricTreatmentStart;
				stopEmpiricTBTreatment();
				startEmpiricTBTreatment(correctTreatNum, previousDuration);
			}
		}
	}
} /* end checkPendingDSTResults */

/** \brief performTBTreatmentUpdates handles treatment of TB for those in TB care */
void TBClinicalUpdater::performTBTreatmentUpdates() {
	if (patient->getTBState()->careState != SimContext::TB_CARE_IN_CARE)
		return;
	checkPendingDSTResults();	

	//If patient is not on treatment or not scheduled for treatment start them on initial line
	if (!patient->getTBState()->isOnTreatment){
		if(!patient->getTBState()->isScheduledForTreatment){
			SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS;
	if (patient->getTBState()->hasObservedTBResistanceStrain)
		obsvStrain = patient->getTBState()->currObservedTBResistanceStrain;
		// Roll for initial line
		int lineNum = SimContext::TB_NUM_TREATMENTS - 1;
		double randNum = CepacUtil::getRandomDouble(60190, patient);
		bool treathist = patient->getTBState()->everOnTreatmentOrEmpiric;
		for (int i = 0; i < SimContext::TB_NUM_TREATMENTS; i++) {
			if (randNum < simContext->getTBInputs()->TBTreatmentProbInitialLine[treathist][obsvStrain][i]) {
				lineNum = i;
				break;
			}
			randNum -= simContext->getTBInputs()->TBTreatmentProbInitialLine[treathist][obsvStrain][i];
		}
		//determine lag to treatment start
		double timeLagMean = simContext->getTBInputs()->monthsToTreatmentMean;
		double timeLagStdDev = simContext->getTBInputs()->monthsToTreatmentStdDev;
		int timeLag = (int) (CepacUtil::getRandomGaussian(timeLagMean, timeLagStdDev, 60200, patient) + 0.5);
		if (timeLag < 0)
			timeLag = 0;

		//Schedule initial treatment
		scheduleNextTBTreatment(lineNum, patient->getGeneralState()->monthNum + timeLag);
	}
	}
	// Evaluate treatment changes if on treatment and the intended duration has been exceeded
	else{
		int monthsTreat = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentStart + patient->getTBState()->previousTreatmentDuration;

		int treatNum = patient->getTBState()->currTreatmentNum;
		SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];

		//Check to see if time to stop treatment
		if (monthsTreat >= tbTreat.totalDuration) {
			bool isSuccess = patient->getTBState()->treatmentSuccess;

			SimContext::TB_STATE tbState = patient->getTBState()->currTrueTBDiseaseState;
			//Patients who complete a full duration on treatment in the Latent state transition to the Uninfected TB state. They are still flagged as having a history of TB. Note that this does not occur with TB prophyalxis, only with TB treatment (regular or empiric).
			if(tbState == SimContext::TB_STATE_LATENT && patient->getTBState()->monthOfTBStateChange <= patient->getTBState()->monthOfTreatmentStart){
				setTBDiseaseState(SimContext::TB_STATE_UNINFECTED, false, SimContext::TB_INFECT_INITIAL, tbState);
			}
			//Change TB state if successful.  Those who are active move to the prev treated state.  Those in other states stay where they are.
			if (isSuccess){
				//Remove them from the in care health state
				setTBCareState(SimContext::TB_CARE_UNLINKED);
				if (tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM || tbState == SimContext::TB_STATE_ACTIVE_PULM)
					setTBDiseaseState(SimContext::TB_STATE_PREV_TREATED, false, SimContext::TB_INFECT_INITIAL, tbState);
				//Remove their observed strain status
				setObservedTBResistanceStrain(false);

				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d TB TREAT FINISHED SUCCESSFULLY;\n", patient->getGeneralState()->monthNum);
				}
			}
			else{
				// Output tracing if enabled
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d TB TREAT FINISHED UNSUCCESSFULLY;\n", patient->getGeneralState()->monthNum);
				}
				if (tbState != SimContext::TB_STATE_UNINFECTED){
					//If failed treatment roll for increased resistance
					SimContext::TB_STRAIN trueStrain = patient->getTBState()->currTrueTBResistanceStrain;
					double randNum = CepacUtil::getRandomDouble(60210, patient);
					if ((randNum < simContext->getTBInputs()->TBTreatmentProbResistAfterFail[treatNum]) &&
						(trueStrain < SimContext::TB_STRAIN_XDR)) {
							increaseTBDrugResistance(true);
							SimContext::TB_STRAIN newTBStrain = patient->getTBState()->currTrueTBResistanceStrain;
							if (patient->getGeneralState()->tracingEnabled) {
								tracer->printTrace(1, "**%d TB TREAT INCR RESIST TO %s;\n",
									patient->getGeneralState()->monthNum, SimContext::TB_STRAIN_STRS[newTBStrain]);
							}
					}
				}
				//Determine next line and schedule treatment
				bool willRepeat = false;
				//Check to see if should repeat same line
				if (patient->getTBState()->numRepeatCurrTreatment < simContext->getTBInputs()->TBTreatmentMaxRepeats[treatNum]){
					double randNum = CepacUtil::getRandomDouble(60210, patient);
					if (randNum < simContext->getTBInputs()->TBTreatmentProbRepeatLine[treatNum])
						willRepeat = true;
				}

				if (willRepeat)
					scheduleNextTBTreatment(treatNum, patient->getGeneralState()->monthNum, true);
				else{
					int nextLine = tbTreat.nextTreatNumNormalFail;
					if (nextLine != SimContext::NOT_APPL){
						scheduleNextTBTreatment(nextLine, patient->getGeneralState()->monthNum, false);
					}
					else{
						//Remove them from the in care health state if they have no next line
						setTBCareState(SimContext::TB_CARE_UNLINKED);
						//Remove their observed strain status
						setObservedTBResistanceStrain(false);
					}
				}
			}
			stopCurrTBTreatment(true, isSuccess);
		} //end if duration is complete
		else{
			//Check for early observed failure
			if (!patient->getTBState()->treatmentSuccess){
				//Roll for observing failure early
				double randNum = CepacUtil::getRandomDouble(60210, patient);
				if (randNum < tbTreat.probObsvEarlyFail){
					//Roll for whether early failure was observed using a TB test
					randNum = CepacUtil::getRandomDouble(60211, patient);
					if(randNum < tbTreat.probEarlyFailObservedWithTBTest){
						// Output tracing if enabled
						if (patient->getGeneralState()->tracingEnabled) {
							tracer->printTrace(1, "**%d EARLY TB TREATMENT FAILURE OBSERVED WITH TB TEST;\n", patient->getGeneralState()->monthNum);
						}
						incrementCostsTBTreatment(tbTreat.costObsvEarlyFailTBTest, treatNum);
					}
					else{
						// Output tracing if enabled
						if (patient->getGeneralState()->tracingEnabled) {
							tracer->printTrace(1, "**%d EARLY TB TREATMENT FAILURE OBSERVED;\n", patient->getGeneralState()->monthNum);
						}
					}
					
					//Roll for switch lines
					randNum = CepacUtil::getRandomDouble(60212, patient);
					
					if (randNum < tbTreat.probSwitchEarlyFail){
						//determine next line and switch - note that stopCurrTBTreatment() will not be called before they start the next line
						int nextLine = tbTreat.nextTreatNumEarlyFail;
						if (nextLine != SimContext::NOT_APPL){
							scheduleNextTBTreatment(nextLine, patient->getGeneralState()->monthNum, false);
						}
					}
				}
			}
		}
	}

	//Check to see if treatment is scheduled to start this month
	if (patient->getTBState()->isScheduledForTreatment &&
			(patient->getGeneralState()->monthNum >= patient->getTBState()->monthOfTreatmentStart))
		startNextTBTreatment();

} /* end performTBTreatmentUpdates */

/** \brief performTBLTFUUpdates handles LTFU and RTC in cases which are not handled by an integrated HIV/TB clinic */
void TBClinicalUpdater::performTBLTFUUpdates() {
	if (!simContext->getTBInputs()->useTBLTFU)
		return;
	if (simContext->getTBInputs()->isIntegrated && patient->getMonitoringState()->careState >= SimContext::HIV_CARE_IN_CARE)
		return;
	if (patient->getTBState()->careState == SimContext::TB_CARE_UNLINKED)
		return;
	SimContext::TB_STATE tbState = patient->getTBState()->currTrueTBDiseaseState;
	//If patient is LTFU from TB roll for RTC
	if (patient->getTBState()->careState == SimContext::TB_CARE_LTFU){
		//Check if patient has exceded max months to remain LTFU
		int monthsLTFU = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfLTFU;
		if (monthsLTFU > simContext->getTBInputs()->maxMonthsLTFU){
			setTBRTC(true);
			return;
		}

		double probRTC;
		if (patient->getDiseaseState()->infectedHIVState == SimContext::HIV_INF_NEG)
			probRTC = simContext->getTBInputs()->probRTCHIVNeg[tbState];
		else
			probRTC = simContext->getTBInputs()->probRTCHIVPos[tbState];

		double randNum = CepacUtil::getRandomDouble(80060, patient);
		if (randNum < probRTC)
			setTBRTC();
	}
	else{
		int stage = 0;
		if(patient->getTBState()->isOnTreatment){
			int treatNum = patient->getTBState()->currTreatmentNum;
			if((patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentStart + patient->getTBState()->previousTreatmentDuration) > simContext->getTBInputs()->TBTreatments[treatNum].stage1Duration)
				stage = 1;
		}		
		//Roll for going LTFU
		double probLTFU = simContext->getTBInputs()->probLTFU[stage][tbState];
		double randNum = CepacUtil::getRandomDouble(80060, patient);
		if (randNum < probLTFU)
			setTBLTFU();
	}


} /* end performTBLTFUUpdates */


/** \brief performTBProphProgramUpdates evaluates TB proph policies and alters the treatment program */
void TBClinicalUpdater::performTBProphProgramUpdates() {
	if (!simContext->getTBInputs()->enableTB)
		return;
	if (patient->getTBState()->careState != SimContext::TB_CARE_UNLINKED)
		return;

	// If patient is on proph, evaluate if they should stop by calling evaluateStopTBProphPolicy() and update if so
	if (patient->getTBState()->isOnProph) {
		bool finishedProphLine = false;
		int prophDuration = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfProphStart;

		int currProphIndex = patient->getTBState()->currProphIndex;
		int currProphNum = patient->getTBState()->currProphNum;

		if(prophDuration >= simContext->getTBInputs()->prophDuration[currProphIndex] ){
			finishedProphLine = true;
			//roll for increased resistance if they had Active TB for the full duration
			if(patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM || patient->getTBState()->currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM){
				if(patient->getTBState()->monthOfTBStateChange <= patient->getTBState()->monthOfProphStart && patient->getTBState()->currTrueTBResistanceStrain < SimContext::TB_STRAIN_XDR){
					double randNum = CepacUtil::getRandomDouble(140070, patient);
					if (randNum < simContext->getTBInputs()->tbProphInputs[currProphNum].probResistanceInActiveStates) {
						increaseTBDrugResistance(false);
						SimContext::TB_STRAIN newTBStrain = patient->getTBState()->currTrueTBResistanceStrain;
						if (patient->getGeneralState()->tracingEnabled) {
							tracer->printTrace(1, "**%d TB PROPH INCR RESIST %s;\n",
							patient->getGeneralState()->monthNum, SimContext::TB_STRAIN_STRS[newTBStrain]);
						}
					}
				}	
			}
		}
			
		if (finishedProphLine || evaluateStopTBProphPolicy()){
			// Stop the TB proph
			stopCurrTBProph(finishedProphLine);

			// determine the next TB proph that is available for use
			bool hasNext = false;
			int nextProphIndex = SimContext::NOT_APPL;
			int nextProphNum = SimContext::NOT_APPL;
			if (!patient->getTBState()->hasMajorProphToxicity || simContext->getTBInputs()->moveToNextProphAfterTox){
				for (int i = currProphIndex + 1; i < SimContext::TB_NUM_PROPHS; i++) {
					if (simContext->getTBInputs()->prophOrder[i] != SimContext::NOT_APPL) {
						if(patient->getTBState()->numProphStarts[i] < simContext->getTBInputs()->maxRestarts[i]+1){
							hasNext = true;
							nextProphIndex = i;
							nextProphNum = simContext->getTBInputs()->prophOrder[i];
							break;
						}
					}
				}
			}
			setNextTBProph(hasNext, nextProphIndex, nextProphNum);
		}
	}

	// If proph available and patient is not on proph or scheduled, evaluate whether to start by calling TBClinicalUpdater::evaluateStartTBProphPolicy() and rolling for the lag to start time
	if (!patient->getTBState()->isOnProph && patient->getTBState()->hasNextProphAvailable && !patient->getTBState()->isScheduledForProph) {
		if (evaluateStartTBProphPolicy()) {
			bool isEligible = false;
			//Check for eligibility to get tb proph
			if (patient->getTBState()->hasRolledEligibleForProph){
				isEligible = patient->getTBState()->isEligibleForProph;
			}
			else{
				// Roll for prob of receiving proph
				double probProph = 0.0;
				if (!patient->getMonitoringState()->isDetectedHIVPositive)
					probProph = simContext->getTBInputs()->probReceiveProphNotKnownPos;
				else if (patient->getARTState()->isOnART)
					probProph = simContext->getTBInputs()->probReceiveProphOnART;
				else
					probProph = simContext->getTBInputs()->probReceiveProphKnownPosOffART;
				double randNum = CepacUtil::getRandomDouble(60160, patient);
				if (randNum < probProph)
					isEligible = true;
				setTBProphEligibility(isEligible, true);
			}

			if (isEligible) {
				double lagTimeMean = simContext->getTBInputs()->monthsLagToStartProphMean;
				double lagTimeStdDev = simContext->getTBInputs()->monthsLagToStartProphStdDev;
				int lagTime = (int) (CepacUtil::getRandomGaussian(lagTimeMean, lagTimeStdDev, 60170, patient) + 0.5);
				if (lagTime < 0)
					lagTime = 0;
				scheduleNextTBProph(patient->getGeneralState()->monthNum + lagTime);
			}
		}
	}

	// Start proph if scheduled to do so and reached month of schedule
	if (patient->getTBState()->isScheduledForProph &&
		(patient->getGeneralState()->monthNum >= patient->getTBState()->monthOfProphStart)) {
			startNextTBProph();

			// Output tracing if enabled
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d START TB PROPH %d;\n",
					patient->getGeneralState()->monthNum,
					patient->getTBState()->currProphNum);
			}
	}

} /* end performTBProphProgramUpdates */

/** \brief performTBTreatmentCostsUpdates handles cost of untreated TB or costs related to tb treatment **/
void TBClinicalUpdater::performTBTreatmentCostsUpdates(){
	SimContext::TB_STATE tbState = patient->getTBState()->currTrueTBDiseaseState;

	if (patient->getTBState()->isOnEmpiricTreatment || patient->getTBState()->isOnTreatment){
		//Add costs related to tb treatment
		int monthOfTreatmentStart;
		const double *costArray;
		if (patient->getTBState()->isOnTreatment){
			monthOfTreatmentStart = patient->getTBState()->monthOfTreatmentStart;
		}
		else{
			monthOfTreatmentStart = patient->getTBState()->monthOfEmpiricTreatmentStart;
		}


		if ((patient->getGeneralState()->monthNum - monthOfTreatmentStart)%simContext->getTBInputs()->frequencyVisitCosts == 0){
			costArray = simContext->getTBInputs()->treatedCostsVisit;
			incrementCostsTBProviderVisit(costArray, 1.0);
		}

		if ((patient->getGeneralState()->monthNum - monthOfTreatmentStart)%simContext->getTBInputs()->frequencyMedCosts == 0){
			costArray = simContext->getTBInputs()->treatedCostsMed;
			incrementCostsTBMedVisit(costArray, 1.0);
		}

	}
	else{
		//Add costs of untreated tb if patient is active
		if (tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM || tbState == SimContext::TB_STATE_ACTIVE_PULM){
			const double *costArray = simContext->getTBInputs()->untreatedCosts;
			incrementCostsTBMisc(costArray, 1.0);
		}
	}


} /* end performTBTreatmentCostsUpdates */

/** \brief evaluateStartTBDiagnostics determines if the patient is eligible for TB Diagnostics **/
bool TBClinicalUpdater::evaluateStartTBDiagnostics(){
	bool hasPassedOneCriteria = false;
	bool hasFailedOneCriteria = false;

	if(patient->getTBState()->isOnEmpiricTreatment){
		return false;
	}

	// Check criteria based on TB symptoms first because these can override the post-treatment waiting period
	if (simContext->getTBInputs()->TBDiagnosticsInitPolicies[SimContext::TB_DIAG_INIT_SYMPTOMS]){
		if (patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS]){
			double randNum = CepacUtil::getRandomDouble(80060, patient);
			if (randNum < simContext->getTBInputs()->TBDiagnosticsInitSymptomsProb){
				hasPassedOneCriteria = true;
			}	
			else{
				hasFailedOneCriteria = true;
			}	
		}
		else{
				hasFailedOneCriteria = true;
		}
	}		
	// TB Symptoms criteria are the only ones which are checked during the post-treatment diagnostics hiatus
	if(patient->getTBState()->hasStoppedTreatmentOrEmpiric && (patient->getGeneralState()->monthNum - patient->getTBState()->monthOfTreatmentOrEmpiricStop) < simContext->getTBInputs()->TBDiagnosticsInitMinMthsPostTreat){
		return hasPassedOneCriteria;
	}

	//Upon HIV Diagnosis
	if (simContext->getTBInputs()->TBDiagnosticsInitPolicies[SimContext::TB_DIAG_INIT_HIV]){
		if (patient->getMonitoringState()->isDetectedHIVPositive && patient->getMonitoringState()->monthOfDetection == patient->getGeneralState()->monthNum)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}

	//With OI
	if (simContext->getTBInputs()->TBDiagnosticsInitPolicies[SimContext::TB_DIAG_INIT_OI]){
		if (patient->getDiseaseState()->hasCurrObservedOI)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}

	//Observed CD4
	if (simContext->getTBInputs()->TBDiagnosticsInitPolicies[SimContext::TB_DIAG_INIT_CD4]){
		if (patient->getMonitoringState()->hasObservedCD4){
			double currCD4 = patient->getMonitoringState()->currObservedCD4;
			if (currCD4 >= simContext->getTBInputs()->TBDiagnosticsInitCD4Bounds[0] && currCD4 <= simContext->getTBInputs()->TBDiagnosticsInitCD4Bounds[1])
				hasPassedOneCriteria = true;
			else
				hasFailedOneCriteria = true;
		}
	}

	//Calendar Month
	if (simContext->getTBInputs()->TBDiagnosticsInitPolicies[SimContext::TB_DIAG_INIT_MONTH]){
		if (patient->getGeneralState()->monthNum == simContext->getTBInputs()->TBDiagnosticsInitMonth)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}

	//Interval
	if (simContext->getTBInputs()->TBDiagnosticsInitPolicies[SimContext::TB_DIAG_INIT_INTERVAL]){

		int month = patient->getGeneralState()->monthNum;
		int stage = SimContext::TB_DIAG_INIT_POLICY_INTV_NUM-1;
		for(int i = 0; i < SimContext::TB_DIAG_INIT_POLICY_INTV_NUM-1; i++){
			if (month <= simContext->getTBInputs()->TBDiagnosticsInitIntervalBounds[i]){
				stage = i;
				break;
			}
		}

		int interval = simContext->getTBInputs()->TBDiagnosticsInitInterval[stage];

		if (patient->getTBState()->isEligibleTBInitPolicyInterval && ((patient->getGeneralState()->monthNum % interval) == 0))
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}

	// return true if using or evaluation and at least one criteria has been met,
	//	return true if using and evaluation and at least one criteria has been met and none have failed,
	//	return false otherwise
	if (simContext->getTBInputs()->TBDiagnosticsInitPoliciesUseOrEvaluation && hasPassedOneCriteria)
		return true;
	if (!simContext->getTBInputs()->TBDiagnosticsInitPoliciesUseOrEvaluation && hasPassedOneCriteria && !hasFailedOneCriteria)
		return true;

	return false;

} /* end evaluateStartTBDiagnostics */


/** \brief evaluateStartTBProphPolicy determines if the start criteria for TB proph has been met
 *
 * \return true if Patient meets the TB Prophylaxis starting criteria */
bool TBClinicalUpdater::evaluateStartTBProphPolicy() {
	// return false if there is not another available proph
	if (!patient->getTBState()->hasNextProphAvailable)
		return false;
	// return false if on TB treatment
	if ((patient->getTBState()->isOnTreatment))
		return false;
	if (patient->getTBState()->isOnEmpiricTreatment)
		return false;
	// return false if the patient is LTFU from HIV care at an integrated TB/HIV clinic and they can't start while HIV LTFU from the integrated clinic
	if(patient->getMonitoringState()->careState == SimContext::HIV_CARE_LTFU){
		if(simContext->getTBInputs()->useTBLTFU && simContext->getTBInputs()->isIntegrated && !simContext->getTBInputs()->allowTBProphStartWhileHIVLTFU ){
			return false;
		}
	}		
	bool hasPassedOneCriteria = false;
	bool hasFailedOneCriteria = false;

	//HIV Positive, Detected
	if (patient->getMonitoringState()->isDetectedHIVPositive){
		//All on proph criteria
		if (simContext->getTBInputs()->startProphAllKnownPos){
			hasPassedOneCriteria = true;
		}

		// Evaluate the current observed CD4 criteria
		if (patient->getMonitoringState()->hasObservedCD4) {
			double currCD4 = patient->getMonitoringState()->currObservedCD4;
			if ((simContext->getTBInputs()->startProphObservedCD4Bounds[SimContext::LOWER_BOUND] != SimContext::NOT_APPL) ||
				(simContext->getTBInputs()->startProphObservedCD4Bounds[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
					if ((currCD4 >= simContext->getTBInputs()->startProphObservedCD4Bounds[SimContext::LOWER_BOUND]) &&
						(currCD4 <= simContext->getTBInputs()->startProphObservedCD4Bounds[SimContext::UPPER_BOUND])) {
						hasPassedOneCriteria = true;
					}
					else {
						hasFailedOneCriteria = true;
					}
			}
		}

		// Evaluate ART initiation criteria
		if (simContext->getTBInputs()->startProphARTStatusKnownPos != SimContext::NOT_APPL) {
			bool requiredARTStatus = (bool) simContext->getTBInputs()->startProphARTStatusKnownPos;
			if (patient->getARTState()->isOnART == requiredARTStatus) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		// Evaluate history of TB diagnosis
		if (simContext->getTBInputs()->startProphHistTBDiagKnownPos != SimContext::NOT_APPL) {
			bool requiredHistStatus = (bool) simContext->getTBInputs()->startProphHistTBDiagKnownPos;
			bool hasHistTBPosDiag = (patient->getTBState()->monthOfTBPosDiagnosis != SimContext::NOT_APPL);
			if (hasHistTBPosDiag == requiredHistStatus) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}
		// Evaluate history of TB treatment completion
		if (simContext->getTBInputs()->startProphHistTreatmentKnownPos != SimContext::NOT_APPL) {
			bool requiredHistStatus = (bool) simContext->getTBInputs()->startProphHistTreatmentKnownPos;
			if (patient->getTBState()->everCompletedTreatmentOrEmpiric == requiredHistStatus) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		//Evaluate immune reactive criteria
		if (simContext->getTBInputs()->startProphImmuneReactiveKnownPos != SimContext::NOT_APPL){
			bool requiredImmuneStatus = (bool) simContext->getTBInputs()->startProphImmuneReactiveKnownPos;
			if (patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE] == requiredImmuneStatus){
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		// return true if using or evaluation and at least one criteria has been met,
		//	return true if using and evaluation and at least one criteria has been met and none have failed,
		//	return false otherwise
		if (simContext->getTBInputs()->startProphUseOrEvaluationKnownPos && hasPassedOneCriteria)
			return true;
		if (!simContext->getTBInputs()->startProphUseOrEvaluationKnownPos && hasPassedOneCriteria && !hasFailedOneCriteria)
			return true;
	}
	//HIV Negatives and Undetected HIV+
	else{
		//All on proph criteria
		if (simContext->getTBInputs()->startProphAllNotKnownPos){
			hasPassedOneCriteria = true;
		}

		// Evaluate history of TB diagnosis
		if (simContext->getTBInputs()->startProphHistTBDiagNotKnownPos != SimContext::NOT_APPL) {
			bool requiredHistStatus = (bool) simContext->getTBInputs()->startProphHistTBDiagNotKnownPos;
			bool hasHistTBPosDiag = (patient->getTBState()->monthOfTBPosDiagnosis != SimContext::NOT_APPL);
			if (hasHistTBPosDiag == requiredHistStatus) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}
		// Evaluate history of active TB
		if (simContext->getTBInputs()->startProphHistTreatmentNotKnownPos != SimContext::NOT_APPL) {
			bool requiredHistStatus = (bool) simContext->getTBInputs()->startProphHistTreatmentNotKnownPos;
			if (patient->getTBState()->everCompletedTreatmentOrEmpiric == requiredHistStatus) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		//Evaluate immune reactive criteria
		if (simContext->getTBInputs()->startProphImmuneReactiveNotKnownPos!= SimContext::NOT_APPL) {
			bool requiredImmuneStatus = (bool) simContext->getTBInputs()->startProphImmuneReactiveNotKnownPos;
			if (patient->getTBState()->currTrueTBTracker[SimContext::TB_TRACKER_IMMUNE_REACTIVE] == requiredImmuneStatus){
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		// return true if using or evaluation and at least one criteria has been met,
		//	return true if using and evaluation and at least one criteria has been met and none have failed,
		//	return false otherwise
		if (simContext->getTBInputs()->startProphUseOrEvaluationNotKnownPos && hasPassedOneCriteria)
			return true;
		if (!simContext->getTBInputs()->startProphUseOrEvaluationNotKnownPos && hasPassedOneCriteria && !hasFailedOneCriteria)
			return true;
	}
	return false;


} /* end evaluateStartTBProphPolicy */

/** \brief evaluateStopTBProphPolicy determines if the stopping criteria for TB proph has been met
 *
 * \return true if Patient meets criteria for stopping TB prophylaxis
 **/
bool TBClinicalUpdater::evaluateStopTBProphPolicy() {
	// return true if patient is on tb treatment
	if (patient->getTBState()->isOnTreatment)
		return true;
	if (patient->getTBState()->isOnEmpiricTreatment)
		return true;

	bool hasPassedOneCriteria = false;
	bool hasFailedOneCriteria = false;

	//HIV Positive, Detected
	if (patient->getMonitoringState()->isDetectedHIVPositive){
		// Evaluate the current observed CD4 criteria
		if (patient->getMonitoringState()->hasObservedCD4) {
			double currCD4 = patient->getMonitoringState()->currObservedCD4;
			if ((simContext->getTBInputs()->stopProphObservedCD4Bounds[SimContext::LOWER_BOUND] != SimContext::NOT_APPL) ||
				(simContext->getTBInputs()->stopProphObservedCD4Bounds[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
					if ((currCD4 >= simContext->getTBInputs()->stopProphObservedCD4Bounds[SimContext::LOWER_BOUND]) &&
						(currCD4 <= simContext->getTBInputs()->stopProphObservedCD4Bounds[SimContext::UPPER_BOUND])) {
						hasPassedOneCriteria = true;
					}
					else {
						hasFailedOneCriteria = true;
					}
			}
		}

		// Evaluate months on proph criteria
		if (simContext->getTBInputs()->stopProphNumMonthsKnownPos != SimContext::NOT_APPL) {
			int monthsOnProph = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfProphStart;
			if (monthsOnProph >= simContext->getTBInputs()->stopProphNumMonthsKnownPos) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		//Evaluate stopping after TB diagnosis criteria
		if (simContext->getTBInputs()->stopProphAfterTBDiagKnownPos){
			if (patient->getGeneralState()->monthNum == patient->getTBState()->monthOfTBPosDiagnosis) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		//Evaluate major toxicity criteria
		if (simContext->getTBInputs()->stopProphMajorToxKnownPos){
			if (patient->getTBState()->hasMajorProphToxicity) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		// return true if using or evaluation and at least one criteria has been met,
		//	return true if using and evaluation and at least one criteria has been met and none have failed,
		//	return false otherwise
		if (simContext->getTBInputs()->stopProphUseOrEvaluationKnownPos && hasPassedOneCriteria)
			return true;
		if (!simContext->getTBInputs()->stopProphUseOrEvaluationKnownPos && hasPassedOneCriteria && !hasFailedOneCriteria)
			return true;
	}
	else{
		//HIV Negatives and Undetected HIV+

		// Evaluate months on proph criteria,
		if (simContext->getTBInputs()->stopProphNumMonthsNotKnownPos != SimContext::NOT_APPL) {
			int monthsOnProph = patient->getGeneralState()->monthNum - patient->getTBState()->monthOfProphStart;
			if (monthsOnProph >= simContext->getTBInputs()->stopProphNumMonthsNotKnownPos) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		//Evaluate stopping after TB diagnosis criteria
		if (simContext->getTBInputs()->stopProphAfterTBDiagNotKnownPos){
			if (patient->getGeneralState()->monthNum == patient->getTBState()->monthOfTBPosDiagnosis) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		//Evaluate major toxicity criteria
		if (simContext->getTBInputs()->stopProphMajorToxNotKnownPos){
			if (patient->getTBState()->hasMajorProphToxicity) {
				hasPassedOneCriteria = true;
			}
			else {
				hasFailedOneCriteria = true;
			}
		}

		// return true if using or evaluation and at least one criteria has been met,
		//	return true if using and evaluation and at least one criteria has been met and none have failed,
		//	return false otherwise
		if (simContext->getTBInputs()->stopProphUseOrEvaluationNotKnownPos && hasPassedOneCriteria)
			return true;
		if (!simContext->getTBInputs()->stopProphUseOrEvaluationNotKnownPos && hasPassedOneCriteria && !hasFailedOneCriteria)
			return true;
	}


	return false;
} /* end evaluateStopTBProphPolicy */
