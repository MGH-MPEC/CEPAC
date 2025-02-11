#pragma once

#include "include.h"

/**
	SimContext class contains all the natural history information needed for running
	simulations.  Contains functions to read all the necessary information from an input file.
	Data is read into subclasses that correspond to the tabs on the input sheet.  Read only
	access to this is information is provided through public accessor functions that return const
	pointers to these classes.
*/
class SimContext
{
public:
	/* Constructors and Destructor */
	SimContext(string runName);
	~SimContext(void);
	int counter;
	/* Misc cohort and simulation constants */
	/** The maximum number of years that could be lived by a patient */
	static const int AGE_YRS = 101;
	/** The youngest patient age possible (for life tables) */
	static const int AGE_STARTING = 0;
	/** The oldest patient age possible (for life tables) */
	static const int AGE_MAXIMUM = 100;
	/** The number of age categories */
	static const int AGE_CATEGORIES = 20;
	/** The number of age strata for custom age dist */
	static const int INIT_AGE_NUM_STRATA = 30;
	/** The size of SimContext::GENDER_TYPE */
	static const int GENDER_NUM = 2;
	/** The specific genders available */
	enum GENDER_TYPE {GENDER_MALE, GENDER_FEMALE};
	/** Strings correlated with SimContext::GENDER_TYPE */
	static const char *GENDER_STRS[];
	/** The size of SimContext::BOUNDS_TYPE */
	static const int NUM_BOUNDS = 2;
	/** The number of discount rates */
	static const int NUM_DISCOUNT_RATES = 4;
	/**The number of age categories for for art start policy in peds*/
	static const int NUM_ART_START_CD4PERC_PEDS=4;
	/** The bounds calculated */
	enum BOUNDS_TYPE {LOWER_BOUND, UPPER_BOUND};
	/**Conditionals */
	enum CONDITIONS_TYPE {AND,OR};
	/**Directions */
	enum DIRECTIONS_TYPE {LEFT,RIGHT};
	/** The number of generic risk factors used by the model */
	static const int RISK_FACT_NUM = 5;
	/** string labels for the generic risk factors */
	static char RISK_FACT_STRS[RISK_FACT_NUM][32];
	/** The type of longitudinal summary recording available: none, detailed monthly, brief monthly, detailed yearly */
	enum LONGIT_SUMM_TYPE {LONGIT_SUMM_NONE, LONGIT_SUMM_MTH_DET, LONGIT_SUMM_MTH_BRF, LONGIT_SUMM_YR_DET};
	/** A constant used for null values */
	static const int NOT_APPL = -1;
	/** The maximum number of patients allowed to be traced - may be temporarily increased for specific projects if deemed necessary */
	static const int MAX_NUM_TRACES = 100;
	/** The number of patients to be traced in the trace file (max 100 -- may be modified by GUI or input file input) */
	static int numPatientsToTrace;
	/** The maximum number of subcohorts allowed*/
	static const int MAX_NUM_SUBCOHORTS = 25;
	/** The number of transmission risk groups */
	static const int TRANSM_RISK_NUM = 3;
	/** The transmission risk groups */
	enum TRANSM_RISK {TRANSM_RISK_MSM, TRANSM_RISK_IDU, TRANSM_RISK_OTHER};
	/** Strings correlated with SimContext::TRANSM_RISK */
	static const char *TRANSM_RISK_STRS[];
	/** The number of transmission risk age categories */
	static const int TRANSM_RISK_AGE_NUM = 7;
	/** The number of output age categories */
	static const int OUTPUT_AGE_CAT_NUM = 14;
	static const char *OUTPUT_AGE_CAT_STRS[];
	/* CD4 and HVL strata constants */
	/** The size of SimContext::CD4_STRATA */
	static const int CD4_NUM_STRATA = 6;
	/** The CD4 strata */
	enum CD4_STRATA {CD4_VLO, CD4__LO, CD4_MLO, CD4_MHI, CD4__HI, CD4_VHI};
	/** Strings corresponding to SimContext::CD4_STRATA */
	static const char *CD4_STRATA_STRS[];
	/** The size of SimContext::PEDS_CD4_AGE_CAT */
	static const int PEDS_CD4_AGE_CAT_NUM = 2;
	/** The CD4 metric age categories */
	enum PEDS_CD4_AGE_CAT {CD4_PERC, CD4_ABSOLUTE};
	/** The size of SimContext::HVL_STRATA */
	static const int HVL_NUM_STRATA = 7;
	/** The viral load strata */
	enum HVL_STRATA {HVL_VLO, HVL__LO, HVL_MLO, HVL_MED, HVL_MHI, HVL__HI, HVL_VHI};
	/** Strings corresponding to SimContext::HVL_STRATA */
	static const char *HVL_STRATA_STRS[];
	/** The midpoints of the HVL strata corresponding to SimContext::HVL_STRATA */
	static const double HVL_STRATA_MIDPTS[];
	/** A constant fixing suppressed viral load to HVL__LO */
	static const int HVL_SUPPRESSION = HVL__LO;

	/* OI and cause of death constants */
	/** The number of OIs */
	static const int OI_NUM = 15;
	/** All availabled OIs */
	enum OI_TYPE {OI_1, OI_2, OI_3, OI_4, OI_5, OI_6, OI_7, OI_8,
		OI_9, OI_10, OI_11, OI_12, OI_13, OI_14, OI_15, OI_NONE};
	/** Strings corresponding to SimContext::OI_TYPE from RunSpecs input tab, RunSpecs E20-E34 */
	static char OI_STRS[OI_NUM][32];	  // from RunSpecs input tab, RunSpecs E20-E34
	/** The size of SimContext::DTH_CAUSES */
	static const int DTH_NUM_CAUSES = 38;
	/** Number of basic death causes (OIs, background mortality and HIV only) */
	static const int DTH_NUM_CAUSES_BASIC = 17;
	/** Enum of the death causes */
	enum DTH_CAUSES {DTH_OI_1, DTH_OI_2, DTH_OI_3, DTH_OI_4, DTH_OI_5, DTH_OI_6,
		DTH_OI_7, DTH_OI_8, DTH_OI_9, DTH_OI_10, DTH_OI_11, DTH_OI_12, DTH_OI_13,
		DTH_OI_14, DTH_OI_15, DTH_HIV, DTH_BKGD_MORT, DTH_ACTIVE_TB, 
		DTH_TOX_ART, DTH_TOX_PROPH, DTH_TOX_TB_PROPH, DTH_TOX_TB_TREATM, DTH_TOX_INFANT_HIV_PROPH,
		DTH_CHRM_1, DTH_CHRM_2, DTH_CHRM_3, DTH_CHRM_4, DTH_CHRM_5, DTH_CHRM_6, DTH_CHRM_7, DTH_CHRM_8, DTH_CHRM_9, DTH_CHRM_10,
		DTH_RISK_1, DTH_RISK_2, DTH_RISK_3, DTH_RISK_4, DTH_RISK_5};
	/** Strings corresponding to SimContext::DTH_CAUSES filled in with OIs and CHRMs */
	static char DTH_CAUSES_STRS[DTH_NUM_CAUSES][32];	// filled in with OIs and CHRMs
	/** Enum of the PSA background mortality modifier types */
	enum BACKGROUND_MORT_MOD_TYPES {MORT_MOD_INCREMENTAL, MORT_MOD_MULT};
	/** Size of SimContext::HIST_TYPE */
	static const int HIST_NUM = 2;
	/** The number of types of OI history (yes or no) */
	enum HIST_TYPE {HIST_N, HIST_Y};
	/** The size of SimContext::HIST_EXT_NUM */
	static const int HIST_EXT_NUM = 3;
	/** Enum of the extent of OI histories: no history of OIs in history, patient has a history of mild OIs and no severe OIs, patient has a history of at least one severe OI */
	enum HIST_EXT {
		HIST_EXT_N,			// no history of OI's in history
		HIST_EXT_MILD,		// patient has a history of mild OIs, and no severe OIs
		HIST_EXT_SEVR		// patient has a history of at least one severe OI
	};
	/** Strings corresponding to SimContext::HIST_EXT */
	static const char *HIST_OI_CATS_STRS[];

	/** This class keeps track of an individual mortality risk */
	class MortalityRisk {
	public:
		/** The cause of death for this risk */
		DTH_CAUSES causeOfDeath;
		/** The death rate ratio for this risk */
		double deathRateRatio;
		/** The cost accrued if this death occurs */
		double costDeath;
	};

	/* CHRMs constants */
	static const int CHRM_NUM = 10;
	enum CHRM_TYPE {CHRM_1, CHRM_2, CHRM_3, CHRM_4, CHRM_5, CHRM_6, CHRM_7, CHRM_8, CHRM_9, CHRM_10};
	static char CHRM_STRS[CHRM_NUM][32];	  // from CHRMs input tab, CHRMs B4-B7
	static const int CHRM_AGE_CAT_NUM = 7;
	static const char *CHRM_AGE_CAT_STRS[];
	static const int CHRM_TIME_PER_NUM = 3;
	static const int CHRM_ORPHANS_AGE_CAT_NUM = 15;
	static const int CHRM_ORPHANS_OUTPUT_AGE_CAT_NUM = 19;

	/* Cost tab constants */
	static const int COST_AGE_CAT_NUM = 7;
	
	/* Cost Stats constants */
	static const int LINKAGE_STATS_AGE_CAT_NUM = 9;
	static const char *LINKAGE_STATS_AGE_CAT_STRS[];
	static const int COST_REPORT_DISCOUNT_NUM = 2;
	enum COST_REPORT_DISCOUNT{COST_REPORT_DISCOUNTED, COST_REPORT_UNDISCOUNTED};
	//Subgroups of population for reporting cost statistics
	static const int COST_SUBGROUPS_NUM = 13;
	enum COST_SUBGROUPS{COST_SUBGROUP_ALL, COST_SUBGROUP_HIV_NEGATIVE, COST_SUBGROUP_HIV_POSITIVE,
		COST_SUBGROUP_PRE_LINKAGE,COST_SUBGROUP_INCARE_NOT_ON_ART, COST_SUBGROUP_ON_ART, COST_SUBGROUP_LTFU_AFTER_ART,
		COST_SUBGROUP_LTFU_NEVER_ON_ART, COST_SUBGROUP_RTC_AFTER_ART,COST_SUBGROUP_ON_ART_NEVER_LOST, COST_SUBGROUP_ON_ART_FIRST_SIX_MONTHS,
		COST_SUBGROUP_FIRST_LINE_ART, COST_SUBGROUP_SECOND_LINE_OR_HIGHER};
	static const char *COST_SUBGROUPS_STRS[];
	//Strata of CD4 for reporting in cost statistics
	static const int COST_CD4_STRATA_NUM = 8;
	enum COST_CD4_STRATA {COST_CD4_VLO, COST_CD4__LO, COST_CD4_MLO, COST_CD4_MHI, COST_CD4__HI, COST_CD4_VHI, COST_CD4_NONE, COST_CD4_ALL};
	static const char *COST_CD4_STRATA_STRS[];

	/*Peds Cost constants */
	static const int PEDS_COST_AGE_CAT_NUM = 4;
	enum PEDS_COST_AGE{PEDS_COST_AGE_1,PEDS_COST_AGE_2,PEDS_COST_AGE_3,PEDS_COST_AGE_4,PEDS_COST_AGE_ADULT};

	/*Peds ART cost categories*/
	static const int PEDS_ART_COST_AGE_CAT_NUM=6;
	enum PEDS_ART_COST_AGE{PEDS_ART_COST_AGE_5MTH, PEDS_ART_COST_AGE_11MTH, PEDS_ART_COST_AGE_2YR, PEDS_ART_COST_AGE_4YR, PEDS_ART_COST_AGE_7YR, PEDS_ART_COST_AGE_12YR, PEDS_ART_COST_AGE_ADULT};

	/* Treatment and therapy constants */
	/** The size of SimContext::CLINIC_VISITS */
	static const int CLINIC_VISITS_NUM = 3;
	/** The different types of clinic visits */
	enum CLINIC_VISITS {
		/** attend clinic only if initial visit or if on ART or OI proph */
		CLINIC_INITIAL,
		/** attend clinic if acute OI, or if initial visit, or if on ART or OI proph */
		CLINIC_INIT_ACUTE,
		/** always attend clinic visits if scheduled, or if acute OI, or if on ART or OI proph */
		CLINIC_SCHED
	};
	/** Strings corresponding to SimContext::CLINIC_VISITS */
	static const char* CLINIC_VISITS_STRS[];
	/** The size of SimContext::THERAPY_IMPL */
	static const int THERAPY_IMPL_NUM = 3;
	/** An enum of therapy implementation types: patient type never implementing ART or OI prophylaxis,
	 *  patient type implementing OI proph but not ART, and patient type implementing OI proph and ART
	 */
	enum THERAPY_IMPL {
		THERAPY_IMPL_NONE,		// patient type never implementing ART or OI prophylaxis
		THERAPY_IMPL_PROPH,		// patient type implementing OI proph but not ART
		THERAPY_IMPL_PROPH_ART	// patient type implementing OI proph and ART
	};
	/** The different types of emergencies that can trigger clinic visits - if no emergency then it is a regular visit **/
	enum EMERGENCY_TYPE {
		EMERGENCY_OI, // emergency clinic visit triggered by an acute OI
		EMERGENCY_TEST, // emergency clinic visit triggered by a CD4 or HVL test
		EMERGENCY_ART, // emergency clinic visit related to ART (failure, stop, etc.)
		EMERGENCY_PROPH, // emergency clinic visit related to OI prophylaxis
		EMERGENCY_NONE // non-emergency ("regular") clinic visit
	};
	/** Strings corresponding to the emergencies in SimContext::EMERGENCY_TYPE (for trace output; non-emergencies not currently included) */
	static const char* EMERGENCY_TYPE_STRS[];

	/** The size of SimContext::RTC_COEFF */
	static const int RTC_NUM_COEFF = 5;
	/** An enum of the return-to-care regression coefficients */
	enum RTC_COEFF {RTC_BACKGROUND, RTC_CD4, RTC_ACUTESEVEREOI, RTC_ACUTEMILDOI, RTC_TBPOS};
	/** The lost-to-follow up states: never lost, currently lost, returned from lost */
	enum LTFU_STATE {LTFU_STATE_NONE, LTFU_STATE_LOST, LTFU_STATE_RETURNED};

	/* Heterogeneity constants */
	/** The size of SimContext::RESP_TYPE */
	static const int RESP_NUM_TYPES = 3;
	/** An enum of heterogeneity response types */
	enum RESP_TYPE {RESP_TYPE_FULL, RESP_TYPE_PARTIAL, RESP_TYPE_NON};
	/** Strings corresponding to SimContext::RESP_TYPE */
	static const char *RESP_TYPE_STRS[];
	/**size of SimContext::HET_OUTCOME */
	static const int HET_NUM_OUTCOMES = 10;
	/** An enum of heterogeneity outcome types*/
	enum HET_OUTCOME{HET_OUTCOME_SUPP,HET_OUTCOME_LATEFAIL,HET_OUTCOME_ARTEFFECT_OI,HET_OUTCOME_ARTEFFECT_CHRMS,HET_OUTCOME_ARTEFFECT_MORT,HET_OUTCOME_RESIST,HET_OUTCOME_TOX,HET_OUTCOME_COST,HET_OUTCOME_RESTART,HET_OUTCOME_RESUPP};
	/** Strings corresponding to SimContexxt::HET_OUTCOME*/
	static const char *HET_OUTCOME_STRS[];
	/** The number of heterogeneity age categories */
	static const int RESP_AGE_CAT_NUM = 7;
	/** An enum of the possible distributions for ART specific heterogeneity regression coefficients */
	enum HET_ART_LOGIT_DISTRIBUTION{HET_ART_LOGIT_DISTRIBUTION_NORMAL, HET_ART_LOGIT_DISTRIBUTION_TRUNC_NORMAL, HET_ART_LOGIT_DISTRIBUTION_SQROOT};
	/** The number of Intervention periods */
	static const int HET_INTV_NUM_PERIODS = 5;

	/* ART and Prophylaxis constants */
	/** The number of ART lines */
	static const int ART_NUM_LINES = 10;
	/** The size of SimContext::ART_STATES */
	static const int ART_NUM_STATES = 2;
	/** An enum representing the ART states (on or off ART) */
	enum ART_STATES {ART_OFF_STATE, ART_ON_STATE};
	/** The size of SimContext::ART_EFF_TYPE */
	static const int ART_EFF_NUM_TYPES = 2;
	/** An enum of the ART efficacy types (suppressed and failed) */
	enum ART_EFF_TYPE {ART_EFF_SUCCESS, ART_EFF_FAILURE};
	/** Strings corresponding to SimContext::ART_EFF_TYPE */
	static const char *ART_EFF_STRS[];
	/** The size of SimContext::CD4_RESPONSE_TYPE */
	static const int CD4_RESPONSE_NUM_TYPES = 4;
	/** An enum of the possible CD4 response types */
	enum CD4_RESPONSE_TYPE {CD4_RESPONSE_1, CD4_RESPONSE_2, CD4_RESPONSE_3, CD4_RESPONSE_4};
	/** Strings corresponding to SimContext::CD4_RESPONSE_TYPE */
	static const char *CD4_RESPONSE_STRS[];
	/** The number of ART subregimens (per regimen) */
	static const int ART_NUM_SUBREGIMENS = 4;
	/** The size of SimContext::ART_TOX_SEVERITY */
	static const int ART_NUM_TOX_SEVERITY = 3;
	static const int ART_NUM_TOX_PER_SEVERITY = 6;
	enum ART_TOX_SEVERITY {ART_TOX_MINOR, ART_TOX_CHRONIC, ART_TOX_MAJOR};
	/** Strings corresponding to SimContext::ART_TOX_SEVERITY */
	static const char *ART_TOX_SEVERITY_STRS[];
	/** An enum of the duration of ART toxicity */
	enum ART_TOX_DUR {ART_TOX_DUR_MONTH, ART_TOX_DUR_SUBREG, ART_TOX_DUR_REG, ART_TOX_DUR_DEATH};
	/** The size of SimContext::ART_FAIL_TYPE */
	static const int ART_NUM_FAIL_TYPES = 3;
	/** An enum of the ART failure types */
	enum ART_FAIL_TYPE {ART_FAIL_VIROLOGIC, ART_FAIL_IMMUNOLOGIC, ART_FAIL_CLINICAL, ART_FAIL_NOT_FAILED};
	/** Strings corresponding to SimContext::ART_FAIL_TYPE */
	static const char *ART_FAIL_TYPE_STRS[];
	/** The size of SimContext::ART_FAIL_BY_OI */
	static const int ART_FAIL_BY_OI_NUM = 4;
	/** An enum of the ART failure by OI types */
	enum ART_FAIL_BY_OI {ART_FAIL_BY_OI_NONE, ART_FAIL_BY_OI_PRIMARY,
		ART_FAIL_BY_OI_SECONDARY, ART_FAIL_BY_OI_ANY};
	/** The size of SimContext::ART_STOP_TYPE less two */
	static const int ART_NUM_STOP_TYPES = 8;
	/** An enum of the ART stopping types */
	enum ART_STOP_TYPE {ART_STOP_MAX_MTHS, ART_STOP_MAJ_TOX, ART_STOP_CHRN_TOX, ART_STOP_FAIL, ART_STOP_CD4, ART_STOP_SEV_OI,
		ART_STOP_FAIL_MTHS, ART_STOP_LTFU, ART_STOP_NOT_STOPPED, ART_STOP_STI};
	/** Strings corresponding to SimContext::ART_STOP_TYPE */
	static const char *ART_STOP_TYPE_STRS[];
	/** The number of months to record ART stats */
	static const int ART_NUM_MTHS_RECORD = 3;
	/** The number of STI cycles */
	static const int STI_NUM_CYCLES = 5;
	/** The number of STI periods */
	static const int STI_NUM_PERIODS = 2;
	/** The number of STIs to track */
	static const int STI_NUM_TRACKED = 30;
	/** An enum of STI states */
	enum STI_STATE {STI_STATE_NONE, STI_STATE_INTERRUPT, STI_STATE_RESTART, STI_STATE_ENDPOINT};
	/** One more than the number of prophylaxis types for iterating */
	static const int PROPH_NUM = 3;
	/** The size of SimContext::PROPH_TYPE */
	static const int PROPH_NUM_TYPES = 2;
	/** An enum of the types of prophylaxis (primary and secondary) */
	enum PROPH_TYPE {PROPH_PRIMARY, PROPH_SECONDARY};
	/** Strings corresponding to SimContext::PROPH_TYPE */
	static const char *PROPH_TYPE_STRS[];
	/** An enum of prophylaxis toxicity types */
	enum PROPH_TOX_TYPE {PROPH_TOX_NONE, PROPH_TOX_MINOR, PROPH_TOX_MAJOR};

	/** A class representing a single ART Toxicity Effect */
	class ARTToxicityEffect {
	public:
		/** The month of the toxicity start */
		int monthOfToxStart;
		/** The SimContext::ART_TOX_SEVERITY corresponding to this toxicity */
		ART_TOX_SEVERITY toxSeverityType;
		/** The toxicity number */
		int toxNum;
		/** The corresponding ART regimen number */
		int ARTRegimenNum;
		/** The corresponding ART subregimen number */
		int ARTSubRegimenNum;
	};

	/** An enum of CD4 envelope types */
	enum ENVL_CD4_TYPE {ENVL_CD4_OVERALL, ENVL_CD4_INDIV, ENVL_CD4_PERC_OVERALL, ENVL_CD4_PERC_INDIV};
	/** A class representing a single CD4 envelope */
	class CD4Envelope {
	public:
		/** True if this is an active envelope */
		bool isActive;
		/** The regimen number corresponding to this envelope */
		int regimenNum;
		/** The month this envelope started */
		int monthOfStart;
		/** The slope of this envelope */
		double slope;
		/** The current value of this envelope */
		double value;
	};

	/* Cost and QOL constants */
	/** The size of SimContext::COST_TYPES */
	static const int COST_NUM_TYPES = 4;
	/** An enum of the different cost types */
	enum COST_TYPES {
		/** direct medical costs */
		COST_DIR_MED,		// direct medical costs
		/** direct non-medical costs */
		COST_DIR_NONMED,	// direct non-medical costs
		/** time costs */
		COST_TIME,			// time costs
		/** indirect costs */
		COST_INDIR,			// indirect costs
		/**  sum of costs, only used in special cases */
		COST_SUM			// sum of costs, only used in special cases
	};

	/** Strings corresponding to SimContext::Cost_Types */
	static const char *COST_TYPES_STRS[];
	/** An enum of the possible methods for accumulating QOL */
	enum QOL_CALC_TYPE{MULT, ADD, MIN, MARGINAL };

	/* TB specific constants */
	/** Size of SimContext::TB_STRAIN */
	static const int TB_NUM_STRAINS = 3;
	/** An enum of the different TB strains: Drug Sensitive (DS), Multi-Drug Resistant (MDR), eXtensive Drug Resistant (XDR) */
	enum TB_STRAIN {TB_STRAIN_DS, TB_STRAIN_MDR, TB_STRAIN_XDR};
	/** Strings corresponding to SimContext::TB_STRAIN */
	static const char *TB_STRAIN_STRS[];
	/** Number of TB states */
	static const int TB_NUM_STATES = 6;
	/** An enum of the possible TB states */
	enum TB_STATE {TB_STATE_UNINFECTED, TB_STATE_LATENT, TB_STATE_ACTIVE_PULM, TB_STATE_ACTIVE_EXTRAPULM, TB_STATE_PREV_TREATED, TB_STATE_TREAT_DEFAULT};
	/** Strings corresponding to SimContext::TB_STATE */
	static const char *TB_STATE_STRS[];
	/** Number of TB tracker variables */
	static const int TB_NUM_TRACKER = 3;
	/** An enum of the possible TB tracker variables */
	enum TB_TRACKER {TB_TRACKER_SPUTUM_HI, TB_TRACKER_IMMUNE_REACTIVE, TB_TRACKER_SYMPTOMS};
	/** Strings corresponding to SimContext::TB_TRACKER */
	static const char *TB_TRACKER_STRS[];
	/** An enum of the types of TB infection */
	enum TB_INFECT {TB_INFECT_PREVALENT, TB_INFECT_INITIAL, TB_INFECT_REINFECT, TB_INFECT_REACTIVATE, TB_INFECT_RELAPSE};
	/** An enum of the different types of calendar-based TB natural history rate multipliers **/
	enum TB_MULT_TYPE {TB_MULT_NONE, TB_MULT_ACTIVATION, TB_MULT_MORTALITY};
	/** The number of TB infection age categories */
	static const int TB_INFECT_NUM_AGE_CAT = 7;
	/** The number of stages used in TB treatment */
	static const int TB_TREATM_STAGE_NUM = 2;
		
	/** Size of TB_DIAG_INIT_POLICY*/
	static const int TB_DIAG_INIT_POLICY_NUM = 6;
	static const int TB_DIAG_INIT_POLICY_INTV_NUM = 4;
	/** An enum of the types of TB diagnostics initiation policies */
	enum TB_DIAG_INIT_POLICY {TB_DIAG_INIT_HIV, TB_DIAG_INIT_SYMPTOMS, TB_DIAG_INIT_OI, TB_DIAG_INIT_CD4, TB_DIAG_INIT_MONTH, TB_DIAG_INIT_INTERVAL};
	/** Size of TB_DIAG_TEST_ORDER_NUM */
	static const int TB_DIAG_TEST_ORDER_NUM = 4;
	/** Size of TB_DIAG_STATUS */
	static const int TB_DIAG_STATUS_NUM = 2;
	/** An enum of tb diagnostic test results */
	enum TB_DIAG_STATUS {TB_DIAG_STATUS_POS, TB_DIAG_STATUS_NEG};
	/** Strings corresponding to SimContext::TB_DIAG_STATUS */
	static const char *TB_DIAG_STATUS_STRS[];
	/** Size of SimContext::TB_CARE */
	static const int TB_CARE_NUM = 3;
	/** An enum of the different states of TB care */
	enum TB_CARE {TB_CARE_UNLINKED, TB_CARE_IN_CARE, TB_CARE_LTFU};
	/** unfavorable outcomes for TB treatment (first regimen only)*/
	static const int TB_NUM_UNFAVORABLE = 4;
	enum TB_UNFAVORABLE{TB_UNFAVORABLE_FAILURE,TB_UNFAVORABLE_RELAPSE, TB_UNFAVORABLE_LTFU,TB_UNFAVORABLE_DEATH};
	static const char *TB_UNFAVORABLE_STRS[];

	static const int TB_NUM_TESTS = 4;
	static const int TB_NUM_TREATMENTS = 10;
	static const int TB_NUM_PROPHS = 5;
	/**Number of categories for time spent TB LTFU, used for TB RTC */
	static const int TB_RTC_TIME_CAT_NUM = 5;

	/* Testing specific constants */
	/** Size of SimContext::HIV_ID */
	static const int HIV_ID_NUM = 3;
	/** An enum of HIV testing status: HIV negative, unidentified HIV positive, identified HIV positive */
	enum HIV_ID {HIV_ID_NEG, HIV_ID_UNID, HIV_ID_IDEN};
	/** Strings corresponding to SimContext::HIV_ID */
	static const char *HIV_ID_STRS[];
	/** Size of SimContext::HIV_INF */
	static const int HIV_INF_NUM = 4;
	/** An enum of the types of HIV infection: negative (no infection), asymptomatic chronic, symptomatic chronic, and acute syndrome*/
	enum HIV_INF {HIV_INF_NEG, HIV_INF_ASYMP_CHR_POS, HIV_INF_SYMP_CHR_POS, HIV_INF_ACUTE_SYN};
	/** Size of SimContext::HIV_POS */
	static const int HIV_POS_NUM=3;
	/** Strings corresponding to SimContext::HIV_POS */
	static const char *HIV_POS_STRS[];
	/** An enum of the types of Positive HIV Infection: asymptomatic chronic, symptomatic chronic, and acute syndrome - identical to HIV_INF, but without HIV_INF_NEG as the first element */
	enum HIV_POS {HIV_POS_ASYMP_CHR_POS, HIV_POS_SYMP_CHR_POS, HIV_POS_ACUTE_SYN};
	/** Size of SimContext::HIV_EXT_INF */
	static const int HIV_EXT_INF_NUM = 5;
	/** An extended enum of HIV status, similar to SimContext::HIV_INF but including risk type for HIV negative patients (high (HI) or low (LO)) */
	enum HIV_EXT_INF {HIV_EXT_INF_NEG_HI, HIV_EXT_INF_ASYMP_CHR_POS, HIV_EXT_INF_SYMP_CHR_POS, HIV_EXT_INF_ACUTE_SYN, HIV_EXT_INF_NEG_LO};
	/** Strings corresponding to SimContext::HIV_EXT_INF */
	static const char *HIV_EXT_INF_STRS[];
	/** Size of SimContext::HIV_BEHAV */
	static const int HIV_BEHAV_NUM = 2;
	/** Enums of risk levels for HIV negative patients (HI or LO) */
	enum HIV_BEHAV {HIV_BEHAV_HI, HIV_BEHAV_LO};
	/** Size of SimContext::HIV_DET */
	static const int HIV_DET_NUM = 10;
	/** An enum of the different HIV detection methods */
	enum HIV_DET {HIV_DET_INITIAL, HIV_DET_SCREENING, HIV_DET_SCREENING_PREV_DET, HIV_DET_BACKGROUND, HIV_DET_BACKGROUND_PREV_DET, HIV_DET_OI, HIV_DET_OI_PREV_DET, HIV_DET_TB, HIV_DET_TB_PREV_DET, HIV_DET_UNDETECTED};
	/** Strings corresponding to SimContext::HIV_DET */
	static const char *HIV_DET_STRS[];
	/** Size of SimContext::HIV_CARE */
	static const int HIV_CARE_NUM =6;
	static const char *HIV_CARE_STRS[];

	/** An enum of the different states of HIV care */
	enum HIV_CARE {HIV_CARE_NEG, HIV_CARE_UNDETECTED, HIV_CARE_UNLINKED, HIV_CARE_IN_CARE, HIV_CARE_LTFU, HIV_CARE_RTC};

	/** Size of SimContext::HIV_POS_PREP_STATE  */
	static const int HIV_POS_PREP_STATES_NUM = 3;
	/** An enum of the different PrEP states for HIV positive patients */
	enum HIV_POS_PREP_STATE { HIV_POS_ON_PREP, HIV_POS_PREP_DROPOUT, HIV_POS_NEVER_PREP};
	/** Size of SimContext::EVER_PREP */
	static const int EVER_PREP_NUM = 2;
	/** An enum of the possible states for whether a patient has ever had PrEP */
	enum EVER_PREP {NEVER_PREP, EVER_PREP};
	/**Number of incidence reduction multiplier periods*/
	static const int INC_REDUC_PERIODS_NUM = 6;
	/** Number of age categories for HIV incidence */
	static const int AGE_CAT_HIV_INC = 20;
	/** Number of HIV test frequencies */
	static const int HIV_TEST_FREQ_NUM = 5;
	/** Number of test acceptance types */
	static const int TEST_ACCEPT_NUM = 3;
	/** Size of SimContext::TEST_RESULT */
	static const int TEST_RESULT_NUM = 4;
	/** An enum of the different HIV test results */
	enum TEST_RESULT {TEST_TRUE_POS, TEST_FALSE_POS, TEST_TRUE_NEG, TEST_FALSE_NEG};
	/** Strings corresponding to SimContext::TEST_RESULT */
	static const char *TEST_RESULT_STRS[];

	/* Pediatrics specific constants */
	/** Size of SimContext::PEDS_HIV_STATE */
	static const int PEDS_HIV_NUM = 4;
	/** Number of elements of SimContext::PEDS_HIV_STATE that don't correspond to HIV negative **/
	static const int PEDS_HIV_POS_NUM = 3;
	/** An enum of the different Pediatric HIV states: intra-uterine infected, intra-partum infected, post-partum infected, and uninfected */
	enum PEDS_HIV_STATE {PEDS_HIV_POS_IU, PEDS_HIV_POS_IP, PEDS_HIV_POS_PP, PEDS_HIV_NEG};
	/** Strings corresponding to SimContext::PEDS_HIV_STATE */
	static const char *PEDS_HIV_STATE_STRS[];
	/** Number of scheduled base assays allowed for EID - one per visit for 24 visits*/
	static const int EID_NUM_ASSAYS = 24;
	/** Number of infant HIV Proph regimens */
	static const int INFANT_HIV_PROPHS_NUM = 4;
	/** Number of age categories for infant Proph */
	static const int INFANT_PROPH_AGES_NUM = 36;
	/** Number of age categories for infant Proph costs */
	static const int INFANT_PROPH_COST_AGES_NUM = 8;

	/** Number of age categories for EID cost of visit */
	static const int EID_COST_VISIT_NUM = 24;
	/** Number of HIV tests used by EID */
	static const int EID_NUM_TESTS = 10;
	/** Maternal Status (uninfected, chronic HIV cd4 >350, chronic HIV cd4 < 350, acute HIV)**/
	static const int PEDS_MATERNAL_STATUS_NUM = 4;
	enum PEDS_MATERNAL_STATUS{PEDS_MATERNAL_STATUS_NEG, PEDS_MATERNAL_STATUS_CHR_HIGH, PEDS_MATERNAL_STATUS_CHR_LOW, PEDS_MATERNAL_STATUS_ACUTE};
	/** Strings corresponding to PEDS_MATERNAL_STATUS */
	static const char *PEDS_MATERNAL_STATUS_STRS[];
	/** Maternal Status known positive or not known positive */
	static const int PEDS_MATERNAL_KNOWLEDGE_NUM = 2;
	enum PEDS_MATERNAL_KNOWLEDGE{PEDS_MATERNAL_KNOWLEDGE_NOT_KNOWN_POSITIVE, PEDS_MATERNAL_KNOWLEDGE_KNOWN_POSITIVE};
	/** number of month categories for sensitivity and specificity of EID tests */
	static const int EID_TEST_AGE_CATEGORY_NUM = 18;
	/** Status of confirmatory tests (not a confirmatory, 1st confirmatory, second confirmatory) */
	static const int EID_TEST_TYPE_NUM = 3;
	enum EID_TEST_TYPE{EID_TEST_TYPE_BASE, EID_TEST_TYPE_FIRST_CONF, EID_TEST_TYPE_SECOND_CONF};
	static const char *EID_TEST_TYPE_STRS[];

	static const int ADOLESCENT_NUM_AGES = 18;
	static const int ADOLESCENT_NUM_ART_AGES = 5;

	/** Holds test state of pending Eid test result or scheduled EID tests*/
	class EIDTestState{
	public:
		//The index of the base HIV test that starts the chain of confirmatory tests
		int baseAssay;
		//The index of the HIV test being used
		int testAssay;
		/** whether this is normal test, first confirmatory test, or second confirmatory */
		SimContext::EID_TEST_TYPE testType;
		/** If this test was caused by an acute OI, the OI type that triggered it (OI_NONE if not triggered by OI)*/
		OI_TYPE triggeredByOI;

		/** result of the test */
		//If this is for a scheduled EID test, result will not be used
		bool result;
		int monthToReturn;
		//whether the result will be returned to the patient
		bool returnToPatient;
	};

	/** Size of PEDS_EXPOSED_CONDITIONS */
	static const int PEDS_EXPOSED_CONDITIONS_NUM = 4;
	/** Enum of user-defined pediatric HIV exposure conditions, used for mortality and aggregate HIV exposure outputs*/
	enum PEDS_EXPOSED_CONDITIONS {MOM_CHRONIC_PREGNANCY, MOM_ACUTE_PREGNANCY, MOM_ACUTE_BREASTFEEDING, MOM_ON_ART_UNEXPOSED};
	/** Size of PEDS_EXPOSED_BREASTFEEDING*/
	static const int PEDS_EXPOSED_BREASTFEEDING_NUM = 2;
	/** Enum of additional categories used for monthly HIV exposed uninfected outputs, independent of the user-defined exposure conditions for mortality */
	enum PEDS_EXPOSED_BREASTFEEDING {EXPOSED_BF_MOM_CHRONIC, EXPOSED_BF_MOM_ACUTE};	
	/** The number of month a pediatric patient's mother stays in the acute state */
	static const int PEDS_MOM_MTHS_ACUTE = 3;
	/** Size of SimContext::PEDS_BF_TYPE */
	static const int PEDS_BF_NUM = 4;
	/** An enum of the breast feeding types: exclusive breast feeding, mixed, complementary, and replacement feeding */
	enum PEDS_BF_TYPE {PEDS_BF_EXCL, PEDS_BF_MIXED, PEDS_BF_COMP, PEDS_BF_REPL};
	/** Strings corresponding to SimContext::PEDS_BF_TYPE */
	static const char *PEDS_BF_TYPE_STRS[];
	/** Age at which a patient should transition from exclusive or mixed to complementary breast feeding */
	static const int PEDS_BF_STOP_EXCL_MIXED = 6;
	/** Minimum age at which to stop breastfeeding safely; if they stop at a safe age the replacement feeding death rate ratio will not be applied */
	static const int PEDS_SAFE_BF_STOP_AGE = 36;
	/** Number of age categories for pp maternal ART status */
	static const int PEDS_PP_MATERNAL_ART_STATUS_NUM = 24;
	/** Number of age categories that just correspond to infancy (from SimContext::PEDS_AGE_CAT) */
	static const int PEDS_AGE_INFANT_NUM = 7;
	/** Number of age categories that just correspond to infancy and early childhood (from SimContext::PEDS_AGE_CAT) */
	static const int PEDS_AGE_EARLY_NUM = 10;
	/** Number of age categories that just correspond to infancy and all of childhood (from SimContext::PEDS_AGE_CAT) */
	static const int PEDS_AGE_CHILD_NUM = 11;
	/** An enum of the pediatric age categories */
	enum PEDS_AGE_CAT {PEDS_AGE_2MTH, PEDS_AGE_5MTH, PEDS_AGE_8MTH, PEDS_AGE_11MTH, PEDS_AGE_14MTH,
		PEDS_AGE_17MTH, PEDS_AGE_23MTH, PEDS_AGE_2YR, PEDS_AGE_3YR, PEDS_AGE_4YR, PEDS_AGE_LATE, PEDS_AGE_ADULT};
	/** Strings corresponding to SimContext::PEDS_AGE_CAT */
	static const char *PEDS_AGE_CAT_STRS[];
	/** Age cut off in years for "early childhood" */
	static const int PEDS_YEAR_EARLY = 5;
	/** Final age cut off in years for "late childhood */
	static const int PEDS_YEAR_LATE = 13;
	/** Size of SimContext::PEDS_CD4_PERC */
	static const int PEDS_CD4_PERC_NUM = 8;
	/** An enum of the pediatric CD4 percentage categories */
	enum PEDS_CD4_PERC {PEDS_CD4_PERC_L5, PEDS_CD4_PERC_L10, PEDS_CD4_PERC_L15, PEDS_CD4_PERC_L20,
		PEDS_CD4_PERC_L25, PEDS_CD4_PERC_L30, PEDS_CD4_PERC_L35, PEDS_CD4_PERC_HIGHER};
	/** Strings corresponding to SimContext:: PEDS_CD4_PERC */
	static const char *PEDS_CD4_PERC_STRS[];


	/** RunSpecsInputs class contains inputs from the RunSpecs tab,
		one per simulation context */
	class RunSpecsInputs {
	public:
		/* Shortcuts and Miscellaneous inputs */
		/** RunSpecs D3 */
		string runSetName;
		string runName;
		/** RunSpecs D6 */
		int numCohorts;
		/** RunSpecs D7 */
		double discountFactor;
		double originalDiscRate;
		/** RunSpecs E9 */
		double maxPatientCD4;
		/** RunSpecs F36-F38 */
		int monthRecordARTEfficacy[ART_NUM_MTHS_RECORD];
		/** RunSpecs E12 */
		bool randomSeedByTime;
		/** RunSpecs E37 */
		string userProgramLocale;
		/** RunSpecs E40-G40 */
		string inputVersion;
		string modelVersion;
		/** RunSpecs B22 */
		bool OIsIncludeTB;
		/** RunSpecs C46-C60 */
		double OIsFractionOfBenefit[OI_NUM];
		/** RunSpecs C67-E81 **/
		bool severeOIs[OI_NUM];
		/** RunSpecs C87-E91 **/
		double CD4StrataUpperBounds[CD4_NUM_STRATA-1];

		/* Logging inputs */
		/** RunSpecs E14 **/
		LONGIT_SUMM_TYPE longitLoggingLevel;
		/** RunSpecs H20-H34 **/
		int firstOIsLongitLogging[OI_NUM];

		/** RunSpecs N8 **/
		bool enableOIHistoryLogging;
		/** RunSpecs N9 **/
		int numARTFailuresForOIHistoryLogging;
		/** RunSpecs N12-O12 **/
		double CD4BoundsForOIHistoryLogging[NUM_BOUNDS];
		/** RunSpecs N14-O14 **/
		int HVLBoundsForOIHistoryLogging[NUM_BOUNDS];
		/** RunSpecs N17-N31 **/
		bool OIsToExcludeOIHistoryLogging[OI_NUM];

		/* Output inputs */
		/** RunSpecs AA3 **/
		bool enableMultipleDiscountRates;
		/** RunSpecs X8-Y10 */
		double multDiscountRatesCost[NUM_DISCOUNT_RATES];
		double multDiscountRatesBenefit[NUM_DISCOUNT_RATES];
	}; /* end RunSpecsInputs */

	/** OutputInputs class contains inputs from the Output tab,
		one per simulation context */
	class OutputInputs {
	public:
		int traceNumSelection;
		/* Subcohort Inputs*/
		/** Output C5 **/
		bool enableSubCohorts;
		/** Output H5 **/
		bool enableDetailedCostOutputs;
		/** Output A12-A36 **/
		int subCohorts[MAX_NUM_SUBCOHORTS];

	}; /* end OutputInputs */

	/** CohortInputs class contains inputs from the Cohort tab,
		one per simulation context */
	class CohortInputs {
	public:
		/* Initial characteristics and history inputs */
		/** Cohort C5 **/
		double initialCD4Mean;
		/** Cohort C6 **/
		double initialCD4StdDev;
		/** Cohort G6 **/
		bool enableSquareRootTransform;
		/** Cohort C10-H16 **/
		double initialHVLDistribution[CD4_NUM_STRATA][HVL_NUM_STRATA];
		/**Cohort C20 **/
		double initialAgeMean;
		/**Cohort C21 **/
		double initialAgeStdDev;
		/**Cohort G20 **/
		bool useCustomAgeDist;
		/**Cohort M21-M50 + N21-N50 **/
		double ageStrata[2 * INIT_AGE_NUM_STRATA];
		/**Cohort O21-O50 **/
		double ageProbs[INIT_AGE_NUM_STRATA];
		/**Cohort C23-C24 **/
		double maleGenderDistribution;
		/**Cohort C26 **/
		double OIProphNonComplianceRisk;
		/**Cohort C27 **/
		double OIProphNonComplianceDegree;
		/**Cohort G46-G48 **/
		double clinicVisitTypeDistribution[CLINIC_VISITS_NUM];
		/**Cohort G51-G53 **/
		double therapyImplementationDistribution[THERAPY_IMPL_NUM];
		/**Cohort C57-C60 **/
		double CD4ResponseTypeOnARTDistribution[CD4_RESPONSE_NUM_TYPES];
		/**Cohort C67-H185 **/
		double probOIHistoryAtEntry[CD4_NUM_STRATA][HVL_NUM_STRATA][OI_NUM];
		/**Cohort M6-Q6 **/
		double probRiskFactorPrev[RISK_FACT_NUM];
		/**Cohort M7-Q7 **/
		double probRiskFactorIncid[RISK_FACT_NUM];

		/**Cohort W3 **/
		bool showTransmissionOutput;
		/** Cohort V8-AI15 **/
		double transmRateOnART[CD4_NUM_STRATA][HVL_NUM_STRATA];
		double transmRateOnARTAcute[CD4_NUM_STRATA];
		double transmRateOffART[CD4_NUM_STRATA][HVL_NUM_STRATA];
		double transmRateOffARTAcute[CD4_NUM_STRATA];
		/** Cohort W17-W18 **/
		bool transmUseHIVTestAcuteDefinition;
		int transmAcuteDuration;
		/** Cohort AF17-AH19 **/
		int transmRateMultInterval[2];
		double transmRateMult[3];
		/** Cohort V22-AI24 **/
		double transmRiskDistrib[GENDER_NUM][TRANSM_RISK_AGE_NUM][TRANSM_RISK_NUM];
		/** Cohort X26-Z26 **/
		int transmRiskMultBounds[2];
		/** Cohort V28-X30 **/
		double transmRiskMult[TRANSM_RISK_NUM][3];
		/** Cohort X35 **/
		bool useDynamicTransm;
		bool updateDynamicTransmInc;
		/** Cohort AH40 **/
		double dynamicTransmHRGTransmissions;
		/** Cohort Z42-Z47**/
		double dynamicTransmPropHRGAttributable;
		double dynamicTransmNumHIVPosHRG;
		double dynamicTransmNumHIVNegHRG;
		int dynamicTransmWarmupSize;
		bool keepPrEPAfterWarmup;
		bool usePrEPDuringWarmup;
		/** Cohort X51-X53**/
		bool useTransmEndLifeHVLAdjustment;
		double transmEndLifeHVLAdjustmentCD4Threshold;
		int transmEndLifeHVLAdjustmentARTLineThreshold;
	}; /* end CohortInputs */

	/** TreatmentInputs class contains inputs from the Treatment tab,
		one per simulation context */
	class TreatmentInputs {
	public:
		/* ART start policy class */
		class ARTStartPolicy {
		public:
			double CD4BoundsOnly[NUM_BOUNDS];
			int HVLBoundsOnly[NUM_BOUNDS];
			double CD4BoundsWithHVL[NUM_BOUNDS];
			int HVLBoundsWithCD4[NUM_BOUNDS];
			bool OIHistory[OI_NUM];
			int numOIs;
			double CD4BoundsWithOIs[NUM_BOUNDS];
			bool OIHistoryWithCD4[OI_NUM];
			bool ensureSuppFalsePositiveFailure;
			int minMonthNum;
			int maxMonthNum;
			int monthsSincePrevRegimen;
		};
		/* ART failure policy class */
		class ARTFailPolicy {
		public:
			int HVLNumIncrease;
			int HVLBounds[NUM_BOUNDS];
			bool HVLFailAtSetpoint;
			int HVLMonthsFromInit;
			double CD4PercentageDrop;
			bool CD4BelowPreARTNadir;
			double CD4BoundsOR[NUM_BOUNDS];
			double CD4BoundsAND[NUM_BOUNDS];
			int CD4MonthsFromInit;
			ART_FAIL_BY_OI OIsEvent[OI_NUM];
			int OIsMinNum;
			int OIsMonthsFromInit;
			int diagnoseNumTestsFail;
			bool diagnoseUseHVLTestsConfirm;
			bool diagnoseUseCD4TestsConfirm;
			int diagnoseNumTestsConfirm;
		};
		/* ART stopping policy class */
		class ARTStopPolicy {
		public:
			int maxMonthsOnART;
			bool withMajorToxicty;
			bool afterFailImmediate;
			double afterFailCD4LowerBound;
			bool afterFailWithSevereOI;
			int afterFailMonthsFromObserved;
			int afterFailMinMonthNum;
			int afterFailMonthsFromInit;
			int nextLineAfterMajorTox;
		};
		/* Prophylaxis policy classes */
		class ProphStartPolicy {
		public:
			bool useOrEvaluation;
			double currCD4Bounds[NUM_BOUNDS];
			double minCD4Bounds[NUM_BOUNDS];
			int OIHistory[OI_NUM];
			int minMonthNum;
		};
		class ProphStopPolicy {
		public:
			bool useOrEvaluation;
			double currCD4Bounds[NUM_BOUNDS];
			double minCD4Bounds[NUM_BOUNDS];
			int OIHistory[OI_NUM];
			int minMonthNum;
			int monthsOnProph;
		};

		/* Clinical visit and testing inputs */
		//Treatment AO18 **/
		int clinicVisitInterval;
		//Treatment AM22-BA23  **/
		double probDetectOIAtEntry[OI_NUM];
		double probDetectOISinceLastVisit[OI_NUM];
		//Treatment AM26-BA26  **/
		double probSwitchSecondaryProph[OI_NUM];
		//Treatment AO4-AV4  **/
		double testingIntervalCD4Threshold;
		int testingIntervalARTMonthsThreshold;
		int testingIntervalLastARTMonthsThreshold;
		//Treatment AN7-AX8  **/
		int CD4TestingIntervalPreARTHighCD4;
		int CD4TestingIntervalPreARTLowCD4;
		int CD4TestingIntervalOnART[2];
		int CD4TestingIntervalOnLastART[2];
		int CD4TestingIntervalPostART;
		int HVLTestingIntervalPreARTHighCD4;
		int HVLTestingIntervalPreARTLowCD4;
		int HVLTestingIntervalOnART[2];
		int HVLTestingIntervalOnLastART[2];
		int HVLTestingIntervalPostART;
		//Treatment AN12-AN15  **/
		double probHVLTestErrorHigher;
		double probHVLTestErrorLower;
		double CD4TestStdDevPercentage;
		double CD4TestBiasMean;
		double CD4TestBiasStdDevPercentage;

		//Treatment AQ29-AQ34  **/
		bool ARTFailureOnlyAtRegularVisit;
		int numARTInitialHVLTests;
		int numARTInitialCD4Tests;
		bool emergencyVisitIsNotRegularVisit;

		//Treatment AQ36-AQ37
		int CD4TestingLag;
		int HVLTestingLag;

		//Treatment AQ39-AQ41
		bool cd4MonitoringStopEnable;
		double cd4MonitoringStopThreshold;
		int cd4MonitoringStopMthsPostARTInit;

		/* ART policy and failure inputs */
		//Treatment D5-M48 **/
		ARTStartPolicy startART[ART_NUM_LINES];
		//Treatment K52-K61 **/
		bool enableSTIForART[ART_NUM_LINES];
		//Treatment D67-M101 **/
		ARTFailPolicy failART[ART_NUM_LINES];
		//Treatment D105-M109 **/
		ARTStopPolicy stopART[ART_NUM_LINES];
		//Treatment D114-M129 **/
		double ARTResistancePriorRegimen[ART_NUM_LINES][ART_NUM_LINES];
		double ARTResistanceHVL[HVL_NUM_STRATA];

		/* Proph policy inputs */
		//Treatment S5-AG114
		ProphStartPolicy startProph[PROPH_NUM_TYPES][OI_NUM];
		ProphStopPolicy stopProph[PROPH_NUM_TYPES][OI_NUM];
	}; /* end TreatmentInputs */

	/** LTFUInputs class contains inputs from the LTFU tab, one per simulation context */
	class LTFUInputs {
	public:
		//LTFU E4 **/
		bool useLTFU;
		//LTFU B8-C8 **/
		double propRespondLTFUPreARTLogitMean;
		double propRespondLTFUPreARTLogitStdDev;
		//LTFU G11
		bool useInterventionLTFU;
		//LTFU B13-E13 **/
		double responseThresholdLTFU[RESP_NUM_TYPES-1];
		double responseValueLTFU[RESP_NUM_TYPES-1];

		//LTFU H13-K18 **/
		double responseThresholdPeriodLTFU[HET_INTV_NUM_PERIODS][RESP_NUM_TYPES-1];
		double responseValuePeriodLTFU[HET_INTV_NUM_PERIODS][RESP_NUM_TYPES-1];
		double responseThresholdLTFUOffIntervention[RESP_NUM_TYPES-1];
		double responseValueLTFUOffIntervention[RESP_NUM_TYPES-1];

		//LTFU B17-E17 **/
		double propGeneralMedicineCost[HIV_CARE_NUM];
		//LTFU B22-E22 **/
		double propInterventionCost[HIV_CARE_NUM];
		//LTFU F33-F37 **/
		double probRemainOnOIProph;
		double probRemainOnOITreatment;
		//LTFU Q4-Q13 **/
		int minMonthsRemainLost;
		double regressionCoefficientsRTC[RTC_NUM_COEFF];
		double CD4ThresholdRTC;
		//LTFU W8-W22 **/
		bool severeOIsRTC[OI_NUM];
		//LTFU U30-U32 **/
		int maxMonthsAfterObservedFailureToRestartRegimen;
		double probRestartRegimenWithoutObservedFailure;
		bool recheckARTStartPoliciesAtRTC;
		//LTFU Q37-Z44 **/
		bool useProbSuppByPrevOutcome;
		double probSuppressionWhenReturnToFailed[ART_NUM_LINES];
		double probSuppressionWhenReturnToSuppressed[ART_NUM_LINES];
		//LTFU T48-T49 **/
		double probResumeInterventionRTC;
		double costResumeInterventionRTC;

	};

	/* HeterogeneityInputs class contains inputs from the Heterogeneity tab,
	 * 	one per simulation context */
	class HeterogeneityInputs {
	public:
		//Hetero D10-E10
		double propRespondBaselineLogitMean;
		double propRespondBaselineLogitStdDev;
		//Hetero D13-J13
		double propRespondAge[RESP_AGE_CAT_NUM];
		//Hetero H19-I19
		double propRespondAgeEarly;
		double propRespondAgeLate;
		//Hetero D16-I16
		double propRespondCD4[CD4_NUM_STRATA];
		//Hetero D19-D26
		double propRespondFemale;
		double propRespondHistoryOIs;
		double propRespondPriorARTToxicity;
		double propRespondRiskFactor[RISK_FACT_NUM];
		//Hetero C34-K38
		bool useIntervention[HET_INTV_NUM_PERIODS];
		double interventionDurationMean[HET_INTV_NUM_PERIODS];
		double interventionDurationSD[HET_INTV_NUM_PERIODS];
		double interventionAdjustmentMean[HET_INTV_NUM_PERIODS];
		double interventionAdjustmentSD[HET_INTV_NUM_PERIODS];
		SimContext::HET_ART_LOGIT_DISTRIBUTION interventionAdjustmentDistribution[HET_INTV_NUM_PERIODS];
		double interventionCostInit[HET_INTV_NUM_PERIODS];
		double interventionCostMonthly[HET_INTV_NUM_PERIODS];

	};

	/* STIInputs class contains inputs from the STI tab,
		one per simulation context */
	class STIInputs {
	public:
		/* STI initiation policy class, same inputs as ART start policy plus months since init ART */
		class InitiationPolicy : public TreatmentInputs::ARTStartPolicy {
		public:
			int monthsSinceARTStart;
		};
		/* STI endpoint policy class, same inputs as ART start policy plus months since init STI */
		class EndpointPolicy : public TreatmentInputs::ARTStartPolicy {
		public:
			int monthsSinceSTIStart;
		};

		/* STI initiation, restart, and subsequent stopping policy inputs */
		//STI D5-M50
		InitiationPolicy firstInterruption[ART_NUM_LINES];
		//STI D56-M59
		double ARTRestartCD4Bounds[ART_NUM_LINES][NUM_BOUNDS];
		int ARTRestartHVLBounds[ART_NUM_LINES][NUM_BOUNDS];
		//STI D62-F62
		int ARTRestartFirstTestMonth;
		int ARTRestartSecondTestMonth;
		int ARTRestartTestInterval;
		//STI D67-M70
		double ARTRestopCD4Bounds[ART_NUM_LINES][NUM_BOUNDS];
		int ARTRestopHVLBounds[ART_NUM_LINES][NUM_BOUNDS];
		//STI D73-F73
		int ARTRestopFirstTestMonth;
		int ARTRestopSecondTestMonth;
		int ARTRestopTestInterval;

		/* STI endpoint policy inputs */
		//STI S5-AB45
		EndpointPolicy endpoint[ART_NUM_LINES];
	}; /* end STIInputs */

	/* ProphInputs class contains inputs from the Prophs tab,
		one for each proph type x OI x proph line in the simulation context */
	class ProphInputs {
	public:
		//Prophs E5-S24
		double primaryOIEfficacy[OI_NUM];
		//Prophs E28-S47
		double secondaryOIEfficacy[OI_NUM];
		//Prophs E51-I70
		double monthlyProbResistance;
		double percentResistance;
		int timeOfResistance;
		double costFactorResistance;
		double deathRateRatioResistance;
		//Prophs E74-G93
		double probMinorToxicity;
		double probMajorToxicity;
		int monthsToToxicity;
		double deathRateRatioMajorToxicity;
		//Prophs E97-I116
		double costMonthly;
		double costMinorToxicity;
		double QOLMinorToxicity;
		double costMajorToxicity;
		double QOLMajorToxicity;
		//Prophs E120-E139
		int monthsToSwitch;
		bool switchOnMinorToxicity;
		bool switchOnMajorToxicity;
	}; /* end ProphInputs */

	/* ARTInputs class contains inputs from the ARTs tab,
		one for each ART line in the simulation context */
	class ARTInputs {
	public:
		/* ART toxicity class */
		class ARTToxicity {
		public:
			string toxicityName;
			double probToxicity;
			double timeToToxicityMean;
			double timeToToxicityStdDev;
			double QOLModifier;
			ART_TOX_DUR QOLDuration;
			double costAmount;
			ART_TOX_DUR costDuration;
			int switchARTRegimenOnToxicity; // CHRONIC ONLY
			int switchSubRegimenOnToxicity;
			int timeToChronicDeathImpact;
			double chronicToxDeathRateRatio;
			ART_TOX_DUR chronicDeathDuration;
			double acuteMajorToxDeathRateRatio;
			double costAcuteDeathMajorToxicity;
		};
		//ARTs  C4-D4
		double costInitial;
		double costMonthly;
		//ARTs C5-C6
		int efficacyTimeHorizon;
		int efficacyTimeHorizonResuppression;
		//ARTs D8
		int forceFailAtMonth;
		//ARTs D11-H29
		int stageBoundsCD4ChangeOnSuppART[2];
		int stageBoundCD4ChangeOnARTFail;
		double CD4ChangeOnSuppARTMean[CD4_RESPONSE_NUM_TYPES][3];
		double CD4ChangeOnSuppARTStdDev[CD4_RESPONSE_NUM_TYPES][3];
		double CD4MultiplierOnFailedART[CD4_RESPONSE_NUM_TYPES][2];
		double secondaryCD4ChangeOnARTStdDev;
		//ARTs C34-D35
		double monthlyCD4MultiplierOffARTPreSetpoint[ART_EFF_NUM_TYPES];
		double monthlyCD4MultiplierOffARTPostSetpoint[ART_EFF_NUM_TYPES];
		//ARTs C38-D39
		double monthlyProbHVLChange[ART_EFF_NUM_TYPES];
		int monthlyNumStrataHVLChange[ART_EFF_NUM_TYPES];
		//ARTs  C43-H207
		ARTToxicity toxicity[ART_NUM_SUBREGIMENS][ART_NUM_TOX_SEVERITY][ART_NUM_TOX_PER_SEVERITY];
		int monthsToSwitchSubRegimen[ART_NUM_SUBREGIMENS];

		//ARTs F212
		double propMthCostNonResponders;
		//ARTs D216-G218
		double probRestartARTRegimenAfterFailure[RESP_NUM_TYPES];
		int maxRestartAttempts;
		
		//ART C223-H224
		double propRespondARTRegimenLogitMean;
		double propRespondARTRegimenLogitStdDev;
		int propRespondARTRegimenLogitDistribution;
		bool propRespondARTRegimenUseDuration;
		double propRespondARTRegimenDurationMean;
		double propRespondARTRegimenDurationStdDev;

		//ARTs C228-D237
		double responseTypeThresholds[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1];
		//ARTs E228-F237
		double responseTypeValues[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1];
		//ARTs G228-G237
		double responseTypeExponents[HET_NUM_OUTCOMES];
		//ARTs D239
		bool applyARTEffectOnFailed;
		
		
	}; /* end ARTInputs */

	/* NatHistInputs class contains inputs from the NatHist tab,
		one per simulation context */
	class NatHistInputs {
	public:
		//NatHist C6-C11
		double HIVDeathRateRatio[CD4_NUM_STRATA];
		//NatHist D17
		double ARTDeathRateRatio;
		//NatHist D23-R35
		double monthlyOIProbOffART[CD4_NUM_STRATA][OI_NUM][HIST_NUM];
		//NatHist C40-Q45
		double monthlyOIProbOnARTMult[CD4_NUM_STRATA][OI_NUM];
		//NatHist C66-D71
		double acuteOIDeathRateRatio[CD4_NUM_STRATA];
		double acuteOIDeathRateRatioTB[CD4_NUM_STRATA];
		//NatHist I65-I67
		double severeOIHistDeathRateRatio;
		int severeOIHistEffectDuration;
		//NatHist M65-M67
		double TB_OIHistDeathRateRatio;
		int TB_OIHistEffectDuration;

		//NatHist D76-H76 
		double genericRiskDeathRateRatio[RISK_FACT_NUM];
		//NatHist D82-J93 and L82
		double monthlyCD4DeclineMean[CD4_NUM_STRATA][HVL_NUM_STRATA];
		double monthlyCD4DeclineStdDev[CD4_NUM_STRATA][HVL_NUM_STRATA];
		double monthlyCD4DeclineBtwSubject;
		//NatHist C98-D198
		double monthlyBackgroundDeathRate[GENDER_NUM][AGE_YRS];
		//NatHist G99-G101
		int backgroundMortModifierType;
		double backgroundMortModifier;
	}; /* end NatHistInputs */

	/* CHRMsInputs class contains inputs from the CHRMs tab,
		one per simulation context */
	class CHRMsInputs {
	public:
		//CHRMs P3
		bool showCHRMsOutput;

		//CHRMs P5, P7
		bool enableOrphans;
		bool showOrphansOutput;
		//CHRMS C4-H13
		int ageBounds[CHRM_NUM][CHRM_AGE_CAT_NUM-1];
		// CHRMS I4-L14
		double durationCHRMSstage[CHRM_TIME_PER_NUM-1][CHRM_NUM][2];
		bool enableCHRMSDurationSqrtTransform;

		// Cell notes are for first CHRM panel only where applicable; they repeat every 19 columns
		//CHRMs X7-AK13
		double probPrevalentCHRMsHIVneg[CHRM_NUM][GENDER_NUM][CHRM_AGE_CAT_NUM];
		double probPrevalentCHRMs[CHRM_NUM][CD4_NUM_STRATA][GENDER_NUM][CHRM_AGE_CAT_NUM];
		//CHRMs AD18-AH18
		double probPrevalentCHRMsRiskFactorLogit[CHRM_NUM][RISK_FACT_NUM];
		//CHRMs X17-Y17
		double prevalentCHRMsMonthsSinceStartMean[CHRM_NUM];
		double prevalentCHRMsMonthsSinceStartStdDev[CHRM_NUM];
		//CHRMs D87-R96
		int prevalentCHRMsMonthsSinceStartOrphans[CHRM_NUM][CHRM_ORPHANS_AGE_CAT_NUM];
		//CHRMs F99
		int incidentCHRMsMonthsSincePreviousOrphans;
		//CHRMs X31-AK37
		double probIncidentCHRMsHIVneg[CHRM_NUM][GENDER_NUM][CHRM_AGE_CAT_NUM];
		double probIncidentCHRMs[CHRM_NUM][CD4_NUM_STRATA][GENDER_NUM][CHRM_AGE_CAT_NUM];
		//CHRMs X42-AC42
		double probIncidentCHRMsOnARTMult[CHRM_NUM][CD4_NUM_STRATA];
		//CHRMs AG42-AK42
		double probIncidentCHRMsRiskFactorLogit[CHRM_NUM][RISK_FACT_NUM];
		//CHRMs D50-M59
		double probIncidentCHRMsPriorHistoryLogit[CHRM_NUM][CHRM_NUM];
		//CHRMs X62-AK62
		double CHRMsDeathRateRatio[CHRM_NUM][CHRM_TIME_PER_NUM][GENDER_NUM][CHRM_AGE_CAT_NUM];
		//CHRMs X71-AK73
		double costCHRMs[CHRM_NUM][CHRM_TIME_PER_NUM][GENDER_NUM][CHRM_AGE_CAT_NUM];
		//CHRMs X77
		double costDeathCHRMs[CHRM_NUM];
		//CHRMs X84-AK86
		double QOLModCHRMs[CHRM_NUM][CHRM_TIME_PER_NUM][GENDER_NUM][CHRM_AGE_CAT_NUM];
		//CHRMs X90
		double QOLModDeathCHRMs[CHRM_NUM];
		//CHRMs C71-C79
		double QOLModMultipleCHRMs[CHRM_NUM-1];
	};

	/* CostInputs class contains inputs from the Cost tab,
		one per simulation context */
	class CostInputs {
	public:
		//Cost D4-D9
		int costAgeBounds[COST_AGE_CAT_NUM-1];
		//Cost I7-S39, repeats for each age category
		double acuteOICostTreated[COST_AGE_CAT_NUM][ART_NUM_STATES][OI_NUM][COST_NUM_TYPES];
		double acuteOICostUntreated[COST_AGE_CAT_NUM][ART_NUM_STATES][OI_NUM][COST_NUM_TYPES];
		//Cost I45-S45, repeats for each age category
		double CD4TestCost[COST_AGE_CAT_NUM][COST_NUM_TYPES];
		double HVLTestCost[COST_AGE_CAT_NUM][COST_NUM_TYPES];
		//Cost I51-M87, repeats for each age category
		double deathCostTreated[COST_AGE_CAT_NUM][ART_NUM_STATES][DTH_NUM_CAUSES_BASIC][COST_NUM_TYPES];
		double deathCostUntreated[COST_AGE_CAT_NUM][ART_NUM_STATES][DTH_NUM_CAUSES_BASIC][COST_NUM_TYPES];
		//Cost DE6-DR65
		double generalMedicineCost[GENDER_NUM][COST_AGE_CAT_NUM][COST_NUM_TYPES];
		double routineCareCostHIVPositive[ART_NUM_STATES][CD4_NUM_STRATA][GENDER_NUM][COST_AGE_CAT_NUM][COST_NUM_TYPES];
		//Cost DE71-DT82
		double routineCareCostHIVNegative[GENDER_NUM][COST_AGE_CAT_NUM][COST_NUM_TYPES];
		double routineCareCostHIVPositiveUndetected[GENDER_NUM][COST_AGE_CAT_NUM][COST_NUM_TYPES];
		int routineCareCostHIVNegativeStopAge;
		int routineCareCostHIVPositiveUndetectedStopAge;
		//Cost I96-S101, repeats for each age category
		double clinicVisitCostRoutine[COST_AGE_CAT_NUM][GENDER_NUM][CD4_NUM_STRATA][COST_NUM_TYPES];
	}; /* end CostInputs */

	/* TBInputs class contains inputs from the TB tabs,
		one per simulation context */
	class TBInputs {
	public:
		/* TB Proph class */
		class TBProph {
		public:
			double efficacyInfectionHIVNeg[2];
			double efficacyInfectionHIVPos[CD4_NUM_STRATA][2];
			int monthsOfEfficacyInfection;
			int decayPeriodInfection;
			double efficacyActivationHIVNeg[TB_NUM_STRAINS][2];
			double efficacyActivationHIVPos[TB_NUM_STRAINS][CD4_NUM_STRATA][2];
			int monthsOfEfficacyActivation[TB_NUM_STRAINS];
			int decayPeriodActivation[TB_NUM_STRAINS];
			double efficacyReinfectionHIVNeg[TB_NUM_STRAINS][2];
			double efficacyReinfectionHIVPos[TB_NUM_STRAINS][CD4_NUM_STRATA][2];
			int monthsOfEfficacyReinfection[TB_NUM_STRAINS];
			int decayPeriodReinfection[TB_NUM_STRAINS];
			double costMonthly;
			double costMinorToxicity;
			double QOLModifierMinorToxicity;
			double costMajorToxicity;
			double QOLModifierMajorToxicity;
			
			double probMinorToxicityHIVNeg;
			double probMinorToxicityOffART;
			double probMinorToxicityOnART;	
			double probMajorToxicityHIVNeg;
			double probMajorToxicityOffART;
			double probMajorToxicityOnART;
			
			double deathRateRatioMajorToxicity;
			double probResistanceInActiveStates;
		};

		/*Class for characteristics of TB Tests*/
		class TBTest{
		public:
			//TBDiag
			double probPositiveHIVNeg[TB_NUM_STATES];
			double probPositiveHIVPos[TB_NUM_STATES][CD4_NUM_STRATA];
			double probAccept;
			double probPickup;
			int monthsToReset;
			double timeToPickupMean;
			double timeToPickupStdDev;
			double DSTMatrixObsvTrue[TB_NUM_STRAINS][TB_NUM_STRAINS];
			int DSTMonthsToResult[TB_NUM_STRAINS];
			bool DSTLinked;
			double DSTProbPickup;

			double probEmpiricAwaitingResultsSymptomaticHIVNeg;
			double probEmpiricAwaitingResultsSymptomaticHIVPos[CD4_NUM_STRATA];
			double probEmpiricAwaitingResultsAsymptomaticHIVNeg;
			double probEmpiricAwaitingResultsAsymptomaticHIVPos[CD4_NUM_STRATA];
			double probEmpiricPositiveResultSymptomaticHIVNeg;
			double probEmpiricPositiveResultSymptomaticHIVPos[CD4_NUM_STRATA];
			double probEmpiricPositiveResultAsymptomaticHIVNeg;
			double probEmpiricPositiveResultAsymptomaticHIVPos[CD4_NUM_STRATA];
			double probEmpiricTestOfferSymptomaticHIVNeg;
			double probEmpiricTestOfferSymptomaticHIVPos[CD4_NUM_STRATA];
			double probEmpiricTestOfferAsymptomaticHIVNeg;
			double probEmpiricTestOfferAsymptomaticHIVPos[CD4_NUM_STRATA];
			double probStopEmpiricSymptomatic;
			double probStopEmpiricAsymptomatic;

			double initCost;
			double QOLMod;
			double DSTCost;
		};

		/*Class for charactersitics of TB Treatments*/
		class TBTreatment{
		public:
			int stage1Duration;
			int totalDuration;
			double costInitial;
			double costMonthly[TB_TREATM_STAGE_NUM];

			double probSuccessHIVNeg[TB_NUM_STRAINS];
			double probSuccessHIVPos[TB_NUM_STRAINS][CD4_NUM_STRATA];

			double probMinorToxHIVNeg[TB_TREATM_STAGE_NUM];
			double probMajorToxHIVNeg[TB_TREATM_STAGE_NUM];
			double probMinorToxOffARTHIVPos[TB_TREATM_STAGE_NUM];
			double probMajorToxOffARTHIVPos[TB_TREATM_STAGE_NUM];
			double probMinorToxOnARTHIVPos[TB_TREATM_STAGE_NUM];
			double probMajorToxOnARTHIVPos[TB_TREATM_STAGE_NUM];
			
			double deathRateRatioMajorToxicity;

			double costMinorToxHIVNeg;
			double QOLModMinorToxHIVNeg;
			double costMajorToxHIVNeg;
			double QOLModMajorToxHIVNeg;

			double costMinorToxHIVPos;
			double QOLModMinorToxHIVPos;
			double costMajorToxHIVPos;
			double QOLModMajorToxHIVPos;

			double probObsvEarlyFail;
			double probEarlyFailObservedWithTBTest;
			double costObsvEarlyFailTBTest;
			double probSwitchEarlyFail;
			int nextTreatNumEarlyFail;
			int nextTreatNumNormalFail;

			double efficacyInfectionHIVNeg;
			double efficacyInfectionHIVPos[CD4_NUM_STRATA];
			int monthsOfEfficacyInfection;
			double efficacyActivationHIVNeg[TB_NUM_STRAINS];
			double efficacyActivationHIVPos[TB_NUM_STRAINS][CD4_NUM_STRATA];
			int monthsOfEfficacyActivation[TB_NUM_STRAINS];
			double efficacyReinfectionHIVNeg[TB_NUM_STRAINS];
			double efficacyReinfectionHIVPos[TB_NUM_STRAINS][CD4_NUM_STRATA];
			int monthsOfEfficacyReinfection[TB_NUM_STRAINS];

			double relapseMult;
			int relapseMultDuration;

		};

		/* Cohort TB initialization inputs */
		//TB E3
		bool enableTB;
		bool isIntegrated;

		//TB D8-H25
		double distributionTBStateAtEntryHIVNeg[TB_NUM_STATES];
		double distributionTBStateAtEntryHIVPos[CD4_NUM_STRATA][TB_NUM_STATES];
		double distributionTBStrainAtEntry[TB_NUM_STRAINS];
		double monthsSinceInitTreatStopMean;
		double monthsSinceInitTreatStopStdDev;

		//TB D30-H59
		double distributionTBTrackerAtEntryHIVNeg[TB_NUM_TRACKER][TB_NUM_STATES];
		double distributionTBTrackerAtEntryHIVPos[CD4_NUM_STRATA][TB_NUM_TRACKER][TB_NUM_STATES];

		/* Natural history inputs, Infection and reinfection */
		//TB M6-S13
		double probInfectionHIVNeg[TB_INFECT_NUM_AGE_CAT];
		double probInfectionHIVPos[CD4_NUM_STRATA][TB_INFECT_NUM_AGE_CAT];
		double infectionMultiplier[TB_NUM_STATES];
		//TB N17-24
		double probImmuneReactiveOnInfectionHIVNeg;
		double probImmuneReactiveOnInfectionHIVPos[CD4_NUM_STRATA];

		/* Natural history inputs, Activation */
		//TB M30-N30
		int probActivateMthThreshold;
		double probActivateHIVNeg[2];
		double probActivateHIVPos[2][CD4_NUM_STRATA];

		//TB R32-R38
		double probPulmonaryOnActivationHIVNeg;
		double probPulmonaryOnActivationHIVPos[CD4_NUM_STRATA];

		//TB N42-48
		double probSputumHiOnActivationPulmHIVNeg;
		double probSputumHiOnActivationPulmHIVPos[CD4_NUM_STRATA];

		/* Natural history inputs, Relapse */
		//TB M54-P64
		double probRelapseTtoARateMultiplier;
		double probRelapseTtoAExponent;
		int probRelapseTtoAThreshold;
		int probRelapseEffHorizon;

		double probRelapseTtoAFCD4[CD4_NUM_STRATA];
		double relapseRateMultTBTreatDefault;

		//TB S56-S62
		double probPulmonaryOnRelapseHIVNeg;
		double probPulmonaryOnRelapseHIVPos[CD4_NUM_STRATA];

		//TB T66-T72
		double probSputumHiOnRelapsePulmHIVNeg;
		double probSputumHiOnRelapsePulmHIVPos[CD4_NUM_STRATA];

		//TB P83
		int monthRelapseUnfavorableOutcome;

		//TB N88-S95
		double probTBSymptomsMonthHIVNeg[TB_NUM_STATES];
		double probTBSymptomsMonthHIVPos[CD4_NUM_STRATA][TB_NUM_STATES];

		//TB N102-O109
		double TBDeathRateRatioActivePulmHIVneg;
		double TBDeathRateRatioExtraPulmHIVneg;
		double TBDeathRateRatioActivePulmHIVPos[CD4_NUM_STRATA];
		double TBDeathRateRatioExtraPulmHIVPos[CD4_NUM_STRATA];
		//TB M115-T125
		int TBDeathRateRatioTxSuccessBounds[2];
		double TBDeathRateRatioTreatmentSuccessHIVNeg[3];
		double TBDeathRateRatioTreatmentSuccessHIVPos[3][CD4_NUM_STRATA];
		int TBDeathRateRatioTxFailureBounds[2];
		double TBDeathRateRatioTreatmentFailureHIVNeg[3];
		double TBDeathRateRatioTreatmentFailureHIVPos[3][CD4_NUM_STRATA];

		//TB M132-O138
		TB_MULT_TYPE natHistMultType;
		int natHistMultTimeBounds[2];
		double natHistMultTime[3];

		//TB M142-M143
		bool enableSelfCure;
		int selfCureTime;

		/* TB prophylaxis inputs */
		//TB AR5-AU9
		int prophOrder[TB_NUM_PROPHS];
		int prophDuration[TB_NUM_PROPHS];
		int maxRestarts[TB_NUM_PROPHS];
		//TB AR13-AW20
		bool startProphUseOrEvaluationKnownPos;
		bool startProphAllKnownPos;
		double startProphObservedCD4Bounds[NUM_BOUNDS];
		int startProphHistTBDiagKnownPos;
		int startProphHistTreatmentKnownPos;
		int startProphARTStatusKnownPos;
		int startProphImmuneReactiveKnownPos;

		bool startProphUseOrEvaluationNotKnownPos;
		bool startProphAllNotKnownPos;
		int startProphHistTBDiagNotKnownPos;
		int startProphHistTreatmentNotKnownPos;
		int startProphImmuneReactiveNotKnownPos;

		//TB AS26-AV37
		double probReceiveProphNotKnownPos;
		double probReceiveProphKnownPosOffART;
		double probReceiveProphOnART;
		double monthsLagToStartProphMean;
		double monthsLagToStartProphStdDev;
		double probDropoffProph;

		//TB AS40-AW45
		bool stopProphUseOrEvaluationKnownPos;
		double stopProphObservedCD4Bounds[NUM_BOUNDS];
		int stopProphNumMonthsKnownPos;
		bool stopProphAfterTBDiagKnownPos;
		bool stopProphMajorToxKnownPos;

		bool stopProphUseOrEvaluationNotKnownPos;
		int stopProphNumMonthsNotKnownPos;
		bool stopProphAfterTBDiagNotKnownPos;
		bool stopProphMajorToxNotKnownPos;

		//TB AR49
		bool moveToNextProphAfterTox;

		//TB AY6-BF73
		TBProph tbProphInputs[TB_NUM_PROPHS];

		/* TB LTFU inputs */
		//TB BE3-BE4
		bool useTBLTFU;
		int monthsToLongTermEffectsLTFU;

		//TB BD10-BH23 non-integrated clinic only
		int maxMonthsLTFU;
		double probLTFU[TB_TREATM_STAGE_NUM][TB_NUM_STATES];
		double probRTCHIVNeg[TB_NUM_STATES];
		double probRTCHIVPos[TB_NUM_STATES];

		//TB BG26-BG27 integrated clinic only
		bool allowTBProphStartWhileHIVLTFU;
		double probStopTBProphAtHIVLTFU;

		//TB BE33-BI36 both clinic types
		double rtcProbRestart[TB_RTC_TIME_CAT_NUM];
		double rtcProbResume[TB_RTC_TIME_CAT_NUM];
		double rtcProbRetest[TB_RTC_TIME_CAT_NUM];
		double rtcProbNext[TB_RTC_TIME_CAT_NUM];

		/* TB costs inputs*/
		double untreatedCosts[COST_NUM_TYPES];
		double treatedCostsVisit[COST_NUM_TYPES];
		double treatedCostsMed[COST_NUM_TYPES];
		int frequencyVisitCosts;
		int frequencyMedCosts;

		/*TB death costs */
		double costTBDeath[COST_NUM_TYPES];
		double costTBDeathPeds[PEDS_COST_AGE_CAT_NUM][COST_NUM_TYPES];

		/* QOL modifiers for active TB */
		double QOLModActiveTB;
		double QOLModDeathActiveTB;

		/* TB Diagnostics inputs */
		//TBDiag E3
		bool enableTBDiagnostics;
		bool allowMultipleTests;
		bool TBDiagnosticsInitPoliciesUseOrEvaluation;
		bool TBDiagnosticsInitPolicies[TB_DIAG_INIT_POLICY_NUM];
		double TBDiagnosticsInitSymptomsProb;
		double TBDiagnosticsInitCD4Bounds[2];
		int TBDiagnosticsInitMonth;
		double TBDiagnosticsInitIntervalProb;
		int TBDiagnosticsInitInterval[TB_DIAG_INIT_POLICY_INTV_NUM];
		int TBDiagnosticsInitIntervalBounds[TB_DIAG_INIT_POLICY_INTV_NUM-1];
		int TBDiagnosticsInitMinMthsPostTreat;

		//TBDiag C19-E22
		int TBDiagnosticsTestOrder[2][TB_DIAG_TEST_ORDER_NUM];
		bool TBDiagnosticsTestOrderDST[2][TB_DIAG_TEST_ORDER_NUM];

		//TBDiag C82-H83
		double TBDiagnosticsInitInTreatmentHIVNeg[TB_NUM_STATES];
		double TBDiagnosticsInitInTreatmentHIVPos[TB_NUM_STATES];

		//TBDiag F27-K37
		bool TBDiagnosticSequenceMatrix2Tests[TB_DIAG_STATUS_NUM];
		bool TBDiagnosticSequenceMatrix3Tests[TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM];
		bool TBDiagnosticSequenceMatrix4Tests[TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM];

		//TBDiag A40-K63
		bool TBDiagnosticAllowIncomplete;
		bool TBDiagnosticAllowNoDiagnosis;
		bool TBDiagnosticResultMatrix1Test[TB_DIAG_STATUS_NUM];
		bool TBDiagnosticResultMatrix2Tests[TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM];
		bool TBDiagnosticResultMatrix3Tests[TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM];
		bool TBDiagnosticResultMatrix4Tests[TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM][TB_DIAG_STATUS_NUM];

		//TBDiag F79-84
		double probLinkTBTreatmentIntegrated;
		double probLinkTBTreatmentNonIntegrated;
		bool RTCForHIVUponLinkageIntegrated;

		//TB Diag D73-74
		double probHIVDetUponLinkageIntegrated;
		double probHIVDetUponLinkageNonIntegrated;

		TBTest TBTests[TB_NUM_TESTS];

		//TBTreat C5-G14
		double TBTreatmentProbInitialLine[2][TB_NUM_STRAINS][TB_NUM_TREATMENTS];

		//TBTreat C20-F29
		double TBTreatmentProbRepeatLine[TB_NUM_TREATMENTS];
		int TBTreatmentMaxRepeats[TB_NUM_TREATMENTS];
		double TBTreatmentProbResistAfterFail[TB_NUM_TREATMENTS];
		double TBTreatmentProbResistAfterDefault[TB_NUM_TREATMENTS];
		double probEmpiricWithObservedHistMDR;
		double probEmpiricWithObservedHistXDR;
		int empiricTreatmentNum[TB_NUM_STRAINS];

		TBTreatment TBTreatments[TB_NUM_TREATMENTS];


	}; /* end TBInputs */

	/* QOLInputs class contains inputs from the QOL tab,
		one per simulation context */
	class QOLInputs {
	public:
		//QOL C6-E11 base QOL for HIV+ detected patients
		double routineCareQOL[CD4_NUM_STRATA][HIST_EXT_NUM];
		int routineCareQOLSevereOIHistDuration[CD4_NUM_STRATA];
		//QOL C16-D32 acute OI QOL
		double acuteOIQOL[OI_NUM];
		double deathBasicQOL[DTH_NUM_CAUSES_BASIC];
		//QOL C38-D138 background QOL, applied to all patients regardless of HIV status
		double backgroundQOL[GENDER_NUM][AGE_YRS];
		// QOL H18
		QOL_CALC_TYPE QOLCalculationType;
	}; /* end QOLInputs */

	/* HIVTestInputs class contains inputs from the HIVTest tab,
		one per simulation context */
	class HIVTestInputs {
	public:
		//HIVTest E3
		bool enableHIVTesting;
		//HIVTest E4
		bool HIVTestAvailable;
		//HIVTest E5
		bool enablePrEP;
		//HIVTest E6
		bool CD4TestAvailable;
		//HIVTest E7
		bool useAlternateStoppingRule;
		//HIVTest E8
		int totalCohortsWithHIVPositiveLimit;
		//HIVTest E9
		int totalCohortsLimit;
		//HIVTest E15:E34
		int HIVIncAgeBounds[AGE_CAT_HIV_INC];

		//HIVTest L5-L7
		double initialHIVDistribution[HIV_INF_NUM];
		//HIVTest P5-Q5
		double initialRiskDistribution[HIV_BEHAV_NUM];
		//HIVTest L12-M12
		double initialAcuteCD4DistributionMean;
		double initialAcuteCD4DistributionStdDev;
		//HIVTest L17-R17
		double initialAcuteHVLDistribution[CD4_NUM_STRATA][HVL_NUM_STRATA];
		//HIVTest O27-T46
		double probHIVInfection[GENDER_NUM][AGE_CAT_HIV_INC][HIV_BEHAV_NUM];
		
		// HIVTest K26-L33
		bool useHIVIncReductionMult;
		double HIVIncReducMultipliers[INC_REDUC_PERIODS_NUM];
		int HIVIncReducStageBounds[INC_REDUC_PERIODS_NUM];
		
		//HIVTest R49
		int monthsFromAcuteToChronic;
		//HIVTest R51-X52
		double CD4ChangeAtChronicHIVMean[HVL_NUM_STRATA];
		double CD4ChangeAtChronicHIVStdDev[HVL_NUM_STRATA];
		//HIVTest R58-X64
		double HVLDistributionAtChronicHIV[HVL_NUM_STRATA][HVL_NUM_STRATA];

		//HIVTest K54-K56
		double probHIVDetectionInitial[HIV_POS_NUM];
		//HIVTest K59-K73
		double probHIVDetectionWithOI[OI_NUM];
		//HIVTest K77-91
		double probLinkToCareWithOIDet[OI_NUM];
		
		//HIVTest S70-T75
		double monthCostHIVUndetected[CD4_NUM_STRATA];
		double monthQOLHIVUndetected[CD4_NUM_STRATA];
		
		//HIVTest S79-T81
		double deathCostHIVNegative;
		double HIVDeathCostHIVUndetected;
		double backgroundMortDeathCostHIVUndetected;
		double deathQOLHIVNegative;
		double HIVDeathQOLHIVUndetected;
		double backgroundMortDeathQOLHIVUndetected;

		//HIVTest AD5-AD9
		int HIVRegularTestingStartAge;
		int HIVRegularTestingStopAge;
		//HIVTest AH6-AI10
		int HIVTestingInterval[HIV_TEST_FREQ_NUM];
		double HIVTestingProbability[HIV_TEST_FREQ_NUM];
	
		//HIVTest AF15-AJ24
		double HIVTestAcceptDistribution[HIV_EXT_INF_NUM][TEST_ACCEPT_NUM];
		double HIVTestAcceptProb[HIV_EXT_INF_NUM][TEST_ACCEPT_NUM];
		
		//HIVTest AF28-AI34
		double HIVTestReturnProb[HIV_EXT_INF_NUM];
		double HIVTestPositiveProb[HIV_INF_NUM];
		double HIVTestPositiveCost[HIV_INF_NUM];
		double HIVTestNegativeCost[HIV_INF_NUM];
		double HIVTestPositiveQOLModifier[HIV_INF_NUM];
		double HIVTestNegativeQOLModifier[HIV_INF_NUM];
		//HIV test perform cost is stratified by HIV_INF in the input file but expanded via copying the HIV negative cost upon reading into the model, for consistency with the initial cost
		double HIVTestCost[HIV_EXT_INF_NUM];

		//HIVTest AF38-AJ39
		double HIVTestInitialCost[HIV_EXT_INF_NUM];
		double HIVTestNonReturnCost[HIV_EXT_INF_NUM];
		
		//HIVTest AF43-AJ48 
		double HIVBackgroundAcceptProb[HIV_EXT_INF_NUM];
		double HIVBackgroundReturnProb[HIV_EXT_INF_NUM];
		double HIVBackgroundPositiveProb[HIV_EXT_INF_NUM];
		double HIVBackgroundTestCost[HIV_EXT_INF_NUM];
		double HIVBackgroundPositiveCost[HIV_EXT_INF_NUM];
		double HIVBackgroundNegativeCost[HIV_EXT_INF_NUM];
		//HIVTest AF50
		int HIVBackgroundStartAge;
		//HIVTest AF51 
		double HIVBackgroundProbLinkage;

		//HIVTest AF55-AH55
		double HIVTestDetectionCost[HIV_POS_NUM];

		//HIVTest AO4
		int PrEPDropoutThreshold;
		//HIVTest AO5
		bool dropoutThresholdFromPrEPStart;
		
		//HIVTest AO9:AP16 
		double PrEPHIVTestAcceptProb[HIV_BEHAV_NUM];
		double PrEPInitialDistribution[HIV_BEHAV_NUM];
		bool PrEPAfterRollout[HIV_BEHAV_NUM];
		double PrEPDropoutPreThreshold[HIV_BEHAV_NUM];
		double PrEPDropoutPostThreshold[HIV_BEHAV_NUM];
		double PrEPCoverage[HIV_BEHAV_NUM];
		int PrEPRolloutDuration[HIV_BEHAV_NUM];
		double PrEPShape[HIV_BEHAV_NUM];

		//HIVTest AO30:AP32 
		double PrEPQOL[HIV_BEHAV_NUM];
		double costPrEPMonthly[HIV_BEHAV_NUM];
		double costPrEPInitial[HIV_BEHAV_NUM];
		
		//HIVTest AS88:AY107
		double PrEPIncidence[GENDER_NUM][AGE_CAT_HIV_INC][HIV_BEHAV_NUM];
		
		//HIVTEST BH5-BJ10
		double CD4TestAcceptProb[HIV_POS_NUM];
		double CD4TestReturnProb[HIV_POS_NUM];
		double CD4TestInitialCost[HIV_POS_NUM];
		double CD4TestCost[HIV_POS_NUM];
		double CD4TestNonReturnCost[HIV_POS_NUM];
		double CD4TestReturnCost[HIV_POS_NUM];
		//HIVTEST BH11-BH13
		double CD4TestStdDevPercentage;
		double CD4TestBiasMean;
		double CD4TestBiasStdDevPercentage;
		
		//HIVTEST BG16-BL16
		double CD4TestLinkageProb[CD4_NUM_STRATA];
	}; /* end HIVTestInputs */

	/* PedsInputs class contains inputs from the Peds tab,
	 * 	one per simulation context */
	class PedsInputs {
	public:
		//Peds E2
		bool enablePediatricsModel;

		//Peds C5-C6
		double initialAgeMean;
		double initialAgeStdDev;

		//Peds G10-J19
		double maternalStatusDistribution[PEDS_MATERNAL_STATUS_NUM];
		double probMotherIncidentInfection[PEDS_MATERNAL_STATUS_NUM];
		double probMaternalDeath[PEDS_MATERNAL_STATUS_NUM];

		double probMotherStatusKnownPregnancy[PEDS_MATERNAL_STATUS_NUM];
		double probMotherStatusBecomeKnown[PEDS_MATERNAL_STATUS_NUM];
		double probMotherOnARTInitial[PEDS_MATERNAL_STATUS_NUM];
		double probMotherOnSuppressedART[PEDS_MATERNAL_STATUS_NUM];
		double probMotherKnownSuppressed[PEDS_MATERNAL_STATUS_NUM];
		double probMotherKnownNotSuppressed[PEDS_MATERNAL_STATUS_NUM];
		double probMotherLowHVL[PEDS_MATERNAL_STATUS_NUM];

		//Peds G29 proportion of early Vertical Transmission of HIV (VTHIV) that is IU
		double propEarlyVTHIVIU;

		//Peds G24-J27
		double earlyVTHIVDistributionMotherOnArtSuppressed[PEDS_MATERNAL_STATUS_NUM];
		double earlyVTHIVDistributionMotherOnArtNotSuppressedLowHVL[PEDS_MATERNAL_STATUS_NUM];
		double earlyVTHIVDistributionMotherOnArtNotSuppressedHighHVL[PEDS_MATERNAL_STATUS_NUM];
		double earlyVTHIVDistributionMotherOffArt[PEDS_MATERNAL_STATUS_NUM];

		//Peds G36-J38, G40-J42
		double probVTHIVPPMotherOnARTSuppressed[PEDS_MATERNAL_STATUS_NUM];
		double probVTHIVPPMotherOnARTNotSuppressedLowHVL[PEDS_MATERNAL_STATUS_NUM];
		double probVTHIVPPMotherOnARTNotSuppressedHighHVL[PEDS_MATERNAL_STATUS_NUM];
		double probVTHIVPPMotherOffART[PEDS_MATERNAL_STATUS_NUM][PEDS_BF_NUM];

		//Peds D47-D49, G47-G49
		double initialBFDistribution[PEDS_BF_NUM];
		double initialBFStopAgeMean;
		double initialBFStopAgeStdDev;
		double initialBFStopAgeMax;

		//Peds J47-J48
		double probStopBFNotSuppressedLowHVL;
		double probStopBFNotSuppressedHighHVL;


		//Peds E53-F61
		double initialCD4PercentageIUMean;
		double initialCD4PercentageIUStdDev;
		double initialCD4PercentageIPMean;
		double initialCD4PercentageIPStdDev;
		double initialCD4PercentagePPMean[PEDS_AGE_INFANT_NUM];
		double initialCD4PercentagePPStdDev[PEDS_AGE_INFANT_NUM];
		//Peds E64-K72
		double initialHVLDistributionIU[HVL_NUM_STRATA];
		double initialHVLDistributionIP[HVL_NUM_STRATA];
		double initialHVLDistributionPP[PEDS_AGE_INFANT_NUM][HVL_NUM_STRATA];
		//Peds C76-J95
		CD4_STRATA adultCD4Strata[PEDS_AGE_EARLY_NUM][PEDS_CD4_PERC_NUM];

		//Peds F107
		bool usePPMaternalARTStatus;

		//Peds D111-F134
		double ppMaternalARTStatusProbSuppressed[PEDS_PP_MATERNAL_ART_STATUS_NUM];
		double ppMaternalARTStatusProbNotSuppressedLowHVL[PEDS_PP_MATERNAL_ART_STATUS_NUM];
		double ppMaternalARTStatusProbNotSuppressedHighHVL[PEDS_PP_MATERNAL_ART_STATUS_NUM];
		double ppMaternalARTStatusProbOffART[PEDS_PP_MATERNAL_ART_STATUS_NUM];

		//Peds J111-J134
		double ppMaternalARTStatusProbSuppressionKnown[PEDS_PP_MATERNAL_ART_STATUS_NUM];

		//Peds R5-Y36
		double monthlyCD4PercentDecline[PEDS_HIV_POS_NUM][PEDS_AGE_EARLY_NUM][PEDS_CD4_PERC_NUM];
		//Peds R40-S47
		double absoluteCD4TransitionMean[PEDS_CD4_PERC_NUM];
		double absoluteCD4TransitionStdDev[PEDS_CD4_PERC_NUM];
		//Peds R52-X58
		double setpointHVLTransition[HVL_NUM_STRATA][HVL_NUM_STRATA];
		//Peds R62-Y73
		double HIVDeathRateRatioPedsEarly[PEDS_AGE_EARLY_NUM][PEDS_CD4_PERC_NUM];
		//Peds Q76-Q81
		double HIVDeathRateRatioPedsLate[CD4_NUM_STRATA];
		//Peds AD62-AI70
		double maternalMortDeathRateRatioEarlyHIVNeg;
		double maternalMortDeathRateRatioEarlyHIVPos;
		int durationMaternalMortDeathRateRatioEarly;
		double replacementFedDeathRateRatioEarlyHIVNeg;
		double replacementFedDeathRateRatioEarlyHIVPos;
		int durationReplacementFedDeathRateRatioEarly;
		double maternalMortDeathRateRatioLateHIVNeg;
		double maternalMortDeathRateRatioLateHIVPos;
		int durationMaternalMortDeathRateRatioLate;
	
		//Peds Z76-AD86
		double genericRiskDeathRateRatio[RISK_FACT_NUM][PEDS_AGE_CHILD_NUM];
		//Peds Q92-R101
		double backgroundDeathRateEarly[GENDER_NUM][PEDS_AGE_EARLY_NUM];
		//Peds V92-W101
		double backgroundDeathRateExposedUninfectedEarly[GENDER_NUM][PEDS_AGE_EARLY_NUM];

		//Peds AB99
		bool useExposedUninfectedDefs;
		//Peds AC94-AC97
		bool exposedUninfectedDefsEarly[PEDS_EXPOSED_CONDITIONS_NUM];
		//Peds R109-AH286
		double probAcuteOIEarly[OI_NUM][PEDS_AGE_EARLY_NUM][PEDS_CD4_PERC_NUM][HIST_NUM];
		//Peds R290-AF302
		double probAcuteOILate[OI_NUM][CD4_NUM_STRATA][HIST_NUM];
		//Peds R307-Y316 
		double acuteOIDeathRateRatioPedsEarly[PEDS_CD4_PERC_NUM][PEDS_AGE_EARLY_NUM];
		//Peds R322-Y331 
		double acuteOIDeathRateRatioTBPedsEarly[PEDS_CD4_PERC_NUM][PEDS_AGE_EARLY_NUM];
		//Peds AC307-AC316
		double severeOIHistDeathRateRatioPedsEarly[PEDS_AGE_EARLY_NUM];
		//Peds AC318
		int severeOIHistEffectDurationPedsEarly;
		//Peds AC322-AC331
		double TB_OIHistDeathRateRatioPedsEarly[PEDS_AGE_EARLY_NUM];
		// Peds AC333
		int TB_OIHistEffectDurationPedsEarly;

		//Peds R338-S343
		double acuteOIDeathRateRatioPedsLate[CD4_NUM_STRATA];
		double acuteOIDeathRateRatioTBPedsLate[CD4_NUM_STRATA];
		//Peds AA337
		double severeOIHistDeathRateRatioPedsLate;
		//Peds AA339
		int severeOIHistEffectDurationPedsLate;
		//Peds AE337
		double TB_OIHistDeathRateRatioPedsLate;
		//Peds AE339
		int TB_OIHistEffectDurationPedsLate;

		//Peds BC5-BC14
		double maxCD4Percentage[PEDS_AGE_EARLY_NUM];
		//Peds BK6-BL7
		int CD4TestingIntervalPreARTEarly;
		int CD4TestingIntervalPreARTLate;
		int HVLTestingIntervalPreARTEarly;
		int HVLTestingIntervalPreARTLate;

		//Peds BC26-BH38
		int stageBoundsARTDeathRateRatioPeds[2];
		double ARTDeathRateRatio[PEDS_AGE_CHILD_NUM][3];
		
		//Peds BC42-BH51 
		int stageBoundsMonthlyOIProbOnARTMultEarly[2];
		double monthlyOIProbOnARTMultEarly[PEDS_CD4_PERC_NUM][3];
	
		//Peds BC57-BQ62
		double monthlyOIProbOnARTMultLate[CD4_NUM_STRATA][OI_NUM];

		/*Peds ART start policy class */
		class ARTStartPolicy {
		public:
			int CD4PercStageMonths[NUM_ART_START_CD4PERC_PEDS-1];
			double CD4PercBounds[NUM_ART_START_CD4PERC_PEDS][ART_NUM_LINES][NUM_BOUNDS];
			int HVLBounds[ART_NUM_LINES][NUM_BOUNDS];
			bool OIHistory[ART_NUM_LINES][OI_NUM];
			int numOIs[ART_NUM_LINES];
			int minMonthNum[ART_NUM_LINES];
			int monthsSincePrevRegimen[ART_NUM_LINES];
		};

		/*Peds ART Observed failure policy class */
		class ARTFailPolicy {
		public:
			int HVLNumIncrease;
			int HVLBounds[NUM_BOUNDS];
			bool HVLFailAtSetpoint;
			int HVLMonthsFromInit;
			double CD4PercPercentageDrop;
			bool CD4PercBelowPreARTNadir;
			double CD4PercBoundsOR[NUM_BOUNDS];
			double CD4PercBoundsAND[NUM_BOUNDS];
			int CD4PercMonthsFromInit;
			ART_FAIL_BY_OI OIsEvent[OI_NUM];
			int OIsMinNum;
			int OIsMonthsFromInit;
			int diagnoseNumTestsFail;
			bool diagnoseUseHVLTestsConfirm;
			bool diagnoseUseCD4TestsConfirm;
			int diagnoseNumTestsConfirm;
		};

		/*Peds ART stopping policy class */
		class ARTStopPolicy {
		public:
			int maxMonthsOnART;
			bool withMajorToxicty;
			bool afterFailImmediate;
			double afterFailCD4PercLowerBound;
			bool afterFailWithSevereOI;
			int afterFailMonthsFromObserved;
			int afterFailMinMonthNum;
			int afterFailMonthsFromInit;
		};
		/*Peds CO5-DA34 */
		ARTStartPolicy startART;

		/*Peds CR39-DA74 */
		ARTFailPolicy failART[ART_NUM_LINES];

		/*Peds CR79-DA87 */
		ARTStopPolicy stopART[ART_NUM_LINES];

		/* Prophylaxis policy classes */
		class ProphStartPolicy {
			public:
				double ageBounds[NUM_BOUNDS][OI_NUM];
				double currCD4PercBounds[NUM_BOUNDS][OI_NUM];
				int OIHistory[OI_NUM][OI_NUM];
				CONDITIONS_TYPE firstCondition;
				CONDITIONS_TYPE secondCondition;
				DIRECTIONS_TYPE parDirection;
		};
		class ProphStopPolicy {
			public:
				double ageLowerBound[OI_NUM];
				double currCD4PercLowerBound[OI_NUM];
				int OIHistory[OI_NUM][OI_NUM];
				int monthsOnProph[OI_NUM];
				CONDITIONS_TYPE firstCondition;
				CONDITIONS_TYPE secondCondition;
				DIRECTIONS_TYPE parDirection;
		};

		/* Proph policy inputs */
		//Peds BW5-CK121
		ProphStartPolicy startProph[PROPH_NUM_TYPES];
		ProphStopPolicy stopProph[PROPH_NUM_TYPES];
	};

	/* PedsARTInputs class contains inputs from the PedsARTs tab,
		one for each ART line in the simulation context */
	class PedsARTInputs {
	public:
		//PedsARTs G5-H10
		double costInitial[PEDS_ART_COST_AGE_CAT_NUM];
		double costMonthly[PEDS_ART_COST_AGE_CAT_NUM];

		//PedsARTs C9-C10
		int efficacyTimeHorizonEarly;
		int efficacyTimeHorizonLate;
		//PedsARTs D14, D17
		int forceFailAtMonthEarly;
		int forceFailAtMonthLate;
		//PedsARTs C20-H104
		int stageBoundsCD4PercentageChangeOnSuppARTEarly[2];
		int stageBoundCD4PercentageChangeOnARTFailEarly;
		double CD4PercentageChangeOnSuppARTMeanEarly[PEDS_AGE_EARLY_NUM][CD4_RESPONSE_NUM_TYPES][3];
		double CD4PercentageChangeOnSuppARTStdDevEarly[PEDS_AGE_EARLY_NUM][CD4_RESPONSE_NUM_TYPES][3];
		double CD4PercentageMultiplierOnFailedARTEarly[CD4_RESPONSE_NUM_TYPES][2];
		double secondaryCD4PercentageChangeOnARTStdDevEarly;
		int stageBoundsCD4ChangeOnSuppARTLate[2];
		int stageBoundCD4ChangeOnARTFailLate;
		double CD4ChangeOnSuppARTMeanLate[CD4_RESPONSE_NUM_TYPES][3];
		double CD4ChangeOnSuppARTStdDevLate[CD4_RESPONSE_NUM_TYPES][3];
		double CD4MultiplierOnFailedARTLate[CD4_RESPONSE_NUM_TYPES][2];
		double secondaryCD4ChangeOnARTStdDevLate;
		//PedsARTs C110-D117
		double monthlyCD4PercentageMultiplierOffARTPreSetpointEarly[ART_EFF_NUM_TYPES];
		double monthlyCD4PercentageMultiplierOffARTPostSetpointEarly[ART_EFF_NUM_TYPES];
		double monthlyCD4MultiplierOffARTPreSetpointLate[ART_EFF_NUM_TYPES];
		double monthlyCD4MultiplierOffARTPostSetpointLate[ART_EFF_NUM_TYPES];
		//PedsARTs  C123-D130
		double monthlyProbHVLChangeEarly[ART_EFF_NUM_TYPES];
		int monthlyNumStrataHVLChangeEarly[ART_EFF_NUM_TYPES];
		double monthlyProbHVLChangeLate[ART_EFF_NUM_TYPES];
		int monthlyNumStrataHVLChangeLate[ART_EFF_NUM_TYPES];

		//PedsARTs G138
		double propMthCostNonRespondersEarly;
		//PedsARTs D142-D144
		double probRestartARTRegimenAfterFailureEarly[RESP_NUM_TYPES];
		//PedsARTs C148
		double propRespondARTRegimenLogitMeanEarly;
		//PedsARTs D148
		double propRespondARTRegimenLogitStdDevEarly;
		//PedsARTs C151-G160
		double responseTypeThresholdsEarly[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1];
		double responseTypeValuesEarly[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1];
		double responseTypeExponentsEarly[HET_NUM_OUTCOMES];
	
		//PedsARTs D162
		bool applyARTEffectOnFailedARTEarly;
		//PedsARTs F165
		double propMthCostNonRespondersLate;
		//PedsARTs D169-D171
		double probRestartARTRegimenAfterFailureLate[RESP_NUM_TYPES];
		//PedsARTs C175
		double propRespondARTRegimenLogitMeanLate;
		//PedsARTs D175
		double propRespondARTRegimenLogitStdDevLate;
		//PedsARTs C178-G187
		double responseTypeThresholdsLate[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1];
		double responseTypeValuesLate[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1];
		double responseTypeExponentsLate[HET_NUM_OUTCOMES];
	
		//PedsARTs D189
		bool applyARTEffectOnFailedARTLate;
	}; /* end PedsARTInputs */

	/* PedsCostInputs class contains inputs from the Cost tab,
		one per simulation context */
	class PedsCostInputs {
	public:
		//Cost C7-M39
		double acuteOICostTreated[PEDS_COST_AGE_CAT_NUM][ART_NUM_STATES][OI_NUM][COST_NUM_TYPES];
		double acuteOICostUntreated[PEDS_COST_AGE_CAT_NUM][ART_NUM_STATES][OI_NUM][COST_NUM_TYPES];
		//Cost C45-M45
		double CD4TestCost[PEDS_COST_AGE_CAT_NUM][COST_NUM_TYPES];
		double HVLTestCost[PEDS_COST_AGE_CAT_NUM][COST_NUM_TYPES];
		//Cost C51-M87
		double deathCostTreated[PEDS_COST_AGE_CAT_NUM][ART_NUM_STATES][DTH_NUM_CAUSES_BASIC][COST_NUM_TYPES];
		double deathCostUntreated[PEDS_COST_AGE_CAT_NUM][ART_NUM_STATES][DTH_NUM_CAUSES_BASIC][COST_NUM_TYPES];
		//Cost S6-AF65
		double routineCareCostHIVNegative[GENDER_NUM][PEDS_COST_AGE_CAT_NUM][COST_NUM_TYPES];
		double routineCareCostHIVPositive[ART_NUM_STATES][CD4_NUM_STRATA][GENDER_NUM][PEDS_COST_AGE_CAT_NUM][COST_NUM_TYPES];
	}; /* end PedsCostInputs */

	/* EIDInputs class contains inputs from the EID tab,
		one per simulation context */
	class EIDInputs {
	public:
		/*Class for characteristics of EID HIV Tests*/
		class EIDTest{
		public:
			//EID BK4
			bool includeInEIDCosts;

			//EID BM7-BM8
			double EIDTestOfferProb;
			double EIDTestAcceptProb;

			//EID BM13-BM15
			double EIDTestCost;
			double EIDTestResultReturnCost;
			double EIDTestNegativeResultReturnCost;
			double EIDTestPositiveResultReturnCost;

			//EID BM18-BM22
			double EIDTestResultReturnProbLab;
			double EIDTestResultReturnProbPatient;
			int EIDTestReturnTimeMean;
			int EIDTestReturnTimeStdDev;

			//EID BK34-BW51
			double EIDSensitivityIU[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSensitivityIP[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSensitivityPPBeforeSRMotherInfectedPreDelivery[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSensitivityPPAfterSRMotherInfectedPreDelivery[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSensitivityPPMotherInfectedPostDelivery[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSensitivityMultiplierMaternalART[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSensitivityMultiplierInfantHIVProph[INFANT_HIV_PROPHS_NUM][EID_TEST_AGE_CATEGORY_NUM];

			//EID BM60-BW77
			double EIDSpecificityHEUBeforeSRMotherInfectedPreDelivery[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSpecificityHEUAfterSRMotherInfectedPreDelivery[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSpecificityMotherUninfected[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSpecificityMotherInfectedPostDelivery[EID_TEST_AGE_CATEGORY_NUM];
			double EIDSpecificityMultiplierInfantHIVProph[INFANT_HIV_PROPHS_NUM][EID_TEST_AGE_CATEGORY_NUM];

			//EID BK82-BM83
			int EIDFirstConfirmatoryTestAssay;
			int EIDSecondConfirmatoryTestAssay;
			int EIDFirstConfirmatoryTestDelay;
			int EIDSecondConfirmatoryTestDelay;

			//EID BL87
			double EIDProbLinkage;

			//EID BL91
			double EIDProbMaternalStatusKnownOnPosResult;
		};
		//end EID HIV Tests

        class InfantHIVProph {
        public:
        /* class for Infant HIV Prophs */
            //EID W2
            bool infantProphEnabled;

            //EID V4-V16
            int infantProphMaxAge;
            int infantProphEligibilityMaternalStatusKnown;
            int infantProphEligibilityMaternalStatusPos;
            int infantProphEligibilityMotherOnART;
            int infantProphEligibilityMaternalVSKnownDelivery;
            int infantProphEligibilityMotherSuppressedDelivery;
            int infantProphEligibilityMotherHVLHighDelivery;
            int infantProphEligibilityMaternalVSKnown;
            int infantProphEligibilityMotherSuppressed;
            int infantProphEligibilityMotherHVLHigh;
            bool infantProphEligibilityStopOnPosEID;

            //EID W21-X44
            double infantProphProb[INFANT_PROPH_AGES_NUM];
            int infantProphNegEIDMonths[INFANT_PROPH_AGES_NUM];

            //EID W49-X56 + V57-V59
            double infantProphStartupCost[INFANT_PROPH_COST_AGES_NUM];
            double infantProphDoseCost[INFANT_PROPH_COST_AGES_NUM];
            int infantProphEffHorizon;
            int infantProphDecayTime;
            double infantProphProbEff;

			//EID L117-O138
			int infantProphVTHIVPPMultAgeThreshold;
			double infantProphVTHIVPPMultMotherOnARTSuppressed[PEDS_MATERNAL_STATUS_NUM][2];
			double infantProphVTHIVPPMultMotherOnARTNotSuppressedLowHVL[PEDS_MATERNAL_STATUS_NUM][2];
			double infantProphVTHIVPPMultMotherOnARTNotSuppressedHighHVL[PEDS_MATERNAL_STATUS_NUM][2];
			double infantProphVTHIVPPMultMotherOffART[PEDS_MATERNAL_STATUS_NUM][PEDS_BF_NUM][2];

			//EID L108-M114
			double infantProphMajorToxProb;
			double infantProphMajorToxQOLMod;
			double infantProphMajorToxCost;
			double infantProphMajorToxDeathRateRatio;
			int infantProphMajorToxDeathRateRatioDuration;
			double infantProphMajorToxDeathCost;
			bool infantProphMajorToxStopOnTox;
			double infantProphMinorToxProb;
			double infantProphMinorToxQOLMod;
			double infantProphMinorToxCost;
		};
        //end Infant HIV Prophs

		//EID E3-E6
		bool enableHIVTestingEID;
		bool useAlternateStoppingRuleEID;
		int totalCohortsWithHIVPositiveLimitEID;
		int totalCohortsLimitEID;

		//EID A18-H70
		int testingAdminAssay[PEDS_MATERNAL_KNOWLEDGE_NUM][EID_NUM_ASSAYS];
		int testingAdminAge[PEDS_MATERNAL_KNOWLEDGE_NUM][EID_NUM_ASSAYS];
		double testingAdminProbPresent[PEDS_MATERNAL_KNOWLEDGE_NUM][EID_NUM_ASSAYS];
		double testingAdminProbNonMaternalCaregiver[PEDS_MATERNAL_KNOWLEDGE_NUM][EID_NUM_ASSAYS];
		bool testingAdminIsEIDVisit[PEDS_MATERNAL_KNOWLEDGE_NUM][EID_NUM_ASSAYS];
		bool testingAdminReofferTestIfMissed[PEDS_MATERNAL_KNOWLEDGE_NUM][EID_NUM_ASSAYS];

		//EID E73-74
		int ageOfSeroreversionMean;
		int ageOfSeroreversionStdDev;

		//EID E77
		double probMultMissedVisit;

		//EID E80
		double probMultNonMaternalCaregiver;

		//EID E83
		double probKnowedgePriorResult;

		//EID B88-111
		double costVisit[EID_COST_VISIT_NUM];

		//EID L4-17
		double probHIVDetectionWithOI[OI_NUM];

		//EID N21
		double probHIVDetectionWithOIConfirmedByLab;

		//EID M25
		int hivDetectionWithOIMonthsThreshold;

		//EID L29-Q30
		int hivDetectionWithOIAssayBeforeN1[PEDS_MATERNAL_KNOWLEDGE_NUM];
		int hivDetectionWithOIAssayAfterN1[PEDS_MATERNAL_KNOWLEDGE_NUM];

		InfantHIVProph infantHIVProphs[INFANT_HIV_PROPHS_NUM];
		EIDTest EIDTests[EID_NUM_TESTS];
	}; /* end EIDInputs */


	/* AdolescentInputs class contains inputs from the Adolescent tab,
		one per simulation context */
	class AdolescentInputs {
	public:
		//Adolescent E2-E8
		bool enableAdolescent;
		bool transitionToAdult;
		int ageTransitionToAdult;
		int ageTransitionFromPeds;
		//Adolescent B18-B34 
		int ageBounds[ADOLESCENT_NUM_AGES-1];

		//Adolescent K6-Z128
		double monthlyCD4DeclineMean[CD4_NUM_STRATA][HVL_NUM_STRATA][ADOLESCENT_NUM_AGES];
		double monthlyCD4DeclineStdDev[CD4_NUM_STRATA][HVL_NUM_STRATA][ADOLESCENT_NUM_AGES];
		//Adolescent AV7-BA24
		double HIVDeathRateRatio[CD4_NUM_STRATA][ADOLESCENT_NUM_AGES];
		//Adolescent BE7-BE24
		double ARTDeathRateRatio[ADOLESCENT_NUM_AGES];
		//Adolescent AW28-BA45
		double genericRiskDeathRateRatio[RISK_FACT_NUM][ADOLESCENT_NUM_AGES];
		
		//Adolescent AV49-BJ66
		double acuteOIDeathRateRatio[CD4_NUM_STRATA][ADOLESCENT_NUM_AGES];
		double acuteOIDeathRateRatioTB[CD4_NUM_STRATA][ADOLESCENT_NUM_AGES];
		//Adolescent AW71-BA90
		double severeOIHistDeathRateRatio[ADOLESCENT_NUM_AGES];
		int severeOIHistEffectDuration;
		double TB_OIHistDeathRateRatio[ADOLESCENT_NUM_AGES];
		int TB_OIHistEffectDuration;
		
		//Adolescent AE7-AR606
		double monthlyOIProbOffART[CD4_NUM_STRATA][OI_NUM][HIST_NUM][ADOLESCENT_NUM_AGES];
		double monthlyOIProbOnART[CD4_NUM_STRATA][OI_NUM][HIST_NUM][ADOLESCENT_NUM_AGES];

	}; /* end AdolescentInputs */



	/* AdolescentARTInputs class contains inputs from the AdolescentART tab,
			one for each ART line in the simulation context */
	class AdolescentARTInputs {
	public:
		//AdolescentART C6-E23
		double costInitial[ADOLESCENT_NUM_AGES];
		double costMonthly[ADOLESCENT_NUM_AGES];
		int efficacyTimeHorizon[ADOLESCENT_NUM_AGES];

		//AdolescentART H25
		int forceFailAtMonth;
		
		//AdolescentART C27-30
		int ageBounds[ADOLESCENT_NUM_ART_AGES-1];

		//AdolescentART C34-F51
		int stageBoundsCD4ChangeOnSuppART[2];
		int stageBoundCD4ChangeOnARTFail;
		double CD4ChangeOnSuppARTMean[3][ADOLESCENT_NUM_ART_AGES];
		double CD4ChangeOnSuppARTStdDev[3][ADOLESCENT_NUM_ART_AGES];
		double CD4MultiplierOnFailedART[2][ADOLESCENT_NUM_ART_AGES];

		//AdolescentART C56-C60
		double secondaryCD4ChangeOnARTStdDev[ADOLESCENT_NUM_ART_AGES];

		//AdolescentART C65-F69
		double monthlyCD4MultiplierOffARTSuppPreSetpoint[ADOLESCENT_NUM_ART_AGES];
		double monthlyCD4MultiplierOffARTSuppPostSetpoint[ADOLESCENT_NUM_ART_AGES];
		double monthlyCD4MultiplierOffARTFailPreSetpoint[ADOLESCENT_NUM_ART_AGES];
		double monthlyCD4MultiplierOffARTFailPostSetpoint[ADOLESCENT_NUM_ART_AGES];

		//AdolescentART C75-F79
		double monthlyProbHVLChangeSupp[ADOLESCENT_NUM_ART_AGES];
		int monthlyNumStrataHVLChangeSupp[ADOLESCENT_NUM_ART_AGES];
		double monthlyProbHVLChangeFail[ADOLESCENT_NUM_ART_AGES];
		int monthlyNumStrataHVLChangeFail[ADOLESCENT_NUM_ART_AGES];

		//Noting cell ranges for first age category only for Heterogeneity inputs
		//AdolescentART G85
		double propMthCostNonResponders[ADOLESCENT_NUM_ART_AGES];
		//AdolescentART D89-D91
		double probRestartARTRegimenAfterFailure[RESP_NUM_TYPES][ADOLESCENT_NUM_ART_AGES];

		//AdolescentART C95-D95
		double propRespondARTRegimenLogitMean[ADOLESCENT_NUM_ART_AGES];
		double propRespondARTRegimenLogitStdDev[ADOLESCENT_NUM_ART_AGES];

		//AdolescentART C98-G107
		double responseTypeThresholds[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1][ADOLESCENT_NUM_ART_AGES];

		double responseTypeValues[HET_NUM_OUTCOMES][RESP_NUM_TYPES-1][ADOLESCENT_NUM_ART_AGES];

		double responseTypeExponents[HET_NUM_OUTCOMES][ADOLESCENT_NUM_ART_AGES];
		//AdolescentART D109
		bool applyARTEffectOnFailed[ADOLESCENT_NUM_ART_AGES];

	}; /* end AdolescentARTInputs */




	/* readInputs function reads in all the inputs from the given input file,
		throws exception if there is an error */
	void readInputs();
	/* enablePrEP turns on/off PrEP in the inputs */
	inline void enablePrEP(bool enable) {
		testingInputs.enablePrEP = enable;
	}

	/* DisableDynamicTransmInc turns off incidence recording for dynamic transmissions after warmup period */
	inline void disableDynamicTransmInc() {
		cohortInputs.updateDynamicTransmInc = false;
	}


	/* public accessor functions that return const pointers to the input data classes */
	const RunSpecsInputs *getRunSpecsInputs();
	const OutputInputs *getOutputInputs();
	const CohortInputs *getCohortInputs();
	const TreatmentInputs *getTreatmentInputs();
	const LTFUInputs *getLTFUInputs();
	const HeterogeneityInputs *getHeterogeneityInputs();
	const STIInputs *getSTIInputs();
	const ProphInputs *getProphInputs(int prophType, int OINum, int prophNum);
	const ProphInputs *getPedsProphInputs(int prophType, int OINum, int prophNum);
	const ARTInputs *getARTInputs(int artLineNum);
	const NatHistInputs *getNatHistInputs();
	const CHRMsInputs *getCHRMsInputs();
	const CostInputs *getCostInputs();
	const TBInputs *getTBInputs();
	const QOLInputs *getQOLInputs();
	const HIVTestInputs *getHIVTestInputs();
	const PedsInputs *getPedsInputs();
	const PedsCostInputs *getPedsCostInputs();
	const PedsARTInputs *getPedsARTInputs(int artLineNum);
	const EIDInputs *getEIDInputs();
	const AdolescentInputs *getAdolescentInputs();
	const AdolescentARTInputs *getAdolescentARTInputs(int artLineNum);


private:
	/* Input file name and file pointer */
	string inputFileName;
	FILE *inputFile;

	/* Classes for storing the input data */
	RunSpecsInputs runSpecsInputs;
	OutputInputs outputInputs;
	CohortInputs cohortInputs;
	TreatmentInputs treatmentInputs;
	LTFUInputs ltfuInputs;
	HeterogeneityInputs heterogeneityInputs;
	STIInputs stiInputs;
	ProphInputs *prophsInputs[PROPH_NUM_TYPES][OI_NUM][PROPH_NUM];
	ProphInputs *pedsProphsInputs[PROPH_NUM_TYPES][OI_NUM][PROPH_NUM];
	ARTInputs *artInputs[ART_NUM_LINES];
	NatHistInputs natHistInputs;
	CHRMsInputs chrmsInputs;
	CostInputs costInputs;
	TBInputs tbInputs;
	QOLInputs qolInputs;
	HIVTestInputs testingInputs;
	PedsInputs pedsInputs;
	PedsARTInputs *pedsARTInputs[ART_NUM_LINES];
	PedsCostInputs pedsCostInputs;
	EIDInputs eidInputs;
	AdolescentInputs adolescentInputs;
	AdolescentARTInputs *adolescentARTInputs[ART_NUM_LINES];

	/* Private functions for reading in the inputs, called by readInputs */
	void readRunSpecsInputs();
	void readOutputInputs();
	void readCohortInputs();
	void readTreatmentInputsPart1();
	void readTreatmentInputsPart2();
	void readLTFUInputs();
	void readHeterogeneityInputs();
	void readSTIInputs();
	void readProphInputs();
	void readPedsProphInputs();
	void readARTInputs();
	void readNatHistInputs();
	void readCHRMsInputs();
	void readCostInputs();
	void readTBInputs();
	void readQOLInputs();
	void readHIVTestInputs();
	void readPedsInputs();
	void readPedsARTInputs();
	void readPedsCostInputs();
	void readEIDInputs();
	void readAdolescentInputs();
	void readAdolescentARTInputs();
	bool readAndSkipPast(const char* searchStr, FILE* file);
	bool readAndSkipPast2(const char* searchStr1, const char *searchStr2, FILE *file);

};

/* getRunSpecsInputs returns a const pointer to the RunSpecsInputs data class */
inline const SimContext::RunSpecsInputs *SimContext::getRunSpecsInputs() {
	return &runSpecsInputs;
}

/* getOutputInputs returns a const pointer to the OutputInputs data class */
inline const SimContext::OutputInputs *SimContext::getOutputInputs() {
	return &outputInputs;
}

/* getCohortInputs returns a const pointer to the CohortInputs data class */
inline const SimContext::CohortInputs *SimContext::getCohortInputs() {
	return &cohortInputs;
}

/* getTreatmentInputs returns a const pointer to the TreatmentInputs data class */
inline const SimContext::TreatmentInputs *SimContext::getTreatmentInputs() {
	return &treatmentInputs;
}

/* getLTFUInputs returns a const pointer to the LTFUInputs data class */
inline const SimContext::LTFUInputs *SimContext::getLTFUInputs() {
	return &ltfuInputs;
}

/* getHeterogeneityInputs returns a const pointer to the HeterogeneityInputs data class */
inline const SimContext::HeterogeneityInputs *SimContext::getHeterogeneityInputs() {
	return &heterogeneityInputs;
}

/* getSTIInputs returns a const pointer to the STIInputs data class */
inline const SimContext::STIInputs *SimContext::getSTIInputs() {
	return &stiInputs;
}

/* getProphInputs returns a const pointer to the specified ProphInputs data class */
inline const SimContext::ProphInputs *SimContext::getProphInputs(int prophType, int OINum, int prophNum) {
	assert(prophType < PROPH_NUM_TYPES);
	assert(OINum < OI_NUM);
	assert(prophNum < PROPH_NUM);

	return prophsInputs[prophType][OINum][prophNum];
}

/* getPedsProphInputs returns a const pointer to the specified ProphInputs data class */
inline const SimContext::ProphInputs *SimContext::getPedsProphInputs(int prophType, int OINum, int prophNum) {
	assert(prophType < PROPH_NUM_TYPES);
	assert(OINum < OI_NUM);
	assert(prophNum < PROPH_NUM);

	return pedsProphsInputs[prophType][OINum][prophNum];
}

/* getARTInputs returns a const pointer to the specified ARTInputs data class */
inline const SimContext::ARTInputs *SimContext::getARTInputs(int artLineNum) {

	assert(artLineNum < ART_NUM_LINES);
	return artInputs[artLineNum];
}

/* getNatHistInputs returns a const pointer to the NatHistInputs data class */
inline const SimContext::NatHistInputs *SimContext::getNatHistInputs() {
	return &natHistInputs;
}

/* getCHRMsInputs returns a const pointer to the CHRMsInputs data class */
inline const SimContext::CHRMsInputs *SimContext::getCHRMsInputs() {
	return &chrmsInputs;
}

/* getCostInputs returns a const pointer to the CostInputs data class */
inline const SimContext::CostInputs *SimContext::getCostInputs() {
	return &costInputs;
}

/* getTBInputs returns a const pointer to the TBInputs data class */
inline const SimContext::TBInputs *SimContext::getTBInputs() {
	return &tbInputs;
}

/* getQOLInputs returns a const pointer to the QOLInputs data class */
inline const SimContext::QOLInputs *SimContext::getQOLInputs() {
	return &qolInputs;
}

/* getHIVTestInputs returns a const pointer to the HIVTestInputs data class */
inline const SimContext::HIVTestInputs *SimContext::getHIVTestInputs() {
	return &testingInputs;
}

/* getPedsInputs returns a const pointer to the PedsInputs data class */
inline const SimContext::PedsInputs *SimContext::getPedsInputs() {
	return &pedsInputs;
}

/* getPedsARTInputs returns a const pointer to the specified PedsARTInputs data class */
inline const SimContext::PedsARTInputs *SimContext::getPedsARTInputs(int artLineNum) {

	assert(artLineNum < ART_NUM_LINES);
	return pedsARTInputs[artLineNum];
}

/* getCostInputs returns a const pointer to the CostInputs data class */
inline const SimContext::PedsCostInputs *SimContext::getPedsCostInputs() {
	return &pedsCostInputs;
}

/* getEIDInputs returns a const pointer to the EIDInputs data class */
inline const SimContext::EIDInputs *SimContext::getEIDInputs() {
	return &eidInputs;
}

/* getAdolescentInputs returns a const pointer to the AdolescentInputs data class */
inline const SimContext::AdolescentInputs *SimContext::getAdolescentInputs() {
	return &adolescentInputs;
}

/* getAdolescentARTInputs returns a const pointer to the specified getAdolescentARTInputs data class */
inline const SimContext::AdolescentARTInputs *SimContext::getAdolescentARTInputs(int artLineNum) {

	assert(artLineNum < ART_NUM_LINES);
	return adolescentARTInputs[artLineNum];
}
