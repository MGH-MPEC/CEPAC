#include "include.h"

/** \brief Constructor takes in the run name, simulation context, and desired tracing level
 *
 * \param runName a string identify the run corresponding to this Tracer: this->traceFileName will be the runName appended to CepacUtil::FILE_EXTENSION_FOR_TRACE
 * \param *simContext this->simContext
 * \param traceLevel this->traceLevel
 **/
Tracer::Tracer(string runName, SimContext *simContext, int traceLevel) {
	traceFileName = runName;
	traceFileName.append(CepacUtil::FILE_EXTENSION_FOR_TRACE);
	this->traceLevel = traceLevel;
	this->simContext = simContext;
} /* end Constructor */

/** \brief Destructor is empty, no cleanup required */
Tracer::~Tracer(void) {

} /* end Destructor */

/** \brief openTraceFile opens the trace file for writing */
void Tracer::openTraceFile() {
	CepacUtil::changeDirectoryToResults();
	traceFile = CepacUtil::openFile(traceFileName.c_str(), "w");
} /* end openTraceFile */

/** \brief closeTraceFile closes the trace file */
void Tracer::closeTraceFile() {
	// return if trace file is not valid
	if (traceFile == NULL)
		return;

	CepacUtil::closeFile(traceFile);
} /* end closeTraceFile */

/** \brief printTraceOutputHeader prints out the HVL/CD4 strata information to trace file */
void Tracer::printTraceHeader() {
	// return if trace file is not valid
	if (traceFile == NULL)
		return;

	const SimContext::RunSpecsInputs *runSpecsInputs = simContext->getRunSpecsInputs();
	printTrace(1, "=============================\n");
	printTrace(1, "HVL: %s = 0 - 20\n", SimContext::HVL_STRATA_STRS[SimContext::HVL_VLO]);
	printTrace(1, "     %s = 20 - 500\n", SimContext::HVL_STRATA_STRS[SimContext::HVL__LO]);
	printTrace(1, "     %s = 500 - 3k\n", SimContext::HVL_STRATA_STRS[SimContext::HVL_MLO]);
	printTrace(1, "     %s = 3k - 10k\n", SimContext::HVL_STRATA_STRS[SimContext::HVL_MED]);
	printTrace(1, "     %s = 10k - 30k\n", SimContext::HVL_STRATA_STRS[SimContext::HVL_MHI]);
	printTrace(1, "     %s = 30k - 100k\n", SimContext::HVL_STRATA_STRS[SimContext::HVL__HI]);
	printTrace(1, "     %s = > 100k\n", SimContext::HVL_STRATA_STRS[SimContext::HVL_VHI]);
	printTrace(1, "=============================\n");
	printTrace(1, "CD4: %s = 0 - %1.0f\n", SimContext::CD4_STRATA_STRS[SimContext::CD4_VLO],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4_VLO]);
	printTrace(1, "     %s = %1.0f - %1.0f\n", SimContext::CD4_STRATA_STRS[SimContext::CD4__LO],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4_VLO],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4__LO]);
	printTrace(1, "     %s = %1.0f - %1.0f\n", SimContext::CD4_STRATA_STRS[SimContext::CD4_MLO],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4__LO],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4_MLO]);
	printTrace(1, "     %s = %1.0f - %1.0f\n", SimContext::CD4_STRATA_STRS[SimContext::CD4_MHI],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4_MLO],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4_MHI]);
	printTrace(1, "     %s = %1.0f - %1.0f\n", SimContext::CD4_STRATA_STRS[SimContext::CD4__HI],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4_MHI],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4__HI]);
	printTrace(1, "     %s = > %1.0f\n", SimContext::CD4_STRATA_STRS[SimContext::CD4_VHI],
		runSpecsInputs->CD4StrataUpperBounds[SimContext::CD4__HI]);
	printTrace(1, "=============================\n");
	printTrace(1, "\n\n-----------------------------\n");
	printTrace(1, "BEGIN SCENARIO %s [%d]\n", runSpecsInputs->runName.c_str(), traceLevel);
	printTrace(1, "-----------------------------\n");
} /* end printTraceHeader */

/** \brief printTrace prints out the specified text to the trace file if at the specified tracing level
 *
 * \param level an integer representing the trace level of the information to be printed: only prints if level <= this->traceLevel
 * \param format the information to be printed
 **/
void Tracer::printTrace(int level, const char *format, ...) {
	// return if trace file is not valid
	if (traceFile == NULL)
		return;

	if (level > traceLevel)
		return;
	va_list args;
	va_start (args, format);
	vfprintf(traceFile, format, args);
	va_end(args);
} /* end printTrace */
