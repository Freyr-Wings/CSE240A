//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include "predictor.h"
#include "gshare.h"
#include "tournament.h"
#include "tage.h"

//
// TODO:Student Information
//
const char *studentName = "Binlu Wang, Kai Wang";
const char *studentID   = "A53316819, A53319240";
const char *email       = "b8wang@ucsd.edu, wkai@eng.ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//



struct GShare * gshare_predictor = NULL;
struct Tournament * tournament_predictor = NULL;
struct TAGE * tage_predictor = NULL;



//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //

    switch (bpType) {
        case STATIC:
            break;
        case GSHARE:
            gshare_predictor = NewGShare(ghistoryBits, ghistoryBits, 2);
            break;
        case TOURNAMENT:
            tournament_predictor = new_tournament(ghistoryBits, lhistoryBits, pcIndexBits);
            break;
        case CUSTOM:
            tage_predictor = NewTAGE(4);
        default:
            break;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{
    //
    //TODO: Implement prediction scheme
    //

    // Make a prediction based on the bpType
    switch (bpType) {
        case STATIC:
            return TAKEN;
        case GSHARE:
            return gshare_predict(gshare_predictor, pc);
        case TOURNAMENT:
            return tournament_predict(tournament_predictor, pc);
        case CUSTOM:
            return tage_predict(tage_predictor, pc);
        default:
            break;
    }

    // If there is not a compatable bpType then return NOTTAKEN
    return NOTTAKEN;
}

void
train_predictor(uint32_t pc, uint8_t outcome)
{
    //
    //TODO: Implement Predictor training
    //
    switch (bpType) {
        case GSHARE:
            gshare_train(gshare_predictor, pc, outcome);
            break;
        case TOURNAMENT:
            tournament_train(tournament_predictor, pc, outcome);
            break;
        case CUSTOM:
            tage_train(tage_predictor, pc, outcome);
            break;
        default:
            break;
    }
}
