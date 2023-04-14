#pragma once

#include "include.h"

/**
	TBClinicalUpdater is a state updater that handles diagnostics and treatment of TB
*/
class TBClinicalUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	TBClinicalUpdater(Patient *patient);
	~TBClinicalUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();

private:
	/* performTBTestingUpdates performs TB diagnostics tests on those who are eligible */
	bool performTBTestingUpdates();
	/* checkForStoppingEmpiricTherapy stops empiric therapy if duration is exceeded */
	void checkForStoppingEmpiricTherapy();
	/* checkPendingDSTResults process the results of any pending DST tests */
	void checkPendingDSTResults();
	/* performTBTreatmentUpdates() handles treatment of TB for those in TB care */
	void performTBTreatmentUpdates();
	/* performTBLTFUUpdates() handles LTFU if clinics are not integrated */
	void performTBLTFUUpdates();
	/* evaluateStartTBDiagnostics determines if the patient is eligible for TB Diagnostics */
	bool evaluateStartTBDiagnostics();
	/* performTBProphProgramUpdates evaluates TB proph policies and alters the treatment program */
	void performTBProphProgramUpdates();
	/* performTBTreatmentCostsUpdates handles cost of untreated TB or costs related to tb treatment */
	void performTBTreatmentCostsUpdates();
	/* evaluateStartTBProphPolicy determines if the start criteria for TB proph has been met */
	bool evaluateStartTBProphPolicy();
	/* evaluateStopTBProphPolicy determines if the stopping criteria for TB proph has been met */
	bool evaluateStopTBProphPolicy();

};
