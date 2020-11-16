// Author: Hongliang Gao;   Created: Jan 27 2011
// Description: sample predictors for cbp3.

#include <stdio.h>
#include <cassert>
#include <string.h>
#include <inttypes.h>

using namespace std;
#include "cbp3_def.h"
#include "cbp3_framework.h"

// this file includes two sample predictors
// one is a 64 KB gshare conditional predictor
// the other is a 64 KB indirect predictor indexed by (pc ^ history)

// rewind_marked is also used to show how to rewind the reader for multiple runs
// the predictor will use gshare in the first run
// the indirect predictor will be used in the second run

// NOTE: rewind_marked is only provided to help tuning work. the final
// submitted code should only include one run.


#define GSHARE_SIZE 11 //  3-bit counters = 64 KB cost

// predictor tables
int8_t   *gtable;

// two branch history registers:
// the framework provids real branch results at fetch stage to simplify branch history
// update for predicting later branches. however, they are not available until execution stage
// in a real machine. therefore, you can only use them to update predictors at or after the
// branch is executed.
// in this sample code, we update predictors at retire stage where uops are processed
// in order to enable easy regneration of branch history.

// cost: depending on predictor size
uint32_t brh_fetch;
uint32_t brh_retire;

// count number of runs
uint32_t runs;

void PredictorInit() {
    runs = 0;
    gtable = new int8_t[1 << GSHARE_SIZE];
    assert(gtable);
}

void PredictorReset() {
    // this function is called before EVERY run
    // it is used to reset predictors and change configurations

    if (runs == 0)
        printf("Predictor:gshare\nconfig: %i counters, %i KB cost\n", 1 << GSHARE_SIZE, (1 << GSHARE_SIZE) * 3 / 8 / 1024);
    for (int i = 0; i < (1 << GSHARE_SIZE); i ++)
        gtable[i] = 0;
  
    brh_fetch = 0;
    brh_retire = 0;
}

void PredictorRunACycle() {
    // get info about what uops are processed at each pipeline stage
    const cbp3_cycle_activity_t *cycle_info = get_cycle_info();

    // make prediction at fetch stage
    for (int i = 0; i < cycle_info->num_fetch; i++) {
        uint32_t fe_ptr = cycle_info->fetch_q[i];
        const cbp3_uop_dynamic_t *uop = &fetch_entry(fe_ptr)->uop;

        if (runs == 0 && uop->type & IS_BR_CONDITIONAL) {
            // get prediction
            uint32_t gidx = (brh_fetch ^ uop->pc) & ((1 << GSHARE_SIZE) - 1);
            bool gpred = (gtable[gidx] >= 0);
            assert(report_pred(fe_ptr, false, gpred));
        }

        // update fetch branch history
        if (uop->type & IS_BR_CONDITIONAL)
            brh_fetch = (brh_fetch << 2) | (uop->br_taken ? 1 : 0);
        else if (uop_is_branch(uop->type))
            brh_fetch = (brh_fetch << 1) | 1;
    }

    for (int i = 0; i < cycle_info->num_retire; i++) {
        uint32_t rob_ptr = cycle_info->retire_q[i];
        const cbp3_uop_dynamic_t *uop = &rob_entry(rob_ptr)->uop;

        if (runs == 0 && uop->type & IS_BR_CONDITIONAL) {
            uint32_t gidx = (brh_retire ^ uop->pc) & ((1 << GSHARE_SIZE) - 1);

            // update predictor
            bool t = uop->br_taken;
            if (t && gtable[gidx] < 3)
                gtable[gidx] ++;
            else if (!t && gtable[gidx] > -4)
                gtable[gidx] --;
        }
        // update retire branch history
        if (uop->type & IS_BR_CONDITIONAL)
            brh_retire = (brh_retire << 2) | (uop->br_taken ? 1 : 0);
        else if (uop_is_branch(uop->type))
            brh_retire = (brh_retire << 1) | 1;
    }
}

void PredictorRunEnd() {
    runs ++;
    if (runs < 1) // set rewind_marked to indicate that we want more runs
        rewind_marked = true;
}

void PredictorExit() {
    delete [] gtable;
}
