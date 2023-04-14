#pragma once

#include "include.h"

/**
	HIVTestingUpdater is an updater that manages the HIV testing of all patients and the
	montly events for HIV negative patients.  It handles all the scheduled testing programs and
	background screening for the detection of HIV.  
*/
class HIVTestingUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	HIVTestingUpdater(Patient *patient);
	~HIVTestingUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

	/* changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model
	 * changes the testing frequency and acceptance probability based on the testing strategy of the new simContext */
	void setSimContext(SimContext *newSimContext);

private:
	/* initRegularScreening sets an unlinked patient's user-defined testing interval and acceptance probability for the run if tests are available */
	void initRegularScreening(bool atInit = true);
	/* performRegularScreeningUpdates determines if a screening occurs, if its accepted, if
		they are detected, and updates the associated state and statistics */
	void performRegularScreeningUpdates();
	/* performBackgroundScreeningUpdates handles whether patient is detected by background
		screening and updates the associated state and statistics */
	void performBackgroundScreeningUpdates();
	/* performLabStagingUpdates determines if lab staging occurs, if its accepted, and updates state and statistics*/
	void performLabStagingUpdates(bool wasPrevDetected);
	/* performEIDScreeningUpdates() determines if screening occurs for EID, if its accepted, if they are detected and updates teh associated state and statistics */
	void performEIDScreeningUpdates();

};
