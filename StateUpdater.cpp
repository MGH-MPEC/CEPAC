#include "include.h"

/** Constructor takes in the patient object */
StateUpdater::StateUpdater(Patient *patient) :
		patient(patient)
{

}

/** Destructor is empty, no cleanup required */
StateUpdater::~StateUpdater(void) {

}

/**
 * \brief Virtual function to perform the initial updates upon patient creation.
 * Sets the simContext, runStats, and tracer to match that of this->patient */
void StateUpdater::performInitialUpdates() {
	// Copy the pointers to the simContext, runStats, and tracer objects
	this->simContext = this->patient->simContext;
	this->runStats = this->patient->runStats;
	this->costStats = this->patient->costStats;
	this->tracer = this->patient->tracer;
}

/**
 * \brief Virtual function to perform all updates for a simulated month.
 * Empty for now, no actions to perform if child does not override
 * */
void StateUpdater::performMonthlyUpdates() {
	// Empty for now, no actions to perform if child does not override
}

/** Virtual function changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model */
void StateUpdater::setSimContext(SimContext *newSimContext){
	this->simContext = newSimContext;
}

/** \brief initializePatient initializes the patient's basic state
 * \param patientNum an integer argument that assigns the patient a (hopefully unique!) identifier
 * \param tracingEnabled a bool argument that is true if this patient is to be output in the trace file
 *
 * This function initializes patient->monthNum to the initial month number (0 unless otherwise specified by the transmission model),
 * all costs to 0, all life months (LMs) and discounted LMs to 0, the discount factor to 1, and marks the patient as alive */
void StateUpdater::initializePatient(int patientNum, bool tracingEnabled) {
	patient->generalState.patientNum = patientNum;
	patient->generalState.tracingEnabled = tracingEnabled;
	patient->generalState.monthNum = patient->generalState.initialMonthNum;
	patient->generalState.costsDiscounted = 0;
	patient->artState.costsART = 0;
	patient->monitoringState.costsCD4Testing = 0;
	patient->monitoringState.costsHVLTesting = 0;
	patient->monitoringState.costsPrEP = 0;
	patient->monitoringState.costsHIVTesting = 0;
	patient->generalState.LMsDiscounted = 0;
	patient->generalState.qualityAdjustLMsDiscounted = 0;
	patient->generalState.LMsUndiscounted = 0;
	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		patient->generalState.multDiscCosts[i] = 0;
		patient->generalState.multDiscLMs[i] = 0;
		patient->generalState.multDiscQALMs[i] = 0;
		patient->generalState.multDiscFactorCost[i] = 1.0;
		patient->generalState.multDiscFactorBenefit[i] = 1.0;
	}
	patient->generalState.discountFactor = 1.0;
	patient->generalState.loggedPatientOIs = false;
	patient->diseaseState.isAlive = true;
	patient->diseaseState.useHEUMortality = false;
	patient->diseaseState.neverExposed = false;
}

/** \brief setPatientAgeGender set the patients age and gender
 * \param gender a SimContext::GENDER_TYPE (male or female)
 * \param ageMonths an integer specifying the patient's initial age
 *
 * The patient's age categories are also set using helper functions
 * \see StateUpdater::getAgeCategoryHIVInfection(int)
 * \see StateUpdater::getAgeCategoryCHRMs(int)
 * \see StateUpdater::getAgeCategoryPediatrics(int) */
void StateUpdater::setPatientAgeGender(SimContext::GENDER_TYPE gender, int ageMonths) {
	patient->generalState.gender = gender;
	patient->generalState.ageMonths = ageMonths;
	patient->generalState.ageCategoryHIVInfection = getAgeCategoryHIVInfection(ageMonths);
	patient->generalState.ageCategoryHeterogeneity = getAgeCategoryHeterogeneity(ageMonths);
	patient->generalState.ageCategoryCost = getAgeCategoryCost(ageMonths);
	patient->pedsState.ageCategoryCD4Metric = getAgeCategoryCD4Metric(ageMonths);
	patient->pedsState.ageCategoryPediatrics = getAgeCategoryPediatrics(ageMonths);
	patient->pedsState.ageCategoryPedsCost = getAgeCategoryPediatricsCost(ageMonths);
	patient->pedsState.ageCategoryPedsARTCost = getAgeCategoryPediatricsARTCost(ageMonths);
} /* end setPatientAgeGender */

/** \brief setInitialARTState initializes the patients ARTState object
 *
 *  Initializes all ART variables: by default, isOnART and hasTakenART are false and number of observed
 *  failures is 0.  The CD4 envelope and CD4 percentage envelope is set to non-active, all
 *  "number of months since" data relating to ART is set to 0, and all toxicity effects are cleared.
 **/
void StateUpdater::setInitialARTState() {
	patient->artState.isOnART = false;
	patient->artState.applyARTEffect = false;
	patient->artState.isOnResupp = false;
	patient->artState.numFailedResupp = 0;
	patient->artState.hasTakenART = false;
	patient->artState.numObservedFailures = 0;
	patient->artState.hadSuccessOnRegimen = false;

	patient->artState.overallCD4Envelope.isActive = false;
	patient->artState.indivCD4Envelope.isActive = false;
	patient->artState.overallCD4PercentageEnvelope.isActive = false;
	patient->artState.indivCD4PercentageEnvelope.isActive = false;
	patient->artState.hadPrevToxicity = false;

	patient->artState.monthOfNewCD4MultArtFail=0;
	patient->artState.currCD4MultArtFail=1.0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		patient->artState.numObservedOIsSinceFailOrStopART[i] = 0;
	}
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++) {
		patient->artState.numMonthsOnUnsuccessfulByRegimen[i] = 0;
		patient->artState.hasTakenARTRegimen[i] = false;
	}
	for (int i = 0; i < SimContext::HVL_NUM_STRATA; i++) {
		patient->artState.numMonthsOnUnsuccessfulByHVL[i] = 0;
	}
	patient->artState.activeToxicityEffects.clear();
} /* end setInitialARTState */

/**
 * \brief setInitialProphState initializes the patients ProphState object
 *
 * Initializes the state to reflect that no prophylaxis drugs are currently taken
 * or have a history of being taken
 * */
void StateUpdater::setInitialProphState() {

	// Initialize state to reflect that no drugs are currently taken and
	patient->prophState.currTotalNumProphsOn = 0;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		patient->prophState.isOnProph[i] = false;
		for (int j = 0; j < SimContext::PROPH_NUM_TYPES; j++){
			patient->prophState.hasTakenProph[i][j] = false;
		}
	}
} /* end setInitialProphState */

/** \brief setInitialTBProphState sets the initial TB proph state to not be on any TB prophs
 *
 *  Also sets the patient to not be scheduled to start any TB proph
 * */
void StateUpdater::setInitialTBProphState() {
	patient->tbState.isOnProph = false;
	patient->tbState.isScheduledForProph = false;
	for(int i = 0; i < SimContext::TB_NUM_PROPHS; i++){
		patient->tbState.numProphStarts[i] = 0;
	}	
	patient->tbState.hasCompletedProph = false;
	patient->tbState.isEligibleForProph = false;
	patient->tbState.hasRolledEligibleForProph = false;
} /* end setInitialTBProphState */

/** \brief setInitialTBTreatmentState sets the initial TB treatment state to not be on TB treatment
 *
 *  Also sets the patient not to be scheduled to start any TB treatment and determines initial TB treatment history for those who start the model with such a history
 * */
void StateUpdater::setInitialTBTreatmentState() {
	patient->tbState.isScheduledForTreatment = false;
	patient->tbState.isOnTreatment = false;
	patient->tbState.isOnEmpiricTreatment = false;
	patient->tbState.hasIncompleteTreatment = false;
	patient->tbState.everHadNonInitialTreatmentOrEmpiric = false;
	patient->tbState.willIncreaseResistanceUponDefault = false;
	patient->tbState.monthOfMortEfficacyStop = SimContext::NOT_APPL;
	for(int i = 0; i < SimContext::TB_NUM_UNFAVORABLE; i++)
		patient->tbState.hasUnfavorableOutcome[i] = false;
	
	if(patient->tbState.observedHistActiveTBAtEntry){
		patient->tbState.everOnTreatmentOrEmpiric = true;
		patient->tbState.hasStoppedTreatmentOrEmpiric = true;
		// Treatment stop or transition to treatment default TB state must have occurred at least 1 month ago to keep outcomes synced with those who stop treatment after model entry
		double monthsSinceStopMean = simContext->getTBInputs()->monthsSinceInitTreatStopMean;
		double monthsSinceStopStdDev = simContext->getTBInputs()->monthsSinceInitTreatStopStdDev;
		int monthsSinceInitTreatStop = (int) (CepacUtil::getRandomGaussian(monthsSinceStopMean, monthsSinceStopStdDev, 13020, patient) + 0.5);
		if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_TREAT_DEFAULT){
			patient->tbState.everCompletedTreatmentOrEmpiric = false;
			if(monthsSinceInitTreatStop < simContext->getTBInputs()->monthsToLongTermEffectsLTFU + 1){
				monthsSinceInitTreatStop = simContext->getTBInputs()->monthsToLongTermEffectsLTFU + 1;
			}
		}
		// Previously Treated
		else{
			patient->tbState.everCompletedTreatmentOrEmpiric = true;
			if(monthsSinceInitTreatStop < 1){
				monthsSinceInitTreatStop = 1;
			}	
		}	
		patient->tbState.monthOfInitialTreatmentStop = -1 * monthsSinceInitTreatStop;
		patient->tbState.monthOfTreatmentOrEmpiricStop = patient->tbState.monthOfInitialTreatmentStop;
		// Determine the initial TB treatment line they stopped before model entry - the assumption is made that their TB strain was identified correctly because they were successfully cured by this treatment
		int lineNum = SimContext::TB_NUM_TREATMENTS - 1;
		SimContext::TB_STRAIN obsvStrain = patient->tbState.currTrueTBResistanceStrain;
		double randNum = CepacUtil::getRandomDouble(13021, patient);
		for (int i = 0; i < SimContext::TB_NUM_TREATMENTS; i++) {
			if (randNum < simContext->getTBInputs()->TBTreatmentProbInitialLine[0][obsvStrain][i]) {
				lineNum = i;
				break;
			}
			randNum -= simContext->getTBInputs()->TBTreatmentProbInitialLine[0][obsvStrain][i];
		}
		patient->tbState.mostRecentTreatNum = lineNum;
	}
	else{
	patient->tbState.everOnTreatmentOrEmpiric = false;
		patient->tbState.hasStoppedTreatmentOrEmpiric = false;
		patient->tbState.everCompletedTreatmentOrEmpiric = false;
	}
} /* end setInitialTBTreatmentState */

/** \brief incrementMonth increments the simulation month number and patient age by 1
 *
 *  Also resets the age categories using helper functions
 * \see StateUpdater::getAgeCategoryHIVInfection(int)
 * \see StateUpdater::getAgeCategoryCHRMs(int)
 * \see StateUpdater::getAgeCategoryPediatrics(int)
 *
 * */
void StateUpdater::incrementMonth() {
	patient->generalState.monthNum++;
	patient->generalState.ageMonths++;

	// Update the HIV infection and pediatrics age category
	patient->generalState.ageCategoryHIVInfection = getAgeCategoryHIVInfection(patient->generalState.ageMonths);
	patient->generalState.ageCategoryHeterogeneity = getAgeCategoryHeterogeneity(patient->generalState.ageMonths);
	patient->generalState.ageCategoryCost = getAgeCategoryCost(patient->generalState.ageMonths);
	patient->pedsState.ageCategoryCD4Metric = getAgeCategoryCD4Metric(patient->generalState.ageMonths);
	patient->pedsState.ageCategoryPediatrics = getAgeCategoryPediatrics(patient->generalState.ageMonths);
	patient->pedsState.ageCategoryPedsCost = getAgeCategoryPediatricsCost(patient->generalState.ageMonths);
	patient->pedsState.ageCategoryPedsARTCost = getAgeCategoryPediatricsARTCost(patient->generalState.ageMonths);
} /* end incrementMonth */

/** \brief incrementDiscountFactor adjusts the discounting factor for each new month
 *
 * \param amount a double, the inverse of which is multiplied by the current discount factor
 *
 * \f$DiscountFactor_{new} = \frac{1}{amount} * DiscountFactor_{old}\f$
 * */
void StateUpdater::incrementDiscountFactor(double amount) {
	patient->generalState.discountFactor *= (1 / amount);
} /* end incrementDiscountFactor */

/** \brief incrementMultDiscountFactor adjusts the discounting factor for each new month when using multiple discount factors
 *
 * \param amount a double, the inverse of which is multiplied by the current discount factor
 *
 * \f$DiscountFactor_{new} = \frac{1}{amount} * DiscountFactor_{old}\f$
 * */
void StateUpdater::incrementMultDiscountFactor(double amountCost, double amountBenefit, int i) {
	patient->generalState.multDiscFactorCost[i] *= (1 / amountCost);
	patient->generalState.multDiscFactorBenefit[i] *= (1/ amountBenefit);
} /* end incrementDiscountFactor */

/** \brief resetQOL resets the quality of life factor back to the background value for the patient's age in years and gender
 **/
void StateUpdater::resetQOL(){
	int ageYears = patient->generalState.ageMonths / 12;
	SimContext::GENDER_TYPE gender = patient->generalState.gender;
	patient->generalState.QOLValue = simContext->getQOLInputs()->backgroundQOL[gender][ageYears];
}/*end resetQOL */
	
/** \brief accumulateQOLModifier accumulates the QOL in one of 4 ways, defined by user inputs: by multiplying the new factor with the existing one (MULT), subtracting the new factor from the existing one (ADD), taking the minimum of the old one and the new one (MIN), or adding the new factor to the existing one (MARGINAL)
 *
 *  \param amount a double indicating the new factor to accumulate the QOL by
 *
 *  \f$QOL_{new} = QOL_{old} * amount\f$
 *  \f$QOL_{new} = QOL_{old} - amount\f$
 *  \f$QOL_{new} = min(QOL_{old}, amount)\f$
 *  \f$QOL_{new} = QOL_{old} + amount\f$
 * */
void StateUpdater::accumulateQOLModifier(double amount) {
	if(simContext->getQOLInputs()->QOLCalculationType==SimContext::MULT){
		patient->generalState.QOLValue *= amount;
	}
	if(simContext->getQOLInputs()->QOLCalculationType==SimContext::ADD){
		patient->generalState.QOLValue -= amount;
	}
	if(simContext->getQOLInputs()->QOLCalculationType==SimContext::MIN){
		patient->generalState.QOLValue = min(patient->generalState.QOLValue, amount);
	}
	if(simContext->getQOLInputs()->QOLCalculationType==SimContext::MARGINAL){
		patient->generalState.QOLValue += amount;
	}
} /* end accumulateQOLModifier */

/** \brief finalizeQOLValue checks that the patient's QOL value is between 0 and 1 and bounds it if necessary before it is used
 * 
 **/
void StateUpdater::finalizeQOLValue(){
	if(patient->generalState.QOLValue < 0){
		patient->generalState.QOLValue = 0;
	}
	if(patient->generalState.QOLValue > 1){
		patient->generalState.QOLValue = 1;
	}
} /* end finalizeQOLValue */

/** \brief setHIVIncReducMultiplier sets the current HIV incidence reduction multiplier if incidence reduction is enabled 
 * 	\param reducMult a double indicating the multiplier to be applied for the current time period to the patient's monthly HIV infection probability
 * 
*/
void StateUpdater::setHIVIncReducMultiplier(double reducMult){
	patient->monitoringState.HIVIncReducMultiplier = reducMult;
}

/** \brief setInfectedHIVState sets the patient to the specified HIV infection state and updates statistics
 *
 *	\param infectedState a SimContext::HIV_INF (HIV infection state) to set the patient to
 *	\param isInitial a bool marking whether or not this is an initial (i.e. prevalent) case or not (i.e. incident case)
 *  \param resetTimeOfInfection if infectedState is not negative, this bool marks whether the HIV infection occurred this month 
 *  \param isHighRisk a bool defaulting to true marking whether or not the patient is high risk; 
 * this only matters for people who start the model HIV-negative and draw from a different incident 
 * infection distribution/ PrEP inputs; others default to true
 *	All statistics counting different infection types/times are incremented in this function
 **/
void StateUpdater::setInfectedHIVState(SimContext::HIV_INF infectedState, bool isInitial, bool resetTimeOfInfection, bool isHighRisk) {
	patient->diseaseState.infectedHIVState = infectedState;
	if (infectedState != SimContext::HIV_INF_NEG) {
		// Currently HIV_POS is identical to HIV_INF, but with the first value HIV_INF_NEG removed - if either enum changes this will need to change
		patient->diseaseState.infectedHIVPosState = (SimContext::HIV_POS) (infectedState-1);
		if (resetTimeOfInfection)
			patient->diseaseState.monthOfHIVInfection = patient->generalState.monthNum;
	}
	if (infectedState == SimContext::HIV_INF_ACUTE_SYN) {
		patient->diseaseState.monthOfAcuteToChronicHIV = patient->generalState.monthNum + simContext->getHIVTestInputs()->monthsFromAcuteToChronic;
	}
	if (isInitial) {
		if (infectedState == SimContext::HIV_INF_NEG) {
			patient->diseaseState.isPrevalentHIVCase = false;
			patient->monitoringState.isHighRiskForHIV = isHighRisk;
		}
		else {
			patient->diseaseState.isPrevalentHIVCase = true;
		}
	}

	// Update statistics for a prevalent or incident infection
	if (isInitial) {
		SimContext::HIV_EXT_INF extInfectedState = (SimContext::HIV_EXT_INF) infectedState;
		if ((infectedState == SimContext::HIV_INF_NEG) && !isHighRisk)
			extInfectedState = SimContext::HIV_EXT_INF_NEG_LO;
		runStats->hivScreening.numPatientsInitialHIVState[extInfectedState]++;
		if (infectedState == SimContext::HIV_INF_NEG) {
			runStats->hivScreening.numHIVNegativeAtInit++;
		}
		else if (infectedState == SimContext::HIV_INF_SYMP_CHR_POS) {
			runStats->hivScreening.numPatientsInitialHIVState[SimContext::HIV_INF_ASYMP_CHR_POS]--;
		}
		else {
			runStats->hivScreening.numPrevalentCases++;
		}
	}
	else if (infectedState == SimContext::HIV_INF_ACUTE_SYN) {
		runStats->hivScreening.numIncidentCases++;
		if(patient->monitoringState.hasPrEP){
			runStats->hivScreening.numIncidentCasesByPrEPState[SimContext::HIV_POS_ON_PREP]++;
		}
		else if(patient->monitoringState.isPrEPDropout){
			runStats->hivScreening.numIncidentCasesByPrEPState[SimContext::HIV_POS_PREP_DROPOUT]++;
		}
		else{
			runStats->hivScreening.numIncidentCasesByPrEPState[SimContext::HIV_POS_NEVER_PREP]++;
		}
		runStats->hivScreening.monthsToInfectionSum += patient->generalState.monthNum;
		runStats->hivScreening.monthsToInfectionSumSquares += patient->generalState.monthNum * patient->generalState.monthNum;
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if (currTime) {
			currTime->numIncidentHIVInfections++;
			if (simContext->getCohortInputs()->useDynamicTransm && simContext->getCohortInputs()->updateDynamicTransmInc)
				currTime->dynamicNumIncidentHIVInfections++;
		}
	}
} /* end setInfectedHIVState */

/** \brief setInfectedPediatricsHIVState sets the pediatrics HIV state
 *
 * \param hivState a SimContext::PEDS_HIV_STATE (HIV infection state for pediatrics) that the patient is set to
 * \param isInitial a bool that specifies if this is called during initialzation
 * 
 **/
void StateUpdater::setInfectedPediatricsHIVState(SimContext::PEDS_HIV_STATE hivState, bool isInitial) {
	patient->diseaseState.infectedPediatricsHIVState = hivState;
	if (hivState != SimContext::PEDS_HIV_NEG)
		patient->diseaseState.useHEUMortality = false; 
	if (hivState == SimContext::PEDS_HIV_POS_PP){
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if (currTime){
			currTime->numIncidentPPInfections++;
		}
	}
	if (isInitial){
		runStats->initialDistributions.numInitialPediatrics[patient->diseaseState.infectedPediatricsHIVState][patient->pedsState.maternalStatus]++;
		if(hivState == SimContext::PEDS_HIV_NEG){	
			// update exposure status if the patient is HIV-negative
			if(patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_CHRONIC_PREGNANCY]){
				runStats->hivScreening.numHIVExposed[SimContext::MOM_CHRONIC_PREGNANCY]++;
				if(simContext->getPedsInputs()->exposedUninfectedDefsEarly[SimContext::MOM_CHRONIC_PREGNANCY] && !(simContext->getPedsInputs()->exposedUninfectedDefsEarly[SimContext::MOM_ON_ART_UNEXPOSED] && patient->pedsState.motherOnARTInitially))
					patient->diseaseState.useHEUMortality = true;
			}
			else if(patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_ACUTE_PREGNANCY]){
				runStats->hivScreening.numHIVExposed[SimContext::MOM_ACUTE_PREGNANCY]++;
				if(simContext->getPedsInputs()->exposedUninfectedDefsEarly[SimContext::MOM_ACUTE_PREGNANCY] && !(simContext->getPedsInputs()->exposedUninfectedDefsEarly[SimContext::MOM_ON_ART_UNEXPOSED] && patient->pedsState.motherOnARTInitially))
					patient->diseaseState.useHEUMortality = true;
			}
		}
	}	

} /* end setInfectedPediatricsHIVState */

/** \brief setMaternalHIVState sets the maternal HIV state for pediatrics at initiation and the maternal transition from acute to chronic. See rollForMaternalInfection() to update maternal HIV state for incident cases.
 *
 *  \param momHIVState a SimContext::PEDS_MATERNAL_HIV_STATE that specifies the (pediatric) patient's mother's HIV status
 *	\param motherBecameInfected a bool that specifies if the mother's state is HIV+ 
 *	\param isInit a bool that specifies if this is called during initialzation - if not, it is the maternal transition from acute to chronic
 **/
void StateUpdater::setMaternalHIVState(SimContext::PEDS_MATERNAL_STATUS momHIVState, bool motherBecameInfected, bool isInit) {
	patient->pedsState.maternalStatus = momHIVState;
	if(motherBecameInfected){
		patient->pedsState.monthOfMaternalHIVInfection = patient->generalState.monthNum;
		// mother was infected during pregnancy
		if(isInit){
			if((momHIVState == SimContext::PEDS_MATERNAL_STATUS_CHR_LOW) || (momHIVState == SimContext::PEDS_MATERNAL_STATUS_CHR_HIGH)){
				patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_CHRONIC_PREGNANCY] = true;	
			}
			else if (momHIVState == SimContext::PEDS_MATERNAL_STATUS_ACUTE){
				patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_ACUTE_PREGNANCY] = true;
			}
		}
	}		
} /* end setMaternalHIVState */

/** \brief setInitalMaternalState sets the maternal HIV state for pediatrics at init
 *
 **/
void StateUpdater::setInitialMaternalState() {
	patient->pedsState.isMotherAlive = true;
	for (int i = 0; i < SimContext::MOM_ACUTE_BREASTFEEDING; i++){
		patient->pedsState.motherInfectedDuringDelivery[i] = false;
	}
	patient->pedsState.motherInfectedDuringBF = false;
	patient->pedsState.breastfeedingStoppedEarly = false;
} /* end setInitialMaternalState */

/* \brief rollForMaternalInfection determines whether an HIV-negative mother becomes infected with HIV, and if so, updates the patient and maternal states */
void StateUpdater::rollForMaternalInfection() {
	double randNum = CepacUtil::getRandomDouble(90105, patient);
	if (randNum < simContext->getPedsInputs()->probMotherIncidentInfection[SimContext::PEDS_MATERNAL_STATUS_NEG]) {
		patient->pedsState.maternalStatus = SimContext::PEDS_MATERNAL_STATUS_ACUTE;
		patient->pedsState.monthOfMaternalHIVInfection = patient->generalState.monthNum;
		if (patient->pedsState.breastfeedingStatus != SimContext::PEDS_BF_REPL){
			patient->pedsState.motherInfectedDuringBF = true;
			if(patient->diseaseState.infectedPediatricsHIVState == SimContext::PEDS_HIV_NEG){
				// add to the aggregate totals for patients who have ever been exposed uninfected
				runStats->hivScreening.numHIVExposed[SimContext::MOM_ACUTE_BREASTFEEDING]++;
				// update exposure status for mortality purposes if enabled
				if(simContext->getPedsInputs()->exposedUninfectedDefsEarly[SimContext::MOM_ACUTE_BREASTFEEDING]) {
					patient->diseaseState.useHEUMortality = true;
				}
			}
		}	
		// if the patient is HIV-negative and already replacement feeding, the patient was never exposed to HIV - this ensures we count patients who stopped breastfeeding the same month the mother was infected
		else if(patient->diseaseState.infectedPediatricsHIVState == SimContext::PEDS_HIV_NEG){
			if(!patient->diseaseState.neverExposed){
				patient->diseaseState.neverExposed = true;
				// add to the aggregate totals for patients who are never exposed to HIV by their mothers
				runStats->hivScreening.numNeverHIVExposed++;
			}
		}
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d MATERNAL HIV INFECTION\n", patient->generalState.monthNum);
		}
	}
}	 
/* end rollForMaternalInfection */

/** \brief setMaternalStatusKnown sets the maternal HIV state for pediatrics
 *
 *  \param isKnown a bool that specifies whether the mother knows about her status
 **/
void StateUpdater::setMaternalStatusKnown(bool isKnown) {
	patient->pedsState.maternalStatusKnown = isKnown;
}

/** \brief setMaternalARTSTatus sets whether the mother is on ART
 *
 * \param isOnART is a bool for whether the mother is on ART
 * \param isSuppressed is a bool for whether the mother is on Suppressed ART
 * \param isInitial is a bool for whether this is called at initiation and therefore refers to the mother's ART status in pregnancy
 * \param suppressionKnown is a bool for whether the mother's suppression status is known (known suppressed or known not suppressed)
 **/
void StateUpdater::setMaternalARTStatus(bool isOnART, bool isSuppressed, bool suppressionKnown, bool isInitial) {
	patient->pedsState.motherOnART = isOnART;
	patient->pedsState.motherOnSuppressedART = isSuppressed;
	patient->pedsState.motherSuppressionKnown = suppressionKnown;
	if(isInitial){
		patient->pedsState.motherOnARTInitially = isOnART;
		patient->pedsState.motherOnSuppressedARTInitially = isSuppressed;
		patient->pedsState.motherSuppressionKnownInitially = suppressionKnown;
	}
}


/** \brief  setMaternalHVLStatus sets whether the mother is lowHVl or high HVL
 * \param isLow is a bool for whether the mother is LowHVL
 *  \param isInitial is a bool for whether this is called at initiation and therefore refers to the mother's HVL during pregnancy
 *
 **/
void StateUpdater::setMaternalHVLStatus(bool isLow, bool isInitial){
	patient->pedsState.motherLowHVL = isLow;
	if (isInitial)
		patient->pedsState.motherLowHVLInitially = isLow;
}

/** \brief updatePedsNeverExposed performs updates on HIV negative children who were never exposed to HIV
 * 
 * \param atDeath a bool defaulting to false for whether we are logging a patient at death; they were never exposed if they die during breastfeeding from an HIV negative mother. 
*/
void StateUpdater::updatePedsNeverExposed(bool atDeath){
	// Check whether the child qualifies as never having been exposed to HIV 
	if(patient->pedsState.maternalStatus == SimContext::PEDS_MATERNAL_STATUS_NEG){
		// if they die while breastfeeding from an HIV negative mother, they were never exposed and are counted in the exposure outputs as such
		if(atDeath){
			if(patient->pedsState.breastfeedingStatus != SimContext::PEDS_BF_REPL){
				patient->diseaseState.neverExposed = true;
				runStats->hivScreening.numNeverHIVExposed++;	
			}
		}
		else if(!patient->diseaseState.neverExposed && (patient->pedsState.breastfeedingStatus == SimContext::PEDS_BF_REPL)){
			patient->diseaseState.neverExposed = true;
			runStats->hivScreening.numNeverHIVExposed++;
		}
	}
}/* end updatePedsNeverExposed */

/** \brief updatePedsHIVExposureStats performs statistics on whether HIV negative children are exposed or unexposed to HIV
 * 
 * \param momHIVState a SimContext::PEDS_MATERNAL_HIV_STATE that specifies the (pediatric) patient's mother's HIV status
*/
void StateUpdater::updatePedsHIVExposureStats(SimContext::PEDS_MATERNAL_STATUS momHIVState){
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	// Update statistics on HIV exposure
	if(patient->diseaseState.neverExposed){
		if(currTime)
			currTime->numNeverHIVExposed++;
	}
	else if(momHIVState == SimContext::PEDS_MATERNAL_STATUS_ACUTE){
		if(patient->pedsState.breastfeedingStatus != SimContext::PEDS_BF_REPL){
			if(currTime)
				currTime->numHIVExposedUninf[SimContext::EXPOSED_BF_MOM_ACUTE]++;				
		}	
	}
	else if ((momHIVState == SimContext::PEDS_MATERNAL_STATUS_CHR_LOW || momHIVState == 	SimContext::PEDS_MATERNAL_STATUS_CHR_HIGH)) {
		if(patient->pedsState.breastfeedingStatus != SimContext::PEDS_BF_REPL){
			if(currTime)
				currTime->numHIVExposedUninf[SimContext::EXPOSED_BF_MOM_CHRONIC]++;
		}		
	}
}/* end updatePedsHIVExposureStats */

/** \brief setBreastfeedingStatus for pediatrics and updates statistics
 *
 * \param bfType a SimContext::PEDS_BF_TYPE to set the breastfeeding status to
 **/
void StateUpdater::setBreastfeedingStatus(SimContext::PEDS_BF_TYPE bfType) {
	patient->pedsState.breastfeedingStatus = bfType;
	if(bfType==SimContext::PEDS_BF_REPL){
		patient->pedsState.monthOfReplacementFeedingStart=patient->generalState.monthNum;
		if(patient->generalState.ageMonths < SimContext::PEDS_SAFE_BF_STOP_AGE)
			patient->pedsState.breastfeedingStoppedEarly = true;
	}
} /* end setBreastfeedingStatus */

/** \brief setBreastfeedingStopAge sets the age at which to stop breastfeeding for pediatrics
 *
 * \param stopAge an int for age at which to end bf
 **/
void StateUpdater::setBreastfeedingStopAge(int stopAge) {
	if (stopAge < 0)
		stopAge = 0;
	// Breastfeeding must stop before late childhood
	if (stopAge > 60)
		stopAge = 60;
	patient->pedsState.breastfeedingStopAge = stopAge;
} /* end setBreastfeedingStopAge */

/** \brief setAgeOfSeroreversion sets the age of seroreversion for pediatrics and updates statistics
 *
 * \param age an int for age of seroreversion
 **/
void StateUpdater::setAgeOfSeroreversion(int age) {
	if (age < 0)
		age = 0;
	patient->pedsState.ageOfSeroreversion = age;
} /* end setAgeOfSeroreversion */

/** \brief resetNumMissedVisitsEID sets the number of missed eid visits to 0
 **/
void StateUpdater::resetNumMissedVisitsEID(){
	patient->pedsState.numMissedVistsEID = 0;
} /* end resetNumMissedVisitsEID */

/** \brief incrementNumMissedVisitsEID increases the number of missed eid visits by 1
 **/
void StateUpdater::incrementNumMissedVisitsEID(){
	patient->pedsState.numMissedVistsEID++;
} /* end incrementNumMissedVisitsEID */

/** \brief setCareState sets the patient's care state
 *
 *  \param typeCare a SimContext::HIV_Care indicating the state of patient care
 **/
void StateUpdater::setCareState(SimContext::HIV_CARE typeCare){
	patient->monitoringState.careState = typeCare;
}

/** \brief setDetectedHIVState sets the patient's detection status and updates statistics
 *
 *  \param isDetected a bool indicating whether or not the patient has been detected with HIV
 *  \param typeDetection a SimContext::HIV_DET indicating the type of detection
 *  \param oiType a SimContext::OI_TYPE indicating which OI (default OI_NONE) triggered the detection
 *
 *  If isDetected is true, all detection statistics are updated.
 **/
void StateUpdater::setDetectedHIVState(bool isDetected, SimContext::HIV_DET typeDetection, SimContext::OI_TYPE oiType) {
	patient->monitoringState.isDetectedHIVPositive = isDetected;
	// Update statistics for a newly detected patient, if screening module is enabled
	if (isDetected) {
		patient->monitoringState.monthOfDetection = patient->generalState.monthNum;
		setCareState(SimContext::HIV_CARE_UNLINKED);
		SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
		SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
		SimContext::HIV_POS hivPosState = patient->diseaseState.infectedHIVPosState;
		int monthNum = patient->generalState.monthNum;
		int ageMonths = patient->generalState.ageMonths;
		
		// Monthly detection outputs
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if(currTime){
			currTime->numHIVDetections[typeDetection]++;
			if (simContext->getPedsInputs()->enablePediatricsModel && patient->pedsState.isMotherAlive && !patient->pedsState.maternalStatusKnown){
				currTime->numNewlyDetectedPediatricsMotherStatusUnknown++;
			}
		}

        //Disable PrEP on detection
		if(simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP){
			if(patient->monitoringState.hasPrEP){
            	setPrEP(false);
            }	
		}
		// Overall detection outputs
		if (typeDetection != SimContext::HIV_DET_BACKGROUND_PREV_DET && typeDetection != SimContext::HIV_DET_OI_PREV_DET && typeDetection != SimContext::HIV_DET_SCREENING_PREV_DET && typeDetection != SimContext::HIV_DET_TB_PREV_DET){
			runStats->hivScreening.numDetectedGender[patient->generalState.gender]++;
			costStats->popSummary.numDetected++;
		}
		if (typeDetection == SimContext::HIV_DET_OI) {
			runStats->hivScreening.numDetectedByOIs[oiType]++;
		}
		if (typeDetection == SimContext::HIV_DET_OI_PREV_DET){
			runStats->hivScreening.numDetectedByOIsPrevDetected[oiType]++;
		}
		if (patient->diseaseState.isPrevalentHIVCase) {
			runStats->hivScreening.numDetectedPrevalentMeans[typeDetection]++;
			if (typeDetection != SimContext::HIV_DET_BACKGROUND_PREV_DET && typeDetection != SimContext::HIV_DET_OI_PREV_DET && typeDetection != SimContext::HIV_DET_SCREENING_PREV_DET){
				runStats->hivScreening.numAtDetectionPrevalentCD4Metric[patient->pedsState.ageCategoryCD4Metric]++;
				runStats->hivScreening.numAtDetectionPrevalentHIVCD4Metric[hivPosState][patient->pedsState.ageCategoryCD4Metric]++;
				runStats->hivScreening.numAtDetectionPrevalentCD4HIV[cd4Strata][hivPosState]++;
				runStats->hivScreening.numAtDetectionPrevalentHVLHIV[hvlStrata][hivPosState]++;
				if (patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
					runStats->hivScreening.CD4AtDetectionPrevalentSumHIV[hivPosState]+= patient->diseaseState.currTrueCD4;
				runStats->hivScreening.monthsToDetectionPrevalentSum += monthNum;
				runStats->hivScreening.monthsToDetectionPrevalentSumSquares += monthNum * monthNum;
				runStats->hivScreening.ageMonthsAtDetectionPrevalentSum += ageMonths;
				runStats->hivScreening.ageMonthsAtDetectionPrevalentSumSquares += ageMonths * ageMonths;
			}
		}
		else {
			runStats->hivScreening.numDetectedIncidentMeans[typeDetection]++;
			if (typeDetection != SimContext::HIV_DET_BACKGROUND_PREV_DET && typeDetection != SimContext::HIV_DET_OI_PREV_DET && typeDetection != SimContext::HIV_DET_SCREENING_PREV_DET && typeDetection != SimContext::HIV_DET_TB_PREV_DET){
				runStats->hivScreening.numAtDetectionIncidentCD4Metric[patient->pedsState.ageCategoryCD4Metric]++;
				runStats->hivScreening.numAtDetectionIncidentHIVCD4Metric[hivPosState][patient->pedsState.ageCategoryCD4Metric]++;
				runStats->hivScreening.numAtDetectionIncidentCD4HIV[cd4Strata][hivPosState]++;
				runStats->hivScreening.numAtDetectionIncidentHVLHIV[hvlStrata][hivPosState]++;
				if (patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
					runStats->hivScreening.CD4AtDetectionIncidentSumHIV[hivPosState] += patient->diseaseState.currTrueCD4;
				runStats->hivScreening.monthsToDetectionIncidentSum += monthNum;
				runStats->hivScreening.monthsToDetectionIncidentSumSquares += monthNum * monthNum;
				runStats->hivScreening.ageMonthsAtDetectionIncidentSum += ageMonths;
				runStats->hivScreening.ageMonthsAtDetectionIncidentSumSquares += ageMonths * ageMonths;
				int monthDiff = monthNum - patient->diseaseState.monthOfHIVInfection;
				runStats->hivScreening.monthsAfterInfectionToDetectionSum += monthDiff;
				runStats->hivScreening.monthsAfterInfectionToDetectionSumSquares += monthDiff * monthDiff;
			}
		}
	}//end if(isdetected)
	else{
		if(patient->diseaseState.infectedHIVState!=SimContext::HIV_INF_NEG)
			setCareState(SimContext::HIV_CARE_UNDETECTED);
	}
} /* end setDetectedHIVState */

/** \brief setLinkedState sets the patient's Link status
  *  \param typeDetection a SimContext::HIV_DET defaulting to SimContext::HIV_DET_UNDETECTED indicating the type of detection
 **/
void StateUpdater::setLinkedState(bool isLinked, SimContext::HIV_DET typeLinked) {
	// Update statistics for a newly linked patient, if screening module is enabled
	patient->monitoringState.isLinked = isLinked;
	if (isLinked){
		patient->monitoringState.monthOfLinkage=patient->generalState.monthNum;
		setCareState(SimContext::HIV_CARE_IN_CARE);
		SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
		SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;

		SimContext::HIV_POS hivPosState = patient->diseaseState.infectedHIVPosState;
		int monthNum = patient->generalState.monthNum;
		int ageMonths = patient->generalState.ageMonths;

		if(typeLinked == SimContext::HIV_DET_INITIAL)
			runStats->hivScreening.numLinkedAtInit[hivPosState]++;

		if (patient->monitoringState.hasObservedCD4){
			SimContext::CD4_STRATA obsvCD4Strata = patient->monitoringState.currObservedCD4Strata;
			costStats->popSummary.observedCD4DistributionAtLinkage[obsvCD4Strata]++;
		}
		costStats->popSummary.genderDistributionAtLinkage[patient->generalState.gender]++;
		costStats->popSummary.ageDistributionAtLinkage[getAgeCategoryLinkageStats(patient->generalState.ageMonths)]++;
		runStats->hivScreening.numLinkedMeans[typeLinked]++;
		runStats->hivScreening.numAtLinkageCD4Metric[patient->pedsState.ageCategoryCD4Metric]++;
		runStats->hivScreening.numAtLinkageHIVCD4Metric[hivPosState][patient->pedsState.ageCategoryCD4Metric]++;
		runStats->hivScreening.numAtLinkageCD4HIV[cd4Strata][hivPosState]++;
		runStats->hivScreening.numAtLinkageHVLHIV[hvlStrata][hivPosState]++;
		if (patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
			runStats->hivScreening.CD4AtLinkageSumHIV[hivPosState]+=patient->diseaseState.currTrueCD4;
		runStats->hivScreening.monthsToLinkageSum += monthNum;
		runStats->hivScreening.monthsToLinkageSumSquares += monthNum*monthNum;
		runStats->hivScreening.ageMonthsAtLinkageSum += ageMonths;
		runStats->hivScreening.ageMonthsAtLinkageSumSquares += ageMonths * ageMonths;

		runStats->hivScreening.monthsToLinkageSumMeans[typeLinked] += monthNum;
		runStats->hivScreening.monthsToLinkageSumSquaresMeans[typeLinked] += monthNum*monthNum;
	}
} /* end setLinkedState */

/** \brief setFalsePositiveStatus sets whether the patient is false positive and whether they are linked or not
 * \param isFalsePositive is a bool of whether the patient is false positive
 * \param isLinked is a bool of whether the patient is linked to care (their care state however is still HIV Neg)
 **/
void StateUpdater::setFalsePositiveStatus(bool isFalsePositive, bool isLinked){
	patient->pedsState.isFalsePositive = isFalsePositive;
	patient->pedsState.isFalsePositiveLinked = isLinked;

} /* end setFalsePositiveStatus */

/** \brief setCanRecieveEID sets the status of whether the patient can get EID test visits
 **/
void StateUpdater::setCanReceiveEID(bool canReceiveEID){
	patient->pedsState.canReceiveEID = canReceiveEID;
} /* end setCanRecieveEID */

/** \brief removePendingEIDTestsAndResults gets rid of pending test results and scheduled confirmatory EID tests
 **/
void StateUpdater::removePendingEIDTestsAndResults(bool removeOITests){
	if(removeOITests){
		patient->pedsState.eidPendingTestResults.clear();
		patient->pedsState.eidScheduledConfirmatoryTests.clear();
	}
	else{
		int i = 0;
		while (i < patient->pedsState.eidPendingTestResults.size()){
			bool triggeredByOI = patient->pedsState.eidPendingTestResults[i].triggeredByOI;
			if (!triggeredByOI){
				// Slightly hackish way to get a regular iterator from a const iterator, this is necessary
				//	since erase requires a regular iterator
				vector<SimContext::EIDTestState>::const_iterator beginIter = patient->pedsState.eidPendingTestResults.begin();
				vector<SimContext::EIDTestState>::iterator eraseIter = patient->pedsState.eidPendingTestResults.begin();
				advance(eraseIter, i);

				//remove element from vector
				patient->pedsState.eidPendingTestResults.erase(eraseIter);
			}
			else
				i++;
		}

		i = 0;
		while (i < patient->pedsState.eidScheduledConfirmatoryTests.size()){
			bool triggeredByOI = patient->pedsState.eidScheduledConfirmatoryTests[i].triggeredByOI;
			if (!triggeredByOI){
				// Slightly hackish way to get a regular iterator from a const iterator, this is necessary
				//	since erase requires a regular iterator
				vector<SimContext::EIDTestState>::const_iterator beginIter = patient->pedsState.eidScheduledConfirmatoryTests.begin();
				vector<SimContext::EIDTestState>::iterator eraseIter = patient->pedsState.eidScheduledConfirmatoryTests.begin();
				advance(eraseIter, i);

				//remove element from vector
				patient->pedsState.eidScheduledConfirmatoryTests.erase(eraseIter);
			}
			else
				i++;
		}

	}
} /* end removePendingEIDTestsAndResults */


/** \brief updateHIVTestingStats updates all statistics after an HIV testing event
 *
 * \param acceptTest a bool that indicates whether or not the patient accepted the test -- returnResults and isPositive are
 * irrelevant if this is false
 * \param returnResults a bool that indicates whether or not the patient returned for the result -- if this is false,
 * isPositive is irrelevant
 * \param isPositive a bool indicating whether or not the patient's test came back HIV positive
 *
 * If the patient accepted the test and returned for the test, the test results are recorded in the statistics depending
 * on whether it was a true positive, true negative, false positive, or false negative.
 **/
void StateUpdater::updateHIVTestingStats(bool acceptTest, bool returnResults, bool isPositive) {
	if (!acceptTest) {
		runStats->hivScreening.numRefuseTest++;
		return;
	}
	SimContext::HIV_EXT_INF infectedState;
	if (!patient->monitoringState.isHighRiskForHIV && (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)) {
		infectedState = SimContext::HIV_EXT_INF_NEG_LO;
	}
	else {
		infectedState = (SimContext::HIV_EXT_INF) patient->diseaseState.infectedHIVState;
	}
	runStats->hivScreening.numTestsHIVState[infectedState]++;
	runStats->hivScreening.numAcceptTest++;

	if (!returnResults) {
		runStats->hivScreening.numNoReturnForResults++;
		return;
	}
	runStats->hivScreening.numReturnForResults++;

	if (isPositive) {
		if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG) {
			runStats->hivScreening.numTestResultsHIVNegativeType[SimContext::TEST_FALSE_POS]++;
		}
		else if (patient->diseaseState.isPrevalentHIVCase) {
			runStats->hivScreening.numTestResultsPrevalentType[SimContext::TEST_TRUE_POS]++;
		}
		else {
			runStats->hivScreening.numTestResultsIncidentType[SimContext::TEST_TRUE_POS]++;
		}
	}
	else {
		if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG) {
			runStats->hivScreening.numTestResultsHIVNegativeType[SimContext::TEST_TRUE_NEG]++;
		}
		else if (patient->diseaseState.isPrevalentHIVCase) {
			runStats->hivScreening.numTestResultsPrevalentType[SimContext::TEST_FALSE_NEG]++;
		}
		else {
			runStats->hivScreening.numTestResultsIncidentType[SimContext::TEST_FALSE_NEG]++;
		}
	}
} /* end updateHIVTestingStats */

/** \brief updateLabStagingStats updates all statistics after a Lab Staging event
 *
 * \param acceptTest a bool that indicates whether or not the patient accepted the test -- returnResults and isPositive are
 * irrelevant if this is false
 * \param returnResults a bool that indicates whether or not the patient returned for the result -- if this is false,
 * hasLinked is irrelevant
 * \param hasLinked a bool indicating whether or not the patient links to care
 *
 **/
void StateUpdater::updateLabStagingStats(bool acceptTest, bool returnResults, bool hasLinked) {
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
		return;
	if (!acceptTest) {
		runStats->hivScreening.numRefuseLabStaging++;
		return;
	}
	SimContext::HIV_POS infectedState = (SimContext::HIV_POS) (patient->diseaseState.infectedHIVState-1);

	runStats->hivScreening.numAcceptLabStagingHIVState[infectedState]++;
	runStats->hivScreening.numAcceptLabStaging++;

	if (!returnResults) {
		runStats->hivScreening.numNoReturnForResultsLabStaging++;
		return;
	}
	SimContext::CD4_STRATA trueCD4Strata = patient->diseaseState.currTrueCD4Strata;
	SimContext::CD4_STRATA obsvCD4Strata = patient->monitoringState.currObservedCD4Strata;

	runStats->hivScreening.numReturnLabStagingHIVState[infectedState]++;
	runStats->hivScreening.numReturnForResultsLabStaging++;

	runStats->hivScreening.numReturnLabStagingObsvCD4[obsvCD4Strata]++;
	runStats->hivScreening.numReturnLabStagingTrueCD4[trueCD4Strata]++;
	runStats->hivScreening.numReturnLabStagingObsvTrueCD4[obsvCD4Strata][trueCD4Strata]++;
	if(!hasLinked){
		runStats->hivScreening.numNoLinkLabStaging++;
		return;
	}

	runStats->hivScreening.numLinkLabStagingObsvCD4[obsvCD4Strata]++;
	runStats->hivScreening.numLinkLabStagingTrueCD4[trueCD4Strata]++;
	runStats->hivScreening.numLinkLabStagingObsvTrueCD4[obsvCD4Strata][trueCD4Strata]++;
	runStats->hivScreening.numLinkLabStaging++;
} /* end updateLabStagingStats */

/** \brief setHIVTestingParams sets the interval and acceptance probability for HIV testing
 *
 * \param intervalIndex an integer representing which HIV Testing Interval to assign to the patient based on user specified stratification
 * \param acceptanceProbIndex an integer representing which HIV Test acceptance probability to assign to the patient based on user specified stratification
 *
 * The patient's HIV testing interval and acceptance probability is set and the runStats relating to the number of patients in each testing interval and acceptance probability are incremented
 **/
void StateUpdater::setHIVTestingParams(int intervalIndex, int acceptanceProbIndex) {
	SimContext::HIV_EXT_INF extInfectedState = (SimContext::HIV_EXT_INF) patient->diseaseState.infectedHIVState;
	if ((patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG) && !patient->monitoringState.isHighRiskForHIV)
		extInfectedState = SimContext::HIV_EXT_INF_NEG_LO;
	patient->monitoringState.intervalHIVTest = simContext->getHIVTestInputs()->HIVTestingInterval[intervalIndex];
	patient->monitoringState.acceptanceProbHIVTest = simContext->getHIVTestInputs()->HIVTestAcceptProb[extInfectedState][acceptanceProbIndex];

	runStats->hivScreening.numTestingInterval[intervalIndex]++;
	runStats->hivScreening.numTestingAcceptProb[acceptanceProbIndex][extInfectedState]++;
} /* end setHIVTestingParams */


/** \brief setPrEP sets whether the patient is on PrEP and updates statistics about PrEP takeup and dropout
 *
 * \param hasPrEP a bool representing if the patient is on PrEP
 * \param isDropout a bool defaulting to false indicating if the patient is dropping out of PrEP 
 *
  **/
void StateUpdater::setPrEP(bool hasPrEP, bool isDropout) {
	patient->monitoringState.hasPrEP = hasPrEP;
	if (hasPrEP){
		patient->monitoringState.everPrEP = true;
		runStats->hivScreening.numEverPrEP++;
		
		if(simContext->getHIVTestInputs()->dropoutThresholdFromPrEPStart)
			patient->monitoringState.PrEPDropoutThresholdMonth = patient->generalState.monthNum + simContext->getHIVTestInputs()->PrEPDropoutThreshold;
		else
			patient->monitoringState.PrEPDropoutThresholdMonth = simContext->getHIVTestInputs()->PrEPDropoutThreshold;
	}	
	else if(isDropout){
		patient->monitoringState.isPrEPDropout = true;
		runStats->hivScreening.numDropoutPrEP++;
	}
} /* end setPrEP */

/** \brief setInitialPrEPParams sets the initial PrEP parameters
 *
 *
  **/
void StateUpdater::setInitialPrEPParams(){
	patient->monitoringState.everPrEP = false;
	patient->monitoringState.isPrEPDropout = false;
} /* end setInitialPrEPParams */

/** \brief updatePrEPProbLogging logs the monthly probability of PrEP uptake
 *
 * \param prepProb a double indicating the probability of PrEP uptake calculated for this risk level
 * \param risk a SimContext::HIV_BEHAV indicating the patient's HIV risk level
 *
  **/
void StateUpdater::updatePrEPProbLogging(double prepProb, SimContext::HIV_BEHAV risk){
    RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime)
		currTime->probPrepUptake[risk] = prepProb;
} /* end updatePrEPProbLogging */

/** \brief setCD4TestingAvailable Sets whether the patient is able to get CD4 tests
 *
 * \param isAvail a bool indicating whether or not patient is able to get CD4 tests
 **/
void StateUpdater::setCD4TestingAvailable(bool isAvail){
	patient->monitoringState.CD4TestingAvailable = isAvail;
} /* end setCD4TestingAvailable */

/** \brief setChanceCD4Test sets whether the patient has had an option to get a cd4 test
 *
 * \param hadChance a bool indicating whether or not patient had a chance for cd4 test
 **/
void StateUpdater::setChanceCD4Test(bool hadChance) {
	patient->monitoringState.hadChanceCD4Test=hadChance;
} /* end setChanceCD4Test */

/** \brief setChanceHVLTest sets whether the patient has had an option to get a HVL test
 *
 * \param hadChance a bool indicating whether or not patient had a chance for HVL test
 **/
void StateUpdater::setChanceHVLTest(bool hadChance) {
	patient->monitoringState.hadChanceHVLTest=hadChance;
} /* end setChanceHVLTest */

/** \brief scheduleHIVTest sets the month of the next HIV test
 *
 * \param hasNext a bool indicating whether or not an HIV test should be scheduled
 * \param monthNum an integer specifying which month the next HIV test should be scheduled for if hasNext is true
 **/
void StateUpdater::scheduleHIVTest(bool hasNext, int monthNum) {
	patient->monitoringState.hasScheduledHIVTest = hasNext;
	if (hasNext)
		patient->monitoringState.monthOfScheduledHIVTest = monthNum;
} /* end scheduleHIVTest */

/** \brief scheduleCD4Test sets the month of the next CD4 test
 *
 * \param hasNext a bool indicating whether or not a CD4 test should be scheduled
 * \param monthNum an integer specifying which month the next CD4 test should be scheduled for if hasNext is true
 **/
void StateUpdater::scheduleCD4Test(bool hasNext, int monthNum) {
	if (monthNum<simContext->getTreatmentInputs()->CD4TestingLag || !patient->monitoringState.CD4TestingAvailable){
		patient->monitoringState.hasScheduledCD4Test=false;
		return;
	}
	patient->monitoringState.hadChanceCD4Test=true;
	patient->monitoringState.hasScheduledCD4Test = hasNext;
	if (hasNext)
		patient->monitoringState.monthOfScheduledCD4Test = monthNum;
} /* end scheduleCD4Test */

/** \brief scheduleHVLTest sets the month of the next HVL test
 *
 * \param hasNext a bool indicating whether or not an HVL test should be scheduled
 * \param monthNum an integer specifying which month the next HVL test should be scheduled for if hasNext is true
 **/
void StateUpdater::scheduleHVLTest(bool hasNext, int monthNum) {
	if (monthNum<simContext->getTreatmentInputs()->HVLTestingLag){
		patient->monitoringState.hasScheduledHVLTest=false;
		return;
	}
	patient->monitoringState.hadChanceHVLTest=true;
	patient->monitoringState.hasScheduledHVLTest = hasNext;
	if (hasNext)
		patient->monitoringState.monthOfScheduledHVLTest = monthNum;
} /* end scheduleHVLTest */


/** \brief performEIDTest handles doing an eid base test or confirmatory test
 *
 * \param assayNum a bool indicating which hiv test is to be performed (test assay)
 * \param testType is a SimContext::EID_TEST_TYPE specifying whether it is a base test or confirmatory test
 * \param triggerdByOI is a bool indicating if the test was due to an OI
 * \param isNonMaternal is a bool indicating if child is brought by non-maternal caregiver
 **/
void StateUpdater::performEIDTest(int baseAssay, int testAssay, SimContext::EID_TEST_TYPE testType, bool triggeredByOI, bool isNonMaternal){
	SimContext::EIDInputs::EIDTest eidTest = simContext->getEIDInputs()->EIDTests[testAssay];

	/** patient has presented to visit*/
	//Roll for prob of being offered test

	double probOffer = eidTest.EIDTestOfferProb;

	//modify if primary test offer and child brought by non maternal caregiver
	if (isNonMaternal && testType == SimContext::EID_TEST_TYPE_BASE)
		probOffer *= simContext->getEIDInputs()->probMultNonMaternalCaregiver;

	double randNum = CepacUtil::getRandomDouble(100130, patient);
	if (randNum < probOffer){
		//Roll for prob of accepting test
		randNum = CepacUtil::getRandomDouble(100140, patient);
		if (randNum < eidTest.EIDTestAcceptProb){
			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d EID test %d %s, offer accepted\n", patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType]);
			if (testType == SimContext::EID_TEST_TYPE_BASE)
				setMissedEIDTest(false, -1);
		}
		else{
			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d EID test %d %s, offer not accepted\n", patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType]);
			if (testType == SimContext::EID_TEST_TYPE_BASE)
				setMissedEIDTest(true, baseAssay);
			return;
		}
	}
	else{
		if (patient->generalState.tracingEnabled)
			tracer->printTrace(1, "**%d EID test %d %s, failed to offer\n", patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType]);
		if (testType == SimContext::EID_TEST_TYPE_BASE)
			setMissedEIDTest(true, baseAssay);
		return;
	}

	/** patient has accepted test */
	if (eidTest.includeInEIDCosts)
		incrementCostsEIDTest(eidTest.EIDTestCost);
	else
		incrementCostsMisc(eidTest.EIDTestCost, 1.0);

	//roll for test result
	bool testResult = rollForEIDTestResult(testAssay, testType);

	//Roll for result return lab to clinic
	randNum = CepacUtil::getRandomDouble(100140, patient);
	if (randNum >= eidTest.EIDTestResultReturnProbLab){
		if (patient->generalState.tracingEnabled)
			tracer->printTrace(1, "**%d EID test %d %s, result will not be returned lab to clinic\n", patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType]);
		return;
	}

	//costs for result return lab to clinic
	if (eidTest.includeInEIDCosts)
		incrementCostsEIDMisc(eidTest.EIDTestResultReturnCost);
	else
		incrementCostsMisc(eidTest.EIDTestResultReturnCost, 1.0);

	/** Roll for and set test return month */
	int monthsToReturn = (int)(CepacUtil::getRandomGaussian(eidTest.EIDTestReturnTimeMean, eidTest.EIDTestReturnTimeStdDev, 100180, patient)+0.5);
	if (monthsToReturn < 0)
		monthsToReturn = 0;

	//Roll for result return clinic to patient
	bool returnResult;
	randNum = CepacUtil::getRandomDouble(100150, patient);
	if (randNum < eidTest.EIDTestResultReturnProbPatient){
		returnResult = true;
		//add costs for result return to patient
		if (testResult){
			if (eidTest.includeInEIDCosts)
				incrementCostsEIDMisc(eidTest.EIDTestPositiveResultReturnCost);
			else
				incrementCostsMisc(eidTest.EIDTestPositiveResultReturnCost, 1.0);
		}
		else{
			if (eidTest.includeInEIDCosts)
				incrementCostsEIDMisc(eidTest.EIDTestNegativeResultReturnCost);
			else
				incrementCostsMisc(eidTest.EIDTestNegativeResultReturnCost, 1.0);
		}
	}
	else
		returnResult = false;

	if (patient->generalState.tracingEnabled)
		tracer->printTrace(1, "**%d EID test %d %s, %s result will be returned in month %d, lab to clinic:yes, clinic to patient:%s\n",
				patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType],
				testResult?"Positive":"Negative",patient->generalState.monthNum + monthsToReturn,returnResult?"yes":"no");

	addPendingResultEIDTest(baseAssay, testAssay, testType, testResult, patient->generalState.monthNum + monthsToReturn, returnResult,triggeredByOI);
} /* end performEIDTest */

/** \brief rollForEIDTestResult handles getting a result for eid tests
 *
 * \param assayNum a bool indicating which hiv test is being used (test assay)
 **/
bool StateUpdater::rollForEIDTestResult(int assayNum, SimContext::EID_TEST_TYPE testType){
	SimContext::EIDInputs::EIDTest eidTest = simContext->getEIDInputs()->EIDTests[assayNum];
	//ages 17+ are grouped in one categroy
	int timeFromBirth = min(patient->generalState.ageMonths, SimContext::EID_TEST_AGE_CATEGORY_NUM-1);
	SimContext::PEDS_HIV_STATE pedsHIVState = patient->diseaseState.infectedPediatricsHIVState;
	bool testPositive;

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if(currTime){
		currTime->numEIDTestsGivenType[testType]++;
		currTime->numEIDTestsGivenTest[assayNum]++;
	}
	if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG){
		int timeFromInfection = min(patient->generalState.monthNum - patient->diseaseState.monthOfHIVInfection, SimContext::EID_TEST_AGE_CATEGORY_NUM-1 );
		double sensitivity;
		double sensMult;
		int timeProph;

		//Patient is positive use sensitivity
		if(patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_CHRONIC_PREGNANCY] || patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_ACUTE_PREGNANCY]){
			if(pedsHIVState == SimContext::PEDS_HIV_POS_IU){
				sensitivity = eidTest.EIDSensitivityIU[timeFromBirth];
				sensMult = eidTest.EIDSensitivityMultiplierMaternalART[timeFromBirth];
				timeProph = timeFromBirth;
			}
			else if(pedsHIVState == SimContext::PEDS_HIV_POS_IP){
				sensitivity = eidTest.EIDSensitivityIP[timeFromBirth];
				sensMult = eidTest.EIDSensitivityMultiplierMaternalART[timeFromBirth];
				timeProph = timeFromBirth;
			}
			else if(pedsHIVState == SimContext::PEDS_HIV_POS_PP){
				if(patient->generalState.ageMonths < patient->pedsState.ageOfSeroreversion){
					sensitivity = eidTest.EIDSensitivityPPBeforeSRMotherInfectedPreDelivery[timeFromInfection];
					sensMult = eidTest.EIDSensitivityMultiplierMaternalART[timeFromInfection];
					timeProph = timeFromInfection;
				}
				else{
					sensitivity = eidTest.EIDSensitivityPPAfterSRMotherInfectedPreDelivery[timeFromInfection];
					sensMult = eidTest.EIDSensitivityMultiplierMaternalART[timeFromInfection];
					timeProph = timeFromInfection;
				}
			}
		}
		else{
			sensitivity = eidTest.EIDSensitivityPPMotherInfectedPostDelivery[timeFromInfection];
			sensMult = eidTest.EIDSensitivityMultiplierMaternalART[timeFromInfection];
			timeProph = timeFromInfection;
		}

		//Multiplier to sensitivity if mother was on art during preg
		if (patient->pedsState.motherOnARTInitially)
			sensitivity *= sensMult;

		//Modify by infant proph
		for(int i = 0; i < SimContext::INFANT_HIV_PROPHS_NUM; ++i){
			if (patient->pedsState.hasEffectiveInfantHIVProph[i]){
				//Check if in efficacy time horizon
				int timeSinceProph = patient->generalState.monthNum - patient->pedsState.monthOfEffectiveInfantHIVProph[i];
				double prophMult = eidTest.EIDSensitivityMultiplierInfantHIVProph[i][timeProph];
	
				if (timeSinceProph <= simContext->getEIDInputs()->infantHIVProphs[i].infantProphEffHorizon)
					sensitivity*=prophMult;
				else if(timeSinceProph <= simContext->getEIDInputs()->infantHIVProphs[i].infantProphEffHorizon + simContext->getEIDInputs()->infantHIVProphs[i].infantProphDecayTime){
					int timeSinceDecay = timeSinceProph - simContext->getEIDInputs()->infantHIVProphs[i].infantProphEffHorizon;
					double propDecay = (double) timeSinceDecay / simContext->getEIDInputs()->infantHIVProphs[i].infantProphDecayTime;					
					sensitivity *=  (prophMult + propDecay*(1-prophMult));
				}
			}
		}
		//Roll for result
		double randNum = CepacUtil::getRandomDouble(100160+assayNum, patient);
		if (randNum < sensitivity)
			testPositive = true;
		else
			testPositive = false;

		if(currTime){
			if (testPositive){
				currTime->numTruePositiveEIDTestResultsType[testType]++;
				currTime->numTruePositiveEIDTestResultsTest[assayNum]++;
			}
			else{
				currTime->numFalseNegativeEIDTestResultsType[testType]++;
				currTime->numFalseNegativeEIDTestResultsTest[assayNum]++;
			}
		}
	} // end if patient is HIV+
	else{
		//Patient is negative use specificity
		double specificity;
		int timeProph;

		if(patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_CHRONIC_PREGNANCY] || patient->pedsState.motherInfectedDuringDelivery[SimContext::MOM_ACUTE_PREGNANCY]){
			if(patient->generalState.ageMonths < patient->pedsState.ageOfSeroreversion){
				specificity = eidTest.EIDSpecificityHEUBeforeSRMotherInfectedPreDelivery[timeFromBirth];
				timeProph = timeFromBirth;
			}
			else{
				specificity = eidTest.EIDSpecificityHEUAfterSRMotherInfectedPreDelivery[timeFromBirth];
				timeProph = timeFromBirth;
			}
		}
		else{
			if(patient->pedsState.maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG){
				int timeFromMaternalInfection = min(patient->generalState.monthNum - patient->pedsState.monthOfMaternalHIVInfection, SimContext::EID_TEST_AGE_CATEGORY_NUM-1 );
				specificity = eidTest.EIDSpecificityMotherInfectedPostDelivery[timeFromMaternalInfection];
				timeProph = timeFromMaternalInfection;
			}
			else{
				specificity = eidTest.EIDSpecificityMotherUninfected[timeFromBirth];
				timeProph = timeFromBirth;
			}
		}

		//Modify by infant proph
		for(int i = 0; i < SimContext::INFANT_HIV_PROPHS_NUM; ++i){
			if (patient->pedsState.hasEffectiveInfantHIVProph[i]){
				//Check if in efficacy time horizon
				int timeSinceProph = patient->generalState.monthNum - patient->pedsState.monthOfEffectiveInfantHIVProph[i];
				double prophMult = eidTest.EIDSpecificityMultiplierInfantHIVProph[i][timeProph];
	
				if (timeSinceProph <= simContext->getEIDInputs()->infantHIVProphs[i].infantProphEffHorizon)
					specificity*=prophMult;
				else if(timeSinceProph <= simContext->getEIDInputs()->infantHIVProphs[i].infantProphEffHorizon + simContext->getEIDInputs()->infantHIVProphs[i].infantProphDecayTime){
					int timeSinceDecay = timeSinceProph - simContext->getEIDInputs()->infantHIVProphs[i].infantProphEffHorizon;
					double propDecay = (double) timeSinceDecay / simContext->getEIDInputs()->infantHIVProphs[i].infantProphDecayTime;
					specificity *=  (prophMult + propDecay*(1-prophMult));
				}
			}
		}
		//Roll for result
		double randNum = CepacUtil::getRandomDouble(100170+assayNum, patient);
		if (randNum < specificity)
			testPositive = false;
		else
			testPositive = true;
		if(currTime){
			if (testPositive){
				currTime->numFalsePositiveEIDTestResultsType[testType]++;
				currTime->numFalsePositiveEIDTestResultsTest[assayNum]++;
			}
			else{
				currTime->numTrueNegativeEIDTestResultsType[testType]++;
				currTime->numTrueNegativeEIDTestResultsTest[assayNum]++;
			}
		}
	}


	return testPositive;
} /* end rollForEIDTestResult */

/** \brief rollForEIDLinkage handles linkage to care from EID
 *
 * \param assayNum an int indicating which hiv test is being used (test assay)
 * \param triggeredByOI a bool indicating whether the test was a confirmatory one triggered by an occurrence of an acute OI
 **/
void StateUpdater::rollForEIDLinkage (int assayNum, bool triggeredByOI){
	SimContext::EIDInputs::EIDTest baseEIDTest = simContext->getEIDInputs()->EIDTests[assayNum];
	double randNum = CepacUtil::getRandomDouble(100190, patient);
	if (randNum < baseEIDTest.EIDProbLinkage){
		if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG){
			//true positives
			bool wasPrevDetected = patient->monitoringState.isDetectedHIVPositive;

			if (triggeredByOI){
				if (wasPrevDetected)
					setLinkedState(true, SimContext::HIV_DET_OI_PREV_DET);
				else
					setLinkedState(true, SimContext::HIV_DET_OI);
			}
			else{
				if (wasPrevDetected)
					setLinkedState(true, SimContext::HIV_DET_SCREENING_PREV_DET);
				else
					setLinkedState(true, SimContext::HIV_DET_SCREENING);
			}

			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d EID patient links to care\n", patient->generalState.monthNum);

			scheduleInitialClinicVisit();
		}
		else{
			//false positives - patients who are falsely diagnosed and linked to HIV care will not actually receive HIV care such as ART while they are HIV-negative
			setFalsePositiveStatus(true, true);
			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d EID patient becomes false positive linked\n", patient->generalState.monthNum);

		}
		//chain completed reset provider knowledge - technically not needed since they will be retunred out of HIVTestingUpdater::performEIDScreeningUpdates() but draws a distinction between what happens to those who do and do not link - see below
		setMostRecentPositiveEIDTest(false, -1, SimContext::EID_TEST_TYPE_BASE );
		//Remove all scheduled EID results and tests
		removePendingEIDTestsAndResults();
	}
	else{
		if (patient->generalState.tracingEnabled)
			tracer->printTrace(1, "**%d EID patient failed to link\n", patient->generalState.monthNum);

		if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG){
			setFalsePositiveStatus(true, false);
			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d EID patient becomes false positive unlinked\n", patient->generalState.monthNum);
		}

		//Remove all scheduled non OI EID results and tests
		removePendingEIDTestsAndResults(false);
	}
	//whether or not they linked, remove from EID regular testing
	setCanReceiveEID(false);

} /* end rollForEIDLinkage */

/** \brief addPendingResultEIDTest adds a result to the number of pending eid test results
 *
 * \param baseAssay  is the index of the original non conf test that started the chain of tests (not the index of any of the conf tests)
 * \param testAssay is the index of the test result
 * \param testType specifies whether this is a standard test or a conf test
 * \param result gives the test result
 * \param monthOfReturn is the month the test result will be known
 * \param willBeReturnedToPatient is whether the patient receives the result
 **/
void StateUpdater::addPendingResultEIDTest(int baseAssay, int testAssay, SimContext::EID_TEST_TYPE testType, bool result, int monthOfReturn, bool willBeReturnedToPatient, bool triggeredByOI){
	SimContext::EIDTestState testResult;
	testResult.baseAssay = baseAssay;
	testResult.testAssay = testAssay;
	testResult.monthToReturn = monthOfReturn;
	testResult.result = result;
	testResult.testType = testType;
	testResult.returnToPatient = willBeReturnedToPatient;
	testResult.triggeredByOI = triggeredByOI;

	patient->pedsState.eidPendingTestResults.push_back(testResult);
} /* end addPendingResultEIDTest */

/** \brief performEIDResultReturnUpdates processes the return of EID test results and rolls for linkage to care **/
void StateUpdater::performEIDResultReturnUpdates() {
	//Check to see all results that should be returned this week
	int i = 0;

	while (i < patient->pedsState.eidPendingTestResults.size()){
		int baseAssay = patient->pedsState.eidPendingTestResults[i].baseAssay;
		int testAssay = patient->pedsState.eidPendingTestResults[i].testAssay;
		SimContext::EID_TEST_TYPE testType = patient->pedsState.eidPendingTestResults[i].testType;
		int monthToReturn = patient->pedsState.eidPendingTestResults[i].monthToReturn;
		bool returnToPatient = patient->pedsState.eidPendingTestResults[i].returnToPatient;
		bool result = patient->pedsState.eidPendingTestResults[i].result;
		bool triggeredByOI = patient->pedsState.eidPendingTestResults[i].triggeredByOI;

		//Is month to get this result
		if (patient->generalState.monthNum == monthToReturn){
			// Slightly hackish way to get a regular iterator from a const iterator, this is necessary
			//	since erase requires a regular iterator
			vector<SimContext::EIDTestState>::const_iterator beginIter = patient->pedsState.eidPendingTestResults.begin();
			vector<SimContext::EIDTestState>::iterator eraseIter = patient->pedsState.eidPendingTestResults.begin();
			advance(eraseIter, i);

			//remove element from vector
			patient->pedsState.eidPendingTestResults.erase(eraseIter);

			//update provider knowledge of the result
			if (result == true)
				setMostRecentPositiveEIDTest(true, baseAssay, testType);
			else
				setMostRecentNegativeEIDTest(true);
			//if result is not returned to patient return
			if (!returnToPatient)
				continue;

			if (patient->generalState.tracingEnabled)
					tracer->printTrace(1, "**%d EID test %d %s, %s result returned to patient\n",
							patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType], result? "Positive":"Negative");
			if(result == false){
				//A new negative result will reset false positive status.
				if (patient->pedsState.isFalsePositive){
					setFalsePositiveStatus(false, false);
					if (patient->generalState.tracingEnabled)
						tracer->printTrace(1, "**%d EID patient no longer false positive\n", patient->generalState.monthNum);
				}
			}
			else{
				//check to see if maternal status should become known after positive result
				if (patient->pedsState.maternalStatus != SimContext::PEDS_MATERNAL_STATUS_NEG && !patient->pedsState.maternalStatusKnown){
					SimContext::EIDInputs::EIDTest EIDTest = simContext->getEIDInputs()->EIDTests[testAssay];

					//roll for maternal status becoming known
					double randNum = CepacUtil::getRandomDouble(100195, patient);

					if (randNum < EIDTest.EIDProbMaternalStatusKnownOnPosResult){
						setMaternalStatusKnown(true);
						if (patient->generalState.tracingEnabled) {
							tracer->printTrace(1, "**%d MATERNAL HIV Status Known\n", patient->generalState.monthNum);
						}
						//Check if mother should start ART
						if (patient->pedsState.isMotherAlive){
							randNum = CepacUtil::getRandomDouble(100196, patient);
							SimContext::PEDS_MATERNAL_STATUS momHIVState = patient->pedsState.maternalStatus;
							bool isOnART = false;
							if (momHIVState!=SimContext::PEDS_MATERNAL_STATUS_NEG){
								if (randNum < simContext->getPedsInputs()->probMotherOnARTInitial[momHIVState])
									isOnART = true;
							}

							bool isSuppressed = false;
							if (isOnART){
								randNum = CepacUtil::getRandomDouble(100197, patient);
								if (randNum < simContext->getPedsInputs()->probMotherOnSuppressedART[momHIVState])
									isSuppressed = true;

								randNum = CepacUtil::getRandomDouble(100197, patient);
								bool suppressionKnown = false;

								double prob = 0;
								if(isSuppressed)
									prob = simContext->getPedsInputs()->probMotherKnownSuppressed[momHIVState];
								else
									prob= simContext->getPedsInputs()->probMotherKnownNotSuppressed[momHIVState];

								if (randNum < prob)
									suppressionKnown = true;

								setMaternalARTStatus(isOnART, isSuppressed, suppressionKnown, true);
								if (patient->generalState.tracingEnabled) {
									tracer->printTrace(1, "**%d MATERNAL HIV Start ART\n", patient->generalState.monthNum);
								}

								//Roll for stop BF upon status known not supp
								if (patient->pedsState.motherOnART && !patient->pedsState.motherOnSuppressedART && patient->pedsState.motherSuppressionKnown){
									randNum = CepacUtil::getRandomDouble(90009, patient);
									double prob = 0;
									if (patient->pedsState.motherLowHVL)
										prob = simContext->getPedsInputs()->probStopBFNotSuppressedLowHVL;
									else
										prob = simContext->getPedsInputs()->probStopBFNotSuppressedHighHVL;

									if (randNum < prob){
										setBreastfeedingStatus(SimContext::PEDS_BF_REPL);
										if (patient->generalState.tracingEnabled) {
											tracer->printTrace(1, "**%d PEDS BREASTFEEDING STATUS: %s\n",  patient->generalState.monthNum, SimContext::PEDS_BF_TYPE_STRS[patient->pedsState.breastfeedingStatus]);
										}
									}
								}


							}
						}
					}
				}

				//Check to see if more confirmatory tests are needed
				SimContext::EIDInputs::EIDTest baseEIDTest = simContext->getEIDInputs()->EIDTests[baseAssay];
				//Schedule conf tests if necessary
				bool rollForLinkage = true;
				if (testType == SimContext::EID_TEST_TYPE_BASE && baseEIDTest.EIDFirstConfirmatoryTestAssay != SimContext::NOT_APPL){
					scheduleEIDConfirmatoryTest(baseAssay, baseEIDTest.EIDFirstConfirmatoryTestAssay, SimContext::EID_TEST_TYPE_FIRST_CONF, patient->generalState.monthNum + baseEIDTest.EIDFirstConfirmatoryTestDelay, triggeredByOI);
					rollForLinkage = false;
				}
				if (testType == SimContext::EID_TEST_TYPE_FIRST_CONF && baseEIDTest.EIDSecondConfirmatoryTestAssay != SimContext::NOT_APPL){
					scheduleEIDConfirmatoryTest(baseAssay, baseEIDTest.EIDSecondConfirmatoryTestAssay, SimContext::EID_TEST_TYPE_SECOND_CONF, patient->generalState.monthNum + baseEIDTest.EIDSecondConfirmatoryTestDelay, triggeredByOI);
					rollForLinkage = false;
				}

				//This is the last test in the chain so process final positive result

				if(rollForLinkage){
					//set detected state if not false positive
					if(patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG){
						bool wasPrevDetected = patient->monitoringState.isDetectedHIVPositive;
						if (triggeredByOI){
							if (wasPrevDetected)
								setDetectedHIVState(true, SimContext::HIV_DET_OI_PREV_DET);
							else
								setDetectedHIVState(true, SimContext::HIV_DET_OI);
						}
						else{
							if (wasPrevDetected)
								setDetectedHIVState(true, SimContext::HIV_DET_SCREENING_PREV_DET);
							else
								setDetectedHIVState(true, SimContext::HIV_DET_SCREENING);
						}
					}

					//roll for linkage to care
					rollForEIDLinkage(baseAssay, triggeredByOI);

				}
			}
		}
		else{
			i++;
		}
	}
} /* end performEIDResultReturnUpdates */


/** \brief performEIDFirstConfirmatoryTests processes first confirmatory tests that are scheduled */
void StateUpdater::performEIDFirstConfirmatoryTests(){
	int i = 0;

	while (i < patient->pedsState.eidScheduledConfirmatoryTests.size()) {
		const SimContext::EIDTestState &currTest = patient->pedsState.eidScheduledConfirmatoryTests[i];
		if (currTest.testType == SimContext::EID_TEST_TYPE_FIRST_CONF && currTest.monthToReturn == patient->generalState.monthNum){
			performEIDTest(currTest.baseAssay, currTest.testAssay, SimContext::EID_TEST_TYPE_FIRST_CONF, currTest.triggeredByOI, false);

			// Slightly hackish way to get a regular iterator from a const iterator, this is necessary
			//	since erase requires a regular iterator
			vector<SimContext::EIDTestState>::const_iterator beginIter = patient->pedsState.eidScheduledConfirmatoryTests.begin();
			vector<SimContext::EIDTestState>::iterator eraseIter = patient->pedsState.eidScheduledConfirmatoryTests.begin();
			advance(eraseIter, i);

			//remove element from vector
			patient->pedsState.eidScheduledConfirmatoryTests.erase(eraseIter);
		}
		else
			i++;
	}
} /* end performEIDFirstConfirmatoryTests */

/** \brief performEIDFirstConfirmatoryTests processes second confirmatory tests that are scheduled */
void StateUpdater::performEIDSecondConfirmatoryTests(){
	int i = 0;

	while( i < patient->pedsState.eidScheduledConfirmatoryTests.size()) {
		const SimContext::EIDTestState &currTest = patient->pedsState.eidScheduledConfirmatoryTests[i];
		if (currTest.testType == SimContext::EID_TEST_TYPE_SECOND_CONF && currTest.monthToReturn == patient->generalState.monthNum){
			performEIDTest(currTest.baseAssay, currTest.testAssay, SimContext::EID_TEST_TYPE_SECOND_CONF, currTest.triggeredByOI, false);

			// Slightly hackish way to get a regular iterator from a const iterator, this is necessary
			//	since erase requires a regular iterator
			vector<SimContext::EIDTestState>::const_iterator beginIter = patient->pedsState.eidScheduledConfirmatoryTests.begin();
			vector<SimContext::EIDTestState>::iterator eraseIter = patient->pedsState.eidScheduledConfirmatoryTests.begin();
			advance(eraseIter, i);

			//remove element from vector
			patient->pedsState.eidScheduledConfirmatoryTests.erase(eraseIter);
		}
		else
			i++;
	}

} /* end performEIDSecondConfirmatoryTests */

/** scheduleEIDConfirmatoryTest schedules the next confirmatory test after a positive result for that line of EID tests
 *
 * \param baseAssay  is the index of the original non conf test that started the chain of tests (not the index of any of the conf tests)
 * \param testAssay is the index of the test result
 * \param testType specifies which conf test this is
 * \param month is the month the conf test visit will be scheduled for
 **/
void StateUpdater::scheduleEIDConfirmatoryTest(int baseAssay, int testAssay, SimContext::EID_TEST_TYPE testType, int month, bool triggeredByOI){
	SimContext::EIDTestState scheduledTest;
	scheduledTest.baseAssay = baseAssay;
	scheduledTest.testAssay = testAssay;
	scheduledTest.monthToReturn = month;
	scheduledTest.triggeredByOI = triggeredByOI;
	//Result is arbitrary since we do not use this information for scheduled tests
	scheduledTest.result = true;
	scheduledTest.testType = testType;
	//Arbitrary
	scheduledTest.returnToPatient = true;


	if (patient->generalState.tracingEnabled)
		tracer->printTrace(1, "**%d EID test %d %s, scheduled for month %d\n",
				patient->generalState.monthNum, testAssay, SimContext::EID_TEST_TYPE_STRS[testType], month);
	patient->pedsState.eidScheduledConfirmatoryTests.push_back(scheduledTest);
} /* end scheduleEIDConfirmatoryTest */

/** \brief setMostRecentPositiveEIDTest sets the most recent positive EID test result that did not link to care
 *
 * \param hasMostrecent  is a boolean specifying whether a most recent test result exists
 * \param baseAssay is the index of the base assay of the chain
 * \param testType specifies which test type this is
 **/
void StateUpdater::setMostRecentPositiveEIDTest(bool hasMostRecent, int baseAssay, SimContext::EID_TEST_TYPE testType){
	patient->pedsState.hasMostRecentPositiveEIDTest = hasMostRecent;
	patient->pedsState.mostRecentPositiveEIDTestBaseAssay = baseAssay;
	patient->pedsState.mostRecentPositiveEIDTestType = testType;
	
	if(hasMostRecent)
		setMostRecentNegativeEIDTest(false);
} /* end setMostRecentPositiveEIDTest */

/** setMostRecentNegativeEIDTest sets the most recent negative EID test result and the month of the test
 *
 * \param hasMostRecent  is a boolean specifying whether a most recent test result exists
 **/
void StateUpdater::setMostRecentNegativeEIDTest(bool hasMostRecent){
	patient->pedsState.hasMostRecentNegativeEIDTest = hasMostRecent;

	if (hasMostRecent){
		patient->pedsState.monthOfMostRecentNegativeEIDTest = patient->generalState.monthNum;
		setMostRecentPositiveEIDTest(false, -1, SimContext::EID_TEST_TYPE_BASE);
	}	
} /* end setMostRecentNegativeEIDTest */

/** \brief setMissedEIDTest sets the most recent missed scheduled EID test due to a missed visit
 *
 * \param hasMostrecent  is a boolean specifying whether a most recent test result exists
 * \param baseAssay is the index of the base assay of the chain
 * \param testType specifies which test type this is
 **/
void StateUpdater::setMissedEIDTest(bool hasMissed, int baseAssay){
	patient->pedsState.hasMissedEIDTest = hasMissed;
	patient->pedsState.missedEIDTestBaseAssay = baseAssay;
} /* end setMissedEIDTest */

/** \brief setInfantHIVProphEffProb sets the prob of an infant HIV proph dose being effective
 *
 * \param probEff  is a double for the prob of dose efficacy
 * \param prophNum is the index of the infant HIV prophylactic
 **/
void StateUpdater::setInfantHIVProphEffProb(int prophNum, double probEff){
	patient->pedsState.probHIVProphEffective[prophNum] = probEff;
} /* end setInfantHIVProphEffProb */

/** \brief setInfantHIVProph sets the infant HIV proph state and month of proph
 *
 * \param prophNum is the index of the infant HIV prophylactic
 * \param hasProph  is a bool for if they have proph
 * \param isEff  is a bool for if the dose was effective
 * \param isInit a bool indicating whether this is called at initiation
 **/
void StateUpdater::setInfantHIVProph(int prophNum, bool hasProph, bool isEff, bool isInit){

	if (isInit){
		patient->pedsState.hasEffectiveInfantHIVProph[prophNum] = false;
		patient->pedsState.everInfantHIVProph[prophNum] = false;
	}
	else{
		if (hasProph){
			patient->pedsState.everInfantHIVProph[prophNum] = true;
			patient->pedsState.hasEffectiveInfantHIVProph[prophNum] = isEff;
			if (isEff)
				patient->pedsState.monthOfEffectiveInfantHIVProph[prophNum] = patient->generalState.monthNum;

		}
	}
} /* end setInfantHIVProph */

/** \brief setInfantHIVProphMajorToxicity sets the toxicity from infant HIV proph and the month in which it occurred
 *
 * \param hasTox  is a bool for if they have major tox
 * \param prophNum is the index of the infant HIV prophylactic
 **/
void StateUpdater::setInfantHIVProphMajorToxicity(int prophNum, bool hasTox){
	patient->pedsState.hasInfantHIVProphMajorToxicity[prophNum] = hasTox;
	if(hasTox)
		patient->pedsState.monthOfInfantHIVProphMajorToxicity[prophNum] = patient->generalState.monthNum;
} /* end setInfantHIVProphMajorToxicity */

/** \brief setClinicVisitType sets the conditions for a clinic visit and available treatments
 *
 * \param visitType a SimContext::CLINIC_VISITS specifying what type of clinic visit this is
 * \param treatmentType a SimContext::THERAPY_IMPL indicating what the treatment type the patient has available
 *
 * If treatmentType is SimContext::THERAPY_IMPL_NONE, the patient may neither receive ART nor receive prophylaxis.
 * If treatmentType is SimContext::THERAPY_IMPL_PROPH, the patient may receive prophylaxis, but not ART.
 * If treatmentType is SimContext::THERAPY_IMPL_PROPH_ART, the patient may receive both ART and prophylaxis
 * */
void StateUpdater::setClinicVisitType(SimContext::CLINIC_VISITS visitType, SimContext::THERAPY_IMPL treatmentType) {
	patient->monitoringState.clinicVisitType = visitType;
	switch (treatmentType) {
		case SimContext::THERAPY_IMPL_NONE:
			patient->prophState.mayReceiveProph = false;
			patient->artState.mayReceiveART = false;
			break;
		case SimContext::THERAPY_IMPL_PROPH:
			patient->prophState.mayReceiveProph = true;
			patient->artState.mayReceiveART = false;
			break;
		case SimContext::THERAPY_IMPL_PROPH_ART:
		default:
			patient->prophState.mayReceiveProph = true;
			patient->artState.mayReceiveART = true;
			break;
	}
} /* end setClinicVisitType */

/** \brief setAdherenceInterventionState sets whether the patient is on an adherence intervention
 *
 * \param isOnIntervention is a bool for patients intervention state
 * */
void  StateUpdater::setAdherenceInterventionState(bool isOnIntervention, int interventionIndex, int duration){
	patient->generalState.isOnAdherenceIntervention = isOnIntervention;
	if (isOnIntervention){
		patient->generalState.monthStartAdherenceIntervention = patient->generalState.monthNum;
		if (duration < 0)
			duration = 0;
		patient->generalState.monthToEndAdherenceIntervention = patient->generalState.monthNum + duration;
		patient->generalState.currAdherenceIntervention = interventionIndex;
	}
	else
		patient->generalState.currAdherenceIntervention = SimContext::NOT_APPL;
} /* end setAdherenceInterventionState */

/** \brief setNextAdherenceIntervention sets the index of the next adherence intervention
 *
 * \param interventionIndex is a int for the next intervention index
 * */
void  StateUpdater::setNextAdherenceIntervention(int interventionIndex){
	patient->generalState.nextAdherenceIntervention = interventionIndex;
} /* end setNextAdherenceIntervention */


/** \brief setResponseBaseline sets the baseline propensity to respond coefficient
 *
 * \param baseline a double representing the baseline propensity to respond logit
 **/
void StateUpdater::setResponseBaseline(double baseline) {
	patient->generalState.responseBaselineLogit = baseline;
} /* end setResponseBaseline */

/** \brief setPreARTResponseBase sets the baseline propensity to respond coefficient for preART
 *
 * \param baseline a double representing the baseline propensity to respond logit
 **/
void StateUpdater::setPreARTResponseBase(double baseline) {
	patient->generalState.responseLogitPreARTBase = baseline;
} /* end setPreARTResponseBase */

/** \brief setARTResponsCurrRegimenBase sets the propensity to respond coefficient for the current ART regimen without adherence interventions
 *
 * \param responseLogit a double representing the propensity to respond logit including the increment
 * \param responseRegimenIncrLogit is the regimen specific increment to propensity to respond
 * \param responseLogitPreIncr is the regression coefficient before the responseRegimenIncrLogit is added
 * \param enableDuration is a bool for whether or not to inplement a limited duration for the increment
 * \param duration is the duration for which the increment is added
 **/
void StateUpdater::setARTResponseCurrRegimenBase(double responseLogit, double responseRegimenIncrLogit, double responseLogitPreIncr, bool enableDuration, int duration) {
	patient->artState.responseLogitCurrRegimenBase = responseLogit;
	patient->artState.responseLogitCurrRegimenIncrement = responseRegimenIncrLogit;
	patient->artState.responseLogitCurrRegimenPreIncrement = responseLogitPreIncr;
	patient->artState.enableDurationCurrRegimenResponseIncrement = enableDuration;
	patient->artState.monthToStopCurrRegimenResponseIncrement = patient->generalState.monthNum + duration;
} /* end setARTResponseCurrRegimenBase */

/** \brief setAdherenceInterventionResponseAdjustment sets the adjustment to logit for propensity if adherence intervention is active
 *
 * 	\param adjustment is the adjustment to be applied to logit propensity
 * */
void StateUpdater::setAdherenceInterventionResponseAdjustment(double adjustment){
	patient->generalState.responseLogitAdherenceInterventionIncrement = adjustment;
} /* end setAdherenceInterventionResponseAdjustment */


/** \brief setCD4ResponseType sets the predisposed CD4 response type of the patient
 *
 * \param responseType the SimContext::CD4_RESPONSE_TYPE to set as the patient's CD4 response type
 **/
void StateUpdater::setCD4ResponseType(SimContext::CD4_RESPONSE_TYPE responseType) {
	patient->artState.CD4ResponseType = responseType;
} /* end setCD4ResponseType */

/** \brief setRiskFactor sets whether or not the patient has risk factor x
 *
 * \param riskNum an integer specifying the risk number
 * \param hasRisk a bool specifying whether or not the patient has the specified risk factor
 * \param isInitial a bool indicating whether or not this is the initial state of the risk factor
 *
 * If isInitial is true, the runStats corresponding to tallying the initial distribution of risk factors is incremented
 **/
void StateUpdater::setRiskFactor(int riskNum, bool hasRisk, bool isInitial) {
	patient->generalState.hasRiskFactor[riskNum] = hasRisk;

	if (isInitial && hasRisk) {
		runStats->initialDistributions.numRiskFactors[riskNum]++;
	}
} /* end setRiskFactor */

/** \brief setTransmRiskCategory sets the transmission risk category for the patient
 *
 * \param risk is the transmission risk category the patient belongs to
 *
 **/
void StateUpdater::setTransmRiskCategory(SimContext::TRANSM_RISK risk) {
	patient->generalState.transmRiskCategory = risk;
}

/** \brief scheduleInitialCD4Test determines the CD4 testing interval and schedules CD4 tests accordingly. It is "initial" only in the sense that the interval may need to be updated due to changes in care state, not necessarily the first ever test to become available
 *
 * \see StateUpdater::scheduleCD4Test(bool, int)
 * \see StateUpdater::scheduleHVLTest(bool, int)
 * */
void StateUpdater::scheduleInitialCD4Test(int monthNum) {
	if (!patient->artState.hasTakenART){
		if(patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			if (patient->diseaseState.currTrueCD4 > simContext->getTreatmentInputs()->testingIntervalCD4Threshold) {
				if (simContext->getTreatmentInputs()->CD4TestingIntervalPreARTHighCD4 != SimContext::NOT_APPL)
					scheduleCD4Test(true, monthNum);
				else
					scheduleCD4Test(false);
			}
			else {
				if (simContext->getTreatmentInputs()->CD4TestingIntervalPreARTLowCD4 != SimContext::NOT_APPL)
					scheduleCD4Test(true, monthNum);
				else
					scheduleCD4Test(false);
			}
		}
		else if(patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
			if (simContext->getPedsInputs()->CD4TestingIntervalPreARTLate != SimContext::NOT_APPL)
				scheduleCD4Test(true, monthNum);
			else
				scheduleCD4Test(false);
		}	
		else{
			if (simContext->getPedsInputs()->CD4TestingIntervalPreARTEarly != SimContext::NOT_APPL)
				scheduleCD4Test(true, monthNum);
			else
				scheduleCD4Test(false);
		}
	}
	else if (patient->artState.hasNextRegimenAvailable){
		int monthsOnART = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;
		if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalARTMonthsThreshold){
			if(simContext->getTreatmentInputs()->CD4TestingIntervalOnART[0] != SimContext::NOT_APPL){
				scheduleCD4Test(true,monthNum);
			}
			else{
				scheduleCD4Test(false);
			}
		}
		else{
			if(simContext->getTreatmentInputs()->CD4TestingIntervalOnART[1]!= SimContext::NOT_APPL){
				scheduleCD4Test(true,monthNum);
			}
			else{
				scheduleCD4Test(false);
			}
		}
	}
	else if (!patient->artState.hasObservedFailure){
		int monthsOnART = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;
		if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalLastARTMonthsThreshold){
			if(simContext->getTreatmentInputs()->CD4TestingIntervalOnLastART[0] != SimContext::NOT_APPL){
				scheduleCD4Test(true,monthNum);
			}
			else{
				scheduleCD4Test(false);
			}
		}
		else{
			if(simContext->getTreatmentInputs()->CD4TestingIntervalOnLastART[1]!= SimContext::NOT_APPL){
				scheduleCD4Test(true,monthNum);
			}
			else{
				scheduleCD4Test(false);
			}
		}
	}
	else{
		if(simContext->getTreatmentInputs()->CD4TestingIntervalPostART != SimContext::NOT_APPL){
			scheduleCD4Test(true,monthNum);
		}
		else{
			scheduleCD4Test(false);
		}
	}
} /* end scheduleInitialCD4Test */

/** \brief scheduleInitialHVLTest determines the HVL testing interval and schedules HVL tests accordingly. It is "initial" only in the sense that the interval may need to be updated due to changes in care state, not necessarily the first ever test to become available
 *
 * \see StateUpdater::scheduleCD4Test(bool, int)
 * \see StateUpdater::scheduleHVLTest(bool, int)
 * */
void StateUpdater::scheduleInitialHVLTest(int monthNum) {
	if (!patient->artState.hasTakenART){
		if(patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			if (patient->diseaseState.currTrueCD4 > simContext->getTreatmentInputs()->testingIntervalCD4Threshold) {
				if (simContext->getTreatmentInputs()->HVLTestingIntervalPreARTHighCD4 != SimContext::NOT_APPL)
					scheduleHVLTest(true, monthNum);
				else
					scheduleHVLTest(false);
			}
			else {
				if (simContext->getTreatmentInputs()->HVLTestingIntervalPreARTLowCD4 != SimContext::NOT_APPL)
					scheduleHVLTest(true, monthNum);
				else
					scheduleHVLTest(false);
			}
		}
		else if(patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
			if (simContext->getPedsInputs()->HVLTestingIntervalPreARTLate != SimContext::NOT_APPL)
				scheduleHVLTest(true, monthNum);
			else
				scheduleHVLTest(false);
		}	
		else{
			if (simContext->getPedsInputs()->HVLTestingIntervalPreARTEarly != SimContext::NOT_APPL)
				scheduleHVLTest(true, monthNum);
			else
				scheduleHVLTest(false);
		}
	}
	else if (patient->artState.hasNextRegimenAvailable){
		int monthsOnART = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;
		if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalARTMonthsThreshold){
			if(simContext->getTreatmentInputs()->HVLTestingIntervalOnART[0] != SimContext::NOT_APPL){
				scheduleHVLTest(true,monthNum);
			}
			else{
				scheduleHVLTest(false);
			}
		}
		else{
			if(simContext->getTreatmentInputs()->HVLTestingIntervalOnART[1]!= SimContext::NOT_APPL){
				scheduleHVLTest(true,monthNum);
			}
			else{
				scheduleHVLTest(false);
			}
		}
	}
	else if (!patient->artState.hasObservedFailure){
		int monthsOnART = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;
		if (monthsOnART < simContext->getTreatmentInputs()->testingIntervalLastARTMonthsThreshold){
			if(simContext->getTreatmentInputs()->HVLTestingIntervalOnLastART[0] != SimContext::NOT_APPL){
				scheduleHVLTest(true,monthNum);
			}
			else{
				scheduleHVLTest(false);
			}
		}
		else{
			if(simContext->getTreatmentInputs()->HVLTestingIntervalOnLastART[1]!= SimContext::NOT_APPL){
				scheduleHVLTest(true,monthNum);
			}
			else{
				scheduleHVLTest(false);
			}
		}
	}
	else{
		if(simContext->getTreatmentInputs()->HVLTestingIntervalPostART != SimContext::NOT_APPL){
			scheduleHVLTest(true,monthNum);
		}
		else{
			scheduleHVLTest(false);
		}
	}
} /* end scheduleInitialHVLTest */

/** \brief scheduleInitialClinicVisit sets the month of initial clinic visit, CD4 test, and HVL test
 *
 * The initial clinic visit is set to occur in the current month.  Depending on user specification for
 * pre-ART behavior with a given CD4 threshold, CD4 and HVL tests either are or aren't scheduled for the
 * current month based on the patient's CD4 count.
 *
 * \see StateUpdater::scheduleInitialCD4Test(int)
 * \see StateUpdater::scheduleInitialHVLTest(int)
 * */
void StateUpdater::scheduleInitialClinicVisit(int monthOfVisit) {
	int monthNum;
	if (monthOfVisit == SimContext::NOT_APPL)
		monthNum = patient->generalState.monthNum;
	else
		monthNum = monthOfVisit;

	scheduleRegularClinicVisit(true, monthNum);
	if(patient->generalState.monthNum>=simContext->getTreatmentInputs()->CD4TestingLag && patient->monitoringState.CD4TestingAvailable){
		scheduleInitialCD4Test(monthNum);
	}
	if(patient->generalState.monthNum>=simContext->getTreatmentInputs()->HVLTestingLag){
		scheduleInitialHVLTest(monthNum);
	}
} /* end scheduleInitialClinicVisit */

/** \brief scheduleRegularClinicVisit sets the month of the next regularly scheduled clinic visit
 *
 * \param hasNext a bool indicating whether or not the patient should schedule an upcoming clinic visit
 * \param monthNum an integer indicating which month the regular clinic visit should be scheduled for if hasNext is true
 * \param scheduleInitialCD4 indicates whether to attempt to schedule an initial cd4 test or not.  This is by default true
 * \param scheduleInitialHVL indicates whether to attempt to schedule an initial HVL test or not.  This is by default true
 **/
void StateUpdater::scheduleRegularClinicVisit(bool hasNext, int monthNum,bool scheduleInitialCD4,bool scheduleInitialHVL) {
	patient->monitoringState.hasRegularClinicVisit = hasNext;
	if (hasNext){
		patient->monitoringState.monthOfRegularClinicVisit = monthNum;
		if(scheduleInitialCD4){
			if(monthNum>=simContext->getTreatmentInputs()->CD4TestingLag && !patient->monitoringState.hadChanceCD4Test && patient->monitoringState.CD4TestingAvailable){
				scheduleInitialCD4Test(monthNum);
			}
		}
		if(scheduleInitialHVL){
			if(monthNum>=simContext->getTreatmentInputs()->HVLTestingLag && !patient->monitoringState.hadChanceHVLTest){
				scheduleInitialHVLTest(monthNum);
			}
		}
	}
} /* end scheduleRegularClinicVisit */

/** \brief scheduleEmergencyClinicVisit sets the month of the next emergency clinic visit
 *
 * \param emergencyType a SimContext::EMERGENCY_TYPE indicating the type of emergency associated with this clinic visit
 * \param monthNum an integer indicating which month the emergency clinic visit should be scheduled for if hasNext is true
 * */
void StateUpdater::scheduleEmergencyClinicVisit(SimContext::EMERGENCY_TYPE emergencyType, int monthNum,bool scheduleInitialCD4,bool scheduleInitialHVL) {
	patient->monitoringState.emergencyClinicVisitType = emergencyType;

	if (emergencyType != SimContext::EMERGENCY_NONE){
		patient->monitoringState.monthOfEmergencyClinicVisit = monthNum;
		if(scheduleInitialCD4){
			if(monthNum>=simContext->getTreatmentInputs()->CD4TestingLag && !patient->monitoringState.hadChanceCD4Test && patient->monitoringState.CD4TestingAvailable){
				scheduleInitialCD4Test(monthNum);
			}
		}
		if(scheduleInitialHVL){
			if(monthNum>=simContext->getTreatmentInputs()->HVLTestingLag && !patient->monitoringState.hadChanceHVLTest){
				scheduleInitialHVLTest(monthNum);
			}
		}
	}
} /* end scheduleEmergencyClinicVisit */

/** \brief resetCliniVisitState resets state keeping track of event since the last clinic visit
 *
 * \param isInitial a bool indicating whether or not this is called before the first clinic visit.
 * If isInitial is true, "hadPrevClinicVisit" is set to false (because no clinic visit has occurred).
 * Otherwise, "hadPrevClinicVisit" is set to true.
 *
 * The counters for number of observed and true OIs since last clinic visit are also reset by this function
 **/
void StateUpdater::resetClinicVisitState(bool isInitial) {
	if (isInitial)
		patient->monitoringState.hadPrevClinicVisit = false;
	else
		patient->monitoringState.hadPrevClinicVisit = true;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (isInitial)
			patient->monitoringState.numObservedOIsTotal[i] = 0;
		patient->diseaseState.numTrueOIsSinceLastVisit[i] = 0;
		patient->monitoringState.numObservedOIsSinceLastVisit[i] = 0;
	}
}

/** \brief incrementNumClinicVisits increments the total number of clinic visits by 1*/
void StateUpdater::incrementNumClinicVisits() {
	runStats->popSummary.totalClinicVisits++;
	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Num Clinic Visits
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numClinicVisits++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numClinicVisits++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numClinicVisits++;
		}
	}
} /* end incrementNumClinicVisits */

/** \brief incrementNumObservedOIs increments the patients observed OIs during a clinic visit
 *
 * \param oiType a SimContext::OI_TYPE specifying the OI that was observed
 * \param numObserved the number of OIs that were observed
 *
 * The patient's number of observed OIs of type oiType are incremented by numObserved.
 **/
void StateUpdater::incrementNumObservedOIs(SimContext::OI_TYPE oiType, int numObserved) {
	// Update the patient state for the observed OI
	patient->monitoringState.numObservedOIsTotal[oiType] += numObserved;
	patient->monitoringState.numObservedOIsSinceLastVisit[oiType] += numObserved;
	patient->artState.numObservedOIsSinceFailOrStopART[oiType] += numObserved;

	// Update the statistics for the observed OI
	SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
	runStats->oiStats.numDetectedOIsCD4OI[cd4Strata][oiType] += numObserved;
} /* end incrementNumObservedOIs */

/** \brief setCurrLTFUStats updates the state and statisitics for a patient being LTFU or RTC
 *
 * \param ltfuState a SimContext::LTFU_STATE indicating which LTFU state to set the patient to
 *
 * The month of state change is recorded, as well as the number of months in the previous state.
 * If the new state is SimContext::LTFU_STATE_LOST, the patient's monitoring state keeps track of
 * whether or not the patient was on ART when entering this state.  The patient's previous state
 * is recorded in the monitoring state.  RunStats statistics related to LTFU are also updated.
 **/
void StateUpdater::setCurrLTFUState(SimContext::LTFU_STATE ltfuState) {
	// Update patient state for initial state, lost to follow up, or return to care
	patient->monitoringState.currLTFUState = ltfuState;
	int monthsPrevState = patient->generalState.monthNum - patient->monitoringState.monthOfLTFUStateChange;
	patient->monitoringState.monthOfLTFUStateChange = patient->generalState.monthNum;
	if (ltfuState == SimContext::LTFU_STATE_LOST) {
		setCareState(SimContext::HIV_CARE_LTFU);
		if (patient->artState.isOnART)
			patient->monitoringState.wasOnARTWhenLostToFollowUp = true;
		else
			patient->monitoringState.wasOnARTWhenLostToFollowUp = false;
	}

	// Update statistics for lost to follow up or return to care
	if (ltfuState == SimContext::LTFU_STATE_LOST) {
		runStats->ltfuStats.numLostToFollowUpCD4[patient->diseaseState.currTrueCD4Strata]++;
		if (!patient->monitoringState.hadPrevLTFU)
			runStats->ltfuStats.numPatientsLost++;
		if (patient->artState.isOnART)
			runStats->ltfuStats.numLostToFollowUpART[patient->artState.currRegimenNum]++;
		else if (!patient->artState.hasTakenART)
			runStats->ltfuStats.numLostToFollowUpPreART++;
		else
			runStats->ltfuStats.numLostToFollowUpPostART++;
		// Update longitudinal statistics for LTFU
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if (currTime) {
			if (patient->artState.isOnART)
				currTime->numStartingLostToFollowUpART[patient->artState.currRegimenNum]++;
			else if (!patient->artState.hasTakenART)
				currTime->numStartingLostToFollowUpPreART++;
			else
				currTime->numStartingLostToFollowUpPostART++;
		}
	}
	else if (ltfuState == SimContext::LTFU_STATE_RETURNED) {
		setCareState(SimContext::HIV_CARE_RTC);
		runStats->ltfuStats.numReturnToCareCD4[patient->diseaseState.currTrueCD4Strata]++;
		runStats->ltfuStats.monthsLostBeforeReturnSum += monthsPrevState;
		runStats->ltfuStats.monthsLostBeforeReturnSumSquares += monthsPrevState * monthsPrevState;
		if (!patient->monitoringState.hadPrevRTC)
			runStats->ltfuStats.numPatientsReturned++;
		if (patient->monitoringState.wasOnARTWhenLostToFollowUp &&
			patient->artState.hasNextRegimenAvailable) {
				if (patient->artState.nextRegimenNum == patient->artState.prevRegimenNum)
					runStats->ltfuStats.numReturnOnPrevART[patient->artState.nextRegimenNum]++;
				else
					runStats->ltfuStats.numReturnOnNextART[patient->artState.prevRegimenNum]++;
		}
		else if (!patient->artState.hasTakenART) {
			runStats->ltfuStats.numReturnToCarePreART++;
		}
		else {
			runStats->ltfuStats.numReturnToCarePostART++;
		}
		// Update longitudinal statistics for RTC
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if (currTime) {
			if (patient->monitoringState.wasOnARTWhenLostToFollowUp &&
				patient->artState.hasNextRegimenAvailable) {
					if (patient->artState.nextRegimenNum == patient->artState.prevRegimenNum)
						currTime->numReturnOnPrevART[patient->artState.nextRegimenNum]++;
					else
						currTime->numReturnOnNextART[patient->artState.prevRegimenNum]++;
			}
			else if (!patient->artState.hasTakenART)
				currTime->numReturnToCarePreART++;
			else
				currTime->numReturnToCarePostART++;
		}
	}

	// Update previous LTFU/RTC state after updating statistics
	if (ltfuState == SimContext::LTFU_STATE_NONE) {
		patient->monitoringState.hadPrevLTFU = false;
		patient->monitoringState.hadPrevRTC = false;
	}
	else if ((ltfuState == SimContext::LTFU_STATE_LOST) && !patient->monitoringState.hadPrevLTFU) {
		patient->monitoringState.hadPrevLTFU = true;
	}
	else if ((ltfuState == SimContext::LTFU_STATE_RETURNED) && !patient->monitoringState.hadPrevRTC) {
		patient->monitoringState.hadPrevRTC = true;
	}
} /* end setCurrLTFUState */

/** \brief startNextARTRegimen updates the state to begin the next ART treatment regimen
 *
 *\param isResupp is true if this is a resuppression regimen
 * The patient state is updated to reflect the beginning of an ART regimen (resetting counters, etc).
 * ART initiation statistics are NOT updated here.  That is done after ART efficacy is determined
 **/
void StateUpdater::startNextARTRegimen(bool isResupp) {
	// Update patient state for beginning ART regimen
	patient->artState.isOnART = true;
	patient->artState.applyARTEffect = true;
	patient->artState.isOnResupp = isResupp;
	patient->artState.currRegimenNum = patient->artState.nextRegimenNum;
	patient->artState.monthOfCurrRegimenStart = patient->generalState.monthNum;
	patient->artState.numFailedCD4Tests = 0;
	patient->artState.numFailedHVLTests = 0;
	patient->artState.numFailedOIs = 0;
	patient->artState.hasObservedFailure = false;
	patient->artState.typeObservedFailure = SimContext::ART_FAIL_NOT_FAILED;
	if (!patient->artState.hasTakenART){
		if (patient->monitoringState.hasObservedCD4){
			SimContext::CD4_STRATA obsvCD4Strata = patient->monitoringState.currObservedCD4Strata;
			costStats->popSummary.observedCD4DistributionAtARTStart[obsvCD4Strata]++;
		}
		costStats->popSummary.genderDistributionAtARTStart[patient->generalState.gender]++;
		costStats->popSummary.ageDistributionAtARTStart[getAgeCategoryLinkageStats(patient->generalState.ageMonths)]++;
	}
	if (!patient->artState.hasTakenART || (patient->artState.currRegimenNum != patient->artState.prevRegimenNum)) {
		patient->artState.hadSuccessOnRegimen = false;
		patient->artState.indivCD4Envelope.isActive = false;
		patient->artState.indivCD4PercentageEnvelope.isActive = false;
	}
	if (patient->monitoringState.hasObservedCD4)
		patient->artState.maxObservedCD4OnCurrART = patient->monitoringState.currObservedCD4;
	if (patient->monitoringState.hasObservedCD4Percentage)
		patient->artState.maxObservedCD4PercentageOnCurrART = patient->monitoringState.currObservedCD4Percentage;
	if (patient->monitoringState.hasObservedHVLStrata) {
		patient->artState.observedHVLStrataAtRegimenStart = patient->monitoringState.currObservedHVLStrata;
		patient->artState.minObservedHVLStrataOnCurrART = patient->monitoringState.currObservedHVLStrata;
	}
	if (!patient->artState.hasTakenART){
		patient->artState.hasTakenART = true;
		patient->artState.monthFirstStartART = patient->generalState.monthNum;
	}
	// ART initiation statistics are only set once the efficacy is determined
	if (!patient->artState.hasTakenARTRegimen[patient->artState.currRegimenNum]){
		int artLineNum = patient->artState.currRegimenNum;
		runStats->artStats.numARTEverInit[artLineNum]++;
		runStats->artStats.numARTEverInitCD4Metric[artLineNum][patient->pedsState.ageCategoryCD4Metric]++;

		double currTrueCD4Value = 0; 
		if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
			currTrueCD4Value = patient->diseaseState.currTrueCD4;
		else
			currTrueCD4Value = patient->diseaseState.currTrueCD4Percentage;	
		runStats->artStats.trueCD4AtARTEverInitSum[artLineNum][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;

		if (patient->monitoringState.hasObservedCD4){
			runStats->artStats.numWithObservedCD4AtARTEverInit[artLineNum]++;
			runStats->artStats.observedCD4AtARTEverInitSum[artLineNum] += patient->monitoringState.currObservedCD4;
		}
		double responseLogit = patient->artState.responseLogitCurrRegimen;
		double propRespond = pow(1 + exp(0 - responseLogit), -1);
		runStats->artStats.propensityAtARTEverInitSum[artLineNum]+= propRespond;

		patient->artState.hasTakenARTRegimen[patient->artState.currRegimenNum] = true;
	}
} /* end startNextARTRegimen */

/** \brief startNextARTSubRegimen updates the state to begin the next ART treatment subregimen
 *
 * Only counters relating to subregimens (i.e. toxicity and subregimen counters) are updated here
 *
 * \param nextSubRegimen an integer specifying the index of the new subregimen to start
 **/
void StateUpdater::startNextARTSubRegimen(int nextSubRegimen) {
	patient->artState.currSubRegimenNum = nextSubRegimen;
	patient->artState.monthOfCurrSubRegimenStart = patient->generalState.monthNum;
	patient->artState.hasChronicToxSwitch = false;
	patient->artState.chronicToxSwitchToLine = SimContext::NOT_APPL;
	patient->artState.hasMajorToxicity = false;
	patient->artState.hasSevereToxicity = false;
} /* end startNextARTSubRegmin */

/** \brief setCurrARTEfficacy updates the destined efficacy of the ART regimen
 *
 * The ART suppression level is set based on efficacyType.  If this is the initial draw, ART statistics are
 * updated here.  Statistics are also updated if efficacyType indicates true failure.
 *
 * \param efficacyType a SimContext::ART_EFF_TYPE indicating the efficacy of the current ART regimen
 * \param isInitial a boolean that is true if this is the first time this ART regimen has had its efficacy set
 **/
void StateUpdater::setCurrARTEfficacy(SimContext::ART_EFF_TYPE efficacyType, bool isInitial) {
	// Update patient state with new ART suppression level
	patient->artState.currRegimenEfficacy = efficacyType;
	// Set month of efficacy change for initial draw or transition to failure
	if (isInitial || (efficacyType == SimContext::ART_EFF_FAILURE)) {
		patient->artState.monthOfEfficacyChange = patient->generalState.monthNum;
	}

	// Update statistics for the initial efficacy and true failure, don't update for STI restarts
	if (patient->artState.currSTIState == SimContext::STI_STATE_RESTART)
		return;

	double currTrueCD4Value = 0; 
	if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
		currTrueCD4Value = patient->diseaseState.currTrueCD4;
	else
		currTrueCD4Value = patient->diseaseState.currTrueCD4Percentage;		
	
	if (isInitial) {
		// Update statistics for beginning ART regimen, don't update for STI restarts
		int artLineNum = patient->artState.currRegimenNum;
		SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
		SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;

		SimContext::CD4_RESPONSE_TYPE cd4Response = patient->artState.CD4ResponseType;

		for(int i=0;i<SimContext::HET_NUM_OUTCOMES;i++){
			SimContext::RESP_TYPE responseType = patient->artState.responseTypeCurrRegimen[i];
			if (patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE){
				runStats->artStats.numOnARTAtInitResp[artLineNum][i][responseType]++;
				runStats->artStats.trueCD4AtInitSumResp[artLineNum][i][responseType] += patient->diseaseState.currTrueCD4;
				if(patient->monitoringState.hasObservedCD4){
					runStats->artStats.observedCD4AtInitSumResp[artLineNum][i][responseType] += patient->monitoringState.currObservedCD4;
					runStats->artStats.numWithObservedCD4AtInitResp[artLineNum][i][responseType]++;
				}	
			}
			runStats->artStats.numDrawEfficacyAtInitResp[artLineNum][efficacyType][i][responseType]++;
			runStats->artStats.numCD4ResponseTypeAtInitResp[artLineNum][cd4Response][i][responseType]++;

			for (int j = 0; j < SimContext::RISK_FACT_NUM; j++) {
				if (patient->generalState.hasRiskFactor[i]){
					runStats->artStats.numWithRiskFactorAtInitResp[artLineNum][j][i][responseType]++;
				}
			}
		}
		runStats->artStats.distributionAtInit[artLineNum][cd4Strata][hvlStrata]++;
		runStats->artStats.numOnARTAtInit[artLineNum]++;
		runStats->artStats.numOnARTAtInitCD4Metric[artLineNum][patient->pedsState.ageCategoryCD4Metric]++;
		runStats->artStats.trueCD4AtInitSum[artLineNum][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;

		if (patient->monitoringState.hasObservedCD4){
			runStats->artStats.observedCD4AtInitSum[artLineNum]+=patient->monitoringState.currObservedCD4;
			runStats->artStats.numWithObservedCD4AtInit[artLineNum]++;
		}

		runStats->artStats.numDrawEfficacyAtInit[artLineNum][efficacyType]++;
		runStats->artStats.numCD4ResponseTypeAtInit[artLineNum][cd4Response]++;

		for (int i = 0; i < SimContext::RISK_FACT_NUM; i++) {
			if (patient->generalState.hasRiskFactor[i]){
				runStats->artStats.numWithRiskFactorAtInit[artLineNum][i]++;
				break;
			}
		}
	}
	if (efficacyType == SimContext::ART_EFF_FAILURE) {
		// Update true failure statistics, same as initial if efficacy was failure
		int artLineNum = patient->artState.currRegimenNum;
		int monthsToFail = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;

		for(int i=0;i<SimContext::HET_NUM_OUTCOMES;i++){
			SimContext::RESP_TYPE responseType = patient->artState.responseTypeCurrRegimen[i];
			
			if (patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE){
				runStats->artStats.numTrueFailureResp[artLineNum][i][responseType]++;
				runStats->artStats.trueCD4AtTrueFailureSumResp[artLineNum][i][responseType] += patient->diseaseState.currTrueCD4;
				if (patient->monitoringState.hasObservedCD4){
					runStats->artStats.observedCD4AtTrueFailureSumResp[artLineNum][i][responseType] += patient->monitoringState.currObservedCD4;
					runStats->artStats.numWithObservedCD4AtTrueFailureResp[artLineNum][i][responseType]++;
				}	
			}
			runStats->artStats.monthsToTrueFailureSumResp[artLineNum][i][responseType] += monthsToFail;
			runStats->artStats.monthsToTrueFailureSumSquaresResp[artLineNum][i][responseType] += monthsToFail * monthsToFail;
		}
		runStats->artStats.numTrueFailure[artLineNum]++;
		runStats->artStats.numTrueFailureCD4Metric[artLineNum][patient->pedsState.ageCategoryCD4Metric]++;
		runStats->artStats.trueCD4AtTrueFailureSum[artLineNum][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;

		if (patient->monitoringState.hasObservedCD4){
			runStats->artStats.observedCD4AtTrueFailureSum[artLineNum] += patient->monitoringState.currObservedCD4;
			runStats->artStats.numWithObservedCD4AtTrueFailure[artLineNum]++;	
		}

		runStats->artStats.monthsToTrueFailureSum[artLineNum] += monthsToFail;
		runStats->artStats.monthsToTrueFailureSumSquares[artLineNum]+= monthsToFail * monthsToFail;
        // disable ART Effect if toggle is off
        if (patient->generalState.isAdolescent){
			if(!simContext->getAdolescentARTInputs(artLineNum)->applyARTEffectOnFailed[getAgeCategoryAdolescentART(artLineNum)])
				patient->artState.applyARTEffect = false;
		}
		else if(patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			if(!simContext->getARTInputs(artLineNum)->applyARTEffectOnFailed)
           		patient->artState.applyARTEffect = false;
        }
		else if(patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
			if(!simContext->getPedsARTInputs(artLineNum)->applyARTEffectOnFailedARTLate)
				patient->artState.applyARTEffect = false;
		} 
		else {
			if(!simContext->getPedsARTInputs(artLineNum)->applyARTEffectOnFailedARTEarly)
				patient->artState.applyARTEffect = false;
		} 
	}
} /* end setCurrARTEfficacy */

/** \brief setResuppEfficacy updates the efficacy each time a resuppression is attempted
 *
 * \param efficacyType a SimContext::ART_EFF_TYPE indicating the efficacy of the current ART regimen
 **/
void StateUpdater::setResuppEfficacy(SimContext::ART_EFF_TYPE efficacyType) {
	if (efficacyType == SimContext::ART_EFF_FAILURE)
		patient->artState.numFailedResupp++;
	else
		patient->artState.numFailedResupp = 0;
} /* end setResuppEfficacy */

/** \brief setCurrARTResponse sets the calculated ART propensity to respond and response type
 *
 * The response type is set based on propRespond and the user defined responseTypeThresholds found in
 * the heterogeneity inputs.  The response factor is calculated as:
 * \f$ \frac{propRespond - prop_{Lower}}{prop_{Upper} - prop_{Lower}}\f$
 *	this value is 0 if the response=L1 and 1 if response=L2
 *
 * \param propRespond a double indicating the propensity to respond
 **/
void StateUpdater::setCurrARTResponse(double responseLogit) {
	// Update the patient state with the propensity to respond and response type
	double propRespond = pow(1 + exp(0 - responseLogit), -1);
	int artLineNum = patient->artState.currRegimenNum;
	patient->artState.responseLogitCurrRegimen=responseLogit;
	// Print information to trace file if enabled
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d ART %d Propensity to Respond:%f, Logit Value of Propensity to Respond %f;\n",
				patient->generalState.monthNum,
				artLineNum+1,
				propRespond,
				responseLogit);
	}
	for(int i=0;i<SimContext::HET_NUM_OUTCOMES;i++){
		double L1,L2;

		if (patient->generalState.isAdolescent){
			int ayaARTAgeCat = getAgeCategoryAdolescentART(artLineNum);
			L1=simContext->getAdolescentARTInputs(artLineNum)->responseTypeThresholds[i][0][ayaARTAgeCat];
			L2=simContext->getAdolescentARTInputs(artLineNum)->responseTypeThresholds[i][1][ayaARTAgeCat];
		}
		else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
			L1=simContext->getARTInputs(artLineNum)->responseTypeThresholds[i][0];
			L2=simContext->getARTInputs(artLineNum)->responseTypeThresholds[i][1];
		}
		else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
			L1=simContext->getPedsARTInputs(artLineNum)->responseTypeThresholdsLate[i][0];
			L2=simContext->getPedsARTInputs(artLineNum)->responseTypeThresholdsLate[i][1];
		}
		else{
			L1=simContext->getPedsARTInputs(artLineNum)->responseTypeThresholdsEarly[i][0];
			L2=simContext->getPedsARTInputs(artLineNum)->responseTypeThresholdsEarly[i][1];
		}

		if (propRespond > L2) {
			patient->artState.responseTypeCurrRegimen[i] = SimContext::RESP_TYPE_FULL;
			patient->artState.responseFactorCurrRegimen[i] = 1.0;
		}
		else if (propRespond > L1) {
			patient->artState.responseTypeCurrRegimen[i] = SimContext::RESP_TYPE_PARTIAL;
			patient->artState.responseFactorCurrRegimen[i] = (propRespond - L1) / (L2 - L1);

			if (i == SimContext::HET_OUTCOME_SUPP ||
                i == SimContext::HET_OUTCOME_LATEFAIL ||
                i == SimContext::HET_OUTCOME_RESUPP){
				double exponent;
				if(patient->generalState.isAdolescent){
					int ayaARTAgeCat = getAgeCategoryAdolescentART(artLineNum);
					exponent = simContext->getAdolescentARTInputs(artLineNum)->responseTypeExponents[i][ayaARTAgeCat];
				}
				else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
					exponent = simContext->getARTInputs(artLineNum)->responseTypeExponents[i];
				}
				else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
					exponent = simContext->getPedsARTInputs(artLineNum)->responseTypeExponentsLate[i];
				}
				else{
					exponent = simContext->getPedsARTInputs(artLineNum)->responseTypeExponentsEarly[i];
				}
				patient->artState.responseFactorCurrRegimen[i] = pow(patient->artState.responseFactorCurrRegimen[i], exponent);
			}
		}
		else {
			patient->artState.responseTypeCurrRegimen[i] = SimContext::RESP_TYPE_NON;
			patient->artState.responseFactorCurrRegimen[i] = 0.0;
		}
	}

	double upperValue,lowerValue,responseFactor;

	if(patient->generalState.isAdolescent){
		int ayaARTAgeCat = getAgeCategoryAdolescentART(artLineNum);

		upperValue=simContext->getAdolescentARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_SUPP][1][ayaARTAgeCat];
		lowerValue=simContext->getAdolescentARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_SUPP][0][ayaARTAgeCat];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_SUPP];
		patient->artState.probInitialEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getAdolescentARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_RESUPP][1][ayaARTAgeCat];
		lowerValue=simContext->getAdolescentARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_RESUPP][0][ayaARTAgeCat];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_RESUPP];
		patient->artState.probResuppEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getAdolescentARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_LATEFAIL][1][ayaARTAgeCat];
		lowerValue=simContext->getAdolescentARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_LATEFAIL][0][ayaARTAgeCat];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_LATEFAIL];
		patient->artState.probLateFail=lowerValue+responseFactor*(upperValue-lowerValue);
	}
	else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
		upperValue=simContext->getARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_SUPP][1];
		lowerValue=simContext->getARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_SUPP][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_SUPP];
		patient->artState.probInitialEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_RESUPP][1];
		lowerValue=simContext->getARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_RESUPP][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_RESUPP];
		patient->artState.probResuppEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_LATEFAIL][1];
		lowerValue=simContext->getARTInputs(artLineNum)->responseTypeValues[SimContext::HET_OUTCOME_LATEFAIL][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_LATEFAIL];
		patient->artState.probLateFail=lowerValue+responseFactor*(upperValue-lowerValue);
	}
	else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
		upperValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesLate[SimContext::HET_OUTCOME_SUPP][1];
		lowerValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesLate[SimContext::HET_OUTCOME_SUPP][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_SUPP];
		patient->artState.probInitialEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesLate[SimContext::HET_OUTCOME_RESUPP][1];
		lowerValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesLate[SimContext::HET_OUTCOME_RESUPP][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_RESUPP];
		patient->artState.probResuppEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesLate[SimContext::HET_OUTCOME_LATEFAIL][1];
		lowerValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesLate[SimContext::HET_OUTCOME_LATEFAIL][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_LATEFAIL];
		patient->artState.probLateFail=lowerValue+responseFactor*(upperValue-lowerValue);
	}
	else{
		upperValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesEarly[SimContext::HET_OUTCOME_SUPP][1];
		lowerValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesEarly[SimContext::HET_OUTCOME_SUPP][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_SUPP];
		patient->artState.probInitialEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesEarly[SimContext::HET_OUTCOME_RESUPP][1];
		lowerValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesEarly[SimContext::HET_OUTCOME_RESUPP][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_RESUPP];
		patient->artState.probResuppEfficacy=lowerValue+responseFactor*(upperValue-lowerValue);

		upperValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesEarly[SimContext::HET_OUTCOME_LATEFAIL][1];
		lowerValue=simContext->getPedsARTInputs(artLineNum)->responseTypeValuesEarly[SimContext::HET_OUTCOME_LATEFAIL][0];
		responseFactor=patient->artState.responseFactorCurrRegimen[SimContext::HET_OUTCOME_LATEFAIL];
		patient->artState.probLateFail=lowerValue+responseFactor*(upperValue-lowerValue);
	}

	if (patient->generalState.isAdolescent){
		patient->artState.propMthCostNonResponders = simContext->getAdolescentARTInputs(artLineNum)->propMthCostNonResponders[getAgeCategoryAdolescentART(artLineNum)];
		patient->artState.probRestartAfterFail = simContext->getAdolescentARTInputs(artLineNum)->probRestartARTRegimenAfterFailure[patient->artState.responseTypeCurrRegimen[SimContext::HET_OUTCOME_RESTART]][getAgeCategoryAdolescentART(artLineNum)];
	}
	else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_ADULT){
		patient->artState.propMthCostNonResponders=simContext->getARTInputs(artLineNum)->propMthCostNonResponders;
		patient->artState.probRestartAfterFail=simContext->getARTInputs(artLineNum)->probRestartARTRegimenAfterFailure[patient->artState.responseTypeCurrRegimen[SimContext::HET_OUTCOME_RESTART]];
	}
	else if (patient->pedsState.ageCategoryPediatrics == SimContext::PEDS_AGE_LATE){
		patient->artState.propMthCostNonResponders=simContext->getPedsARTInputs(artLineNum)->propMthCostNonRespondersLate;
		patient->artState.probRestartAfterFail=simContext->getPedsARTInputs(artLineNum)->probRestartARTRegimenAfterFailureLate[patient->artState.responseTypeCurrRegimen[SimContext::HET_OUTCOME_RESTART]];
	}
	else{
		patient->artState.propMthCostNonResponders=simContext->getPedsARTInputs(artLineNum)->propMthCostNonRespondersEarly;
		patient->artState.probRestartAfterFail=simContext->getPedsARTInputs(artLineNum)->probRestartARTRegimenAfterFailureEarly[patient->artState.responseTypeCurrRegimen[SimContext::HET_OUTCOME_RESTART]];
	}

} /* end setCurrARTResponse */

/** \brief setTargetHVLStrata updates the target HVL while on ART or post ART
 *
 * \param targetHVL a SimContext::HVL_STRATA indicating the target HVL strata
 **/
void StateUpdater::setTargetHVLStrata(SimContext::HVL_STRATA targetHVL) {
	patient->diseaseState.targetHVLStrata = targetHVL;
} /* end setTargetHVLStrata */

/** \brief setPatientNatHistSlopePerc sets the Natural history cd4 decline Perc
 *
 * \param cd4Perc a double representing the CD4 Increment Percent
 **/
void StateUpdater::setPatientNatHistSlopePerc(double cd4Perc){
	patient->diseaseState.patientSpecificCD4DeclinePerc = cd4Perc;
	patient->diseaseState.hasDrawnPatientSpecificCD4Decline = true;
}/* end setPatientNatHistSlopePerc */

/** \brief setCurrRegimenCD4Slope sets the CD4 slope for the current ART regimen
 *
 * \param cd4slope a double representing the new CD4 slope
 **/
void StateUpdater::setCurrRegimenCD4Slope(double cd4Slope) {
	patient->artState.currRegimenCD4Slope = cd4Slope;
} /* end setCurrRegimenCD4Slope */

/** \brief setCurrRegimenCD4PercentageSlope sets the CD4 percentage slope for the current ART regimen
 *
 * This is used for the pediatric model
 *
 * \param cd4PercSlope a double representing the new CD4 percentage slope
 **/
void StateUpdater::setCurrRegimenCD4PercentageSlope(double cd4PercSlope) {
	patient->artState.currRegimenCD4PercentageSlope = cd4PercSlope;
} /* end setCurrRegimenCD4PercentageSlope */

/** \brief setSuccessfulARTRegimen updates the state to indicate that the patient has been successful on the ART regimen and the month of initial success */
void StateUpdater::setSuccessfulARTRegimen(){
	patient->artState.hadSuccessOnRegimen = true;
	patient->artState.monthOfInitialRegimenSuccess = patient->generalState.monthNum;
} /* end setSuccessfulARTRegimen */

/** \brief setCD4EnvelopeRegimen initializes the specified CD4 envelope type
 *
 * \param envelopeType a SimContext::ENVL_CD4_TYPE indicating whether this is a percentage envelope (pediatrics) or individual CD4 count (adult)
 * \param artLineNum an integer indicating which ART regimen this envelope is tied to
 * \param monthOfStart an integer (defaults to SimContext::NOT_APPL) indicating the month the patient started on this ART regimen; used to keep the same start date when the envelope is converted from percentage CD4 to true CD4 in early to late childhood transition
 * 
 **/
void StateUpdater::setCD4EnvelopeRegimen(SimContext::ENVL_CD4_TYPE envelopeType, int artLineNum, int monthOfStart) {
	SimContext::CD4Envelope *envelope;

	if (envelopeType == SimContext::ENVL_CD4_OVERALL) {
		envelope = &(patient->artState.overallCD4Envelope);
		envelope->value = patient->diseaseState.currTrueCD4;
	}
	else if (envelopeType == SimContext::ENVL_CD4_INDIV) {
		envelope = &(patient->artState.indivCD4Envelope);
		envelope->value = patient->diseaseState.currTrueCD4;
	}
	else if (envelopeType == SimContext::ENVL_CD4_PERC_OVERALL) {
		envelope = &(patient->artState.overallCD4PercentageEnvelope);
		envelope->value = patient->diseaseState.currTrueCD4Percentage;
	}
	else {
		envelope = &(patient->artState.indivCD4PercentageEnvelope);
		envelope->value = patient->diseaseState.currTrueCD4Percentage;
	}
	envelope->isActive = true;
	envelope->regimenNum = artLineNum;
	if (monthOfStart == SimContext::NOT_APPL)
		envelope->monthOfStart = patient->generalState.monthNum;
	else
		envelope->monthOfStart = monthOfStart;
} /* end setCD4EnvelopeRegimen */

/** \brief setCD4EnvelopeSlope updates the slope used for the specified CD4 envelope type
 * \param envelopeType a SimContext::ENVL_CD4_TYPE indicating whether this is a percentage envelope (pediatrics) or individual CD4 count (adult)
 * \param cd4slope a double representing the CD4 slope (or CD4 percentage slope) of the envelope
 **/
void StateUpdater::setCD4EnvelopeSlope(SimContext::ENVL_CD4_TYPE envelopeType, double cd4Slope) {
	if (cd4Slope < 0)
		cd4Slope = 0;
	if (envelopeType == SimContext::ENVL_CD4_OVERALL)
		patient->artState.overallCD4Envelope.slope = cd4Slope;
	else if (envelopeType == SimContext::ENVL_CD4_INDIV)
		patient->artState.indivCD4Envelope.slope = cd4Slope;
	else if (envelopeType == SimContext::ENVL_CD4_PERC_OVERALL)
		patient->artState.overallCD4PercentageEnvelope.slope = cd4Slope;
	else
		patient->artState.indivCD4PercentageEnvelope.slope = cd4Slope;
} /* end setCD4EnvelopeSlope */

/** \brief incrementCD4Envelope increments the specified CD4 envelope's level according to hypothetical ART success
 *
 * \param envelopeType a SimContext::ENVL_CD4_TYPE indicating whether this is a percentage envelope (pediatrics) or individual CD4 count (adult)
 * \param changeCD4 a double representing the amount to increase the envelope CD4 (or CD4 percentage) by
 * */
void StateUpdater::incrementCD4Envelope(SimContext::ENVL_CD4_TYPE envelopeType, double changeCD4) {
	if (envelopeType == SimContext::ENVL_CD4_OVERALL)
		patient->artState.overallCD4Envelope.value += changeCD4;
	else if (envelopeType == SimContext::ENVL_CD4_INDIV)
		patient->artState.indivCD4Envelope.value += changeCD4;
	else if (envelopeType == SimContext::ENVL_CD4_PERC_OVERALL)
		patient->artState.overallCD4PercentageEnvelope.value += changeCD4;
	else
		patient->artState.indivCD4PercentageEnvelope.value += changeCD4;
} /* end incrementCD4Envelope */

/** \brief setCurrARTRegimenFailure updates the state to begin the next ART treatment regimen
 *
 * ART statistics for observed failure are updated here.
 *
 * \param failType a SimContext::ART_FAIL_TYPE indicating the type of observed failure */
void StateUpdater::setCurrARTObservedFailure(SimContext::ART_FAIL_TYPE failType) {
	// Update patient state for observed failure of an ART regimen
	patient->artState.hasObservedFailure = true;
	patient->artState.numObservedFailures++;
	patient->artState.typeObservedFailure = failType;
	patient->artState.monthOfObservedFailure = patient->generalState.monthNum;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		patient->artState.numObservedOIsSinceFailOrStopART[i] = 0;
	}

	// Update statistics for observed failure of an ART regimen
	int artLineNum = patient->artState.currRegimenNum;
	runStats->artStats.numObservedFailureCD4Metric[artLineNum][patient->pedsState.ageCategoryCD4Metric]++;
	runStats->artStats.numObservedFailureType[artLineNum][failType]++;
	runStats->artStats.numObservedFailureTypeCD4Metric[artLineNum][failType][patient->pedsState.ageCategoryCD4Metric]++;

	double currTrueCD4Value = 0;
	if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
		currTrueCD4Value = patient->diseaseState.currTrueCD4;
	else
		currTrueCD4Value = patient->diseaseState.currTrueCD4Percentage;
 	runStats->artStats.trueCD4AtObservedFailureSumType[artLineNum][failType][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;
	if (patient->monitoringState.hasObservedCD4){
		runStats->artStats.observedCD4AtObservedFailureSumType[artLineNum][failType] += patient->monitoringState.currObservedCD4;
		runStats->artStats.numObservedCD4atObservedFailureType[artLineNum][failType]++;	
	}		
	if (patient->artState.currRegimenEfficacy == SimContext::ART_EFF_FAILURE)
		runStats->artStats.numObservedFailureAfterTrueType[artLineNum][failType]++;
	int monthsToObserve = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;
	runStats->artStats.monthsToObservedFailureSumType[artLineNum][failType] += monthsToObserve;
	runStats->artStats.monthsToObservedFailureSumSquaresType[artLineNum][failType] += monthsToObserve * monthsToObserve;
} /* setCurrARTObservedFailure() */

/** \brief stopCurrARTRegimen updates the state to begin the next ART treatment regimen
 *
 * ART stopping statistics are updated here
 *
 * \param stopType a SimContext::ART_STOP_TYPE indicating the reason the current ART regimen is being stopped */
void StateUpdater::stopCurrARTRegimen(SimContext::ART_STOP_TYPE stopType) {
	int artLineNum = patient->artState.currRegimenNum;
	int monthsToStop = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;

	// Update patient state for stopping an ART regimen
	patient->artState.isOnART = false;
	patient->artState.applyARTEffect = false;
	patient->artState.typeCurrStop = stopType;
	patient->artState.prevRegimenNum = patient->artState.currRegimenNum;
	patient->artState.prevRegimenEfficacy = patient->artState.currRegimenEfficacy;
	patient->artState.monthOfPrevRegimenStop = patient->generalState.monthNum;
	if (!patient->artState.hasObservedFailure) {
		for (int i = 0; i < SimContext::OI_NUM; i++) {
			patient->artState.numObservedOIsSinceFailOrStopART[i] = 0;
		}
	}

	// Update statistics for stopping an ART regimen, don't update for STI interrupts
	if (patient->artState.currSTIState == SimContext::STI_STATE_INTERRUPT)
		return;
	runStats->artStats.numStopType[artLineNum][stopType]++;
	runStats->artStats.numStopTypeCD4Metric[artLineNum][stopType][patient->pedsState.ageCategoryCD4Metric]++;
	double currTrueCD4Value = 0;
	if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
		currTrueCD4Value = patient->diseaseState.currTrueCD4;
	else
		currTrueCD4Value = patient->diseaseState.currTrueCD4Percentage;
	runStats->artStats.trueCD4AtStopSumType[artLineNum][stopType][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;
	if (patient->monitoringState.hasObservedCD4){
		runStats->artStats.observedCD4AtStopSumType[artLineNum][stopType] += patient->monitoringState.currObservedCD4;
		runStats->artStats.numWithObservedCD4StopType[artLineNum][stopType]++;
	}		
	if (patient->artState.prevRegimenEfficacy == SimContext::ART_EFF_FAILURE){
		runStats->artStats.numStopAfterTrueFailureType[artLineNum][stopType]++;
	}	
	runStats->artStats.monthsToStopSumType[artLineNum][stopType] += monthsToStop;
	runStats->artStats.monthsToStopSumSquaresType[artLineNum][stopType] += monthsToStop * monthsToStop;

} /* stopCurrARTRegimen */

/** \brief setNextARTRegimen updates the next ART regimen that is available for use
 *
 * \param hasNext a bool that is true if there is a "next ART regimen" available
 * \param artLineNum an integer indicating the ART regimen number of the next regimen -- this is only set if hasNext is true
 * \param isResupp a bool indicating if next line will be resuppression regimen
 * */
void StateUpdater::setNextARTRegimen(bool hasNext, int artLineNum, bool isResupp) {
	patient->artState.hasNextRegimenAvailable = hasNext;
	if (hasNext){
		patient->artState.nextRegimenNum = artLineNum;
		patient->artState.nextRegimenIsResupp = isResupp;
	}

} /* end setNextARTRegimen */

/** \brief incrementMonthsUnsuccessfulART increments the number of months on failed ART stratified by HVL */
void StateUpdater::incrementMonthsUnsuccessfulART() {
	patient->artState.numMonthsOnUnsuccessfulByRegimen[patient->artState.currRegimenNum]++;
	SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
	patient->artState.numMonthsOnUnsuccessfulByHVL[hvlStrata]++;
} /* end incrementMonthsUnsuccessfulART */

/** \brief addARTToxicityEffect adds the occurrence of a new toxicity to the active effects list
 *
 * \param severity a SimContext::ART_TOX_SEVERITY indicating the severity of the new toxicity
 * \param toxNum an integer representing the index of the toxicity
 * \param timeToTox an integer represent the number of months until the toxicity takes effect
 **/
void StateUpdater::addARTToxicityEffect(SimContext::ART_TOX_SEVERITY severity, int toxNum, int timeToTox) {
	SimContext::ARTToxicityEffect toxicity;
	toxicity.toxSeverityType = severity;
	toxicity.toxNum = toxNum;
	toxicity.monthOfToxStart = patient->generalState.monthNum + timeToTox;
	toxicity.ARTRegimenNum = patient->artState.currRegimenNum;
	toxicity.ARTSubRegimenNum = patient->artState.currSubRegimenNum;
	patient->artState.activeToxicityEffects.push_back(toxicity);
} /* end addARTToxicityEffect */

/** \brief removeARTToxicityEffect removes the specified toxicity from the active effects list
 *
 * \param toxIter a pointer to an iterator pointing to the toxicity that should be removed
 *
 **/
void StateUpdater::removeARTToxicityEffect(list<SimContext::ARTToxicityEffect>::const_iterator &toxIter) {
	// Slightly hackish way to get a regular iterator from a const iterator, this is necessary
	//	since erase requires a regular iterator
	list<SimContext::ARTToxicityEffect>::const_iterator beginIter = patient->artState.activeToxicityEffects.begin();
	list<SimContext::ARTToxicityEffect>::iterator eraseIter = patient->artState.activeToxicityEffects.begin();
	advance(eraseIter, distance(beginIter, toxIter));

	// Erase the appropriate element from the toxicity list
	patient->artState.activeToxicityEffects.erase(eraseIter);
} /* end removeARTToxicityEffect */

/** \brief setARTToxicity updates the patient state and stats for the occurrence of a toxicity
 *
 * \param toxEffect a pointer to a SimConstant::ARTToxicityEffect indicating the toxicity effect to be set
 *
 * Statistics for ART toxicity are updated here
 **/
void StateUpdater::setARTToxicity(const SimContext::ARTToxicityEffect &toxEffect) {
	// Update the patient state as a severe toxicity if it causes a subregimen switch
	const SimContext::ARTInputs::ARTToxicity &toxInputs = simContext->getARTInputs(toxEffect.ARTRegimenNum)->toxicity[toxEffect.ARTSubRegimenNum][toxEffect.toxSeverityType][toxEffect.toxNum];
	bool isSevere = false;
	if (toxEffect.toxSeverityType == SimContext::ART_TOX_MAJOR) {
		patient->artState.hasMajorToxicity = true;
		if(patient->pedsState.ageCategoryPediatrics>=SimContext::PEDS_AGE_LATE){
			if (simContext->getTreatmentInputs()->stopART[toxEffect.ARTRegimenNum].withMajorToxicty){
				isSevere = true;
			}
		}
		else{
			if (simContext->getPedsInputs()->stopART[toxEffect.ARTRegimenNum].withMajorToxicty){
				isSevere = true;
			}
		}
	}
	if (toxInputs.switchSubRegimenOnToxicity != SimContext::NOT_APPL)
		isSevere = true;
	if (isSevere) {
		if (!patient->artState.hasSevereToxicity) {
			patient->artState.hasSevereToxicity = true;
			patient->artState.severeToxicityEffect = &toxEffect;
		}
		else {
			const SimContext::ARTToxicityEffect *severeToxEffect = patient->artState.severeToxicityEffect;
			const SimContext::ARTInputs::ARTToxicity &severeToxInputs = simContext->getARTInputs(severeToxEffect->ARTRegimenNum)->toxicity[severeToxEffect->ARTSubRegimenNum][severeToxEffect->toxSeverityType][severeToxEffect->toxNum];
			if ((toxEffect.toxSeverityType > severeToxEffect->toxSeverityType) ||
				(toxInputs.chronicToxDeathRateRatio > severeToxInputs.chronicToxDeathRateRatio) ||
				(toxInputs.acuteMajorToxDeathRateRatio > severeToxInputs.acuteMajorToxDeathRateRatio) ||
				(toxInputs.QOLModifier < severeToxInputs.QOLModifier)) {
					patient->artState.severeToxicityEffect = &toxEffect;
			}
		}
	}
	patient->artState.hadPrevToxicity = true;

	if (toxEffect.toxSeverityType == SimContext::ART_TOX_CHRONIC && toxInputs.switchARTRegimenOnToxicity != SimContext::NOT_APPL){
        patient->artState.hasChronicToxSwitch = true;
        patient->artState.chronicToxSwitchToLine = toxInputs.switchARTRegimenOnToxicity;
	}

	// Update the statistics for the ART toxicity
	int currRegimen = toxEffect.ARTRegimenNum;
	SimContext::ART_TOX_SEVERITY severity = toxEffect.toxSeverityType;
	SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
	runStats->artStats.numToxicityCases[currRegimen][severity][hvlStrata]++;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Toxicity Cases
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numToxicityCases++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numToxicityCases++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numToxicityCases++;
		}
	}
} /* end setARTToxicity */

/** \brief incrementARTFailedCD4Tests increments the number of failed CD4 tests counting towards ART failure */
void StateUpdater::incrementARTFailedCD4Tests() {
	patient->artState.numFailedCD4Tests++;
} /* end incrementARTFailedCD4Tests */

/** \brief resetARTFailedCD4Tests sets the number of failed CD4 tests counting towards ART failure back to 0 */
void StateUpdater::resetARTFailedCD4Tests() {
	patient->artState.numFailedCD4Tests = 0;
} /* end resetARTFailedCD4Tests */

/** \brief incrementARTFailedHVLTests increments the number of failed CD4 tests counting towards ART failure */
void StateUpdater::incrementARTFailedHVLTests() {
	patient->artState.numFailedHVLTests++;
} /* end incrementARTFailedHVLTests */

/** \brief resetARTFailedHVLTests sets the number of failed HVL tests counting towards ART failure back to 0 */
void StateUpdater::resetARTFailedHVLTests() {
	patient->artState.numFailedHVLTests = 0;
} /* end resetARTFailedHVLTests */

/** \brief incrementARTFailedOIs increments the number of observed OIs counting towards ART failure */
void StateUpdater::incrementARTFailedOIs() {
	patient->artState.numFailedOIs++;
} /* end incrementARTFailedOIs */

/** \brief resetARTFailedOIs resets the number of observed OIs counting towards ART failure */
void StateUpdater::resetARTFailedOIs() {
	patient->artState.numFailedOIs = 0;
} /* end resetARTFailedOIs */

/** \brief setCurrSTIState updates the STI state for the current ART regimen
 * \param newSTIState a SimContext::STI_STATE to transition the patient to
 *
 * Statistics related to STI are updated here
 **/
void StateUpdater::setCurrSTIState(SimContext::STI_STATE newSTIState) {
	SimContext::STI_STATE prevSTIState = patient->artState.currSTIState;
	int prevMonthOfChange = patient->artState.monthOfSTIStateChange;
	patient->artState.currSTIState = newSTIState;
	patient->artState.monthOfSTIStateChange = patient->generalState.monthNum;
	if (newSTIState == SimContext::STI_STATE_INTERRUPT) {
		patient->artState.numSTIInterruptionsOnCurrRegimen++;
		if (prevSTIState == SimContext::STI_STATE_NONE)
			patient->artState.monthOfSTIInitialStop = patient->generalState.monthNum;
	}
	else if (newSTIState == SimContext::STI_STATE_NONE) {
		patient->artState.numSTIInterruptionsOnCurrRegimen = 0;
	}

	int cycle = patient->artState.numSTIInterruptionsOnCurrRegimen - 1;
	if (cycle >= SimContext::STI_NUM_TRACKED)
		cycle = SimContext::STI_NUM_TRACKED - 1;
	if (newSTIState == SimContext::STI_STATE_INTERRUPT) {
		runStats->artStats.numSTIInterruptions[patient->artState.currRegimenNum][cycle]++;
		runStats->artStats.numPatientsWithSTIInterruptions[patient->artState.currRegimenNum][cycle]++;
	}
	else if (newSTIState == SimContext::STI_STATE_RESTART) {
		runStats->artStats.numSTIRestarts[patient->artState.nextRegimenNum][cycle]++;
		runStats->artStats.monthsOnSTIInterruptionSum[patient->artState.nextRegimenNum] += patient->generalState.monthNum - prevMonthOfChange;
	}
	else if (newSTIState == SimContext::STI_STATE_ENDPOINT) {
		runStats->artStats.numSTIEndpoints[patient->artState.currRegimenNum][cycle]++;
	}
} /* end setCurrSTIState */

/** \brief setProphNonCompliance sets whether or not the patient complies with proph
 *
 * \param isNonCompliant a boolean that is true if the patient is compliant
 **/
void StateUpdater::setProphNonCompliance(bool isNonCompliant) {
	patient->prophState.isNonCompliant = isNonCompliant;
} /* setProphNonCompliance */

/** \brief startNextProph updates state to beginning using next proph
 *
 * \param oiType a SimContext::OI_TYPE representing the OI which should have its prophylaxis updated
 *
 * It is assumed that a "next proph" exists for oiType
 **/
void StateUpdater::startNextProph(SimContext::OI_TYPE oiType) {

	SimContext::PROPH_TYPE prophType = patient->prophState.nextProphType[oiType];
	int prophNum = patient->prophState.nextProphNum[oiType];

	// Proph is available, update state to begin taking the proph
	patient->prophState.isOnProph[oiType] = true;
	patient->prophState.hasTakenProph[oiType][prophType] = true;
	patient->prophState.currProphType[oiType] = prophType;
	patient->prophState.currProphNum[oiType] = prophNum;
	patient->prophState.monthOfProphStart[oiType] = patient->generalState.monthNum;
	patient->prophState.typeProphToxicity[oiType] = SimContext::PROPH_TOX_NONE;
	patient->prophState.useProphResistance[oiType] = false;
	patient->prophState.currTotalNumProphsOn++;
	// Update statistics for starting the proph
	if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE){
		runStats->prophStats.trueCD4InitProphSum[prophType][oiType][prophNum] += patient->diseaseState.currTrueCD4;
		if (patient->monitoringState.hasObservedCD4){
			runStats->prophStats.observedCD4InitProphSum[prophType][oiType][prophNum] += patient->monitoringState.currObservedCD4;
			runStats->prophStats.numTimesInitProphWithObservedCD4[prophType][oiType][prophNum]++;
		}	
	}		
	runStats->prophStats.numTimesInitProph[prophType][oiType][prophNum]++;
	runStats->prophStats.numTimesInitProphCD4Metric[prophType][oiType][prophNum][patient->pedsState.ageCategoryCD4Metric]++;
} /* end startNextProph */

/** \brief stopCurrProph updates state to stop using current proph
 *
 * \param oiType a SimContext::OI_TYPE indicating which prophylaxis to stop
 **/
void StateUpdater::stopCurrProph(SimContext::OI_TYPE oiType) {
	patient->prophState.isOnProph[oiType] = false;
	patient->prophState.currTotalNumProphsOn--;
} /* end stopCurrProph */

/** \brief setNextProph updates whether primary or secondary proph should be used for the OI
 *
 * \param hasNext a boolean that is true if there is a "next prophylaxis" to set; if hasNext is false, no "next Prophylaxis" is set
 * \param prophType a SimContext::PROPH_TYPE indicating the type of prophylaxis the "next proph" should be
 * \param oiType a SimContext::OI_TYPE indicating which OI the "next proph" should be set for
 * \param prophNum an integer indicating which prophylaxis should be next
 **/
void StateUpdater::setNextProph(bool hasNext, SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType, int prophNum) {
	patient->prophState.hasNextProphAvailable[oiType] = hasNext;

	if (hasNext) {
		patient->prophState.nextProphType[oiType] = prophType;
		patient->prophState.nextProphNum[oiType] = prophNum;
	}
} /* end setNextProph */

/** \brief setProphToxicity records the toxicity and updates stats
 *
 * \param isMajor a boolean that is true if it a SimContext::PROPH_TOX_MAJOR (major toxicity)
 * \param oiType a SimContext::OI_TYPE indicating which prophylaxis has the toxicity
 *
 * Statistics for toxicity are updated here
 **/
void StateUpdater::setProphToxicity(bool isMajor, SimContext::OI_TYPE oiType) {
	// Update the patient state for the toxicity
	int prophNum = patient->prophState.currProphNum[oiType];

	if (isMajor)
		patient->prophState.typeProphToxicity[oiType] = SimContext::PROPH_TOX_MAJOR;
	else
		patient->prophState.typeProphToxicity[oiType] = SimContext::PROPH_TOX_MINOR;

	// Update the statistics for the toxicity occurrence
	if (isMajor)
		runStats->prophStats.numMajorToxicity[oiType][prophNum]++;
	else
		runStats->prophStats.numMinorToxicity[oiType][prophNum]++;
} /* end setProphToxicity */

/** \brief setProphResistance updates the flag to indicate that proph resistance has occurred
 *
 * \param oiType a SimContext::OI_TYPE indicating which prophylaxis resistance should be updated
 *
 * This only sets useProphResistance to true for the assigned prophylaxis
 **/
void StateUpdater::setProphResistance(SimContext::OI_TYPE oiType) {
	patient->prophState.useProphResistance[oiType] = true;
}

/** \brief setTBDiseaseState updates the TB disease state
 * \param newTBState a SimContext::TB_STATE marking the new TB state
 * \param isInfection is a bool that is true if this is a new infection
 * \param infectType a SimContext::TB_INFECT infection type - initial, prevalent, reinfection, reactivation, or relapse. "Initial" is used both with initial infections that occur after model start and with state transitions after model start where isInfection is false
 * \param previousState a SimContext::TB_STATE marking the previous TB state
 * The month number of the change in state is also updated
 **/
void StateUpdater::setTBDiseaseState(SimContext::TB_STATE newTBState, bool isInfection,  SimContext::TB_INFECT infectType, SimContext::TB_STATE previousState) {
	patient->tbState.currTrueTBDiseaseState = newTBState;
	patient->tbState.monthOfTBStateChange = patient->generalState.monthNum;
	SimContext::TB_STRAIN tbStrain;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();

	if(infectType == SimContext::TB_INFECT_PREVALENT){
		if (newTBState == SimContext::TB_STATE_PREV_TREATED || newTBState == SimContext::TB_STATE_TREAT_DEFAULT){
			patient->tbState.observedHistActiveTBAtEntry = true;
		}
	}
	if (newTBState != SimContext::TB_STATE_UNINFECTED)
		tbStrain = patient->tbState.currTrueTBResistanceStrain;

	if (isInfection){
		patient->tbState.hasTrueHistoryTB = true; 	
		// Update patient state for month of most recent infection
		patient->tbState.monthOfTBInfection = patient->generalState.monthNum;
		if (infectType != SimContext::TB_INFECT_PREVALENT){
			runStats->tbStats.numInfections[previousState][tbStrain]++;

			if (currTime)
				currTime->numTBInfections[previousState][tbStrain]++;
		}
		//Update coststats variables
		for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
			//Tb Events
			if (patient->generalState.costSubgroups[i]){
				costStats->eventStats[SimContext::COST_CD4_ALL][i].numTBInfections++;

				if (patient->monitoringState.hasObservedCD4)
					costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numTBInfections++;
				else
					costStats->eventStats[SimContext::COST_CD4_NONE][i].numTBInfections++;
			}
		}
	}

	if (infectType == SimContext::TB_INFECT_REACTIVATE){
		runStats->tbStats.numReactivationsLatent[tbStrain]++;
		if (newTBState == SimContext::TB_STATE_ACTIVE_PULM)
			runStats->tbStats.numReactivationsPulmLatent[tbStrain]++;
		if (newTBState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
			runStats->tbStats.numReactivationsExtraPulmLatent[tbStrain]++;

		if (currTime){
			currTime->numTBReactivationsLatent[tbStrain]++;
			if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG){
				if (newTBState == SimContext::TB_STATE_ACTIVE_PULM)
					currTime->numTBReactivationsPulmLatentHIVNegative[tbStrain]++;
				if (newTBState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
					currTime->numTBReactivationsExtraPulmLatentHIVNegative[tbStrain]++;
			}
			else{
				SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
				if (newTBState == SimContext::TB_STATE_ACTIVE_PULM)
					currTime->numTBReactivationsPulmLatentHIVPositive[cd4Strata][tbStrain]++;
				if (newTBState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
					currTime->numTBReactivationsExtraPulmLatentHIVPositive[cd4Strata][tbStrain]++;
			}
		}
	}

	if (infectType == SimContext::TB_INFECT_RELAPSE){
		runStats->tbStats.numRelapses[tbStrain]++;
		if (newTBState == SimContext::TB_STATE_ACTIVE_PULM)
			runStats->tbStats.numRelapsesPulm[tbStrain]++;
		if (newTBState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
			runStats->tbStats.numRelapsesExtraPulm[tbStrain]++;

		if (currTime){
			currTime->numTBRelapses[tbStrain]++;
			if (newTBState == SimContext::TB_STATE_ACTIVE_PULM)
				currTime->numTBRelapsesPulm[tbStrain]++;
			if (newTBState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
				currTime->numTBRelapsesExtraPulm[tbStrain]++;
		}
	}
} /* end setTBDiseaseState */

/** \brief setTBUnfavorableOutcome sets unfavorable outcome flag for tb
 * \param outcome a SimContext::TB_UNFAVORABLE outcome that the patient is being flagged as having
 *
 **/
void StateUpdater::setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE outcome){
	patient->tbState.hasUnfavorableOutcome[outcome] = true;
} /* end setTBUnfavorableOutcome */


/** \brief setTBTracker updates the tracker variable for the three tb trackers
 * \param tracker a SimContext::TB_TRACKER for which tracker we are updating
 * \param trackerStatus a boolean for the status of the tracker
 *
 **/
/** updates the tracker variable for the three tb trackers */
void StateUpdater::setTBTracker(SimContext::TB_TRACKER tracker, bool trackerStatus){
	patient->tbState.currTrueTBTracker[tracker] = trackerStatus;
} /* end setTBTracker */

/** \brief setTBResistanceStrain updates the TB disease drug resistance
 * \param newTBStrain a SimContext::TB_STRAIN that the patient will be set to be resistant to -- any previous resistance is overridden by this state
 **/
void StateUpdater::setTBResistanceStrain(SimContext::TB_STRAIN newTBStrain) {
	patient->tbState.currTrueTBResistanceStrain = newTBStrain;
} /* end setTBResistanceStrain */


/** \brief setObservedTBResistanceStrain updates the observed TB disease drug resistance strain 
 * \param hasObserved a bool representing if the patient has an observed TB Strain
 * \param obsvStrain a SimContext::TB_STRAIN defaulting to SimContext::TB_STRAIN_DS that will be the observed strain
 **/
void StateUpdater::setObservedTBResistanceStrain(bool hasObserved, SimContext::TB_STRAIN obsvStrain){
	patient->tbState.hasObservedTBResistanceStrain = hasObserved;
	if (hasObserved){
		patient->tbState.currObservedTBResistanceStrain = obsvStrain;
		setObservedHistTBResistanceStrain(true, obsvStrain);
	}
} /* end setObservedTBResistanceStrain */

/** \brief setObservedHistTBResistanceStrain updates the observed History of a TB disease drug resistance strain
 * \param hasHist a bool representing if the patient has a history of an observed TB Strain
 * \param obsvStrain a SimContext::TB_STRAIN defaulting to SimContext::TB_STRAIN_DS that the patient has an observed history of
 **/
void StateUpdater::setObservedHistTBResistanceStrain(bool hasHist, SimContext::TB_STRAIN obsvStrain){
	patient->tbState.hasHistObservedTBResistanceStrain = hasHist;
	if (hasHist)
		patient->tbState.observedHistTBResistanceStrain = obsvStrain;
} /* end setObservedHistTBResistanceStrain */

/** \brief setTBCareState updates the TB care state
 * \param newTBCareState a SimContext::TB_CARE that the patient will be set to
 **/
void StateUpdater::setTBCareState(SimContext::TB_CARE newTBCareState){
	patient->tbState.careState = newTBCareState;
}/* end setTBCareState */

/** \brief  setTBTestResult sets the test result for a TB test
 * \param result a SimContext::TB_DIAG_STATUS result of the test
 * \param testIndex is the index of the test
 **/
void StateUpdater::setTBTestResult(SimContext::TB_DIAG_STATUS result, int testIndex){
	patient->tbState.testResults[testIndex] = result;
}/* end setTBTestResult */

/** \brief startTBTesting sets the index of the current test to the beginning, identifies the next test (if any), and initalizes all test results and pickup times to none
 **/
void StateUpdater::startTBTesting(){
	//Set curr index and next index
	int currIndex = 0;
	int nextIndex = SimContext::NOT_APPL;
	setCurrTBTestIndex(currIndex);

	//Set next test num if applicable
	if (currIndex+1 < SimContext::TB_DIAG_TEST_ORDER_NUM &&
			simContext->getTBInputs()->TBDiagnosticsTestOrder[patient->tbState.everOnTreatmentOrEmpiric][currIndex+1] != SimContext::NOT_APPL)
		nextIndex = currIndex+1;
	setNextTBTestIndex(nextIndex, true, patient->tbState.everOnTreatmentOrEmpiric);

	/** Initialize all test results to none */
	for (int i = 0; i < SimContext::TB_DIAG_TEST_ORDER_NUM; i++)
		setTBTestResult(SimContext::TB_DIAG_STATUS_NEG, i);
	setTBTestResultPickup(false, -1, false, SimContext::TB_DIAG_STATUS_NEG, -1, false, false);

}/* end startTBTesting */

/** \brief resetTBTesting sets the index of the current test to N/A, and the next test to the first test (or to N/A if none is available) and removes results 
 * \param removeObsvStrain a bool defaulting to true for whether to remove the observed TB resistance strain
 * \param removePendingDSTResult a bool defaulting to true for whether to remove the pending DST result and unschedule its pickup
 * \param isInit a bool indicating whether this is called at initiation 
 **/
void StateUpdater::resetTBTesting(bool removeObsvStrain, bool removePendingDSTResult, bool isInit){
	//Set curr index and next index
	setCurrTBTestIndex(SimContext::NOT_APPL);

	if (simContext->getTBInputs()->TBDiagnosticsTestOrder[patient->tbState.everOnTreatmentOrEmpiric][0]!=SimContext::NOT_APPL)
		setNextTBTestIndex(0, true, patient->tbState.everOnTreatmentOrEmpiric);
	else
		setNextTBTestIndex(SimContext::NOT_APPL, false);

	/** Initialize all test results to none */
	for (int i = 0; i < SimContext::TB_DIAG_TEST_ORDER_NUM; i++)
		setTBTestResult(SimContext::TB_DIAG_STATUS_NEG, i);

	setTBTestResultPickup(false, -1,  false, SimContext::TB_DIAG_STATUS_NEG, -1, false, false);
	if (removePendingDSTResult)
		setTBDSTTestResultPickup(false, SimContext::TB_STRAIN_DS, -1);

	//Remove obsv Tb strain
	if (removeObsvStrain)
		setObservedTBResistanceStrain(false);

	//Initialize month of TB positive diagnosis if called as part of initiation, otherwise do the usual trace for calling function during the run
	if(isInit){
		patient->tbState.monthOfTBPosDiagnosis = SimContext::NOT_APPL;
		// For now, using -2 for those who start in Treat Default state - i.e., the minimum number of months ago they could have been diagnosed - to avoid needing another state variable just for one proph policy 
		if(patient->tbState.observedHistActiveTBAtEntry && patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_TREAT_DEFAULT){
			patient->tbState.monthOfTBPosDiagnosis = -2;
		}
	}
	else if(patient->generalState.tracingEnabled){
		tracer->printTrace(1, "**%d TB TESTING RESET, Patient must meet TB testing initiation criteria to resume testing \n", patient->generalState.monthNum);	
	}	
}/* end resetTBTesting */


/** \brief setCurrTBTestIndex sets the index of the current TB test in the chain the patient is on.
 * \param index an integer representing the current test.  -1 means none
 **/
void StateUpdater::setCurrTBTestIndex(int index){
	patient->tbState.currTestIndex = index;
}/* end setCurrTBTestIndex */

/** \brief setNextTBTestIndex sets the index of the next TB test in the chain the patient is on.
 * \param index an integer representing the next test.  -1 means none
 * \param isStartChain if this is for the first test in the chain
 * \param patientEverTreatedCategory a bool for the patient category at the start of the testing chain
 **/
void StateUpdater::setNextTBTestIndex(int index, bool isStartChain, bool patientEverTreatedCategory){
	patient->tbState.nextTestIndex = index;
	if (isStartChain)
		patient->tbState.everOnTreatmentOrEmpiricStartChain = patientEverTreatedCategory;
}/* end setNextTBTestIndex */


/** \brief performTBTest performs a TB test
 * \param testNum an integer representing the number of the test to perform
 * \param performDST a bool that determines if DST is also done
 **/
bool StateUpdater::performTBTest(int testNum, bool performDST){
	bool continueLoop = true;

	SimContext::TBInputs::TBTest tbTest = simContext->getTBInputs()->TBTests[testNum];

	//Roll for starting empiric therapy upon test offer
	if (!patient->tbState.isOnEmpiricTreatment && !patient->tbState.hadTreatmentMajorTox){
		double randNum = CepacUtil::getRandomDouble(100100, patient);
		double probEmpiric = 0;
		if (!patient->monitoringState.isDetectedHIVPositive){
			if(patient->tbState.currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
				probEmpiric = tbTest.probEmpiricTestOfferSymptomaticHIVNeg;
			else
				probEmpiric = tbTest.probEmpiricTestOfferAsymptomaticHIVNeg;
		}
		//Undetected HIV uses same probabilities as HIV-negative but true CD4 is still used so that the decisions will not rely solely on CD4 tests
		else{
			SimContext::CD4_STRATA currCD4Strata = patient->diseaseState.currTrueCD4Strata;
			if(patient->tbState.currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
				probEmpiric = tbTest.probEmpiricTestOfferSymptomaticHIVPos[currCD4Strata];
			else
				probEmpiric = tbTest.probEmpiricTestOfferAsymptomaticHIVPos[currCD4Strata];
		}
		if (randNum < probEmpiric){
			//Roll for which line to start
			SimContext::TB_STRAIN empiricStrain = SimContext::TB_STRAIN_DS;
			if (patient->tbState.hasHistObservedTBResistanceStrain){
				if (patient->tbState.observedHistTBResistanceStrain == SimContext::TB_STRAIN_MDR){
					randNum = CepacUtil::getRandomDouble(100140, patient);
					if (randNum < simContext->getTBInputs()->probEmpiricWithObservedHistMDR)
						empiricStrain = SimContext::TB_STRAIN_MDR;
				}
				else if(patient->tbState.observedHistTBResistanceStrain == SimContext::TB_STRAIN_XDR){
					randNum = CepacUtil::getRandomDouble(100140, patient);
					if (randNum < simContext->getTBInputs()->probEmpiricWithObservedHistXDR)
						empiricStrain = SimContext::TB_STRAIN_XDR;
				}
			}

			startEmpiricTBTreatment(simContext->getTBInputs()->empiricTreatmentNum[empiricStrain]);
		}
	}

	//Roll for prob of test accept
	double randNum = CepacUtil::getRandomDouble(100130, patient);
	if (randNum < tbTest.probAccept){

		//increment costs and qol for test
		incrementCostsTBTest(tbTest.initCost, 0.0, testNum);
		accumulateQOLModifier(tbTest.QOLMod);

		// Output tracing if enabled
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB TEST %d ACCEPTED, %1.2lf QAred, $ %1.0lf;\n",
				patient->generalState.monthNum,
				testNum,
				patient->generalState.QOLValue,
				patient->generalState.costsDiscounted);
		}

		//Roll for starting empiric therapy while waiting for results
		if (!patient->tbState.isOnEmpiricTreatment && !patient->tbState.hadTreatmentMajorTox){
			randNum = CepacUtil::getRandomDouble(100140, patient);
			double probEmpiric = 0;
			if (!patient->monitoringState.isDetectedHIVPositive){
				if(patient->tbState.currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
					probEmpiric = tbTest.probEmpiricAwaitingResultsSymptomaticHIVNeg;
				else
					probEmpiric = tbTest.probEmpiricAwaitingResultsAsymptomaticHIVNeg;
			}
			//HIV itself must be observed but true CD4 is used here so that the decisions will not rely solely on CD4 tests
			else{
				SimContext::CD4_STRATA currCD4Strata = patient->diseaseState.currTrueCD4Strata;
				if(patient->tbState.currTrueTBTracker[SimContext::TB_TRACKER_SYMPTOMS])
					probEmpiric = tbTest.probEmpiricAwaitingResultsSymptomaticHIVPos[currCD4Strata];
				else
					probEmpiric = tbTest.probEmpiricAwaitingResultsAsymptomaticHIVPos[currCD4Strata];
			}
			if (randNum < probEmpiric){
				//Roll for which line to start
				SimContext::TB_STRAIN empiricStrain = SimContext::TB_STRAIN_DS;
				if (patient->tbState.hasHistObservedTBResistanceStrain){
					if (patient->tbState.observedHistTBResistanceStrain == SimContext::TB_STRAIN_MDR){
						randNum = CepacUtil::getRandomDouble(100140, patient);
						if (randNum < simContext->getTBInputs()->probEmpiricWithObservedHistMDR)
							empiricStrain = SimContext::TB_STRAIN_MDR;
					}
					else if(patient->tbState.observedHistTBResistanceStrain == SimContext::TB_STRAIN_XDR){
						randNum = CepacUtil::getRandomDouble(100140, patient);
						if (randNum < simContext->getTBInputs()->probEmpiricWithObservedHistXDR)
							empiricStrain = SimContext::TB_STRAIN_XDR;
					}
				}

				startEmpiricTBTreatment(simContext->getTBInputs()->empiricTreatmentNum[empiricStrain]);
			}
		}

		//Roll for time of result pickup (or lack thereof)
		int monthsToPickup = (int)(CepacUtil::getRandomGaussian(tbTest.timeToPickupMean, tbTest.timeToPickupStdDev, 100160, patient)+0.5);
		if (monthsToPickup < 0)
			monthsToPickup = 0;

		bool resetTests = false;
		if (monthsToPickup > tbTest.monthsToReset)
			resetTests = true;

		// roll for result
		double probPositive;
		SimContext::TB_STATE tbState = patient->tbState.currTrueTBDiseaseState;

		if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
			probPositive = tbTest.probPositiveHIVNeg[tbState];
		else
			probPositive = tbTest.probPositiveHIVPos[tbState][patient->diseaseState.currTrueCD4Strata];

		randNum = CepacUtil::getRandomDouble(100150, patient);
		SimContext::TB_DIAG_STATUS result;
		if (randNum < probPositive){
			result = SimContext::TB_DIAG_STATUS_POS;
		}
		//DST is only performed if TB is detected 
		else{
			result = SimContext::TB_DIAG_STATUS_NEG;
			performDST = false;
		}	

		// Update the longitudinal statistics for the result of the test
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if (currTime) {
			currTime->numTBTestResults[testNum][tbState][result]++;
		}

		bool willPickup = false;

		//Roll for result pickup
		randNum = CepacUtil::getRandomDouble(100140, patient);
		if (randNum < tbTest.probPickup){
			//will pickup
			willPickup = true;
			setTBTestResultPickup(true, testNum, true, result, patient->generalState.monthNum + monthsToPickup, resetTests, performDST);

			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d TB TEST %d, pickup in month:%d, result:%s;\n", patient->generalState.monthNum,
						testNum,
						patient->generalState.monthNum + monthsToPickup,
						SimContext::TB_DIAG_STATUS_STRS[result]);


		}//end will pickup test
		else{
			setTBTestResultPickup(true, testNum, false, SimContext::TB_DIAG_STATUS_NEG, patient->generalState.monthNum + monthsToPickup, resetTests, performDST);

			//will not pickup
			if (patient->generalState.tracingEnabled)
				tracer->printTrace(1, "**%d TB TEST %d, will not pick up, fail to pickup in month:%d;\n", patient->generalState.monthNum,
						testNum, patient->generalState.monthNum + monthsToPickup);
		}

		//Roll for DST if enabled
		//Only do DST if we have no pending DST results
		//Only do DST here if it is linked to the test (meaning it is performed automatically on the same sample if TB was detected)
		if (performDST && tbTest.DSTLinked && !patient->tbState.hasPendingDSTResult){
			//Add costs DST test
			incrementCostsTBTest(0.0, tbTest.DSTCost, testNum);

			if (willPickup){
				//Will pick up DST if they will pick up the linked test
				SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS;
				if (patient->tbState.currTrueTBDiseaseState != SimContext::TB_STATE_UNINFECTED){
					/** Draw the observed TB Strain */
					SimContext::TB_STRAIN trueStrain = patient->tbState.currTrueTBResistanceStrain;
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
				int monthOfPickup = patient->generalState.monthNum + tbTest.DSTMonthsToResult[obsvStrain];
				setTBDSTTestResultPickup(true, obsvStrain, monthOfPickup);
				if (patient->generalState.tracingEnabled)
					tracer->printTrace(1, "**%d TB DST TEST %d, pickup in month:%d, result:%s;\n", patient->generalState.monthNum,
							testNum,
							monthOfPickup,
							SimContext::TB_STRAIN_STRS[obsvStrain]);

			}
			else{
				if (patient->generalState.tracingEnabled)
					tracer->printTrace(1, "**%d TB DST TEST %d, will not pick up;\n", patient->generalState.monthNum,
							testNum);
			}
		}

	}
	else{
		//Did not accept
		if (patient->generalState.tracingEnabled)
			tracer->printTrace(1, "**%d TB TEST %d, not accepted\n", patient->generalState.monthNum, testNum);

		//check if should interpret partial result
		if (simContext->getTBInputs()->TBDiagnosticAllowIncomplete && patient->tbState.currTestIndex > 0){
			performTBDiagnosis(patient->tbState.currTestIndex -1);
			continueLoop = false;
		}

		//Reset tests if allow no diagnosis - includes clearing pending DST since they are not completing the full testing algorithm
		if(simContext->getTBInputs()->TBDiagnosticAllowNoDiagnosis){
			resetTBTesting(false);
		}
	}

	return continueLoop;
}/* end performTBTest */

/** \brief scheduleTBResultPickup adds a pending result of a TB test to be picked up.
 * \param hasResult a bool for whether their is a result to pickup or not
 * \param testNum the ID number of the TB test
 * \param willPickupResult a bool for whether the patient will pickup the result
 * \param result a SimContext::TB_DIAG_STATUS for the result
 * \param monthOfPickup an int for when the test should be returned
 * \param resetTests a bool that determines if the patient should restart all tests upon pickup (due to taking too long to pickup)
 * \param willDoDST a bool indicating whether to do a DST
 **/
void StateUpdater::setTBTestResultPickup(bool hasResult, int testNum, bool willPickupResult, SimContext::TB_DIAG_STATUS result, int monthOfPickup, bool resetTests, bool willDoDST){
	patient->tbState.hasPendingResult = hasResult;
	if (hasResult){
		patient->tbState.currPendingResultTestNum = testNum;
		patient->tbState.willPickupResult = willPickupResult;
		patient->tbState.currPendingResult = result;
		patient->tbState.monthOfResultPickup = monthOfPickup;
		patient->tbState.resetTestsOnPickup = resetTests;
		patient->tbState.willDoDST = willDoDST;
	}

}/* end setTBTestResultPickup*/

/** \brief setTBDSTTestResultPickup adds a pending result of a DST TB test to be picked up.
 * \param hasResult a bool for whether their is a result to pickup or not
 * \param result a SimContext::TB_STRAIN for the DST test result
 * \param monthOfPickup an int for when the test should be returned
 **/
void StateUpdater::setTBDSTTestResultPickup(bool hasResult, SimContext::TB_STRAIN result, int monthOfPickup){
	patient->tbState.hasPendingDSTResult = hasResult;
	if (hasResult){
		patient->tbState.currPendingDSTResult = result;
		patient->tbState.monthOfDSTResultPickup = monthOfPickup;
	}

}/* end setTBDSTTestResultPickup */

/** \brief updateTBDiagnosticResults updates the statistics for final diagnostic TB results
 * \param result a bool for final diagnostic result
 **/
void StateUpdater::updateTBDiagnosticResults(bool result){
	// Update the longitudinal statistics for the result of the diagnostics
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		SimContext::TB_STATE tbState = patient->tbState.currTrueTBDiseaseState;
		currTime->numTBDiagnosticResults[tbState][result]++;
	}

}/* end updateTBDiagnosticResults */


/** \brief updateDSTResults updates the statisctics for DST test
 * \param strain is the observed strain
 **/
void StateUpdater::updateDSTResults(SimContext::TB_STRAIN strain){
	// Update the longitudinal statistics for the result of the diagnostics
	if (patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_UNINFECTED)
		runStats->tbStats.numDSTTestResultsUninfectedTB[strain]++;
	else
		runStats->tbStats.numDSTTestResultsByTrueTBStrain[strain][patient->tbState.currTrueTBResistanceStrain]++;

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		if (patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_UNINFECTED)
			currTime->numDSTTestResultsUninfectedTB[strain]++;
		else
			currTime->numDSTTestResultsByTrueTBStrain[strain][patient->tbState.currTrueTBResistanceStrain]++;
	}
}/* end updateDSTResults */

/** \brief performTBDiagnosis determines whether a patient is diagnosed with TB, either automatically at model start or by interpreting results at the end of the testing chain, and manages linkage to TB care
 * \param currIndex index of the current TB test (if any)
 * \param atInit a bool defaulting to false indicating whether the patient is being diagnosed upon initiation and starting the model in TB care
 **/
void StateUpdater::performTBDiagnosis(int currIndex, bool atInit){
	bool willLink = false;
	//Automatically diagnosed and linked at initiation
	if(atInit){
		patient->tbState.monthOfTBPosDiagnosis = patient->generalState.monthNum;
		willLink = true;
	}
	//Diagnosed based on test result interpreation matrix at end of testing chain
	else{
		bool posFinalResult = false;
		//Note that the test results are not Booleans but SimContext::TB_DIAG_STATUS results with TB_DIAG_STATUS_POS = 0, TB_DIAG_STATUS_NEG = 1
		if (currIndex == 0){
			if (simContext->getTBInputs()->TBDiagnosticResultMatrix1Test[patient->tbState.testResults[0]])
				posFinalResult = true;
		}
		else if (currIndex == 1){
			if (simContext->getTBInputs()->TBDiagnosticResultMatrix2Tests[patient->tbState.testResults[0]][patient->tbState.testResults[1]])
				posFinalResult = true;
		}
		else if (currIndex == 2){
			if (simContext->getTBInputs()->TBDiagnosticResultMatrix3Tests[patient->tbState.testResults[0]][patient->tbState.testResults[1]][patient->tbState.testResults[2]])
				posFinalResult = true;
		}
		else if (currIndex == 3){
			if (simContext->getTBInputs()->TBDiagnosticResultMatrix4Tests[patient->tbState.testResults[0]][patient->tbState.testResults[1]][patient->tbState.testResults[2]][patient->tbState.testResults[3]])
				posFinalResult = true;
		}

		updateTBDiagnosticResults(posFinalResult);
			
		//roll for linkage
		if (posFinalResult){
			patient->tbState.monthOfTBPosDiagnosis = patient->generalState.monthNum;
			// Output tracing if enabled
			if (patient->generalState.tracingEnabled) {
				tracer->printTrace(1, "**%d TB TESTING FINISHED, Final Result: Pos;\n", patient->generalState.monthNum);
			}
			double randNum = CepacUtil::getRandomDouble(100130, patient);
			double probLink;
			if (simContext->getTBInputs()->isIntegrated)
				probLink = simContext->getTBInputs()->probLinkTBTreatmentIntegrated;
			else
				probLink = simContext->getTBInputs()->probLinkTBTreatmentNonIntegrated;
			if (randNum < probLink){
				willLink = true;
			}
			//reset tests - pending DST results may still be returned (and potentially used to update treatment if they linked) but otherwise they have completed the algorithm
			resetTBTesting(false, false);
		}
		else{
			// Past positive TB diagnoses can be cleared if no changes in state have occurred since self-cure
			if(patient->tbState.isSelfCured){
				patient->tbState.monthOfTBPosDiagnosis = SimContext::NOT_APPL;
			}
			if (patient->generalState.tracingEnabled) {
				tracer->printTrace(1, "**%d TB TESTING FINISHED TB NOT DETECTED\n", patient->generalState.monthNum);
			}
			//reset tests - pending DST results may still be returned but otherwise they have completed the algorithm
			resetTBTesting(false, false);
			// return - nothing more to do since they were not diagnosed with TB
			return;
		}
	}	
	if(willLink){		
		//Set their TB state to in care
		setTBCareState(SimContext::TB_CARE_IN_CARE);
		// Output tracing if enabled
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d LINKING TO TB CARE\n", patient->generalState.monthNum);
		}

		//If patient is on empiric treatment transition them to real treatment
		if (patient->tbState.isOnEmpiricTreatment){
			//If patient has observed strain and empiric treatment is for different strain do not transition
			bool doTransition = true;
			if (patient->tbState.hasObservedTBResistanceStrain){
				SimContext::TB_STRAIN obsvStrain = patient->tbState.currObservedTBResistanceStrain;
				if (simContext->getTBInputs()->empiricTreatmentNum[obsvStrain] != patient->tbState.currEmpiricTreatmentNum)
					doTransition = false;
			}
			if(doTransition)
				transitionEmpiricToRealTreatment();
		}
		// If patient is HIV Pos and has not yet linked to HIV care, roll for detection and schedule initial visit if so
		if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG && !patient->monitoringState.isLinked){
			double randNum = CepacUtil::getRandomDouble(140080, patient);
			double probDet = 0;
			if (simContext->getTBInputs()->isIntegrated)
				probDet = simContext->getTBInputs()->probHIVDetUponLinkageIntegrated;
			else
				probDet = simContext->getTBInputs()->probHIVDetUponLinkageNonIntegrated;

			if (randNum < probDet) {
				if(patient->monitoringState.isDetectedHIVPositive){
					setDetectedHIVState(true, SimContext::HIV_DET_TB_PREV_DET);
					setLinkedState(true, SimContext::HIV_DET_TB_PREV_DET);
				}	
				else{
					setDetectedHIVState(true, SimContext::HIV_DET_TB);
					setLinkedState(true, SimContext::HIV_DET_TB);
				}
				// Set next month as a clinic visit
				scheduleInitialClinicVisit(patient->generalState.monthNum +1);
				if (patient->generalState.tracingEnabled){
					tracer->printTrace(1, "**%d HIV DETECTED AND LINKED BY TB;\n", patient->generalState.monthNum);
				}
			}
		}
		//If they are HIV LTFU from an integrated TB/HIV clinic, check whether they should automatically be returned to care
		else if(patient->monitoringState.careState == SimContext::HIV_CARE_LTFU && simContext->getTBInputs()->isIntegrated && simContext->getTBInputs()->RTCForHIVUponLinkageIntegrated){
			setCurrLTFUState(SimContext::LTFU_STATE_RETURNED);
			// Output tracing if enabled
			if (patient->generalState.tracingEnabled) {
				tracer->printTrace(1, "**%d PATIENT RETURNED TO CARE;\n", patient->generalState.monthNum);
			}
		}
	} //end if link to TB care
	else{
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB TESTING DID NOT LINK TO TB CARE\n",patient->generalState.monthNum);
		}
	}
}/* end performTBDiagnosis */


/** \brief countInitialTBState counts the TB state/strain at entry and records it in the runStats
 *
 * This function only should be called after the initial state has been set
 */
void StateUpdater::countInitialTBState() {
	// Update statistics for the initial TB infection
	SimContext::TB_STATE tbState = patient->tbState.currTrueTBDiseaseState;

	if (tbState == SimContext::TB_STATE_UNINFECTED)
		runStats->tbStats.numUninfectedTBAtEntry++;
	else{
		SimContext::TB_STRAIN tbStrain = patient->tbState.currTrueTBResistanceStrain;
		//Update the "atEntry" stats
		runStats->tbStats.numInStateAtEntryStrain[tbStrain][tbState]++;
	}

} /* end countInitialTBState */

/** \brief setTBProphEligibility sets whether the patient is eligible for TB Proph
 * \param isEligible a bool for whether the patient is eligible for TB Proph
 * \param hasRolledEligibility a bool for whether the patient has ever rolled for Eligibility
 **/
void StateUpdater::setTBProphEligibility(bool isEligible, bool hasRolledEligibility){
	patient->tbState.isEligibleForProph = isEligible;
	patient->tbState.hasRolledEligibleForProph = hasRolledEligibility;
} /* end setTBProphEligibility */


/** \brief startNextTBProph updates state to beginning using next TB proph
 *
 * This assumes there is a next TB proph already assigned
 **/
void StateUpdater::startNextTBProph() {
	patient->tbState.isOnProph = true;
	patient->tbState.hadProph = true;
	patient->tbState.hasCompletedProph = false;
	patient->tbState.currProphIndex = patient->tbState.nextProphIndex;
	patient->tbState.numProphStarts[patient->tbState.currProphIndex]++;
	patient->tbState.currProphNum = patient->tbState.nextProphNum;
	patient->tbState.mostRecentProphNum = patient->tbState.currProphNum;
	patient->tbState.monthOfProphStart = patient->generalState.monthNum;
	patient->tbState.isScheduledForProph = false;
	patient->tbState.hasMajorProphToxicity = false;
} /* end startNextTBProph */

/** \brief stopCurrTBProph updates state to stop using current TB proph, and if the proph line duration is completed updates state and statistics
 * \param isFinished a bool defaulting to false indicating whether the patient has completed the full duration on the current TB proph
 * 
*/
void StateUpdater::stopCurrTBProph(bool isFinished) {
	// Output tracing if enabled
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d STOP TB PROPH %d%s;\n", patient->generalState.monthNum, patient->tbState.currProphNum, isFinished? " DURATION COMPLETED":" NOT COMPLETED");
	}

	patient->tbState.isOnProph = false;
	patient->tbState.monthOfProphStop = patient->generalState.monthNum;

	if(isFinished){
		patient->tbState.hasCompletedProph = true;
		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if(currTime)
			currTime->numCompletedTBProph[patient->tbState.currProphNum][patient->tbState.currTrueTBDiseaseState]++;
	}
} /* end stopCurrTBProph */

/** \brief setNextTBProph updates proph num to be used next for TB
 *
 * \param hasNext a boolean that is true if there is a "next prophylaxis" to set; if hasNext is false, no "next Prophylaxis" is set
 * \param prophIndex an integer indicating which prophylaxis should be next
 **/
void StateUpdater::setNextTBProph(bool hasNext, int prophIndex, int prophNum) {
	patient->tbState.hasNextProphAvailable = hasNext;
	if (hasNext){
		patient->tbState.nextProphIndex = prophIndex;
		patient->tbState.nextProphNum = prophNum;
	}
} /* end setNextTBProph */

/** \brief scheduleNextTBProph updates the time lag for scheduling the next TB proph
 * \param monthStart an integer representing the month that the next TB proph should start
 **/
void StateUpdater::scheduleNextTBProph(int monthStart) {
	patient->tbState.isScheduledForProph = true;
	patient->tbState.monthOfProphStart = monthStart;

	// Output tracing if enabled
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB PROPH %d SCHEDULED FOR MONTH %d;\n",
			patient->generalState.monthNum,
			patient->tbState.nextProphNum,
			monthStart);
	}
} /* end scheduleNextTBProph */

/** \brief unscheduleNextTBProph removes the next scheduled start of TB proph */
void StateUpdater::unscheduleNextTBProph() {
	patient->tbState.isScheduledForProph = false;
	patient->tbState.monthOfProphStart = SimContext::NOT_APPL;
} /* end unscheduleNextTBProph */

/** \brief setTBProphToxicity records the TB proph toxicity
 *
 * \param isMajor a boolean that is true if the new toxicity to be record is a major toxicity
 **/
void StateUpdater::setTBProphToxicity(bool isMajor) {
	int prophNum = patient->tbState.currProphNum;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	// Update the patient state and statistics for a major toxicity
	if (isMajor){
		patient->tbState.hasMajorProphToxicity = true;
		runStats->tbStats.numProphMajorToxicity[prophNum]++;
		if (currTime) {
			currTime->numTBProphMajorTox[prophNum]++;
		}	
	}	
	// Update statistics for a minor toxicity
	else{
		runStats->tbStats.numProphMinorToxicity[prophNum]++;
		if(currTime){
			currTime->numTBProphMinorTox[prophNum]++;
		}	
	}
} /* end setTBProphToxicity */

/** \brief startNextTBTreatment updates the patient state and statistics to begin the next TB treatment
 * */
void StateUpdater::startNextTBTreatment() {
	// Update patient state for starting a new TB treatment line
	patient->tbState.isOnTreatment = true;
	if (patient->tbState.everOnTreatmentOrEmpiric)
		patient->tbState.everHadNonInitialTreatmentOrEmpiric = true;
	patient->tbState.everOnTreatmentOrEmpiric = true;
	patient->tbState.currTreatmentNum = patient->tbState.nextTreatmentNum;
	patient->tbState.monthOfTreatmentStart = patient->generalState.monthNum;
	patient->tbState.monthOfMortEfficacyStop = SimContext::NOT_APPL;
	bool wasScheduled = patient->tbState.isScheduledForTreatment;
	patient->tbState.isScheduledForTreatment = false;
	patient->tbState.mostRecentTreatNum = patient->tbState.currTreatmentNum;

	double cost = simContext->getTBInputs()->TBTreatments[patient->tbState.currTreatmentNum].costInitial;
	incrementCostsTBTreatment(cost, patient->tbState.currTreatmentNum);

	//Stop empiric therapy if any
	if (patient->tbState.isOnEmpiricTreatment)
		stopEmpiricTBTreatment();
	//Unschedule any prophs and stop current prophs
	if (patient->tbState.isScheduledForProph)
		unscheduleNextTBProph();
	if (patient->tbState.isOnProph){
		stopCurrTBProph();
		//Reset next proph line
		/** Identify the first available TB proph line */
		bool hasNext = false;
		int prophIndex = SimContext::NOT_APPL;
		int prophNum =  SimContext::NOT_APPL;
		for (int i = 0; i < SimContext::TB_NUM_PROPHS; i++) {
			if (simContext->getTBInputs()->prophOrder[i]!= SimContext::NOT_APPL) {
				if(patient->tbState.numProphStarts[i] < simContext->getTBInputs()->maxRestarts[i]+1){
					hasNext = true;
					prophIndex = i;
					prophNum = simContext->getTBInputs()->prophOrder[i];
					break;
				}
			}
		}
		setNextTBProph(hasNext, prophIndex, prophNum);
	}

	if (patient->tbState.nextTreatmentIsRepeat)
		patient->tbState.numRepeatCurrTreatment++;
	else
		patient->tbState.numRepeatCurrTreatment = 0;
	// Roll for and update the destined outcome of the treatment
	int treatNum = patient->tbState.currTreatmentNum;
	SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];

	//Set the success/failure of this regimen if it is predetermined
	if (wasScheduled && patient->tbState.setResultForScheduledTreatment){
		patient->tbState.treatmentSuccess = patient->tbState.scheduledTreatmentSuccess;
		patient->tbState.setResultForScheduledTreatment = false;
	}
	else{
		//Roll for success
		double probSuccess = 1.0;

		if (patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM
				|| patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM){
			SimContext::TB_STRAIN tbStrain = patient->tbState.currTrueTBResistanceStrain;
			if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
				probSuccess = tbTreat.probSuccessHIVNeg[tbStrain];
			else
				probSuccess = tbTreat.probSuccessHIVPos[tbStrain][patient->diseaseState.currTrueCD4Strata];
		}
		double randNum = CepacUtil::getRandomDouble(60240, patient);
		if (randNum < probSuccess)
			patient->tbState.treatmentSuccess = true;
		else
			patient->tbState.treatmentSuccess = false;
	}

	if (!patient->tbState.treatmentSuccess){
		if (!patient->tbState.everHadNonInitialTreatmentOrEmpiric)
			setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE_FAILURE);
	}		

	// Output tracing if enabled
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB TREAT %d STARTING, Result: %s, %1.2lf QAred, $ %1.0lf;\n",
			patient->generalState.monthNum,
			patient->tbState.currTreatmentNum,
			patient->tbState.treatmentSuccess?"Success":"Failure",
			patient->generalState.QOLValue,
			patient->generalState.costsDiscounted);
	}

	// Update statistics for the starting a new TB treatment line
	SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS;
	if (patient->tbState.hasObservedTBResistanceStrain)
		obsvStrain = patient->tbState.currObservedTBResistanceStrain;
	runStats->tbStats.numStartOnTreatment[obsvStrain][treatNum]++;
} /* end startNextTBTreatment */

/** \brief stopCurrTBTreatment updates the patient state and statistics to stop the current TB treatment
 *
 * \param isFinished a boolean that is false if they stopped treatment early
 * \param isCured a boolean that is true if the patient was cured while on treatment (i.e., transitioned to Uninfected or Previously Treated at the time they stopped)
 **/
void StateUpdater::stopCurrTBTreatment(bool isFinished, bool isCured) {
	// Output tracing if enabled
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB TREAT %d STOPPING;\n",
			patient->generalState.monthNum,
			patient->tbState.currTreatmentNum);
	}

	// Update patient state for stopping TB treatment
	patient->tbState.isOnTreatment = false;
	if(!patient->tbState.hasStoppedTreatmentOrEmpiric){
		patient->tbState.monthOfInitialTreatmentStop = patient->generalState.monthNum;
	}
	patient->tbState.hasStoppedTreatmentOrEmpiric = true;
	patient->tbState.monthOfTreatmentOrEmpiricStop = patient->generalState.monthNum;

	// Update the statistics for dropping out of treatment (i.e., going LTFU while on treatment) or completing treatment, and if cured
	SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS;
	if (patient->tbState.hasObservedTBResistanceStrain)
		obsvStrain = patient->tbState.currObservedTBResistanceStrain;
	int treatNum = patient->tbState.currTreatmentNum;
	if (isFinished) {
		patient->tbState.everCompletedTreatmentOrEmpiric = true;
		runStats->tbStats.numFinishTreatment[obsvStrain][treatNum]++;
		if (isCured){
			runStats->tbStats.numCuredAtTreatmentFinish[patient->tbState.currTrueTBResistanceStrain][treatNum]++;
		}	
	}
	// Only those who stop due to LTFU are counted as "dropping out"
	else if (patient->tbState.hasIncompleteTreatment){
		runStats->tbStats.numDropoutTreatment[obsvStrain][treatNum]++;

		RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
		if (currTime) {
			currTime->numDropoutTBTreatment[obsvStrain][treatNum]++;
		}
	}
} /* end stopCurrTBTreatment */


/** \brief startEmpiricTBTreatment updates the patient state and statistics to start the empiric TB treatment
 *
 * \param treatNum a int indicating the next treatment line the patient should go on
 * \param previousDuration an int indicating how much of the treatment they have already completed (only applied if moving from a higher resist treat to lower)
 **/
void StateUpdater::startEmpiricTBTreatment(int treatNum, int previousDuration) {
	// Update patient state for starting a new TB treatment line
	patient->tbState.isOnEmpiricTreatment = true;
	if (patient->tbState.everOnTreatmentOrEmpiric)
		patient->tbState.everHadNonInitialTreatmentOrEmpiric = true;
	patient->tbState.everOnTreatmentOrEmpiric = true;
	patient->tbState.currEmpiricTreatmentNum = treatNum;
	patient->tbState.monthOfEmpiricTreatmentStart = patient->generalState.monthNum;
	patient->tbState.previousEmpiricTreatmentDuration = previousDuration;
	patient->tbState.mostRecentTreatNum = patient->tbState.currEmpiricTreatmentNum;

	double cost = simContext->getTBInputs()->TBTreatments[patient->tbState.currEmpiricTreatmentNum].costInitial;
	incrementCostsTBTreatment(cost, patient->tbState.currEmpiricTreatmentNum);

	//Unschedule any prophs and stop current prophs
	if (patient->tbState.isScheduledForProph)
		unscheduleNextTBProph();
	if (patient->tbState.isOnProph){
		stopCurrTBProph();
		//Reset next proph line
		/** Identify the first available TB proph line */
		bool hasNext = false;
		int prophIndex = SimContext::NOT_APPL;
		int prophNum =  SimContext::NOT_APPL;
		for (int i = 0; i < SimContext::TB_NUM_PROPHS; i++) {
			if (simContext->getTBInputs()->prophOrder[i]!= SimContext::NOT_APPL) {
				if(patient->tbState.numProphStarts[i] < simContext->getTBInputs()->maxRestarts[i]+1){
					hasNext = true;
					prophIndex = i;
					prophNum = simContext->getTBInputs()->prophOrder[i];
					break;
				}
			}
		}
		setNextTBProph(hasNext, prophIndex, prophNum);
	}

	// Roll for and update the destined outcome of the treatment
	SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];

	double probSuccess = 1.0;
	if (patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM
			|| patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM){
		SimContext::TB_STRAIN tbStrain = patient->tbState.currTrueTBResistanceStrain;
		if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
			probSuccess = tbTreat.probSuccessHIVNeg[tbStrain];
		else
			probSuccess = tbTreat.probSuccessHIVPos[tbStrain][patient->diseaseState.currTrueCD4Strata];
	}
	double randNum = CepacUtil::getRandomDouble(60240, patient);
	if (randNum < probSuccess)
		patient->tbState.empiricTreatmentSuccess = true;
	else
		patient->tbState.empiricTreatmentSuccess = false;

	if (!patient->tbState.empiricTreatmentSuccess){
		if(!patient->tbState.everHadNonInitialTreatmentOrEmpiric)
			setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE_FAILURE);
	}		

	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB EMPIRIC TREAT %d STARTING, Result: %s, %1.2lf QAred, $ %1.0lf;\n",
			patient->generalState.monthNum,
			patient->tbState.currEmpiricTreatmentNum,
			patient->tbState.empiricTreatmentSuccess?"Success":"Failure",
			patient->generalState.QOLValue,
			patient->generalState.costsDiscounted);
	}
} /* end startEmpiricTBTreatment */

/** \brief stopEmpiricTBTreatment updates the patient state and statistics to stop empiric TB treatment
 * \param isFinished a bool defaulting to false indicating whether they have completed the full duration on empiric TB treatment 
 * \param isCured a bool defaulting to false indicating whether they made a transition to Uninfected or Previously Treated upon stopping empiric treatment 
 *
 **/
void StateUpdater::stopEmpiricTBTreatment(bool isFinished, bool isCured) {
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB EMPIRIC STOPPING, treatnum: %d;\n",
			patient->generalState.monthNum,
			patient->tbState.currEmpiricTreatmentNum);
	}

	patient->tbState.isOnEmpiricTreatment = false;
	if(!patient->tbState.hasStoppedTreatmentOrEmpiric){
		patient->tbState.monthOfInitialTreatmentStop = patient->generalState.monthNum;
	}	
	patient->tbState.hasStoppedTreatmentOrEmpiric = true;
	patient->tbState.monthOfTreatmentOrEmpiricStop = patient->generalState.monthNum;
	if(isFinished){
		patient->tbState.everCompletedTreatmentOrEmpiric = true;
		if(isCured){
			runStats->tbStats.numCuredAtTreatmentFinish[patient->tbState.currTrueTBResistanceStrain][patient->tbState.currEmpiricTreatmentNum]++;
		}
	}
} /* end stopEmpiricTBTreatment */

/** \brief transitionEmpiricToRealTreatment handles the change when patient links to TB care if they are on empiric TB treatment
 *
 **/
void StateUpdater::transitionEmpiricToRealTreatment(){
	if (!patient->tbState.isOnEmpiricTreatment)
		return;

	patient->tbState.isOnTreatment = true;
	patient->tbState.currTreatmentNum = patient->tbState.currEmpiricTreatmentNum;
	patient->tbState.monthOfTreatmentStart = patient->tbState.monthOfEmpiricTreatmentStart;
	patient->tbState.isScheduledForTreatment = false;
	patient->tbState.isOnEmpiricTreatment = false;
	patient->tbState.numRepeatCurrTreatment = 0;
	patient->tbState.nextTreatmentIsRepeat = false;
	patient->tbState.previousTreatmentDuration = 0;
	patient->tbState.setResultForScheduledTreatment = false;
	patient->tbState.scheduledTreatmentSuccess = false;

	// update the destined outcome of the treatment
	int treatNum = patient->tbState.currTreatmentNum;
	SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[treatNum];
	patient->tbState.treatmentSuccess =patient->tbState.empiricTreatmentSuccess;
} /* end transitionEmpiricToRealTreatment */


/** \brief scheduleNextTBTreatment updates the patients next scheduled TB treatment after events such as TB RTC and returning DST results 
 *
 * \param treatNum a int indicating the next treatment line the patient should go on
 * \param monthStart an integer indicating which month the next treatment should start
 * \param isRepeat a bool indicating whether the patient is repeating this treatment line
 * \param previousDuration an int indicating how many months the patient has been on this treatment line, used when they resume incomplete regimens
 * \param hasResult a bool indicating whether the success of the treatment is predetermined (resuming regimen) or the patient will roll for the result at treatment start
 **/
void StateUpdater::scheduleNextTBTreatment(int treatNum, int monthStart, bool isRepeat, int previousDuration, bool hasResult) {
	patient->tbState.isScheduledForTreatment = true;
	patient->tbState.nextTreatmentNum = treatNum;
	patient->tbState.monthOfTreatmentStart = monthStart;
	patient->tbState.nextTreatmentIsRepeat = isRepeat;
	patient->tbState.previousTreatmentDuration = previousDuration;
	patient->tbState.setResultForScheduledTreatment = hasResult;

	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB TREAT %d SCHEDULED, month start:%d, previous duration: %d;\n",
			patient->generalState.monthNum,
			treatNum,
			monthStart,
			previousDuration);
	}
} /* end scheduleNextTBTreatment */

/** \brief unscheduleNextTBTreatment removes the next scheduled start of TB treatment */
void StateUpdater::unscheduleNextTBTreatment() {
	patient->tbState.isScheduledForTreatment = false;
} /* end unscheduleNextTBTreatment */


/** \brief increaseTBDrugResistance updates that patient state and stats for an increase in TB drug resistance
 *
 * \param fromTreatment a boolean that is true if the resistance is triggered by failed treatment or treatment default; otherwise it is triggered by TB proph
 **/
void StateUpdater::increaseTBDrugResistance(bool fromTreatment) {
	// Update the patient state for the increased drug resistance
	if(patient->tbState.currTrueTBDiseaseState != SimContext::TB_STATE_UNINFECTED){
		SimContext::TB_STRAIN tbStrain = patient->tbState.currTrueTBResistanceStrain;
		if (tbStrain < SimContext::TB_STRAIN_XDR){
			patient->tbState.currTrueTBResistanceStrain = (SimContext::TB_STRAIN) (tbStrain + 1);

			// Update the statistics for the increased drug resistance
			if (fromTreatment){
				int treatNum;

				if (patient->tbState.isOnTreatment)
					treatNum = patient->tbState.currTreatmentNum;
				else if (patient->tbState.isOnEmpiricTreatment)
					treatNum = patient->tbState.currEmpiricTreatmentNum;
				else if (patient->tbState.hasIncompleteTreatment)
					treatNum = patient->tbState.incompleteTreatmentLine;

				runStats->tbStats.numIncreaseResistanceAtTreatmentStop[tbStrain][treatNum]++;
			}
			else{
				RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
				if (currTime) {
					currTime->numIncreaseResistanceDueToProph[tbStrain][patient->tbState.currProphNum]++;
				}
			}
		}
	}
} /* end increaseTBDrugResistance */

/** \brief setTBTreatmentToxicity records the TB treatment toxicity
 *
 * \param isMajor a boolean that is true if this is a major toxicity
 *
 * Statistics related to TB treatment toxicity are updated here
 **/
void StateUpdater::setTBTreatmentToxicity(bool isMajor, int treatNum) {
	// Update the statistics for the toxicity occurrence
	if (isMajor){
		setHadTBTreatmentMajorTox(true);
		runStats->tbStats.numTreatmentMajorToxicity[treatNum]++;
	}
	else
		runStats->tbStats.numTreatmentMinorToxicity[treatNum]++;
} /* end setTBTreatmentToxicity */

/** \brief setHadTBTreatmentMajorTox  records whether the patient ever had major tox while on empiric or non-empiric TB treatment
 *
 **/

void StateUpdater::setHadTBTreatmentMajorTox(bool hadMajor){
	patient->tbState.hadTreatmentMajorTox = hadMajor;
} /* end setHadTBTreatmentMajorTox */


/** \brief setTBLTFU handles the patient being LTFU to TB care
 *
 **/
void StateUpdater::setTBLTFU() {
	patient->tbState.careState = SimContext::TB_CARE_LTFU;
	patient->tbState.monthOfLTFU = patient->generalState.monthNum;
	patient->tbState.willTreatmentDefault = false;
	//Now that there is no longer an input for lag to treatment start, people should not be scheduled for TB treatment when they go TB LTFU 
	assert(!patient->tbState.isScheduledForTreatment);
	//Stop the current treatment regimen if any
	if (patient->tbState.isOnTreatment){
		SimContext::TB_STATE tbState = patient->tbState.currTrueTBDiseaseState;
		int currTreatNum = patient->tbState.currTreatmentNum;
		SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[currTreatNum];
		int timeOnTreatment = patient->generalState.monthNum - patient->tbState.monthOfTreatmentStart + patient->tbState.previousTreatmentDuration;

		if (!patient->tbState.everHadNonInitialTreatmentOrEmpiric)
			setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE_LTFU);

		patient->tbState.hasIncompleteTreatment = true;
		patient->tbState.incompleteTreatmentLine = currTreatNum;
		patient->tbState.previousTreatmentDuration = timeOnTreatment;
		patient->tbState.scheduledTreatmentSuccess = patient->tbState.treatmentSuccess;
		//Roll for Treatment Default based on how long they have been on the regimen
		if( tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM || tbState == SimContext::TB_STATE_ACTIVE_PULM){
			//Long-term effects of stopping treatment early such as higher mortality and Treatment Default / drug resistance set in after people are LTFU for a user-defined number of months
			patient->tbState.monthOfMortEfficacyStop = patient->generalState.monthNum + simContext->getTBInputs()->monthsToLongTermEffectsLTFU;
			if (patient->tbState.treatmentSuccess){
				double propOfTreat = timeOnTreatment/(float) tbTreat.totalDuration;
				double randNum = CepacUtil::getRandomDouble(60240, patient);
				if (randNum < propOfTreat){
					patient->tbState.willTreatmentDefault = true;

					//Roll for increased resistance after default
					SimContext::TB_STRAIN trueStrain = patient->tbState.currTrueTBResistanceStrain;
					double randNum = CepacUtil::getRandomDouble(60210, patient);
					if ((randNum < simContext->getTBInputs()->TBTreatmentProbResistAfterDefault[currTreatNum]) &&
						(trueStrain < SimContext::TB_STRAIN_XDR)) {
						patient->tbState.willIncreaseResistanceUponDefault=true;
					}
					else
						patient->tbState.willIncreaseResistanceUponDefault=false;

				}
			}
		}
		//Stop the current treatment
		stopCurrTBTreatment(false, false);
	}

	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB LTFU, TB State: %s;\n",
			patient->generalState.monthNum,
			SimContext::TB_STATE_STRS[patient->tbState.currTrueTBDiseaseState]);
	}

} /* end setTBLTFU */

/** \brief checkForTreatmentDefault handles the transition from active state to treatment default
 *
 **/
void StateUpdater::checkForTreatmentDefault(){
	SimContext::TB_STATE tbState = patient->tbState.currTrueTBDiseaseState;
	//Patient must still be in active state (no self-cure) and be ltfu
	if (patient->tbState.careState == SimContext::TB_CARE_LTFU &&
			(tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM || tbState == SimContext::TB_STATE_ACTIVE_PULM)){
		if (patient->tbState.willTreatmentDefault && (patient->generalState.monthNum == patient->tbState.monthOfLTFU + simContext->getTBInputs()->monthsToLongTermEffectsLTFU)){
			setTBDiseaseState(SimContext::TB_STATE_TREAT_DEFAULT, false, SimContext::TB_INFECT_INITIAL, tbState);

			if (patient->tbState.hasIncompleteTreatment){
				int treatNum = patient->tbState.incompleteTreatmentLine;
				SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS;
				if (patient->tbState.hasObservedTBResistanceStrain)
					obsvStrain = patient->tbState.currObservedTBResistanceStrain;
				runStats->tbStats.numTransitionsToTBTreatmentDefault[obsvStrain][treatNum]++;
			
				RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
				if (currTime) {
					currTime->numDefaultTBTreatment[treatNum]++;
					if(tbState == SimContext::TB_STATE_ACTIVE_PULM)
						currTime->numDefaultTBTreatmentPulm[treatNum]++;
					if(tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
						currTime->numDefaultTBTreatmentExtraPulm[treatNum]++;
				}
			}

			if (patient->generalState.tracingEnabled) {
				tracer->printTrace(1, "**%d TB TREAT DEFAULT;\n",
					patient->generalState.monthNum);
			}

			//Roll for increased resistance after default

			if (patient->tbState.willIncreaseResistanceUponDefault){
				increaseTBDrugResistance(true);
				SimContext::TB_STRAIN newTBStrain = patient->tbState.currTrueTBResistanceStrain;
				if (patient->generalState.tracingEnabled) {
					tracer->printTrace(1, "**%d TB TREAT DEFAULT INCR RESIST TO %s;\n",
						patient->generalState.monthNum, SimContext::TB_STRAIN_STRS[newTBStrain]);
				}
			}

		}
	}
} /* end checkForTreatmentDefault */


/** \brief setTBRTC handles the patient returning to TB care
 *  \param maxMonthsExceeded a bool defaulting to false indicating whether the patient has been TBLTFU for more than the maximum number of months, in which case they will be unlinked from TB care
 *
 **/
void StateUpdater::setTBRTC(bool maxMonthsExceded) {
	patient->tbState.careState = SimContext::TB_CARE_IN_CARE;
	bool hasIncompleteTreatment = patient->tbState.hasIncompleteTreatment;
	patient->tbState.hasIncompleteTreatment = false;
	SimContext::TB_STATE tbState = patient->tbState.currTrueTBDiseaseState;

	//Those who have exceded max months LTFU or those not in the active state or default state will exit from care upon returning
	if( maxMonthsExceded || !(tbState == SimContext::TB_STATE_ACTIVE_EXTRAPULM || tbState == SimContext::TB_STATE_ACTIVE_PULM || tbState == SimContext::TB_STATE_TREAT_DEFAULT)){
		patient->tbState.careState = SimContext::TB_CARE_UNLINKED;
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB RTC UNLINKED;\n",
				patient->generalState.monthNum);
		}
		return;
	}

	//Roll for what to do upon return (only active state and default state)
	int timeCat = getTimeCatTBRTC(patient->generalState.monthNum-patient->tbState.monthOfLTFU);
	double randNum = CepacUtil::getRandomDouble(140020, patient);
	if(randNum < simContext->getTBInputs()->rtcProbRestart[timeCat]) {
		//Restart previous line
		if (hasIncompleteTreatment)
			scheduleNextTBTreatment(patient->tbState.incompleteTreatmentLine, patient->generalState.monthNum);
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB RTC RESTART PREVIOUS;\n",
				patient->generalState.monthNum);
		}
		return;
	}
	randNum -= simContext->getTBInputs()->rtcProbRestart[timeCat];
	if (randNum < simContext->getTBInputs()->rtcProbResume[timeCat]){
		//Resume previous line
		if (hasIncompleteTreatment){
			scheduleNextTBTreatment(patient->tbState.incompleteTreatmentLine, patient->generalState.monthNum, false, patient->tbState.previousTreatmentDuration, true);
		}
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB RTC RESUME PREVIOUS;\n",
				patient->generalState.monthNum);
		}
		return;
	}
	randNum -= simContext->getTBInputs()->rtcProbResume[timeCat];

	if (randNum < simContext->getTBInputs()->rtcProbRetest[timeCat]){
		//Redo all tests
		patient->tbState.careState = SimContext::TB_CARE_UNLINKED;
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB RTC RETEST;\n",
				patient->generalState.monthNum);
		}
		return;
	}
	randNum -= simContext->getTBInputs()->rtcProbRetest[timeCat];

	if (randNum < simContext->getTBInputs()->rtcProbNext[timeCat]){
		//Move onto next line
		if (hasIncompleteTreatment){
			SimContext::TBInputs::TBTreatment tbTreat = simContext->getTBInputs()->TBTreatments[patient->tbState.incompleteTreatmentLine];
			if (tbTreat.nextTreatNumNormalFail != SimContext::NOT_APPL)
				scheduleNextTBTreatment(tbTreat.nextTreatNumNormalFail, patient->generalState.monthNum);
		}
		if (patient->generalState.tracingEnabled) {
			tracer->printTrace(1, "**%d TB RTC NEXT LINE;\n",
				patient->generalState.monthNum);
		}
		return;
	}
} /* end setTBRTC */


/** \brief getTimeCatTBRTC gets the bucket of time that the patient has been lost to TB Care
 * \param timeSinceLost is the number of months the patient has been LTFU from TB care
 **/
/* getTimeCatTBRTC gets the bucket of time that the patient has been lost to TB Care */
int StateUpdater::getTimeCatTBRTC(int timeSinceLost) {
	if (timeSinceLost <= 2)
		return 0;
	if (timeSinceLost <= 4)
		return 1;
	if (timeSinceLost <= 6)
		return 2;
	if (timeSinceLost <= 8)
		return 3;
	return 4;
} /* end getTimeCatTBRTC */


/** \brief setTBSelfCure handles TB self-cure
 * 	\param isSelfCured a bool indicating whether the patient transitioned into their current TB state via self-cure
*/
void StateUpdater::setTBSelfCure(bool isSelfCured){
	patient->tbState.isSelfCured = isSelfCured;
	if(!isSelfCured){
		return;
	}
	// If TB self-cure has occurred, perform updates 
	// Increment the number of self-cures
	runStats->tbStats.numTBSelfCures[patient->tbState.currTrueTBResistanceStrain]++;
	//Set them to unlinked
	setTBCareState(SimContext::TB_CARE_UNLINKED);

	setTBDiseaseState(SimContext::TB_STATE_PREV_TREATED, false, SimContext::TB_INFECT_INITIAL, patient->tbState.currTrueTBDiseaseState);

	// Output tracing if enabled
	if (patient->generalState.tracingEnabled) {
		tracer->printTrace(1, "**%d TB SELF CURE;\n", patient->generalState.monthNum);
	}

	//Reset Tb Testing
	resetTBTesting(true, true);

	//Stop empiric therapy if any
	if (patient->tbState.isOnEmpiricTreatment)
		stopEmpiricTBTreatment(true, true);
	//stop tb treatment if any
	if (patient->tbState.isOnTreatment)
		stopCurrTBTreatment(true, true);
	//Unschedule TB treatment if scheduled (for instance, if they just returned to care at a non-integrated clinic last month)
	if (patient->tbState.isScheduledForTreatment)
		unscheduleNextTBTreatment();
	//Unschedule any prophs and stop current prophs
	if (patient->tbState.isScheduledForProph)
		unscheduleNextTBProph();
	if (patient->tbState.isOnProph){
		stopCurrTBProph();
		//Reset next proph line
		/** Identify the first available TB proph line */
		bool hasNext = false;
		int prophIndex = SimContext::NOT_APPL;
		int prophNum =  SimContext::NOT_APPL;
		for (int i = 0; i < SimContext::TB_NUM_PROPHS; i++) {
			if (simContext->getTBInputs()->prophOrder[i]!= SimContext::NOT_APPL) {
				if(patient->tbState.numProphStarts[i] < simContext->getTBInputs()->maxRestarts[i]+1){
					hasNext = true;
					prophIndex = i;
					prophNum = simContext->getTBInputs()->prophOrder[i];
					break;
				}
			}
		}
		setNextTBProph(hasNext, prophIndex, prophNum);
	}

} /* end setTBSelfCure */

/** \brief setTBInitPolicyIntervalEligible sets eligibility for interval-based TB testing initiation policy*/
void StateUpdater::setTBInitPolicyIntervalEligible(bool isEligible){
	patient->tbState.isEligibleTBInitPolicyInterval = isEligible;
} /* end setTBInitPolicyIntervalEligible */



/** \brief setTrueCHRMsState updates the patients state for occurrence of CHRMs diseases
 *
 * \param chrmNum is an integer representing which CHRM is being set
 * \param hasCHRM a boolean that is true if the patient has the CHRM chrmNum
 * \param isInitial a boolean that is true if this is the state being set at time 0
 * \param monthsStart an integer representing the number of months prior to the current month that the CHRM state started (this is only in effect if isInitial is true)
 **/
void StateUpdater::setTrueCHRMsState(int chrmNum, bool hasCHRM, bool isInitial, int monthsStart) {
    patient->diseaseState.hasTrueCHRMs[chrmNum] = hasCHRM;
	if(hasCHRM){
		runStats->chrmsStats.numPatientsWithCHRM[chrmNum]++;
        // Update the longitudinal statistics for the occurrence of the CHRM
        RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
        if (currTime) {
            if (!isInitial)
                currTime->numIncidentCHRMs[chrmNum]++;
        }
		if(patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG){
			runStats->chrmsStats.numPatientsWithCHRMHIVPos[chrmNum]++;
		    SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
			if (isInitial)
				runStats->chrmsStats.numPrevalentCHRMCD4[chrmNum][cd4Strata]++;
			else
				runStats->chrmsStats.numIncidentCHRMCD4[chrmNum][cd4Strata]++;
		}
		else{
			runStats->chrmsStats.numPatientsWithCHRMHIVNeg[chrmNum]++;
			if (isInitial)
				runStats->chrmsStats.numPrevalentCHRMHIVNeg[chrmNum]++;
			else
				runStats->chrmsStats.numIncidentCHRMHIVneg[chrmNum]++;
		}
		
        // set month of stage 0 (incidence/prevalence)
		if (isInitial)
			patient->diseaseState.monthOfCHRMsStageStart[chrmNum][0] = patient->generalState.monthNum - monthsStart;
		else
			patient->diseaseState.monthOfCHRMsStageStart[chrmNum][0] = patient->generalState.monthNum;
		// draw for months of subsequent stage transitions
		for (int i = 0; i < SimContext::CHRM_TIME_PER_NUM-1; i++){
            double stageDuration = -1;
            double meanDuration = simContext->getCHRMsInputs()->durationCHRMSstage[i][chrmNum][0];
            double stdDuration = simContext->getCHRMsInputs()->durationCHRMSstage[i][chrmNum][1];
            if (simContext->getCHRMsInputs()->enableCHRMSDurationSqrtTransform){
                while (stageDuration < 0){
                    stageDuration = CepacUtil::getRandomGaussian(meanDuration, stdDuration, 150040, patient);
                }
                stageDuration = stageDuration * stageDuration;
            }
            else{
                while (stageDuration < 0){
                    stageDuration = CepacUtil::getRandomGaussian(meanDuration, stdDuration, 150050, patient);
                }
            }
            int rounded = (int) stageDuration + .5;
            patient->diseaseState.monthOfCHRMsStageStart[chrmNum][i+1] = patient->diseaseState.monthOfCHRMsStageStart[chrmNum][i] + rounded;
		}
	}
}

/** \brief setInitialOIHistory updates the patient state for initial OI history
 *
 * \param hasHistory[SimContext::OI_NUM] an array of booleans indicating which OI the patient has an initial history of
 **/
void StateUpdater::setInitialOIHistory(bool hasHistory[SimContext::OI_NUM]) {
	patient->diseaseState.typeTrueOIHistory = SimContext::HIST_EXT_N;
	patient->diseaseState.monthOfFirstOI = SimContext::NOT_APPL;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		patient->diseaseState.lastMonthSevereOI[i] = SimContext::NOT_APPL;
		patient->diseaseState.hasTrueOIHistory[i] = hasHistory[i];
		if (hasHistory[i]) {
			if (simContext->getRunSpecsInputs()->severeOIs[i]) {
				patient->diseaseState.typeTrueOIHistory = SimContext::HIST_EXT_SEVR;
			}
			else if (patient->diseaseState.typeTrueOIHistory != SimContext::HIST_EXT_SEVR) {
				patient->diseaseState.typeTrueOIHistory = SimContext::HIST_EXT_MILD;
			}
		}
	}
} /* setInitialOIHistory */

/** \brief setOIHistory updates the patient state for OI history
 *
 * OI history is set based on whether or not an OI occurred this month.  Also sets the time of the last severe OI. This is called at the
 * end of the month since current acute OI should not count as history until the next month */
void StateUpdater::setOIHistory() {
	if (patient->diseaseState.hasCurrTrueOI) {
		SimContext::OI_TYPE oiType = patient->diseaseState.typeCurrTrueOI;
		patient->diseaseState.hasTrueOIHistory[oiType] = true;
		if (simContext->getRunSpecsInputs()->severeOIs[oiType]) {
			patient->diseaseState.typeTrueOIHistory = SimContext::HIST_EXT_SEVR;
			patient->diseaseState.lastMonthSevereOI[oiType] = patient->generalState.monthNum;
		}
		else if (patient->diseaseState.typeTrueOIHistory != SimContext::HIST_EXT_SEVR) {
			patient->diseaseState.typeTrueOIHistory = SimContext::HIST_EXT_MILD;
		}
	}
} /* end setOIHistory */

/** \brief setCurrTrueOI updates the patient state when an acute OI event occurs or resets back to none
 *
 * \param oiType a SimContext::OI_TYPE indicating which OI occured
 *
 * RunStats related to acute OIs are updated here
 **/
void StateUpdater::setCurrTrueOI(SimContext::OI_TYPE oiType) {
	// Update the patient state for the occurrence of the OI
	if (oiType == SimContext::OI_NONE) {
		patient->diseaseState.hasCurrTrueOI = false;
		return;
	}
	patient->diseaseState.hasCurrTrueOI = true;
	patient->diseaseState.typeCurrTrueOI = oiType;
	patient->diseaseState.numTrueOIsSinceLastVisit[oiType]++;


	//Update OI if on ART
	if(patient->artState.isOnART){
		int artLineNum = patient->artState.currRegimenNum;
		double responseLogit = patient->artState.responseLogitCurrRegimen;
		double propRespond = pow(1 + exp(0 - responseLogit), -1);
		double currTrueCD4Value = 0;

		runStats->artStats.numARTOI[artLineNum][oiType]++;
		runStats->artStats.numARTOICD4Metric[artLineNum][oiType][patient->pedsState.ageCategoryCD4Metric]++;
		if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
			currTrueCD4Value = patient->diseaseState.currTrueCD4;
		else
			currTrueCD4Value = patient->diseaseState.currTrueCD4Percentage;
		runStats->artStats.trueCD4AtARTOISum[artLineNum][oiType][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;
		if (patient->monitoringState.hasObservedCD4){
			runStats->artStats.numWithObservedCD4AtARTOI[artLineNum][oiType]++;
			runStats->artStats.observedCD4AtARTOISum[artLineNum][oiType] += patient->monitoringState.currObservedCD4;
		}
		runStats->artStats.propensityAtARTOISum[artLineNum][oiType]+= propRespond;
	}

	// Update the statistics for the occurrence of the OI
	SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
	if (!patient->diseaseState.hasTrueOIHistory[oiType]) {
		runStats->oiStats.numPrimaryOIsCD4OI[cd4Strata][oiType]++;
	}
	else {
		runStats->oiStats.numSecondaryOIsCD4OI[cd4Strata][oiType]++;
	}

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//OI Events
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numOIEvents++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numOIEvents++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numOIEvents++;
		}
	}

	// Update the longitudinal statistics for the occurrence of the OI
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->numOIsTotal++;
		SimContext::OI_TYPE oiType = patient->diseaseState.typeCurrTrueOI;
		if (!patient->diseaseState.hasTrueOIHistory[oiType]) {
			currTime->numPrimaryOIs[oiType]++;
			currTime->numPrimaryOIsTotal++;
		}
		else {
			currTime->numSecondaryOIs[oiType]++;
			currTime->numSecondaryOIsTotal++;
		}
		if ((patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N) &&
			(simContext->getRunSpecsInputs()->firstOIsLongitLogging[oiType])) {
			currTime->numWithFirstOI[oiType]++;
			patient->diseaseState.monthOfFirstOI = patient->generalState.monthNum;
			patient->diseaseState.firstOIType = oiType;
		}
	}
} /* end setCurrTrueOI */

/** \brief setCurrObservedOI updates the patient state when a patient's current true OI is observed or resets back to no observed OI
 *
 * \param isObserved a bool indicating whether the current true OI has been observed
 *
 **/
void StateUpdater::setCurrObservedOI(bool isObserved) {
	// Update the patient state for the occurrence of the OI
	patient->diseaseState.hasCurrObservedOI = isObserved;
} /* end setCurrObservedOI */	

/** \brief clearMortalityRisks clears the list of possible death risks for the month */
void StateUpdater::clearMortalityRisks() {
	patient->diseaseState.mortalityRisks.clear();
} /* end clearMortalityRisks */

/** \brief addMortalityRisk adds a new risk of death and its death rate ratio for the current month
 *
 * \param causeOfDeath a SimContext::DTH_CAUSES indicating the new risk of death
 * \param deathRateRatio a double representing the ratio of the death rate for those with the risk to the death rate for those without the risk
 * \param costDeath a double representing the cost accrued by this type of death
 **/
void StateUpdater::addMortalityRisk(SimContext::DTH_CAUSES causeOfDeath, double deathRateRatio, double costDeath) {
	SimContext::MortalityRisk deathRisk;
	deathRisk.causeOfDeath = causeOfDeath;
	deathRisk.deathRateRatio = deathRateRatio;
	deathRisk.costDeath = costDeath;
	patient->diseaseState.mortalityRisks.push_back(deathRisk);
} /* end addMortalityRisk */

/** \brief setCauseOfDeath updates the patient state to reflect that death has occurred
 *
 * \param causeOfDeath a SimContext::DTH_CAUSES representing the cause of death
 *
 * RunStats statistics relating to the patient's state at death are updated here
 **/
void StateUpdater::setCauseOfDeath(SimContext::DTH_CAUSES causeOfDeath) {
	// Update the patient state to reflect that death has occurred
	patient->diseaseState.isAlive = false;
	patient->diseaseState.causeOfDeath = causeOfDeath;
	// Increment care and age stratified death stats
	runStats->deathStats.numDeathsType[patient->diseaseState.causeOfDeath]++;
	runStats->deathStats.numDeathsCareType[patient->monitoringState.careState][patient->diseaseState.causeOfDeath]++;
    int ageBracketOutput = getAgeCategoryOutput(patient->generalState.ageMonths);
    runStats->deathStats.numDeathsTypeAge[patient->diseaseState.causeOfDeath][ageBracketOutput]++;
	//Update death statatics if on ART
	if(patient->artState.isOnART){
		int artLineNum = patient->artState.currRegimenNum;
		double currTrueCD4Value = 0; 
		if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE)
			currTrueCD4Value = patient->diseaseState.currTrueCD4;
		else
			currTrueCD4Value = patient->diseaseState.currTrueCD4Percentage;	

		runStats->artStats.numARTDeath[artLineNum]++;
		runStats->artStats.numARTDeathCD4Metric[artLineNum][patient->pedsState.ageCategoryCD4Metric]++;
		runStats->artStats.numARTDeathCause[artLineNum][causeOfDeath]++;
		runStats->artStats.numARTDeathCauseCD4Metric[artLineNum][causeOfDeath][patient->pedsState.ageCategoryCD4Metric]++;
		runStats->artStats.trueCD4AtARTDeathSum[artLineNum][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;
		runStats->artStats.trueCD4AtARTDeathCauseSum[artLineNum][causeOfDeath][patient->pedsState.ageCategoryCD4Metric] += currTrueCD4Value;

		if (patient->monitoringState.hasObservedCD4){
			runStats->artStats.numWithObservedCD4AtARTDeath[artLineNum]++;
			runStats->artStats.observedCD4AtARTDeathSum[artLineNum] += patient->monitoringState.currObservedCD4;
			runStats->artStats.numWithObservedCD4AtARTDeathCause[artLineNum][causeOfDeath]++;
			runStats->artStats.observedCD4AtARTDeathCauseSum[artLineNum][causeOfDeath] += patient->monitoringState.currObservedCD4;
		}
		double responseLogit = patient->artState.responseLogitCurrRegimen;
		double propRespond = pow(1 + exp(0 - responseLogit), -1);
		runStats->artStats.propensityAtARTDeathSum[artLineNum]+= propRespond;
		runStats->artStats.propensityAtARTDeathCauseSum[artLineNum][causeOfDeath]+= propRespond;
	}
	// update the run statistics for HIV-negative patients
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG) {
		runStats->deathStats.numDeathsUninfected++;
		if (causeOfDeath == SimContext::DTH_ACTIVE_TB) {
			SimContext::TB_STRAIN tbStrain = patient->tbState.currTrueTBResistanceStrain;
			runStats->tbStats.numDeathsHIVNeg[tbStrain]++;
		}
	}
	// HIV-positive patients only
	else {
		runStats->deathStats.numDeathsCD4Type[patient->diseaseState.currTrueCD4Strata][patient->diseaseState.causeOfDeath]++;
		runStats->deathStats.numDeathsHVLCD4[patient->diseaseState.currTrueHVLStrata][patient->diseaseState.currTrueCD4Strata]++;

		if (patient->monitoringState.currLTFUState == SimContext::LTFU_STATE_LOST) {
			runStats->ltfuStats.numDeathsWhileLostCD4[patient->diseaseState.currTrueCD4Strata]++;
			if (patient->monitoringState.wasOnARTWhenLostToFollowUp) {
				runStats->ltfuStats.numDeathsWhileLostART[patient->artState.prevRegimenNum]++;
			}
			else if (!patient->artState.hasTakenART) {
				runStats->ltfuStats.numDeathsWhileLostPreART++;
			}
			else {
				runStats->ltfuStats.numDeathsWhileLostPostART++;
			}
		}
		if (causeOfDeath == SimContext::DTH_HIV) {
			if (patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N)
				runStats->deathStats.numHIVDeathsNoOIHistoryCD4[patient->diseaseState.currTrueCD4Strata]++;
			else
				runStats->deathStats.numHIVDeathsOIHistoryCD4[patient->diseaseState.currTrueCD4Strata]++;
		}
		else if (causeOfDeath == SimContext::DTH_BKGD_MORT) {
			if (patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N)
				runStats->deathStats.numBackgroundMortDeathsNoOIHistoryCD4[patient->diseaseState.currTrueCD4Strata]++;
			else
				runStats->deathStats.numBackgroundMortDeathsOIHistoryCD4[patient->diseaseState.currTrueCD4Strata]++;
		}
		else if (causeOfDeath == SimContext::DTH_TOX_ART) {
			int currRegimen = patient->artState.currRegimenNum;
			SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
			SimContext::CD4_STRATA cd4Strata=patient->diseaseState.currTrueCD4Strata;

			runStats->deathStats.numARTToxDeaths++;
			runStats->deathStats.numARTToxDeathsCD4Metric[patient->pedsState.ageCategoryCD4Metric]++;
			if(patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE){
				runStats->deathStats.ARTToxDeathsCD4Sum+=patient->diseaseState.currTrueCD4;
				runStats->deathStats.ARTToxDeathsCD4SumSquares+=patient->diseaseState.currTrueCD4*patient->diseaseState.currTrueCD4;
			}
			runStats->artStats.numToxicityDeaths[currRegimen][hvlStrata]++;
			runStats->deathStats.numARTToxDeathsCD4HVL[cd4Strata][hvlStrata]++;
			runStats->deathStats.numARTToxDeathsCD4[cd4Strata]++;

			for (int i=0;i<SimContext::OI_NUM;i++){
				if(patient->diseaseState.hasTrueOIHistory[i]){
					runStats->deathStats.numARTToxDeathsCD4HVLOIHist[cd4Strata][hvlStrata][i]++;
				}
			}
		}
		else if (causeOfDeath == SimContext::DTH_ACTIVE_TB) {
			SimContext::TB_STRAIN tbStrain = patient->tbState.currTrueTBResistanceStrain;
			runStats->tbStats.numDeathsHIVPos[tbStrain]++;
		}
		else if ((causeOfDeath >= SimContext::DTH_CHRM_1) && (causeOfDeath < SimContext::DTH_CHRM_1 + SimContext::CHRM_NUM)) {
			runStats->chrmsStats.numDeathsCHRMCD4[causeOfDeath - SimContext::DTH_CHRM_1][patient->diseaseState.currTrueCD4Strata]++;
		}
	} //end if patient is HIV positive

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Death Events
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numDeaths++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numDeaths++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numDeaths++;
		}
	}

	// update the longitudinal death statistics if requested, will be NULL if not
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		// Start with causes of death that apply to all patients regardless of HIV status
		currTime->numDeaths++;
		currTime->numDeathsAgeBracket[ageBracketOutput]++;
		currTime->numDeathsType[patient->diseaseState.causeOfDeath]++;
		currTime->numDeathsTypeCare[patient->diseaseState.causeOfDeath][patient->monitoringState.careState]++;
		currTime->numDeathsCare[patient->monitoringState.careState]++;
		currTime->numDeathsAgeBracketCare[patient->monitoringState.careState][ageBracketOutput]++;

		// CHRMs death stats
		bool hasCHRM=false;
		RunStats::OrphanStats *currOrphanTime;
		if (simContext->getCHRMsInputs()->enableOrphans)
			currOrphanTime = getOrphanStatsForUpdate();
		for(int i=0;i<SimContext::CHRM_NUM;i++){
			if(patient->diseaseState.hasTrueCHRMs[i]){
				currTime->numDeathsWithCHRMsTypeCHRM[patient->diseaseState.causeOfDeath][i]++;
				currTime->numDeathsWithCHRMsCHRM[i]++;
				hasCHRM=true;
				if (simContext->getCHRMsInputs()->enableOrphans){
					int ageOrphanYears = (patient->generalState.monthNum - patient->diseaseState.monthOfCHRMsStageStart[i][0])/12;
					currOrphanTime->numOrphans++;
					if (ageOrphanYears >=0 && ageOrphanYears < SimContext::CHRM_ORPHANS_OUTPUT_AGE_CAT_NUM)
						currOrphanTime->numOrphansAge[ageOrphanYears]++;
				}
			}
		}
		if (!hasCHRM){
			currTime->numDeathsWithoutCHRMsType[patient->diseaseState.causeOfDeath]++;
			currTime->numDeathsWithoutCHRMs++;
		}
		
		// TB death stats
		if(simContext->getTBInputs()->enableTB){
			bool unfavorableIndex[SimContext::TB_NUM_UNFAVORABLE];
			for (int i = 0; i < SimContext::TB_NUM_UNFAVORABLE; i++)
				unfavorableIndex[i] = patient->tbState.hasUnfavorableOutcome[i];

			currTime->numDeathsTBUnfavorableOutcome[unfavorableIndex[0]][unfavorableIndex[1]][unfavorableIndex[2]][unfavorableIndex[3]]++;

			if (patient->tbState.isOnTreatment || patient->tbState.isOnEmpiricTreatment){
				bool treatSuccess = false;
				if (patient->tbState.isOnTreatment)
					treatSuccess = patient->tbState.treatmentSuccess;
				if (patient->tbState.isOnEmpiricTreatment)
					treatSuccess = patient->tbState.empiricTreatmentSuccess;
				if (!treatSuccess){
					currTime->numAllDeathsWhileFailedTBTreatment++;
					if (causeOfDeath == SimContext::DTH_ACTIVE_TB)
						currTime->numDeathsTBWhileFailedTBTreatment++;
				}
			}

			if (causeOfDeath == SimContext::DTH_ACTIVE_TB){
				currTime->numDeathsTB++;

				if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG){
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM)
						currTime->numDeathsTBPulmHIVNegative++;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
						currTime->numDeathsTBExtraPulmHIVNegative++;
				}
				else{
					SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM)
						currTime->numDeathsTBPulmHIVPositive[cd4Strata]++;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
						currTime->numDeathsTBExtraPulmHIVPositive[cd4Strata]++;
				}

				if (patient->tbState.careState == SimContext::TB_CARE_LTFU){
					if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
						currTime->numDeathsTBLTFUHIVNegative++;
					else{
						SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
						currTime->numDeathsTBLTFUHIVPositive[cd4Strata]++;
					}
				}
			}
		} // end if TB module is enabled

		// HIV-positive patients
		if (patient->monitoringState.careState != SimContext::HIV_CARE_NEG){
			currTime->numDeathsPositive++;
			currTime->numDeathsAgeBracketHIVPositive[ageBracketOutput]++;
			for(int oiType = 0; oiType < SimContext::OI_NUM; oiType++){
				if((patient->diseaseState.causeOfDeath == (SimContext::DTH_CAUSES) oiType) && (patient->diseaseState.firstOIType == oiType) && (patient->generalState.monthNum == patient->diseaseState.monthOfFirstOI)){
					currTime->numDeathsFromFirstOI[oiType]++;
					break;
				}
			}
			if (patient->monitoringState.hadPrevClinicVisit){				
				currTime->numDeathsHIVPosHadClinicVisit++;
			}
			else {
				currTime->numDeathsHIVPosNeverVisitedClinic++;
			}			
			if (patient->monitoringState.careState == SimContext::HIV_CARE_IN_CARE || patient->monitoringState.careState == SimContext::HIV_CARE_RTC){
				if (patient->artState.isOnART){
					currTime->totalDeathsOnART++;
					currTime->numDeathsOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy]++;
					currTime->numDeathsAgeBracketOnART[ageBracketOutput]++;
				}
				else{
					currTime->numDeathsInCareOffART++;
					currTime->numDeathsAgeBracketInCareOffART[ageBracketOutput]++;
				}
			}
			if (patient->monitoringState.currLTFUState == SimContext::LTFU_STATE_LOST) {
				if (patient->monitoringState.wasOnARTWhenLostToFollowUp) {
					currTime->numDeathsWhileLostART[patient->artState.prevRegimenNum]++;
				}
				else if (!patient->artState.hasTakenART) {
					currTime->numDeathsWhileLostPreART++;
				}
				else {
					currTime->numDeathsWhileLostPostART++;
				}
			}
		} // end if patient is HIV positive
		else{
			currTime->numDeathsUninfected++;
		}
	}
} /* end setCauseOfDeath */

/** \brief setMaternalDeath updates the patient state for pediatric maternal death
 *
 *	Breast feeding is also stopped here: If mom is dead, the infant can't be breastfeeding anymore!
 **/
void StateUpdater::setMaternalDeath() {
	patient->pedsState.isMotherAlive = false;
	//If mom is dead, the infant can't be breastfeeding anymore!
	patient->pedsState.breastfeedingStatus = SimContext::PEDS_BF_REPL;
	patient->pedsState.monthOfMaternalDeath = patient->generalState.monthNum;
} /* end setMaternalDeath */

/** \brief incrementCohortSize increments the total population size by one
 *
 * This is done at patient creation instead of patient death (with the other population statistics)
 * for the sake of the transmission model: since the transmission model runs multiple patients at once,
 * waiting until the end to increment the cohort size results in all persons having a patientNum of 1
 */
void StateUpdater::incrementCohortSize(){
	runStats->popSummary.numCohorts++;
	costStats->popSummary.numPatients++;
}

/** \brief updatePopulationStats updates the final population summary after a death occurs
 *
 * All of the patient's running lifetime totals (such as cost and life months) are added to the population totals here
 **/
void StateUpdater::updatePopulationStats() {
	// Increment the number of cohorts and HIV positive cohorts
	//The total population is now incremented at patient creation
	//runStats->popSummary.numCohorts++ is in incrementCohortSize();
	if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {
		runStats->popSummary.numCohortsHIVPositive++;
		costStats->popSummary.numPatientsHIVPositive++;
	}

	//Update general statistics for cost file
	if (patient->monitoringState.hadPrevClinicVisit)
		costStats->popSummary.numInCare++;
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++){
		if (patient->artState.hasTakenARTRegimen[i])
			costStats->popSummary.numEverOnART[i]++;
	}

	// Update sums for costs and life months of all patients
	runStats->popSummary.costsSum += patient->generalState.costsDiscounted;
	runStats->popSummary.costsSumSquares += patient->generalState.costsDiscounted * patient->generalState.costsDiscounted;
	runStats->popSummary.LMsSum += patient->generalState.LMsDiscounted;
	runStats->popSummary.LMsSumSquares += patient->generalState.LMsDiscounted * patient->generalState.LMsDiscounted;
	runStats->popSummary.QALMsSum += patient->generalState.qualityAdjustLMsDiscounted;
	runStats->popSummary.QALMsSumSquares += patient->generalState.qualityAdjustLMsDiscounted * patient->generalState.qualityAdjustLMsDiscounted;

	if (patient->pedsState.isFalsePositive){
		runStats->hivScreening.LMsFalsePositive += patient->generalState.LMsDiscounted;
		if(patient->pedsState.isFalsePositiveLinked){
			runStats->hivScreening.LMsFalsePositiveLinked += patient->generalState.LMsDiscounted;
		}
	}


	//Add monthly cumulative costs
	double cumulCohortCosts= 0;
	double cumulCohortCostsType[SimContext::COST_NUM_TYPES];
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		cumulCohortCostsType[i] = 0;
	}
	double cumulARTCosts = 0;
	double cumulCD4Costs = 0;
	double cumulHVLCosts = 0;
	double cumulHIVTestCosts = 0;
	double cumulHIVMiscCosts = 0;

	for (vector<RunStats::TimeSummary *>::iterator t = runStats->timeSummaries.begin(); t != runStats->timeSummaries.end(); t++) {
		RunStats::TimeSummary *currTime = *t;
		cumulCohortCosts += currTime->totalMonthlyCohortCosts;
		for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
			cumulCohortCostsType[i] += currTime->totalMonthlyCohortCostsType[i];
		}

		for (int i = 0; i < SimContext::ART_NUM_LINES; i++)
			cumulARTCosts += currTime->costsART[i];
		cumulCD4Costs += currTime->costsCD4Testing;
		cumulHVLCosts += currTime->costsHVLTesting;
		cumulHIVTestCosts += currTime->costsHIVTests;
		cumulHIVMiscCosts += currTime->costsHIVMisc;
		currTime->cumulativeCohortCosts = cumulCohortCosts;
		for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
			currTime->cumulativeCohortCostsType[i] = cumulCohortCostsType[i];
		}
		currTime->cumulativeARTCosts = cumulARTCosts;
		currTime->cumulativeCD4TestingCosts = cumulCD4Costs;
		currTime->cumulativeHVLTestingCosts = cumulHVLCosts;
		currTime->cumulativeHIVTestingCosts = cumulHIVTestCosts;
		currTime->cumulativeHIVMiscCosts = cumulHIVMiscCosts;
	}

	// Add cumulative HIV detection and testing numbers
	int cumulHIVDetections[SimContext::HIV_DET_NUM];
	for (int i = 0; i < SimContext::HIV_DET_NUM; i++) {
		cumulHIVDetections[i] = 0;
	}
	int cumulHIVTests = 0;
	int cumulHIVTestsAtInitOffer = 0;
	int cumulHIVTestsPostStartup = 0;
		
	for (vector<RunStats::TimeSummary *>::iterator t = runStats->timeSummaries.begin(); t != runStats->timeSummaries.end(); t++) {
		RunStats::TimeSummary *currTime = *t;

		for (int i = 0; i < SimContext::HIV_DET_NUM; i++) {
			cumulHIVDetections[i] += currTime->numHIVDetections[i];
		}	
		cumulHIVTests += currTime->numHIVTestsPerformed;
		cumulHIVTestsAtInitOffer += currTime->numHIVTestsPerformedAtInitOffer;
		cumulHIVTestsPostStartup += currTime->numHIVTestsPerformedPostStartup;
			
		for (int i = 0; i < SimContext::HIV_DET_NUM; i++) {
			currTime->cumulativeNumHIVDetections[i] = cumulHIVDetections[i];
		}	
		currTime->cumulativeNumHIVTests = cumulHIVTests;
		currTime->cumulativeNumHIVTestsAtInitOffer = cumulHIVTestsAtInitOffer;
		currTime->cumulativeNumHIVTestsPostStartup = cumulHIVTestsPostStartup;
	}
	if(simContext->getOutputInputs()->enableSubCohorts){
		for (int i = 0; i < SimContext::MAX_NUM_SUBCOHORTS; i++){
			if (simContext->getOutputInputs()->subCohorts[i]<=0)
				break;
			if (patient->generalState.patientNum <= simContext->getOutputInputs()->subCohorts[i]){
				runStats->popSummary.costsSumCohortParsing[i] += patient->generalState.costsDiscounted;
				runStats->popSummary.costsSumSquaresCohortParsing[i] += patient->generalState.costsDiscounted * patient->generalState.costsDiscounted;
				runStats->popSummary.LMsSumCohortParsing[i] += patient->generalState.LMsDiscounted;
				runStats->popSummary.LMsSumSquaresCohortParsing[i] += patient->generalState.LMsDiscounted * patient->generalState.LMsDiscounted;
				runStats->popSummary.QALMsSumCohortParsing[i] += patient->generalState.qualityAdjustLMsDiscounted;
				runStats->popSummary.QALMsSumSquaresCohortParsing[i] += patient->generalState.qualityAdjustLMsDiscounted * patient->generalState.qualityAdjustLMsDiscounted;
			}
		}
	}

	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		runStats->popSummary.multDiscCostsSum[i] += patient->generalState.multDiscCosts[i];
		runStats->popSummary.multDiscCostsSumSquares[i] += patient->generalState.multDiscCosts[i] * patient->generalState.multDiscCosts[i];
		runStats->popSummary.multDiscLMsSum[i] += patient->generalState.multDiscLMs[i];
		runStats->popSummary.multDiscLMsSumSquares[i] += patient->generalState.multDiscLMs[i] * patient->generalState.multDiscLMs[i];
		runStats->popSummary.multDiscQALMsSum[i] += patient->generalState.multDiscQALMs[i];
		runStats->popSummary.multDiscQALMsSumSquares[i] += patient->generalState.multDiscQALMs[i] * patient->generalState.multDiscQALMs[i];
	}
	// Update sums for costs and life months of patients with X number of ART failures
	for (int i = 0; i <= SimContext::ART_NUM_LINES; i++) {
		if (i >= patient->artState.numObservedFailures) {
			runStats->popSummary.numFailART[i]++;
			runStats->popSummary.costsFailARTSum[i] += patient->generalState.costsDiscounted;
			runStats->popSummary.LMsFailARTSum[i] += patient->generalState.LMsDiscounted;
			runStats->popSummary.QALMsFailARTSum[i] += patient->generalState.qualityAdjustLMsDiscounted;
		}
	}

	// Update sums for costs and life months of HIV positive patients
	if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {
		runStats->popSummary.costsHIVPositiveSum += patient->generalState.costsDiscounted;
		runStats->popSummary.LMsHIVPositiveSum += patient->generalState.LMsDiscounted;
		runStats->popSummary.QALMsHIVPositiveSum += patient->generalState.qualityAdjustLMsDiscounted;
		// If patient dies HIV positive and undetected, update the associated HIV screening stats
		if (!patient->monitoringState.isDetectedHIVPositive) {
			if (patient->diseaseState.isPrevalentHIVCase)
				runStats->hivScreening.numDetectedPrevalentMeans[SimContext::HIV_DET_UNDETECTED]++;
			else
				runStats->hivScreening.numDetectedIncidentMeans[SimContext::HIV_DET_UNDETECTED]++;
		}
		if(!patient->monitoringState.isLinked){
			runStats->hivScreening.numLinkedMeans[SimContext::HIV_DET_UNDETECTED]++;
		}
	}
	else{
		if(patient->monitoringState.everPrEP){
			runStats->hivScreening.numNeverHIVPositive[SimContext::EVER_PREP]++;
			runStats->hivScreening.LMsHIVNegativeSum[SimContext::EVER_PREP] += patient->generalState.LMsDiscounted;
			runStats->hivScreening.QALMsHIVNegativeSum[SimContext::EVER_PREP] += patient->generalState.qualityAdjustLMsDiscounted;
		}
		else{
			runStats->hivScreening.numNeverHIVPositive[SimContext::NEVER_PREP]++;
			runStats->hivScreening.LMsHIVNegativeSum[SimContext::NEVER_PREP] += patient->generalState.LMsDiscounted;
			runStats->hivScreening.QALMsHIVNegativeSum[SimContext::NEVER_PREP] += patient->generalState.qualityAdjustLMsDiscounted;
		}
	}

	//TB unfavorable outcome
	if(simContext->getTBInputs()->enableTB){
		bool unfavorableIndex[SimContext::TB_NUM_UNFAVORABLE];
		for (int i = 0; i < SimContext::TB_NUM_UNFAVORABLE; i++)
			unfavorableIndex[i] = patient->tbState.hasUnfavorableOutcome[i];

		runStats->tbStats.numWithUnfavorableOutcome[unfavorableIndex[0]][unfavorableIndex[1]][unfavorableIndex[2]][unfavorableIndex[3]]++;
	}

} /* end updatePopulationStats */

/** \brief AddPatientSummary creates a patient summary object and adds it to the patients vector
 *
 * Only HIV positive patients up to 1,000,000 are added
 **/
void StateUpdater::addPatientSummary() {
	// only include the HIV positive patients up to 1,000,000
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
		return;
	if (patient->generalState.patientNum > 1000000)
		return;

	// add a new PatientSummary to the vector for this patient
	RunStats::PatientSummary patientSummary;
	patientSummary.costs = patient->generalState.costsDiscounted;
	patientSummary.LMs = patient->generalState.LMsDiscounted;
	patientSummary.QALMs = patient->generalState.qualityAdjustLMsDiscounted;
	runStats->patients.push_back(patientSummary);
} /* end addPatientSummary */

/** \brief setTrueCD4 updates the patients actual CD4 level and confines it within the bounds of the envelope
 *
 * Also checks if it is the patients minimum CD4 value
 **/
void StateUpdater::setTrueCD4(double newCD4, bool isInitial) {
	// make sure CD4 does not exceed upper and lower bounds, and CD4 envelopes if set
	patient->diseaseState.currTrueCD4 = newCD4;
	if (patient->diseaseState.currTrueCD4 < 0.0) {
		patient->diseaseState.currTrueCD4 = 0.0;
	}
	else if ((simContext->getRunSpecsInputs()->maxPatientCD4 != SimContext::NOT_APPL) &&
		(patient->diseaseState.currTrueCD4 > simContext->getRunSpecsInputs()->maxPatientCD4)) {
			patient->diseaseState.currTrueCD4 = simContext->getRunSpecsInputs()->maxPatientCD4;
	}
	// the CD4 envelopes are only activated if the CD4 envelope is enabled in RunSpecs F10, after an initial success on ART - see ClinicVisitUpdater::performARTProgramUpdates()
	else {
		if (!isInitial && patient->artState.overallCD4Envelope.isActive &&
		(patient->diseaseState.currTrueCD4 > patient->artState.overallCD4Envelope.value)) {
			patient->diseaseState.currTrueCD4 = patient->artState.overallCD4Envelope.value;
		}
		if (!isInitial && patient->artState.indivCD4Envelope.isActive &&
		(patient->diseaseState.currTrueCD4 > patient->artState.indivCD4Envelope.value)) {
			patient->diseaseState.currTrueCD4 = patient->artState.indivCD4Envelope.value;
		}
	}

	// update the true CD4 strata
	patient->diseaseState.currTrueCD4Strata = getCD4Strata(patient->diseaseState.currTrueCD4);

	// update the minimum true CD4 if needed
	if (isInitial || (patient->diseaseState.currTrueCD4 < patient->diseaseState.minTrueCD4)) {
		patient->diseaseState.minTrueCD4 = patient->diseaseState.currTrueCD4;
		patient->diseaseState.minTrueCD4Strata = patient->diseaseState.currTrueCD4Strata;
	}
} /* end setTrueCD4 */

/** \brief setTrueCD4Percentage updates the pediatrics patients actual CD4 percentage
 * \param newCD4Perc a double (which should be between 0 and 1 inclusive) that represents the new CD4 percentage
 * \param isInitial a boolean that is true if this is the first time the CD4 percentage is being set
 *
 * This function also checks if newCD4Perc is the patient's minimum CD4 percentage
 **/
void StateUpdater::setTrueCD4Percentage(double newCD4Perc, bool isInitial) {
	if(isInitial){
		// initialize the pediatrics patient's adult true CD4 value to -1 for Not Applicable as a debugging measure - it will be assigned a value at age 5 when they transition to late childhood
		patient->diseaseState.currTrueCD4 = -1.0;
	}	
	// make sure CD4 percentage is within 0 and 1, and below the maximum CD4 value
	double maxCD4Perc = simContext->getPedsInputs()->maxCD4Percentage[patient->pedsState.ageCategoryPediatrics];
	patient->diseaseState.currTrueCD4Percentage = newCD4Perc;
	if (patient->diseaseState.currTrueCD4Percentage < 0)
		patient->diseaseState.currTrueCD4Percentage = 0;
	else if (patient->diseaseState.currTrueCD4Percentage > maxCD4Perc)
		patient->diseaseState.currTrueCD4Percentage = maxCD4Perc;
	else if (patient->diseaseState.currTrueCD4Percentage > 1)
		patient->diseaseState.currTrueCD4Percentage = 1;
	// the CD4 percentage envelopes are only activated if the CD4 envelope is enabled in RunSpecs F10, after an initial success on ART - see ClinicVisitUpdater::performARTProgramUpdates()	
	else {
		if (!isInitial && patient->artState.overallCD4PercentageEnvelope.isActive &&
				(patient->diseaseState.currTrueCD4Percentage > patient->artState.overallCD4PercentageEnvelope.value)) {
			patient->diseaseState.currTrueCD4Percentage = patient->artState.overallCD4PercentageEnvelope.value;
		}
		if (!isInitial && patient->artState.indivCD4PercentageEnvelope.isActive &&
				(patient->diseaseState.currTrueCD4Percentage > patient->artState.indivCD4PercentageEnvelope.value)) {
			patient->diseaseState.currTrueCD4Percentage = patient->artState.indivCD4PercentageEnvelope.value;
		}
	}

	// update the true CD4 percentage strata
	patient->diseaseState.currTrueCD4PercentageStrata = getCD4PercentageStrata(patient->diseaseState.currTrueCD4Percentage);

	// update the minimum true CD4 percentage if needed
	if (isInitial || (patient->diseaseState.currTrueCD4Percentage < patient->diseaseState.minTrueCD4Percentage)) {
		patient->diseaseState.minTrueCD4Percentage = patient->diseaseState.currTrueCD4Percentage;
	}

	// update the corresponding adult true CD4 strata for pediatrics patients,
	//	set minimum if this is the initial or below the previous minimum CD4 strata
	patient->diseaseState.currTrueCD4Strata = simContext->getPedsInputs()->adultCD4Strata[patient->pedsState.ageCategoryPediatrics][patient->diseaseState.currTrueCD4PercentageStrata];
	if (isInitial || (patient->diseaseState.currTrueCD4Strata < patient->diseaseState.minTrueCD4Strata)) {
		patient->diseaseState.minTrueCD4Strata = patient->diseaseState.currTrueCD4Strata;
	}
} /* setTrueCD4Percentage */

/** \brief setCD4MultOnARTFail updates the cd4 decline mutiplier on failed ART
 * \param monthOfNewCD4Decline an int that represents the next month in which a new lag period can be set
 * \param newCD4Mult a double setting the current cd4 decline multiplier on failed ART
 *
 **/
void StateUpdater::setCD4MultOnARTFail(int monthOfNewCD4Decline,double newCD4Mult){
	patient->artState.monthOfNewCD4MultArtFail=monthOfNewCD4Decline;
	patient->artState.currCD4MultArtFail=newCD4Mult;
}

/** \brief setTrueHVLStrata set the patient's actual HVL strata to the given level
 *
 * \param newHVL a SimContext::HVL_STRATA indicating the new HVL strata to be set
 **/
void StateUpdater::setTrueHVLStrata(SimContext::HVL_STRATA newHVL) {
	// update the HVL strata
	patient->diseaseState.currTrueHVLStrata = newHVL;
} /* end setTrueHVLStrata */

/** \brief setSetpointHVLStrata sets the patients setpoint HVL level
 *
 * \param newSetpoint a SimContext::HVL_STRATA indicating the new HVL setpoint
 **/
void StateUpdater::setSetpointHVLStrata(SimContext::HVL_STRATA newSetpoint) {
	// update the HVL setpoint strata
	patient->diseaseState.setpointHVLStrata = newSetpoint;
} /* end setSetpointHVLStrata */

/** \brief setObservedCD4 updates the patients observed CD4 level and confines it within the bounds
 *
 * \param isKnown a boolean that is true if there is an observed CD4 value; if isKnown is false, the patient has no observed CD4
 * \param cd4Value a double representing the new observed cd4Value (this is only in effect if isKnown is true)
 *
 * The function also checks if cd4Value is the new minimum or maximum observed cd4 value
 **/
void StateUpdater::setObservedCD4(bool isKnown, double cd4Value, bool isLabStaging) {
	// If it is unknown (for untested patients), set the CD4 to unknown and return
	if (!isKnown) {
		patient->monitoringState.hasObservedCD4 = false;
		patient->monitoringState.hasObservedCD4NonLabStaging = false;
		return;
	}

	// make sure observed CD4 is not below zero
	patient->monitoringState.monthOfObservedCD4 = patient->generalState.monthNum;
	if(!isLabStaging)
		patient->monitoringState.monthOfObservedCD4NonLabStaging = patient->generalState.monthNum;
	patient->monitoringState.currObservedCD4 = cd4Value;
	if (patient->monitoringState.currObservedCD4 < 0.0) {
		patient->monitoringState.currObservedCD4 = 0.0;
	}

	// update the observed CD4 strata
	patient->monitoringState.currObservedCD4Strata = getCD4Strata(patient->monitoringState.currObservedCD4);

	// set the min and max observed CD4s if needed
	if (!patient->monitoringState.hasObservedCD4 ||
		(patient->monitoringState.currObservedCD4 < patient->monitoringState.minObservedCD4)) {
			patient->monitoringState.minObservedCD4 = patient->monitoringState.currObservedCD4;
	}
	if (patient->artState.isOnART) {
		if (!patient->monitoringState.hasObservedCD4 ||
			(patient->monitoringState.currObservedCD4 > patient->artState.maxObservedCD4OnCurrART)) {
				patient->artState.maxObservedCD4OnCurrART = patient->monitoringState.currObservedCD4;
		}
	}

	// set that the patient now has an observed CD4
	if (!patient->monitoringState.hasObservedCD4)
		patient->monitoringState.hasObservedCD4 = true;
	if (!isLabStaging)
		patient->monitoringState.hasObservedCD4NonLabStaging = true;
} /* end setObservedCD4 */

/** \brief setObservedCD4Percentage updates the patients observed CD4 percentage and confines it within the bounds
 *
 * \param isKnown a boolean that is true if there is an observed CD4 percentage; if isKnown is false, the patient has no observed CD4 percentage
 * \param cd4Percent a double (that should be between 0 and 1 inclusive) representing the new observed cd4 percentage (this is only in effect if isKnown is true)
 *
 * The function also checks if cd4Percent is the new minimum or maximum observed cd4 percentage
 **/
void StateUpdater::setObservedCD4Percentage(bool isKnown, double cd4Percent) {
	// If it is unknown (for untested patients), set the CD4 to unknown and return
	if (!isKnown) {
		patient->monitoringState.hasObservedCD4Percentage = false;
		return;
	}

	// Make sure the observed value is between 0 and 1
	patient->monitoringState.monthOfObservedCD4Percentage = patient->generalState.monthNum;
	patient->monitoringState.currObservedCD4Percentage = cd4Percent;
	if (patient->monitoringState.currObservedCD4Percentage < 0)
		patient->monitoringState.currObservedCD4Percentage = 0;
	else if (patient->monitoringState.currObservedCD4Percentage > 1)
		patient->monitoringState.currObservedCD4Percentage = 1;

	// update the observed CD4 percentage strata
	patient->monitoringState.currObservedCD4PercentageStrata = getCD4PercentageStrata(patient->monitoringState.currObservedCD4Percentage);

	// set the min and max observed CD4s if needed
	if (!patient->monitoringState.hasObservedCD4Percentage ||
		(patient->monitoringState.currObservedCD4Percentage < patient->monitoringState.minObservedCD4Percentage)) {
			patient->monitoringState.minObservedCD4Percentage = patient->monitoringState.currObservedCD4Percentage;
	}
	if (patient->artState.isOnART) {
		if (!patient->monitoringState.hasObservedCD4Percentage ||
			(patient->monitoringState.currObservedCD4Percentage > patient->artState.maxObservedCD4PercentageOnCurrART)) {
				patient->artState.maxObservedCD4PercentageOnCurrART = patient->monitoringState.currObservedCD4Percentage;
		}
	}

	// set that the patient now has an observed CD4 percentage
	if (!patient->monitoringState.hasObservedCD4Percentage)
		patient->monitoringState.hasObservedCD4Percentage = true;
} /* end setObservedCD4Percentage */

/** \brief setObservedHVLStrata updates the patients observed HVL strata
 *
 * \param isKnown a boolean that is true if there is an observed HVL strata; if isKnown is false, the patient has no observed HVL
 * \param hvlStrata a SimContext::HVL_STRATA representing the new observed hvlStrata (this is only in effect if isKnown is true)
 *
 * The function also checks if hvlStrata is the new minimum or maximum observed hvl
 **/
void StateUpdater::setObservedHVLStrata(bool isKnown, SimContext::HVL_STRATA hvlStrata) {
	// If value is unobserved (for untest patients) set the state and return
	if (!isKnown) {
		patient->monitoringState.hasObservedHVLStrata = false;
		return;
	}

	// Update the observed HVL strata
	patient->monitoringState.monthOfObservedHVLStrata = patient->generalState.monthNum;
	patient->monitoringState.currObservedHVLStrata = hvlStrata;

	// set the maximum observed HVL, used as a proxy for the HVL setpoint
	if (!patient->monitoringState.hasObservedHVLStrata ||
		(patient->monitoringState.currObservedHVLStrata > patient->monitoringState.maxObservedHVLStrata)) {
			patient->monitoringState.maxObservedHVLStrata = patient->monitoringState.currObservedHVLStrata;
	}

	// set the initial or minimum observed HVL on ART
	if (patient->artState.isOnART) {
		if (patient->artState.monthOfCurrRegimenStart == patient->generalState.monthNum)
			patient->artState.observedHVLStrataAtRegimenStart = patient->monitoringState.currObservedHVLStrata;
		if (!patient->monitoringState.hasObservedHVLStrata ||
			(patient->monitoringState.currObservedHVLStrata < patient->artState.minObservedHVLStrataOnCurrART)) {
				patient->artState.minObservedHVLStrataOnCurrART = patient->monitoringState.currObservedHVLStrata;
		}
	}

	// set that the patient now has an observed HVL
	if (!patient->monitoringState.hasObservedHVLStrata) {
		patient->monitoringState.hasObservedHVLStrata = true;
	}
} /* end setObservedHVLStrata */

/** \brief incrementNumCD4Tests increases the number of CD4 tests given by 1
 *
 **/
void StateUpdater::incrementNumCD4Tests() {
	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Num CD4 Tests
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numCD4Tests++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numCD4Tests++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numCD4Tests++;
		}
	}

} /* end incrementNumCD4Tests */

/** \brief incrementNumHVLTests increases the number of HVL tests given by 1
 *
 **/
void StateUpdater::incrementNumHVLTests() {
	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Num HVL Tests
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numHVLTests++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numHVLTests++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numHVLTests++;
		}
	}

} /* end incrementNumHVLTests */


/** \brief incrementNumHIVTests increases the number of HIV tests given by 1
 *
 **/
void StateUpdater::incrementNumHIVTests() {
	// Increment current number of tests performed during and after Month 0, to be used for cumulative outputs 
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if(currTime){
		currTime->numHIVTestsPerformed++;
		if((patient->generalState.monthNum == patient->generalState.initialMonthNum) || (patient->generalState.ageMonths == simContext->getHIVTestInputs()->HIVRegularTestingStartAge)){
			currTime->numHIVTestsPerformedAtInitOffer++;
		}
		else{
			currTime->numHIVTestsPerformedPostStartup++;
		}
	}	
	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Num HVL Tests
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numHIVTests++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numHIVTests++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numHIVTests++;
		}
	}

} /* end incrementNumHIVTests */

/** \brief incrementNumLabStagingTests increases the number of Lab Staging tests given by 1
 *
 **/
void StateUpdater::incrementNumLabStagingTests() {
	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Num HVL Tests
		if (patient->generalState.costSubgroups[i]){
			costStats->eventStats[SimContext::COST_CD4_ALL][i].numLabStagingTests++;

			if (patient->monitoringState.hasObservedCD4)
				costStats->eventStats[patient->monitoringState.currObservedCD4Strata][i].numLabStagingTests++;
			else
				costStats->eventStats[SimContext::COST_CD4_NONE][i].numLabStagingTests++;
		}
	}

} /* end incrementNumLabStagingTests */

/** \brief incrementCostsPrEP adds a PrEP cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 * \param percent a double representing the percent of the cost to apply - currently we are always applying the full PrEP cost because it is a drug prescription, but this may be updated later
 *
 * The PrEP costs will be discounted. 
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsPrEP(double cost, double percent) {
	runStats->overallCosts.costsPrEP += cost * patient->generalState.discountFactor;
	patient->monitoringState.costsPrEP += cost * patient->generalState.discountFactor;

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsPrEP += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, percent);
} /* end incrementCostsPrEP */

/** \brief finalizePrEPCostsByState adds the final total for PrEP costs to the overall total for the patient's PrEP state 
 * \param isPrEPDropout a bool indicating whether the patient is a PrEP dropout, used to count costs for those infected with HIV after dropping out 
 * 
 **/
void StateUpdater::finalizePrEPCostsByState(bool isPrEPDropout){
	if(patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG){
		runStats->overallCosts.costsPrEPNeverHIV += patient->monitoringState.costsPrEP;
	}
	else if(isPrEPDropout){
		runStats->overallCosts.costsPrEPHIVPos[SimContext::HIV_POS_PREP_DROPOUT] += patient->monitoringState.costsPrEP;
	}	
	else {
		runStats->overallCosts.costsPrEPHIVPos[SimContext::HIV_POS_ON_PREP] += patient->monitoringState.costsPrEP;
	}
} /* end finalizePrEPCostsByState */

/** \brief incrementCostsHIVTest adds an HIV testing cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 *
 * Both discounted and undiscounted costs will be calculated. 
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsHIVTest(double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;

	runStats->overallCosts.costsHIVScreeningTests += discountedCost;
	patient->monitoringState.costsHIVTesting += discountedCost;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//HIV Screening Cost
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsHIVScreeningTests += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsHIVScreeningTests += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHIVScreeningTests += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHIVScreeningTests += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsHIVScreeningTests += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsHIVScreeningTests += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsHIVTests += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);


} /* end incrementCostsHIVTest */

/** \brief incrementCostsHIVMisc adds an HIV misc related cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsHIVMisc(double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;

	runStats->overallCosts.costsHIVScreeningMisc += discountedCost;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//HIV Screening Cost
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsHIVScreeningMisc += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsHIVScreeningMisc += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHIVScreeningMisc += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHIVScreeningMisc += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsHIVScreeningMisc += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsHIVScreeningMisc += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsHIVMisc += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsHIVTest */

/** \brief incrementCostsLabStagingTest adds a Lab Staging testing cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsLabStagingTest(double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;

	runStats->overallCosts.costsLabStagingTests += discountedCost;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Lab Staging Cost
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsLabStagingTests += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsLabStagingTests += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsLabStagingTests += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsLabStagingTests += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsLabStagingTests += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsLabStagingTests += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsLabStagingTests += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsLabStagingTest */

/** \brief incrementCostsEIDTest adds an EID testing cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsEIDTest(double cost) {
	runStats->overallCosts.costsEIDTests += cost * patient->generalState.discountFactor;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsEIDTests += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsEIDTest */

/** \brief incrementCostsInfantHIVProphDirect adds a Infant HIV Proph cost to the patient's total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsInfantHIVProphDirect(double cost) {
	runStats->overallCosts.costsInfantHIVProph += cost * patient->generalState.discountFactor;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsInfantHIVProphDirect += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsInfantHIVProphDirect */

/** \brief incrementCostsInfantHIVProphTox adds a Infant HIV Proph cost to the patient's total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsInfantHIVProphTox(double cost) {
	runStats->overallCosts.costsInfantHIVProph += cost * patient->generalState.discountFactor;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsInfantHIVProphTox += cost * patient->generalState.discountFactor;
	}
	incrementCostsToxicity(cost);
} /* end incrementCostsInfantHIVProphTox */

/** \brief incrementCostsLabStagingMisc adds a Lab Staging misc related cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsLabStagingMisc(double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;

	runStats->overallCosts.costsLabStagingMisc += discountedCost;

	//Update coststats variables
		for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
			//Lab Staging Cost
			if (patient->generalState.costSubgroups[i]){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsLabStagingMisc += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsLabStagingMisc += discountedCost;

				if (patient->monitoringState.hasObservedCD4){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsLabStagingMisc += undiscountedCost;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsLabStagingMisc += discountedCost;
				}
				else{
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsLabStagingMisc += undiscountedCost;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsLabStagingMisc += discountedCost;
				}
			}
		}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsLabStagingMisc += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsLabStagingMisc*/

/** \brief incrementCostsEIDMisc adds an EID misc cost to the patients total
 *
 * \param cost a double representing the cost to be incremented
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsEIDMisc(double cost) {
	runStats->overallCosts.costsEIDMisc += cost * patient->generalState.discountFactor;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsEIDMisc += cost * patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsEIDMisc */


/** \brief incrementCostsCD4Test adds the CD4 testing related costs to the patients total
 *
 * \param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(const double*, double)
 **/
void StateUpdater::incrementCostsCD4Test(const double *costArray) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	discountedCostTotal *= patient->generalState.discountFactor;
	runStats->overallCosts.costsCD4Testing += discountedCostTotal;
	patient->monitoringState.costsCD4Testing += discountedCostTotal;

	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double discountedCostMultDisc = undiscountedCostTotal * patient->generalState.multDiscFactorCost[i];
		runStats->overallCosts.costsCD4TestingMultDisc[i] += discountedCostMultDisc;
	}

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//CD4 Test Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsCD4Testing += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsCD4Testing += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsCD4TestingCategory[j] += costArray[j];
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsCD4TestingCategory[j] += costArray[j]*patient->generalState.discountFactor;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsCD4Testing += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsCD4Testing += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsCD4TestingCategory[j] += costArray[j];
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsCD4TestingCategory[j] += costArray[j]*patient->generalState.discountFactor;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsCD4Testing += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsCD4Testing += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsCD4TestingCategory[j] += costArray[j];
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsCD4TestingCategory[j] += costArray[j]*patient->generalState.discountFactor;
				}
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsCD4Testing += discountedCostTotal;
	}
	incrementCostsCommon(costArray, 1.0);
} /* end incrementCostsCD4Test */

/* \brief incrementCostsHVLTest adds the HVL testing related costs to the patients total
 *
 * \param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(const double*, double)
 **/
void StateUpdater::incrementCostsHVLTest(const double *costArray) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	discountedCostTotal *= patient->generalState.discountFactor;
	runStats->overallCosts.costsHVLTesting += discountedCostTotal;
	patient->monitoringState.costsHVLTesting += discountedCostTotal;

	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double discountedCostMultDisc = undiscountedCostTotal * patient->generalState.multDiscFactorCost[i];
		runStats->overallCosts.costsHVLTestingMultDisc[i] += discountedCostMultDisc;
	}

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//HVL Test Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsHVLTesting += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsHVLTesting += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsHVLTestingCategory[j] += costArray[j];
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsHVLTestingCategory[j] += costArray[j]*patient->generalState.discountFactor;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHVLTesting += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHVLTesting += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHVLTestingCategory[j] += costArray[j];
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsHVLTestingCategory[j] += costArray[j]*patient->generalState.discountFactor;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsHVLTesting += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsHVLTesting += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsHVLTestingCategory[j] += costArray[j];
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsHVLTestingCategory[j] += costArray[j]*patient->generalState.discountFactor;
				}
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsHVLTesting += discountedCostTotal;
	}
	incrementCostsCommon(costArray, 1.0);
} /* end incrementCostsHVLTest */

/** \brief incrementCostsClinicVisit adds the general clinic visit costs to the patients total
 *
 * \param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *
 * The costs will be discounted.
 *
 * \see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsClinicVisit(const double *costArray) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	discountedCostTotal *= patient->generalState.discountFactor;
	runStats->overallCosts.costsClinicVisits += discountedCostTotal;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Visit Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsClinicVisit += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsClinicVisit += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsClinicVisitCategory[j] += costArray[j];
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsClinicVisitCategory[j] += costArray[j]*patient->generalState.discountFactor;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsClinicVisit += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsClinicVisit += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsClinicVisitCategory[j] += costArray[j];
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsClinicVisitCategory[j] += costArray[j]*patient->generalState.discountFactor;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsClinicVisit += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsClinicVisit += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsClinicVisitCategory[j] += costArray[j];
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsClinicVisitCategory[j] += costArray[j]*patient->generalState.discountFactor;
				}
			}
		}
	}


	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsClinicVisits += discountedCostTotal;
	}
	incrementCostsCommon(costArray, 1.0);
} /* end incrementCostsClinicVisit */


/** \brief incrementCostsEIDVisit adds a EID visit cost to the patient's total
 *
 * \param cost a double representing the cost to be added
 *
 * The costs will be discounted.
 *
 * \see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsEIDVisit(double cost){
	double discountedCost = cost * patient->generalState.discountFactor;

	runStats->overallCosts.costsEIDVisits += discountedCost;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsEIDVisits += discountedCost;
	}
	incrementCostsCommon(cost, 1.0);
}



/** \brief incrementCostsART adds an ART treatment cost to the patients total
 *
 * \param artLineNum an integer representing the line of ART that the cost stems from
 * \param cost a double representing the cost to be added
 *
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsART(int artLineNum, double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;
	runStats->overallCosts.costsDrugs += undiscountedCost;
	runStats->overallCosts.costsDrugsDiscounted += discountedCost;
	runStats->overallCosts.directCostsART += discountedCost;
	runStats->overallCosts.directCostsARTLine[artLineNum] += discountedCost;
	patient->artState.costsART += discountedCost;
	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double discountedCostMultDisc = cost * patient->generalState.multDiscFactorCost[i];
		runStats->overallCosts.directCostsARTLineMultDisc[artLineNum][i] += discountedCostMultDisc;
	}

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//ART costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsART += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsART += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsART += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsART += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsART += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsART += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsART[artLineNum] += discountedCost;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsART */


/** \brief incrementCostsARTInit adds an initial ART treatment cost to the patients total
 *
 * \param artLineNum an integer representing the line of ART that the cost stems from
 * \param cost a double representing the cost to be added
 *
 * The cost will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsARTInit(int artLineNum, double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	runStats->overallCosts.costsARTInitLine[artLineNum] += discountedCost;

	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double discountedCostMultDisc = cost * patient->generalState.multDiscFactorCost[i];
		runStats->overallCosts.costsARTInitLineMultDisc[artLineNum][i] += discountedCostMultDisc;
	}
	incrementCostsART(artLineNum, cost);
} /* end incrementCostsARTInit */


/** \brief incrementCostsARTMonthly adds a monthly ART treatment cost to the patients total
 *
 * \param artLineNum an integer representing the line of ART that the cost stems from
 * \param cost a double representing the cost to be added
 *
 * The cost will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsARTMonthly(int artLineNum, double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	runStats->overallCosts.costsARTMonthlyLine[artLineNum] += discountedCost;
	for (int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double discountedCostMultDisc = cost * patient->generalState.multDiscFactorCost[i];
		runStats->overallCosts.costsARTMonthlyLineMultDisc[artLineNum][i] += discountedCostMultDisc;
	}
	incrementCostsART(artLineNum, cost);
} /* end incrementCostsARTMonthly */


/** \brief incrementCostsInterventionInit adds the init costs for starting an intervention
 * \param cost a double representing the cost to be added
 *
 * The cost will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsInterventionInit(double cost){
	double undiscountedCost = cost;
	double discountedCost = cost * patient->generalState.discountFactor;
	runStats->overallCosts.costsInterventionStartup += discountedCost;


	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsInterventionStartup += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsInterventionStartup += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsInterventionStartup += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsInterventionStartup += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsInterventionStartup += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsInterventionStartup += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsInterventionStartup += discountedCost;
	}

	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsInterventionInit */


/** \brief incrementCostsInterventionMonthly adds the monthly costs for starting an intervention
 * \param cost a double representing the cost to be added
 *
 * The cost will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsInterventionMonthly(double cost){
	double undiscountedCost = cost;
	double discountedCost = cost * patient->generalState.discountFactor;
	runStats->overallCosts.costsInterventionMonthly += discountedCost;


	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsInterventionMonthly += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsInterventionMonthly += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsInterventionMonthly += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsInterventionMonthly += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsInterventionMonthly += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsInterventionMonthly += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsInterventionMonthly += discountedCost;
	}

	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsInterventionMonthly */


/** \brief incrementCostsProph adds a prophylaxis treatment cost to the patients total
 *
 * \param oiType a SimContext::OI_TYPE indicating the OI prophylaxis regimen that caused the cost
 * \param prophNum an integer representing the index of which specific prophylaxis caused the cost
 * \param cost a double representing the cost to be added
 *
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsProph(SimContext::OI_TYPE oiType, int prophNum, double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;
	runStats->overallCosts.costsDrugs += undiscountedCost;
	runStats->overallCosts.costsDrugsDiscounted += discountedCost;
	runStats->overallCosts.directCostsProphOIsProph[oiType][prophNum] += discountedCost;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//OI Proph costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsOIProph += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsOIProph += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOIProph += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOIProph += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsOIProph += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsOIProph += discountedCost;
			}
		}
	}

	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsProph[oiType][prophNum] += discountedCost;
	}
	incrementCostsCommon(cost, 1.0);
} /* incrementCostsProph */

/** \brief incrementCostsTBProph adds a TB proph cost to patients total
 *
 * \param prophNum an integer representing which prophylaxis regimen caused the cost to be accrued
 * \param cost a double representing the cost to be added
 *
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsTBProph(int prophNum, double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;
	runStats->overallCosts.costsDrugs += undiscountedCost;
	runStats->overallCosts.costsDrugsDiscounted += discountedCost;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//TB Proph costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsTBProph += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsTBProph += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsTBProph += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsTBProph += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsTBProph += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsTBProph += discountedCost;
			}
		}
	}

	incrementCostsTBCommon(cost, 1.0);
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsTBProph */

/** \brief incrementCostsTBTreatment adds a TB treatment cost to patients total
 *
 * \param testCost a double representing the treatment cost to be added
 * \param treatmentNum an int representing the treatment number
 *
 * The costs will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(const double, double) */
void StateUpdater::incrementCostsTBTreatment(double treatCost, int treatmentNum) {
	double discountedCost = treatCost * patient->generalState.discountFactor;
	double undiscountedCost = treatCost;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//TB treatment Cost
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsTBTreatment += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsTBTreatment += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsTBTreatment += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsTBTreatment += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsTBTreatment += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsTBTreatment += discountedCost;
			}
		}
	}


	runStats->overallCosts.costsDrugs += undiscountedCost;
	runStats->overallCosts.costsDrugsDiscounted += discountedCost;
    runStats->overallCosts.costsTBTreatmentByLine[treatmentNum] += discountedCost;
    runStats->overallCosts.costsTBTreatment += discountedCost;

	incrementCostsTBCommon(treatCost, 1.0);
	incrementCostsCommon(treatCost, 1.0);
} /* end incrementCostsTBTreatment */


/** \brief incrementCostsTBTest adds a TB test cost to patients total
 *
 * \param testCost a double representing the test cost to be added
 * \param dstCost a double representing the cost of the DST test to be added
 * \param testNum an int representing the id of the TB Test
 *
 * The cost will be discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsTBTest(double testCost, double dstCost, int testNum){
    double totalCost = testCost + dstCost;
	runStats->overallCosts.costsTBTests += totalCost * patient->generalState.discountFactor;
    incrementCostsCommon(totalCost, 1.0);
    incrementCostsTBCommon(totalCost, 1.0);

	if (testNum > -1 && testNum < SimContext::TB_NUM_TESTS){
        runStats->overallCosts.costsTBTestsInit[testNum] += testCost * patient->generalState.discountFactor;
        runStats->overallCosts.costsTBTestsDST[testNum] += dstCost * patient->generalState.discountFactor;
	}
}



/** \brief incrementCostsToxicity adds a toxicity cost to the patients total
 *
 * \param cost a double representing the cost to be added
 *
 * both discounted and undiscounted costs will be calculated
 * 
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsToxicity(double cost) {
	double discountedCost = cost * patient->generalState.discountFactor;
	double undiscountedCost = cost;

	runStats->overallCosts.costsToxicity += undiscountedCost;
	runStats->overallCosts.costsToxicityDiscounted += discountedCost;
	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Toxicity costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsToxicity += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsToxicity += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsToxicity += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsToxicity += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsToxicity += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsToxicity += discountedCost;
			}
		}
	}


	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsToxicity */

/** \brief incrementCostsTBToxicity adds a TB toxicity cost to the patients total
 *
 * \param cost a double representing the cost to be added
 * \param treatmentNum an int representing the TB treatment line corresponding to the tox
 *
 * This function calls incrementCostsToxicity, which will calculate both discounted and undiscounted costs
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
	void StateUpdater::incrementCostsTBToxicity(double cost, int treatmentNum) {
	incrementCostsToxicity(cost);
	incrementCostsTBCommon(cost, 1.0);
	if (treatmentNum != SimContext::NOT_APPL)
        runStats->overallCosts.costsTBTreatmentToxByLine[treatmentNum] += patient->generalState.discountFactor * cost;
} /* end incrementCostsTBToxicity */

/** \brief incrementCostsCHRMs adds a CHRMs cost to the patients total
 *
 * \param cost a double representing the cost to be added
 *
 * This cost is discounted
 *
 * \see StateUpdater::incrementCostsCommon(double, double)
 **/
void StateUpdater::incrementCostsCHRMs(SimContext::CHRM_TYPE CHRMType,double cost) {
	runStats->overallCosts.costsCHRMs[CHRMType] += cost*patient->generalState.discountFactor;
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->costsCHRMs[CHRMType]+= cost*patient->generalState.discountFactor;
	}
	incrementCostsCommon(cost, 1.0);
} /* end incrementCostsCHRMs */

/** \brief incrementCostsRoutineCare adds a routine care cost to the patients total costs
 *
 *	overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *	\param cost a double representing the cost to be added
 *	\param percent a double representing the percent of the cost to apply
 *
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsRoutineCare(const double *costArray, double percent, double multiplier) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	undiscountedCostTotal *= percent * multiplier;
	discountedCostTotal *= patient->generalState.discountFactor * percent * multiplier;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Routine care Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsRoutineCare += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsRoutineCare += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsRoutineCareCategory[j] += costArray[j] * percent * multiplier;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsRoutineCareCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsRoutineCare += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsRoutineCare += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsRoutineCareCategory[j] += costArray[j] * percent * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsRoutineCareCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsRoutineCare += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsRoutineCare += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsRoutineCareCategory[j] += costArray[j ]* percent * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsRoutineCareCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
				}
			}
		}
	}
	incrementCostsCommon(costArray, percent, multiplier);
} /* end incrementCostsRoutineCare */

/** \brief incrementCostsGeneralMedicine adds a general medicine cost to the patients total costs
 *
 *	overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *	\param cost a double representing the cost to be added
 *	\param percent a double representing the percent of the cost to apply
 *	\param multiplier a double representing the proportion of general medicine costs incurred by state from the LTFU tab
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsGeneralMedicine(const double *costArray, double percent, double multiplier) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	undiscountedCostTotal *= percent * multiplier;
	discountedCostTotal *= patient->generalState.discountFactor * percent * multiplier;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Routine care Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsGeneralMedicine += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsGeneralMedicine += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsGeneralMedicineCategory[j] += costArray[j] * percent * multiplier;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsGeneralMedicineCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsGeneralMedicine += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsGeneralMedicine += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsGeneralMedicineCategory[j] += costArray[j] * percent * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsGeneralMedicineCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsGeneralMedicine += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsGeneralMedicine += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsGeneralMedicineCategory[j] += costArray[j ]* percent * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsGeneralMedicineCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
				}
			}
		}
	}
	incrementCostsCommon(costArray, percent, multiplier);
} /* end incrementCostsGeneralmedicine */

/** \brief incrementCostsAcuteOITreatment adds a cost of treating acute OIs to a patient's total costs
 *
 *	\param cost a double representing the cost to be added
 *	\param multiplier a double representing the percent of the cost to apply
 *
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsAcuteOITreatment(const double *costArray, double multiplier) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	undiscountedCostTotal *= multiplier;
	discountedCostTotal *= patient->generalState.discountFactor * multiplier;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Treated OI Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsOITreatment += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsOITreatment += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsOITreatmentCategory[j] += costArray[j] * multiplier;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsOITreatmentCategory[j] += costArray[j]*patient->generalState.discountFactor * multiplier;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOITreatment += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOITreatment += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOITreatmentCategory[j] += costArray[j] * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOITreatmentCategory[j] += costArray[j]*patient->generalState.discountFactor * multiplier;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsOITreatment += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsOITreatment += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsOITreatmentCategory[j] += costArray[j] * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsOITreatmentCategory[j] += costArray[j]*patient->generalState.discountFactor * multiplier;
				}
			}
		}
	}
	incrementCostsCommon(costArray, 1.0, multiplier);
} /* end incrementCostsAcuteOITreatment */

/** \brief incrementCostsAcuteOIUntreated adds a cost of an untreated acute OI to a patient's total costs
 *
 *	\param cost a double representing the cost to be added
 *	\param multiplier a double representing the percent of the cost to apply
 *
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsAcuteOIUntreated(const double *costArray, double multiplier) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	undiscountedCostTotal *= multiplier;
	discountedCostTotal *= patient->generalState.discountFactor * multiplier;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Routine care Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsOIUntreated += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsOIUntreated += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsOIUntreatedCategory[j] += costArray[j] * multiplier;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsOIUntreatedCategory[j] += costArray[j]*patient->generalState.discountFactor * multiplier;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOIUntreated += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOIUntreated += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOIUntreatedCategory[j] += costArray[j] * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsOIUntreatedCategory[j] += costArray[j]*patient->generalState.discountFactor * multiplier;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsOIUntreated += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsOIUntreated += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsOIUntreatedCategory[j] += costArray[j] * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsOIUntreatedCategory[j] += costArray[j]*patient->generalState.discountFactor * multiplier;
				}
			}
		}
	}
	incrementCostsCommon(costArray, 1.0, multiplier);
} /* end incrementCostsAcuteOIUntreated */

/** \brief incrementCostsDeath adds a death medicine cost to the patients total costs
 *
 *	overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *	\param cost a double representing the cost to be added
 *	\param percent a double representing the percent of the cost to apply
 *
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsDeath(const double *costArray, double percent, double multiplier) {
	double discountedCostTotal = 0.0;
	double undiscountedCostTotal = 0.0;
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostTotal += costArray[i];
		undiscountedCostTotal += costArray[i];
	}
	undiscountedCostTotal *= percent * multiplier;
	discountedCostTotal *= patient->generalState.discountFactor * percent * multiplier;

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Death Costs
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsDeath += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsDeath += discountedCostTotal;

			for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costsDeathCategory[j] += costArray[j] * percent * multiplier;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costsDeathCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
			}

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsDeath += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsDeath += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsDeathCategory[j] += costArray[j] * percent * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costsDeathCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
				}
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsDeath += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsDeath += discountedCostTotal;
				for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
					costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costsDeathCategory[j] += costArray[j ]* percent * multiplier;
					costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costsDeathCategory[j] += costArray[j]*patient->generalState.discountFactor * percent * multiplier;
				}
			}
		}
	}
	incrementCostsCommon(costArray, percent, multiplier);
} /* end incrementCostsDeath */

/** \brief incrementCostsMisc adds a miscellaneous cost to the patients total costs
 *
 *	overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *	\param cost a double representing the cost to be added
 *	\param percent a double representing the percent of the cost to apply
 *
 *	This function calls incrementCostsCommon, which will calculate both discounted and undiscounted costs
 *
 *	\see StateUpdater::incrementCostsCommon(double, double)
 */
void StateUpdater::incrementCostsMisc(double cost, double percent) {
	incrementCostsCommon(cost, percent);
} /* end incrementCostsMisc */

/** \brief incrementCostsMisc adds a miscellaneous cost to the patients total costs
 *
 *	overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *	\param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *	\param percent a double representing the percent of the cost to apply
 *
 *	This function calls incrementCostsCommon, which will calculate both discounted and undiscounted costs
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsMisc(const double *costArray, double percent, double multiplier) {
	incrementCostsCommon(costArray, percent, multiplier);
} /* end incrementCostsMisc */

/** \brief incrementCostsTBMisc adds an array of miscellaneous TB costs to the patients total costs
 *
 *	\param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *	\param percent a double representing the percent of the cost to apply
 *
 *	This function calls incrementCostsCommon, which will calculate both discounted and undiscounted costs
 *
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsTBMisc(const double *costArray, double percent, double multiplier) {
	incrementCostsCommon(costArray, percent, multiplier);
	incrementCostsTBCommon(costArray, percent, multiplier);
} /* end incrementCostsTBMisc */

/** \brief incrementCostsTBProviderVisit adds an array of TB provider visit costs to the patients total costs and TB provider costs
 *
 *	\param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *	\param percent a double representing the percent of the cost to apply
	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsTBProviderVisit(const double *costArray, double percent, double multiplier) {
	incrementCostsCommon(costArray, percent, multiplier);
	incrementCostsTBCommon(costArray, percent, multiplier);

    for (int i = 0; i < SimContext::COST_NUM_TYPES; i++){
        runStats->overallCosts.costsTBProviderVisits[i] += costArray[i] * percent * multiplier* patient->generalState.discountFactor;
	}
} /* end incrementCostsTBProviderVisit */

/** \brief incrementCostsTBMedVisit adds an array of TB medication visit costs to the patients total costs and med visit costs
 *
 *	\param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *	\param percent a double representing the percent of the cost to apply
 *	\see StateUpdater::incrementCostsCommon(const double*, double)
 */
void StateUpdater::incrementCostsTBMedVisit(const double *costArray, double percent, double multiplier) {
	incrementCostsCommon(costArray, percent, multiplier);
	incrementCostsTBCommon(costArray, percent, multiplier);

	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++){
        runStats->overallCosts.costsTBMedicationVisits[i] += costArray[i] * percent * multiplier* patient->generalState.discountFactor;
	}
} /* end incrementCostsTBMedVisit */

/** \brief updateInitialDistributions updates the initial statistics for patients upon HIV infection */
void StateUpdater::updateInitialDistributions() {
	runStats->initialDistributions.numPatientsCD4Level[patient->diseaseState.currTrueCD4Strata]++;
	runStats->initialDistributions.numPatientsHVLLevel[patient->diseaseState.currTrueHVLStrata]++;
	runStats->initialDistributions.numPatientsHVLSetpointLevel[patient->diseaseState.setpointHVLStrata]++;
	runStats->initialDistributions.sumInitialAgeMonths += patient->generalState.ageMonths;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->diseaseState.hasTrueOIHistory[i]) {
			runStats->initialDistributions.numPriorOIHistories[i]++;
		}
	}
	if (patient->generalState.gender == SimContext::GENDER_MALE)
		runStats->initialDistributions.numMalePatients++;
	else
		runStats->initialDistributions.numFemalePatients++;
	runStats->initialDistributions.numARTResposneTypes[patient->artState.CD4ResponseType]++;
} /* end updateInitialDistributions */

/** \brief updatePatientSurvival updates the patient state for discounted LMs and QALMs for the individual patient,
 * used with a half month length if death occurred that month
 *
 * \param percentOfMonth a double representing the percent of the month the patient was alive: if the patient died this month, this value should be 0.5.  If the patient did not die, this value should be 1.0
 **/
void StateUpdater::updatePatientSurvival(double percentOfMonth) {
	patient->generalState.LMsDiscounted += percentOfMonth * patient->generalState.discountFactor;
	patient->generalState.qualityAdjustLMsDiscounted +=
		percentOfMonth * patient->generalState.discountFactor * patient->generalState.QOLValue;
	patient->generalState.LMsUndiscounted += percentOfMonth;
	for( int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		patient->generalState.multDiscLMs[i] += percentOfMonth * patient->generalState.multDiscFactorBenefit[i];
		patient->generalState.multDiscQALMs[i] +=
				percentOfMonth * patient->generalState.multDiscFactorBenefit[i] * patient->generalState.QOLValue;
	}
} /* end updatePatientSurvival */

/** \brief updateOverallSurvival updates the statistics for stratified LMs and QALMs for the whole population,
 *	used with a half month length if death occurred that month
 *
 *	\param percentOfMonth a double representing the percent of the month the patient was alive: if the patient died this month, this value should be 0.5.  If the patient did not die, this value should be 1.0
 **/
void StateUpdater::updateOverallSurvival(double percentOfMonth) {
	SimContext::HIV_ID detectedState = patient->monitoringState.isDetectedHIVPositive ? SimContext::HIV_ID_IDEN : SimContext::HIV_ID_UNID;
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
		detectedState = SimContext::HIV_ID_NEG;
	// Update survival statistics by detected HIV state first
	runStats->overallSurvival.LMsHIVState[detectedState] +=
		percentOfMonth * patient->generalState.discountFactor;
	runStats->overallSurvival.QALMsHIVState[detectedState] +=
		percentOfMonth * patient->generalState.discountFactor * patient->generalState.QOLValue;

	// If HIV negative, only update a few additional survival statistics and return
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG) {
		runStats->overallSurvival.LMsInScreening +=
			percentOfMonth * patient->generalState.discountFactor;
		runStats->overallSurvival.QALMsInScreening +=
			percentOfMonth * patient->generalState.discountFactor * patient->generalState.QOLValue;
		if(patient->monitoringState.hasPrEP)
			runStats->overallSurvival.LMsHIVNegativeOnPrEP += percentOfMonth * patient->generalState.discountFactor;
		for(int i = 0; i < SimContext::CHRM_NUM; i++){
			if(patient->diseaseState.hasTrueCHRMs[i]){
				runStats->overallSurvival.LMsCHRMHistoryCHRMsHIVNeg[i]+=
					percentOfMonth*patient->generalState.discountFactor;
			}
		}	
		return;
	}

	// Update all the overall survival statistics for HIV positive patients
	if (patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N) {
		runStats->overallSurvival.LMsNoOIHistoryCD4[patient->diseaseState.currTrueCD4Strata] +=
			percentOfMonth * patient->generalState.discountFactor;
	}
	else {
		runStats->overallSurvival.LMsOIHistoryCD4[patient->diseaseState.currTrueCD4Strata] +=
			percentOfMonth * patient->generalState.discountFactor;
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->diseaseState.hasTrueOIHistory[i]) {
			runStats->overallSurvival.LMsOIHistoryOIs[i] +=
				percentOfMonth * patient->generalState.discountFactor;
		}
		else {
			runStats->overallSurvival.LMsNoOIHistoryOIs[i] +=
				percentOfMonth * patient->generalState.discountFactor;
		}
	}
	for(int i=0;i<SimContext::CHRM_NUM;i++){
		if(patient->diseaseState.hasTrueCHRMs[i]){
			runStats->overallSurvival.LMsCHRMHistoryCHRMsHIVPos[i]+=
					percentOfMonth*patient->generalState.discountFactor;
		}
	}
	runStats->overallSurvival.LMsHVL[patient->diseaseState.currTrueHVLStrata] +=
		percentOfMonth * patient->generalState.discountFactor;
	runStats->overallSurvival.LMsHVLSetpoint[patient->diseaseState.setpointHVLStrata] +=
		percentOfMonth * patient->generalState.discountFactor;
	runStats->overallSurvival.LMsInRegularCEPAC +=
		percentOfMonth * patient->generalState.discountFactor;
	runStats->overallSurvival.LMsGender[patient->generalState.gender] +=
		percentOfMonth * patient->generalState.discountFactor;
	runStats->overallSurvival.QALMsGender[patient->generalState.gender] +=
		percentOfMonth * patient->generalState.discountFactor * patient->generalState.QOLValue;
} /* end updateOverallSurvival */

/* \brief updateCostStats updates statistics used for the coststats file
*	 used with a half month length if death occurred that month
*	\param percentOfMonth a double representing the percent of the month the patient was alive: if the patient died this month, this value should be 0.5.  If the patient did not die, this value should be 1.0
*/
void StateUpdater::updateCostStats (double percentOfMonth){
	//Update sums for life months for cost statistics
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Life months
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].LMs += percentOfMonth;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].LMs += percentOfMonth * patient->generalState.discountFactor;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].LMs += percentOfMonth;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].LMs += percentOfMonth * patient->generalState.discountFactor;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].LMs += percentOfMonth;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].LMs += percentOfMonth * patient->generalState.discountFactor;
			}
		}
	}
}/* end updateCostStats */

/*setCostSubgroups sets the cost subgroups that the patient belongs to
 * \param if isInit then set all values to false
 * */
void StateUpdater::setCostSubgroups(bool isInit){
	if (isInit){
		for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++)
			patient->generalState.costSubgroups[i] = false;
		return;
	}

	SimContext::HIV_CARE careState = patient->monitoringState.careState;
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++)
		patient->generalState.costSubgroups[i] = false;
	patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_ALL] = true;
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
		patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_HIV_NEGATIVE] = true;
	else{
		patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_HIV_POSITIVE] = true;
		if (careState < SimContext::HIV_CARE_IN_CARE)
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_PRE_LINKAGE] = true;
		if ((careState == SimContext::HIV_CARE_IN_CARE || careState == SimContext::HIV_CARE_RTC) && !patient->artState.isOnART)
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_INCARE_NOT_ON_ART] = true;
		if (patient->artState.isOnART)
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_ON_ART] = true;
		if (careState == SimContext::HIV_CARE_LTFU && patient->artState.hasTakenART)
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_LTFU_AFTER_ART] = true;
		if (careState == SimContext::HIV_CARE_LTFU && !patient->artState.hasTakenART)
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_LTFU_NEVER_ON_ART] = true;
		if (careState == SimContext::HIV_CARE_RTC && patient->artState.hasTakenART)
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_RTC_AFTER_ART] = true;
		if (careState == SimContext::HIV_CARE_IN_CARE && patient->artState.isOnART){
			patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_ON_ART_NEVER_LOST] = true;
			if (patient->generalState.monthNum - patient->artState.monthFirstStartART <=6)
				patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_ON_ART_FIRST_SIX_MONTHS] = true;
		}
		if (patient->artState.isOnART){
			if(patient->artState.currRegimenNum == 0)
				patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_FIRST_LINE_ART] = true;
			else
				patient->generalState.costSubgroups[SimContext::COST_SUBGROUP_SECOND_LINE_OR_HIGHER] = true;
		}
	}
}

/** \brief updateLongitSurvival updates the longitudinal statistics related to survival
 *
 * If no longitudinal logging, nothing happens.  If yearly logging was specified and the current month is not the end of the year, nothing is logged.
 **/
void StateUpdater::updateLongitSurvival() {
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		// return if using yearly longitudinal logging and current month is not the end of the year
		if ((simContext->getRunSpecsInputs()->longitLoggingLevel == SimContext::LONGIT_SUMM_YR_DET) &&
			(patient->generalState.monthNum %12 != 11))
				return;

		int ageBracketOutput = getAgeCategoryOutput(patient->generalState.ageMonths);
		currTime->numAgeBracketAlive[ageBracketOutput]++;

		SimContext::HIV_ID detectedState = patient->monitoringState.isDetectedHIVPositive ? SimContext::HIV_ID_IDEN : SimContext::HIV_ID_UNID;
		if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
			detectedState = SimContext::HIV_ID_NEG;

		SimContext::HIST_EXT oiHist = patient->diseaseState.typeTrueOIHistory;
		SimContext::GENDER_TYPE gender = patient->generalState.gender;

		// start with outputs that apply to all patients regardless of HIV status
		currTime->numAlive++;
		currTime->numAliveCare[patient->monitoringState.careState]++;
		currTime->numAliveCareCD4Metric[patient->monitoringState.careState][patient->pedsState.ageCategoryCD4Metric]++;

		currTime->numWithOIHistExt[oiHist]++;
		currTime->numWithOIHistExtCare[oiHist][patient->monitoringState.careState]++;
		currTime->numGender[gender]++;
		currTime->numGenderCare[gender][patient->monitoringState.careState]++;

		currTime->ageSum += patient->generalState.ageMonths;
		currTime->ageSumSquares += patient->generalState.ageMonths * patient->generalState.ageMonths;
		currTime->ageSumCare[patient->monitoringState.careState] += patient->generalState.ageMonths;
		currTime->ageSumSquaresCare[patient->monitoringState.careState] += patient->generalState.ageMonths * patient->generalState.ageMonths;
		currTime->numAgeBracketCare[patient->monitoringState.careState][ageBracketOutput]++;

		//Increase QOL modifier sum
		currTime->sumQOLModifiers += patient->generalState.QOLValue;

		// HIV-negative patients have a PTR value but it isn't used unless they become HIV-positive
		currTime->propRespSum += patient->generalState.responseBaselineLogit;
		currTime->propRespSumSquares += patient->generalState.responseBaselineLogit * patient->generalState.responseBaselineLogit;
		currTime->propRespSumCare[patient->monitoringState.careState] += patient->generalState.responseBaselineLogit;
		currTime->propRespSumSquaresCare[patient->monitoringState.careState] += patient->generalState.responseBaselineLogit * patient->generalState.responseBaselineLogit;
		
		bool hasCHRMs=false;
		int ageCatCHRMs = 0;
		for(int i=0;i<SimContext::CHRM_NUM;i++){
			if(patient->diseaseState.hasTrueCHRMs[i]){
				ageCatCHRMs = getAgeCategoryCHRMs(patient->generalState.ageMonths, i);
				currTime->numAliveTypeCHRMs[detectedState][i]++;
				currTime->numCHRMsAge[i][ageCatCHRMs]++;
				currTime->numCHRMsGender[i][patient->generalState.gender]++;
				if(patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG)
					currTime->numCHRMsCD4[i][patient->diseaseState.currTrueCD4Strata]++;
				hasCHRMs=true;
			}
		}
		if(hasCHRMs){
			currTime->numCHRMsAgeTotal[ageCatCHRMs]++;
			currTime->numCHRMsGenderTotal[patient->generalState.gender]++;
			currTime->numAliveWithCHRMsDetState[detectedState]++;
			if(patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG)
				currTime->numCHRMsCD4Total[patient->diseaseState.currTrueCD4Strata]++;						
		}
		else{
			currTime->numAliveWithoutCHRMsDetState[detectedState]++;
		}

		//TB
		if (simContext->getTBInputs()->enableTB){
			currTime->numAliveTB[patient->tbState.currTrueTBDiseaseState]++;
			for (int i = 0; i < SimContext::TB_NUM_TRACKER; i++){
				if (patient->tbState.currTrueTBTracker[i]){
					currTime->numAliveTBTrackerCare[i][patient->monitoringState.careState]++;
					if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG)
						currTime->numHIVTBTrackerCD4[i][patient->diseaseState.currTrueCD4Strata]++;
				}
			}
			if (patient->tbState.careState == SimContext::TB_CARE_LTFU)
				currTime->numTBLTFU++;
			if (patient->tbState.isOnProph)
				currTime->numOnTBProph[patient->tbState.currProphNum][patient->tbState.currTrueTBDiseaseState]++;
			if (patient->tbState.isOnTreatment){
				currTime->numOnTBTreatmentByState[patient->tbState.currTrueTBDiseaseState][patient->tbState.currTreatmentNum]++;
				currTime->numOnTBTreatmentTotal[patient->tbState.currTreatmentNum]++;
			}	
			else if (patient->tbState.isOnEmpiricTreatment){
				currTime->numOnEmpiricTBTreatmentByState[patient->tbState.currTrueTBDiseaseState][patient->tbState.currEmpiricTreatmentNum]++;
				currTime->numOnEmpiricTBTreatmentTotal[patient->tbState.currEmpiricTreatmentNum]++;
			}
			if (patient->tbState.currTrueTBDiseaseState != SimContext::TB_STATE_UNINFECTED)
				currTime->numTBStrain[patient->tbState.currTrueTBResistanceStrain]++;

			if (patient->tbState.hasObservedTBResistanceStrain){
				SimContext::TB_STRAIN obsvStrain = patient->tbState.currObservedTBResistanceStrain;
				if (patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_UNINFECTED)
					currTime->numObservedTBUninfectedTB[obsvStrain]++;
				else
					currTime->numObservedTBByTrueTBStrain[obsvStrain][patient->tbState.currTrueTBResistanceStrain]++;
			}

			bool unfavorableIndex[SimContext::TB_NUM_UNFAVORABLE];
			for (int i = 0; i < SimContext::TB_NUM_UNFAVORABLE; i++)
				unfavorableIndex[i] = patient->tbState.hasUnfavorableOutcome[i];

			currTime->numTBUnfavorableOutcome[unfavorableIndex[0]][unfavorableIndex[1]][unfavorableIndex[2]][unfavorableIndex[3]]++;

			if (patient->tbState.isOnTreatment || patient->tbState.isOnEmpiricTreatment){
				bool treatSuccess = false;
				int treatNum = 0;
				if (patient->tbState.isOnTreatment){
					treatNum = patient->tbState.currTreatmentNum;
					treatSuccess = patient->tbState.treatmentSuccess;
				}
				if (patient->tbState.isOnEmpiricTreatment){
					treatNum = patient->tbState.currEmpiricTreatmentNum;
					treatSuccess = patient->tbState.empiricTreatmentSuccess;
				}

				if (treatSuccess){
					currTime->numOnSuccessfulTBTreatment[treatNum]++;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM)
						currTime->numOnSuccessfulTBTreatmentPulm[treatNum]++;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
						currTime->numOnSuccessfulTBTreatmentExtraPulm[treatNum]++;
				}
				else{
					currTime->numOnFailedTBTreatment[treatNum]++;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_PULM)
						currTime->numOnFailedTBTreatmentPulm[treatNum]++;
					if(patient->tbState.currTrueTBDiseaseState == SimContext::TB_STATE_ACTIVE_EXTRAPULM)
						currTime->numOnFailedTBTreatmentExtraPulm[treatNum]++;
				}
			}
		} // end if TB is enabled
		
		// patients with undetected HIV can still be on PrEP - their risk level is the same as it was before infection
		if (simContext->getHIVTestInputs()->enableHIVTesting && simContext->getHIVTestInputs()->enablePrEP && patient->monitoringState.hasPrEP){
            SimContext::HIV_BEHAV risk = patient->monitoringState.isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
            currTime->numOnPrEP[risk]++;
		}
		
		if(patient->pedsState.ageCategoryPediatrics < SimContext::PEDS_AGE_LATE){
			currTime->numAlivePediatrics[patient->diseaseState.infectedPediatricsHIVState]++;
			if (patient->pedsState.isMotherAlive)
				currTime->numAlivePediatricsMotherAlive[patient->pedsState.maternalStatus]++;
			else
				currTime->numAlivePediatricsMotherDead++;
		}

		// HIV-negative patients
		if (patient->diseaseState.infectedHIVState == SimContext::HIV_CARE_NEG){
			SimContext::HIV_BEHAV risk = patient->monitoringState.isHighRiskForHIV?SimContext::HIV_BEHAV_HI:SimContext::HIV_BEHAV_LO;
			currTime->numAliveNegRisk[risk]++;
			currTime->numWithoutOIHistory++;
			if (patient->pedsState.isFalsePositive){
				currTime->numAliveFalsePositive++;
				if(patient->pedsState.isFalsePositiveLinked)
					currTime->numAliveFalsePositiveLinked++;
			}
		}	
		// HIV-positive patients
		else{
			currTime->numAlivePositive++;
			currTime->numAgeBracketHIVPositive[ageBracketOutput]++;
			currTime->ageSumPositive += patient->generalState.ageMonths;
			currTime->ageSumSquaresPositive += patient->generalState.ageMonths * patient->generalState.ageMonths;
			currTime->numGenderPositive[gender]++;

			if (patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N) {
				currTime->numWithoutOIHistory++;
			}
			else {
				for (int i = 0; i < SimContext::OI_NUM; i++) {
					if (patient->diseaseState.hasTrueOIHistory[i]) {
						currTime->numWithOIHistory[i]++;
					}
				}
			}
			currTime->numWithOIHistExtPositive[oiHist]++;
			
			currTime->propRespSumPositive += patient->generalState.responseBaselineLogit;
			currTime->propRespSumSquaresPositive += patient->generalState.responseBaselineLogit * patient->generalState.responseBaselineLogit;

			if (patient->monitoringState.careState == SimContext::HIV_CARE_IN_CARE || patient->monitoringState.careState == SimContext::HIV_CARE_RTC){
				if (patient->artState.isOnART){
					currTime->numAliveOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy]++;
					currTime->ageSumOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->generalState.ageMonths;
					currTime->ageSumSquaresOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->generalState.ageMonths * patient->generalState.ageMonths;
					currTime->propRespSumOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->generalState.responseBaselineLogit;
					currTime->propRespSumSquaresOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->generalState.responseBaselineLogit * patient->generalState.responseBaselineLogit;
					if(patient->pedsState.ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE)
						currTime->numAliveOnARTCD4Metric[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy][SimContext::CD4_ABSOLUTE]++;
					else
						currTime->numAliveOnARTCD4Metric[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy][SimContext::CD4_PERC]++;
					currTime->numWithOIHistExtOnART[oiHist][patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy]++;
					currTime->numGenderOnART[gender][patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy]++;
					currTime->numAgeBracketOnART[ageBracketOutput]++;
					currTime->totalAliveOnART++;
				}
				else{
					currTime->numAliveInCareOffART++;
					currTime->ageSumInCareOffART += patient->generalState.ageMonths;
					currTime->ageSumSquaresInCareOffART += patient->generalState.ageMonths * patient->generalState.ageMonths;
					currTime->propRespSumInCareOffART += patient->generalState.responseBaselineLogit;
					currTime->propRespSumSquaresInCareOffART += patient->generalState.responseBaselineLogit * patient->generalState.responseBaselineLogit;
					if(patient->pedsState.ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE)
						currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_ABSOLUTE]++;
					else
						currTime->numAliveInCareOffARTCD4Metric[SimContext::CD4_PERC]++;
					currTime->numWithOIHistExtInCareOffART[oiHist]++;
					currTime->numGenderInCareOffART[gender]++;
					currTime->numAgeBracketInCareOffART[ageBracketOutput]++;
				}
			}
			// CD4 and HVL stats for HIV+ patients
			// use true CD4 for late childhood and adults
			currTime->numAlivePositiveCD4Metric[patient->pedsState.ageCategoryCD4Metric]++;
			if (patient->pedsState.ageCategoryCD4Metric == SimContext::CD4_ABSOLUTE){
				currTime->trueCD4Sum += patient->diseaseState.currTrueCD4;
				currTime->trueCD4SumSquares += patient->diseaseState.currTrueCD4 * patient->diseaseState.currTrueCD4;
				currTime->trueCD4SumCare[patient->monitoringState.careState] += patient->diseaseState.currTrueCD4;
				currTime->trueCD4SumSquaresCare[patient->monitoringState.careState] += patient->diseaseState.currTrueCD4 * patient->diseaseState.currTrueCD4;
				if (patient->monitoringState.careState == SimContext::HIV_CARE_IN_CARE || patient->monitoringState.careState == SimContext::HIV_CARE_RTC){
					if (patient->artState.isOnART){
						currTime->trueCD4SumOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->diseaseState.currTrueCD4;
						currTime->trueCD4SumSquaresOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->diseaseState.currTrueCD4 * patient->diseaseState.currTrueCD4;
					}
					else{
						currTime->trueCD4SumInCareOffART += patient->diseaseState.currTrueCD4;
						currTime->trueCD4SumSquaresInCareOffART += patient->diseaseState.currTrueCD4 * patient->diseaseState.currTrueCD4;
					}
				}
				if (patient->monitoringState.hasObservedCD4) {
					currTime->numHIVPosWithObservedCD4++;
					currTime->observedCD4Sum += patient->monitoringState.currObservedCD4;
					currTime->observedCD4SumSquares += patient->monitoringState.currObservedCD4 * patient->monitoringState.currObservedCD4;
					currTime->observedCD4SumCare[patient->monitoringState.careState] += patient->monitoringState.currObservedCD4;
					currTime->observedCD4SumSquaresCare[patient->monitoringState.careState] += patient->monitoringState.currObservedCD4 * patient->monitoringState.currObservedCD4;
					currTime->observedCD4Distribution[patient->monitoringState.currObservedCD4Strata]++;
					currTime->observedCD4DistributionCare[patient->monitoringState.currObservedCD4Strata][patient->monitoringState.careState]++;
					if (patient->monitoringState.careState == SimContext::HIV_CARE_IN_CARE || patient->monitoringState.careState == SimContext::HIV_CARE_RTC){
						if (patient->artState.isOnART){
							currTime->observedCD4DistributionOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy]++;
							currTime->observedCD4SumOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->monitoringState.currObservedCD4;
							currTime->observedCD4SumSquaresOnART[patient->artState.currRegimenNum][patient->artState.currRegimenEfficacy] += patient->monitoringState.currObservedCD4 * patient->monitoringState.currObservedCD4;
						}
						else{
							currTime->numWithObservedCD4InCareOffART++;
							currTime->observedCD4SumInCareOffART += patient->monitoringState.currObservedCD4;
							currTime->observedCD4SumSquaresInCareOffART += patient->monitoringState.currObservedCD4 * patient->monitoringState.currObservedCD4;
						}
					}
				} // end if patient has observed CD4
			}	// end if patient is in late childhood or an adult patient 
			// pediatric HIV+ patients in early childhood use the cd4 percentage
			else {
				currTime->trueCD4PercentageSum += patient->diseaseState.currTrueCD4Percentage;
				currTime->trueCD4PercentageSumSquares += patient->diseaseState.currTrueCD4Percentage * patient->diseaseState.currTrueCD4Percentage;
			}
			double hvlValue = SimContext::HVL_STRATA_MIDPTS[patient->diseaseState.currTrueHVLStrata];
			currTime->trueHVLSum += hvlValue;
			currTime->trueHVLSumSquares += hvlValue * hvlValue;
			if (patient->monitoringState.hasObservedHVLStrata) {
				hvlValue = SimContext::HVL_STRATA_MIDPTS[patient->monitoringState.currObservedHVLStrata];
				currTime->observedHVLSum += hvlValue;
				currTime->observedHVLSumSquares += hvlValue * hvlValue;
				currTime->observedHVLDistribution[patient->monitoringState.currObservedHVLStrata]++;
			}
			if (patient->artState.isOnART) {
				currTime->trueCD4HVLARTDistribution[SimContext::ART_ON_STATE][patient->diseaseState.currTrueCD4Strata][patient->diseaseState.currTrueHVLStrata]++;
				currTime->numOnART[patient->artState.currRegimenNum]++;
				if (patient->artState.monthOfCurrRegimenStart == patient->generalState.monthNum)
					currTime->numStartingART[patient->artState.currRegimenNum]++;
				currTime->numARTEfficacyState[patient->artState.currRegimenEfficacy]++;
			}
			else {
				SimContext::HIV_CARE careState = patient->monitoringState.careState;
				if (careState == SimContext::HIV_CARE_IN_CARE || careState == SimContext::HIV_CARE_RTC){
					if(!patient->artState.hasTakenART){
						currTime->numInCarePreART++;
						if (patient->monitoringState.monthOfLinkage == patient->generalState.monthNum)
							currTime->numStartingPreART++;
					}
					else{
						currTime->numInCarePostART++;
						if (patient->artState.monthOfPrevRegimenStop == patient->generalState.monthNum)
							currTime->numStartingPostART++;
					}
				}
				currTime->trueCD4HVLARTDistribution[SimContext::ART_OFF_STATE][patient->diseaseState.currTrueCD4Strata][patient->diseaseState.currTrueHVLStrata]++;
			}
			if (simContext->getCohortInputs()->showTransmissionOutput){
				double transmRate = 0;
				bool isAcute = false;
				SimContext::CD4_STRATA trueCD4Strata = patient->diseaseState.currTrueCD4Strata;
				SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
				if (simContext->getCohortInputs()->transmUseHIVTestAcuteDefinition){
					if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_ACUTE_SYN)
						isAcute = true;
				}
				else{
					if(patient->generalState.monthNum - patient->diseaseState.monthOfHIVInfection < simContext->getCohortInputs()->transmAcuteDuration)
						isAcute = true;
				}
				if (patient->artState.isOnART) {
					if (isAcute)
						transmRate = simContext->getCohortInputs()->transmRateOnARTAcute[trueCD4Strata];
					else
						transmRate = simContext->getCohortInputs()->transmRateOnART[trueCD4Strata][hvlStrata];
				}
				else{
					if (isAcute)
						transmRate = simContext->getCohortInputs()->transmRateOffARTAcute[trueCD4Strata];
					else
						transmRate = simContext->getCohortInputs()->transmRateOffART[trueCD4Strata][hvlStrata];
				}

				//Multiply by rate based on time
				int monthNum = patient->generalState.monthNum;
				if (monthNum <= simContext->getCohortInputs()->transmRateMultInterval[0])
					transmRate *= simContext->getCohortInputs()->transmRateMult[0];
				else if (monthNum <= simContext->getCohortInputs()->transmRateMultInterval[1])
					transmRate *= simContext->getCohortInputs()->transmRateMult[1];
				else
					transmRate *= simContext->getCohortInputs()->transmRateMult[2];

				if (simContext->getCohortInputs()->useDynamicTransm){
					//Calculate risk multiplier with incidence output from warmup run - the only output saved from it
					double incidenceRate = 0;

					if (currTime->dynamicNumHIVNegAtStartMonth > 0){
                        // Calculate current incidence rate and multiply by 1200 to convert to incidence rate / 100 person years
						incidenceRate = (double) currTime->dynamicNumIncidentHIVInfections/currTime->dynamicNumHIVNegAtStartMonth*1200.0;
					}
					// Estimate the number of transmissions which would have had to occur within the HRG (i.e., cohort) in the index year to account for this incidence rate ("inside" transmissions rather than infections coming from outside the cohort)
					double selfTransmissionRate = incidenceRate * simContext->getCohortInputs()->dynamicTransmPropHRGAttributable *
							simContext->getCohortInputs()->dynamicTransmNumHIVNegHRG/simContext->getCohortInputs()->dynamicTransmNumHIVPosHRG;
					// Find the ratio of that estimate for this month to the number of transmissions of the HRG in the index year; this is the multiplier 
					double selfTransmissionMult = selfTransmissionRate/simContext->getCohortInputs()->dynamicTransmHRGTransmissions;
					// Multiply by the multiplier to update the expected number of transmissions from this cohort member per 100PY
					transmRate *= selfTransmissionMult;
					currTime->dynamicSelfTransmissionMult = selfTransmissionMult;
				}
				else{
					//Multiply by risk factor multiplier
					int transmRiskAgeCat = 2;
					int age = patient->generalState.ageMonths;
					if ( age <= simContext->getCohortInputs()->transmRiskMultBounds[0])
						transmRiskAgeCat = 0;
					else if(age <=simContext->getCohortInputs()->transmRiskMultBounds[1])
						transmRiskAgeCat = 1;

					transmRate *= simContext->getCohortInputs()->transmRiskMult[patient->generalState.transmRiskCategory][transmRiskAgeCat];
				}
				double numTransm = transmRate/1200.0;
				currTime->numTransmissions += numTransm;
				currTime->numTransmissionsHVL[hvlStrata] += numTransm;
				currTime->numTransmissionsRisk[patient->generalState.transmRiskCategory] += numTransm;
			} // end if transmission output is enabled
			if (patient->monitoringState.careState == SimContext::HIV_CARE_LTFU){
				if(patient->monitoringState.wasOnARTWhenLostToFollowUp){
					currTime->numLostToFollowUpART[patient->artState.prevRegimenNum]++;
				}
				else if(!patient->artState.hasTakenART){
					currTime->numLostToFollowUpPreART++;
				}
				else{
					currTime->numLostToFollowUpPostART++;
				}
			}
		} // end if patient is HIV+	
	} // end if(currTime)
} /* end updateLongitSurvival */

/** \brief updateOIHistoryLogging updates the statistics for patient OI history logging */
void StateUpdater::updateOIHistoryLogging() {
	SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
	SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
	int numARTFailures = patient->artState.numObservedFailures;

	// Return if OI history logging is not enabled or logging criteria is not met
	if (!simContext->getRunSpecsInputs()->enableOIHistoryLogging)
		return;
	if (patient->pedsState.ageCategoryPediatrics >= SimContext::PEDS_AGE_LATE){
		double cd4Level = patient->diseaseState.currTrueCD4;
		if ((cd4Level < simContext->getRunSpecsInputs()->CD4BoundsForOIHistoryLogging[SimContext::LOWER_BOUND]) ||
		(cd4Level > simContext->getRunSpecsInputs()->CD4BoundsForOIHistoryLogging[SimContext::UPPER_BOUND]))
			return;
	}	
	if ((hvlStrata < simContext->getRunSpecsInputs()->HVLBoundsForOIHistoryLogging[SimContext::LOWER_BOUND]) ||
		(hvlStrata > simContext->getRunSpecsInputs()->HVLBoundsForOIHistoryLogging[SimContext::UPPER_BOUND]))
		return;
	if (simContext->getRunSpecsInputs()->numARTFailuresForOIHistoryLogging != SimContext::NOT_APPL) {
		if (patient->artState.isOnART)
			return;
		if (numARTFailures != simContext->getRunSpecsInputs()->numARTFailuresForOIHistoryLogging)
			return;
	}
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->diseaseState.hasTrueOIHistory[i] && simContext->getRunSpecsInputs()->OIsToExcludeOIHistoryLogging[i])
			return;
	}

	// Increment the stratified total months and months with OI history
	runStats->oiStats.numMonthsHVLCD4[hvlStrata][cd4Strata]++;
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (patient->diseaseState.hasTrueOIHistory[i]) {
			runStats->oiStats.numMonthsOIHistoryHVLCD4[i][hvlStrata][cd4Strata]++;
		}
	}

	// Increment the stratified number of patients and patients with OI history,
	//	only called once for each patient
	if (!patient->generalState.loggedPatientOIs) {
		runStats->oiStats.numPatientsHVLCD4[hvlStrata][cd4Strata]++;
		for (int i = 0; i < SimContext::OI_NUM; i++) {
			if (patient->diseaseState.hasTrueOIHistory[i]) {
				runStats->oiStats.numPatientsOIHistoryHVLCD4[i][hvlStrata][cd4Strata]++;
			}
		}
		patient->generalState.loggedPatientOIs = true;
	}
} /* end updateOIHistoryLogging */

/** \brief updateStartMonthHIVNeg updates the number of people who were HIV neg at start of Month*/
void StateUpdater::updateStartMonthHIVNeg() {
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->debugNumHIVNegAtStartMonth++;
		if (simContext->getCohortInputs()->useDynamicTransm && simContext->getCohortInputs()->updateDynamicTransmInc){
			currTime->dynamicNumHIVNegAtStartMonth++;
		}
	}
} /* end updateStartMonthHIVNeg */

/** \brief updateARTToxIncidenceTracking increments the count of incident ART toxicities
 * \param incidentTox a SimContext::ARTToxicityEffect whhich is the incident toxicity being tracked
 * 
*/
void StateUpdater::updateARTToxIncidenceTracking(const SimContext::ARTToxicityEffect &incidentTox){
    RunStats::TimeSummary *currtime = getTimeSummaryForUpdate();
    if (currtime){
        currtime->incidentToxicities[incidentTox.ARTRegimenNum][incidentTox.ARTSubRegimenNum][incidentTox.toxSeverityType][incidentTox.toxNum]++;
    }
}

/** \brief updateARTToxPrevalenceTracking increments the count of prevalent chronic ART toxicities with active mortality risks
 * \param prevalentTox a SimContext::ARTToxicityEffect which is the chronic toxicity being tracked
 * 
*/
void StateUpdater::updateARTToxPrevalenceTracking(const SimContext::ARTToxicityEffect &prevalentTox){
    RunStats::TimeSummary *currtime = getTimeSummaryForUpdate();
    if (currtime){
        currtime->prevalentChronicToxicities[prevalentTox.ARTRegimenNum][prevalentTox.ARTSubRegimenNum][prevalentTox.toxNum]++;
    }
}

/** \brief updateARTEfficacyStats updates the totals for months in ART suppression and HVL drops */
void StateUpdater::updateARTEfficacyStats() {
	if (!patient->artState.isOnART)
		return;

	// Update the months in suppression states
	int artLineNum = patient->artState.currRegimenNum;
	SimContext::ART_EFF_TYPE efficacy = patient->artState.currRegimenEfficacy;
	SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
	switch (efficacy) {
		case SimContext::ART_EFF_SUCCESS:
			runStats->artStats.monthsSuppressedLine[artLineNum]++;
			break;
		case SimContext::ART_EFF_FAILURE:
			runStats->artStats.monthsFailedLineHVL[artLineNum][hvlStrata]++;
			break;
	}

	// Update the number suppressed at HVL drops if month is specified for efficacy recording
	for (int i = 0; i < SimContext::ART_NUM_MTHS_RECORD; i++) {
		int monthsOnART = patient->generalState.monthNum - patient->artState.monthOfCurrRegimenStart;
		if (monthsOnART == simContext->getRunSpecsInputs()->monthRecordARTEfficacy[i]) {
			runStats->artStats.numOnARTAtMonth[artLineNum][i]++;
			if (patient->diseaseState.currTrueHVLStrata <= SimContext::HVL_SUPPRESSION)
				runStats->artStats.numSuppressedAtMonth[artLineNum][i]++;
			int hvlDrop = patient->diseaseState.setpointHVLStrata - patient->diseaseState.currTrueHVLStrata;
			runStats->artStats.HVLDropsAtMonthSum[artLineNum][i] += hvlDrop;
			runStats->artStats.HVLDropsAtMonthSumSquares[artLineNum][i] += hvlDrop * hvlDrop;
			break;
		}
	}
} /* end updateARTEfficacyStats */

/** \brief willAttendClinicThisMonth returns true if patient will go in for a scheduled clinic visit or OI emergency clinic visit this month
 *
 * \return true if the patient should have a clinic visit this month, false otherwise
 **/
bool StateUpdater::willAttendClinicThisMonth() {
	// No clinic visits can occur while the patient is lost or undetected as HIV positive
	if (patient->monitoringState.currLTFUState == SimContext::LTFU_STATE_LOST)
		return false;
	if (!patient->monitoringState.isDetectedHIVPositive)
		return false;

	// Test if a regularly scheduled clinic visit will occur this month, also depends on the
	//	patient's type for when they attend scheduled visits
	if (patient->monitoringState.hasRegularClinicVisit &&
		(patient->generalState.monthNum >= patient->monitoringState.monthOfRegularClinicVisit)) {
			if (!patient->monitoringState.hadPrevClinicVisit)
				return true;
			if (patient->artState.isOnART)
				return true;
			if (patient->prophState.currTotalNumProphsOn > 0)
				return true;
			if (patient->monitoringState.clinicVisitType == SimContext::CLINIC_SCHED)
				return true;
	}

	// Determine if an emergency clinic visit has been triggered for this month
	if ((patient->monitoringState.emergencyClinicVisitType != SimContext::EMERGENCY_NONE) &&
		(patient->generalState.monthNum >= patient->monitoringState.monthOfEmergencyClinicVisit)) {
			return true;
	}

	return false;
} /* end willAttendClinicThisMonth */

/** \brief getCD4Strata returns the CD4 strata for a given value
 *
 * \param valueCD4 a double indicating the CD4 value that should have its strata returned
 *
 * \return a SimContext::CD4_STRATA indicating the CD4 strata corresponding to the exact CD4 value
 **/
SimContext::CD4_STRATA StateUpdater::getCD4Strata(double valueCD4) {
	for (int i = 0; i < SimContext::CD4_NUM_STRATA - 1; i++) {
		if (valueCD4 < simContext->getRunSpecsInputs()->CD4StrataUpperBounds[i]) {
			return ((SimContext::CD4_STRATA) i);
		}
	}
	return SimContext::CD4_VHI;
}

/** \brief getCD4PercentageStrata returns the CD4 percentage strata for a given value
 *
 * \param percCD4 a double (which should be between 0 and 1 inclusive) that indicates the CD4 percentage that should have its strata returned
 *
 * \return a SimContext::PEDS_CD4_PERC indicating the CD4 percentage strata corresponding to the exact CD4 percentage
 **/
SimContext::PEDS_CD4_PERC StateUpdater::getCD4PercentageStrata(double percCD4) {
	if (percCD4 < 0.05)
		return SimContext::PEDS_CD4_PERC_L5;
	if (percCD4 < 0.1)
		return SimContext::PEDS_CD4_PERC_L10;
	if (percCD4 < 0.15)
		return SimContext::PEDS_CD4_PERC_L15;
	if (percCD4 < 0.2)
		return SimContext::PEDS_CD4_PERC_L20;
	if (percCD4 < 0.25)
		return SimContext::PEDS_CD4_PERC_L25;
	if (percCD4 < 0.3)
		return SimContext::PEDS_CD4_PERC_L30;
	if (percCD4 < 0.35)
		return SimContext::PEDS_CD4_PERC_L35;
	return SimContext::PEDS_CD4_PERC_HIGHER;
}

/** \brief getAgeCategoryHIVInfection returns the HIV testing age category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the HIV infection age categories
 **/
int StateUpdater::getAgeCategoryHIVInfection(int ageMonths) {
    int ageYears = ageMonths / 12;
	for (int ageCat = 0; ageCat < SimContext::AGE_CAT_HIV_INC - 1; ageCat++){
        if (ageYears <= simContext->getHIVTestInputs()->HIVIncAgeBounds[ageCat]){
            return ageCat;
        }
	}
	return SimContext::AGE_CAT_HIV_INC - 1;
}

/** \brief getAgeCategoryHeterogeneity returns the Heterogeneity category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the Heterogeneity age categories
 **/
int StateUpdater::getAgeCategoryHeterogeneity(int ageMonths) {
	int ageYears = ageMonths / 12;
	if (ageYears < 18)
		return 0;
	if (ageYears <= 25)
		return 1;
	if (ageYears <= 30)
		return 2;
	if (ageYears <= 35)
		return 3;
	if (ageYears <= 40)
		return 4;
	if (ageYears <= 45)
		return 5;
	return 6;
}

/** \brief getAgeCategoryCost returns the adult cost category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the adult cost age categories
 **/
int StateUpdater::getAgeCategoryCost(int ageMonths){
	int ageYears = ageMonths / 12;
	for (int ageCat = 0; ageCat < SimContext::COST_AGE_CAT_NUM - 1; ageCat++){
        if (ageYears <= simContext->getCostInputs()->costAgeBounds[ageCat]){
            return ageCat;
        }
	}
	return SimContext::COST_AGE_CAT_NUM - 1;
}	

/** \brief getAgeCategoryTBInfection returns the TB Infection age category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the TB infection age categories
 **/
int StateUpdater::getAgeCategoryTBInfection(int ageMonths) {
	int ageYears = ageMonths / 12;
	if (ageYears < 18)
		return 0;
	if (ageYears <= 25)
		return 1;
	if (ageYears <= 30)
		return 2;
	if (ageYears <= 35)
		return 3;
	if (ageYears <= 40)
		return 4;
	if (ageYears <= 45)
		return 5;
	return 6;
}

/** \brief getAgeCategoryTransmRisk returns the age category for transmission risk group distribution
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the age categories
 **/
int StateUpdater::getAgeCategoryTransmRisk(int ageMonths) {
	int ageYears = ageMonths / 12;
	if (ageYears < 20)
		return 0;
	if (ageYears <= 29)
		return 1;
	if (ageYears <= 39)
		return 2;
	if (ageYears <= 49)
		return 3;
	if (ageYears <= 59)
		return 4;
	if (ageYears <= 69)
		return 5;
	return 6;

}

/** \brief getAgeCategoryLinkageStats returns the linkage stats age category for the given age, used for displaying age at linkage in coststats
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the age categories
 **/
int StateUpdater::getAgeCategoryLinkageStats(int ageMonths) {
	int ageYears = ageMonths / 12;
	if (ageYears < 15)
		return 0;
	if (ageYears <= 19)
		return 1;
	if (ageYears <= 24)
		return 2;
	if (ageYears <= 29)
		return 3;
	if (ageYears <= 34)
		return 4;
	if (ageYears <= 39)
		return 5;
	if (ageYears <= 44)
		return 6;
	if (ageYears <= 49)
		return 7;
	return 8;
}

/** \brief getAgeCategoryCHRMs returns the CHRMs age category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 * \param CHRMType is the type of CHRM
 *
 * \return an integer representing the index of the CHRMs age categories
 **/
int StateUpdater::getAgeCategoryCHRMs(int ageMonths, int CHRMType) {
	int ageYears = ageMonths / 12;

	for (int i = 0; i < SimContext::CHRM_AGE_CAT_NUM-1; i++){
		if (ageYears < simContext->getCHRMsInputs()->ageBounds[CHRMType][i])
			return i;
	}
	return SimContext::CHRM_AGE_CAT_NUM-1;
}

/** \brief getAgeCategoryCHRMsMthsStart returns the patient age category to be used when Orphans are enabled to determine the age in months ('months since start') of any children ('prevalent CHRMs') the patient has at model start
 *
 * \param ageMonths an integer representing the age in months of the patient, for whom we will return the associated age category
 *
 * \return an integer representing the index of the patient age categories for determining the age in months ('months since start') of the child ('CHRM')
 **/
int StateUpdater::getAgeCategoryCHRMsMthsStart(int ageMonths) {
	int ageYears = ageMonths / 12;
	if (ageYears < 15)
		return 0;
	if (ageYears <= 19)
		return 1;
	if (ageYears <= 24)
		return 2;
	if (ageYears <= 29)
		return 3;
	if (ageYears <= 34)
		return 4;
	if (ageYears <= 39)
		return 5;
	if (ageYears <= 44)
		return 6;
	if (ageYears <= 49)
		return 7;
	if (ageYears <= 54)
		return 8;
	if (ageYears <= 59)
		return 9;
	if (ageYears <= 64)
		return 10;
	if (ageYears <= 69)
		return 11;
	if (ageYears <= 74)
		return 12;
	if (ageYears <= 79)
		return 13;
	return 14;
}
/** \brief getAgeCategoryCD4Metric returns the CD4 metric for the given age 
 * 
 * \param ageMonths an integer representing the exact age of which to return the CD4 measurement age category
 * 
 * \return a SimContext::PEDS_CD4_AGE_CAT indicating whether the patient would use a percentage CD4 count or absolute CD4 count if they were HIV+
 * 
**/
SimContext::PEDS_CD4_AGE_CAT StateUpdater::getAgeCategoryCD4Metric(int ageMonths){
	if(simContext->getPedsInputs()->enablePediatricsModel && ageMonths < 60)
		return SimContext::CD4_PERC;
	else
		return SimContext::CD4_ABSOLUTE;
};


/** \brief getAgeCategoryPediatrics returns the Pediatrics testing category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the pediatric age categories
 **/
SimContext::PEDS_AGE_CAT StateUpdater::getAgeCategoryPediatrics(int ageMonths) {
	if (!simContext->getPedsInputs()->enablePediatricsModel)
		return SimContext::PEDS_AGE_ADULT;

	if (ageMonths < 3)
		return SimContext::PEDS_AGE_2MTH;
	if (ageMonths < 6)
		return SimContext::PEDS_AGE_5MTH;
	if (ageMonths < 9)
		return SimContext::PEDS_AGE_8MTH;
	if (ageMonths < 12)
		return SimContext::PEDS_AGE_11MTH;
	if (ageMonths < 15)
		return SimContext::PEDS_AGE_14MTH;
	if (ageMonths < 18)
		return SimContext::PEDS_AGE_17MTH;
	if (ageMonths < 24)
		return SimContext::PEDS_AGE_23MTH;
	if (ageMonths < 36)
		return SimContext::PEDS_AGE_2YR;
	if (ageMonths < 48)
		return SimContext::PEDS_AGE_3YR;
	if (ageMonths < 60)
		return SimContext::PEDS_AGE_4YR;
	if (ageMonths < 156)
		return SimContext::PEDS_AGE_LATE;
	return SimContext::PEDS_AGE_ADULT;
}

/** \brief getAgeCategoryPediatricsCost returns the Pediatrics Cost category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the pediatric cost age categories
 **/
SimContext::PEDS_COST_AGE StateUpdater::getAgeCategoryPediatricsCost(int ageMonths) {
	if(!simContext->getPedsInputs()->enablePediatricsModel)
		return SimContext::PEDS_COST_AGE_ADULT;

	if (ageMonths < 24)
		return SimContext::PEDS_COST_AGE_1;
	if (ageMonths < 60)
		return SimContext::PEDS_COST_AGE_2;
	if (ageMonths < 156)
		return SimContext::PEDS_COST_AGE_3;
	if(ageMonths<216)
		return SimContext::PEDS_COST_AGE_4;
	else
		return SimContext::PEDS_COST_AGE_ADULT;
}

/** \brief getAgeCategoryPediatricsARTCost returns the Pediatrics ART Cost category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the pediatric cost ART age categories
 **/
SimContext::PEDS_ART_COST_AGE StateUpdater::getAgeCategoryPediatricsARTCost(int ageMonths) {
	if(!simContext->getPedsInputs()->enablePediatricsModel)
		return SimContext::PEDS_ART_COST_AGE_ADULT;

	if (ageMonths < 6)
		return SimContext::PEDS_ART_COST_AGE_5MTH;
	if (ageMonths < 12)
		return SimContext::PEDS_ART_COST_AGE_11MTH;
	if (ageMonths < 36)
		return SimContext::PEDS_ART_COST_AGE_2YR;
	if (ageMonths < 60)
		return SimContext::PEDS_ART_COST_AGE_4YR;
	if (ageMonths < 96)
		return SimContext::PEDS_ART_COST_AGE_7YR;
	if(ageMonths < 156)
		return SimContext::PEDS_ART_COST_AGE_12YR;
	else
		return SimContext::PEDS_ART_COST_AGE_ADULT;
}
/** \brief getAgeCategoryOutput returns the Output age category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the Output age categories
 **/
int StateUpdater::getAgeCategoryOutput(int ageMonths) {
	int ageYears = ageMonths / 12;
	if (ageYears < 20)
		return 0;
	if (ageYears <80)
		return (ageYears / 5)-3;
	return 13;
}
/** \brief getAgeCategoryInfant returns the infant age category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the age categories
 **/
int StateUpdater::getAgeCategoryInfant(int ageMonths) {
	if(!simContext->getPedsInputs()->enablePediatricsModel)
		return SimContext::PEDS_AGE_INFANT_NUM-1;

	if (ageMonths < 3)
		return SimContext::PEDS_AGE_2MTH;
	if (ageMonths < 6)
		return SimContext::PEDS_AGE_5MTH;
	if (ageMonths < 9)
		return SimContext::PEDS_AGE_8MTH;
	if (ageMonths < 12)
		return SimContext::PEDS_AGE_11MTH;
	if (ageMonths < 15)
		return SimContext::PEDS_AGE_14MTH;
	if (ageMonths < 18)
		return SimContext::PEDS_AGE_17MTH;
	else
		return SimContext::PEDS_AGE_INFANT_NUM-1;
}

/** \brief setPediatricState sets whether or not the patient is currently a Pediatric one 
 *	\param isPediatric a bool that indicates whether or not the patient is a Pediatric one
**/
void StateUpdater::setPediatricState(bool isPediatric) {
	patient->generalState.isPediatric = isPediatric;
} /* end setPediatricState */

/** \brief setAdolescentState sets whether or not the patient is an Adolescent
 *  \param isAdolescent a bool that indicates whether or not the patient is an Adolescent
 **/
void StateUpdater::setAdolescentState(bool isAdolescent) {
	patient->generalState.isAdolescent = isAdolescent;
} /* end setAdolescentState */

/** \brief getAgeCategoryAdolescent returns the adolescent age category for the given age
 *
 *
 * \return an integer representing the index of the age categories
 **/
int StateUpdater::getAgeCategoryAdolescent() {
	int ageMonths = patient->generalState.ageMonths;
	int ageYrs = ageMonths/12;
	for(int i = 0; i <SimContext::ADOLESCENT_NUM_AGES-1;i++)
		if (ageYrs < simContext->getAdolescentInputs()->ageBounds[i])
			return i;

	return SimContext::ADOLESCENT_NUM_AGES-1;
}



/** \brief getAgeCategoryAdolescentART returns the adolescent ART age category for the given age
 *
 *
 * \return an integer representing the index of the age categories
 **/
int StateUpdater::getAgeCategoryAdolescentART(int artLineNum) {
	int ageMonths = patient->generalState.ageMonths;
	int ageYrs = ageMonths/12;

	const SimContext::AdolescentARTInputs *ayaART = simContext->getAdolescentARTInputs(artLineNum);
	for(int i = 0; i <SimContext::ADOLESCENT_NUM_ART_AGES-1;i++)
		if (ageYrs < ayaART->ageBounds[i])
			return i;

	return SimContext::ADOLESCENT_NUM_ART_AGES-1;
}

/** \brief getAgeCategoryInfantHIVProphCost returns the infant proph cost age category for the given age
 *
 * \param ageMonths an integer representing the exact age of which to return the age category
 *
 * \return an integer representing the index of the age categories
 **/
int StateUpdater::getAgeCategoryInfantHIVProphCost(int ageMonths) {
	if (ageMonths < 3)
		return 0;
	if (ageMonths < 6)
		return 1;
	if (ageMonths < 9)
		return 2;
	if (ageMonths < 12)
		return 3;
	if (ageMonths < 15)
		return 4;
	if (ageMonths < 18)
		return 5;
	if (ageMonths < 24)
		return 6;
	else
		return 7;
}


/** \brief getTimeSummary returns a non-const pointer to the TimeSummary object for the current time period,
 *
 * \return the TimeSummary for the current time; creates a new one if needed or returns null if not keeping longitudinal stats
 **/
RunStats::TimeSummary *StateUpdater::getTimeSummaryForUpdate() {
	int timePeriod;
	SimContext::LONGIT_SUMM_TYPE longitLevel = simContext->getRunSpecsInputs()->longitLoggingLevel;

	if (longitLevel != SimContext::LONGIT_SUMM_NONE) {
		if (longitLevel == SimContext::LONGIT_SUMM_YR_DET)
			timePeriod = patient->generalState.monthNum / 12;
		else
			timePeriod = patient->generalState.monthNum;

		if (timePeriod < (int) runStats->timeSummaries.size())
			return runStats->timeSummaries[timePeriod];

		// Add new time summaries until reach desired time period
		for (int i = (int) runStats->timeSummaries.size(); i <= timePeriod; i++) {
			RunStats::TimeSummary *currTime = new RunStats::TimeSummary();
			runStats->initTimeSummary(currTime);
			currTime->timePeriod = i;
			runStats->timeSummaries.push_back(currTime);
		}
		return runStats->timeSummaries[timePeriod];
	}

	return NULL;
} /* end getTimeSummaryForUpdate */

/** \brief getOrphanStatsForUpdate returns a non-const pointer to the OrphanStats object for the current time period,
 *
 * \return the OrphanStats for the current time; creates a new one if needed
 **/
RunStats::OrphanStats *StateUpdater::getOrphanStatsForUpdate() {
	int timePeriod = patient->generalState.monthNum;

	if (timePeriod < (int) runStats->orphanStats.size())
		return runStats->orphanStats[timePeriod];

	// Add new orphan stats until reach desired time period
	for (int i = (int) runStats->orphanStats.size(); i <= timePeriod; i++) {
		RunStats::OrphanStats *currTime = new RunStats::OrphanStats();
		runStats->initOrphanStats(currTime);
		currTime->timePeriod = i;
		runStats->orphanStats.push_back(currTime);
	}
	return runStats->orphanStats[timePeriod];

} /* end getOrphanStatsForUpdate */

/** \brief incrementCostsCommon increases all general cost stats that are independent of the type of cost
 *
 * overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *
 * This cost will be discounted
 *
 *  \param cost a double representing the cost to be added
 *	\param percent a double representing the percent of the cost to apply
 **/
void StateUpdater::incrementCostsCommon(double cost, double percent) {
	double discountFactor = patient->generalState.discountFactor;
	double discountedCost = cost * discountFactor * percent;
	double undiscountedCost = cost * percent;

	// Update the patient state costs
	patient->generalState.costsDiscounted += discountedCost;

	for( int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double multDiscountedCost = cost * patient->generalState.multDiscFactorCost[i]*percent;
		patient->generalState.multDiscCosts[i] += multDiscountedCost;
	}

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Life months
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costs += undiscountedCost;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costs += discountedCost;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costs += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costs += discountedCost;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costs += undiscountedCost;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costs += discountedCost;
			}
		}
	}

	// Update the run stats overall costs
	if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {
		SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
		SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
		SimContext::HVL_STRATA setpointHVL = patient->diseaseState.setpointHVLStrata;
		runStats->overallCosts.costsTotalCD4[cd4Strata] += discountedCost;
		if (patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N)
			runStats->overallCosts.costsNoOIHistoryCD4[cd4Strata] += discountedCost;
		else
			runStats->overallCosts.costsOIHistoryCD4[cd4Strata] += discountedCost;
		runStats->overallCosts.costsHVL[hvlStrata] += discountedCost;
		runStats->overallCosts.costsHVLSetpoint[setpointHVL] += discountedCost;
	}
	SimContext::HIV_ID detectedState = patient->monitoringState.isDetectedHIVPositive ? SimContext::HIV_ID_IDEN : SimContext::HIV_ID_UNID;
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
		detectedState = SimContext::HIV_ID_NEG;
	runStats->overallCosts.costsHIVState[detectedState] += discountedCost;
	runStats->overallCosts.costsGender[patient->generalState.gender] += discountedCost;

	// Update the longitudinal cost statistics
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->totalMonthlyCohortCosts += discountedCost;
	}
} /* end incrementCostsCommon */

/** \brief incrementCostsCommon increases all general cost stats that are independent of the type of cost
 *
 * overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *
 * This cost will be discounted
 *
 *  \param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *	\param percent a double representing the percent of the cost to apply
 *	\param multiplier a double defaulting to 1, used when updating general medicine costs (see StateUpdater::incrementCostsGeneralMedicine)
 **/
void StateUpdater::incrementCostsCommon(const double *costArray, double percent, double multiplier) {
	double discountFactor = patient->generalState.discountFactor;
	double discountedCostArray[SimContext::COST_NUM_TYPES];
	double discountedCostTotal = 0;
	double undiscountedCostArray[SimContext::COST_NUM_TYPES];
	double undiscountedCostTotal = 0;

	// Calculated the discounted cost of each type and the total costs
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostArray[i] = costArray[i] * discountFactor * percent * multiplier;
		discountedCostTotal += discountedCostArray[i];

		undiscountedCostArray[i] = costArray[i] * percent * multiplier;
		undiscountedCostTotal += undiscountedCostArray[i];
	}



	// Update the patient state costs
	patient->generalState.costsDiscounted += discountedCostTotal;

	for( int i = 0; i < SimContext::NUM_DISCOUNT_RATES; i++){
		double multDiscCostTotal = 0;
		double multDiscCostArray[SimContext::COST_NUM_TYPES];
		for (int j = 0; j < SimContext::COST_NUM_TYPES; j++){
			multDiscCostArray[j] = costArray[j]*patient->generalState.multDiscFactorCost[i]*percent*multiplier;
			multDiscCostTotal += multDiscCostArray[j];
		}
		patient->generalState.multDiscCosts[i] += multDiscCostTotal;
	}

	// Update the sum of costs by each cost type
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		runStats->overallCosts.totalUndiscountedCosts[i] += costArray[i] * percent * multiplier;
		runStats->overallCosts.totalDiscountedCosts[i] += discountedCostArray[i];
	}

	//Update coststats variables
	for (int i = 0; i < SimContext::COST_SUBGROUPS_NUM; i++){
		//Life months
		if (patient->generalState.costSubgroups[i]){
			costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_ALL][i].costs += undiscountedCostTotal;
			costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_ALL][i].costs += discountedCostTotal;

			if (patient->monitoringState.hasObservedCD4){
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costs += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][patient->monitoringState.currObservedCD4Strata][i].costs += discountedCostTotal;
			}
			else{
				costStats->allStats[SimContext::COST_REPORT_UNDISCOUNTED][SimContext::COST_CD4_NONE][i].costs += undiscountedCostTotal;
				costStats->allStats[SimContext::COST_REPORT_DISCOUNTED][SimContext::COST_CD4_NONE][i].costs += discountedCostTotal;
			}
		}
	}

	// Update the run stats overall costs
	if (patient->diseaseState.infectedHIVState != SimContext::HIV_INF_NEG) {
		SimContext::CD4_STRATA cd4Strata = patient->diseaseState.currTrueCD4Strata;
		SimContext::HVL_STRATA hvlStrata = patient->diseaseState.currTrueHVLStrata;
		SimContext::HVL_STRATA setpointHVL = patient->diseaseState.setpointHVLStrata;
		runStats->overallCosts.costsTotalCD4[cd4Strata] += discountedCostTotal;
		if (patient->diseaseState.typeTrueOIHistory == SimContext::HIST_EXT_N)
			runStats->overallCosts.costsNoOIHistoryCD4[cd4Strata] += discountedCostTotal;
		else
			runStats->overallCosts.costsOIHistoryCD4[cd4Strata] += discountedCostTotal;
		runStats->overallCosts.costsHVL[hvlStrata] += discountedCostTotal;
		runStats->overallCosts.costsHVLSetpoint[setpointHVL] += discountedCostTotal;
	}
	SimContext::HIV_ID detectedState = patient->monitoringState.isDetectedHIVPositive ? SimContext::HIV_ID_IDEN : SimContext::HIV_ID_UNID;
	if (patient->diseaseState.infectedHIVState == SimContext::HIV_INF_NEG)
		detectedState = SimContext::HIV_ID_NEG;
	runStats->overallCosts.costsHIVState[detectedState] += discountedCostTotal;
	runStats->overallCosts.costsGender[patient->generalState.gender] += discountedCostTotal;

	// Update the longitudinal cost statistics
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->totalMonthlyCohortCosts += discountedCostTotal;
		// Update the sum of costs by each cost type
		for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
			currTime->totalMonthlyCohortCostsType[i] += discountedCostArray[i];
		}
	}
} /* end incrementCostsCommon */


/** \brief incrementCostsTBCommon increases all tb cost stats that are independent of the type of cost
 *
 * overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *
 * This cost will be discounted
 *
 *  \param cost a double representing the cost to be added
 *	\param percent a double representing the percent of the cost to apply
 **/
void StateUpdater::incrementCostsTBCommon(double cost, double percent) {
	double discountFactor = patient->generalState.discountFactor;
	double discountedCost = cost * discountFactor * percent;

	// Update the patient state costs
	runStats->overallCosts.costsTBTotal += discountedCost;

	// Update the longitudinal cost statistics
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->totalMonthlyTBCohortCosts += discountedCost;
	}
} /* end incrementCostsTBCommon for single value */

/** \brief incrementCostsTBCommon increases all TB cost stats that are independent of the type of cost
 *
 * overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs
 *
 * This cost will be discounted
 *
 *  \param costArray a pointer to an array of doubles of size SimContext::COST_NUM_TYPES representing the costs to be added
 *	\param percent a double representing the percent of the cost to apply
 **/
void StateUpdater::incrementCostsTBCommon(const double *costArray, double percent, double multiplier) {
	double discountFactor = patient->generalState.discountFactor;
	double discountedCostArray[SimContext::COST_NUM_TYPES];
	double discountedCostTotal = 0;

	// Calculated the discounted cost of each type and the total costs
	for (int i = 0; i < SimContext::COST_NUM_TYPES; i++) {
		discountedCostArray[i] = costArray[i] * discountFactor * percent * multiplier;
		discountedCostTotal += discountedCostArray[i];
	}

	// Update the patient state costs
	runStats->overallCosts.costsTBTotal += discountedCostTotal;

	// Update the longitudinal cost statistics
	RunStats::TimeSummary *currTime = getTimeSummaryForUpdate();
	if (currTime) {
		currTime->totalMonthlyTBCohortCosts += discountedCostTotal;
	}
} /* end incrementCostsTBCommon for cost array */
