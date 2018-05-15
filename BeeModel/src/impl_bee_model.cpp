//impl_bee_model.cpp

#include "bee_model.h"
#include "impl_bee_model.h"

namespace My {

	namespace BeeModel {

		std::shared_ptr<model_data> model_data::create_sample() {
			auto res = std::make_shared<model_data>();
			res->day_number = 1;

			return std::move(res);
		}

		void model_data::date_struct::inc() {
			if (++day > DAYS_IN_YEAR) {
				year += day / DAYS_IN_YEAR;
				day = day % DAYS_IN_YEAR;
			}
		}

		int model_data::getForagingPeriodForToday() {
			auto part_of_day = 0.45 * -cos(2 * PI * date.day/(float)DAYS_IN_YEAR);
			if (part_of_day < 0) {
				part_of_day = 0;
			}

			return (int)((24 * 60 * 60 /*seconds in day*/) * part_of_day);
		}

		namespace Impl {

			bool verify_data(const model_data& data, const char*& info) {
				if (data.date.day > DAYS_IN_YEAR || data.date.day < 1) {
					info = "date.day";
					return false;
				}

				if (Queenage <= 0)
				{
					info = "Queenage";
					return false;
				}

				return true;
			}

			
			void model_impl::daily_step() {
				ticks++;
				daily_update_proc();
			}

			void model_impl::daily_update_proc() {
				data->date.inc();
				DeathsAdultWorkers_t = 0;
				SumLifeSpanAdultWorkers_t = 0;
				DailyMiteFall = 0;
				Pupae_W&D_KilledByVirusToday = 0;
				NewReleasedMitesToday = 0;
					// all (healthy and infected) mites released from cells (mothers+offspring)
					// on current day (calculated after MiteFall!)

				for(auto& obj: data->forager_squadrons) {
					obj.km_today = 0;
				}

				foragingPeriodForToday = data->getForagingPeriodForToday();
				data->Queenage++;

if ReadInfile = false
  [
    ask flowerPatches
    [ ; flower patches are set to the max. amount of nectar and pollen possible today:
      set quantityMyl FlowerPatchesMaxFoodAvailableTodayREP who "Nectar"
      set amountPollen_g FlowerPatchesMaxFoodAvailableTodayREP who "Pollen"
    ]
   ]

 ask flowerPatches
 [
   set nectarVisitsToday 0 set pollenVisitsToday 0
   if detectionProbability < -1
   [
     set BugAlarm true
     user-message "Wrong detection probability! Set 'ModelledInsteadCalcDetectProb' 'false' and re-start run!"
   ]
 ]

 if ReadInfile = true
 [
   set TodaysSinglePatchList []
     ; short list, contains data of current patch and only for today
   set TodaysAllPatchesList []
     ; shorter list, contains data of all patches, but only for today
   let counter (Day - 1)
   repeat N_FLOWERPATCHES
   [
     ; todays data for ALL N_FLOWERPATCHES flower patches are saved in a new,
     ; shorter list (= todaysAllPatchesList)

     set TodaysSinglePatchList (item counter AllDaysAllPatchesList)
       ; this new, shorter list (= todaysAllPatchesList) is comprised of very
       ; short lists (=todaysSinglePatchList) that contain only the data of the
       ; current patch and only for today

     set TodaysAllPatchesList fput TodaysSinglePatchList TodaysAllPatchesList
       ; fput: faster as lput (NetLogo version 4)! however: list is in reversed order!

     set counter counter + 365
     let id item 1 TodaysSinglePatchList ; patch number

     ask flowerpatch id
     [
       set amountPollen_g item 8 TodaysSinglePatchList ; [g]
       if amountPollen_g < 0 [ set amountPollen_g 0 ]
       set quantityMyl (item 10 TodaysSinglePatchList) * 1000 * 1000
         ; [microlitres]  new nectar value from infile (emptied flowers
         ; replenish nectar completely (or are replace by new flowers))

       if quantityMyl < 0 [ set quantityMyl 0 ]
       if id != who [  user-message "Error in id / who!" set BugAlarm true  ]

       if shape != "fadedflower"
       [
         ifelse amountPollen_g > 250
         [ set shape "flowerorange" ]
         [ set shape "flower" ]
       ]
         ; if a "reasonable" amount of pollen available, patch is shown
         ; as 'pollen patch'

       ifelse quantityMyl < CROPVOLUME * SQUADRON_SIZE [ set color grey ]
       [
         set color scale-color red eef 0 50
           ; colour: reddish, dependent on eef, if eff >= 50: white
       ]
     ]
   ] ; ask flowerpatch ID

   set todaysAllPatchesList reverse todaysAllPatchesList
     ; to correct the reversed order, caused by the fput command
 ] ; repeat

 ask patches [ set pcolor PATCHCOLOR ]

 ask hives
 [
   set shape "beehiveDeepHive"
     ; # of supers on drawn colony depends on honey store

   if HoneyEnergyStore / ENERGY_HONEY_per_g > 15000 [ set shape "beehive1super" ]
   if HoneyEnergyStore / ENERGY_HONEY_per_g > 30000 [ set shape "beehive2super" ]
   if HoneyEnergyStore / ENERGY_HONEY_per_g > 45000 [ set shape "beehive3super" ]
   if HoneyEnergyStore / ENERGY_HONEY_per_g > 60000 [ set shape "beehive4super" ]
   if HoneyEnergyStore / ENERGY_HONEY_per_g > 75000 [ set shape "beehive5super" ]
   if HoneyEnergyStore / ENERGY_HONEY_per_g > 90000 [ set shape "beehive6super" ]
   if HoneyEnergyStore / ENERGY_HONEY_per_g > 105000 [ set shape "beehive7super" ]
   if HoneyEnergyStore < 0
   [
     if ColonyDied = false
     [
       output-print word "Starvation! Colony died on Day " ticks
     ]
     set ColonyDied true
   ]
 ] ; ask hives

 if (ticks > 1) and (TotalWorkerAndDroneBrood + TotalIHbees + TotalForagers = 0)
 [
   if ColonyDied = false
   [
     output-print word "No bees left! Colony died on Day " ticks
   ]
   set ColonyDied true
 ]

 if (Day = 365)
 [
   output-type word "31.12.: COLONY SIZE: " (TotalIHbees + TotalForagers)
   output-type "   HONEY STORE [kg]: "
   output-print precision (HoneyEnergyStore / (1000 * ENERGY_HONEY_per_g)) 1
 ]

 if (Day = 365) and (TotalIHbees + TotalForagers < CRITICAL_COLONY_SIZE_WINTER)
 [
   if ColonyDied = false
   [
     output-print word "Winter mortality! Colony died on Day " ticks
   ]
   set ColonyDied true
 ]

 if ColonyDied = true
 [
   ask hives [ set color grey ]
     ; grey colony: died! (even if it "recovers" later, it remains grey)

   if stopDead = true
   [
     ask Signs with [shape = "skull"]
     [
       show-turtle
     ]
   ]
   ask patches [ set pcolor black ]
   if stopDead = true
   [
     ask eggCohorts [ set number 0]
     ask larvaeCohorts [ set number 0]
     ask pupaeCohorts
     [
       set number 0
       set number_Healthy 0
       set number_infectedAsPupa 0
     ]
     ask IHbeeCohorts
     [
       set number 0
       set number_Healthy 0
       set number_infectedAsPupa 0
       set number_infectedAsAdult 0
     ]
     ask foragerSquadrons [ die ]
     ask droneEggCohorts [ set number 0]
     ask droneLarvaeCohorts [ set number 0]
     ask dronePupaeCohorts
     [
       set number 0
       set number_Healthy 0
       set number_infectedAsPupa 0
     ]
     ask droneCohorts [ set number 0  ]
   ]
 ]
			}

			/*
			DailyUpdateProc
			SeasonProc_HoPoMo
			; Egg laying & development:
			WorkerEggsDevProc
			DroneEggsDevProc
			NewEggsProc
			SwarmingProc
			WorkerEggLayingProc
			DroneEggLayingProc
			WorkerLarvaeDevProc
			DroneLarvaeDevProc
			NewWorkerLarvaeProc
			NewDroneLarvaeProc
			WorkerPupaeDevProc
			DronePupaeDevProc
			NewWorkerPupaeProc
			NewDronePupaeProc
			WorkerIHbeesDevProc
			DronesDevProc
			BroodCareProc
			NewIHbeesProc
			NewDronesProc
			; Varroa mite module:
			MiteProc
			BeekeepingProc
			DrawIHcohortsProc
			; Foraging module:
			GenericPlotClearProc
			Start_IBM_ForagingProc
			CountingProc
			PollenConsumptionProc
			HoneyConsumptionProc
			DoPlotsProc
			*/

		} // namespace Impl

	} // namespace BeeModel

} // namespace My
