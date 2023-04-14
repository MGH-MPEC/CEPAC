#pragma once

#include "include.h"

/**
	The AcuteOIUpdater class tests for the occurrence of acute OIs each month and updates
	all the patient state and statistics if they occur.
*/
class AcuteOIUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	AcuteOIUpdater(Patient *patient);
	~AcuteOIUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

private:
	/* determineAcuteOI returns the acute OI that occurs this month or OI_NONE if no OI occurs */
	SimContext::OI_TYPE determineAcuteOI();
	/* determineDeathRiskSevereOI determines the death rate ratio for a severe acute OI or TB modeled as a severe acute OI and adds it to the mortality risks */
	void determineDeathRiskSevereOI(SimContext::OI_TYPE oiType);
};
