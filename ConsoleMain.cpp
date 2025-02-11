// console_main.cpp : Defines the main entry point for a console based application
//

/** \mainpage The CEPAC Model: Main Page
 *
 * This document was generated using Doxygen.  To maintain this document, keep your comments consistent with Doxygen
 * formatting and run Doxygen using the CEPAC-treatm Doxygen configuration file.  (This can be found on the shared drive at
 * cepac$/CEPAC - All Users/Projects/Programmers/Doxygen)
 *
 * This should mainly be used as a reference guide for programmers in conjunction with the flowcharts and CEPAC user guide.
 * Good places to start are the Patient object, the StateUpdater object, and the SimContext object.  As a general overview, Patient
 * contains the current patient state, SimContext primarily contains information drawn from the CEPAC .in files, RunStats
 * primarily contains information used in CEPAC .out files, SummaryStats primarily contains information for the popStats files, and Tracer
 * contains information for the trace files.  In addition, the monthly update functions from the main page of the flowchart each
 * have their own class which inherits from StateUpdater.  The main function can be found in ConsoleMain.cpp or, if you are using the
 * wxWidgets GUI, main.cpp.  A number of helper functions are found in the CepacUtil class.
 *
 * Good luck!
 *
 */

#include "include.h"

/** \brief Main function for a console based application */
int main(int argc, char *argv[]) {

	/** Console application uses the directory given on the command line or the
		current directory if none is given (like the old model) */
	if (argc > 1) {
		CepacUtil::inputsDirectory = argv[1];
		CepacUtil::changeDirectoryToInputs();
	}
	else {
		/** Specify that the directory of the exe is the inputsDirectory */
		CepacUtil::useCurrentDirectoryForInputs();
		/** Need to actively cd into the inputsDirectory if on Mac */
		#if __APPLE__
			CepacUtil::changeDirectoryToInputs();
		#endif
	}

	/** Create the results directory and the summaries stats object */
	CepacUtil::createResultsDirectory();
	string summariesFileName = CepacUtil::FILE_NAME_SUMMARIES;
	SummaryStats *summaryStats = new SummaryStats(summariesFileName);

	/** Determine and loop over the list of input files in the working directory */
	CepacUtil::findInputFiles();
	for (vector<string>::iterator inputFileIter = CepacUtil::filesToRun.begin();
		inputFileIter != CepacUtil::filesToRun.end(); inputFileIter++) {

		/** Display the file name being run */
		printf("Running File: %s\n", (*inputFileIter).c_str());

		/** Get the input file name and strip off the extension to get the run name */
		string inputFileName = *inputFileIter;
		string runName = inputFileName.substr(0, inputFileName.find(CepacUtil::FILE_EXTENSION_FOR_INPUT));

		/** Create the simulation context and read in the input file */
		SimContext *simContext = new SimContext(runName);

		try {
			simContext->readInputs();
		}
		catch (string &errorString) {
			printf("%s\n", errorString.c_str());
			continue;
		}

		/** Initializes the random number generator with either a random or fixed seed */
		CepacUtil::setRandomSeedType(simContext->getRunSpecsInputs()->randomSeedByTime);

		/** Create a new run statistics object for this simulation context */
		RunStats *runStats = new RunStats(runName, simContext);
		CostStats *costStats = new CostStats(runName, simContext);

		/** Create a tracing object for this simulation context and print out the trace file header */
		Tracer *tracer = new Tracer(runName, simContext, 1);
		tracer->openTraceFile();
		tracer->printTraceHeader();

		/** Load the number of cohorts and settings for stopping simulation */
		int numCohortsLimit = simContext->getRunSpecsInputs()->numCohorts;

		bool useAlternateStopping = false;
		int totalHIVPositiveLimit = 0;
		int totalCohortsLimit = 0;

		if (simContext->getHIVTestInputs()->enableHIVTesting){
			useAlternateStopping = simContext->getHIVTestInputs()->useAlternateStoppingRule;
			totalHIVPositiveLimit = simContext->getHIVTestInputs()->totalCohortsWithHIVPositiveLimit;
			totalCohortsLimit = simContext->getHIVTestInputs()->totalCohortsLimit;
		}
		else if (simContext->getEIDInputs()->enableHIVTestingEID){
			useAlternateStopping = simContext->getEIDInputs()->useAlternateStoppingRuleEID;
			totalHIVPositiveLimit = simContext->getEIDInputs()->totalCohortsWithHIVPositiveLimitEID;
			totalCohortsLimit = simContext->getEIDInputs()->totalCohortsLimitEID;
		}


		bool useCohortParsing = simContext->getOutputInputs()->enableSubCohorts;
		//determine max subcohort size
		int runsizeSubcohorts = 0;
		if (useCohortParsing){
			for(int i =0; i < SimContext::MAX_NUM_SUBCOHORTS; i++){
				int nextCohortSize = simContext->getOutputInputs()->subCohorts[i];
				if (nextCohortSize > runsizeSubcohorts)
					runsizeSubcohorts = nextCohortSize;
				else
					break;
			}
		}

		/** Loop over the patient simulations for this context */
		int numRun = 0;

		while (true) {
			if (useCohortParsing){
				if (runStats->getPopulationSummary()->numCohorts >= runsizeSubcohorts)
					break;
			}
			else{
				if (!useAlternateStopping) {
					if (runStats->getPopulationSummary()->numCohorts >= numCohortsLimit)
						break;
				}
				else {
					if ((runStats->getPopulationSummary()->numCohortsHIVPositive >= totalHIVPositiveLimit)||(runStats->getPopulationSummary()->numCohorts >= totalCohortsLimit))
						break;
				}
			}

			/** If using dynamic transmission, the warmup run will be used to calculate monthly transmission rate multipliers from the monthly incidence. After the warmup period all other outputs will be discarded and the run will restart anew.*/
			if (simContext->getCohortInputs()->showTransmissionOutput && simContext->getCohortInputs()->useDynamicTransm &&
					numRun == simContext->getCohortInputs()->dynamicTransmWarmupSize){
				// reinitializing the outputs after the warmup run, but keeping the dynamic transmission incidence outputs to use in calculating monthly transmission rates and multipliers for the rest of the run  
				// since stopping the main run is based on runStats this restarts the run; the first 50 patients will be traced again and show up twice in the trace file
				runStats->initRunStats(true);

				simContext->disableDynamicTransmInc();
				
				simContext->enablePrEP(simContext->getCohortInputs()->keepPrEPAfterWarmup);
			}

			/** Create a new patient with the given simulation context*/
			Patient *patient = new Patient(simContext, runStats, costStats, tracer);


			/** Loop over the lifetime of the patient and simulate months */
			while (patient->isAlive()) {
				patient->simulateMonth();
			}

			delete patient;
			numRun++;
		}

		/** Write out the stats file for this simulation context and add to the summary stats */
		runStats->finalizeStats();
		try {
			runStats->writeStatsFile();
		}
		catch (string &errorString) {
			printf("%s\n", errorString.c_str());
		}

		if (simContext->getOutputInputs()->enableDetailedCostOutputs){
			costStats->finalizeStats();
			/** Write out the cost stats file for this simulation context*/
			try {
				costStats->writeStatsFile();
			}
			catch (string &errorString) {
				printf("%s\n", errorString.c_str());
			}
		}

		/** Add the individual run stats to the summary stats object */
		summaryStats->addRunStats(runStats);

		/** Display the summary stats for this run in the results window */
		printf("%s\t cost $%0.0f \t LMs %0.2f \t QALMs %0.2f \n",
			runStats->getPopulationSummary()->runName.c_str(),
			runStats->getPopulationSummary()->costsAverage,
			runStats->getPopulationSummary()->LMsAverage,
			runStats->getPopulationSummary()->QALMsAverage);

		/** Close the trace file and destroy the tracer object */
		tracer->closeTraceFile();
		delete tracer;

		/** Destroy the runStats and simContext objects */
		delete runStats;
		delete costStats;
		delete simContext;
	}

	/** Finalize the summary stats and print to the popstats file,
		destroy the summary stats object */
	summaryStats->finalizeStats();
	try {
		summaryStats->writeSummariesFile();
	}
	catch (string &errorString) {
		printf("%s\n", errorString.c_str());
	}
	delete summaryStats;

	return 0;
} /* end main */

