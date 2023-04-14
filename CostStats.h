#pragma once

#include "include.h"

/**
	CostStats class contains the aggregate statistics related to costs for a given
	input file (simulation context).  Contains all the functions to update these statistics
	and generate the costs output files.  Its statistics are updated by the StateUpdater objects
	that are called every month for each patient.  Read only access to the
	statistics is provided through accessor functions that return const pointers to the
	subclass objects.
*/
class CostStats
{
public:
	/** Make the StateUpdater class a friend class so it can modify the private data */
	friend class StateUpdater;

	/* Constructors and Destructor */
	CostStats(string runName, SimContext *simContext);
	~CostStats(void);

	/** CostPopulationSummary holds summary aggregate statistics and other misc information */
	class CostPopulationSummary {
	public:
		/** Total number of patients in this cohort */
		int numPatients;
		/** Total number ever HIV Positive */
		int numPatientsHIVPositive;
		/** Total number ever detected HIV Positive */
		int numDetected;
		/** Total number ever in care */
		int numInCare;
		/** Num ever on art by art line */
		int numEverOnART[SimContext::ART_NUM_LINES];
		/** Observed CD4 distribution when entering care */
		int observedCD4DistributionAtLinkage[SimContext::CD4_NUM_STRATA];
		/** gender distribtion when entering care */
		int genderDistributionAtLinkage[SimContext::GENDER_NUM];
		/** age distribution when entering care */
		int ageDistributionAtLinkage[SimContext::LINKAGE_STATS_AGE_CAT_NUM];
		/** Observed CD4 distribution when starting ART (first time ART) */
		int observedCD4DistributionAtARTStart[SimContext::CD4_NUM_STRATA];
		/** gender distribtion when starting ART (first time ART) */
		int genderDistributionAtARTStart[SimContext::GENDER_NUM];
		/** age distribution when starting ART (first time ART) */
		int ageDistributionAtARTStart[SimContext::LINKAGE_STATS_AGE_CAT_NUM];
	};/* end CostPopulationSummary */

	/** class for stats for entire population can be discounted/undisc can by any cd4 strata*/
	class AllStats {
	public:
		//Life months lived by patient
		double LMs;
		//Overall costs accrued by patient
		double costs;
		//ART costs
		double costsART;
		//OI Prohylaxis costs
		double costsOIProph;
		//TB Prophylaxis costs
		double costsTBProph;
		//CD4 HVL test costs
		double costsCD4Testing;
		double costsHVLTesting;
		double costsCD4TestingCategory[SimContext::COST_NUM_TYPES];
		double costsHVLTestingCategory[SimContext::COST_NUM_TYPES];
		//intervention costs
		double costsInterventionStartup;
		double costsInterventionMonthly;

		//Visit costs
		double costsClinicVisit;
		double costsClinicVisitCategory[SimContext::COST_NUM_TYPES];

		//routine care costs
		double costsRoutineCare;
		double costsRoutineCareCategory[SimContext::COST_NUM_TYPES];

		//general medicine costs
		double costsGeneralMedicine;
		double costsGeneralMedicineCategory[SimContext::COST_NUM_TYPES];

		//treated and untreated acute OI costs
		double costsOITreatment;
		double costsOITreatmentCategory[SimContext::COST_NUM_TYPES];
		double costsOIUntreated;
		double costsOIUntreatedCategory[SimContext::COST_NUM_TYPES];

		//Toxicity costs
		double costsToxicity;

		//death costs
		double costsDeath;
		double costsDeathCategory[SimContext::COST_NUM_TYPES];

		//screening costs
		double costsHIVScreeningTests;
		double costsHIVScreeningMisc;

		//lab staging costs
		double costsLabStagingTests;
		double costsLabStagingMisc;

		//TB treatment cost
		double costsTBTreatment;
	};

	/** class for stats for counts of events can by any cd4 strata*/
	class EventStats {
	public:
		//Toxicity Events
		int numToxicityCases;
		//TB Events
		int numTBInfections;
		//OI Events
		int numOIEvents;
		//Death Events
		int numDeaths;
		//Num tests
		int numCD4Tests;
		int numHVLTests;
		//Num clinic visits
		int numClinicVisits;
		//Num HIV Tests
		int numHIVTests;
		//Num Lab Staging Tests
		int numLabStagingTests;

	};

	/* Accessor functions returning const pointers to the statistics subclass objects */
	const CostPopulationSummary *getCostPopulationSummary();
	const AllStats *getAllStats(SimContext::COST_REPORT_DISCOUNT disc, SimContext::CD4_STRATA obsvCD4Strata, SimContext::COST_SUBGROUPS subgroup);
	const EventStats *getEventStats(SimContext::CD4_STRATA obsvCD4Strata, SimContext::COST_SUBGROUPS subgroup);

	/* Functions to calculate final aggregate statistics and to write out the stats file */
	void finalizeStats();
	void writeStatsFile();
private:
	/** Pointer to the associated simulation context */
	SimContext *simContext;
	/**Cost Stats file name */
	string costStatsFileName;
	/** Stats file pointer */
	FILE *costStatsFile;

	/** Statistics subclass object */
	CostPopulationSummary popSummary;
	/** Statistics of cohort stratified by obsv CD4, disc/undisc, and subpopulation **/
	AllStats allStats[SimContext::COST_REPORT_DISCOUNT_NUM][SimContext::COST_CD4_STRATA_NUM][SimContext::COST_SUBGROUPS_NUM];
	EventStats eventStats[SimContext::COST_CD4_STRATA_NUM][SimContext::COST_SUBGROUPS_NUM];

	/* Initialization functions for statistics objects, called by constructor */
	void initCostPopulationSummary();
	void initAllStats();
	void initEventStats();

	/* Functions to finalize aggregate statistics before printing out */
	void finalizeCostPopulationSummary();
	void finalizeAllStats();
	void finalizeEventStats();

	/* Functions to write out each subclass object to the statistics file, called by writeStatsFile */
	void writeCostPopulationSummary();
	void writeAllStats(SimContext::COST_REPORT_DISCOUNT disc);
	void writeEventStats();

};

/** \brief getPopulationSumary returns a const pointer to the CostPopulationSummary statistics object */
inline const CostStats::CostPopulationSummary *CostStats::getCostPopulationSummary() {
	return &popSummary;
}

/** \brief getAllStats returns a const pointer to the allStats statistics object */
inline const CostStats::AllStats *CostStats::getAllStats(SimContext::COST_REPORT_DISCOUNT disc, SimContext::CD4_STRATA obsvCD4Strata, SimContext::COST_SUBGROUPS subgroup) {
	return &allStats[disc][obsvCD4Strata][subgroup];
}

/** \brief getEventStats returns a const pointer to the eventStats statistics object */
inline const CostStats::EventStats *CostStats::getEventStats(SimContext::CD4_STRATA obsvCD4Strata, SimContext::COST_SUBGROUPS subgroup) {
	return &eventStats[obsvCD4Strata][subgroup];
}
