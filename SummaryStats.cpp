#include "include.h"

/** \brief Constructor takes summariesFileName as input, clears summaries vector
 *
 *	/param runName a string identifying the file name of the summaries file (most likely 'popstats.out')
 */
SummaryStats::SummaryStats(string summariesFileName) {
	this->summariesFileName = summariesFileName;
	summaries.clear();
} /* end Constructor */

/** \brief Destructor frees allocated Summary objects and clears summaries vector */
SummaryStats::~SummaryStats(void)
{
	for (list<vector<Summary *> >::iterator i = summaries.begin(); i != summaries.end(); i++) {
		vector<Summary *> &runSetVector = *i;
		for (vector<Summary *>::iterator j = runSetVector.begin(); j != runSetVector.end(); j++) {
			Summary *summary = *j;
			delete summary;
		}
		runSetVector.clear();
	}
	summaries.clear();
} /* end Destructor */

/** \brief addRunStats adds a new summary to the vector from a RunStats object
 *
 * \param runStats a pointer to the RunStats object that the new Summary object will get its information from
 **/
void SummaryStats::addRunStats(RunStats *runStats) {
	/** Create a new summary object */
	Summary *summary = new Summary();

	/** Copy the population summary stats from runStats */
	const RunStats::PopulationSummary *popSummary = runStats->getPopulationSummary();
	summary->runSetName = popSummary->runSetName;
	summary->runName = popSummary->runName;
	summary->runDate = popSummary->runDate;
	summary->runTime = popSummary->runTime;
	summary->numCohorts = popSummary->numCohorts;
	summary->costsAverage = popSummary->costsAverage;
	summary->LMsAverage = popSummary->LMsAverage;
	summary->QALMsAverage = popSummary->QALMsAverage;
	summary->costsHIVPositiveAverage = popSummary->costsHIVPositiveAverage;
	summary->LMsHIVPositiveAverage = popSummary->LMsHIVPositiveAverage;
	summary->QALMsHIVPositiveAverage = popSummary->QALMsHIVPositiveAverage;
	if (popSummary->numCohortsHIVPositive > 0)
		summary->numClinicVisitsPer1000 = 1000.0 * popSummary->totalClinicVisits / popSummary->numCohortsHIVPositive;

	/** Copy the HIV screening stats */
	const RunStats::HIVScreening *hivScreening = runStats->getHIVScreening();
	summary->monthsAfterInfectionToDetectionAverage =  hivScreening->monthsAfterInfectionToDetectionAverage;
	summary->monthsToDetectionPrevalentAverage = hivScreening->monthsToDetectionPrevalentAverage;
	summary->CD4AtDetectionIncidentAverage = hivScreening->CD4AtDetectionIncidentAverage;
	summary->CD4AtDetectionPrevalentAverage = hivScreening->CD4AtDetectionPrevalentAverage;

	/** Copy the OI and death stats */
	const RunStats::OIStats *oiStats = runStats->getOIStats();
	const RunStats::DeathStats *deathStats = runStats->getDeathStats();
	for (int i = 0; i < SimContext::OI_NUM; i++) {
		if (popSummary->numCohortsHIVPositive > 0) {
			summary->numPrimaryOIsPer1000[i] = 1000.0 * oiStats->numPrimaryOIsOI[i] / popSummary->numCohortsHIVPositive;
			summary->numOIsPer1000[i] = 1000.0 * (oiStats->numPrimaryOIsOI[i] + oiStats->numSecondaryOIsOI[i]) / popSummary->numCohortsHIVPositive;
			summary->numDetecedOIsPer1000[i] = 1000.0 * oiStats->numDetectedOIsOI[i] / popSummary->numCohortsHIVPositive;
			summary->numOIDeathsPer1000[i] = 1000.0 * deathStats->numDeathsHIVPosType[i] / popSummary->numCohortsHIVPositive;
		}
		summary->numDeathsPer1000[i] = summary->numOIDeathsPer1000[i];
	}
	for (int i = SimContext::OI_NUM; i < SimContext::DTH_NUM_CAUSES; i++) {
		if (popSummary->numCohortsHIVPositive > 0)
			summary->numDeathsPer1000[i] = 1000.0 * deathStats->numDeathsHIVPosType[i] / popSummary->numCohortsHIVPositive;
	}

	/* initialize the cost effectivenes to zero, calculated in finalizeStats */
	summary->costEffectivenessLYs = 0;
	summary->costEffectivenessQALYs = 0;

	/** Add the new summary to the appropriate vector, create a new vector if this
		is the first run of a run set */
	for (list<vector<Summary *> >::iterator i = summaries.begin(); i != summaries.end(); i++) {
		vector<Summary *> &runSetVector = *i;
		if (summary->runSetName.compare(runSetVector[0]->runSetName) == 0) {
			runSetVector.push_back(summary);
			return;
		}
	}
	vector<Summary *> runSetVector;
	runSetVector.push_back(summary);
	summaries.push_back(runSetVector);
} /* end addRunStats */

/** \brief finalizeStats calculates the final cost-effectiveness ratios for each run */
void SummaryStats::finalizeStats() {
	/** Loop over the run set vectors */
	for (list<vector<Summary *> >::iterator i = summaries.begin(); i != summaries.end(); i++) {
		vector<Summary *> &runSetVector = *i;

		/** Sort the summary vector elements by cost */
		sort(runSetVector.begin(), runSetVector.end(), Summary::compareCosts());

		/** Mark the runs that have dominated cost effectiveness ratios */
		double prevLMs = runSetVector[0]->LMsAverage;
		double prevQALMs = runSetVector[0]->QALMsAverage;
		for (unsigned int i = 1; i < runSetVector.size(); i++) {
			if (runSetVector[i]->LMsAverage <= prevLMs)
				runSetVector[i]->costEffectivenessLYs = -1;
			else
				prevLMs = runSetVector[i]->LMsAverage;
			if (runSetVector[i]->QALMsAverage <= prevQALMs)
				runSetVector[i]->costEffectivenessQALYs = -1;
			else
				prevQALMs = runSetVector[i]->QALMsAverage;
		}

		/** Continue calculating the cost effectiveness ratios until there is no more extended dominance */
		bool doRecalculation = true;
		while (doRecalculation) {
			doRecalculation = false;

			/** Calculate the actual ratios */
			int prevIndexLM = 0;
			int prevIndexQALM = 0;
			for (unsigned int i = 1; i < runSetVector.size(); i++) {
				if (runSetVector[i]->costEffectivenessLYs >= 0) {
					runSetVector[i]->costEffectivenessLYs =
						(runSetVector[i]->costsAverage - runSetVector[prevIndexLM]->costsAverage) /
						(runSetVector[i]->LMsAverage - runSetVector[prevIndexLM]->LMsAverage) * 12.0;
					prevIndexLM = i;
				}
				if (runSetVector[i]->costEffectivenessQALYs >= 0) {
					runSetVector[i]->costEffectivenessQALYs =
						(runSetVector[i]->costsAverage - runSetVector[prevIndexQALM]->costsAverage) /
						(runSetVector[i]->QALMsAverage - runSetVector[prevIndexQALM]->QALMsAverage) * 12.0;
					prevIndexQALM = i;
				}
			}

			/** Determine if any of the ratios have extended dominance, break if so */
			for (unsigned int i = 1; i < runSetVector.size(); i++) {
				if (runSetVector[i]->costEffectivenessLYs >= 0.0) {
					prevIndexLM = i;
					break;
				}
			}
			for (unsigned int i = prevIndexLM + 1; i < runSetVector.size(); i++) {
				if (runSetVector[i]->costEffectivenessLYs >= 0.0) {
					if (runSetVector[i]->costEffectivenessLYs < runSetVector[prevIndexLM]->costEffectivenessLYs) {
						runSetVector[prevIndexLM]->costEffectivenessLYs = -2;
						doRecalculation = true;
						break;
					}
					else {
						prevIndexLM = i;
					}
				}
			}
			for (unsigned int i = 1; i < runSetVector.size(); i++) {
				if (runSetVector[i]->costEffectivenessQALYs >= 0.0) {
					prevIndexQALM = i;
					break;
				}
			}
			for (unsigned int i = prevIndexQALM + 1; i < runSetVector.size(); i++) {
				if (runSetVector[i]->costEffectivenessQALYs >= 0.0) {
					if (runSetVector[i]->costEffectivenessQALYs < runSetVector[prevIndexQALM]->costEffectivenessQALYs) {
						runSetVector[prevIndexQALM]->costEffectivenessQALYs = -2;
						doRecalculation = true;
						break;
					}
					else {
						prevIndexQALM = i;
					}
				}
			}
		}
	}
} /* end finalizeStats */

/** \brief writeSummariesFile appends the summary information to the popstats.out file */
void SummaryStats::writeSummariesFile() {
	/** Open the popstats file and write header if needed by calling SummaryStats::writeSummariesFileHeader() */
	CepacUtil::changeDirectoryToResults();
	if (CepacUtil::fileExists(summariesFileName.c_str())) {
		summariesFile = CepacUtil::openFile(summariesFileName.c_str(), "a");
		if (summariesFile == NULL) {
			summariesFileName.append("-tmp");
			summariesFile = CepacUtil::openFile(summariesFileName.c_str(), "w");
			if (summariesFile == NULL) {
				string errorString = "   ERROR - Could not write popstats or temporary popstats file";
				throw errorString;
			}
			writeSummariesFileHeader();
		}
	}
	else {
		summariesFile = CepacUtil::openFile(summariesFileName.c_str(), "w");
		if (summariesFile == NULL) {
			summariesFileName.append("-tmp");
			summariesFile = CepacUtil::openFile(summariesFileName.c_str(), "w");
			if (summariesFile == NULL) {
				string errorString = "   ERROR - Could not write popstats or temporary popstats file";
				throw errorString;
			}
		}
		writeSummariesFileHeader();
	}

	/** Loop over the run set vectors */
	for (list<vector<Summary *> >::iterator i = summaries.begin(); i != summaries.end(); i++) {
		vector<Summary *> &runSetVector = *i;

		/** Loop over the individual run summaries of the run set vector */
		int j;
		for (vector<Summary *>::iterator i = runSetVector.begin(); i != runSetVector.end(); i++) {
			Summary *summary = *i;
			fprintf(summariesFile, "%s\t", summary->runSetName.c_str());
			fprintf(summariesFile, "%s\t", summary->runName.c_str());
			fprintf(summariesFile, "%s\t", summary->runDate.c_str());
			fprintf(summariesFile, "%s\t", summary->runTime.c_str());
			fprintf(summariesFile, "%ld\t", summary->numCohorts);
			fprintf(summariesFile, "%1.0lf\t", summary->costsAverage);
			fprintf(summariesFile, "%1.4lf\t", summary->LMsAverage);
			fprintf(summariesFile, "%1.4lf\t", summary->QALMsAverage);
			// write out dominated or a real ratio per LY
			if (summary->costEffectivenessLYs == -1)
				fprintf(summariesFile, "DOMINATED\t");
			else if (summary->costEffectivenessLYs == -2)
				fprintf(summariesFile, "dominated\t");
			else if (summary->costEffectivenessLYs != 0.0)
				fprintf(summariesFile, "%1.0lf\t", summary->costEffectivenessLYs);
			else
				fprintf(summariesFile, "\t");
			// write out dominated or a real ratio per QALY
			if (summary->costEffectivenessQALYs == -1)
				fprintf(summariesFile, "DOMINATED");
			else if (summary->costEffectivenessQALYs == -2)
				fprintf(summariesFile, "dominated");
			else if (summary->costEffectivenessQALYs != 0.0)
				fprintf(summariesFile, "%1.0lf", summary->costEffectivenessQALYs);
			// write out HIV+ summ stats
			fprintf(summariesFile, "\t%1.0lf", summary->costsHIVPositiveAverage);
			fprintf(summariesFile, "\t%1.4lf", summary->LMsHIVPositiveAverage);
			fprintf(summariesFile, "\t%1.4lf", summary->QALMsHIVPositiveAverage);
			// write out # OIs, dth causes
			for (j = 0; j < SimContext::OI_NUM; ++j)
				fprintf(summariesFile,"\t%1.3lf", summary->numPrimaryOIsPer1000[j]);
			for (j = SimContext::OI_NUM; j < SimContext::DTH_NUM_CAUSES; ++j)
				fprintf(summariesFile,"\t%1.3lf", summary->numDeathsPer1000[j]);
			// write out HIV+ det stats
			if (summary->monthsToDetectionPrevalentAverage != -1)
				fprintf(summariesFile,"\t%1.3lf",summary->monthsToDetectionPrevalentAverage );
			else
				fprintf(summariesFile,"\t" );
			if (summary->monthsAfterInfectionToDetectionAverage != -1)
				fprintf(summariesFile,"\t%1.3lf", summary->monthsAfterInfectionToDetectionAverage );
			else
				fprintf(summariesFile,"\t" );
			if (summary->CD4AtDetectionPrevalentAverage != -1)
				fprintf(summariesFile,"\t%1.3lf", summary->CD4AtDetectionPrevalentAverage );
			else
				fprintf(summariesFile,"\t" );
			if (summary->CD4AtDetectionIncidentAverage != -1)
				fprintf(summariesFile,"\t%1.3lf", summary->CD4AtDetectionIncidentAverage );
			else
				fprintf(summariesFile,"\t" );

			/** write out total # OIs, acute OI dths, and detected OIs */
			for (j = 0; j < SimContext::OI_NUM; ++j)
				fprintf(summariesFile,"\t%1.3lf", summary->numOIsPer1000[j] );
			for (j = 0; j < SimContext::OI_NUM; ++j)
				fprintf(summariesFile,"\t%1.3lf", summary->numOIDeathsPer1000[j] );
			for (j = 0; j < SimContext::OI_NUM; ++j)
				fprintf(summariesFile,"\t%1.3lf", summary->numDetecedOIsPer1000[j] );
			// write out total # clinic visits
			fprintf(summariesFile,"\t%1.3lf", summary->numClinicVisitsPer1000 );

			fprintf(summariesFile, "\n");
		}
	}

	fprintf(summariesFile, "=========\t=========\t=========\t=========\t=====\t====\t=====" );
		fprintf(summariesFile, "=\t========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
		fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========\n" );
	CepacUtil::closeFile(summariesFile);
} /* end writeSummariesFile */

/** \brief writes out summaries file header */
void SummaryStats::writeSummariesFileHeader() {
	int i;
	fprintf(summariesFile, "\t\t\t\t\tTOTAL COHORT\t\t\t\t\tHIV+ PATIENTS");
	fprintf(summariesFile, "\t\t\tPRIMARY OI CASES PER THOUSAND HIV+ PATIENTS" );
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, "\t");
	fprintf(summariesFile, "DTHS /1000");
	for(i = SimContext::OI_NUM; i < SimContext::DTH_NUM_CAUSES; i++)
		fprintf(summariesFile, "\t");
    fprintf(summariesFile, "MTHS TO DET\tMTHS TO DET\tAVG CD4 AT\tAVG CD4 AT" );
	fprintf(summariesFile, "\tTOTAL NUMBER OI CASES (PRIMARY + SECONDARY) /1000" );
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, "\t");
	fprintf(summariesFile, "TOTAL NUMBER DEATHS FROM ACUTE OI CASES /1000" );
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, "\t");
	fprintf(summariesFile, "TOTAL NUMBER DETECTED OI CASES /1000" );
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, "\t");
	fprintf(summariesFile, "TOTAL # /1000" );
	fprintf(summariesFile, "\n" );
	// NOTE: make sure these titles correspond to what's actually written out
	// in vPrntSummStats()
    fprintf(summariesFile, "RUN SET \tRUN NAME \tRUN DATE \tRUN TIME \tCOHORT \tCOST \tLMs \tQALMs" );
    fprintf(summariesFile, " \tCOST/LY \tCOST/QALY" );
    fprintf(summariesFile, " \tCOST \tLMs \tQALMs" );
	for (i = 0; i < SimContext::DTH_NUM_CAUSES; ++i)
		fprintf(summariesFile, " \t%s", SimContext::DTH_CAUSES_STRS[i]);
    fprintf(summariesFile, " \t(PREVAL) \t(INCID) \tDET (PREVAL) \tDET (INCID)" );
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, " \t%s", SimContext::OI_STRS[i]);
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, " \t%s", SimContext::OI_STRS[i]);
	for (i = 0; i < SimContext::OI_NUM; ++i)
		fprintf(summariesFile, " \t%s", SimContext::OI_STRS[i]);
	fprintf(summariesFile, " \tCLINIC VISITS" );
	fprintf(summariesFile, "\n");
    fprintf(summariesFile, "=========\t=========\t=========\t=========\t=====\t====\t=====" );
    fprintf(summariesFile, "=\t========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
	fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
    fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
	fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
	fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========" );
	fprintf(summariesFile, "\t=========\t=========\t=========\t=========\t=========\n" );
} /* end writeSummariesFileHeader */
