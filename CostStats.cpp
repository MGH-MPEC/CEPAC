#include "include.h"

/** \brief Constructor takes run name and associated simulation context pointer as parameters
 *
 * \param runName a string representing the run name associated with this costStats object
 * \param simContext a pointer to the SimContext representing the inputs associated with this CostSTats object*/
CostStats::CostStats(string runName, SimContext *simContext) {
	costStatsFileName = runName;
	costStatsFileName.append(CepacUtil::FILE_EXTENSION_FOR_COSTS_OUTPUT);
	this->simContext = simContext;

	/** Initialize the statistics subclasses */
	initCostPopulationSummary();
	initAllStats();
	initEventStats();
}

/** \brief Destructor */
CostStats::~CostStats(void) {
} /* end Destructor */

/** \brief finalizeStats calculate all aggregate statistics and values to be outputted
 *
 * The finalize functions calculate all averages and standard deviations.
 *
 * Calls:
 * - CostStats::finalizeCostPopulationSummary();
*/
void CostStats::finalizeStats() {
	finalizeCostPopulationSummary();
	finalizeAllStats();
	finalizeEventStats();
}; /* end finalizeStats */

/** \brief writeStatsFile outputs all statistics to the stats file
 *
 *  * Calls:
 * - CostStats::writeCostPopulationSummary();
*/
void CostStats::writeStatsFile() {
	CepacUtil::changeDirectoryToResults();
	costStatsFile = CepacUtil::openFile(costStatsFileName.c_str(), "w");
	if (costStatsFile == NULL) {
		costStatsFileName.append("-tmp");
		costStatsFile = CepacUtil::openFile(costStatsFileName.c_str(), "w");
		if (costStatsFile == NULL) {
			string errorString = "   ERROR - Could not write cost stats or temporary stats file";
			throw errorString;
		}
	}

	writeCostPopulationSummary();
	writeAllStats(SimContext::COST_REPORT_DISCOUNTED);
	writeAllStats(SimContext::COST_REPORT_UNDISCOUNTED);
	writeEventStats();

	CepacUtil::closeFile(costStatsFile);
} /* end writeStatsFile */

/** \brief initCostPopulationSummary initializes the CostPopulationSummary object */
void CostStats::initCostPopulationSummary() {
	popSummary.numPatients = 0;
	popSummary.numPatientsHIVPositive = 0;
	popSummary.numDetected = 0;
	popSummary.numInCare = 0;
	for (int i = 0; i < SimContext::ART_NUM_LINES; i++){
		popSummary.numEverOnART[i] = 0;
	}
	for (int i = 0; i < SimContext::CD4_NUM_STRATA; i++){
		popSummary.observedCD4DistributionAtLinkage[i] = 0;
		popSummary.observedCD4DistributionAtARTStart[i] = 0;
	}
	for (int i = 0; i < SimContext::GENDER_NUM; i++){
		popSummary.genderDistributionAtLinkage[i] = 0;
		popSummary.genderDistributionAtARTStart[i] = 0;
	}
	for (int i = 0; i < SimContext::LINKAGE_STATS_AGE_CAT_NUM; i++){
		popSummary.ageDistributionAtLinkage[i] = 0;
		popSummary.ageDistributionAtARTStart[i] = 0;
	}
}

/** \brief initAllStats initializes the AllStats object */
void CostStats::initAllStats() {
	for (int i = 0; i < SimContext::COST_REPORT_DISCOUNT_NUM; i++){
		for (int j = 0; j < SimContext::COST_CD4_STRATA_NUM; j++){
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++){
				allStats[i][j][k].LMs = 0.0;
				allStats[i][j][k].costs = 0.0;
				allStats[i][j][k].costsART = 0.0;
				allStats[i][j][k].costsOIProph = 0.0;
				allStats[i][j][k].costsTBProph = 0.0;
				allStats[i][j][k].costsCD4Testing = 0.0;
				allStats[i][j][k].costsHVLTesting = 0.0;
				allStats[i][j][k].costsInterventionStartup = 0.0;
				allStats[i][j][k].costsInterventionMonthly = 0.0;
				allStats[i][j][k].costsClinicVisit = 0.0;
				allStats[i][j][k].costsRoutineCare = 0.0;
				allStats[i][j][k].costsGeneralMedicine = 0.0;
				allStats[i][j][k].costsOITreatment = 0.0;
				allStats[i][j][k].costsOIUntreated = 0.0;
				allStats[i][j][k].costsToxicity = 0.0;
				allStats[i][j][k].costsDeath = 0.0;
				allStats[i][j][k].costsHIVScreeningTests = 0.0;
				allStats[i][j][k].costsHIVScreeningMisc = 0.0;
				allStats[i][j][k].costsLabStagingTests = 0.0;
				allStats[i][j][k].costsLabStagingMisc = 0.0;
				allStats[i][j][k].costsTBTreatment = 0.0;
				for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
					allStats[i][j][k].costsCD4TestingCategory[l] = 0.0;
					allStats[i][j][k].costsHVLTestingCategory[l] = 0.0;
					allStats[i][j][k].costsClinicVisitCategory[l] = 0.0;
					allStats[i][j][k].costsRoutineCareCategory[l] = 0.0;
					allStats[i][j][k].costsGeneralMedicineCategory[l] = 0.0;
					allStats[i][j][k].costsOITreatmentCategory[l] = 0.0;
					allStats[i][j][k].costsOIUntreatedCategory[l] = 0.0;
					allStats[i][j][k].costsDeathCategory[l] = 0.0;
				}
			}
		}
	}
}


/** \brief initEventStats initializes the EventStats object */
void CostStats::initEventStats() {
	for (int j = 0; j < SimContext::COST_CD4_STRATA_NUM; j++){
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++){
			eventStats[j][k].numToxicityCases = 0;
			eventStats[j][k].numTBInfections = 0;
			eventStats[j][k].numOIEvents = 0;
			eventStats[j][k].numDeaths = 0;
			eventStats[j][k].numCD4Tests = 0;
			eventStats[j][k].numHVLTests = 0;
			eventStats[j][k].numClinicVisits = 0;
			eventStats[j][k].numHIVTests = 0;
			eventStats[j][k].numLabStagingTests = 0;
		}
	}
}

/** \brief finalizeCostPopulationSummary calculates aggregate statistics for the CostPopulationSummary object */
void CostStats::finalizeCostPopulationSummary() {

}

/** \brief finalizeAllStats calculates aggregate statistics for the Allstats object */
void CostStats::finalizeAllStats() {

}

/** \brief finalizeEventStats calculates aggregate statistics for the Eventstats object */
void CostStats::finalizeEventStats() {

}

/** \brief writeCostPopulationSummary outputs the CostPopulationSummary statistics to the  cost stats file */
void CostStats::writeCostPopulationSummary() {
	int i,j;

	// Print out the section header
	fprintf(costStatsFile, "CEPAC COST SUMMARY");

	// General Population statistics
	fprintf (costStatsFile, "\n\t\tTotal\tEver HIV+\tEver Detected\tEver in Care\tEver on 1st line ART\tEver on 2nd line ART");
	fprintf (costStatsFile, "\n\tNum Patients\t%d\t%d\t%d\t%d\t%d\t%d", popSummary.numPatients,
			popSummary.numPatientsHIVPositive,
			popSummary.numDetected,
			popSummary.numInCare,
			popSummary.numEverOnART[0],
			popSummary.numEverOnART[1]);

	// Observed CD4 dist
	fprintf (costStatsFile, "\n\tObserved CD4 Distribution");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(costStatsFile, "\t%s", SimContext::CD4_STRATA_STRS[j]);
	fprintf (costStatsFile, "\n\tPatients in Linkage Month");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(costStatsFile, "\t%d", popSummary.observedCD4DistributionAtLinkage[j]);
	fprintf (costStatsFile, "\n\tPatients in ART Initiation Month");
	for (j = SimContext::CD4_NUM_STRATA - 1; j >= 0; --j)
		fprintf(costStatsFile, "\t%d", popSummary.observedCD4DistributionAtARTStart[j]);

	// gender dist
	fprintf (costStatsFile, "\n\tGender Distribution");
	for (j = 0; j < SimContext::GENDER_NUM; j++)
		fprintf(costStatsFile, "\t%s", SimContext::GENDER_STRS[j]);
	fprintf (costStatsFile, "\n\tPatients in Linkage Month");
	for (j = 0; j < SimContext::GENDER_NUM; j++)
		fprintf(costStatsFile, "\t%d", popSummary.genderDistributionAtLinkage[j]);
	fprintf (costStatsFile, "\n\tPatients in ART Initiation Month");
	for (j = 0; j < SimContext::GENDER_NUM; j++)
		fprintf(costStatsFile, "\t%d", popSummary.genderDistributionAtARTStart[j]);

	//Age dist
	fprintf (costStatsFile, "\n\tAge Distribution");
	for (j = 0; j < SimContext::LINKAGE_STATS_AGE_CAT_NUM; j++)
		fprintf(costStatsFile, "\t%s", SimContext::LINKAGE_STATS_AGE_CAT_STRS[j]);
	fprintf (costStatsFile, "\n\tPatients in Linkage Month");
	for (j = 0; j < SimContext::LINKAGE_STATS_AGE_CAT_NUM; j++)
		fprintf(costStatsFile, "\t%d", popSummary.ageDistributionAtLinkage[j]);
	fprintf (costStatsFile, "\n\tPatients in ART Initiation Month");
	for (j = 0; j < SimContext::LINKAGE_STATS_AGE_CAT_NUM; j++)
		fprintf(costStatsFile, "\t%d", popSummary.ageDistributionAtARTStart[j]);
}

/** \brief writeAllstats outputs the AllStats statistics to the  cost stats file */
void CostStats::writeAllStats(SimContext::COST_REPORT_DISCOUNT disc) {
	for (int j = SimContext::COST_CD4_STRATA_NUM-1; j >= 0; j--){
		if (disc == SimContext::COST_REPORT_DISCOUNTED)
			fprintf(costStatsFile, "\n\nDiscounted\tDiscounting Rate\t%1.2lf\tObsv CD4\t%s", simContext->getRunSpecsInputs()->originalDiscRate, SimContext::COST_CD4_STRATA_STRS[j]);
		else
			fprintf(costStatsFile, "\n\nUndiscounted\tObsv CD4\t%s", SimContext::COST_CD4_STRATA_STRS[j]);

		fprintf(costStatsFile, "\n\t");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%s", SimContext::COST_SUBGROUPS_STRS[k]);

		//total patient months
		fprintf(costStatsFile, "\n\tTotal Patient Mths");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].LMs);

		//total costs
		fprintf(costStatsFile, "\n\tTotal Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costs);

		//ART Costs
		fprintf(costStatsFile, "\n\tART Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsART);

		//OI proph costs
		fprintf(costStatsFile, "\n\tOI Prophylaxis Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsOIProph);

		//TB proph costs
		fprintf(costStatsFile, "\n\tTB Prophylaxis Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsTBProph);

		//Intervention costs
		fprintf(costStatsFile, "\n\tIntervention Startup Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsInterventionStartup);

		fprintf(costStatsFile, "\n\tIntervention Monthly Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsInterventionMonthly);

		//CD4 Test costs
		fprintf(costStatsFile, "\n\tCD4 Test Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsCD4Testing);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tCD4 Test Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsCD4TestingCategory[l]);
		}

		//HVL Test costs
		fprintf(costStatsFile, "\n\tHVL Test Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsHVLTesting);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tHVL Test Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsHVLTestingCategory[l]);
		}

		//Clinic Visit costs
		fprintf(costStatsFile, "\n\tClinic Visit Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsClinicVisit);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tClinic Visit Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsClinicVisitCategory[l]);
		}

		//Routine care costs
		fprintf(costStatsFile, "\n\tRoutine Care Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsRoutineCare);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tRoutine Care Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsRoutineCareCategory[l]);
		}

		//General Medicine
		fprintf(costStatsFile, "\n\tGeneral Medicine Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsGeneralMedicine);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tGeneral Medicine Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsGeneralMedicineCategory[l]);
		}

		//Acute OI Treatment
		fprintf(costStatsFile, "\n\tOI Treatment Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsOITreatment);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tOI Treatment Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsOITreatmentCategory[l]);
		}

		fprintf(costStatsFile, "\n\tUntreated Acute OI Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsOIUntreated);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tUntreated Acute OI Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsOIUntreatedCategory[l]);
		}


		//Toxicity Costs
		fprintf(costStatsFile, "\n\tToxicity Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsToxicity);

		//Death Costs
		fprintf(costStatsFile, "\n\tDeath Costs");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsDeath);

		for (int l = 0; l < SimContext::COST_NUM_TYPES; l++){
			fprintf(costStatsFile, "\n\tDeath Costs %s", SimContext::COST_TYPES_STRS[l]);
			for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
				fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsDeathCategory[l]);
		}

		//Screening Costs
		fprintf(costStatsFile, "\n\tHIV Screening Tests Cost");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsHIVScreeningTests);
		fprintf(costStatsFile, "\n\tHIV Screening Misc Cost");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsHIVScreeningMisc);

		//Lab Staging Costs
		fprintf(costStatsFile, "\n\tLab Staging Tests Cost");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsLabStagingTests);
		fprintf(costStatsFile, "\n\tLab Staging Misc Cost");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsLabStagingMisc);

		//TB Treatment Costs
		fprintf(costStatsFile, "\n\tTB Treatment Cost");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%1.2lf", allStats[disc][j][k].costsTBTreatment);


	}
}/* End writeAllStats */

/** \brief writeEventstats outputs the EventStats statistics to the  cost stats file */
void CostStats::writeEventStats() {
	for (int j = 0; j < SimContext::COST_CD4_STRATA_NUM; j++){
		fprintf(costStatsFile, "\n\n\tObsv CD4\t%s", SimContext::COST_CD4_STRATA_STRS[j]);
		fprintf(costStatsFile, "\n\t");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%s", SimContext::COST_SUBGROUPS_STRS[k]);

		//Toxicity Cases
		fprintf(costStatsFile, "\n\tToxicity Event Quantity");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numToxicityCases);

		//Tb Events
		fprintf(costStatsFile, "\n\tTB Event Quantity");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numTBInfections);

		//OI Events
		fprintf(costStatsFile, "\n\tOI Event Quantity");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numOIEvents);

		//Death Events
		fprintf(costStatsFile, "\n\tDeaths");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numDeaths);

		//Num CD4 Tests
		fprintf(costStatsFile, "\n\tNum CD4 Tests");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numCD4Tests);

		//Num HVL Tests
		fprintf(costStatsFile, "\n\tNum HVL Tests");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numHVLTests);

		//Num Clinic Visits
		fprintf(costStatsFile, "\n\tNum Clinic Visits");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numClinicVisits);

		//Num HIV Tests
		fprintf(costStatsFile, "\n\tNum HIV Tests");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numHIVTests);

		//Num Lab Staging Tests
		fprintf(costStatsFile, "\n\tNum Lab Staging Tests");
		for (int k = 0; k < SimContext::COST_SUBGROUPS_NUM; k++)
			fprintf(costStatsFile, "\t%d", eventStats[j][k].numLabStagingTests);
	}
}


