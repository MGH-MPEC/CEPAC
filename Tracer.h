#pragma once

#include "include.h"

/**
	The Tracer class contains the functionality for creating and writing to a patient trace file.
*/
class Tracer
{
public:
	/* Constructor takes in the run name, simulation context, and desired tracing level */
	Tracer(string runName, SimContext *simContext, int traceLevel);
	~Tracer(void);

	/* Functions to open/close the trace file */
	void openTraceFile();
	void closeTraceFile();
	/* Functions to print the trace header and print tracing text */
	void printTraceHeader();
	void printTrace(int level, const char *format, ...);

private:
	/* Local variables for the simulation context, trace file, and tracing level */
	/** The SimContext corresponding to this Tracer */
	SimContext *simContext;
	/** The name of the file printed to from this Tracer */
	string traceFileName;
	/** The file printed to from this Tracer*/
	FILE *traceFile;
	/** The level of tracing used */
	int traceLevel;
};
