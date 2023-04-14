#include "include.h"

/** \brief Constructor takes in the patient object */
DrugEfficacyUpdater::DrugEfficacyUpdater(Patient *patient) : StateUpdater(patient) {

}

/** \brief Destructor is empty, no cleanup required */
DrugEfficacyUpdater::~DrugEfficacyUpdater(void) {

}

/** \brief performInitialUpdates perform all of the state and statistics updates upon patient creation */
void DrugEfficacyUpdater::performInitialUpdates() {
	/** Calls the parent function to perform general updates and initialization */
	StateUpdater::performInitialUpdates();
} /* end performInitialUpdates */

/** \brief performMonthlyUpdates perform all of the state and statistics updates for a simulated month
 *
 * - If on ART, calls DrugEfficacyUpdater::performARTEfficacyUpdates
 * - If either CD4 envelope or CD4 percentage envelope is active, calls DrugEfficacyUpdater::performARTEnvelopeEfficacyUpdates
 * - If on any (non-TB) OI prophs, calls DrugEfficacyUpdater::performProphEfficacyUpdates
 **/
void DrugEfficacyUpdater::performMonthlyUpdates() {
	if (patient->getARTState()->isOnART)
		performARTEfficacyUpdates();
	if (patient->getARTState()->overallCD4Envelope.isActive ||
		patient->getARTState()->overallCD4PercentageEnvelope.isActive)
			performARTEnvelopeEfficacyUpdates();
	if (patient->getProphState()->currTotalNumProphsOn > 0)
		performProphEfficacyUpdates();
} /* end performMonthlyUpdates */

/** \brief performARTEfficacyUpdates handles all efficacy updates for ART regimens */
void DrugEfficacyUpdater::performARTEfficacyUpdates() {

	int artLineNum = patient->getARTState()->currRegimenNum;
	int subRegNum = patient->getARTState()->currSubRegimenNum;
	SimContext::ART_EFF_TYPE efficacy = patient->getARTState()->currRegimenEfficacy;
	int monthsEfficacy = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfEfficacyChange;
	SimContext::RESP_TYPE responseType = patient->getARTState()->responseTypeCurrRegimen[SimContext::HET_OUTCOME_RESIST];
	SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;

	const SimContext::ARTInputs *artInputs = simContext->getARTInputs(artLineNum);
	const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(artLineNum);
	const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(artLineNum);
	SimContext::PEDS_AGE_CAT pedsAgeCat = patient->getPedsState()->ageCategoryPediatrics;
	int ayaAgeCat = getAgeCategoryAdolescent();

	/** Increment the number of months on failed ART for resistance,
	/*	only count if patient is a full/partial responder */
	if ((efficacy != SimContext::ART_EFF_SUCCESS) && (responseType != SimContext::RESP_TYPE_NON)) {
		incrementMonthsUnsuccessfulART();
	}

	/** Check if patient is suppressed and has reached the next CD4 slope time segment, if so redraw slope */
	if (efficacy == SimContext::ART_EFF_SUCCESS) {
		if (patient->getGeneralState()->isAdolescent){
			int ayaARTAgeCat = getAgeCategoryAdolescentART(artLineNum);
			for (int i = 0; i < 2; i++) {
				if (monthsEfficacy == ayaART->stageBoundsCD4ChangeOnSuppART[i]) {
					double cd4SlopeMean = ayaART->CD4ChangeOnSuppARTMean[i + 1][ayaARTAgeCat];
					double cd4SlopeStdDev = ayaART->CD4ChangeOnSuppARTStdDev[i + 1][ayaARTAgeCat];
					double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70010, patient);
					setCurrRegimenCD4Slope(cd4Slope);
				}
			}

		}
		else if (pedsAgeCat == SimContext::PEDS_AGE_ADULT) {
			for (int i = 0; i < 2; i++) {
				if (monthsEfficacy == artInputs->stageBoundsCD4ChangeOnSuppART[i]) {
					double cd4SlopeMean = artInputs->CD4ChangeOnSuppARTMean[cd4Response][i + 1];
					double cd4SlopeStdDev = artInputs->CD4ChangeOnSuppARTStdDev[cd4Response][i + 1];
					double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70010, patient);
					setCurrRegimenCD4Slope(cd4Slope);
				}
			}
		}
		else if (pedsAgeCat == SimContext::PEDS_AGE_LATE) {
			for (int i = 0; i < 2; i++) {
				if (monthsEfficacy == pedsART->stageBoundsCD4ChangeOnSuppARTLate[i]) {
					double cd4SlopeMean = pedsART->CD4ChangeOnSuppARTMeanLate[cd4Response][i + 1];
					double cd4SlopeStdDev = pedsART->CD4ChangeOnSuppARTStdDevLate[cd4Response][i + 1];
					double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70011, patient);

					setCurrRegimenCD4Slope(cd4Slope);

				}
			}
		}
		else {
			for (int i = 0; i < 2; i++) {
				if (monthsEfficacy == pedsART->stageBoundsCD4PercentageChangeOnSuppARTEarly[i]) {
					int ageStartedEfficacyChange=patient->getGeneralState()->ageMonths-monthsEfficacy;
					// for Peds early childhood, the slope comes from the age category they were in when they started successful ART, not their current age category
					SimContext::PEDS_AGE_CAT pedsAgeCatStartedEfficacy=getAgeCategoryPediatrics(ageStartedEfficacyChange);
					double cd4PercSlopeMean = pedsART->CD4PercentageChangeOnSuppARTMeanEarly[pedsAgeCatStartedEfficacy][cd4Response][i + 1];
					double cd4PercSlopeStdDev = pedsART->CD4PercentageChangeOnSuppARTStdDevEarly[pedsAgeCatStartedEfficacy][cd4Response][i + 1];
					double cd4PercSlope = CepacUtil::getRandomGaussian(cd4PercSlopeMean, cd4PercSlopeStdDev, 70012, patient);

					setCurrRegimenCD4PercentageSlope(cd4PercSlope);
				}
			}
		}
	}

	/** Trigger an emergency clinic visit if max months on ART or months to
	/*	subregimen switch has been reached */
	int monthsOnART = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart;
	if (patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
		const SimContext::TreatmentInputs::ARTStopPolicy &stopART = simContext->getTreatmentInputs()->stopART[artLineNum];
		if ((stopART.maxMonthsOnART != SimContext::NOT_APPL) &&
			(monthsOnART >= stopART.maxMonthsOnART)) {
				scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
		}
	}
	else{
		const SimContext::PedsInputs::ARTStopPolicy &stopART = simContext->getPedsInputs()->stopART[artLineNum];
		if ((stopART.maxMonthsOnART != SimContext::NOT_APPL) &&
			(monthsOnART >= stopART.maxMonthsOnART)) {
				scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
		}
	}

	int monthsOnSubReg = patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrSubRegimenStart;
	if ((artInputs->monthsToSwitchSubRegimen[subRegNum] != SimContext::NOT_APPL) &&
		(monthsOnSubReg >= artInputs->monthsToSwitchSubRegimen[subRegNum])) {
			scheduleEmergencyClinicVisit(SimContext::EMERGENCY_ART, patient->getGeneralState()->monthNum);
	}

	/** Check if force failure month has been reached if it is set */
	if (efficacy != SimContext::ART_EFF_FAILURE) {
		bool forceFail = false;
		if(patient->getGeneralState()->isAdolescent){
			if((ayaART->forceFailAtMonth != SimContext::NOT_APPL) && monthsOnART >= ayaART->forceFailAtMonth)
				forceFail = true;
		}
		else if (pedsAgeCat == SimContext::PEDS_AGE_ADULT){
			if ((artInputs->forceFailAtMonth != SimContext::NOT_APPL) &&
			(monthsOnART >= artInputs->forceFailAtMonth))
				forceFail = true;
		}
		else if (pedsAgeCat == SimContext::PEDS_AGE_LATE) {
			if ((pedsART->forceFailAtMonthLate != SimContext::NOT_APPL) &&
			(monthsOnART >= pedsART->forceFailAtMonthLate))
				forceFail = true;
		}
		else if ((pedsART->forceFailAtMonthEarly != SimContext::NOT_APPL) &&
			(monthsOnART >= pedsART->forceFailAtMonthEarly))
				forceFail = true;
		if (forceFail) {
			/** If forcing fail:
			 *  - Set the efficacy to failure
			 */
			setCurrARTEfficacy(SimContext::ART_EFF_FAILURE, false);
			/** - Set the target HVL to the setpoint HVL */
			setTargetHVLStrata(patient->getDiseaseState()->setpointHVLStrata);
			/** - Output tracing if enabled */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d ART LATE FAIL;\n", patient->getGeneralState()->monthNum);
			}
			return;
		}
	}

	/** If this is the patient's initial suppression on the current regimen
	//	and it is within the efficacy time horizon, do not roll for any late transitions */
	if (patient->getGeneralState()->isAdolescent){
		int efficacyHorizon=ayaART->efficacyTimeHorizon[ayaAgeCat];

		if (patient->getARTState()->hadSuccessOnRegimen &&
				(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart <= efficacyHorizon)){
			//Only have efficacy time horizon for initial attempt or resupp attempt not RTC; return if within the efficacy time horizon	
			if(patient->getARTState()->isOnResupp || patient->getARTState()->monthOfCurrRegimenStart == patient->getARTState()->monthOfInitialRegimenSuccess){
				return;
			}
		}
	}
	else if (pedsAgeCat == SimContext::PEDS_AGE_ADULT) {
		int efficacyHorizon;
		if (patient->getARTState()->isOnResupp)
			efficacyHorizon = artInputs->efficacyTimeHorizonResuppression;
		else
			efficacyHorizon = artInputs->efficacyTimeHorizon;

		if (patient->getARTState()->hadSuccessOnRegimen &&
				(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart <= efficacyHorizon)){
			//Only have efficacy time horizon for initial attempt or resupp attempt not RTC; return if within the efficacy time horizon
			if(patient->getARTState()->isOnResupp || patient->getARTState()->monthOfCurrRegimenStart == patient->getARTState()->monthOfInitialRegimenSuccess){
				return;
			}
		}

	}
	else if (pedsAgeCat == SimContext::PEDS_AGE_LATE) {
		int efficacyHorizon=pedsART->efficacyTimeHorizonLate;
		if (patient->getARTState()->hadSuccessOnRegimen &&
				(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart <= efficacyHorizon)){
			//Only have efficacy time horizon for initial attempt or resupp attempt not RTC; return if within the efficacy time horizon
			if(patient->getARTState()->isOnResupp || patient->getARTState()->monthOfCurrRegimenStart == patient->getARTState()->monthOfInitialRegimenSuccess){
				return;
			}
		}
	}
	else {
		int efficacyHorizon=pedsART->efficacyTimeHorizonEarly;
		if (patient->getARTState()->hadSuccessOnRegimen &&
				(patient->getGeneralState()->monthNum - patient->getARTState()->monthOfCurrRegimenStart <= efficacyHorizon)){
			//Only have efficacy time horizon for initial attempt or resupp attempt not RTC; return if within the efficacy time horizon
			if (patient->getARTState()->isOnResupp || patient->getARTState()->monthOfCurrRegimenStart == patient->getARTState()->monthOfInitialRegimenSuccess){
				return;
			}
		}
	}


	if (efficacy == SimContext::ART_EFF_SUCCESS) {
		/** If patient is suppressed, roll for late fail transition */
		double probFail = 0.0;
		probFail = patient->getARTState()->probLateFail;

		double randNum = CepacUtil::getRandomDouble(70020, patient);
		if ((probFail > 0) && (randNum < probFail)) {
			/** If fail:
			* - Set the efficacy to failure */
			setCurrARTEfficacy(SimContext::ART_EFF_FAILURE, false);
			/** - Set the target HVL to the setpoint HVL */
			setTargetHVLStrata(patient->getDiseaseState()->setpointHVLStrata);
			/** - Output tracing if enabled */
			if (patient->getGeneralState()->tracingEnabled) {
				tracer->printTrace(1, "**%d ART LATE FAIL;\n", patient->getGeneralState()->monthNum);
			}
		}
	}

} /* end performARTEfficacyUpdates */

/** \brief performARTEnvelopeEfficacyUpdates handles the efficacy updates of the ART CD4 envelope */
void DrugEfficacyUpdater::performARTEnvelopeEfficacyUpdates() {
	SimContext::PEDS_AGE_CAT pedsAgeCat = patient->getPedsState()->ageCategoryPediatrics;

	if (patient->getGeneralState()->isAdolescent){
		/** Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope */
		int overallEnvLineNum = patient->getARTState()->overallCD4Envelope.regimenNum;
		int monthOverallEnvStart = patient->getARTState()->overallCD4Envelope.monthOfStart;
		int monthsSinceOverallEnv = patient->getGeneralState()->monthNum - monthOverallEnvStart;
		bool hasIndivEnv = patient->getARTState()->indivCD4Envelope.isActive;
		int indivEnvLineNum = patient->getARTState()->indivCD4Envelope.regimenNum;
		const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(overallEnvLineNum);
		int ayaARTAgeCat = getAgeCategoryAdolescentART(overallEnvLineNum);

		for (int i = 0; i < 2; i++) {
			if (monthsSinceOverallEnv == ayaART->stageBoundsCD4ChangeOnSuppART[i]) {
				/** If patient is still suppressed on the regimen that is setting the envelope,
				//	use the current CD4 slope as the CD4 envelope slope */
				if (patient->getARTState()->isOnART &&
					(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
					(patient->getARTState()->currRegimenNum == overallEnvLineNum) &&
					(patient->getARTState()->monthOfCurrRegimenStart == monthOverallEnvStart)) {
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, patient->getARTState()->currRegimenCD4Slope);
						if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
				}
				else {
					/** Otherwise, draw for a new CD4 envelope slope */
					double cd4SlopeMean = ayaART->CD4ChangeOnSuppARTMean[i + 1][ayaARTAgeCat];
					double cd4SlopeStdDev = ayaART->CD4ChangeOnSuppARTStdDev[i + 1][ayaARTAgeCat];
					double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70050, patient);

					setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
					if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
				}
			}
		}

		/** Update the individual regimen CD4 envelope */
		if (hasIndivEnv && (indivEnvLineNum != overallEnvLineNum)) {
			int monthIndivEnvStart = patient->getARTState()->indivCD4Envelope.monthOfStart;
			int monthsSinceIndivEnv = patient->getGeneralState()->monthNum - monthIndivEnvStart;
			/** Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope */
			const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(indivEnvLineNum);
			for (int i = 0; i < 2; i++) {
				if (monthsSinceIndivEnv == ayaART->stageBoundsCD4ChangeOnSuppART[i]) {
					/** If patient is still suppressed on the regimen that is setting the envelope,
					//	use the current CD4 slope as the CD4 envelope slope */
					if (patient->getARTState()->isOnART &&
						(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
						(patient->getARTState()->currRegimenNum == indivEnvLineNum) &&
						(patient->getARTState()->monthOfCurrRegimenStart == monthIndivEnvStart)) {
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
					}
					else {
						/** Otherwise, draw for a new CD4 envelope slope */
						double cd4SlopeMean = ayaART->CD4ChangeOnSuppARTMean[i + 1][ayaARTAgeCat];
						double cd4SlopeStdDev = ayaART->CD4ChangeOnSuppARTStdDev[i + 1][ayaARTAgeCat];
						double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70051, patient);

						setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
					}
				}
			}
		}
	}
	else if (pedsAgeCat == SimContext::PEDS_AGE_ADULT) {
		/** Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope */
		int overallEnvLineNum = patient->getARTState()->overallCD4Envelope.regimenNum;
		int monthOverallEnvStart = patient->getARTState()->overallCD4Envelope.monthOfStart;
		int monthsSinceOverallEnv = patient->getGeneralState()->monthNum - monthOverallEnvStart;
		bool hasIndivEnv = patient->getARTState()->indivCD4Envelope.isActive;
		int indivEnvLineNum = patient->getARTState()->indivCD4Envelope.regimenNum;
		const SimContext::ARTInputs *artInputs = simContext->getARTInputs(overallEnvLineNum);
		for (int i = 0; i < 2; i++) {
			if (monthsSinceOverallEnv == artInputs->stageBoundsCD4ChangeOnSuppART[i]) {
				/** If patient is still suppressed on the regimen that is setting the envelope,
				//	use the current CD4 slope as the CD4 envelope slope */
				if (patient->getARTState()->isOnART &&
					(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
					(patient->getARTState()->currRegimenNum == overallEnvLineNum) &&
					(patient->getARTState()->monthOfCurrRegimenStart == monthOverallEnvStart)) {
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, patient->getARTState()->currRegimenCD4Slope);
						if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
				}
				else {
					/** Otherwise, draw for a new CD4 envelope slope */
					SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
					double cd4SlopeMean = artInputs->CD4ChangeOnSuppARTMean[cd4Response][i + 1];
					double cd4SlopeStdDev = artInputs->CD4ChangeOnSuppARTStdDev[cd4Response][i + 1];
					double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70050, patient);

					setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
					if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
				}
			}
		}

		/** Update the individual regimen CD4 envelope */
		if (hasIndivEnv && (indivEnvLineNum != overallEnvLineNum)) {
			int monthIndivEnvStart = patient->getARTState()->indivCD4Envelope.monthOfStart;
			int monthsSinceIndivEnv = patient->getGeneralState()->monthNum - monthIndivEnvStart;
			/** Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope */
			const SimContext::ARTInputs *artInputs = simContext->getARTInputs(indivEnvLineNum);
			for (int i = 0; i < 2; i++) {
				if (monthsSinceIndivEnv == artInputs->stageBoundsCD4ChangeOnSuppART[i]) {
					/** If patient is still suppressed on the regimen that is setting the envelope,
					//	use the current CD4 slope as the CD4 envelope slope */
					if (patient->getARTState()->isOnART &&
						(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
						(patient->getARTState()->currRegimenNum == indivEnvLineNum) &&
						(patient->getARTState()->monthOfCurrRegimenStart == monthIndivEnvStart)) {
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
					}
					else {
						/** Otherwise, draw for a new CD4 envelope slope */
						SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
						double cd4SlopeMean = artInputs->CD4ChangeOnSuppARTMean[cd4Response][i + 1];
						double cd4SlopeStdDev = artInputs->CD4ChangeOnSuppARTStdDev[cd4Response][i + 1];
						double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70051, patient);

						setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
					}
				}
			}
		}
	}
	else if (pedsAgeCat == SimContext::PEDS_AGE_LATE) {
		// Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope
		int overallEnvLineNum = patient->getARTState()->overallCD4Envelope.regimenNum;
		int monthOverallEnvStart = patient->getARTState()->overallCD4Envelope.monthOfStart;
		int monthsSinceOverallEnv = patient->getGeneralState()->monthNum - monthOverallEnvStart;
		bool hasIndivEnv = patient->getARTState()->indivCD4Envelope.isActive;
		int indivEnvLineNum = patient->getARTState()->indivCD4Envelope.regimenNum;

		const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(overallEnvLineNum);
		for (int i = 0; i < 2; i++) {
			if (monthsSinceOverallEnv == pedsART->stageBoundsCD4ChangeOnSuppARTLate[i]) {
				// If patient is still suppressed on the regimen that is setting the envelope,
				//	use the current CD4 slope as the CD4 envelope slope
				if (patient->getARTState()->isOnART &&
					(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
					(patient->getARTState()->currRegimenNum == overallEnvLineNum) &&
					(patient->getARTState()->monthOfCurrRegimenStart == monthOverallEnvStart)) {
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, patient->getARTState()->currRegimenCD4Slope);
						if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
				}
				else {
					// Otherwise, draw for a new CD4 envelope slope
					SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
					double cd4SlopeMean = pedsART->CD4ChangeOnSuppARTMeanLate[cd4Response][i + 1];
					double cd4SlopeStdDev = pedsART->CD4ChangeOnSuppARTStdDevLate[cd4Response][i + 1];
					double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70052, patient);

					setCD4EnvelopeSlope(SimContext::ENVL_CD4_OVERALL, cd4Slope);
					if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
				}
			}
		}

		// Update the individual regimen CD4 envelope
		if (hasIndivEnv && (indivEnvLineNum != overallEnvLineNum)) {
			int monthIndivEnvStart = patient->getARTState()->indivCD4Envelope.monthOfStart;
			int monthsSinceIndivEnv = patient->getGeneralState()->monthNum - monthIndivEnvStart;
			// Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope

			const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(indivEnvLineNum);
			for (int i = 0; i < 2; i++) {
				if (monthsSinceIndivEnv == pedsART->stageBoundsCD4ChangeOnSuppARTLate[i]) {
					// If patient is still suppressed on the regimen that is setting the envelope,
					//	use the current CD4 slope as the CD4 envelope slope
					if (patient->getARTState()->isOnART &&
						(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
						(patient->getARTState()->currRegimenNum == indivEnvLineNum) &&
						(patient->getARTState()->monthOfCurrRegimenStart == monthIndivEnvStart)) {
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, patient->getARTState()->currRegimenCD4Slope);
					}
					else {
						// Otherwise, draw for a new CD4 envelope slope
						SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
						double cd4SlopeMean = pedsART->CD4ChangeOnSuppARTMeanLate[cd4Response][i + 1];
						double cd4SlopeStdDev = pedsART->CD4ChangeOnSuppARTStdDevLate[cd4Response][i + 1];
						double cd4Slope = CepacUtil::getRandomGaussian(cd4SlopeMean, cd4SlopeStdDev, 70053, patient);

						setCD4EnvelopeSlope(SimContext::ENVL_CD4_INDIV, cd4Slope);
					}
				}
			}
		}
	}
	else {
		/** For Peds early childhood, use the same process as above, but with the CD4 Percentage envelope */
		// Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope
		int overallEnvLineNum = patient->getARTState()->overallCD4PercentageEnvelope.regimenNum;
		int monthOverallEnvStart = patient->getARTState()->overallCD4PercentageEnvelope.monthOfStart;
		int monthsSinceOverallEnv = patient->getGeneralState()->monthNum - monthOverallEnvStart;
		bool hasIndivEnv = patient->getARTState()->indivCD4PercentageEnvelope.isActive;
		int indivEnvLineNum = patient->getARTState()->indivCD4PercentageEnvelope.regimenNum;
		const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(overallEnvLineNum);
		for (int i = 0; i < 2; i++) {
			if (monthsSinceOverallEnv == pedsART->stageBoundsCD4PercentageChangeOnSuppARTEarly[i]) {
				// If patient is still suppressed on the regimen that is setting the envelope,
				//	use the current CD4 slope as the CD4 envelope slope
				if (patient->getARTState()->isOnART &&
					(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
					(patient->getARTState()->currRegimenNum == overallEnvLineNum) &&
					(patient->getARTState()->monthOfCurrRegimenStart == monthOverallEnvStart)) {
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_OVERALL, patient->getARTState()->currRegimenCD4PercentageSlope);
						if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_INDIV, patient->getARTState()->currRegimenCD4PercentageSlope);
				}
				else {
					// Otherwise, draw for a new CD4 envelope slope
					SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
					double cd4PercSlopeMean = pedsART->CD4PercentageChangeOnSuppARTMeanEarly[pedsAgeCat][cd4Response][i + 1];
					double cd4PercSlopeStdDev = pedsART->CD4PercentageChangeOnSuppARTStdDevEarly[pedsAgeCat][cd4Response][i + 1];
					double cd4PercSlope = CepacUtil::getRandomGaussian(cd4PercSlopeMean, cd4PercSlopeStdDev, 70054, patient);

					setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_OVERALL, cd4PercSlope);
					if (hasIndivEnv && (overallEnvLineNum == indivEnvLineNum))
						setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_INDIV, cd4PercSlope);
				}
			}
		}

		// Update the individual regimen CD4 envelope
		if (hasIndivEnv && (indivEnvLineNum != overallEnvLineNum)) {
			int monthIndivEnvStart = patient->getARTState()->indivCD4PercentageEnvelope.monthOfStart;
			int monthsSinceIndivEnv = patient->getGeneralState()->monthNum - monthIndivEnvStart;
			// Check if patient has reached the next CD4 envelope slope time segment, if so redraw slope
			const SimContext::PedsARTInputs *pedsART = simContext->getPedsARTInputs(indivEnvLineNum);
			for (int i = 0; i < 2; i++) {
				if (monthsSinceIndivEnv == pedsART->stageBoundsCD4PercentageChangeOnSuppARTEarly[i]) {
					// If patient is still suppressed on the regimen that is setting the envelope,
					//	use the current CD4 slope as the CD4 envelope slope
					if (patient->getARTState()->isOnART &&
						(patient->getARTState()->currRegimenEfficacy == SimContext::ART_EFF_SUCCESS) &&
						(patient->getARTState()->currRegimenNum == indivEnvLineNum) &&
						(patient->getARTState()->monthOfCurrRegimenStart == monthIndivEnvStart)) {
							setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_INDIV, patient->getARTState()->currRegimenCD4PercentageSlope);
					}
					else {
						// Otherwise, draw for a new CD4 envelope slope
						SimContext::CD4_RESPONSE_TYPE cd4Response = patient->getARTState()->CD4ResponseType;
						double cd4PercSlopeMean = pedsART->CD4PercentageChangeOnSuppARTMeanEarly[pedsAgeCat][cd4Response][i + 1];
						double cd4PercSlopeStdDev = pedsART->CD4PercentageChangeOnSuppARTStdDevEarly[pedsAgeCat][cd4Response][i + 1];
						double cd4PercSlope = CepacUtil::getRandomGaussian(cd4PercSlopeMean, cd4PercSlopeStdDev, 70055, patient);

						setCD4EnvelopeSlope(SimContext::ENVL_CD4_PERC_INDIV, cd4PercSlope);
					}
				}
			}
		}
	}
} /* end performARTEnvelopeEfficacyUpdates */

/** \brief performProphEfficacyUpdates handles all efficacy updates for prophylaxis */
void DrugEfficacyUpdater::performProphEfficacyUpdates() {
	/** Loop over all prophs that the patient is currently on */
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->getProphState()->isOnProph[i]) {
			SimContext::PROPH_TYPE prophType = patient->getProphState()->currProphType[i];
			int prophNum = patient->getProphState()->currProphNum[i];
			const SimContext::ProphInputs *prophInputs;
			if(patient->getPedsState()->ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
				prophInputs=simContext->getProphInputs(prophType, i, prophNum);
			}
			else{
				prophInputs=simContext->getPedsProphInputs(prophType, i, prophNum);
			}
			/** Trigger an emergency clinic visit if max month number, months on proph,
			//	or month to switch proph line has been reached */
			int monthsOnProph = patient->getGeneralState()->monthNum - patient->getProphState()->monthOfProphStart[i];
			const SimContext::TreatmentInputs::ProphStopPolicy &stopProph = simContext->getTreatmentInputs()->stopProph[prophType][i];
			if ((stopProph.minMonthNum != SimContext::NOT_APPL)&&
				(patient->getGeneralState()->monthNum >= stopProph.minMonthNum)) {
					scheduleEmergencyClinicVisit(SimContext::EMERGENCY_PROPH, patient->getGeneralState()->monthNum);
			}
			else if ((stopProph.monthsOnProph != SimContext::NOT_APPL) &&
				(monthsOnProph >= stopProph.monthsOnProph)) {
					scheduleEmergencyClinicVisit(SimContext::EMERGENCY_PROPH, patient->getGeneralState()->monthNum);
			}
			else if ((prophInputs->monthsToSwitch != SimContext::NOT_APPL) &&
				(monthsOnProph >= prophInputs->monthsToSwitch)) {
					scheduleEmergencyClinicVisit(SimContext::EMERGENCY_PROPH, patient->getGeneralState()->monthNum);
			}

			/** Evaluate if proph resistance should begin for this drug */
			if (!patient->getProphState()->useProphResistance[i]) {
				double resistTime = prophInputs->timeOfResistance;
				if (patient->getProphState()->isNonCompliant)
					resistTime = resistTime / (1 - simContext->getCohortInputs()->OIProphNonComplianceDegree);
				if (patient->getGeneralState()->monthNum - patient->getProphState()->monthOfProphStart[i] > resistTime) {
					double randNum = CepacUtil::getRandomDouble(70060, patient);
					if (randNum < prophInputs->monthlyProbResistance) {
						setProphResistance((SimContext::OI_TYPE) i);
					}
				}
			}
		}
	}
} /* end performProphEfficacyUpdates */

