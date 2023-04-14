#include "include.h"

/** \brief Constructor takes in the patient object */
HVLTestUpdater::HVLTestUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
HVLTestUpdater::~HVLTestUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void HVLTestUpdater::performInitialUpdates() {
	/** Calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();
	setChanceHVLTest(false);
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void HVLTestUpdater::performMonthlyUpdates() {
	/** Return if a HVL test has already occurred for this month or patient is LTFU */
	if (patient->getMonitoringState()->hasObservedHVLStrata &&
		(patient->getMonitoringState()->monthOfObservedHVLStrata == patient->getGeneralState()->monthNum))
			return;
	if (patient->getMonitoringState()->currLTFUState == SimContext::LTFU_STATE_LOST)
		return;

	/** Check for conditions for a regularly scheduled test, also trigger a clinic visit if test should be done */
	bool performTest = false;
	if (patient->getMonitoringState()->hasScheduledHVLTest &&
		(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfScheduledHVLTest)) {
			scheduleEmergencyClinicVisit(SimContext::EMERGENCY_TEST, patient->getGeneralState()->monthNum, false);
			performTest = true;
	}
	/** Check for conditions for an initial ART test or repeat test after failure */
	else if (patient->getARTState()->isOnART) {
		int artLineNum = patient->getARTState()->currRegimenNum;
		if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
			if (!patient->getARTState()->hasObservedFailure){
				const SimContext::TreatmentInputs::ARTFailPolicy &failART = simContext->getTreatmentInputs()->failART[artLineNum];
				if (!simContext->getTreatmentInputs()->ARTFailureOnlyAtRegularVisit) {
					if ((patient->getARTState()->numFailedHVLTests > 0) &&
						(patient->getARTState()->numFailedHVLTests < failART.diagnoseNumTestsFail))
							performTest = true;
					else if (failART.diagnoseUseHVLTestsConfirm &&
						(patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail) &&
						(patient->getARTState()->numFailedHVLTests < failART.diagnoseNumTestsConfirm))
							performTest = true;
					else if (failART.diagnoseUseHVLTestsConfirm &&
						(patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
						(patient->getARTState()->numFailedHVLTests < failART.diagnoseNumTestsConfirm))
							performTest = true;
				}
			}
		}
		else{
			const SimContext::PedsInputs::ARTFailPolicy &failART = simContext->getPedsInputs()->failART[artLineNum];
			if (!patient->getARTState()->hasObservedFailure){
				if (!simContext->getTreatmentInputs()->ARTFailureOnlyAtRegularVisit) {
					if ((patient->getARTState()->numFailedHVLTests > 0) &&
						(patient->getARTState()->numFailedHVLTests < failART.diagnoseNumTestsFail))
							performTest = true;
					else if (failART.diagnoseUseHVLTestsConfirm &&
						(patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail) &&
						(patient->getARTState()->numFailedHVLTests < failART.diagnoseNumTestsConfirm))
							performTest = true;
					else if (failART.diagnoseUseHVLTestsConfirm &&
						(patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
						(patient->getARTState()->numFailedHVLTests < failART.diagnoseNumTestsConfirm))
							performTest = true;
				}
			}
		}
		if (patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart < simContext->getTreatmentInputs()->numARTInitialHVLTests)
			performTest = true;
	}
	if (!performTest)
		return;

	/** If performing a test, calculate the observed HVL value from the probabilities of higher/lower errors */
	double randNum = CepacUtil::getRandomDouble(110010, patient);
	if ((simContext->getTreatmentInputs()->probHVLTestErrorHigher > 0) &&
		(randNum < simContext->getTreatmentInputs()->probHVLTestErrorHigher)) {
		int hvlStrata = patient->getDiseaseState()->currTrueHVLStrata + 1;
		if (hvlStrata > SimContext::HVL_VHI)
			hvlStrata = SimContext::HVL_VHI;
		setObservedHVLStrata(true, (SimContext::HVL_STRATA) hvlStrata);
	}
	else {
		randNum -= simContext->getTreatmentInputs()->probHVLTestErrorHigher;
		if ((simContext->getTreatmentInputs()->probHVLTestErrorLower > 0) &&
			(randNum  < simContext->getTreatmentInputs()->probHVLTestErrorLower)) {
			int hvlStrata = patient->getDiseaseState()->currTrueHVLStrata - 1;
			if (hvlStrata < SimContext::HVL_VLO)
				hvlStrata = SimContext::HVL_VLO;
			setObservedHVLStrata(true, (SimContext::HVL_STRATA) hvlStrata);
		}
		else {
			setObservedHVLStrata(true, patient->getDiseaseState()->currTrueHVLStrata);
		}
	}

	SimContext::PEDS_COST_AGE pedsCostAgeCat=patient->getPedsState()->ageCategoryPedsCost;
	/** Accumulate the costs of the HVL test */
	if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
		incrementCostsHVLTest(simContext->getCostInputs()->HVLTestCost[patient->getGeneralState()->ageCategoryCost]);
	}
	else{
		incrementCostsHVLTest(simContext->getPedsCostInputs()->HVLTestCost[pedsCostAgeCat]);
	}

	incrementNumHVLTests();

	/** Print tracing for the HVL test if enabled */
	if (patient->getGeneralState()->tracingEnabled) {
		tracer->printTrace(1, "  %d HVL TEST: obsv HVL %s, $ %1.0lf;\n",
			patient->getGeneralState()->monthNum,
			SimContext::HVL_STRATA_STRS[patient->getMonitoringState()->currObservedHVLStrata],
			patient->getGeneralState()->costsDiscounted);
	}

	/** If on ART and not an initial test, determine if this HVL test counts as a failed test */
	bool failedHVL = false;
	if (patient->getARTState()->isOnART && (patient->getARTState()->monthOfCurrRegimenStart != patient->getGeneralState()->monthNum)) {
		int artLineNum = patient->getARTState()->currRegimenNum;


		if (patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
			const SimContext::TreatmentInputs::ARTFailPolicy &failART = simContext->getTreatmentInputs()->failART[artLineNum];

			/** - Is HVL level beyond specified number of buckets or above initial level */
			if (failART.HVLNumIncrease != SimContext::NOT_APPL) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				SimContext::HVL_STRATA minHVL = patient->getARTState()->minObservedHVLStrataOnCurrART;
				if (currHVL - minHVL >= failART.HVLNumIncrease) {
					failedHVL = true;
				}
			}
			/** - Is HVL level at setpoint/maximum level */
			if (!failedHVL && failART.HVLFailAtSetpoint) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				SimContext::HVL_STRATA maxHVL = patient->getMonitoringState()->maxObservedHVLStrata;
				if ((currHVL > SimContext::HVL_VLO) && (currHVL >= maxHVL))
					failedHVL = true;
			}
			/** - Is HVL level outside the absolute bounds */
			if (!failedHVL && (failART.HVLBounds[SimContext::LOWER_BOUND] != SimContext::NOT_APPL)) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				if (currHVL < failART.HVLBounds[SimContext::LOWER_BOUND]) {
					failedHVL = true;
				}
			}
			if (!failedHVL && (failART.HVLBounds[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				if (currHVL > failART.HVLBounds[SimContext::UPPER_BOUND]) {
					failedHVL = true;
				}
			}

			/** If failed criteria, evaluate if months on ART has been reached */
			if (failedHVL && (failART.HVLMonthsFromInit > 0)) {
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				if (monthsOnART < failART.HVLMonthsFromInit) {
					failedHVL = false;
				}
			}

			/** If failed HVL test, increment the number of failed tests */
			if (failedHVL) {
				incrementARTFailedHVLTests();

				/** First check for only the virologic failure criteria being met */
				if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsFail)
					scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum, false);
				if (failART.diagnoseUseHVLTestsConfirm) {
					/** Using confirmatory HVL testing, trigger an emergency clinic visit if the
					//	confirmatory criteria and clinical or immunologic failure has occurred */
					if ((patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail) &&
						(patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm))
							scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum, false);
					if ((patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
						(patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm))
							scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum, false);
				}
			}
			else {
				/** If test was not failed, reset the count of failed tests if there were previous failures */
				if (patient->getARTState()->numFailedHVLTests > 0)
					resetARTFailedHVLTests();
				/** If this was a confirmatory HVL test, also reset the number of failed clinical
				//	or immunological tests that caused it */
				if (failART.diagnoseUseHVLTestsConfirm &&
					(patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail))
						resetARTFailedCD4Tests();
				if (failART.diagnoseUseHVLTestsConfirm &&
					(patient->getARTState()->numFailedOIs >= failART.OIsMinNum))
						resetARTFailedOIs();
			}
		}
		else{
			const SimContext::PedsInputs::ARTFailPolicy &failART=simContext->getPedsInputs()->failART[artLineNum];

			/** - Is HVL level beyond specified number of buckets or above initial level */
			if (failART.HVLNumIncrease != SimContext::NOT_APPL) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				SimContext::HVL_STRATA minHVL = patient->getARTState()->minObservedHVLStrataOnCurrART;
				if (currHVL - minHVL >= failART.HVLNumIncrease) {
					failedHVL = true;
				}
			}
			/** - Is HVL level at setpoint/maximum level */
			if (!failedHVL && failART.HVLFailAtSetpoint) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				SimContext::HVL_STRATA maxHVL = patient->getMonitoringState()->maxObservedHVLStrata;
				if ((currHVL > SimContext::HVL_VLO) && (currHVL >= maxHVL))
					failedHVL = true;
			}
			/** - Is HVL level outside the absolute bounds */
			if (!failedHVL && (failART.HVLBounds[SimContext::LOWER_BOUND] != SimContext::NOT_APPL)) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				if (currHVL < failART.HVLBounds[SimContext::LOWER_BOUND]) {
					failedHVL = true;
				}
			}
			if (!failedHVL && (failART.HVLBounds[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
				SimContext::HVL_STRATA currHVL = patient->getMonitoringState()->currObservedHVLStrata;
				if (currHVL > failART.HVLBounds[SimContext::UPPER_BOUND]) {
					failedHVL = true;
				}
			}

			/** If failed criteria, evaluate if months on ART has been reached */
			if (failedHVL && (failART.HVLMonthsFromInit > 0)) {
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				if (monthsOnART < failART.HVLMonthsFromInit) {
					failedHVL = false;
				}
			}

			/** If failed HVL test, increment the number of failed tests */
			if (failedHVL) {
				incrementARTFailedHVLTests();

				/** First check for only the virologic failure criteria being met */
				if (patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsFail)
					scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
				if (failART.diagnoseUseHVLTestsConfirm) {
					/** Using confirmatory HVL testing, trigger an emergency clinic visit if the
					//	confirmatory criteria and clinical or immunologic failure has occurred */
					if ((patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail) &&
						(patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm))
							scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
					if ((patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
						(patient->getARTState()->numFailedHVLTests >= failART.diagnoseNumTestsConfirm))
							scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
				}
			}
			else {
				/** If test was not failed, reset the count of failed tests if there were previous failures */
				if (patient->getARTState()->numFailedHVLTests > 0)
					resetARTFailedHVLTests();
				/** If this was a confirmatory HVL test, also reset the number of failed clinical
				//	or immunological tests that caused it */
				if (failART.diagnoseUseHVLTestsConfirm &&
					(patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail))
						resetARTFailedCD4Tests();
				if (failART.diagnoseUseHVLTestsConfirm &&
					(patient->getARTState()->numFailedOIs >= failART.OIsMinNum))
						resetARTFailedOIs();
			}
		}

	}

	/** If this was a scheduled HVL test, determine if another one should be scheduled */
	if (patient->getMonitoringState()->hasScheduledHVLTest &&
		(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfScheduledHVLTest)) {
		int testingInterval;
		if (!patient->getARTState()->hasTakenART) {
			/** - If Patient has not yet begun ART, use CD4 threshold to determine testing interval */
			if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT) {
				if (patient->getMonitoringState()->currObservedCD4 > simContext->getTreatmentInputs()->testingIntervalCD4Threshold) {
					testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalPreARTHighCD4;
				}
				else {
					testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalPreARTLowCD4;
				}
			}
			else if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_LATE) {
				testingInterval = simContext->getPedsInputs()->HVLTestingIntervalPreARTLate;
			}
			else {
				testingInterval = simContext->getPedsInputs()->HVLTestingIntervalPreARTEarly;
			}
		}
		else if (patient->getARTState()->hasNextRegimenAvailable) {
			/** - If Patient has taken ART and is not on the last line of ART, use months on ART threshold to
			//	determine testing interval */
			int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
			if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalARTMonthsThreshold)
				testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalOnART[0];
			else
				testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalOnART[1];
		}
		else if (!patient->getARTState()->hasObservedFailure) {
			/** - If Patient is on last ART line and has no yet been observed to fail, use months on ART
			//	threshold to determine testing interval */
			int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
			if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalLastARTMonthsThreshold)
				testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalOnLastART[0];
			else
				testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalOnLastART[1];
		}
		else {
			/** - Patient has failed last ART line, use post-ART testing interval */
			testingInterval = simContext->getTreatmentInputs()->HVLTestingIntervalPostART;
		}

		/** Schedule the HVL test if there should be a next one */
		if (testingInterval != SimContext::NOT_APPL) {
			scheduleHVLTest(true, patient->getGeneralState()->monthNum + testingInterval);
		}
		else {
			scheduleHVLTest(false);
		}
	}
} /* end performMonthlyUpdates */

