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

		void flower_patch::updateDailyData(const date_struct& date) {
			dailyData = daily_data();
		    dailyData.quantityMyl = 20 * 1000 * 1000;
		    dailyData.amountPollen_g = 1 * 1000;
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
					info = "invalid date.day";
					return false;
				}

				if (Queenage <= 0)
				{
					info = "invalid Queenage";
					return false;
				}

				for (auto& patch : data->flower_patchs) {
					if (!patch.dailyData)
					{
						info = "nullptr flower_patch::dailyData";
						return false;
					}

					if (patch.detectionProbability <= -1)
					{
						info = "invalid detectionProbability";
						return false;
					}
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

				for (auto& patch : data->flower_patchs) {
					patch.dailyData->update(data->date);
				}

				if (honeyEnergyStore < 0) {
					deathReason = "Starvation";
					data->colonyDied = true;
					return;
				}

				if ((data->TotalWorkerAndDroneBrood + data->TotalIHbees + data->TotalForagers) == 0) {
					deathReason = "No bees left";
					data->colonyDied = true;
					return;
				}

				if (data->date.day == DAYS_IN_YEAR && (data->TotalIHbees + data->TotalForagers) < CRITICAL_COLONY_SIZE_WINTER) {
					deathReason = "Winter mortality";
					data->colonyDied = true;
					return;
				}

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
