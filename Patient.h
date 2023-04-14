#pragma once

#include "include.h"

/*
	Patient class holds all of the stats for a patient running in the simulation.  The main
	entry point is the simulateMonth function, which calls all of the StateUpdaters to run the
	CEPAC simulations for a single month and update the patient state and statistics.
	The Patient class creates and initializes the StateUpdater objects for each particular
	Patient as part of the constructor.  Read only access to the patient state objects
	are provided through accessor functions that return const pointers to the
	subclass objects.
*/
class Patient
{
public:
	/** \brief Make the StateUpdater class a friend class so it can modify the private data */
	friend class StateUpdater;

	/* Constructor and Destructor */
	Patient(SimContext *simContext, RunStats *runStats, CostStats *costStats, Tracer *tracer, bool _predefinedAgeAndGender = false, int _ageMonths = 0, SimContext::GENDER_TYPE _gender = SimContext::GENDER_FEMALE, bool _setAsIncidentCase = false, int startingMonth = 0);
	~Patient(void);

	/** GeneralState class holds information about the patients initial characteristics
		and total costs and survival numbers */
	class GeneralState {
	public:
		/** Unique patient identifier */
		int patientNum;
		/** Will default to false; true if transmission (or other?) wants to assign patient age and gender instead of drawing from distribution*/
		bool predefinedAgeAndGender;
		/** True if this patient is traced in the output trace file*/
		bool tracingEnabled;
		/** The current discount factor*/
		double discountFactor;
		/**The discount factor when using multiple discount rates*/
		double multDiscFactorCost[SimContext::NUM_DISCOUNT_RATES];
		/**The discount factor when using multiple discount rates*/
		double multDiscFactorBenefit[SimContext::NUM_DISCOUNT_RATES];
		/** The current value to use for QOL adjustment*/
		double QOLValue;
		/** The current month number */
		int monthNum;
		/** Will default to 0; has a non-zero value if the transmission model (or other?) wants to start the patient at a time other than time 0*/
		int initialMonthNum;
		/** The patient's current age in months*/
		int ageMonths;
		/** The age index the patient belongs to for the HIV infection age bracketing */
		int ageCategoryHIVInfection;
		/** The age index the patient belongs to for Heterogeneity age bracketing */
		int ageCategoryHeterogeneity;
		/** The age index the patient belongs to for adult (non-Pediatric) costs */
		int ageCategoryCost;
		/** True if the Pediatrics module is enabled and the patient has not yet transitioned to Adolescent or Adult */
		bool isPediatric;
		/** True if the Adolescent module is enabled and the patient is an Adolescent who has not yet transitioned to the Adult model */
		bool isAdolescent;

		/** The patient's gender */
		SimContext::GENDER_TYPE gender;
		/** An array of booleans indicating whether or not the patient has any of the generic user defined risk factors*/
		bool hasRiskFactor[SimContext::RISK_FACT_NUM];
		/** The transmission risk category the patient belongs to */
		SimContext::TRANSM_RISK transmRiskCategory;


		/** The current discounted costs accrued for this patient*/
		double costsDiscounted;
		/** The current discounted and undiscounted life months this patient has lived*/
		double LMsDiscounted;
		double LMsUndiscounted;
		/** The current quality adjusted (discounted) life months this patient has lived*/
		double qualityAdjustLMsDiscounted;

		/**The current discounted costs accrued for this patient using multiple discount rates*/
		double multDiscCosts[SimContext::NUM_DISCOUNT_RATES];
		double multDiscLMs[SimContext::NUM_DISCOUNT_RATES];
		double multDiscQALMs[SimContext::NUM_DISCOUNT_RATES];

		/**The cost subgroups that the patient belongs to*/
		bool costSubgroups[SimContext::COST_SUBGROUPS_NUM];

		/** True if the patient has a logged OI history */
		bool loggedPatientOIs;
		/** The patient's baseline logit for heterogeneity (not including Age, cd4, risk etc)*/
		double responseBaselineLogit;
		/** The patient's base response logit increment if pre-ART (used for LTFU)*/
		double responseLogitPreARTBase;
		/** True if the patient has an adherence intervention active*/
		bool isOnAdherenceIntervention;
		/** Index of current adherence intervention the patient is on */
		int currAdherenceIntervention;
		/** Index of next adherence intervention the patient will be on */
		int nextAdherenceIntervention;

		/** The adjustment to logit coefficient for adherence if patient is on Adherence Intervention.  This will be added to baseline*/
		double responseLogitAdherenceInterventionIncrement;
		/** month patient started adherence intervention */
		int monthStartAdherenceIntervention;
		/** month to end adherence intervention */
		int monthToEndAdherenceIntervention;
	}; /* end GeneralState */

	/** PedsState class holds information about the patients pediatric characteristics*/
	class PedsState {
	public:
		/** The age index the patient belongs to for peds cost bracketing */
		SimContext::PEDS_COST_AGE ageCategoryPedsCost;
		/** The age category that will determine the patient's CD4 metric if they become HIV+ */
		SimContext::PEDS_CD4_AGE_CAT ageCategoryCD4Metric;
		/** The age category the patient belongs to for pediatrics */
		SimContext::PEDS_AGE_CAT ageCategoryPediatrics;
		/** The age category the patient belongs to for peds art cost */
		SimContext::PEDS_ART_COST_AGE ageCategoryPedsARTCost;
		int ageOfSeroreversion;

		/** Maternal Status */
		/** A boolean indicating whether or not the patient's mother is alive (used only for pediatrics)*/
		bool isMotherAlive;
		/** A boolean indicating whether the mother was infected with either chronic or acute HIV during delivery */
		bool motherInfectedDuringDelivery[2];
		/** A boolean indicating whether the mother was infected during bf */
		bool motherInfectedDuringBF;
		SimContext::PEDS_MATERNAL_STATUS maternalStatus;
		bool maternalStatusKnown;
		bool motherOnART;
		/** a boolean indicating whether the mother is on suppressed ART */
		bool motherOnSuppressedART;
		/** a boolean indicating whether the mother is suppressed at delivery, given that she was HIV+ on ART at delivery */
		bool motherOnSuppressedARTInitially;
		/** a boolean indicating whether the mother's HVL is known - she must be HIV+ on ART for this to be true */
		bool motherSuppressionKnown;
		/** a boolean indicating whether the mother's HVL is known at delivery, given that she was HIV+ on ART at delivery */
		bool motherSuppressionKnownInitially;
		/** a boolean indicating whether the mother was on ART at delivery, given that she was initially HIV+ */
		bool motherOnARTInitially;
		/** a boolean indicating whether the mother is low HVL */
		bool motherLowHVL;
		/** a boolean indicating whether the mother is low HVL at delivery*/
		bool motherLowHVLInitially;

		/** The time at which the patient's mother was infected */
		int monthOfMaternalHIVInfection;
		/** The time at which the patient's mother died */
		int monthOfMaternalDeath;
		/** The patient's breastfeeding status -- used only for pediatrics*/
		SimContext::PEDS_BF_TYPE breastfeedingStatus;
		/** Age at which the patient stops breastfeeding */
		int breastfeedingStopAge;
		/** True if the patient is replacement fed before their breastfeeding stop age for reasons other than maternal death */
		bool breastfeedingStoppedEarly;
		/** The time at which the patient started replacementfeeding --used only for pediatrics */
		int monthOfReplacementFeedingStart;

		/**EID */
		/** Whether they are in the EID system or not.  If not they will not attend EID test visits (can never lose this flag once false)*/
		bool canReceiveEID;
		/** If they are false positives or not. (This flag can change) False positive's care state are considered to be HIV negative */
		bool isFalsePositive;
		/** If they are false positive and linked to care (can never lose this flag once true; also if they get infected their HIV will not be detected and they cannot link to real HIV care) */
		bool isFalsePositiveLinked;
		/** The number of missed EID Visits or well care visits*/
		int numMissedVistsEID;
		/** This specifies the test type and assay of the most recent positive test that the provider has knowledge of (returned from clinic) that has not completed a test chain (this is used for continuing the cascade when doctor has knowledge of prior diagnosis*/
		bool hasMostRecentPositiveEIDTest;
		int mostRecentPositiveEIDTestBaseAssay;
		SimContext::EID_TEST_TYPE mostRecentPositiveEIDTestType;

		/** This is the most recent negative EID test used for infant proph eligibility */
		bool hasMostRecentNegativeEIDTest;
		int monthOfMostRecentNegativeEIDTest;

		/** This specifies if the patient has missed a test a scheduled visit, they are eligible to retake the test if the reoffer cell is checked*/
		bool hasMissedEIDTest;
		int missedEIDTestBaseAssay;

		/** Stores pending test results waiting to be returned */
		vector<SimContext::EIDTestState> eidPendingTestResults;
		/** Stores scheduled EID confirmatory tests waitng to be done */
		vector<SimContext::EIDTestState> eidScheduledConfirmatoryTests;

		/**The current prob that Infant HIV proph dose will be effective*/
		double probHIVProphEffective[SimContext::INFANT_HIV_PROPHS_NUM];
		/** Infant HIV Proph*/
		bool everInfantHIVProph[SimContext::INFANT_HIV_PROPHS_NUM];
		bool hasEffectiveInfantHIVProph[SimContext::INFANT_HIV_PROPHS_NUM];
		int monthOfEffectiveInfantHIVProph[SimContext::INFANT_HIV_PROPHS_NUM];
		bool hasInfantHIVProphMajorToxicity[SimContext::INFANT_HIV_PROPHS_NUM];
		// The month that Infant HIV proph toxicity occurred
		int monthOfInfantHIVProphMajorToxicity[SimContext::INFANT_HIV_PROPHS_NUM];

	}; /* end PedsState */

	/** DiseaseState contains information about the HIV infection state, CD4/HVL levels,
		OIs and OI history, and death status */
	class DiseaseState {
	public:
		/** Indicates the patient's current HIV state */
		SimContext::HIV_INF infectedHIVState;
		/** Indicates the patient's current HIV state for HIV positive only */
		SimContext::HIV_POS infectedHIVPosState;
		/** Indicates the patient's current HIV state for pediatric HIV*/
		SimContext::PEDS_HIV_STATE infectedPediatricsHIVState;
		/** True if the patient was infected at the beginning of the simulation */
		bool isPrevalentHIVCase;
		/** An integer indicating the month the patient was infected*/
		int monthOfHIVInfection;
		/** An integer indicating the month the patient transitioned from acute to chronic HIV*/
		int monthOfAcuteToChronicHIV;
		/** A double indicating the patient's true CD4 count*/
		double currTrueCD4;
		/** A double indicating the patient's minimum lifetime CD4 count*/
		double minTrueCD4;
		/** The patient's true CD4 strata */
		SimContext::CD4_STRATA currTrueCD4Strata;
		/** The patient's minimum lifetime CD4 strata*/
		SimContext::CD4_STRATA minTrueCD4Strata;
		/** The patient's true CD4 percentage (used for pediatrics)*/
		double currTrueCD4Percentage;
		/** The patient's minimum lifetime CD4 percentage (used for pediatrics)*/
		double minTrueCD4Percentage;
		/** The patient's true CD4 percentage strata (used for pediatrics)*/
		SimContext::PEDS_CD4_PERC currTrueCD4PercentageStrata;
		/**True if the patient has drawn the patient specific cd4 decline increment*/
		bool hasDrawnPatientSpecificCD4Decline;
		/**The patient specific CD4 decline Increment.  This number is drawn once at the start of the simulation for each patient.*/
		double patientSpecificCD4DeclinePerc;
		/** The patient's true viral load strata */
		SimContext::HVL_STRATA currTrueHVLStrata;
		/** The patient's setpoint viral load strata */
		SimContext::HVL_STRATA setpointHVLStrata;
		/** The patient's target viral load strata */
		SimContext::HVL_STRATA targetHVLStrata;
		/** True if the patient currently has an infection*/
		bool hasCurrTrueOI;
		/** The previous month that the patient had a particular severe OI*/
		int lastMonthSevereOI[SimContext::OI_NUM];
		/** If hasCurrTrueOI is true, this indicates which infection the patient currently has (max of one at any given time)*/
		SimContext::OI_TYPE typeCurrTrueOI;
		/** True if the patient currently has an observed OI */
		bool hasCurrObservedOI;
		/** An indicator marking which type of OI history the patient keeps track of (mild OIs, severe OIs, or none)*/
		SimContext::HIST_EXT typeTrueOIHistory;
		/** An array of booleans denoting which OIs the patient has had*/
		bool hasTrueOIHistory[SimContext::OI_NUM];
		/** Month in which the patient had their first logged OI */
		int monthOfFirstOI;
		/** Type of the patient's first logged OI */
		SimContext::OI_TYPE firstOIType;
		/** An array of integers denoting the number of each type of OI the patient has had since the last clinic visit*/
		int numTrueOIsSinceLastVisit[SimContext::OI_NUM];
		/** An array of booleans indicating which CHRM (chronic/hepatic/renal/malignancy) the patient currently has*/
		bool hasTrueCHRMs[SimContext::CHRM_NUM];
		/** An array of integers indicating the month in which each CHRM started*/
		int monthOfCHRMsStageStart[SimContext::CHRM_NUM][SimContext::CHRM_TIME_PER_NUM];
		/** A boolean that is true if the patient is currently alive*/
		bool isAlive;
		/** A vector of the patient's mortality risks*/
		vector<SimContext::MortalityRisk> mortalityRisks;
		/** The patient's cause of death (not assigned until death)*/
		SimContext::DTH_CAUSES causeOfDeath;
		/** True if the patient is HIV negative and fits user definitions of HIV exposure (used for mortality with peds early childhood)*/
		bool useHEUMortality;
		/** True if the patient has only ever breastfed from an HIV-negative mother - i.e., weaned from an HIV-negative mother or dies while breastfeeding from her */
		bool neverExposed;
	}; /* end DiseaseState */

	/** MonitoringState contains information for testing, clinical visits, and observed health state */
	class MonitoringState {
	public:
		/** The current discounted CD4 Test costs accrued for this patient*/
		double costsCD4Testing;
		/** The current discounted HVL Test costs accrued for this patient*/
		double costsHVLTesting;
		/** The current discounted PrEP costs accrued for this patient*/
		double costsPrEP;
		/** The current discounted HIV Test costs accrued for this patient*/
		double costsHIVTesting;
		/** True if the patient has been detected HIV positive*/
		bool isDetectedHIVPositive;
		/** Month the pateint was detected */
		int monthOfDetection;
		/** True if the patient has been linked into care*/
		bool isLinked;
		/** Month the patient linked into care*/
		int monthOfLinkage;
		SimContext::HIV_CARE careState;
		/** True if the patient is high risk for HIV, false if low risk (only matters for HIV negative patients in the testing module for determining incidence rate to use)*/
		bool isHighRiskForHIV;
		/** The current HIV incidence reduction multiplier, to be applied to the patient's probability of HIV infection if enabled */
		double HIVIncReducMultiplier;
		/** An integer defining the frequency of HIV tests*/
		int intervalHIVTest;
		/** True if the patient has an HIV test scheduled*/
		bool hasScheduledHIVTest;
		/** An integer defining the time of the patient's next scheduled HIV test */
		int monthOfScheduledHIVTest;
		/** The patient's HIV test acceptance probability*/
		double acceptanceProbHIVTest;
		/** True if patient is able to get CD4 tests */
		bool CD4TestingAvailable;
		/** True if the patient has an observed CD4 count (absolute CD4 metric)*/
		bool hasObservedCD4;
		/** An integer representing the month the patient's CD4 count was observed (only valid if hasObservedCD4 is true)*/
		int monthOfObservedCD4;
		/** True if the patient has an observed CD4 count through a standard test (not lab staging)*/
		bool hasObservedCD4NonLabStaging;
		/** month of observed cd4 count through non lab staging means*/
		int monthOfObservedCD4NonLabStaging;
		/** A double representing the patient's most recent observed CD4 count (only valid if hasObservedCD4 is true)*/
		double currObservedCD4;
		/** A double representing the patient's minimum observed CD4 count (only valid if hasObservedCD4 is true)*/
		double minObservedCD4;
		/** A SimContext::CD4_STRATA representing the CD4 strata corresponding to the patient's current observed CD4 (only valid if hasObservedCD4 is true)*/
		SimContext::CD4_STRATA currObservedCD4Strata;
		/** True if the patient has an observed CD4 percentage (pediatrics - percentage CD4 metric)*/
		bool hasObservedCD4Percentage;
		/** An integer representing the month the patient's CD4 percentage was observed (only valid if hasObservedCD4Percentage is true)*/
		int monthOfObservedCD4Percentage;
		/** A double representing the patient's most recent observed CD4 percentage (only valid if hasObservedCD4Percentage is true)*/
		double currObservedCD4Percentage;
		/** A double representing the patient's minimum observed CD4 percentage (only valid if hasObservedCD4Percentage is true)*/
		double minObservedCD4Percentage;
		/** A SimContext::PEDS_CD4_PERC representing the CD4 percentage strata corresponding to the patient's current observed CD4 percentage (only valid if hasObservedCD4Percentage is true)*/
		SimContext::PEDS_CD4_PERC currObservedCD4PercentageStrata;
		/** True if the patient has an observed HVL Strata*/
		bool hasObservedHVLStrata;
		/** An integer representing the month the patient's HVL strata was observed (only valid if hasObservedHVLStrata is true)*/
		int monthOfObservedHVLStrata;
		/** A SimContext::HVL_STRATA corresponding to the patient's most recent observed HVL strata (only valid if hasObservedHVLStrata is true)*/
		SimContext::HVL_STRATA currObservedHVLStrata;
		/** A SimContext::HVL_STRATA corresponding to the patient's minimum observed HVL strata (only valid if hasObservedHVLStrata is true)*/
		SimContext::HVL_STRATA maxObservedHVLStrata;
		/** An array of integers corresponding to the number of times each OI has been observed throughout the patient's entire simulation*/
		int numObservedOIsTotal[SimContext::OI_NUM];
		/** An array of integers corresponding to the number of times each OI has been observed since the last clinic visit*/
		int numObservedOIsSinceLastVisit[SimContext::OI_NUM];
		/** A SimContext::CLINIC_VISITS representing the reason for the current clinic visit*/
		SimContext::CLINIC_VISITS clinicVisitType;
		/** True if the patient has a regular clinic visit scheduled*/
		bool hasRegularClinicVisit;
		/** An integer representing the month of the next regular clinic visit (only valid if hasRegularClinicVisit is true)*/
		int monthOfRegularClinicVisit;
		/** An EMERGENCY_TYPE corresponding to the most recent trigger for the emergency clinic visit */
		SimContext::EMERGENCY_TYPE emergencyClinicVisitType;
		/** An integer representing the month of the next emergency clinic visit (only valid if emergencyClinicVisitType is not EMERGENCY_NONE)*/
		int monthOfEmergencyClinicVisit;
		/** True if the patient has ever had a clinic visit*/
		bool hadPrevClinicVisit;
		/** True if the patient has a CD4 Test scheduled*/
		bool hasScheduledCD4Test;
		/** True if the patient is over the cd4 lag period and had a chance to schedule a cd4 test*/
		bool hadChanceCD4Test;
		/** An integer representing the month of the next CD4 test (only valid if hasScheduledCD4Test is true)*/
		int monthOfScheduledCD4Test;
		/** True if the patient has a HVL Test scheduled*/
		bool hasScheduledHVLTest;
		/** True if the patient is over the hvl lag period and had a chance to schedule a hvl test*/
		bool hadChanceHVLTest;
		/** An integer representing the month of the next HVL test (only valid if hasScheduledHVLTest is true)*/
		int monthOfScheduledHVLTest;
		/** The patient's current lost-to-follow-up state*/
		SimContext::LTFU_STATE currLTFUState;
		/** True if the patient was previously lost to follow up*/
		bool hadPrevLTFU;
		/** True if the patient had previously returned to care*/
		bool hadPrevRTC;
		/** An integer representing the time of the last LTFU state change (i.e. the patient was lost or the patient returned to care)*/
		int monthOfLTFUStateChange;
		/** True if the patient was on ART when lost to follow up (only valid if hadPrevLTFU is true)*/
		bool wasOnARTWhenLostToFollowUp;
		/** True if patient is on PrEP*/
		bool hasPrEP;
		/** True if patient has ever had PrEP */
		bool everPrEP;
		/** True if the patient has dropped out of PrEP */
		bool isPrEPDropout;
		/** Month Number in which the patient reaches the PrEP dropout threshold */
		int PrEPDropoutThresholdMonth;
	}; /* end MonitoringState */

	/** ProphState holds information about the current and previous prophylaxis taken,
		and state pertinent to future prophylaxis policy */
	class ProphState {
	public:
		/** True if the patient is eligible for prophylaxis: if false, the patient never goes on prophylaxis*/
		bool mayReceiveProph;
		/** True if the patient is non-compliant with the prophylaxis drugs*/
		bool isNonCompliant;
		/** The total number of prophylaxes the patient is currently on*/
		int currTotalNumProphsOn;
		/** An array of booleans indicating which OIs the patient is taking prophylaxis for*/
		bool isOnProph[SimContext::OI_NUM];
		/** An array indicating whether the patient is on primary or secondary prophylaxis for each OI (currProphType[i] is only valid if isOnProph[i] is true)*/
		SimContext::PROPH_TYPE currProphType[SimContext::OI_NUM];
		/** An array indicating the prophylaxis number the patient is on for each OI (currProphNum[i] is only valid if isOnProph[i] is true)*/
		int currProphNum[SimContext::OI_NUM];
		/** An array indicating when the patient started prophylaxis for each OI (monthOfProphStart[i] is only valid if isOnProph[i] is true)*/
		int monthOfProphStart[SimContext::OI_NUM];
		/** An array of booleans indicating which OIs have a next prophylaxis available*/
		bool hasNextProphAvailable[SimContext::OI_NUM];
		/** An array of SimContext::PROPH_TYPE indicating what the next prophylaxis is for each OI (nextProphType[i] is only valid if hasNextProphAvailable[i] is true*/
		SimContext::PROPH_TYPE nextProphType[SimContext::OI_NUM];
		/** An array of integers indicating the next prophylaxis number corresponding to each OI (nextProphNum[i] is only valid if hasNextProphAvailable[i] is true*/
		int nextProphNum[SimContext::OI_NUM];
		/** An array of toxicity types indicating which toxicity the patient has accrued for each OI*/
		SimContext::PROPH_TOX_TYPE typeProphToxicity[SimContext::OI_NUM];
		/** An array of booleans indicating whether or not resistance is used for each OI's prophylaxis*/
		bool useProphResistance[SimContext::OI_NUM];
		/** A 2D array indicating which prophylaxes the patient has taken: primary or secondary for each OI*/
		bool hasTakenProph[SimContext::OI_NUM][SimContext::PROPH_NUM_TYPES];
	}; /* end ProphState */

	/** ARTState holds information about current and previous ART regimens taken, and
		state pertinent to future ART treatment policy */
	class ARTState {
	public:
		/** The current discounted ART costs accrued for this patient*/
		double costsART;
		/** True if the patient may receive ART.  If false, the patient never receives ART even if s/he meets the starting criteria*/
		bool mayReceiveART;
		/** True if the patient is currently on ART */
		bool isOnART;
		/** True if the patient is on resuppression regimen */
		bool isOnResupp;
		/** Number of failed resuppression events.  Gets reset upon successful resuppression */
		int numFailedResupp;
		/** True if the patient has ever taken ART */
		bool hasTakenART;
		/** Has taken ART by ART regimen */
		bool hasTakenARTRegimen[SimContext::ART_NUM_LINES];
		/** The patient's CD4 response type while on ART -- used for modeling discordant response types*/
		SimContext::CD4_RESPONSE_TYPE CD4ResponseType;
		/** An integer ranging from 0 to 9 representing the ART regimen the patient is currently on (only valid if isOnART is true)*/
		int currRegimenNum;
		/** An integer representing the month the patient started the current ART regimen (only valid if isOnART is true)*/
		int monthOfCurrRegimenStart;
		/** An integer representing the month the patient first started taking ART */
		int monthFirstStartART;
		/** An integer representing the ART line the patient was most recently on, not including a current ART regimen*/
		int prevRegimenNum;
		/** The month the patient most recently stopped an ART regimen*/
		int monthOfPrevRegimenStop;
		/** True if the patient has another ART regimen available*/
		bool hasNextRegimenAvailable;
		/** True if the next regimen is a resuppression regimen */
		bool nextRegimenIsResupp;
		/** The next ART regimen available to the patient (only valid if hasNextRegimenAvailable is true)*/
		int nextRegimenNum;
		/** The current subregimen the patient is on*/
		int currSubRegimenNum;
		/** The month the patient started the current subregimen*/
		int monthOfCurrSubRegimenStart;
		/** The efficacy of the regimen the patient is currently on*/
		SimContext::ART_EFF_TYPE currRegimenEfficacy;
		/** The efficacy of the regimen the patient was most recently on, not including the current regimen*/
		SimContext::ART_EFF_TYPE prevRegimenEfficacy;
		/** The month the patient's ART efficacy most recently changed*/
		int monthOfEfficacyChange;
		/**The month that a new multiplier will be applied to cd4 decline on art fail*/
		int monthOfNewCD4MultArtFail;
		/**The current cd4 decline multiplier applied after failed art**/
		double currCD4MultArtFail;

		/** The patient's response logit for current art regimen (without adherence interventions)*/
		double responseLogitCurrRegimenBase;
		/** The patient's response logit for current art regimen (includes adherence intervention effects)*/
		double responseLogitCurrRegimen;
		/** The patient's regimen specific increment to propensity to respond*/
		double responseLogitCurrRegimenIncrement;
		/** The patient's response Logit for the curr ART Regimen before the regimen specific increment is added */
		double responseLogitCurrRegimenPreIncrement;
		/** Whether or not to stop adding the regimen specific increment after a specified duration */
		bool enableDurationCurrRegimenResponseIncrement;
		/** The month to stop adding the regimen specific response increment */
		int monthToStopCurrRegimenResponseIncrement;
		/** The patient's response factor for the current regimen (set based on the response propensity): 1 for full responder, 0 for non-responder, in between for a partial responder (see StateUpdater::setCurrARTResponse)*/
		double responseFactorCurrRegimen[SimContext::HET_NUM_OUTCOMES];
		/** The patient's response type (full, partial, non) for the current ART regimen*/
		SimContext::RESP_TYPE responseTypeCurrRegimen[SimContext::HET_NUM_OUTCOMES];
		/** Whether the patient is receiving the ART Effect (HIV Mortality, OI Mortality, CHRMs) - pediatric patients will receive the ART Effect as long as they are on ART*/
		bool applyARTEffect;
		/** The patient's probability of being suppressed based on their response factor*/
		double probInitialEfficacy;
		/** The patient's probability of being resuppressed */
		double probResuppEfficacy;
		/** The patient's probability of late failure*/
		double probLateFail;
		/** The patient's probability to restart this regimen after failure based on their response type*/
		double probRestartAfterFail;
		/** The proportion of monthly costs incurred by a non responder*/
		double propMthCostNonResponders;

		/** The CD4 slope for the current regimen*/
		double currRegimenCD4Slope;
		/** The CD4 percentage slope for the current regimen (used for pediatrics)*/
		double currRegimenCD4PercentageSlope;
		/** True once the patient has been successful on the current ART regimen */
		bool hadSuccessOnRegimen;
		/** Month number of success on the current ART regimen */
		int monthOfInitialRegimenSuccess;
		/** The CD4 envelope across all regimens, activated at the time of first success on any regimen*/
		SimContext::CD4Envelope overallCD4Envelope;
		/** The CD4 envelope for the current regimen only, activated at the time of success on it*/
		SimContext::CD4Envelope indivCD4Envelope;
		/** The CD4 percentage envelope across all regimens (used for pediatrics), activated at the time of success on any regimen */
		SimContext::CD4Envelope overallCD4PercentageEnvelope;
		/** The CD4 envelope for the current regimen only (used for pediatrics), activated at the time of success on it*/
		SimContext::CD4Envelope indivCD4PercentageEnvelope;
		/** The patient's observed viral load at the beginning of the current regimen*/
		SimContext::HVL_STRATA observedHVLStrataAtRegimenStart;
		/** The maximum observed CD4 the patient had while on the current ART regimen*/
		double maxObservedCD4OnCurrART;
		/** The maximum observed CD4 percentage the patient had while on the current ART regimen (used for pediatrics)*/
		double maxObservedCD4PercentageOnCurrART;
		/** The minimum observed viral load strata the patient had while on the current ART regimen*/
		SimContext::HVL_STRATA minObservedHVLStrataOnCurrART;
		/** Array of the number of observed OIs since the last time the patient either stopped or failed ART stratified by OI type*/
		int numObservedOIsSinceFailOrStopART[SimContext::OI_NUM];
		/** The number of CD4 test results that currently count towards a diagnosis of observed ART failure */
		int numFailedCD4Tests;
		/** The number of HVL test results that currently count towards a diagnosis of observed ART failure */
		int numFailedHVLTests;
		/** The number of OI events the patient has had that currently count towards a diagnosis of observed ART failure */
		int numFailedOIs;
		/** True if the patient has an observed ART failure*/
		bool hasObservedFailure;
		/** The type of ART failure (only valid if hasObservedFailure is true)*/
		SimContext::ART_FAIL_TYPE typeObservedFailure;
		/** The month of the current observed failure (only valid if hasObservedFailure is true)*/
		int monthOfObservedFailure;
		/** The number of observed failures the patient has had*/
		int numObservedFailures;
		/** The type of the current ART stop*/
		SimContext::ART_STOP_TYPE typeCurrStop;
		/** An array of the number of months on unsuccessful ART stratified by ART regimen*/
		int numMonthsOnUnsuccessfulByRegimen[SimContext::ART_NUM_LINES];
		/** An array of the number of months on unsuccessful ART stratified by HVL strata*/
		int numMonthsOnUnsuccessfulByHVL[SimContext::HVL_NUM_STRATA];
		/** A linked list of the active ART toxicity effects */
		list<SimContext::ARTToxicityEffect> activeToxicityEffects;
		/** True if the patient has a major toxicity*/
		bool hasMajorToxicity;
        /** True if the patient is due to switch ART lines because of chronic toxicity*/
		bool hasChronicToxSwitch;
		/** Line to switch to if hasChronicToxSwitch (only valid if hasChronicToxSwitch)*/
		int chronicToxSwitchToLine;
		/** True if the patient has a toxicity which will trigger a subregimen switch*/
		bool hasSevereToxicity;
		/** A pointer to the current severe ART toxicity effect (only valid if hasSevereToxicity is true)*/
		const SimContext::ARTToxicityEffect *severeToxicityEffect;
		/** True if the patient has ever had a toxicity*/
		bool hadPrevToxicity;
		/** The patient's current STI (standard treatment interruption) state*/
		SimContext::STI_STATE currSTIState;
		/** The month of the most recent STI state change*/
		int monthOfSTIStateChange;
		/** The month of the first STI treatment interruption*/
		int monthOfSTIInitialStop;
		/** The number of times STI was applied during the current regimen*/
		int numSTIInterruptionsOnCurrRegimen;
	}; /* end ARTState */

	/** TBState holds information about the current TB disease and treatment, and
		information pertinent to future TB treatment policy */
	class TBState {
	public:
		/** The patient's current true TB disease state*/
		SimContext::TB_STATE currTrueTBDiseaseState;
		/** The patient's current true TB strain in terms of drug resistance*/
		SimContext::TB_STRAIN currTrueTBResistanceStrain;
		/** The patient's current observed TB strain in terms of drug resistance*/
		bool hasObservedTBResistanceStrain;
		SimContext::TB_STRAIN currObservedTBResistanceStrain;
		/** The patients most recent history of observed TB strain*/
		bool hasHistObservedTBResistanceStrain;
		SimContext::TB_STRAIN observedHistTBResistanceStrain;
		//** whether the patient has an unfavorable outcome
		bool hasUnfavorableOutcome[SimContext::TB_NUM_UNFAVORABLE];

		/** The current status of the patient in the TB Care system*/
		SimContext::TB_CARE careState;
		/** The patient's tracker variable status */
		bool currTrueTBTracker[SimContext::TB_NUM_TRACKER];

		/** A bool representing if patient is eligible to get interval testing for TB */
		bool isEligibleTBInitPolicyInterval;

		/** An integer representing the last time the true TB state changed*/
		int monthOfTBStateChange;
		/** An integer representing the month the patient was infected with TB*/
		int monthOfTBInfection;
		/** True if the patient is in the Previously Treated TB state and got there via self-cure */
		bool isSelfCured;
		/** True if the patient has a true history of any kind of TB, including before model start */
		bool hasTrueHistoryTB;
		/** True if the patient enters the model in either the Previously Treated or Treatment Default TB state */
		bool observedHistActiveTBAtEntry;
		/** Number of months since initial treatment stopped if the patient entered the model in the Previously Treated or Treatment Default state - assumed to have had one round of treatmnent so far */
		int monthsSinceInitTBTreatStopAtEntry;
		/** An array storing current results of all TB tests in the current test chain*/
		SimContext::TB_DIAG_STATUS testResults[SimContext::TB_DIAG_TEST_ORDER_NUM];
		/** The index of the current TB test in the testing chain - not the number of the test itself */
		int currTestIndex;
		/** The index of the next TB test in the testing chain order - if equal to -1, the chain will stop after the current test */
		int nextTestIndex;

		/** Pending results for curr TB test */
		bool hasPendingResult;
		bool willPickupResult;
		SimContext::TB_DIAG_STATUS currPendingResult;
		int monthOfResultPickup;
		bool resetTestsOnPickup;
		int currPendingResultTestNum;
		bool willDoDST;

		/** Pending results for curr dst TB test */
		bool hasPendingDSTResult;
		SimContext::TB_STRAIN currPendingDSTResult;
		int monthOfDSTResultPickup;

		/** True if the patient is eligible to receive TB Proph */
		bool isEligibleForProph;
		/** True if the patient has rolled for eligibility to TB Proph */
		bool hasRolledEligibleForProph;
		/** True if the patient is on TB prophylaxis*/
		bool isOnProph;
		/** True if the patient has had TB Proph*/
		bool hadProph;
		/** True if the patient is scheduled for TB prophylaxis*/
		bool isScheduledForProph;
		/** The current TB prophylaxis the patient is on (only valid if isOnProph is true)*/
		int currProphNum;
		/** The index of the current TB prophylaxis the patient is on */
		int currProphIndex;
		/** An integer representing the month the patient started TB prophylaxis*/
		int monthOfProphStart;
		/** True if patient has completed their most recent proph line */
		bool hasCompletedProph;
		/** Number of times the patient has started each line of TB prophylaxis */
		int numProphStarts[SimContext::TB_NUM_PROPHS];
		/** Most recent TB proph number */
		int mostRecentProphNum;
		/** An integer representing the month the patient stoped TB prophylaxis */
		int monthOfProphStop;
		/** An integer representing the next TB prophylaxis the patient would go on (only valid if hasNextProphAvailable is true)*/
		int nextProphIndex;
		int nextProphNum;
		/** True if the patient has a next TB prophylaxis available*/
		bool hasNextProphAvailable;
		/** True if the patient has a major toxicity from TB prophylaxis*/
		bool hasMajorProphToxicity;

		/** The most recent month number in which the patient received a TB positive diagnosis */
		int monthOfTBPosDiagnosis;
		/** True if the patient is on TB treatment*/
		bool isOnTreatment;
		/** True if the patient has ever completed treatment (empiric or not)*/
		bool everCompletedTreatmentOrEmpiric;
		/** True if patient has ever received treatment or empiric - includes before model entry if initially in the Prev Treated or Treat Default States */
		bool everOnTreatmentOrEmpiric;
		/** True if patient has ever recieved treatment or empiric at the start of current test chain */
		bool everOnTreatmentOrEmpiricStartChain;
		/** True if patient has ever started non-initial treatment or empiric */
		bool everHadNonInitialTreatmentOrEmpiric;
		/** True if the patient is scheduled for TB treatment*/
		bool isScheduledForTreatment;
		/** True if the scheduled treatment result is predetermined */
		bool setResultForScheduledTreatment;
		/** True if the scheduled treatment is guaranteed to succeed (only valid if setResultForScheduledTreatment is true)*/
		bool scheduledTreatmentSuccess;
		/** The patient's current TB treatmentNum (only valid if isOnTreatment is true)*/
		int currTreatmentNum;
		/** The next TB treatment num available to the patient*/
		int nextTreatmentNum;
		/** The number of times this line has been repeated */
		int numRepeatCurrTreatment;
		/** True if the next treatment is a repeat of the curr line */
		bool nextTreatmentIsRepeat;
		/** An integer representing the month the patient started TB treatment*/
		int monthOfTreatmentStart;
		/** An integer representing the month patient most recently finished either regular or Empiric Treatment */
		int monthOfTreatmentOrEmpiricStop;
		/** An integer representing the month the patient finished initial TB treatment*/
		int monthOfInitialTreatmentStop;
		/** An integer representing the time the patient has already been on the TB treatment */
		int previousTreatmentDuration;
		/** True if the treatment is destined to succeed (only valid if isOnTreatment is true)*/
		bool treatmentSuccess;
		/** True if patient has stopped a treatment since model start (empiric counts) - does not include treatment before model entry for those initially in Prev Treated or Treat Default state  */
		bool hasStoppedTreatmentOrEmpiric;
		/** The most recent line of treatment (empiric counts) the patient has been on */
		int mostRecentTreatNum;

		/** True if the patient is on Empiric TB treatment */
		bool isOnEmpiricTreatment;
		int currEmpiricTreatmentNum;
		int monthOfEmpiricTreatmentStart;
		int previousEmpiricTreatmentDuration;
		bool empiricTreatmentSuccess;
		/** True if patient had major tox while on empiric or non-empiric TB treatment */
		bool hadTreatmentMajorTox;

		/**The month the patient was LTFU*/
		int monthOfLTFU;
		/** The month that long term effects set in and mortality protection ends if they have incomplete treatment due to LTFU */
		int monthOfMortEfficacyStop;
		/** True if the patient will default if they remain LTFU long enough for long-term effects to set in, if they don't default they remain in active state */
		bool willTreatmentDefault;
		/** True if the patient will increase resistance upon default */
		bool willIncreaseResistanceUponDefault;
		/** The line of treatment that was stopped by LTFu */
		bool hasIncompleteTreatment;
		int incompleteTreatmentLine;
	}; /* end TBState */

	/* Accessor functions return const pointers to the Patient state subclass objects */
	const GeneralState *getGeneralState();
	const PedsState *getPedsState();
	const DiseaseState *getDiseaseState();
	const MonitoringState *getMonitoringState();
	const ProphState *getProphState();
	const ARTState *getARTState();
	const TBState *getTBState();

	/* Accessor functions return pointers to the CD4/HVL test state updaters */
	CD4TestUpdater *getCD4TestUpdater();
	HVLTestUpdater *getHVLTestUpdater();

	/* simulateMonth runs a single month of simulation for this patient, and updates
		its state and runStats statistics */
	void simulateMonth();
	/* Force new infection sets the patient to a new infected state -- to be used primarily by the transmission model*/
	void forceNewInfection();
	/* Changes the inputs that the patient uses to determine disease progression -- to be used primarily by the transmission model*/
	void setSimContext(SimContext *newSimContext);
	/* isAlive returns true if the patient is alive, false if dead */
	bool isAlive();

private:
	/** pointer to the simulation context*/
	SimContext *simContext;
	/** pointer to the statistics object */
	RunStats *runStats;
	/** pointer to the costs statistics object */
	CostStats *costStats;
	/** pointer to the trace object */
	Tracer *tracer;

	/** Patient state subclass object */
	GeneralState generalState;
	/** Patient state subclass object */
	PedsState pedsState;
	/** Patient state subclass object */
	DiseaseState diseaseState;
	/** Patient state subclass object */
	MonitoringState monitoringState;
	/** Patient state subclass object */
	ProphState prophState;
	/** Patient state subclass object */
	ARTState artState;
	/** Patient state subclass object */
	TBState tbState;
	/** State updater object */
	BeginMonthUpdater beginMonthUpdater;
	/** State updater object */
	HIVInfectionUpdater hivInfectionUpdater;
	/** State updater object */
	CHRMsUpdater chrmsUpdater;
	/** State updater object */
	DrugToxicityUpdater drugToxicityUpdater;
	/** State updater object */
	TBDiseaseUpdater tbDiseaseUpdater;
	/** State updater object */
	AcuteOIUpdater acuteOIUpdater;
	/** State updater object */
	MortalityUpdater mortalityUpdater;
	/** State updater object */
	CD4HVLUpdater cd4HVLUpdater;
	/** State updater object */
	HIVTestingUpdater hivTestingUpdater;
	/** State updater object */
	BehaviorUpdater behaviorUpdater;
	/** State updater object */
	DrugEfficacyUpdater drugEfficacyUpdater;
	/** State updater object */
	CD4TestUpdater cd4TestUpdater;
	/** State updater object */
	HVLTestUpdater hvlTestUpdater;
	/** State updater object */
	ClinicVisitUpdater clinicVisitUpdater;
	/** State updater object */
	TBClinicalUpdater tbClinicalUpdater;
	/** State updater object */
	EndMonthUpdater endMonthUpdater;
};

/** getGeneralState returns a const pointer to the GeneralState object */
inline const Patient::GeneralState *Patient::getGeneralState() {
	return &generalState;
}

/** getPedsState returns a const pointer to the PedsState object */
inline const Patient::PedsState *Patient::getPedsState() {
	return &pedsState;
}

/** getDiseaseState returns a const pointer to the DiseaseState object */
inline const Patient::DiseaseState *Patient::getDiseaseState() {
	return &diseaseState;
}

/** getMonitoringState returns a const pointer to the MonitoringState object */
inline const Patient::MonitoringState *Patient::getMonitoringState() {
	return &monitoringState;
}

/** getProphState returns a const pointer to the ProphState object */
inline const Patient::ProphState *Patient::getProphState() {
	return &prophState;
}

/** getARTState returns a const pointer to the ARTState object */
inline const Patient::ARTState *Patient::getARTState() {
	return &artState;
}

/** getTBState returns a const pointer to the TBState object */
inline const Patient::TBState *Patient::getTBState() {
	return &tbState;
}

/** getCD4TestUpdater returns a pointer to the CD4 test state updater */
inline CD4TestUpdater *Patient::getCD4TestUpdater() {
	return &cd4TestUpdater;
}

/** getHVLTestUpdater returns a pointer to the HVL test state updater */
inline HVLTestUpdater *Patient::getHVLTestUpdater() {
	return &hvlTestUpdater;
}

/** Force new infection if the patient is negative; to be called by the transmission model*/
inline void Patient::forceNewInfection() {
	if (diseaseState.infectedHIVState == SimContext::HIV_INF_NEG){
		this->hivInfectionUpdater.performHIVNewInfectionUpdates();
	}
}

/** Changes the inputs that the patient uses to determine disease progression -- to be used primarily by the transmission model*/
inline void Patient::setSimContext(SimContext *newSimContext){
	this->simContext = newSimContext;
	//Set simContext for all stateUpdaters
	beginMonthUpdater.setSimContext(newSimContext);
	hivInfectionUpdater.setSimContext(newSimContext);
	chrmsUpdater.setSimContext(newSimContext);
	drugToxicityUpdater.setSimContext(newSimContext);
	tbDiseaseUpdater.setSimContext(newSimContext);
	acuteOIUpdater.setSimContext(newSimContext);
	mortalityUpdater.setSimContext(newSimContext);
	cd4HVLUpdater.setSimContext(newSimContext);
	hivTestingUpdater.setSimContext(newSimContext);
	behaviorUpdater.setSimContext(newSimContext);
	drugEfficacyUpdater.setSimContext(newSimContext);
	cd4TestUpdater.setSimContext(newSimContext);
	hvlTestUpdater.setSimContext(newSimContext);
	clinicVisitUpdater.setSimContext(newSimContext);
	tbClinicalUpdater.setSimContext(newSimContext);
	endMonthUpdater.setSimContext(newSimContext);
}

/** isAlive returns true if the patient is alive, false if dead */
inline bool Patient::isAlive() {
	if (diseaseState.isAlive)
		return true;
	return false;
} /* end isAlive */


