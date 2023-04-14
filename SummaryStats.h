#pragma once

#include "include.h"

/**
	SummaryStats class contains a list of the summary statistics from each of the
	input files (simulation contexts) that are executed in a given run of the model.
	It contains the functions to generate a summary from a given RunStats object, add it to
	the list of summaries, and output all the summaries to the popstats file.  Main calls these
	functions to add each new summary and append to popstats.out at the end of the run.
*/
class SummaryStats
{
public:
	/* Contstructor and Destructor */
	SummaryStats(string summariesFileName);
	~SummaryStats(void);

	/** Summary class stores the summary information that is written to the popstats file */
	class Summary {
	public:
		/** The name of the run set this summary belongs to */
		string runSetName;
		/** The name of the run corresponding to this summary */
		string runName;
		/** The date of the run */
		string runDate;
		/** The time the run finished */
		string runTime;
		/** The number of Persons in this cohort */
		int numCohorts;
		/** The average costs per Person */
		double costsAverage;
		/** The average life months lived per Person */
		double LMsAverage;
		/** The average quality adjusted life months lived per Person */
		double QALMsAverage;
		/** The cost effectiveness of this run in terms of life years */
		double costEffectivenessLYs;
		/** The cost effectiveness of this run in terms of quality adjusted life years */
		double costEffectivenessQALYs;
		/** The average costs accrued for HIV positive persons */
		double costsHIVPositiveAverage;
		/** The average life months lived by HIV positive Persons*/
		double LMsHIVPositiveAverage;
		/** The average quality adjusted life months lived by HIV positive Persons */
		double QALMsHIVPositiveAverage;
		/** The number of primary OIs person 1000 persons, stratified by OI type */
		double numPrimaryOIsPer1000[SimContext::OI_NUM];
		/** The number of deaths per 1000 deaths (i.e. Persons) stratified by death causes */
		double numDeathsPer1000[SimContext::DTH_NUM_CAUSES];
		/** The average months from infection to detection for incident cases */
		double monthsAfterInfectionToDetectionAverage;
		/** The average months to detection for prevalent cases */
		double monthsToDetectionPrevalentAverage;
		/** The average CD4 at detection for incident cases */
		double CD4AtDetectionIncidentAverage;
		/** The average CD4 at detection for prevalent cases */
		double CD4AtDetectionPrevalentAverage;
		/** The number of OIs per 1000 persons stratified by OI type */
		double numOIsPer1000[SimContext::OI_NUM];
		/** The number of OI deaths per 1000 persons stratified by OI type */
		double numOIDeathsPer1000[SimContext::OI_NUM];
		/** The number of detected OIs per 1000 persons stratified by OI type */
		double numDetecedOIsPer1000[SimContext::OI_NUM];
		/** The number of clinic visits per 1000 person */
		double numClinicVisitsPer1000;
		/** A struct containing an operator that compares the costs of two summaries that returns true if the first summary had lower costs than the second summary */
		struct compareCosts {
			bool operator()(const Summary *s1, const Summary *s2) const {
				return s1->costsAverage < s2->costsAverage;
			}
		};
	}; /* end Summary */

	/* addRunStats adds a new summary to the vector from a RunStats object */
	void addRunStats(RunStats *runStats);
	/* finalizeStats calculates the final cost-effectiveness ratios for each run */
	void finalizeStats();
	/* writeSummariesFile appends the summary inforation to the popstats.out file */
	void writeSummariesFile();

private:
	/*** list of run set vectors of individual run Summary objects,
		uses Summary pointers since objects are large and copy is expensive */
	// TODO: a map from strings to summary vectors would be more efficient, had problems
	//	using this in VC++
	list<vector<Summary *> > summaries;

	/** summaries file name */
	string summariesFileName;
	/** summaries file pointer */
	FILE *summariesFile;

	/* writes out popstats file header */
	void writeSummariesFileHeader();
};
