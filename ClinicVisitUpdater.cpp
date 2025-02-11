#include "include.h"

/** \brief Constructor takes in the patient as a pointer */
ClinicVisitUpdater::ClinicVisitUpdater(Patient *patient) :
	StateUpdater(patient)
{

}

/** \brief Destructor is empty, no cleanup required */
ClinicVisitUpdater::~ClinicVisitUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void ClinicVisitUpdater::performInitialUpdates() {

	/** First calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();

	/** Sets the type of clinic visits and available treatments for the patient */
	SimContext::CLINIC_VISITS visitType = SimContext::CLINIC_SCHED;
	SimContext::THERAPY_IMPL treatmentType = SimContext::THERAPY_IMPL_PROPH_ART;
	double randNum = CepacUtil::getRandomDouble(60010, patient);
	for (int i = 0; i < SimContext::CLINIC_VISITS_NUM; i++) {
		if ((simContext->getCohortInputs()->clinicVisitTypeDistribution[i] != 0) &&
			(randNum < simContext->getCohortInputs()->clinicVisitTypeDistribution[i])) {
				visitType = (SimContext::CLINIC_VISITS) i;
				break;
		}
		randNum -= simContext->getCohortInputs()->clinicVisitTypeDistribution[i];
	}
	randNum = CepacUtil::getRandomDouble(60020, patient);
	for (int i = 0; i < SimContext::THERAPY_IMPL_NUM; i++) {
		if ((simContext->getCohortInputs()->therapyImplementationDistribution[i] != 0) &&
			(randNum < simContext->getCohortInputs()->therapyImplementationDistribution[i])) {
				treatmentType = (SimContext::THERAPY_IMPL) i;
				break;
		}
		randNum -= simContext->getCohortInputs()->therapyImplementationDistribution[i];
	}
	setClinicVisitType(visitType, treatmentType);

	/** Set the initial ART state to not on ART and determine next available regimen */
	setInitialARTState();
	/** Set the initial proph state to not on any prophs and determine next available ones */
	setInitialProphState();


	/** Sets the patients baseline ART response propensity coefficient */
	double baselineMean = simContext->getHeterogeneityInputs()->propRespondBaselineLogitMean;
	double baselineStdDev = simContext->getHeterogeneityInputs()->propRespondBaselineLogitStdDev;
	double baselineCoeff = CepacUtil::getRandomGaussian(baselineMean, baselineStdDev, 60030, patient);
	setResponseBaseline(baselineCoeff);

	/** Set the patients CD4 response type */
	randNum = CepacUtil::getRandomDouble(60040, patient);
	SimContext::CD4_RESPONSE_TYPE responseType = SimContext::CD4_RESPONSE_1;
	for (int i = 0; i < SimContext::CD4_RESPONSE_NUM_TYPES; i++) {
		if ((simContext->getCohortInputs()->CD4ResponseTypeOnARTDistribution[i] != 0) &&
			(randNum < simContext->getCohortInputs()->CD4ResponseTypeOnARTDistribution[i])) {
				responseType = (SimContext::CD4_RESPONSE_TYPE) i;
				break;
		}
		randNum -= simContext->getCohortInputs()->CD4ResponseTypeOnARTDistribution[i];
	}
	setCD4ResponseType(responseType);

	/** Schedules the initial clinic visits and CD4/HVL test for detected HIV positive patients*/
	if (patient->getMonitoringState()->isDetectedHIVPositive) {
		scheduleInitialClinicVisit();
	}
	else {
		scheduleRegularClinicVisit(false);
		scheduleCD4Test(false);
		scheduleHVLTest(false);
	}
	scheduleEmergencyClinicVisit(SimContext::EMERGENCY_NONE);

	/** Sets the observed health state of the patient to unknown values */
	setObservedCD4(false);
	setObservedCD4Percentage(false);
	setObservedHVLStrata(false);
	resetClinicVisitState(true);

	/** Initialize LTFU state to not lost and STI state to not interrupted */
	setCurrLTFUState(SimContext::LTFU_STATE_NONE);
	setCurrSTIState(SimContext::STI_STATE_NONE);


	/** Identify the first available art line */
	bool hasNext = false;
	int nextRegimen = SimContext::NOT_APPL;
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		if (simContext->getARTInputs(i)) {
				hasNext = true;
				nextRegimen = i;
				break;
		}
	}
	setNextARTRegimen(hasNext, nextRegimen);


	/** Determine patient prophylaxis non compliance */
	randNum = CepacUtil::getRandomDouble(60060, patient);
	if (randNum < simContext->getCohortInputs()->OIProphNonComplianceRisk)
		setProphNonCompliance(true);
	else
		setProphNonCompliance(false);
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		/** Set the initial proph that is available for use */
		bool hasNext = false;
		int prophNum = SimContext::NOT_APPL;
		SimContext::PROPH_TYPE prophType = SimContext::PROPH_PRIMARY;
		if (patient->getDiseaseState()->hasTrueOIHistory[i])
			prophType = SimContext::PROPH_SECONDARY;

		for (int k = 0; k < SimContext::PROPH_NUM; k++) {
			const SimContext::ProphInputs *prophInput;
			if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				prophInput=simContext->getProphInputs(SimContext::PROPH_PRIMARY, i, k);
			}
			else{
				prophInput=simContext->getPedsProphInputs(SimContext::PROPH_PRIMARY, i, k);
			}


			if (prophInput) {

				hasNext = true;
				prophNum = k;
				break;
			}
		}

		setNextProph(hasNext, prophType, (SimContext::OI_TYPE) i, prophNum);

	}

} /* end performInitialUpdates */

/** \brief performMonthlyUpdates performs all of the state and statistics updates for a simulated month */
void ClinicVisitUpdater::performMonthlyUpdates() {

	/** Calls StateUpdater::willAttendClinicThisMonth() and only proceeds if it returns true */
	if (!willAttendClinicThisMonth())
		return;

	/** Calls ClinicVisitUpdater::performOIDetectionUpdates() to handle emergency visits for OIs and determine if OIs are observed */
	performOIDetectionUpdates();

	/** Accrue general costs for the clinic visit and increment number of visits */
	int cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
	SimContext::GENDER_TYPE gender = patient->getGeneralState()->gender;
	bool isRegularPaidVisit = false;

	if(patient->getMonitoringState()->hasRegularClinicVisit &&
			(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfRegularClinicVisit)) {
		isRegularPaidVisit = true;
		const double *costs = simContext->getCostInputs()->clinicVisitCostRoutine[patient->getGeneralState()->ageCategoryCost][gender][cd4Strata];
		incrementCostsClinicVisit(costs);
	}

	incrementNumClinicVisits();

	/** Print tracing for the clinic visit if enabled */
	if (patient->getGeneralState()->tracingEnabled) {
		if(!isRegularPaidVisit && (patient->getMonitoringState()->emergencyClinicVisitType != SimContext::EMERGENCY_NONE)){
			tracer->printTrace(1, "**%d EMERGENCY CLINIC VISIT, Triggered by: %s, $ %1.0lf;\n", 
			patient->getGeneralState()->monthNum, SimContext::EMERGENCY_TYPE_STRS[patient->getMonitoringState()->emergencyClinicVisitType], patient->getGeneralState()->costsDiscounted);
		}
		else{
			tracer->printTrace(1, "**%d CLINIC VISIT, $ %1.0lf;\n", 
			patient->getGeneralState()->monthNum, patient->getGeneralState()->costsDiscounted);
		}	
	}

	/** schedule the next regular clinic visit **/
	// if an emergency visit does not count as a regular visit, they will not schedule a new regular non-emergency appointment until they have attended the last scheduled non-emergency one **/
	if (simContext->getTreatmentInputs()->emergencyVisitIsNotRegularVisit) {
		if (patient->getMonitoringState()->hasRegularClinicVisit &&
			(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfRegularClinicVisit)) {
				scheduleRegularClinicVisit(true, patient->getGeneralState()->monthNum + simContext->getTreatmentInputs()->clinicVisitInterval);
		}
	}
	// if an emergency visit counts as a regular visit, always schedule their next regular non-emergency visit after the regular interval regardless of the reason for the current visit, potentially resulting in a postponement of their next regular non-emergency visit
	else {
		scheduleRegularClinicVisit(true, patient->getGeneralState()->monthNum + simContext->getTreatmentInputs()->clinicVisitInterval);
	}
	/** if clinic visit was an emergency one, update state to indicate that it occurred */
	if ((patient->getMonitoringState()->emergencyClinicVisitType != SimContext::EMERGENCY_NONE) &&
		(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfEmergencyClinicVisit)) {
			scheduleEmergencyClinicVisit(SimContext::EMERGENCY_NONE);
	}

	/** Evaluate ART and prophylaxis policies and make changes to the treatment programs by calling ClinicVisitUpdater::performARTProgramUpdates and ClinicVisitUpdater::performProphProgramUpdates */
	performARTProgramUpdates();

	performProphProgramUpdates();

	/** reset the state for events since the last clinic visit */
	resetClinicVisitState();

} /* end performMonthlyUpdates */

/** \brief setSimContext changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model.
 *
 * Also changes the "nextARTRegimen" based on the ART regimens of the new simContext
 * \param newSimContext a pointer to a SimContext which should now be used to determine future HIV progression
 **/
void ClinicVisitUpdater::setSimContext(SimContext *newSimContext){
	/** First call the parent function to switch to newSimContext */
	StateUpdater::setSimContext(newSimContext);

	/** determine next available regimen
	//Identify the current (if any) art line or the last ART line taken (if ever) */
	int potentialNextARTRegimenNum = 0;
	if (patient->getARTState()->hasTakenART){
		if (patient->getARTState()->isOnART){
			potentialNextARTRegimenNum = patient->getARTState()->currRegimenNum + 1;
		} else {
			potentialNextARTRegimenNum = patient->getARTState()->prevRegimenNum + 1;
		}
	}

	/** Identify the next available art line from the newSimContext
	// Need to do here or if there are new/different ART lines in the new simContext, the patient will ignore them */
	bool hasNext = false;
	int nextRegimen = SimContext::NOT_APPL;
	for (int i = potentialNextARTRegimenNum; i < SimContext::ART_NUM_LINES; i++) {
		if (simContext->getARTInputs(i)) {
				hasNext = true;
				nextRegimen = i;
				break;
		}
	}

	setNextARTRegimen(hasNext, nextRegimen);

	/** Identify current proph state and determine next available one for each OI */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		// Set the initial proph that is available for use
		bool hasNext = false;
		int prophNum = SimContext::NOT_APPL;
		SimContext::PROPH_TYPE prophType = SimContext::PROPH_PRIMARY;
		if (patient->getDiseaseState()->hasTrueOIHistory[i])
			prophType = SimContext::PROPH_SECONDARY;

		/** The first proph to look for -- will be 0 if the patient has never been on Proph or the current/last proph if patient *has* been on proph */
		int potentialNextProphNum = 0;
		if (patient->getProphState()->hasTakenProph[i][prophType]){
			potentialNextProphNum = patient->getProphState()->currProphNum[i];
		}
		for (int k = potentialNextProphNum; k < SimContext::PROPH_NUM; k++) {
			const SimContext::ProphInputs *prophInput;
			if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				prophInput=simContext->getProphInputs(SimContext::PROPH_PRIMARY, i, k);
			}
			else{
				prophInput=simContext->getPedsProphInputs(SimContext::PROPH_PRIMARY, i, k);
			}
			if (prophInput) {
				hasNext = true;
				prophNum = k;
				break;
			}
		}

		setNextProph(hasNext, prophType, (SimContext::OI_TYPE) i, prophNum);

	}
}

/** \brief performOIDetectionUpdates handles the emergency OI clinic visit and
	determines if the current and prior OIs are observed */
void ClinicVisitUpdater::performOIDetectionUpdates() {
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		bool isObserved = false;

		/** Determine if OIs are observed */
		if (patient->getDiseaseState()->hasCurrTrueOI && (patient->getDiseaseState()->typeCurrTrueOI == i)) {
			/** If Patient currently has acute OI, always count as observed */
			isObserved = true;
			setCurrObservedOI(true);
		}
		else if (!patient->getMonitoringState()->hadPrevClinicVisit && patient->getDiseaseState()->hasTrueOIHistory[i]) {
			/** If first clinic visit and patient has history of OI, roll for being observed */
			double randNum = CepacUtil::getRandomDouble(60070, patient);
			if (randNum < simContext->getTreatmentInputs()->probDetectOIAtEntry[i]) {
				isObserved = true;
			}
		}
		else if (patient->getMonitoringState()->hadPrevClinicVisit && (patient->getDiseaseState()->numTrueOIsSinceLastVisit[i] > 0)) {
			/** If subsequent clinic visit and patient had OI since last visit, roll for being observed */
			double randNum = CepacUtil::getRandomDouble(60080, patient);
			if (randNum < simContext->getTreatmentInputs()->probDetectOISinceLastVisit[i]) {
				isObserved = true;
			}
		}

		/** If OI is observed, increment number observed and output tracing */
		if (isObserved) {
			incrementNumObservedOIs((SimContext::OI_TYPE) i, 1);
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d OBSV OI %s;\n",
					patient->getGeneralState()->monthNum, SimContext::OI_STRS[i]);
			}

			/** If on ART, determine if observed OI should count towards ART failure */
			if (patient->getARTState()->isOnART) {
				int artLineNum = patient->getARTState()->currRegimenNum;
				if (patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
					const SimContext::TreatmentInputs::ARTFailPolicy &failART = simContext->getTreatmentInputs()->failART[artLineNum];
					bool isFailureOI = false;

					if (patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart >= failART.OIsMonthsFromInit) {
						if (failART.OIsEvent[i] == SimContext::ART_FAIL_BY_OI_ANY)
							isFailureOI = true;
						else if (!patient->getDiseaseState()->hasTrueOIHistory[i] &&
							(failART.OIsEvent[i] == SimContext::ART_FAIL_BY_OI_PRIMARY))
								isFailureOI = true;
						else if (patient->getDiseaseState()->hasTrueOIHistory[i] &&
							(failART.OIsEvent[i] == SimContext::ART_FAIL_BY_OI_SECONDARY))
								isFailureOI = true;
					}

					/** If OI should count towards failure, increment count and trigger a CD4 or HVL test if
					//	confirmatory testing is needed */
					if (isFailureOI) {
						incrementARTFailedOIs();
						if (patient->getARTState()->numFailedOIs >= failART.OIsMinNum) {
							if (failART.diagnoseUseCD4TestsConfirm)
								patient->getCD4TestUpdater()->performMonthlyUpdates();
							if (failART.diagnoseUseHVLTestsConfirm)
								patient->getHVLTestUpdater()->performMonthlyUpdates();
						}
					}
				}
				// early childhood uses pediatric ART inputs
				else{
					const SimContext::PedsInputs::ARTFailPolicy &failART = simContext->getPedsInputs()->failART[artLineNum];
					bool isFailureOI = false;

					if (patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart >= failART.OIsMonthsFromInit) {
						if (failART.OIsEvent[i] == SimContext::ART_FAIL_BY_OI_ANY)
							isFailureOI = true;
						else if (!patient->getDiseaseState()->hasTrueOIHistory[i] &&
							(failART.OIsEvent[i] == SimContext::ART_FAIL_BY_OI_PRIMARY))
								isFailureOI = true;
						else if (patient->getDiseaseState()->hasTrueOIHistory[i] &&
							(failART.OIsEvent[i] == SimContext::ART_FAIL_BY_OI_SECONDARY))
								isFailureOI = true;
					}

					/** If OI should count towards failure, increment count and trigger a CD4 or HVL test if
					//	confirmatory testing is needed */
					if (isFailureOI) {
						incrementARTFailedOIs();
						if (patient->getARTState()->numFailedOIs >= failART.OIsMinNum) {
							if (failART.diagnoseUseCD4TestsConfirm)
								patient->getCD4TestUpdater()->performMonthlyUpdates();
							if (failART.diagnoseUseHVLTestsConfirm)
								patient->getHVLTestUpdater()->performMonthlyUpdates();
						}
					}
				}

			}
		}
	}
} /* end performOIDetectionUpdates */

/** \brief performARTProgramUpdates evaluates ART policies and alters the treatment program */
void ClinicVisitUpdater::performARTProgramUpdates() {
	/** return if ART treatments are not available to the patient */
	if (!patient->getARTState()->mayReceiveART)
		return;
	bool hadObservedFailCurrMonth = false;

	/** If patient is currently on ART and not already observed to have failed,
	//	determine if failure is observed this month by calling ClinicVisitUpdater::evaluateFailARTPolicy() */
	if (patient->getARTState()->isOnART && !patient->getARTState()->hasObservedFailure) {
		if (patient->getARTState()->currSTIState == SimContext::STI_STATE_NONE) {
			SimContext::ART_FAIL_TYPE failType;
			if (patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				failType=evaluateFailARTPolicy();
			}
			else{
				failType=evaluateFailARTPolicyPeds();
			}

			if (failType != SimContext::ART_FAIL_NOT_FAILED) {
				setCurrARTObservedFailure(failType);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d ART %d FAIL OBSV BY %s;\n", patient->getGeneralState()->monthNum,
						patient->getARTState()->currRegimenNum + 1,
						SimContext::ART_FAIL_TYPE_STRS[patient->getARTState()->typeObservedFailure]);
				}
			}
		}
		else if (patient->getARTState()->currSTIState == SimContext::STI_STATE_RESTART) {
			SimContext::ART_FAIL_TYPE failType = evaluateSTIEndpointPolicy();
			if (failType != SimContext::ART_FAIL_NOT_FAILED) {
				setCurrSTIState(SimContext::STI_STATE_ENDPOINT);
				setCurrARTObservedFailure(failType);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d ART %d STI ENDPOINT OBSV BY %s;\n", patient->getGeneralState()->monthNum,
						patient->getARTState()->currRegimenNum + 1,
						SimContext::ART_FAIL_TYPE_STRS[patient->getARTState()->typeObservedFailure]);
				}
			}
		}
		/** If failure was observed this month, determine if regimen should be restarted based on
		// 	the patient's response type */
		if (patient->getARTState()->hasObservedFailure) {
			/** Get the current ART regimen information */
			int currRegimen = patient->getARTState()->currRegimenNum;
			const SimContext::ARTInputs *artInput = simContext->getARTInputs(currRegimen);
			if (patient->getARTState()->numFailedResupp < artInput->maxRestartAttempts){
				SimContext::RESP_TYPE responseType = patient->getARTState()->responseTypeCurrRegimen[SimContext::HET_OUTCOME_RESTART];
				double probRestart = patient->getARTState()->probRestartAfterFail;
				double randNum = CepacUtil::getRandomDouble(60090, patient);
				if (randNum < probRestart) {
					setNextARTRegimen(true, patient->getARTState()->currRegimenNum, true);
				}
			}
			hadObservedFailCurrMonth = true;
		}
	}

	/** If patient is on ART, evaluate stopping criteria  by calling ClinicVisitUpdater::evaluateStopARTPolicy()
	 * Also check for STI (standard treatment interruption) stopping here by calling ClinicVisitUpdater::evaluateSTIInitialStopPolicy()
	 * or ClinicVisitUpdater::evaluateSTISubsequentStopPolicy() depending on whether or not an STI has previously been applied.
	 **/
	SimContext::ART_STOP_TYPE stopType = SimContext::ART_STOP_NOT_STOPPED;
	bool stiInterrupt = false;
	if (patient->getARTState()->isOnART) {
		if (patient->getARTState()->currSTIState == SimContext::STI_STATE_NONE) {
			if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				stopType = evaluateStopARTPolicy();
			}
			else{
				stopType = evaluateStopARTPolicyPeds();
			}
			if (stopType == SimContext::ART_STOP_NOT_STOPPED) {
				if (evaluateSTIInitialStopPolicy()) {
					stopType = SimContext::ART_STOP_STI;
					stiInterrupt = true;
				}
			}
		}
		else if (patient->getARTState()->currSTIState == SimContext::STI_STATE_RESTART) {
			if (evaluateSTISubsequentStopPolicy()) {
				stopType = SimContext::ART_STOP_STI;
				stiInterrupt = true;
			}
		}
	}

	/** Stop the current ART regimen if it was determined to do so */
	if (stopType != SimContext::ART_STOP_NOT_STOPPED) {
		/** For STI stop, update the state to interrupt and set next regimen to the current one */
		if (stiInterrupt) {
			setCurrSTIState(SimContext::STI_STATE_INTERRUPT);
			setNextARTRegimen(true, patient->getARTState()->currRegimenNum);
		}

		//Check to see if stopping is due to major tox and set next line if it is
		if(patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			int artLineNum = patient->getARTState()->currRegimenNum;
			const SimContext::TreatmentInputs::ARTStopPolicy &stopART = simContext->getTreatmentInputs()->stopART[artLineNum];
			int nextLineAfterTox = stopART.nextLineAfterMajorTox;
			if (stopType == SimContext::ART_STOP_MAJ_TOX &&  nextLineAfterTox != SimContext::NOT_APPL){
				if (simContext->getARTInputs(nextLineAfterTox))
					setNextARTRegimen(true, nextLineAfterTox);
			}
			
		}
		//Check to see if stopping is due to chronic tox and set the next line accordingly
		if (stopType == SimContext::ART_STOP_CHRN_TOX){
            setNextARTRegimen(true, patient->getARTState()->chronicToxSwitchToLine);
		}
		/** Stop the ART regimen and set target HVL back to the setpoint */
		stopCurrARTRegimen(stopType);
		setTargetHVLStrata(patient->getDiseaseState()->setpointHVLStrata);

		/** Output tracing if enabled */
		if (patient->getGeneralState()->tracingEnabled) {
			tracer->printTrace(1, "**%d TAKEN OFF ART %d by %s;\n", patient->getGeneralState()->monthNum,
				patient->getARTState()->prevRegimenNum + 1,
				SimContext::ART_STOP_TYPE_STRS[patient->getARTState()->typeCurrStop]);
		}

		/** If no more lines are available, set the post-ART CD4/HVL testing interval */
		if (!patient->getARTState()->hasNextRegimenAvailable) {
			int intervalCD4 = simContext->getTreatmentInputs()->CD4TestingIntervalPostART;
			if (intervalCD4 != SimContext::NOT_APPL)
				scheduleCD4Test(true, patient->getGeneralState()->monthNum + intervalCD4);
			else
				scheduleCD4Test(false);
			int intervalHVL = simContext->getTreatmentInputs()->HVLTestingIntervalPostART;
			if (intervalHVL != SimContext::NOT_APPL)
				scheduleHVLTest(true, patient->getGeneralState()->monthNum + intervalHVL);
			else
				scheduleHVLTest(false);
		}
	}

	//Restart CD4 Monitoring if it was turned off and patient has either observed fail or is stopping ART
	if (simContext->getTreatmentInputs()->cd4MonitoringStopEnable && !patient->getMonitoringState()->CD4TestingAvailable){
		if (hadObservedFailCurrMonth || stopType != SimContext::ART_STOP_NOT_STOPPED){
			setCD4TestingAvailable(true);
			/** Output tracing if enabled */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d STARTING CD4 MONITORING;\n", patient->getGeneralState()->monthNum);
			}

			int testingInterval;
			if (patient->getARTState()->hasNextRegimenAvailable) {
				/** - If Patient has taken ART and is not on the last line of ART, use months on ART threshold to
				//	determine testing interval */
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalARTMonthsThreshold)
					testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalOnART[0];
				else
					testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalOnART[1];
			}
			else if (!patient->getARTState()->hasObservedFailure) {
			/** - If Patient is on last ART line and has no yet been observed to fail, use months on ART
			//	threshold to determine testing interval */
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalLastARTMonthsThreshold)
					testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalOnLastART[0];
				else
					testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalOnLastART[1];
			}
			else {
				/** - If Patient has failed last ART line, use post-ART testing interval */
				testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalPostART;
			}

			/** Schedule the CD4 test if there should be a next one */
			if (testingInterval != SimContext::NOT_APPL) {
				scheduleCD4Test(true, patient->getGeneralState()->monthNum + testingInterval);
			}
			else {
				scheduleCD4Test(false);
			}
		}
	}

	/** If patient is not on ART, determine if the patient should start a new regimen this month
	 * by calling ClinicVisitUpdater::evaluateStartARTPolicy().  If in the middle of an STI,
	 * check for restart criteria by calling ClinicVisitUpdater::evaluateSTIRestartPolicy().
	 * Also check if the patient returned to care from LTFU this month and if LTFU policies
	 * indicate an ART restart.
	 **/
	bool startNextART = false;
	bool rtcStart = false;
	bool stiRestart = false;
	if (!patient->getARTState()->isOnART) {
		// If returning to care and was on ART when lost, ART should be restarted
		if ((patient->getMonitoringState()->currLTFUState == SimContext::LTFU_STATE_RETURNED && patient->getMonitoringState()->monthOfLTFUStateChange == patient->getGeneralState()->monthNum)){
			rtcStart = true;
		}
		// Check whether they should restart automatically due to RTC
		if(rtcStart && !simContext->getLTFUInputs()->recheckARTStartPoliciesAtRTC && patient->getARTState()->hasNextRegimenAvailable &&
			patient->getMonitoringState()->wasOnARTWhenLostToFollowUp) {
				startNextART = true;
		}
		// If currently on STI interruption, evaluate STI restart criteria
		else if (patient->getARTState()->currSTIState == SimContext::STI_STATE_INTERRUPT) {
			if (evaluateSTIRestartPolicy()) {
				stiRestart = true;
				startNextART = true;
			}
		}
		// Otherwise, evaluate normal ART starting criteria
		else{
			if (patient->getPedsState()->ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE){
				// If patient's next line has passed max month number to start ART parameter look for the next one which is still available this month 
				if (patient->getARTState()->hasNextRegimenAvailable){
					int artLineNum = patient->getARTState()->nextRegimenNum;
					const SimContext::TreatmentInputs::ARTStartPolicy &startART = simContext->getTreatmentInputs()->startART[artLineNum];
					if ((startART.maxMonthNum != SimContext::NOT_APPL) &&
						(patient->getGeneralState()->monthNum > startART.maxMonthNum)) {
						bool hasNext = false;
						int nextRegimen = SimContext::NOT_APPL;
						for (int i = patient->getARTState()->nextRegimenNum + 1; i < SimContext::ART_NUM_LINES; i++) {
							if (simContext->getARTInputs(i)) {
								// Search for the next regimen which is still available to start this calendar month by checking for max mth number inputs
								const SimContext::TreatmentInputs::ARTStartPolicy &startART = simContext->getTreatmentInputs()->startART[i];
								if(startART.maxMonthNum == SimContext::NOT_APPL ||
								patient->getGeneralState()->monthNum <= startART.maxMonthNum){
									hasNext = true;
									nextRegimen = i;
									break;
								}	
							}
						}
						setNextARTRegimen(hasNext, nextRegimen);
					}
				}

				if (evaluateStartARTPolicy()){
					startNextART = true;
				}
			}
			else{
				if (evaluateStartARTPolicyPeds()){
					startNextART = true;
				}
			}

		}
	} // end if patient is not on ART

	/** Start the next ART regimen if it was determined to do so */
	if (startNextART) {
		//Check to see if we should guarantee initial suppression (They are on a resuppression regimen and they where obsv to fail but not true fail previous regimen
		bool guaranteeSuppression = false;
		if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			if (patient->getARTState()->hasTakenART)
				if (patient->getARTState()->hasObservedFailure && patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS)
					guaranteeSuppression = simContext->getTreatmentInputs()->startART[patient->getARTState()->nextRegimenNum].ensureSuppFalsePositiveFailure;
		}
		/** Start the ART regimen, update STI state if this is a restart, by calling StateUpdater::startNextARTRegimen() */
		if (stiRestart) {
			setCurrSTIState(SimContext::STI_STATE_RESTART);
		}
		startNextARTRegimen(patient->getARTState()->nextRegimenIsResupp);

		/** Get the current ART regimen information */
		int currRegimen = patient->getARTState()->currRegimenNum;
		int currSubRegimen = patient->getARTState()->currSubRegimenNum;
		const SimContext::ARTInputs *artInput = simContext->getARTInputs(currRegimen);
		const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(currRegimen);
		const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(currRegimen);

		SimContext::PEDS_AGE_CAT pedsAgeCat = patient->getPedsState()->ageCategoryPediatrics;
		int ayaAgeCat = getAgeCategoryAdolescent();

		/** Determine and set the ART response type for this regimen */
		double responseLogit = patient->getGeneralState()->responseBaselineLogit;
		int ageCat = patient->getGeneralState()->ageCategoryHeterogeneity;

		if(pedsAgeCat==SimContext::PEDS_AGE_ADULT){
			responseLogit += simContext->getHeterogeneityInputs()->propRespondAge[ageCat];
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			responseLogit += simContext->getHeterogeneityInputs()->propRespondCD4[cd4Strata];
		}
		else if(pedsAgeCat==SimContext::PEDS_AGE_LATE){
			responseLogit += simContext->getHeterogeneityInputs()->propRespondAgeLate;
			SimContext::CD4_STRATA cd4Strata = patient->getDiseaseState()->currTrueCD4Strata;
			responseLogit += simContext->getHeterogeneityInputs()->propRespondCD4[cd4Strata];
		}
		else{
			responseLogit += simContext->getHeterogeneityInputs()->propRespondAgeEarly;
		}

		if (patient->getGeneralState()->gender == SimContext::GENDER_FEMALE)
			responseLogit += simContext->getHeterogeneityInputs()->propRespondFemale;
		if (patient->getDiseaseState()->typeTrueOIHistory != SimContext::HIST_EXT_N)
			responseLogit += simContext->getHeterogeneityInputs()->propRespondHistoryOIs;
		if (patient->getARTState()->hadPrevToxicity)
			responseLogit += simContext->getHeterogeneityInputs()->propRespondPriorARTToxicity;
		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
			if (patient->getGeneralState()->hasRiskFactor[i])
				responseLogit += simContext->getHeterogeneityInputs()->propRespondRiskFactor[i];
		}
		// Draw the regimen-specific logit adjustment for the patient using the mean, SD, and distribution from the inputs
		double responseStdDev;
		double responseIncrRegimenMean;
		SimContext::HET_ART_LOGIT_DISTRIBUTION responseDist = SimContext::HET_ART_LOGIT_DISTRIBUTION_NORMAL;
		if (patient->getGeneralState()->isAdolescent){
			int ayaARTAgeCat = getAgeCategoryAdolescentART(currRegimen);
			responseIncrRegimenMean = simContext->getAdolescentARTInputs(currRegimen)->propRespondARTRegimenLogitMean[ayaARTAgeCat];
			responseStdDev = simContext->getAdolescentARTInputs(currRegimen)->propRespondARTRegimenLogitStdDev[ayaARTAgeCat];
		}
		else if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			responseIncrRegimenMean = simContext->getARTInputs(currRegimen)->propRespondARTRegimenLogitMean;
			responseStdDev = simContext->getARTInputs(currRegimen)->propRespondARTRegimenLogitStdDev;
			responseDist = (SimContext::HET_ART_LOGIT_DISTRIBUTION) simContext->getARTInputs(currRegimen)->propRespondARTRegimenLogitDistribution;
		}
		else if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
			responseIncrRegimenMean = simContext->getPedsARTInputs(currRegimen)->propRespondARTRegimenLogitMeanLate;
			responseStdDev = simContext->getPedsARTInputs(currRegimen)->propRespondARTRegimenLogitStdDevLate;
		}
		else{
			responseIncrRegimenMean = simContext->getPedsARTInputs(currRegimen)->propRespondARTRegimenLogitMeanEarly;
			responseStdDev = simContext->getPedsARTInputs(currRegimen)->propRespondARTRegimenLogitStdDevEarly;
		}
		// Adult patients have a choice of distributions, while the Pediatrics and Adolescent modules only use a normal distribution
		double responseLogitDraw = -1;
		if (responseDist == SimContext::HET_ART_LOGIT_DISTRIBUTION_NORMAL)
			responseLogitDraw = CepacUtil::getRandomGaussian(responseIncrRegimenMean, responseStdDev, 60100, patient);
		else if (responseDist == SimContext::HET_ART_LOGIT_DISTRIBUTION_TRUNC_NORMAL){
			while (responseLogitDraw < 0)
				responseLogitDraw = CepacUtil::getRandomGaussian(responseIncrRegimenMean, responseStdDev, 60120, patient);
		}
		else{
			while (responseLogitDraw < 0)
				responseLogitDraw = CepacUtil::getRandomGaussian(responseIncrRegimenMean, responseStdDev, 60120, patient);
			responseLogitDraw = responseLogitDraw * responseLogitDraw;
		}
		// preIncrResponseLogit is the response logit before any regimen-specific adjustments are made
		double preIncrResponseLogit = responseLogit;
		responseLogit += responseLogitDraw;
		/** Draw for duration of increment if enabled */
		int logitDuration = -1;
		bool enableDuration = simContext->getARTInputs(currRegimen)->propRespondARTRegimenUseDuration;
		if (enableDuration){
			double randNum = CepacUtil::getRandomGaussian(simContext->getARTInputs(currRegimen)->propRespondARTRegimenDurationMean,simContext->getARTInputs(currRegimen)->propRespondARTRegimenDurationStdDev,60130, patient);
			logitDuration = (int)(randNum + 0.5);
			if (logitDuration < 0)
				logitDuration = 0;
		}

		setARTResponseCurrRegimenBase(responseLogit, responseLogitDraw, preIncrResponseLogit, enableDuration, logitDuration);

		if (patient->getGeneralState()->isOnAdherenceIntervention)
			responseLogit += patient->getGeneralState()->responseLogitAdherenceInterventionIncrement;
		setCurrARTResponse(responseLogit);

		/** Determine and set the initial efficacy of the regimen
		// Determine the probability of suppression */
		SimContext::HVL_STRATA hvlStrata = patient->getDiseaseState()->currTrueHVLStrata;

		double probSuppress = 0.0;
		if (patient->getARTState()->isOnResupp)
			probSuppress = patient->getARTState()->probResuppEfficacy;
		else
			probSuppress =patient->getARTState()->probInitialEfficacy;


		/** If just Returned To Care and restarting the previous ART, and the users have enabled the alternate probabilities of suppression, find the probability based on their previous outcomes */
		if (rtcStart && currRegimen == patient->getARTState()->prevRegimenNum && simContext->getLTFUInputs()->useProbSuppByPrevOutcome) {
			if (patient->getARTState()->prevRegimenEfficacy == SimContext::ART_EFF_SUCCESS){
				probSuppress = simContext->getLTFUInputs()->probSuppressionWhenReturnToSuppressed[currRegimen];
			}
			else {
				probSuppress = simContext->getLTFUInputs()->probSuppressionWhenReturnToFailed[currRegimen];
			}
		}
		/**  Modify probability of suppression by resistance penalty */
		for (int i = 0; i <= currRegimen; i++) {
			int numMonths = patient->getARTState()->numMonthsOnUnsuccessfulByRegimen[i];
			double resistFactor = simContext->getTreatmentInputs()->ARTResistancePriorRegimen[currRegimen][i];
			probSuppress *= pow(1 - resistFactor, numMonths);
		}
		for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
			int numMonths = patient->getARTState()->numMonthsOnUnsuccessfulByHVL[i];
			double resistFactor = simContext->getTreatmentInputs()->ARTResistanceHVL[i];
			probSuppress *= pow(1 - resistFactor, numMonths);
		}

		/** Set the initial efficacy of the regimen using the calculated probabilities */
		SimContext::ART_EFF_TYPE efficacy;
		if (guaranteeSuppression){
			efficacy = SimContext::ART_EFF_SUCCESS;
		}
		else{
			double randNum = CepacUtil::getRandomDouble(60110, patient);
			if ((probSuppress > 0) && (randNum < probSuppress)) {
				efficacy = SimContext::ART_EFF_SUCCESS;
			}
			else {
				efficacy = SimContext::ART_EFF_FAILURE;
			}
		}

		setCurrARTEfficacy(efficacy, true);
		// Update consecutive failed resupp attempts based on efficacy
		if (patient->getARTState()->isOnResupp){
			if(efficacy == SimContext::ART_EFF_FAILURE){
				incrementARTFailedResupp();
			}
			else{
				resetARTFailedResupp();
			}
		}
		// Reset count if starting first ever regimen or switching to a new one
		else if(!(rtcStart && currRegimen == patient->getARTState()->prevRegimenNum)){
			resetARTFailedResupp();
		}

		/** Set the target HVL based on the destined efficacy */
		if (efficacy == SimContext::ART_EFF_SUCCESS) {
			setTargetHVLStrata(SimContext::HVL_VLO);
		}
		else {
			setTargetHVLStrata(patient->getDiseaseState()->setpointHVLStrata);
		}

		/** Set the initial CD4 slope for suppressive ART based on the response type,
		//	also set the CD4 envelope regimen and slope if this is the first successful regimen and the envelope is enabled; this controls the CD4 gain on successive ART regimens*/
		// the overall CD4 envelope holds across all regimens, so once active the patient always has an active overall envelope
		//  The individual regimen envelope is regimen-specific, so a patient could have an active individual envelope on one regimen and then when they start a new regimen, it is reset to inactive until they are successful on it
		if (efficacy == SimContext::ART_EFF_SUCCESS) {
			SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
			//Adolescent starting successful regimen
			if (patient->getGeneralState()->isAdolescent){
				int ayaARTAgeCat = getAgeCategoryAdolescentART(currRegimen);
				double cd4SlopeMean = ayaART->CD4ChangeOnSuppARTMean[0][ayaARTAgeCat];
				double cd4SlopeStdDev = ayaART->CD4ChangeOnSuppARTStdDev[0][ayaARTAgeCat];
				double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 60120, patient);
				setCurrRegimenCD4Slope(cd4Slope);
				// if this is their first success on the regimen, update the patient state to indicate that they have been successful on it, then update the CD4 envelopes if enabled
				if (!patient->getARTState()->hadSuccessOnRegimen)
					setSuccessfulARTRegimen();
				if (!patient->getARTState()->overallCD4Envelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_OVERALL, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
				}
				if (!patient->getARTState()->indivCD4Envelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_INDIV, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
				}
			}
			//Non-adolescent adult starting successful regimen
			else if (pedsAgeCat == SimContext::PEDS_AGE_ADULT) {
				double cd4SlopeMean = artInput->CD4ChangeOnSuppARTMean[cd4Response][0];
				double cd4SlopeStdDev = artInput->CD4ChangeOnSuppARTStdDev[cd4Response][0];
				double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 60120, patient);
				setCurrRegimenCD4Slope(cd4Slope);
				// if this is their first success on the regimen, update the patient state to indicate that they have been successful on it, then update the CD4 envelopes if enabled
				if (!patient->getARTState()->hadSuccessOnRegimen)
					setSuccessfulARTRegimen();

				if (!patient->getARTState()->overallCD4Envelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_OVERALL, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
				}
				if (!patient->getARTState()->indivCD4Envelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_INDIV, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
				}
			}
			//Late childhood patient starting successful regimen
			else if (pedsAgeCat == SimContext::PEDS_AGE_LATE) {
				double cd4SlopeMean = pedsART->CD4ChangeOnSuppARTMeanLate[cd4Response][0];
				double cd4SlopeStdDev = pedsART->CD4ChangeOnSuppARTStdDevLate[cd4Response][0];
				double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 60121, patient);

				setCurrRegimenCD4Slope(cd4Slope);
				// if this is their first success on the regimen, update the patient state to indicate that they have been successful on it, then update the CD4 envelopes if enabled
				if (!patient->getARTState()->hadSuccessOnRegimen)
					setSuccessfulARTRegimen();

				if (!patient->getARTState()->overallCD4Envelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_OVERALL, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
				}
				if (!patient->getARTState()->indivCD4Envelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_INDIV, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
				}
			}
			//Early childhood patient starting successful regimen
			else {
				/** For Peds early childhood, use CD4 Percentage */
				double cd4PercSlopeMean = pedsART->CD4PercentageChangeOnSuppARTMeanEarly[pedsAgeCat][cd4Response][0];
				double cd4PercSlopeStdDev = pedsART->CD4PercentageChangeOnSuppARTStdDevEarly[pedsAgeCat][cd4Response][0];
				double cd4PercSlope = CepacUtil::getRandomGaussian(cd4PercSlopeMean, cd4PercSlopeStdDev, 60122, patient);

				setCurrRegimenCD4PercentageSlope(cd4PercSlope);
				// if this is their first success on the regimen, update the patient state to indicate that they have been successful on it, then update the CD4 envelopes if enabled
				if (!patient->getARTState()->hadSuccessOnRegimen)
					setSuccessfulARTRegimen();

				if (!patient->getARTState()->overallCD4PercentageEnvelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_PERC_OVERALL, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_OVERALL, cd4PercSlope);
				}
				if (!patient->getARTState()->indivCD4PercentageEnvelope.isActive) {
					setCD4EnvelopeRegimen(SimContext::ENVL_CD4_PERC_INDIV, currRegimen);
					setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_INDIV, cd4PercSlope);
				}
			}
		} // end if (efficacy == SimContext::ART_EFF_SUCCESS)

		/** Accumulate the initial startup cost for this regimen */
		if (patient->getGeneralState()->isAdolescent){
			incrementCostsARTInit(currRegimen, ayaART->costInitial[ayaAgeCat] );
		}
		else if (pedsAgeCat == SimContext::PEDS_AGE_ADULT) {
			incrementCostsARTInit(currRegimen, artInput->costInitial);
		}
		else{
			incrementCostsARTInit(currRegimen, pedsART->costInitial[patient->getPedsState()->ageCategoryPedsARTCost]);
		}

		/** Print debugging information if enabled */
		if (patient->getGeneralState()->tracingEnabled) {

			tracer->printTrace(1, "**%d INIT NEW ART %d, $ %1.0lf;\n", patient->getGeneralState()->monthNum,
				patient->getARTState()->currRegimenNum + 1, patient->getGeneralState()->costsDiscounted);
			tracer->printTrace(1, "**%d ART DRAW %s;\n", patient->getGeneralState()->monthNum,
				SimContext::ART_EFF_STRS[efficacy]);
			if (patient->getARTState()->isOnResupp)
				tracer->printTrace(1, "**%d RESUPPRESSION ATTEMPT;\n", patient->getGeneralState()->monthNum);
			for(int i=0;i<SimContext::HET_NUM_OUTCOMES;i++){
				SimContext::RESP_TYPE responseType = patient->getARTState()->responseTypeCurrRegimen[i];
				tracer->printTrace(1, "**%d ART DRAW %s:%s;\n", patient->getGeneralState()->monthNum,
					SimContext::HET_OUTCOME_STRS[i], SimContext::RESP_TYPE_STRS[responseType]);
			}
		}

		/** Identify the next available art line, need to do here since starting
		//	criteria for the next line may be evaluated before current one is stopped */
		bool hasNext = false;
		int nextRegimen = SimContext::NOT_APPL;
		for (int i = patient->getARTState()->currRegimenNum + 1; i < SimContext::ART_NUM_LINES; i++) {
			if (simContext->getARTInputs(i)) {
					hasNext = true;
					nextRegimen = i;
					break;
			}
		}
		setNextARTRegimen(hasNext, nextRegimen);

		/** If CD4/HVL tests should happen at ART init, trigger them here if they have not yet occurred */
		if (simContext->getTreatmentInputs()->numARTInitialCD4Tests > 0) {
			patient->getCD4TestUpdater()->performMonthlyUpdates();
		}
		if (simContext->getTreatmentInputs()->numARTInitialHVLTests > 0) {
			patient->getHVLTestUpdater()->performMonthlyUpdates();
		}

		/** Set the on-ART CD4/HVL testing intervals */
		int intervalCD4 = SimContext::NOT_APPL;
		if (patient->getARTState()->hasNextRegimenAvailable)
			intervalCD4 = simContext->getTreatmentInputs()->CD4TestingIntervalOnART[0];
		else
			intervalCD4 = simContext->getTreatmentInputs()->CD4TestingIntervalOnLastART[0];
		if (intervalCD4 != SimContext::NOT_APPL)
			scheduleCD4Test(true, patient->getGeneralState()->monthNum + intervalCD4);
		else
			scheduleCD4Test(false);
		int intervalHVL = SimContext::NOT_APPL;
		if (patient->getARTState()->hasNextRegimenAvailable)
			intervalHVL = simContext->getTreatmentInputs()->HVLTestingIntervalOnART[0];
		else
			intervalHVL = simContext->getTreatmentInputs()->HVLTestingIntervalOnLastART[0];
		if (intervalHVL != SimContext::NOT_APPL)
			scheduleHVLTest(true, patient->getGeneralState()->monthNum + intervalHVL);
		else
			scheduleHVLTest(false);
	} //end if (startNextART)

	/** Set the initial ART subregimen or determine if the subregimen needs to be switched */
	if (patient->getARTState()->isOnART) {
		int currRegimen = patient->getARTState()->currRegimenNum;
		const SimContext::ARTInputs *artInput = simContext->getARTInputs(currRegimen);
		bool startSubRegimen = false;
		int nextSubRegimen = 0;
		// Always start the first subregimen for a new ART line
		if (startNextART) {
			startSubRegimen = true;
			nextSubRegimen = 0;
		}
		else {
			int currSubRegimen = patient->getARTState()->currSubRegimenNum;
			int monthsSubRegimen = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrSubRegimenStart;
			// Switch subregimen if triggered by a toxicity
			if (patient->getARTState()->hasSevereToxicity) {
				startSubRegimen = true;
				const SimContext::ARTToxicityEffect *toxEffect = patient->getARTState()->severeToxicityEffect;
				const SimContext::ARTInputs::ARTToxicity &toxInputs = artInput->toxicity[toxEffect->ARTSubRegimenNum][toxEffect->toxSeverityType][toxEffect->toxNum];
				nextSubRegimen = toxInputs.switchSubRegimenOnToxicity;
			}
			// Switch subregimen if specified time on subregimen has been exceeded
			else if ((artInput->monthsToSwitchSubRegimen[currSubRegimen] != SimContext::NOT_APPL) &&
				(monthsSubRegimen >= artInput->monthsToSwitchSubRegimen[currSubRegimen])) {
					startSubRegimen = true;
					nextSubRegimen = currSubRegimen + 1;
			}
		}

		if (startSubRegimen) {
			// Start that next ART subregimen
			startNextARTSubRegimen(nextSubRegimen);
			int currSubRegimen = nextSubRegimen;

			// Output tracing for the start of the ART subregimen
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d %s ART SUBREGIMEN %d.%d\n",
					patient->getGeneralState()->monthNum, startNextART ? "INIT" : "SWITCH",
					currRegimen + 1, currSubRegimen);
			}

			// Roll for all ART toxicities for new subregimen, toxicity does not occur for non-responders
			if (patient->getARTState()->responseTypeCurrRegimen[SimContext::HET_OUTCOME_TOX] != SimContext::RESP_TYPE_NON) {
				for (int i = 0; i < SimContext::ART_NUM_TOX_SEVERITY; i++) {
					for (int j = 0; j < SimContext::ART_NUM_TOX_PER_SEVERITY; j++) {
						double randNum = CepacUtil::getRandomDouble(60130, patient);
						double probTox = artInput->toxicity[currSubRegimen][i][j].probToxicity * patient->getARTState()->responseFactorCurrRegimen[SimContext::HET_OUTCOME_TOX];

						if (randNum < probTox) {
							bool hasTox = false;
							// Determine if the patient already has this toxicity effect
							const list<SimContext::ARTToxicityEffect> &toxicities = patient->getARTState()->activeToxicityEffects;
							for (list<SimContext::ARTToxicityEffect>::const_iterator k = toxicities.begin(); k != toxicities.end(); k++) {
								if ((k->toxSeverityType == i) && (k->toxNum == j)) {
									hasTox = true;
									break;
								}
							}
							// Add toxicity effect to the active list if it is not already present
							if (!hasTox) {
								double timeToToxMean =  artInput->toxicity[currSubRegimen][i][j].timeToToxicityMean;
								double timeToToxStdDev = artInput->toxicity[currSubRegimen][i][j].timeToToxicityStdDev;
								int timeToTox = (int) (CepacUtil::getRandomGaussian(timeToToxMean, timeToToxStdDev, 60140, patient) + 0.5);
								// Time to toxicity must be at least 1 month
								if(timeToTox <= 0){
									timeToTox = 1;
								}
								addARTToxicityEffect((SimContext::ART_TOX_SEVERITY) i, j, timeToTox);
							}
						}
					}
				}
			} // end rolling for toxicities
		} // end if starting subregimen
	} // end if patient is on ART
} /* end performARTProgramUpdates */

/** \brief evaluateStartARTPolicy determines if the starting criteria for ART has been met
 *
 * \return true if the Patient meets ART starting criteria
 **/
bool ClinicVisitUpdater::evaluateStartARTPolicy() {
	/** return false if there are no more available regimens */
	if (!patient->getARTState()->hasNextRegimenAvailable)
		return false;

	int artLineNum = patient->getARTState()->nextRegimenNum;
	const SimContext::TreatmentInputs::ARTStartPolicy &startART = simContext->getTreatmentInputs()->startART[artLineNum];

	/** return false if minimum time before starting has not yet been reached */
	if ((startART.minMonthNum != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum < startART.minMonthNum)) {
			return false;
	}

	/** return false if minimum time since last regimen stop has not yet been reached */
	if (startART.monthsSincePrevRegimen != SimContext::NOT_APPL) {
		if (patient->getARTState()->isOnART)
			return false;
		if ((patient->getARTState()->monthOfPrevRegimenStop != SimContext::NOT_APPL) &&
			(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfPrevRegimenStop < startART.monthsSincePrevRegimen))
			return false;
	}

	/** Evaluate the CD4 only criteria */
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if((observedCD4 >= startART.CD4BoundsOnly[SimContext::LOWER_BOUND]) &&
			(observedCD4 <= startART.CD4BoundsOnly[SimContext::UPPER_BOUND])) {
				return true;
		}
	}

	/** Evaluate the HVL strata only criteria */
	if(patient->getMonitoringState()->hasObservedHVLStrata){
		SimContext::HVL_STRATA observedHVL = patient->getMonitoringState()->currObservedHVLStrata;
		if ((observedHVL >= startART.HVLBoundsOnly[SimContext::LOWER_BOUND]) &&
		(observedHVL <= startART.HVLBoundsOnly[SimContext::UPPER_BOUND])) {
			return true;
		}
		/** Evaluate the CD4 and HVL combined criteria */
		if (patient->getMonitoringState()->hasObservedCD4){
			double observedCD4 = patient->getMonitoringState()->currObservedCD4;
			if((observedCD4 >= startART.CD4BoundsWithHVL[SimContext::LOWER_BOUND]) &&
			(observedCD4 <= startART.CD4BoundsWithHVL[SimContext::UPPER_BOUND]) &&
			patient->getMonitoringState()->hasObservedHVLStrata &&
			(observedHVL >= startART.HVLBoundsWithCD4[SimContext::LOWER_BOUND]) &&
			(observedHVL <= startART.HVLBoundsWithCD4[SimContext::UPPER_BOUND])) {
				return true;
			}
		}	
	}

	/** Evaluate the acute OIs since last ART only criteria */
	int numOIs = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (startART.OIHistory[i]) {
			numOIs += patient->getARTState()->numObservedOIsSinceFailOrStopART[i];
		}
	}
	if (numOIs >= startART.numOIs)
		return true;

	/** Evaluate the acute OIs in patient's history and CD4 count criteria */
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if ((observedCD4 >= startART.CD4BoundsWithOIs[SimContext::LOWER_BOUND]) &&
			(observedCD4 <= startART.CD4BoundsWithOIs[SimContext::UPPER_BOUND])) {
				for (int i = 0; i < SimContext::OI_NUM; i++) {
					if (startART.OIHistoryWithCD4[i] && (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)) {
						return true;
				}
			}
		}
	}

	return false;
} /* end evaluateStartARTPolicy */


/** \brief evaluateStartARTPolicyPeds determines if the starting criteria for ART has been met (Only for early childhood)
 *
 * \return true if the Patient meets ART starting criteria
 **/
bool ClinicVisitUpdater::evaluateStartARTPolicyPeds() {
	/** return false if there are no more available regimens */
	if (!patient->getARTState()->hasNextRegimenAvailable)
		return false;

	int artLineNum = patient->getARTState()->nextRegimenNum;
	const SimContext::PedsInputs::ARTStartPolicy &startART = simContext->getPedsInputs()->startART;

	/** return false if minimum time before starting has not yet been reached */
	if ((startART.minMonthNum[artLineNum] != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum < startART.minMonthNum[artLineNum])) {
			return false;
	}

	/** return false if minimum time since last regimen stop has not yet been reached */
	if (startART.monthsSincePrevRegimen[artLineNum] != SimContext::NOT_APPL) {
		if (patient->getARTState()->isOnART)
			return false;
		if ((patient->getARTState()->monthOfPrevRegimenStop != SimContext::NOT_APPL) &&
			(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfPrevRegimenStop < startART.monthsSincePrevRegimen[artLineNum]))
			return false;
	}

	/** Evaluate the CD4 only criteria */
	int cd4AgeCategory=0;
	for (int i=0;i<SimContext::NUM_ART_START_CD4PERC_PEDS-1;i++){
		if (patient->getGeneralState()->ageMonths > startART.CD4PercStageMonths[i]){
			cd4AgeCategory=i+1;
		}
		else{
			break;
		}
	}
	if (patient->getMonitoringState()->hasObservedCD4Percentage){
		double observedCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
		if((observedCD4Perc >= startART.CD4PercBounds[cd4AgeCategory][artLineNum][SimContext::LOWER_BOUND]) &&
		(observedCD4Perc <= startART.CD4PercBounds[cd4AgeCategory][artLineNum][SimContext::UPPER_BOUND])) {

			return true;
		}
	}

	/** Evaluate the HVL strata only criteria */
	
	if (patient->getMonitoringState()->hasObservedHVLStrata){
		SimContext::HVL_STRATA observedHVL = patient->getMonitoringState()->currObservedHVLStrata;
		if((observedHVL >= startART.HVLBounds[artLineNum][SimContext::LOWER_BOUND]) &&
			(observedHVL <= startART.HVLBounds[artLineNum][SimContext::UPPER_BOUND])) {
	
				return true;
		}
	}

	/** Evaluate the acute OIs since last ART only criteria */
	int numOIs = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (startART.OIHistory[artLineNum][i]) {
			numOIs += patient->getARTState()->numObservedOIsSinceFailOrStopART[i];
		}
	}
	if (numOIs >= startART.numOIs[artLineNum]){

		return true;
	}

	return false;
} /* end evaluateStartARTPolicyPeds */

/** \brief evaluateFailARTPolicy determines if the observed failure criteria for ART has been met
 *
 * \return The SimContext::ART_FAIL_TYPE indicating why ART failed (or that ART didn't fail) */
SimContext::ART_FAIL_TYPE ClinicVisitUpdater::evaluateFailARTPolicy() {
	int artLineNum = patient->getARTState()->currRegimenNum;
	const SimContext::TreatmentInputs::ARTFailPolicy &failART = simContext->getTreatmentInputs()->failART[artLineNum];

	/** check for clinical (OI based) failure */
	if (patient->getARTState()->numFailedOIs >= failART.OIsMinNum) {
		if (failART.diagnoseUseHVLTestsConfirm) {
			/** If using confirmatory HVL testing, also verify that this criteria has been met */
			if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm)
				return SimContext::ART_FAIL_CLINICAL;
		}
		if (failART.diagnoseUseCD4TestsConfirm) {
			// If using confirmatory CD4 testing, also verify that this criteria has been met */
			if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsConfirm)
				return SimContext::ART_FAIL_CLINICAL;
		}
		if (!failART.diagnoseUseHVLTestsConfirm && !failART.diagnoseUseCD4TestsConfirm) {
			// No confirmatory testing, return clinical failure
			return SimContext::ART_FAIL_CLINICAL;
		}
	}

	/** check for immunologic failure */
	if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail) {
		if (failART.diagnoseUseHVLTestsConfirm) {
			/** If using confirmatory HVL testing, also verify that this criteria has been met */
			if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm)
				return SimContext::ART_FAIL_IMMUNOLOGIC;
		}
		else {
			// No confirmatory testing, return immunologic failure
			return SimContext::ART_FAIL_IMMUNOLOGIC;
		}
	}

	/** check for virologic failure */
	if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsFail) {
		return SimContext::ART_FAIL_VIROLOGIC;
	}

	return SimContext::ART_FAIL_NOT_FAILED;
} /* end evaluateFailARTPolicy */


/** \brief evaluateFailARTPolicyPeds determines if the observed failure criteria for ART has been met for early childhood
 *
 * \return The SimContext::ART_FAIL_TYPE indicating why ART failed (or that ART didn't fail) */
SimContext::ART_FAIL_TYPE ClinicVisitUpdater::evaluateFailARTPolicyPeds() {
	int artLineNum = patient->getARTState()->currRegimenNum;
	const SimContext::PedsInputs::ARTFailPolicy &failART = simContext->getPedsInputs()->failART[artLineNum];

	/** check for clinical (OI based) failure */
	if (patient->getARTState()->numFailedOIs >= failART.OIsMinNum) {
		if (failART.diagnoseUseHVLTestsConfirm) {
			/** If using confirmatory HVL testing, also verify that this criteria has been met */
			if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm)
				return SimContext::ART_FAIL_CLINICAL;
		}
		if (failART.diagnoseUseCD4TestsConfirm) {
			// If using confirmatory CD4 testing, also verify that this criteria has been met */
			if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsConfirm)
				return SimContext::ART_FAIL_CLINICAL;
		}
		if (!failART.diagnoseUseHVLTestsConfirm && !failART.diagnoseUseCD4TestsConfirm) {
			// No confirmatory testing, return clinical failure
			return SimContext::ART_FAIL_CLINICAL;
		}
	}

	/** check for immunologic failure */
	if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail) {
		if (failART.diagnoseUseHVLTestsConfirm) {
			/** If using confirmatory HVL testing, also verify that this criteria has been met */
			if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm)
				return SimContext::ART_FAIL_IMMUNOLOGIC;
		}
		else {
			// No confirmatory testing, return immunologic failure
			return SimContext::ART_FAIL_IMMUNOLOGIC;
		}
	}

	/** check for virologic failure */
	if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsFail) {
		return SimContext::ART_FAIL_VIROLOGIC;
	}

	return SimContext::ART_FAIL_NOT_FAILED;
} /* end evaluateFailARTPolicyPeds */

/** \brief evaluateStopARTPolicy determines if the stopping criteria for ART has been met
 *
 * \return a SimContext::ART_STOP_TYPE indicating the reason for stopping ART (or SimContext::ART_STOP_NOT_STOPPED if ART shouldn't be stopped)*/
SimContext::ART_STOP_TYPE ClinicVisitUpdater::evaluateStopARTPolicy() {
	int artLineNum = patient->getARTState()->currRegimenNum;
	const SimContext::TreatmentInputs::ARTStopPolicy &stopART = simContext->getTreatmentInputs()->stopART[artLineNum];

	/** check if maximum months on ART is exceeded */
	if ((stopART.maxMonthsOnART != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart >= stopART.maxMonthsOnART)) {
			return SimContext::ART_STOP_MAX_MTHS;
	}

	/** check if a major toxicity has occurred and is specified to cause the regimen to be stopped */
	if (stopART.withMajorToxicty && patient->getARTState()->hasMajorToxicity){
		return SimContext::ART_STOP_MAJ_TOX;
	}
    /** check if a chronic toxicity has occurred and is specified to cause the regimen to be stopped */
    if (patient->getARTState()->hasChronicToxSwitch){
        return SimContext::ART_STOP_CHRN_TOX;
    }

	/** Return not stopped if there has not been an observed failure, or have not reached the
	//	minimum month number or months on ART */
	if (!patient->getARTState()->hasObservedFailure)
		return SimContext::ART_STOP_NOT_STOPPED;
	if ((stopART.afterFailMinMonthNum > 0) &&
		(patient->getGeneralState()->monthNum < stopART.afterFailMinMonthNum)) {
			return SimContext::ART_STOP_NOT_STOPPED;
	}
	if ((stopART.afterFailMonthsFromInit > 0) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart < stopART.afterFailMonthsFromInit)) {
			return SimContext::ART_STOP_NOT_STOPPED;
	}

	/** If we reach here, observed failure has already occurred
	// check if should stop immediately upon failure */
	if (stopART.afterFailImmediate) {
		return SimContext::ART_STOP_FAIL;
	}
	/** check if minimum CD4 threshold has been reached */
	if ((stopART.afterFailCD4LowerBound != SimContext::NOT_APPL) &&
		patient->getMonitoringState()->hasObservedCD4 &&
		(patient->getMonitoringState()->currObservedCD4 <= stopART.afterFailCD4LowerBound)) {
			return SimContext::ART_STOP_CD4;
	}

	/** check if observance of a severe OI should cause failure */
	if (stopART.afterFailWithSevereOI) {
		for (int i = 0; i < SimContext::OI_NUM; i++) {
			if ((patient->getARTState()->numObservedOIsSinceFailOrStopART[i] > 0) &&
				(simContext->getRunSpecsInputs()->severeOIs[i])) {
					return SimContext::ART_STOP_SEV_OI;
			}
		}
	}
	/** check if maximum numbers of months since failure has exceeded */
	if ((stopART.afterFailMonthsFromObserved != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfObservedFailure >= stopART.afterFailMonthsFromObserved)) {
			return SimContext::ART_STOP_FAIL_MTHS;
	}

	return SimContext::ART_STOP_NOT_STOPPED;
} /* evaluateStopARTPolicy */


/** \brief evaluateStopARTPolicyPeds determines if the stopping criteria for ART has been met for early childhood
 *
 * \return a SimContext::ART_STOP_TYPE indicating the reason for stopping ART (or SimContext::ART_STOP_NOT_STOPPED if ART shouldn't be stopped)*/
SimContext::ART_STOP_TYPE ClinicVisitUpdater::evaluateStopARTPolicyPeds() {
	int artLineNum = patient->getARTState()->currRegimenNum;
	const SimContext::PedsInputs::ARTStopPolicy &stopART = simContext->getPedsInputs()->stopART[artLineNum];

	/** check if maximum months on ART is exceeded */
	if ((stopART.maxMonthsOnART != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart >= stopART.maxMonthsOnART)) {
			return SimContext::ART_STOP_MAX_MTHS;
	}

	/** check if a major toxicity has occurred and is specified to cause the regimen to be stopped */
	if (stopART.withMajorToxicty && patient->getARTState()->hasMajorToxicity){
		return SimContext::ART_STOP_MAJ_TOX;
	}
	/** check if a chronic toxicity has occurred and is specified to cause the regimen to be stopped */
    if (patient->getARTState()->hasChronicToxSwitch){
        return SimContext::ART_STOP_CHRN_TOX;
    }

	/** Return not stopped if there has not been an observed failure, or have not reached the
	//	minimum month number or months on ART */
	if (!patient->getARTState()->hasObservedFailure)
		return SimContext::ART_STOP_NOT_STOPPED;
	if ((stopART.afterFailMinMonthNum > 0) &&
		(patient->getGeneralState()->monthNum < stopART.afterFailMinMonthNum)) {
			return SimContext::ART_STOP_NOT_STOPPED;
	}
	if ((stopART.afterFailMonthsFromInit > 0) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart < stopART.afterFailMonthsFromInit)) {
			return SimContext::ART_STOP_NOT_STOPPED;
	}

	/** If we reach here, observed failure has already occurred
	// check if should stop immediately upon failure */
	if (stopART.afterFailImmediate) {
		return SimContext::ART_STOP_FAIL;
	}
	/** check if minimum CD4 threshold has been reached */
	if ((stopART.afterFailCD4PercLowerBound != SimContext::NOT_APPL) &&
		patient->getMonitoringState()->hasObservedCD4Percentage &&
		(patient->getMonitoringState()->currObservedCD4Percentage <= stopART.afterFailCD4PercLowerBound)) {
			return SimContext::ART_STOP_CD4;
	}

	/** check if observance of a severe OI should cause failure */
	if (stopART.afterFailWithSevereOI) {
		for (int i = 0; i < SimContext::OI_NUM; i++) {
			if ((patient->getARTState()->numObservedOIsSinceFailOrStopART[i] > 0) &&
				(simContext->getRunSpecsInputs()->severeOIs[i])) {
					return SimContext::ART_STOP_SEV_OI;
			}
		}
	}
	/** check if maximum numbers of months since failure has exceeded */
	if ((stopART.afterFailMonthsFromObserved != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfObservedFailure >= stopART.afterFailMonthsFromObserved)) {
			return SimContext::ART_STOP_FAIL_MTHS;
	}

	return SimContext::ART_STOP_NOT_STOPPED;
} /* evaluateStopARTPolicyPeds */

/** \brief evaluateSTIInitialStopPolicy determines if the ART treatment should be stopped for the
	initial STI interruption

	\return true if the Patient meets the criteria for an initial ART treatment interruption
*/
bool ClinicVisitUpdater::evaluateSTIInitialStopPolicy() {
	int artLineNum = patient->getARTState()->currRegimenNum;
	const SimContext::STIInputs::InitiationPolicy &policy = simContext->getSTIInputs()->firstInterruption[artLineNum];

	/** return not stopped if STI is not enabled for this ART regimen*/
	if (!simContext->getTreatmentInputs()->enableSTIForART[artLineNum])
		return false;

	/** return not stopped if we haven't reached the minimum months for first interruption*/
	if ((policy.minMonthNum != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum < policy.minMonthNum))
		return false;
	if ((policy.monthsSinceARTStart != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart < policy.monthsSinceARTStart))
		return false;

	/** Evaluate the CD4 only criteria */
	
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if((observedCD4 >= policy.CD4BoundsOnly[SimContext::LOWER_BOUND]) &&
		(observedCD4 <= policy.CD4BoundsOnly[SimContext::UPPER_BOUND])) {
			return true;
		}
	}

	/** Evaluate the HVL strata only criteria */
	if (patient->getMonitoringState()->hasObservedHVLStrata){
		SimContext::HVL_STRATA observedHVL = patient->getMonitoringState()->currObservedHVLStrata;
		if((observedHVL >= policy.HVLBoundsOnly[SimContext::LOWER_BOUND]) &&
		(observedHVL <= policy.HVLBoundsOnly[SimContext::UPPER_BOUND])) {
			return true;
		}

		/** Evaluate the CD4 and HVL combined criteria */
		if (patient->getMonitoringState()->hasObservedCD4){
			double observedCD4 = patient->getMonitoringState()->currObservedCD4;
			if((observedCD4 >= policy.CD4BoundsWithHVL[SimContext::LOWER_BOUND]) &&
			(observedCD4 <= policy.CD4BoundsWithHVL[SimContext::UPPER_BOUND]) &&
			patient->getMonitoringState()->hasObservedHVLStrata &&
			(observedHVL >= policy.HVLBoundsWithCD4[SimContext::LOWER_BOUND]) &&
			(observedHVL <= policy.HVLBoundsWithCD4[SimContext::UPPER_BOUND])) {
				return true;
			}
		}	
	}

	/** Evaluate the acute OIs since last ART only criteria */
	int numOIs = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (policy.OIHistory[i]) {
			numOIs += patient->getARTState()->numObservedOIsSinceFailOrStopART[i];
		}
	}
	if (numOIs >= policy.numOIs)
		return true;

	/** Evaluate the acute OIs in patient's history and CD4 count criteria */
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if ((observedCD4 >= policy.CD4BoundsWithOIs[SimContext::LOWER_BOUND]) &&
			(observedCD4 <= policy.CD4BoundsWithOIs[SimContext::UPPER_BOUND])) {
			for (int i = 0; i < SimContext::OI_NUM; i++) {
				if (policy.OIHistoryWithCD4[i] && (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)) {
					return true;
				}
			}
		}
	}

	return false;
} /* evaluateSTIInitialStopPolicy */

/** \brief evaluateSTIEndpointPolicy determines the end of the STI cycle and indicates observed failure
 *
 * \return a SimContext::ART_FAIL_TYPE indicating the type of observed failure that ended the STI (or no failure if it didn't end)
 **/
SimContext::ART_FAIL_TYPE ClinicVisitUpdater::evaluateSTIEndpointPolicy() {
	int artLineNum = patient->getARTState()->currRegimenNum;
	const SimContext::STIInputs::EndpointPolicy &policy = simContext->getSTIInputs()->endpoint[artLineNum];

	/** return not failed if we haven't reached the minimum months since STI started */
	if ((policy.monthsSinceSTIStart != SimContext::NOT_APPL) &&
		(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfSTIInitialStop < policy.monthsSinceSTIStart))
		return SimContext::ART_FAIL_NOT_FAILED;

	/** Evaluate the CD4 only criteria */
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if((observedCD4 >= policy.CD4BoundsOnly[SimContext::LOWER_BOUND]) &&
		   (observedCD4 <= policy.CD4BoundsOnly[SimContext::UPPER_BOUND])) {
			return SimContext::ART_FAIL_IMMUNOLOGIC;
		}
	}

	/** Evaluate the HVL strata only criteria */
	if (patient->getMonitoringState()->hasObservedHVLStrata){
	SimContext::HVL_STRATA observedHVL = patient->getMonitoringState()->currObservedHVLStrata;
		if((observedHVL >= policy.HVLBoundsOnly[SimContext::LOWER_BOUND]) &&
		(observedHVL <= policy.HVLBoundsOnly[SimContext::UPPER_BOUND])) {
			return SimContext::ART_FAIL_VIROLOGIC;
		}
		/** Evaluate the CD4 and HVL combined criteria */
		if (patient->getMonitoringState()->hasObservedCD4){
			double observedCD4 = patient->getMonitoringState()->currObservedCD4;
			if((observedCD4 >= policy.CD4BoundsWithHVL[SimContext::LOWER_BOUND]) &&
			(observedCD4 <= policy.CD4BoundsWithHVL[SimContext::UPPER_BOUND]) &&
			patient->getMonitoringState()->hasObservedHVLStrata &&
			(observedHVL >= policy.HVLBoundsWithCD4[SimContext::LOWER_BOUND]) &&
			(observedHVL <= policy.HVLBoundsWithCD4[SimContext::UPPER_BOUND])) {
				return SimContext::ART_FAIL_VIROLOGIC;
			}
		}
	}

	/** Evaluate the acute OIs since last ART only criteria */
	int numOIs = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (policy.OIHistory[i]) {
			numOIs += patient->getARTState()->numObservedOIsSinceFailOrStopART[i];
		}
	}
	if (numOIs >= policy.numOIs)
		return SimContext::ART_FAIL_CLINICAL;

	/** Evaluate the acute OIs in patient's history and CD4 count criteria */
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if((observedCD4 >= policy.CD4BoundsWithOIs[SimContext::LOWER_BOUND]) &&
		(observedCD4 <= policy.CD4BoundsWithOIs[SimContext::UPPER_BOUND])) {
			for (int i = 0; i < SimContext::OI_NUM; i++) {
				if (policy.OIHistoryWithCD4[i] && (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)) {
					return SimContext::ART_FAIL_CLINICAL;
				}
			}
		}
	}

	return SimContext::ART_FAIL_NOT_FAILED;
} /* evaluateSTIEndpointPolicy */

/** \brief evaluateSTIRestartPolicy determines if ART should be restarted while interrupted
 *
 * \return true if Patient meets criteria to restart ART while currently the middle of an interruption */
bool ClinicVisitUpdater::evaluateSTIRestartPolicy() {
	int artLineNum = patient->getARTState()->nextRegimenNum;

	/** Evaluate the CD4 only criteria */
	if (patient->getMonitoringState()->hasObservedCD4){
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if((observedCD4 >= simContext->getSTIInputs()->ARTRestartCD4Bounds[artLineNum][SimContext::LOWER_BOUND]) &&
		(observedCD4 <= simContext->getSTIInputs()->ARTRestartCD4Bounds[artLineNum][SimContext::UPPER_BOUND])) {
			return true;
		}
	}

	/** Evaluate the HVL strata only criteria */
	if (patient->getMonitoringState()->hasObservedHVLStrata){
		SimContext::HVL_STRATA observedHVL = patient->getMonitoringState()->currObservedHVLStrata;
		if((observedHVL >= simContext->getSTIInputs()->ARTRestartHVLBounds[artLineNum][SimContext::LOWER_BOUND]) &&
		(observedHVL <= simContext->getSTIInputs()->ARTRestartHVLBounds[artLineNum][SimContext::UPPER_BOUND])) {
			return true;
		}
	}

	return false;
} /* evaluateSTIRestartPolicy */

/** \brief evaluateSTISubsequentStopPolicy determines if the ART treatment should be stopped for
	subsequent STI interruptions

	\return true if Patient meets criteria for stopping ART due to STI
*/
bool ClinicVisitUpdater::evaluateSTISubsequentStopPolicy() {
	int artLineNum = patient->getARTState()->nextRegimenNum;

	/** Evaluate the CD4 only criteria */
	if (patient->getMonitoringState()->hasObservedCD4) {
		double observedCD4 = patient->getMonitoringState()->currObservedCD4;
		if ((simContext->getSTIInputs()->ARTRestopCD4Bounds[artLineNum][SimContext::UPPER_BOUND] != SimContext::NOT_APPL) &&
			(observedCD4 > simContext->getSTIInputs()->ARTRestopCD4Bounds[artLineNum][SimContext::UPPER_BOUND])) {
				return true;
		}
		if ((simContext->getSTIInputs()->ARTRestopCD4Bounds[artLineNum][SimContext::LOWER_BOUND] != SimContext::NOT_APPL) &&
			(observedCD4 < simContext->getSTIInputs()->ARTRestopCD4Bounds[artLineNum][SimContext::LOWER_BOUND])) {
				return true;
		}
	}

	/** Evaluate the HVL strata only criteria */
	if (patient->getMonitoringState()->hasObservedHVLStrata) {
		SimContext::HVL_STRATA observedHVL = patient->getMonitoringState()->currObservedHVLStrata;
		if ((simContext->getSTIInputs()->ARTRestopHVLBounds[artLineNum][SimContext::UPPER_BOUND] != SimContext::NOT_APPL) &&
			(observedHVL > simContext->getSTIInputs()->ARTRestopHVLBounds[artLineNum][SimContext::UPPER_BOUND])) {
				return true;
		}
		if ((simContext->getSTIInputs()->ARTRestopHVLBounds[artLineNum][SimContext::LOWER_BOUND] != SimContext::NOT_APPL) &&
			(observedHVL < simContext->getSTIInputs()->ARTRestopHVLBounds[artLineNum][SimContext::LOWER_BOUND])) {
				return true;
		}
	}

	return false;
} /* evaluateSTISubsequentStopPolicy */

/** \brief performProphProgramUpdates evaluates prophylaxis policies and alters the treatment program */
void ClinicVisitUpdater::performProphProgramUpdates() {

	/** return if prohylaxes are not avaiable to the patient */
	if (!patient->getProphState()->mayReceiveProph)
		return;

	/** Determine if observed OIs will cause a switch to using secondary prophs and
	//	stopping current primary proph */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->getMonitoringState()->numObservedOIsSinceLastVisit[i] > 0) {
			// Skip if patient is already using or is set to use secondary prophs

			if (patient->getProphState()->isOnProph[i] &&
				(patient->getProphState()->currProphType[i] == SimContext::PROPH_SECONDARY))
				continue;

			if (patient->getProphState()->hasNextProphAvailable[i] &&
				(patient->getProphState()->nextProphType[i] == SimContext::PROPH_SECONDARY))
				continue;

			double randNum = CepacUtil::getRandomDouble(60150, patient);
			if (randNum < simContext->getTreatmentInputs()->probSwitchSecondaryProph[i]) {
				if (patient->getProphState()->isOnProph[i]) {
					// Stop the current proph
					SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[i];
					int prophNum = patient->getProphState()->currProphNum[i];
					stopCurrProph((SimContext::OI_TYPE) i);
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d STOP %s PROPH %d for OI %s;\n", patient->getGeneralState()->monthNum,
							SimContext::PROPH_TYPE_STRS[SimContext::PROPH_PRIMARY],
							prophNum + 1, SimContext::OI_STRS[i]);
					}
				}
				// Set the next proph that is available for use
				bool hasNext = false;
				SimContext::PROPH_TYPE nextProphType = SimContext::PROPH_SECONDARY;
				int nextProphNum = SimContext::NOT_APPL;
				for (int j = 0; j < SimContext::PROPH_NUM; j++) {
					const SimContext::ProphInputs *prophInput;
					if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
						prophInput=simContext->getProphInputs(nextProphType,i,j);
					}
					else{
						prophInput=simContext->getPedsProphInputs(nextProphType,i,j);
					}
					if (prophInput) {
						hasNext = true;
						nextProphNum = j;
						break;
					}
				}

				setNextProph(hasNext, nextProphType, (SimContext::OI_TYPE) i, nextProphNum);
			}
		}
	}


	/** Handle the appropriate stopping/switching for a toxicity or months to switch reached */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->getProphState()->isOnProph[i]) {
			SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[i];
			int prophNum = patient->getProphState()->currProphNum[i];
			const SimContext::ProphInputs *prophInput;
			if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				prophInput=simContext->getProphInputs(prophType,i,prophNum);
			}
			else{
				prophInput=simContext->getPedsProphInputs(prophType,i,prophNum);
			}
			int monthsOnProph = patient->getGeneralState()->monthNum - patient->getProphState()->monthOfProphStart[i];
			SimContext::PROPH_TOX_TYPE toxType = patient->getProphState()->typeProphToxicity[i];

			// Check for a toxicity causing a switch or reached months to switch proph
			if (((toxType == SimContext::PROPH_TOX_MAJOR) && prophInput->switchOnMajorToxicity) ||
				((toxType == SimContext::PROPH_TOX_MINOR) && prophInput->switchOnMinorToxicity) ||
				((prophInput->monthsToSwitch != SimContext::NOT_APPL) && (monthsOnProph >= prophInput->monthsToSwitch))) {
				// Stop the current proph
				stopCurrProph((SimContext::OI_TYPE) i);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d STOP %s PROPH %d for OI %s;\n", patient->getGeneralState()->monthNum,
						SimContext::PROPH_TYPE_STRS[prophType],
						prophNum + 1, SimContext::OI_STRS[i]);
				}

				// Set the next proph that is available for use
				bool hasNext = false;
				int nextProphNum = SimContext::NOT_APPL;
				for (int j = prophNum + 1; j < SimContext::PROPH_NUM; j++) {
					const SimContext::ProphInputs *nextProphInput;
					if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
						nextProphInput=simContext->getProphInputs(prophType,i,j);
					}
					else{
						nextProphInput=simContext->getPedsProphInputs(prophType,i,j);
					}
					if (nextProphInput) {
						hasNext = true;
						nextProphNum = j;
						break;
					}
				}
				setNextProph(hasNext, prophType, (SimContext::OI_TYPE) i, nextProphNum);

				// If next proph is available, start it now
				if (hasNext) {
					startNextProph((SimContext::OI_TYPE) i);
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d SWITCH %s PROPH TO %d for OI %s;\n", patient->getGeneralState()->monthNum,
							SimContext::PROPH_TYPE_STRS[prophType],
							nextProphNum + 1, SimContext::OI_STRS[i]);
					}
				}
			}
		}
	}

	/** If on prophs, determine if stopping policy criteria has been met for each OI */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		SimContext::OI_TYPE oiType = (SimContext::OI_TYPE) i;

		if (patient->getProphState()->isOnProph[i]) {

			// Patient is currently on proph for this OI, evaluate stopping policy
			SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[oiType];
			bool stopProph=false;
			if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				stopProph=evaluateStopProphPolicy(prophType,oiType);
			}
			else{
				stopProph=evaluateStopProphPolicyPeds(prophType,oiType);
			}
			if (stopProph) {
				// Stop the current proph
				SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[i];
				int prophNum = patient->getProphState()->currProphNum[i];
				stopCurrProph(oiType);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d STOP %s PROPH %d for OI %s;\n", patient->getGeneralState()->monthNum,
						SimContext::PROPH_TYPE_STRS[prophType],
						prophNum + 1, SimContext::OI_STRS[i]);
				}
			}
		}
	}

	/** If not on prophs, determine if starting policy criteria has been met for each OI */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		SimContext::OI_TYPE oiType = (SimContext::OI_TYPE) i;

		if (!patient->getProphState()->isOnProph[i]) {

			// Patient is currently not on any proph for this OI, evaluate starting policy
			//	and if stopping policy is not also immediately met
			SimContext::PROPH_TYPE prophType = patient->getProphState()->nextProphType[oiType];
			bool startProph=false;
			bool stopProph=false;

			if(patient->getProphState()->hasNextProphAvailable[oiType]){
				if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
					startProph=evaluateStartProphPolicy(prophType, oiType);
					stopProph=evaluateStopProphPolicy(prophType, oiType);
				}
				else{
					startProph=evaluateStartProphPolicyPeds(prophType, oiType);
					stopProph=evaluateStopProphPolicyPeds(prophType, oiType);
				}
			}

			if (startProph && !stopProph) {
				// Start a new proph if there is another one available
				startNextProph(oiType);
				if (patient->getGeneralState()->tracingEnabled) {
					tracer->printTrace(1, "**%d START %s PROPH %d for OI %s;\n", patient->getGeneralState()->monthNum,
						SimContext::PROPH_TYPE_STRS[patient->getProphState()->currProphType[i]],
						patient->getProphState()->currProphNum[i] + 1, SimContext::OI_STRS[i]);
				}
			}
		}
	}

} /* end performProphProgramUpdates */

/** \brief evaluateStartProphPolicy determines if the start criteria for the proph has been met
 *
 * \param prophType a SimContext::PROPH_TYPE indicating the type of prophylaxis being considered for starting criteria
 * \param oiType a SimContext::OI_TYPE indicating the OI being considered for prophylaxis starting criteria
 *
 * \return true if the Patient meets the criteria for starting prophylaxis
 **/
bool ClinicVisitUpdater::evaluateStartProphPolicy(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType) {

	/** return false if there is not another available proph */
	if (!patient->getProphState()->hasNextProphAvailable[oiType])
		return false;

	bool hasPassedOneCriteria = false;
	bool hasFailedOneCriteria = false;
	const SimContext::TreatmentInputs::ProphStartPolicy &prophStart = simContext->getTreatmentInputs()->startProph[prophType][oiType];

	/** Evaluate the minimum month for starting critera, return false if it is not met */
	int monthNum = patient->getGeneralState()->monthNum;

	if ((prophStart.minMonthNum != SimContext::NOT_APPL) && (monthNum < prophStart.minMonthNum))
		return false;
	/** Evaluate the current CD4 level criteria */
	if (patient->getMonitoringState()->hasObservedCD4) {
		double currCD4 = patient->getMonitoringState()->currObservedCD4;
		if ((currCD4 >= prophStart.currCD4Bounds[SimContext::LOWER_BOUND]) && (currCD4 <= prophStart.currCD4Bounds[SimContext::UPPER_BOUND])) {
			hasPassedOneCriteria = true;
		}
		else {
			hasFailedOneCriteria = true;
		}
	}

	/** Evaluate the minimum CD4 level criteria, */
	if (patient->getMonitoringState()->hasObservedCD4) {
		double minCD4 = patient->getMonitoringState()->minObservedCD4;
		if ((minCD4 >= prophStart.minCD4Bounds[SimContext::LOWER_BOUND]) && (minCD4 <= prophStart.minCD4Bounds[SimContext::UPPER_BOUND])) {
			hasPassedOneCriteria = true;
		}
		else {
			hasFailedOneCriteria = true;
		}
	}

	/** Evaluate the OI history criteria, skip if all the inputs are unspecified */
	bool useHistory = false;
	bool hasPassedOneOIHist = false;
	bool useNoHistory = false;
	bool hasFailedOneOIHist = false;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (prophStart.OIHistory[i] == 1) {
			useHistory = true;
			if (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)
				hasPassedOneOIHist = true;
		}
		else if (prophStart.OIHistory[i] == 0) {
			useNoHistory = true;
			if (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)
				hasFailedOneOIHist = true;
		}
	}
	if (useHistory && useNoHistory) {
		if (hasPassedOneOIHist && !hasFailedOneOIHist)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}
	else if (useHistory) {
		if (hasPassedOneOIHist)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}
	else if (useNoHistory) {
		if (!hasFailedOneOIHist)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}
	/** return true if using or evaluation and at least one criteria has been met,
	//	return true if using and evaluation and at least one criteria has been met and none have failed,
	//	return false otherwise */

	if (prophStart.useOrEvaluation && hasPassedOneCriteria)
		return true;
	if (!prophStart.useOrEvaluation && hasPassedOneCriteria && !hasFailedOneCriteria)
		return true;
	return false;
} /* end evaluateStartProphPolicy */

/** \brief evaluateStartProphPolicyPeds determines if the start criteria for the proph has been met for early childhood (<5 years old)
 *
 * \param prophType a SimContext::PROPH_TYPE indicating the type of prophylaxis being considered for starting criteria
 * \param oiType a SimContext::OI_TYPE indicating the OI being considered for prophylaxis starting criteria
 *
 * \return true if the Patient meets the criteria for starting prophylaxis
 **/
bool ClinicVisitUpdater::evaluateStartProphPolicyPeds(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType) {


	/** return false if there is not another available proph */
	if (!patient->getProphState()->hasNextProphAvailable[oiType])
		return false;

	bool hasPassedFirstCriteria = false;
	bool hasPassedSecondCriteria = false;
	bool hasPassedThirdCriteria = false;
	const SimContext::PedsInputs::ProphStartPolicy &prophStart = simContext->getPedsInputs()->startProph[prophType];

	/** Evaluate the age criteria */
	int ageMonths = patient->getGeneralState()->ageMonths;
	if (ageMonths>=prophStart.ageBounds[SimContext::LOWER_BOUND][oiType] && ageMonths<prophStart.ageBounds[SimContext::UPPER_BOUND][oiType]) {
		hasPassedFirstCriteria = true;
	}
	else {
		hasPassedFirstCriteria = false;
	}

	/** Evaluate the curr cd4 perc level criteria, */
	if (patient->getMonitoringState()->hasObservedCD4Percentage) {
		double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
		if (currCD4Perc>=prophStart.currCD4PercBounds[SimContext::LOWER_BOUND][oiType]&& currCD4Perc<=prophStart.currCD4PercBounds[SimContext::UPPER_BOUND][oiType]) {
			hasPassedSecondCriteria = true;
		}
		else {
			hasPassedSecondCriteria = false;
		}
	}

	/** Evaluate the OI history criteria, skip if all the inputs are unspecified */
	bool useThirdCriteria=false;

	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (prophStart.OIHistory[i][oiType]!=SimContext::NOT_APPL){
			useThirdCriteria=true;
			if(patient->getMonitoringState()->numObservedOIsTotal[i]>0){
				if(prophStart.OIHistory[i][oiType]==0){
					hasPassedThirdCriteria=false;
					break;
				}
				else{
					hasPassedThirdCriteria=true;
				}
			}
		}
	}

	//** Returns based on selection criteria*/
	if(useThirdCriteria){
		bool tempCondition;
		// Check the location of the parentheses in the conditonal statement
		if(prophStart.parDirection==SimContext::RIGHT){
			tempCondition=(prophStart.secondCondition==SimContext::AND)?hasPassedSecondCriteria && hasPassedThirdCriteria:hasPassedSecondCriteria || hasPassedThirdCriteria;
			return (prophStart.firstCondition==SimContext::AND)?hasPassedFirstCriteria && tempCondition:hasPassedFirstCriteria || tempCondition;
		}
		else{
			tempCondition=(prophStart.firstCondition==SimContext::AND)?hasPassedFirstCriteria && hasPassedSecondCriteria:hasPassedFirstCriteria || hasPassedSecondCriteria;
			return (prophStart.secondCondition==SimContext::AND)?tempCondition && hasPassedThirdCriteria:tempCondition || hasPassedThirdCriteria;
		}
	}
	else{
		return (prophStart.firstCondition==SimContext::AND)?hasPassedFirstCriteria && hasPassedSecondCriteria:hasPassedFirstCriteria || hasPassedSecondCriteria;
	}

	return false;
} /* end evaluateStartProphPolicyPeds */

/** \brief evaluateStopProphPolicy determines if the stopping criteria for the proph has been met
 *
 * \param prophType a SimContext::PROPH_TYPE indicating the type of prophylaxis being considered for stopping criteria
 * \param oiType a SimContext::OI_TYPE indicating the OI being considered for prophylaxis stopping criteria
 *
 * \return true if the Patient meets the criteria for stopping prophylaxis
 **/
bool ClinicVisitUpdater::evaluateStopProphPolicy(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType) {


	bool hasPassedOneCriteria = false;
	bool hasFailedOneCriteria = false;
	const SimContext::TreatmentInputs::ProphStopPolicy &prophStop = simContext->getTreatmentInputs()->stopProph[prophType][oiType];

	/** Evaluate the minimum month # and months on proph stopping criteria, return true if they are met */
	int monthNum = patient->getGeneralState()->monthNum;

	if ((prophStop.minMonthNum != SimContext::NOT_APPL) && (monthNum >= prophStop.minMonthNum)){

		return true;
	}

	if (patient->getProphState()->isOnProph[oiType]) {
		int monthsOnProph = monthNum - patient->getProphState()->monthOfProphStart[oiType];
		if ((prophStop.monthsOnProph != SimContext::NOT_APPL) && (monthsOnProph >= prophStop.monthsOnProph))
			return true;
	}

	/** Evaluate the current CD4 level criteria */
	if (patient->getMonitoringState()->hasObservedCD4) {
		double currCD4 = patient->getMonitoringState()->currObservedCD4;
		if ((currCD4 > prophStop.currCD4Bounds[SimContext::UPPER_BOUND]) || (currCD4 < prophStop.currCD4Bounds[SimContext::LOWER_BOUND])) {
			hasPassedOneCriteria = true;
		}
		else {
			hasFailedOneCriteria = true;
		}
	}

	/** Evaluate the minimum CD4 level criteria */
	if (patient->getMonitoringState()->hasObservedCD4) {
		double minCD4 = patient->getMonitoringState()->minObservedCD4;
		if ((minCD4 > prophStop.minCD4Bounds[SimContext::UPPER_BOUND]) || (minCD4 < prophStop.minCD4Bounds[SimContext::LOWER_BOUND])) {
			hasPassedOneCriteria = true;
		}
		else {
			hasFailedOneCriteria = true;
		}
	}

	/** Evaluate the OI history criteria, skip if all the inputs are unspecified */
	bool useHistory = false;
	bool hasPassedOneOIHist = false;
	bool useNoHistory = false;
	bool hasFailedOneOIHist = false;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (prophStop.OIHistory[i] == 1) {
			useHistory = true;
			if (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)
				hasPassedOneOIHist = true;
		}
		else if (prophStop.OIHistory[i] == 0) {
			useNoHistory = true;
			if (patient->getMonitoringState()->numObservedOIsTotal[i] > 0)
				hasFailedOneOIHist = true;
		}
	}
	if (useHistory && useNoHistory) {
		if (hasPassedOneOIHist && !hasFailedOneOIHist)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}
	else if (useHistory) {
		if (hasPassedOneOIHist)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}
	else if (useNoHistory) {
		if (!hasFailedOneOIHist)
			hasPassedOneCriteria = true;
		else
			hasFailedOneCriteria = true;
	}

	/** return true if using or evaluation and at least one criteria has been met,
	//	return true if using and evaluation and at least one criteria has been met and none have failed,
	//	return false otherwise */
	if (prophStop.useOrEvaluation && hasPassedOneCriteria)
		return true;
	if (!prophStop.useOrEvaluation && hasPassedOneCriteria && !hasFailedOneCriteria)
		return true;
	return false;
} /* evaluateStopProphPolicy */

/** \brief evaluateStopProphPolicyPeds determines if the stopping criteria for the proph has been met
 *
 * \param prophType a SimContext::PROPH_TYPE indicating the type of prophylaxis being considered for stopping criteria
 * \param oiType a SimContext::OI_TYPE indicating the OI being considered for prophylaxis stopping criteria
 *
 * \return true if the Patient meets the criteria for stopping prophylaxis
 **/
bool ClinicVisitUpdater::evaluateStopProphPolicyPeds(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType) {
	bool hasPassedFirstCriteria = false;
	bool hasPassedSecondCriteria = false;
	bool hasPassedThirdCriteria = false;
	const SimContext::PedsInputs::ProphStopPolicy &prophStop = simContext->getPedsInputs()->stopProph[prophType];


	/** Evaluate the months on proph stopping criteria, return true if they are met */
	int monthNum = patient->getGeneralState()->monthNum;
	if (patient->getProphState()->isOnProph[oiType]) {
		int monthsOnProph = monthNum - patient->getProphState()->monthOfProphStart[oiType];
		if ((prophStop.monthsOnProph[oiType] != SimContext::NOT_APPL) && (monthsOnProph >= prophStop.monthsOnProph[oiType]))
			return true;
	}

	/** Evaluate the age criteria */
	int ageMonths = patient->getGeneralState()->ageMonths;
	if (ageMonths>=prophStop.ageLowerBound[oiType]) {
		hasPassedFirstCriteria = true;
	}
	else {
		hasPassedFirstCriteria = false;
	}

	/** Evaluate the curr cd4 perc level criteria, */
	if (patient->getMonitoringState()->hasObservedCD4Percentage) {
		double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
		if (currCD4Perc>=prophStop.currCD4PercLowerBound[oiType]) {
			hasPassedSecondCriteria = true;
		}
		else {
			hasPassedSecondCriteria = false;
		}
	}

	/** Evaluate the OI history criteria, skip if all the inputs are unspecified */
	bool useThirdCriteria=false;

	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (prophStop.OIHistory[i][oiType]!=SimContext::NOT_APPL){
			useThirdCriteria=true;
			if(patient->getMonitoringState()->numObservedOIsTotal[i]>0){
				if(prophStop.OIHistory[i][oiType]==0){
					hasPassedThirdCriteria=false;
					break;
				}
				else{
					hasPassedThirdCriteria=true;
				}
			}
		}
	}

	//** Returns based on selection criteria*/
	if(useThirdCriteria){
		bool tempCondition;
		// Check the location of the parentheses in the conditonal statement
		if(prophStop.parDirection==SimContext::RIGHT){
			tempCondition=(prophStop.secondCondition==SimContext::AND)?hasPassedSecondCriteria && hasPassedThirdCriteria:hasPassedSecondCriteria || hasPassedThirdCriteria;
			return (prophStop.firstCondition==SimContext::AND)?hasPassedFirstCriteria && tempCondition:hasPassedFirstCriteria || tempCondition;
		}
		else{
			tempCondition=(prophStop.firstCondition==SimContext::AND)?hasPassedFirstCriteria && hasPassedSecondCriteria:hasPassedFirstCriteria || hasPassedSecondCriteria;
			return (prophStop.secondCondition==SimContext::AND)?tempCondition && hasPassedThirdCriteria:tempCondition || hasPassedThirdCriteria;
		}
	}
	else{
		return (prophStop.firstCondition==SimContext::AND)?hasPassedFirstCriteria && hasPassedSecondCriteria:hasPassedFirstCriteria || hasPassedSecondCriteria;
	}

	return false;

} /* evaluateStopProphPolicyPeds */