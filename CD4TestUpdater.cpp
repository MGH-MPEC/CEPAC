#include "include.h"

/** \brief Constructor takes in the patient object */
CD4TestUpdater::CD4TestUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
CD4TestUpdater::~CD4TestUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void CD4TestUpdater::performInitialUpdates() {
	/** Calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();
	setChanceCD4Test(false);
	setCD4TestingAvailable(true);
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
void CD4TestUpdater::performMonthlyUpdates() {
	/** Return if a CD4 test has already occurred for this month or patient is LTFU */
	if (patient->getMonitoringState()->hasObservedCD4NonLabStaging &&
		(patient->getMonitoringState()->monthOfObservedCD4NonLabStaging == patient->getGeneralState()->monthNum))
			return;
	if (patient->getMonitoringState()->hasObservedCD4Percentage &&
		(patient->getMonitoringState()->monthOfObservedCD4Percentage == patient->getGeneralState()->monthNum))
			return;
	if (patient->getMonitoringState()->currLTFUState == SimContext::LTFU_STATE_LOST)
		return;
	if (!patient->getMonitoringState()->CD4TestingAvailable)
		return;

	/** Check for conditions for a regularly scheduled test, also triggers a clinic visit if test should be done */
	bool performTest = false;
	if (patient->getMonitoringState()->hasScheduledCD4Test &&
		(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfScheduledCD4Test)) {
			scheduleEmergencyClinicVisit(SimContext::EMERGENCY_TEST, patient->getGeneralState()->monthNum);
			performTest = true;
	}
	/** Check for conditions for an initial ART test or repeat test after failure */
	if (patient->getARTState()->isOnART) {
		int artLineNum = patient->getARTState()->currRegimenNum;
		if (patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
			const SimContext::TreatmentInputs::ARTFailPolicy &failART = simContext->getTreatmentInputs()->failART[artLineNum];
			if (!patient->getARTState()->hasObservedFailure){
				if (!simContext->getTreatmentInputs()->ARTFailureOnlyAtRegularVisit) {
					if ((patient->getARTState()->numFailedCD4Tests > 0) &&
						(patient->getARTState()->numFailedCD4Tests < failART.diagnoseNumTestsFail))
							performTest = true;
					else if (failART.diagnoseUseCD4TestsConfirm &&
						(patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
						(patient->getARTState()->numFailedCD4Tests < failART.diagnoseNumTestsConfirm))
							performTest = true;
				}
			}
		}
		else{
			const SimContext::PedsInputs::ARTFailPolicy &failART = simContext->getPedsInputs()->failART[artLineNum];
			if (!simContext->getTreatmentInputs()->ARTFailureOnlyAtRegularVisit) {
				if ((patient->getARTState()->numFailedCD4Tests > 0) &&
					(patient->getARTState()->numFailedCD4Tests < failART.diagnoseNumTestsFail))
						performTest = true;
				else if (failART.diagnoseUseCD4TestsConfirm &&
					(patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
					(patient->getARTState()->numFailedCD4Tests < failART.diagnoseNumTestsConfirm))
						performTest = true;
			}
		}

		if (patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart < simContext->getTreatmentInputs()->numARTInitialCD4Tests)
			performTest = true;
	}
	if (!performTest)
		return;

	/** If performing a test, calculate the observed CD4 level using the standard deviation of testing error */
	if (patient->getPedsState()->ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE) {
		/** - Late childhood and adults use absolute CD4 */
		double stdDevPerc = CepacUtil::getRandomGaussian(0,  simContext->getTreatmentInputs()->CD4TestStdDevPercentage, 50010, patient);
		double biasStdDevPerc = CepacUtil::getRandomGaussian(0 , simContext->getTreatmentInputs()->CD4TestBiasStdDevPercentage, 50013, patient);
		double cd4Value = patient->getDiseaseState()->currTrueCD4*(1+stdDevPerc+simContext->getTreatmentInputs()->CD4TestBiasMean)*(1+biasStdDevPerc);

		setObservedCD4(true, cd4Value);
	}
	else {
		/** - Early childhood uses CD4 percentage */
		double stdDevPerc = simContext->getTreatmentInputs()->CD4TestStdDevPercentage;
		double stdDev = stdDevPerc * patient->getDiseaseState()->currTrueCD4Percentage;
		double cd4Percent = patient->getDiseaseState()->currTrueCD4Percentage + CepacUtil::getRandomGaussian(0, stdDev, 50011, patient);
		setObservedCD4Percentage(true, cd4Percent);
	}

	SimContext::PEDS_COST_AGE pedsCostAgeCat=patient->getPedsState()->ageCategoryPedsCost;
	/** Accumulate the costs of the CD4 test */
	if(pedsCostAgeCat==SimContext::PEDS_COST_AGE_ADULT){
		incrementCostsCD4Test(simContext->getCostInputs()->CD4TestCost[patient->getGeneralState()->ageCategoryCost]);
	}
	else{
		incrementCostsCD4Test(simContext->getPedsCostInputs()->CD4TestCost[pedsCostAgeCat]);
	}

	incrementNumCD4Tests();

	/** Print tracing for the CD4 test if enabled */
	if (patient->getGeneralState()->tracingEnabled) {
		if (patient->getPedsState()->ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE) {
			tracer->printTrace(1, "  %d CD4 TEST: obsv CD4 %1.0lf, $ %1.0lf;\n",
				patient->getGeneralState()->monthNum, patient->getMonitoringState()->currObservedCD4,
				patient->getGeneralState()->costsDiscounted);
		}
		else {
			tracer->printTrace(1, "  %d CD4 TEST: obsv CD4 perc %1.3lf, $ %1.0lf;\n",
				patient->getGeneralState()->monthNum, patient->getMonitoringState()->currObservedCD4Percentage,
				patient->getGeneralState()->costsDiscounted);
		}
	}

	//Disable CD4 Testing if observed value is over threshold and this feature is turned on
	if (patient->getPedsState()->ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE) {
		if (simContext->getTreatmentInputs()->cd4MonitoringStopEnable && patient->getMonitoringState()->CD4TestingAvailable){
			if (patient->getARTState()->isOnART &&
					patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS &&
					patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart >= simContext->getTreatmentInputs()->cd4MonitoringStopMthsPostARTInit){
				if (patient->getMonitoringState()->currObservedCD4 >= simContext->getTreatmentInputs()->cd4MonitoringStopThreshold){
					setCD4TestingAvailable(false);
					/** Output tracing if enabled */
					if (patient->getGeneralState()->tracingEnabled) {
						tracer->printTrace(1, "**%d STOPPING CD4 MONITORING;\n", patient->getGeneralState()->monthNum);
					}
				}
			}
		}
	}

	/** If on ART, determine if this CD4 test counts as a failed test */
	bool failedCD4 = false;
	if (patient->getARTState()->isOnART && (patient->getARTState()->monthOfCurrRegimenStart != patient->getGeneralState()->monthNum)) {
		int artLineNum = patient->getARTState()->currRegimenNum;


		/** Late childhood and adults use absolute CD4 criteria */
		if (patient->getPedsState()->ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE) {
			const SimContext::TreatmentInputs::ARTFailPolicy &failART = simContext->getTreatmentInputs()->failART[artLineNum];

			/**		- Has the percentage CD4 drop from maximum been exceeded */
			if (failART.CD4PercentageDrop != SimContext::NOT_APPL) {
				double currCD4 = patient->getMonitoringState()->currObservedCD4;
				double maxCD4 = patient->getARTState()->maxObservedCD4OnCurrART;
				if ((maxCD4 - currCD4) / maxCD4 >= failART.CD4PercentageDrop) {
					failedCD4 = true;
				}
			}
			/** 	- Has the CD4 count dropped below the pre-ART nadir */
			if (!failedCD4 && failART.CD4BelowPreARTNadir) {
				double currCD4 = patient->getMonitoringState()->currObservedCD4;
				double minCD4 = patient->getMonitoringState()->minObservedCD4;
				if (currCD4 <= minCD4) {
					failedCD4 = true;
				}
			}
			/**		- Is CD4 level outside the absolute OR bounds */
			if (!failedCD4 && (failART.CD4BoundsOR[SimContext::LOWER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4 = patient->getMonitoringState()->currObservedCD4;
				if (currCD4 < failART.CD4BoundsOR[SimContext::LOWER_BOUND]) {
					failedCD4 = true;
				}
			}
			if (!failedCD4 && (failART.CD4BoundsOR[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4 = patient->getMonitoringState()->currObservedCD4;
				if (currCD4 > failART.CD4BoundsOR[SimContext::UPPER_BOUND]) {
					failedCD4 = true;
				}
			}

			/**		- If failed criteria, evaluate CD4 level outside the absolute AND bounds */
			if (failedCD4 && (failART.CD4BoundsAND[SimContext::LOWER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4 = patient->getMonitoringState()->currObservedCD4;
				if (currCD4 >= failART.CD4BoundsAND[SimContext::LOWER_BOUND]) {
					failedCD4 = false;
				}
			}
			if (failedCD4 && (failART.CD4BoundsAND[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4 = patient->getMonitoringState()->currObservedCD4;
				if (currCD4 <= failART.CD4BoundsAND[SimContext::UPPER_BOUND]) {
					failedCD4 = false;
				}
			}
			/**		- If failed criteria, evaluate if months on ART has been reached */
			if (failedCD4 && (failART.CD4MonthsFromInit > 0)) {
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				if (monthsOnART < failART.CD4MonthsFromInit) {
					failedCD4 = false;
				}
			}

			/** If failed the CD4 test, increment the number of failed tests */
			if (failedCD4) {
				incrementARTFailedCD4Tests();

				if (failART.diagnoseUseHVLTestsConfirm) {
					/** If using HVL confirmatory tests, trigger HVL test if the immunologic criteria has been met */
					if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail)
						patient->getHVLTestUpdater()->performMonthlyUpdates();
				}
				else {
					/** If not using HVL confirmatory testing, check for only immunologic failure */
					if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail)
						scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
					if (failART.diagnoseUseCD4TestsConfirm) {
						/** If using CD4 confirmatory testing, trigger an emergency clinic visit if this
						/*	failed test confirms the clinical failure */
						if ((patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
							(patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsConfirm))
								scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
					}
				}
			}
			else {
				/** If test was not failed, reset the count of failed tests if there were previous failures */
				if (patient->getARTState()->numFailedCD4Tests > 0)
					resetARTFailedCD4Tests();
				/** If this was a confirmatory CD4 test, also reset the number of failed clinical
				//	tests that caused it */
				if (failART.diagnoseUseCD4TestsConfirm &&
					(patient->getARTState()->numFailedOIs >= failART.OIsMinNum))
						resetARTFailedOIs();
			}
		}
		else {
			const SimContext::PedsInputs::ARTFailPolicy &failART=simContext->getPedsInputs()->failART[artLineNum];
			/**		- Has the percentage CD4% drop from maximum been exceeded */
			if (failART.CD4PercPercentageDrop != SimContext::NOT_APPL) {
				double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
				double maxCD4Perc = patient->getARTState()->maxObservedCD4PercentageOnCurrART;
				if ((maxCD4Perc - currCD4Perc) / maxCD4Perc >= failART.CD4PercPercentageDrop) {
					failedCD4 = true;
				}
			}
			/** 	- Has the CD4% count dropped below the pre-ART nadir */
			if (!failedCD4 && failART.CD4PercBelowPreARTNadir) {
				double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
				double minCD4Perc = patient->getMonitoringState()->minObservedCD4Percentage;
				if (currCD4Perc <= minCD4Perc) {
					failedCD4 = true;
				}
			}
			/**		- Is CD4% level outside the absolute OR bounds */
			if (!failedCD4 && (failART.CD4PercBoundsOR[SimContext::LOWER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
				if (currCD4Perc < failART.CD4PercBoundsOR[SimContext::LOWER_BOUND]) {
					failedCD4 = true;
				}
			}
			if (!failedCD4 && (failART.CD4PercBoundsOR[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
				if (currCD4Perc > failART.CD4PercBoundsOR[SimContext::UPPER_BOUND]) {
					failedCD4 = true;
				}
			}

			/**		- If failed criteria, evaluate CD4 level outside the absolute AND bounds */
			if (failedCD4 && (failART.CD4PercBoundsAND[SimContext::LOWER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
				if (currCD4Perc >= failART.CD4PercBoundsAND[SimContext::LOWER_BOUND]) {
					failedCD4 = false;
				}
			}
			if (failedCD4 && (failART.CD4PercBoundsAND[SimContext::UPPER_BOUND] != SimContext::NOT_APPL)) {
				double currCD4Perc = patient->getMonitoringState()->currObservedCD4Percentage;
				if (currCD4Perc <= failART.CD4PercBoundsAND[SimContext::UPPER_BOUND]) {
					failedCD4 = false;
				}
			}
			/**		- If failed criteria, evaluate if months on ART has been reached */
			if (failedCD4 && (failART.CD4PercMonthsFromInit > 0)) {
				int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
				if (monthsOnART < failART.CD4PercMonthsFromInit) {
					failedCD4 = false;
				}
			}

			/** If failed the CD4 test, increment the number of failed tests */
			if (failedCD4) {
				incrementARTFailedCD4Tests();

				if (failART.diagnoseUseHVLTestsConfirm) {
					/** If using HVL confirmatory tests, trigger HVL test if the immunologic criteria has been met */
					if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail)
						patient->getHVLTestUpdater()->performMonthlyUpdates();
				}
				else {
					/** If not using HVL confirmatory testing, check for only immunologic failure */
					if (patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsFail)
						scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
					if (failART.diagnoseUseCD4TestsConfirm) {
						/** If using CD4 confirmatory testing, trigger an emergency clinic visit if this
						/*	failed test confirms the clinical failure */
						if ((patient->getARTState()->numFailedOIs >= failART.OIsMinNum) &&
							(patient->getARTState()->numFailedCD4Tests >= failART.diagnoseNumTestsConfirm))
								scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
					}
				}
			}
			else {
				/** If test was not failed, reset the count of failed tests if there were previous failures */
				if (patient->getARTState()->numFailedCD4Tests > 0)
					resetARTFailedCD4Tests();
				/** If this was a confirmatory CD4 test, also reset the number of failed clinical
				//	tests that caused it */
				if (failART.diagnoseUseCD4TestsConfirm &&
					(patient->getARTState()->numFailedOIs >= failART.OIsMinNum))
						resetARTFailedOIs();
			}
		}


	}

	/** If this was a scheduled CD4 test, determine if another one should be scheduled */
	if (patient->getMonitoringState()->hasScheduledCD4Test &&
		(patient->getGeneralState()->monthNum >= patient->getMonitoringState()->monthOfScheduledCD4Test)) {
		int testingInterval;
		if (!patient->getARTState()->hasTakenART) {
			if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT) {
				/** - If Patient has not yet begun ART, use CD4 threshold to determine testing interval */
				if (patient->getMonitoringState()->currObservedCD4 > simContext->getTreatmentInputs()->testingIntervalCD4Threshold) {
					testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalPreARTHighCD4;
				}
				else {
					testingInterval = simContext->getTreatmentInputs()->CD4TestingIntervalPreARTLowCD4;
				}
			}
			else if (patient->getPedsState()->ageCategoryPediatrics == SimContext::PEDS_AGE_LATE) {
				testingInterval = simContext->getPedsInputs()->CD4TestingIntervalPreARTLate;
			}
			else {
				testingInterval = simContext->getPedsInputs()->CD4TestingIntervalPreARTEarly;
			}
		}
		else if (patient->getARTState()->hasNextRegimenAvailable) {
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
} /* end performMonthlyUpdates */
