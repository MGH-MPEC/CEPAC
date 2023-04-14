#pragma once

#include "include.h"

/**
	ClinicVisitUpdater handles all of the patient state and statistics updates for scheduled
	and emergency OI clinic visits.  This includes OI detection and treatment, ART
	treat policy decisions, and OI prophylaxis treatment policy decisions.
*/
class ClinicVisitUpdater : public StateUpdater {
public:
	/* Constructor and Destructor */
	ClinicVisitUpdater(Patient *patient);
	~ClinicVisitUpdater(void);

	/* performInitialUpdates perform all of the state and statistics updates upon patient creation */
	void performInitialUpdates();
	/* performMonthlyUpdates perform all of the state and statistics updates for a simulated month */
	void performMonthlyUpdates();
	/* changes the inputs the updater uses to determine disease progression -- to be used primarily by the transmission model
	 * changes the "nextARTRegimen" based on the ART regimens of the new simContext */
	void setSimContext(SimContext *newSimContext);

private:
	/* performOIDetectionUpdates handles the emergency OI clinic visit and
		determines if the current and prior OIs are observed */
	void performOIDetectionUpdates();
	/* performARTProgramUpdates evaluates ART policies and alters the treatment program */
	void performARTProgramUpdates();
	/* evaluateStartARTPolicy determines if the starting criteria for ART has been met */
	bool evaluateStartARTPolicy();
	/* evaluateStartARTPolicy determines if the starting criteria for ART has been met (Only for early childhood)*/
	bool evaluateStartARTPolicyPeds();
	/* evaluateFailARTPolicy determines if the observed failure criteria for ART has been met */
	SimContext::ART_FAIL_TYPE evaluateFailARTPolicy();
	/* evaluateFailARTPolicyPeds determines if the observed failure criteria for ART has been met (Only for early childhood)*/
	SimContext::ART_FAIL_TYPE evaluateFailARTPolicyPeds();
	/* evaluateStopARTPolicy determines if the stopping criteria for ART has been met */
	SimContext::ART_STOP_TYPE evaluateStopARTPolicy();
	/* evaluateStopARTPolicyPeds determines if the stopping criteria for ART has been met (Only for early childhood)*/
	SimContext::ART_STOP_TYPE evaluateStopARTPolicyPeds();
	/* evaluateSTIInitialStopPolicy determines if the ART treatment should be stopped for the
		initial STI interruption */
	bool evaluateSTIInitialStopPolicy();
	/* evaluateSTIEndpointPolicy determines then end of the STI cycle and indicates observed failure */
	SimContext::ART_FAIL_TYPE evaluateSTIEndpointPolicy();
	/* evaluateSTIRestartPolicy determines if ART should be restarted while interrupted */
	bool evaluateSTIRestartPolicy();
	/* evaluateSTISubsequentStopPolicy determines if the ART treatment should be stopped for
		subsequent STI interruptions */
	bool evaluateSTISubsequentStopPolicy();
	/* performProphProgramUpdates evaluates prophylaxis policies and alters the treatment program */
	void performProphProgramUpdates();
	/* evaluateStartProphPolicy determines if the start criteria for the proph has been met */
	bool evaluateStartProphPolicy(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType);
	/* evaluateStartProphPolicyPeds determines if the start criteria for the proph has been met (Only for early childhood in peds model)*/
	bool evaluateStartProphPolicyPeds(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType);
	/* evaluateStopProphPolicy determines if the sopping criteria for the proph has been met */
	bool evaluateStopProphPolicy(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType);
	/* evaluateStopProphPolicyPeds determines if the stopping criteria for the proph has been met (Only for early childhood in peds model)*/
	bool evaluateStopProphPolicyPeds(SimContext::PROPH_TYPE prophType, SimContext::OI_TYPE oiType);
};
