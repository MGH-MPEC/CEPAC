#pragma once

#include "include.h"

/**
	The StateUpdater class is the base class for all simulation updater classes in the model.
	All of its child classes should implement the performInitialUpdates and performMonthlyUpdates
	virtual functions to perform the patient initialization and subsequent monthly updates to
	both the patient state and statistics objects.  If these functions are omitted in a
	given child class, the parent functions defined in this class will be used instead.
	The child classes cannot directly modify the patient or statistics information, instead they
	must use the update functions provided in this parent class (which is a friend class to both
	Patient and RunStats).
*/
class StateUpdater
{
public:
	/* Constructor and Destructor */
	StateUpdater(Patient *patient);
	virtual ~StateUpdater(void);

	/**
	 *  performInitialUpdates perform all of the state and statistics updates upon patient creation
	 **/
	virtual void performInitialUpdates();
	/**
	 *  performMonthlyUpdates perform all of the state and statistics updates for a simulated month
	 **/
	virtual void performMonthlyUpdates();
	/**
	 *  changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model
	 **/
	void setSimContext(SimContext *newSimContext);

protected:

	/** Pointer  to the patient state */
	Patient *patient;
	/** Pointer to the simulation context (i.e. .in file information) */
	SimContext *simContext;
	/** Pointer to the run stats object (i.e. the .out file information) */
	RunStats *runStats;
	/** Pointer to the cost stats object (i.e. the .cout file information) */
	CostStats *costStats;
	/** Pointer to the tracer (i.e. the trace file information) */
	Tracer *tracer;

	/**
	 * \class StateUpdater
	 * The protected functions are updater functions that are invoked by the StateUpdater child classes,
		child classes cannot directly modify state and must use these functions */

	/* initializePatient initializes the patients basic state */
	void initializePatient(int patientNum, bool tracingEnabled);
	/* setPatientAgeGender set the patients age and gender */
	void setPatientAgeGender(SimContext::GENDER_TYPE gender, int ageMonths);
	/* setInitialARTState sets the initial ART state to not be on ART */
	void setInitialARTState();
	/* setInitialProphState sets the initial proph state to not be on any prophs */
	void setInitialProphState();
	/* setInitialTBProphState sets the initial TB proph state to not be on any TB prophs */
	void setInitialTBProphState();
	/* setInitialTBTreatmentState sets the initial TB treatment state to not be on TB treatment */
	void setInitialTBTreatmentState();
	/* countInitialTBState counts the TB state/strain at entry -- only should be called after these have been set! */
	void countInitialTBState();
	/* incrementMonth updates the simulation month number and patient age */
	void incrementMonth();
	/* incrementDiscountFactor adjusts the discounting factor */
	void incrementDiscountFactor(double amount);
	/* incrementMultDiscountFactor adjusts the discounting factor */
	void incrementMultDiscountFactor(double amountCost, double amountBenefit, int i);
	/* resetQOL resets the quality of life factor back to the background value for the patient's age in years and gender*/
	void resetQOL();
	/* accumulateQOLModifier accumulates the QOL in one of 4 ways, defined by the user. It performs operations that modify the existing value by the new amount */
	void accumulateQOLModifier(double amount);
	/* finalizeQOLValue checks that the patient's QOL value is between 0 and 1 and bounds it if necessary before it is used*/
	void finalizeQOLValue();
	/* setHIVIncReducMultiplier sets the current HIV incidence reduction multiplier if enabled */
	void setHIVIncReducMultiplier(double reducMult);
	/* setInfectedHIVState sets the patients to being HIV infected and updates statistics */
	void setInfectedHIVState(SimContext::HIV_INF infectedState, bool isInitial, bool resetTimeOfInfection, bool isHighRisk = true);
	/* setInfectedPediatricsHIVState sets the pediatrics HIV state*/
	void setInfectedPediatricsHIVState(SimContext::PEDS_HIV_STATE hivState, bool isInitial);
	/* setMaternalHIVState sets the maternal state for pediatrics*/
	void setMaternalHIVState(SimContext::PEDS_MATERNAL_STATUS hivState, bool motherBecameInfected, bool isInit);
	/* setInitalMaternalState sets the maternal HIV state for pediatrics at init */
	void setInitialMaternalState();
	/* rollForMaternalInfection determines whether an HIV-negative mother becomes infected with HIV, and if so, updates the patient and maternal states */
	void rollForMaternalInfection();
	/* setMaternalStatusKnown sets whether the maternal state is known*/
	void setMaternalStatusKnown(bool isKnown);
	/* setMaternalARTStatus sets whether the mother is on ART*/
	void setMaternalARTStatus(bool isOnART, bool isSuppressed, bool suppressionKnown, bool isInitial);
	/* setMaternalHVLStatus sets whether the mother is lowHVl or high HVL*/
	void setMaternalHVLStatus(bool isLow, bool isInitial = false);
	/* updatePedsNeverExposed performs updates on HIV negative children who were never exposed to HIV by their mother during early childhood*/
	void updatePedsNeverExposed(bool atDeath = false);
	/* updatePedsHIVExposureStats performs statistics on whether HIV negative children are exposed or unexposed to HIV*/
	void updatePedsHIVExposureStats(SimContext::PEDS_MATERNAL_STATUS momHIVState);	
	/* setBreastfeedingStatus sets the breastfeeding status for pediatrics */
	void setBreastfeedingStatus(SimContext::PEDS_BF_TYPE bfType);
	/* setBreastfeedingStopAge sets the age at which to stop breastfeeding for pediatrics */
	void setBreastfeedingStopAge(int stopAge);
	/* setAgeOfSeroreversion for pediatrics */
	void setAgeOfSeroreversion(int age);
	/* resetNumMissedVisitsEID sets the number of missed eid visits to 0 */
	void resetNumMissedVisitsEID();
	/* incrementNumMissedVisitsEID increases the number of missed eid visits by 1 */
	void incrementNumMissedVisitsEID();
	/* setCareState sets the care status of patients*/
	void setCareState(SimContext::HIV_CARE typeCare);
	/* setDetectedHIVState sets the patients to being detected as HIV positive and updates statistics */
	void setDetectedHIVState(bool isDetected, SimContext::HIV_DET typeDetection = SimContext::HIV_DET_UNDETECTED, SimContext::OI_TYPE oiType = SimContext::OI_NONE);
	/* setLinkedState sets the patients method of linking to care*/
	void setLinkedState(bool isLinked, SimContext::HIV_DET typeDetection = SimContext::HIV_DET_UNDETECTED);
	/* setFalsePositiveStatus sets whether the patient is false positive and whether they are linked or not */
	void setFalsePositiveStatus(bool isFalsePositve, bool isLinked);
	/* setCanReceiveEID sets the status of whether the patient can get EID test visits */
	void setCanReceiveEID(bool canReceiveEID);
	/* removePendingEIDTestsAndResults gets rid of pending test results and scheduled confirmatory EID tests */
	void removePendingEIDTestsAndResults(bool removeOITests = true);
	/* updateHIVTestingStats updates all statistics after an HIV testing event */
	void updateHIVTestingStats(bool acceptTest, bool returnResults, bool isPositive);
	/* updateLabStagingStats updates all statistics after an Lab Staging testing event */
	void updateLabStagingStats(bool acceptTest, bool returnResults, bool hasLinked);
	/* setHIVTestingParams sets the interval and acceptance probability for HIV testing */
	void setHIVTestingParams(int intervalIndex, int acceptanceProbIndex);
	/* setInitialPrEPParams sets the initial PrEP parameters */
	void setInitialPrEPParams();
	/* updatePrEPProbLogging logs the monthly probability of PrEP uptake */
	void updatePrEPProbLogging(double prepProb, SimContext::HIV_BEHAV risk);
	/* setPrEP sets whether the patient is on PrEP and updates statistics about PrEP takeup and dropout */
	void setPrEP(bool hasPrEP, bool isDropout = false);
	/* scheduleHIVTest sets the month of the next HIV test */
	void scheduleHIVTest(bool hasNext, int monthNum = 0);
	/* scheduleCD4Test sets the month of the next CD4 test */
	void scheduleCD4Test(bool hasNext, int monthNum = 0);
	/* scheduleHVLTest sets the month of the next HVL test */
	void scheduleHVLTest(bool hasNext, int monthNum = 0);

	/* performEIDTest handles doing an eid base test or confirmatory test*/
	void performEIDTest(int baseAssay, int testAssay, SimContext::EID_TEST_TYPE testType, bool triggeredByOI, bool isNonMaternal);
	/* rollForEIDTestResult handles getting a result for eid tests*/
	bool rollForEIDTestResult(int assayNum, SimContext::EID_TEST_TYPE testType);
	/* rollForEIDLinkage handles linkage to care from EID */
	void rollForEIDLinkage (int assayNum, bool triggeredByOI);

	/* addPendingResultEIDTest adds a result to the number of pending eid test results */
	void addPendingResultEIDTest(int baseAssay, int testAssay, SimContext::EID_TEST_TYPE testType, bool result, int monthOfReturn, bool willBeReturnedToPatient, bool triggeredByOI);
	/* performEIDResultReturnUpdates processes the return of EID test results and rolls for linkage to care */
	void performEIDResultReturnUpdates();
	/* performEIDFirstConfirmatoryTests processes first confirmatory tests that are scheduled */
	void performEIDFirstConfirmatoryTests();
	/* performEIDFirstConfirmatoryTests processes second confirmatory tests that are scheduled */
	void performEIDSecondConfirmatoryTests();

	/* scheduleEIDConfirmatoryTest schedules the next confirmatory test after a positive result for that line of EID tests */
	void scheduleEIDConfirmatoryTest(int baseAssay, int testAssay, SimContext::EID_TEST_TYPE testType, int month, bool triggeredByOI);
	/* setMostRecentPositiveEIDTest sets the most recent positive EID test result that did not link to care*/
	void setMostRecentPositiveEIDTest(bool hasMostRecent, int baseAssay, SimContext::EID_TEST_TYPE testType);
	/* setMostRecentNegativeEIDTest sets the most recent negative EID test result and the month of the test*/
	void setMostRecentNegativeEIDTest(bool hasMostRecent);

	/* setMissedEIDTest sets the most recent missed scheduled EID test due to a missed visit*/
	void setMissedEIDTest(bool hasMissed, int baseAssay);

	/* setInfantHIVProphEffProb sets the prob of an infant HIV proph dose being effective*/
	void setInfantHIVProphEffProb(int prophNum, double probEff);
	/* setInfantHIVProph sets the infant HIV proph state, efficacy of dose and  month of proph*/
	void setInfantHIVProph(int prophNum, bool hasProph , bool isEff, bool isInit = false);
	/* setInfantHIVProphMajorToxicty sets the toxicity from infant HIV proph*/
	void setInfantHIVProphMajorToxicity(int prophNum, bool hasTox);

	/* setClinicVisitType sets the conditions for a clinic visit and available treatments */
	void setClinicVisitType(SimContext::CLINIC_VISITS visitType, SimContext::THERAPY_IMPL treatmentType);

	/* setAdherenceInterventionState sets whether the patient is on an adherence intervention and the index of the intervention*/
	void setAdherenceInterventionState(bool isOnIntervention, int interventionIndex = SimContext::NOT_APPL, int duration = 0);
	/* setNextAdherenceIntervention sets the index of the next adherence intervention */
	void setNextAdherenceIntervention(int interventionIndex);
	/* setResponseBaseline sets the baseline propensity to respond coeffecient */
	void setResponseBaseline(double baseline);
	/* setPreARTResponseBase sets the baseline propensity to respond coeffecient for PreART*/
	void setPreARTResponseBase(double baseline);
	/* setARTResponseCurrRegimenBase sets the propensity to respond coeffecient for the current regimen without any adherence interventions*/
	void setARTResponseCurrRegimenBase(double responseLogit, double responseRegimenIncrLogit, double responseLogitPreIncr, bool enableDuration, int duration);
	/* setAdherenceInterventionResponseAdjustment sets the adjustment to logit for propensity if adherence intervention is active */
	void setAdherenceInterventionResponseAdjustment(double adjustment);
	/* setCD4ResponseType sets the predisposed CD4 ART response type of the patient */
	void setCD4ResponseType(SimContext::CD4_RESPONSE_TYPE responseType);
	/* setRiskFactor sets whether or not the patient has risk factor x */
	void setRiskFactor(int riskNum, bool hasRisk, bool isInitial = false);
	/* setTransmRiskCategory sets the transmission risk category for the patient */
	void setTransmRiskCategory(SimContext::TRANSM_RISK risk);
	/* Sets the variable hadChanceCD4Test in patient monitoring state*/
	void setChanceCD4Test(bool hadChance);
	/* Sets the variable hadChanceHVLTest in patient monitoring state*/
	void setChanceHVLTest(bool hadChance);
	/* Sets whether the patient is able to get CD4 tests */
	void setCD4TestingAvailable(bool isAvail);
	/*Schedules the initial cd4 tests during clinic visit*/
	void scheduleInitialCD4Test(int monthNum);
	/*Schedules the initial HVL tests during clinic visit*/
	void scheduleInitialHVLTest(int monthNum);
	/* scheduleInitialClinicVisit sets the month of initial clinic visit, CD4 test, and HVL test */
	void scheduleInitialClinicVisit(int monthOfVisit = SimContext::NOT_APPL);
	/* scheduleRegularClinicVisit sets the month of the next regularly scheduled clinic visit */
	void scheduleRegularClinicVisit(bool hasNext, int monthNum = 0,bool scheduleInitialCD4=true,bool scheduleInitialHVL=true);
	/* scheduleEmergencyClinicVisit sets the month of the next emergency clinic visit */
	void scheduleEmergencyClinicVisit(SimContext::EMERGENCY_TYPE emergencyType, int monthNum = 0,bool scheduleInitialCD4=true,bool scheduleInitialHVL=true);
	/* resetCliniVisitState resets state keeping track of event since the last clinic visit */
	void resetClinicVisitState(bool isInitial = false);
	/* incrementNumClinicVisits increments the total number of clinic visits */
	void incrementNumClinicVisits();
	/* incrementNumObservedOIs increments the patients observed OIs and statistics */
	void incrementNumObservedOIs(SimContext::OI_TYPE oiType, int numObserved);
	/* setCurrLTFUState updates the state and statistics for a patient being LTFU or RTC */
	void setCurrLTFUState(SimContext::LTFU_STATE ltfuState);
	/* startNextARTRegimen updates the state to begin the next ART treatment regimen */
	void startNextARTRegimen(bool isResupp = false);
	/* startNextARTSubRegimen updates the state to begin the next ART treatment subregimen */
	void startNextARTSubRegimen(int nextSubRegimen);
	/* setCurrARTEfficacy updates the destined efficacy of the ART regimen */
	void setCurrARTEfficacy(SimContext::ART_EFF_TYPE efficacyType, bool isInitial);
	/* setResuppEfficacy updates the efficacy each time a resuppression is attempted */
	void setResuppEfficacy(SimContext::ART_EFF_TYPE efficacyType);
	/* setCurrARTResponse sets the calculated ART propensity to respond and response type */
	void setCurrARTResponse(double propRespond);
	/* setTargetHVLStrata updates the target HVL while on ART or post ART */
	void setTargetHVLStrata(SimContext::HVL_STRATA targetHVL);
	/* setPatientNatHistSlopePerc sets the natural history cd4 decline increment Perc*/
	void setPatientNatHistSlopePerc(double cd4Perc);
	/* setCurrRegimenCD4Slope sets the CD4 slope for the current ART regimen */
	void setCurrRegimenCD4Slope(double cd4Slope);
	/* setCurrRegimenCD4PercentageSlope sets the CD4 percentage slope for the current ART regimen */
	void setCurrRegimenCD4PercentageSlope(double cd4PercSlope);
	/* setSuccessfulARTRegimen updates the state to indicate that the patient has been successful on the ART regimen and the month of initial success */
	void setSuccessfulARTRegimen();
	/* setCD4EnvelopeRegimen initializes the specified CD4 envelope type */
	void setCD4EnvelopeRegimen(SimContext::ENVL_CD4_TYPE envelopeType, int artLineNum, int monthOfStart = -1);
	/* setCD4EnvelopeSlope updates the slope used for the specified CD4 envelope type */
	void setCD4EnvelopeSlope(SimContext::ENVL_CD4_TYPE envelopeType, double cd4Slope);
	/* incrementCD4Envelope increments the specified CD4 envelope's level according to hypothetical ART success */
	void incrementCD4Envelope(SimContext::ENVL_CD4_TYPE envelopeType, double changeCD4);
	/* setCurrARTObservedFailure updates the state to begin the next ART treatment regimen */
	void setCurrARTObservedFailure(SimContext::ART_FAIL_TYPE failType);
	/* stopCurrARTRegimen updates the state to end the next ART treatment regimen */
	void stopCurrARTRegimen(SimContext::ART_STOP_TYPE stopType);
	/* setNextARTRegimen updates the next ART regimen that is available for use */
	void setNextARTRegimen(bool hasNext, int artLineNum = 0, bool isResupp = false);
	/* incrementMonthsUnsuccessfulART increments the number of months on failed ART by HVL */
	void incrementMonthsUnsuccessfulART();
	/* addARTToxicityEffect adds the occurrence of a new toxicity to the active effects list */
	void addARTToxicityEffect(SimContext::ART_TOX_SEVERITY severity, int toxNum, int timeToTox);
	/* removeARTToxicityEffect removes the specified toxicity from the active effects list */
	void removeARTToxicityEffect(list<SimContext::ARTToxicityEffect>::const_iterator &toxIter);
	/* setARTToxicity updates the patient state and stats for the occurrence of a toxicity */
	void setARTToxicity(const SimContext::ARTToxicityEffect &toxEffect);
	/* incrementARTFailedCD4Tests increments the number of failed CD4 tests counting towards ART failure */
	void incrementARTFailedCD4Tests();
	/* resetARTFailedCD4Tests sets the number of failed CD4 tests counting towards ART failure back to 0 */
	void resetARTFailedCD4Tests();
	/* incrementARTFailedHVLTests increments the number of failed CD4 tests counting towards ART failure */
	void incrementARTFailedHVLTests();
	/* resetARTFailedHVLTests sets the number of failed CD4 tests counting towards ART failure back to 0 */
	void resetARTFailedHVLTests();
	/* incrementARTFailedOIs increments the number of observed OIs counting towards ART failure */
	void incrementARTFailedOIs();
	/* resetARTFailedOIs resets the number of observed OIs counting towards ART failure */
	void resetARTFailedOIs();
	/* setCurrSTIState updates the STI state for the current ART regimen */
	void setCurrSTIState(SimContext::STI_STATE newSTIState);
	/* setProphNonCompliance sets whether or not the patient complies with proph */
	void setProphNonCompliance(bool isNonCompliant);
	/* startNextProph updates state to beginning using next proph */
	void startNextProph(SimContext::OI_TYPE oiType);
	/* stopCurrProph updates state to stop using current proph */
	void stopCurrProph(SimContext::OI_TYPE oiType);
	/* setNextProph updates type of proph and proph num to be used for the OI */
	void setNextProph(bool hasNext, SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType, int prophNum);
	/* setProphToxicity records the toxicity and updates stats */
	void setProphToxicity(bool isMajor, SimContext::OI_TYPE oiType);
	/* setProphResistance updates the flag to indicate that OI proph resistance has occurred */
	void setProphResistance(SimContext::OI_TYPE oiType);
	// TB-related
	/* setTBDiseaseState updates the TB disease state */
	void setTBDiseaseState(SimContext::TB_STATE newTBState, bool isInfection, SimContext::TB_INFECT infectType, SimContext::TB_STATE previousState);
	/* setTBResistanceStrain updates the TB disease drug resistance */
	void setTBResistanceStrain(SimContext::TB_STRAIN newTBStrain);
	/* setObservedTBResistanceStrain updates the observed TB disease drug resistance strain */
	void setObservedTBResistanceStrain(bool hasObserved, SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS);
	/* setObservedHistTBResistanceStrain updates the observed history of TB disease drug resistance strain */
	void setObservedHistTBResistanceStrain(bool hasHist, SimContext::TB_STRAIN obsvStrain = SimContext::TB_STRAIN_DS);

	/* setTBUnfavorableOutcome sets unfavorable outcome flag for tb */
	void setTBUnfavorableOutcome(SimContext::TB_UNFAVORABLE outcome);
	/* setTBTracker updates the tracker variable for the three tb trackers */
	void setTBTracker(SimContext::TB_TRACKER tracker, bool trackerStatus);
	/* setTBCareState updates the TB care state */
	void setTBCareState(SimContext::TB_CARE newTBCareState);
	/* setTBTestResult sets the test result for a TB test */
	void setTBTestResult(SimContext::TB_DIAG_STATUS result, int testIndex);
	/* startTBTesting sets the index of the current test to the beginning, identifies the next test (if any), and initalizes all test results and pickup times to none */
	void startTBTesting();
	/* resetTBTesting sets the index of the current test to N/A, and the next test to the first test (or to N/A if none is available) and removes all results */
	void resetTBTesting(bool removeObsvStrain = true, bool removePendingDSTResult = true, bool isInit = false);
	/* setCurrTBTestIndex sets the index of the current TB test in the chain the patient is on. -1 if they are not on a test*/
	void setCurrTBTestIndex(int index);
	/* setNextTBTestIndex sets the index of the next TB test in the chain the patient is on. -1 if no next test*/
	void setNextTBTestIndex(int index, bool isStartChain, bool patientEverTreatedCategory = false);
	/* performTBTest performs a TB test*/
	bool performTBTest(int testNum, bool performDST);
	/* scheduleTBTestResultPickup adds a pending result of a TB test to be picked up.*/
	void setTBTestResultPickup(bool hasResult, int testNum, bool willPickupResult, SimContext::TB_DIAG_STATUS result, int monthOfPickup, bool resetTests, bool willDoDST);
	/* setTBDSTTestResultPickup adds a pending result of a TB DST test to be picked up.*/
	void setTBDSTTestResultPickup(bool hasResult, SimContext::TB_STRAIN result, int monthOfPickup);
	/* updateTBDiagnosticResults updates the statistics for final diagnostic TB results*/
	void updateTBDiagnosticResults(bool result);
	/* updateDSTResults updates the statisctics for DST test*/
	void updateDSTResults(SimContext::TB_STRAIN strain);

	/* performTBDiagnosis determines whether a patient is diagnosed with TB, either automatically at model start or by interpreting results at the end of the testing chain, and manages linkage to TB care */
	void performTBDiagnosis(int currIndex, bool atInit = false);

	/* setTBProphEligibility sets whether the patient is eligible for TB Proph */
	void setTBProphEligibility(bool isEligible, bool hasRolledEligibility);
	/* startNextTBProph updates state to beginning using next TB proph */
	void startNextTBProph();
	/* stopCurrProph updates state to stop using current TB proph, and if the proph line is completed updates patient state and statistics */
	void stopCurrTBProph(bool isFinished = false);
	/* setNextTBProph updates proph num to be used next for TB */
	void setNextTBProph(bool hasNext, int prophIndex, int prophNum);
	/* scheduleNextTBProph updates the time lag for scheduling the next TB proph */
	void scheduleNextTBProph(int monthStart);
	/* unscheduleNextTBProph removes the next scheduled start of TB proph */
	void unscheduleNextTBProph();
	/* setTBProphToxicity records the TB proph toxicity */
	void setTBProphToxicity(bool isMajor);
	/* startNextTBTreatment updates the patient state and statistics to begin the next TB treatment */
	void startNextTBTreatment();
	/* stopCurrTBTreatment updates the patient state and statistics to stop the current TB treatment */
	void stopCurrTBTreatment(bool isFinished, bool isCured);
	/* startNextTBEmpiricTreatment updates the patient state and statistics to begin the next empiric TB treatment */
	void startEmpiricTBTreatment(int treatNum, int previousDuration = 0);
	/* stopEmpiricTBTreatment updates the patient state and statistics to stop the empiric TB treatment */
	void stopEmpiricTBTreatment(bool isFinished = false);
	/* transitionEmpiricToRealTreatment handles the change when patient links to TB care if they are on empiric TB treatment */
	void transitionEmpiricToRealTreatment();
	/* scheduleNextTBTreatment updates the patient's next scheduled TB treatment after events such as TB RTC and returning DST results */
	void scheduleNextTBTreatment(int treatNum, int monthStart, bool isRepeat = false, int previousDuration = 0, bool hasResult = false);
	/* unscheduleNextTBTreatment removes the next scheduled start of TB treatment */
	void unscheduleNextTBTreatment();
	/* increaseTBDrugResistance updates the patient state and stats for an increase in TB drug resistance */
	void increaseTBDrugResistance(bool fromTreatment);
	/* setTBTreatmentToxicity records the TB treatment toxicity */
	void setTBTreatmentToxicity(bool isMajor, int treatNum);
	/* setHadTBTreatmentMajorTox records whether the patient ever had major tox while on empiric or non-empiric TB treatment*/
	void setHadTBTreatmentMajorTox(bool hadMajor);
	/* setTBLTFU handles the patient being LTFU to TB care */
	void setTBLTFU();
	/* checkForTreatmentDefault handles the transition from active state to treatment default */
	void checkForTreatmentDefault();
	/* setTBRTC handles the patient Returning to TB care*/
	void setTBRTC(bool maxMonthsExceded = false);
	/* getTimeCatTBRTC gets the bucket of time that the patient has been lost to TB Care */
	int getTimeCatTBRTC(int timeSinceLost);
	/*  setTBSelfCure handles TB self-cure*/
	void setTBSelfCure(bool isSelfCured);
	/* setTBInitPolicyIntervalEligible sets eligibility for interval-based TB testing initiation policy */
	void setTBInitPolicyIntervalEligible(bool isEligible);

	/* setTrueCHRMsState updates the patients state for occurrence of CHRMs diseases */
	void setTrueCHRMsState(int chrmNum, bool hasCHRM, bool isInitial = false, int monthsStart = 0);
	/* setInitialOIHistory updates the patient state for initial OI history */
	void setInitialOIHistory(bool hasHistory[SimContext::OI_NUM]);
	/* setOIHistory updates the patient state for OI history, called at end of month
		since current acute OI should not count as history until the next month */
	void setOIHistory();
	/* setCurrTrueOI updates the patient state and statistics when an acute OI event occurs or resets back to none */
	void setCurrTrueOI(SimContext::OI_TYPE oiType);
	/* setCurrObservedOI updates the patient state when a patient's current true OI is observed or resets back to no observed OI */
	void setCurrObservedOI(bool isObserved);
	/* clearMortalityRisks clears the list of possible death risks for the month */
	void clearMortalityRisks();
	/* addMortalityRisk adds a new risk of death and its death rate ratio for the current month */
	void addMortalityRisk(SimContext::DTH_CAUSES causeOfDeath, double deathRateRatio, double costDeath = 0.0);	
	/* setCauseOfDeath updates the patient state to reflect that death has occurred */
	void setCauseOfDeath(SimContext::DTH_CAUSES causeOfDeath);
	/* setMaternalDeath updates the patient state for pediatric maternal death */
	void setMaternalDeath();
	/* Increases the total number of persons who have been initialized by one */
	void incrementCohortSize();
	/* updatePopulationStats updates the final population summary after a death occurs */
	void updatePopulationStats();
	/* AddPatientSummary creates a patient summary object and adds it to the patients vector */
	void addPatientSummary();
	/* setTrueCD4 updates the patients actual CD4 level and confines it within the bounds,
		also checks if it is the patients minimum CD4 value */
	void setTrueCD4(double newCD4, bool isInitial = false);
	/* setTrueCD4Percentage updates the pediatrics patients actual CD4 percentage */
	void setTrueCD4Percentage(double newCD4Perc, bool isInitial = false);
	/* setCD4MultOnARTFail updates the lag period and current cd4 multiplier for failed ART*/
	void setCD4MultOnARTFail(int monthOfNewCD4Decline,double newCD4Mult);
	/* setTrueHVLStrata set the patient's actual HVL strata to the given level */
	void setTrueHVLStrata(SimContext::HVL_STRATA newHVL);
	/* setSetpointHVLStrata sets the patients setpoint HVL level */
	void setSetpointHVLStrata(SimContext::HVL_STRATA newSetpoint);
	/* setObservedCD4 updates the patients observed CD4 level and confines it within the bounds */
	void setObservedCD4(bool isKnown, double cd4Value = 0.0, bool isLabStaging = false);
	/* setObservedCD4Percentage updates the patients observed CD4 percentage and confines it within the bounds */
	void setObservedCD4Percentage(bool isKnown, double cd4Percent = 0.0);
	/* setObservedHVLStrata updates the patients observed HVL strata */
	void setObservedHVLStrata(bool isKnown, SimContext::HVL_STRATA hvlStrata = SimContext::HVL_VLO);
	/* incrementNumCD4Tests increases the number of cd4 tests by 1 */
	void incrementNumCD4Tests();
	/* incrementNumHVLTests increases the number of HVL tests by 1 */
	void incrementNumHVLTests();
	/* incrementNumHIVTests increases the number of HIV tests by 1 */
	void incrementNumHIVTests();
	/* incrementNumLabStagingTests increases the number of Lab Staging tests by 1 */
	void incrementNumLabStagingTests();
	/* incrementCostsPrEP adds a PrEP cost to the patients total */
	void incrementCostsPrEP(double cost, double percent);
	/* finalizePrEPCostsByState adds the patient's final total for PrEP costs to the overall total for their PrEP state */
	void finalizePrEPCostsByState(bool isPrEPDropout);
	/* incrementCostsHIVTest adds an HIV testing cost to the patients total */
	void incrementCostsHIVTest(double cost);
	/* incrementCostsHIVMisc adds an HIV misc related cost to the patients total */
	void incrementCostsHIVMisc(double cost);
	/* incrementCostsLabStagingTest adds a Lab Staging testing cost to the patients total */
	void incrementCostsLabStagingTest(double cost);
	/* incrementCostsLabStagingMisc adds a Lab Staging misc related cost to the patients total */
	void incrementCostsLabStagingMisc(double cost);
	/* incrementCostsEIDTest adds a EID test cost to the patient's total */
	void incrementCostsEIDTest(double cost);
	/* incrementCostsInfantHIVProphDirect adds an Infant HIV Proph administration cost to the patient's total */
	void incrementCostsInfantHIVProphDirect(double cost);
    /* incrementCostsInfantHIVProphTox adds an Infant HIV Proph toxicity cost to the patient's total */
	void incrementCostsInfantHIVProphTox(double cost);
	/* incrementCostsEIDMisc adds a EID test misc cost to the patient's total */
	void incrementCostsEIDMisc(double cost);
	/* incrementCostsCD4Test adds the CD4 testing related costs to the patients total */
	void incrementCostsCD4Test(const double *costArray);
	/* incrementCostsHVLTest adds the HVL testing related costs to the patients total */
	void incrementCostsHVLTest(const double *costArray);
	/* incrementCostsClinicVisit adds the general clinic visit costs to the patients total */
	void incrementCostsClinicVisit(const double *costArray);
	/* incrementCostsEIDVisit adds a EID visit cost to the patient's total */
	void incrementCostsEIDVisit(double cost);
	/* incrementCostsART adds an ART treatment cost to the patients total */
	void incrementCostsART(int artLineNum, double cost);
	/* incrementCostsARTInit adds an initial ART treatment cost to the patients total */
	void incrementCostsARTInit(int artLineNum, double cost);
	/* incrementCostsARTMonthly adds a monthly ART treatment cost to the patients total */
	void incrementCostsARTMonthly(int artLineNum, double cost);
	/* incrementCostsInterventionInit adds the init costs for starting an intervention */
	void incrementCostsInterventionInit(double cost);
	/* incrementCostsInterventionMonthly adds the monthly costs for starting an intervention */
	void incrementCostsInterventionMonthly(double cost);

	/* incrementCostsProph adds a prophylaxis treatment cost to the patients total */
	void incrementCostsProph(SimContext::OI_TYPE oiType, int prophNum, double cost);
	/* incrementCostsTBProph adds a TB proph cost to patients total */
	void incrementCostsTBProph(int prophNum, double cost);
	/* incrementCostsTBTreatment adds a TB proph cost to patients total */
	void incrementCostsTBTreatment(double treatCost, int treatmentNum);
	/* incrementCostsTBTest adds a TB test cost to patients total */
	void incrementCostsTBTest(double testCost, double dstCost, int testNum);
	/* incrementCostsTBProviderVisit adds cost of TB provider visit to patient's total */
    void incrementCostsTBProviderVisit(const double *costArray, double percent, double multiplier = 1.0);
    /* incrementCostsTBMedVisit adds cost of a TB Medication Pick-Up  visit to patient's total */
    void incrementCostsTBMedVisit(const double *costArray, double percent, double multiplier = 1.0);

	/* incrementCostsToxicity adds a toxicity cost to the patients total */
	void incrementCostsToxicity(double cost);
	/* incrementCostsTBToxicity adds a TB toxicity cost to the patients total */
	void incrementCostsTBToxicity(double cost, int treatmentNum = -1);
	/*incrementCostsChrms adds a CHRMs cost to the patients total*/
	void incrementCostsCHRMs(SimContext::CHRM_TYPE CHRMType,double cost);
	/* incrementCostsRoutineCare adds a routine care cost to the patients total costs,*/
	void incrementCostsRoutineCare(const double *costArray, double percent, double multiplier = 1.0);
	/* incrementCostsGeneralMedicine adds a General Medicine cost to the patients total costs,*/
	void incrementCostsGeneralMedicine(const double *costArray, double percent, double multiplier = 1.0);
	/* incrementCostsAcuteOITreatment adds a cost of treating an acute OI to the patients total costs,*/
	void incrementCostsAcuteOITreatment(const double *costArray, double multiplier = 1.0);
	/* incrementCostsAcuteOIUntreated adds a cost of an untreated acute OI to the patients total costs,*/
	void incrementCostsAcuteOIUntreated(const double *costArray, double multiplier = 1.0);
	/* incrementCostsDeath adds a Death cost to the patients total costs,*/
	void incrementCostsDeath(const double *costArray, double percent, double multiplier = 1.0);
	/* incrementCostsMisc adds a miscellaneous cost to the patients total costs,
		overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs */
	void incrementCostsMisc(double cost, double percent);
	void incrementCostsMisc(const double *costArray, double percent, double multiplier = 1.0);
	void incrementCostsTBMisc(const double *costArray, double percent, double multiplier = 1.0);
	/* updateInitialDistributions updates the initial statistics for patients upon HIV infection */
	void updateInitialDistributions();
	/* updatePatientSurvival updates the patient state for discounted LMs and QALMs,
		used with a half month length if death occurred that month */
	void updatePatientSurvival(double percentOfMonth);
	/* updateOverallSurvival updates the statistics for stratified LMs and QALMs,
		used with a half month length if death occurred that month */
	void updateOverallSurvival(double percentOfMonth);
	/*updateCostStats updates statistics used for the coststats file
		 used with a half month length if death occurred that month*/
	void updateCostStats (double percentOfMonth);
	/*setCostSubgroups sets the cost subgroups that the patient belongs to  */
	void setCostSubgroups(bool isInit = false);
	/* updateLongitSurvival updates the longitudinal statistics related to survival */
	void updateLongitSurvival();
	/* updateARTEfficacyStats updates the totals for months in ART suppression and HVL drops */
	void updateARTEfficacyStats();
	/* updateOIHistoryLogging updates the statistics for patient OI history */
	void updateOIHistoryLogging();
	/* updateStartMonthHIVNeg updates the number of people who were HIV neg at start of Month */
	void updateStartMonthHIVNeg();
    /* updateARTToxIncidenceTracking increments the count of incident ART toxicities*/
    void updateARTToxIncidenceTracking(const SimContext::ARTToxicityEffect &incidentTox);
    /* updateARTToxPrevalenceTracking increments the count of prevalent chronic ART toxicities with active mortality effects*/
    void updateARTToxPrevalenceTracking(const SimContext::ARTToxicityEffect &prevalentTox);

	/* Protected utility functions that are used by multiple state updater child classes */
	/* willAttendClinicThisMonth returns true if patient will go in for a scheduled clinic
		visit or OI emergency clinic visit this month */
	bool willAttendClinicThisMonth();
	
	/* getCD4Strata returns the CD4 strata for a given value */
	SimContext::CD4_STRATA getCD4Strata(double valueCD4);
	/* getCD4PercentageStrata returns the CD4 percentage strata for a given value */
	SimContext::PEDS_CD4_PERC getCD4PercentageStrata(double percCD4);
	/* getAgeCategoryHIVInfection returns the HIV testing age category for the given age */
	int getAgeCategoryHIVInfection(int ageMonths);
	/* getAgeCategoryHeterogeneity returns the Heterogeneity age category for the given age */
	int getAgeCategoryHeterogeneity(int ageMonths);
	/* getAgeCategoryCost returns the adult cost age category for the given age */
	int getAgeCategoryCost(int ageMonths);
	/* getAgeCategoryTBInfection returns the TB Infection age category for the given age */
	int getAgeCategoryTBInfection(int ageMonths);
	/* getAgeCategoryTransmRisk returns the age category for transmission risk group distribution */
	int getAgeCategoryTransmRisk(int ageMonths);
	/* getAgeCategoryLinkageStats returns the age category used for displaying age at linkage in the cost stats */
	int getAgeCategoryLinkageStats(int ageMonths);
	/* getAgeCategoryCHRMs returns the CHRMs age category for the given age */
	int getAgeCategoryCHRMs(int ageMonths, int CHRMType);
	/* getAgeCategoryCHRMsMthsStart returns the patient age category to be used when Orphans are enabled to determine the age in months ('months since start') of any children ('prevalent CHRMs') the patient has at model start */
	int getAgeCategoryCHRMsMthsStart(int ageMonths);
	/* getAgeCategoryCD4Metric returns the age category that determines the CD4 metric used if the patient becomes HIV+ */
	SimContext::PEDS_CD4_AGE_CAT getAgeCategoryCD4Metric(int ageMonths);
	/* getAgeCategoryPediatrics returns the Pediatrics module age category for the given age */
	SimContext::PEDS_AGE_CAT getAgeCategoryPediatrics(int ageMonths);
	/* getAgeCategoryPediatricsCost returns the Pediatrics Cost category for the given age */
	SimContext::PEDS_COST_AGE getAgeCategoryPediatricsCost(int ageMonths);
	/* getAgeCategoryPediatricsARTCost returns the Pediatrics ART Cost category for the given age */
	SimContext::PEDS_ART_COST_AGE getAgeCategoryPediatricsARTCost(int ageMonths);
	/* getAgeCategoryOutput returns the output age category for the given age */
    int getAgeCategoryOutput(int ageMonths);
	/* getAgeCategoryInfant returns the infant age category for the given age */
	int getAgeCategoryInfant(int ageMonths);
	/* getAgeCategoryInfantHIVProphCost returns the infant proph cost age category for the given age */
	int getAgeCategoryInfantHIVProphCost(int ageMonths);
	/* setPediatricState sets whether or not the patient is currently a Pediatric patient */
	void setPediatricState(bool isPediatric);
	/* setAdolescentState sets whether or not the patient is currently an Adolescent */
	void setAdolescentState(bool isAdolescent);
	/* getAgeCategoryAdolescent returns the adolescent age category for the given age */
	int getAgeCategoryAdolescent();
	/* getAgeCategoryAdolescentART returns the adolescent ART age category for the given age */
	int getAgeCategoryAdolescentART(int artLineNum);
private:
	/**
	 * \class StateUpdater
	 * The private functions are utility functions that are used by multiple updater functions */

	/* getTimeSummary returns a non-const pointer to the TimeSummary object for the current time period,
		creates a new one if needed or returns null if not keeping longitudinal stats */
	RunStats::TimeSummary *getTimeSummaryForUpdate();
	/* getOrphanStatsForUpdate returns a non-const pointer to the orphanStats object for the current time period,
		creates a new one if needed or returns null if not keeping longitudinal stats */
	RunStats::OrphanStats *getOrphanStatsForUpdate();
	/* incrementCostsCommon increases all general cost stats that are independent of the type of cost,
		overloaded to take in either a single cost value or a COST_NUM_TYPES sized array of costs */
	void incrementCostsCommon(double cost, double percent);
	void incrementCostsCommon(const double *costArray, double percent , double multiplier = 1.0);
	void incrementCostsTBCommon(double cost, double percent);
	void incrementCostsTBCommon(const double *costArray, double percent , double multiplier = 1.0);

};
