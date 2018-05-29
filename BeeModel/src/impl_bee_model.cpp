//impl_bee_model.cpp

#include "bee_model.h"
#include "impl_bee_model.h"

#include <My/Guard.h>

#include <random>

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

				if (data.Queenage <= 0)
				{
					info = "invalid Queenage";
					return false;
				}

				for (auto& patch : data.flower_patchs) {
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

				if (data.EMERGING_AGE <= 0) {
					info = "invalid EMERGING_AGE";
					return false;
				}

				if (data.DRONE_EMERGING_AGE <= 0) {
					info = "invalid DRONE_EMERGING_AGE";
					return false;
				}

				if (FORAGER_NURSING_CONTRIBUTION < 0 || FORAGER_NURSING_CONTRIBUTION > 1) {
					info = "invalid FORAGER_NURSING_CONTRIBUTION";
					return false;
				}

				return true;
			}

			model_impl::model_impl(std::shared_ptr<model_data> _data_) : data(std::move(_data_))
			{
				if (!data) {
					throw std::invalid_argument("data is nullpr");
				}

				data->colonyDied.register_change(this, [this]() { clear_on_dead(); });
			}

			
			void model_impl::daily_step() {
				ticks++;
				daily_update_proc();
				season_HoPoMo_proc();

				//Egg laying & development:
				worker_eggs_dev_proc();
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
			}

			void model_impl::clear_on_dead() {
				if (data->clearOnDead) {
					for (auto& group : egg_groups) {
						group.number = 0;
					}
					for (auto& group : larva_groups) {
						group.number = 0;
					}
					for (auto& group : pupa_groups) {
						group.number = 0;
						group.healthy = 0;
						group.infectedAsPupa = 0;
					}
					for (auto& group : drone_groups) {
						group.number = 0;
						group.healthy = 0;
						group.infectedAsPupa = 0;
					}
					for (auto& group : egg_drone_groups) {
						group.number = 0;
					}
					for (auto& group : larva_drone_groups) {
						group.number = 0;
					}
					for (auto& group : pupa_drone_groups) {
						group.number = 0;
						group.healthy = 0;
						group.infectedAsPupa = 0;
					}
					for (auto& group : in_hive_bee_groups) {
						group.number = 0;
						group.healthy = 0;
						group.infectedAsPupa = 0;
						group.infectedAsAdult = 0;
					}
				}
			}

			void model_impl::season_HoPoMo_proc() {
				// see Schmickl&Crailsheim2007: p.221 and p.230

				HoPoMo_seasont = season_HoPoMo(date.day);
			}

			static float model_impl::season_HoPoMo(USHORT day, USHORT parameterList[5]) {
				//see Schmickl&Crailsheim2007: p.221 and p.230

				if (!parameterList) {
					parameterList = { 385, 30, 36, 155, 30 };
				}

				auto x1 = parameterList[0];
				auto x2 = parameterList[1]; // earlier increase in egg laying rate than in HoPoMo
				auto x3 = parameterList[2]; // Day of max. egg laying
				auto x4 = parameterList[3];
				auto x5 = parameterList[4];

				float seas1 = (1 - (1 / (1 + x1 * exp(-2 * day / x2))));
				float seas2 = (1 / (1 + x3 * exp(-2 * (day - x4) / x5)));

				return std::max(seas1, seas2);
			}

			void model_impl::worker_eggs_dev_proc() {
				newWorkerLarvae = 0;
				for (auto iter = data->egg_groups.begin(), end = data->egg_groups.end(); iter != end;) {
					iter->age++;
					{
						//random mortality, based on Poisson distribution
						std::random_device rd;
						std::mt19937 gen(rd());
						std::poisson_distribution<USHORT> distr(iter->number * data->MORTALITY_EGGS);
						iter->number -= distr(gen);
						if (iter->number < 0) {
							iter->number = 0;
						}
					}

					if (iter->age == data->HATCHING_AGE) {
						newWorkerLarvae += iter->number;
					}

					auto next = std::next(iter);
					
					if (iter->age >= data->HATCHING_AGE) {
						data->egg_groups.erase(iter);
					}

					iter = next;
				}
			}

			void model_impl::drone_eggs_dev_proc() {
				newDroneLarvae = 0;
				for (auto iter = data->egg_drone_groups.begin(), end = data->egg_drone_groups.end(); iter != end;) {
					iter->age++;
					{
						//random mortality, based on Poisson distribution
						std::random_device rd;
						std::mt19937 gen(rd());
						std::poisson_distribution<USHORT> distr(iter->number * data->MORTALITY_DRONE_EGGS);
						iter->number -= distr(gen);
						if (iter->number < 0) {
							iter->number = 0;
						}
					}

					if (iter->age == data->DRONE_HATCHING_AGE) {
						newDroneLarvae += iter->number;
					}

					auto next = std::next(iter);
					
					if (iter->age >= data->DRONE_HATCHING_AGE) {
						data->egg_drone_groups.erase(iter);
					}

					iter = next;
				}
			}

			void model_impl::new_eggs_proc() {
				// egg laying rate follows a seasonal pattern as described in HoPoMo (Schmickl & Crailsheim 2007)
				// (Queen egg laying rate)
				auto ELRt_HoPoMo = data->MAX_EGG_LAYING * (1 - HoPoMo_seasont); // eggs/day
				
				// egg laying rate considering to available nurse bees
				// (nurse bees egg handling rate)
				auto ELRt_IH = // eggs/day
					(data->TotalIHbees + data->TotalForagers * FORAGER_NURSING_CONTRIBUTION)
					* data->MAX_BROOD_NURSE_RATIO / data->EMERGING_AGE;

				auto ELRt = (data->EGG_LAYING_IH && ELRt_IH < ELRt_HoPoMo) ? ELRt_IH : ELRt_HoPoMo;

				ELRt = std::min(ELRt, MAX_EGG_LAYING);

				//LIMITED BROOD NEST:
				if (data->TotalWorkerAndDroneBrood + ELRt > data->MAX_BROODCELLS) {
					ELRt = data->MAX_BROODCELLS - TotalWorkerAndDroneBrood;
				}

				newWorkerEggs = (UINT)ELRt;

				//CALCULATION OF DRONE EGGS:
				newDroneEggs = (UINT)(newWorkerEggs * data->DRONE_EGGS_PROPORTION);

				//no more drone brood at end of season
				if (
					data->date.day >= (
						data->SEASON_STOP - (data->DRONE_HATCHING_AGE + data->DRONE_PUPATION_AGE + data->DRONE_EMERGING_AGE)
					)
				) {
					newDroneEggs = 0;
				}

				//newWorkerEggs -= newDroneEggs;

				//AGEING OF QUEEN - based on deGrandi-Hofmann, BEEPOP:
				if (data->QueenAgeing) {
					
				}
  if QueenAgeing = true ; GUI: "switch"
  [
    let potentialEggs (MAX_EGG_LAYING
        + (-0.0027 * Queenage ^ 2)
        + (0.395 * Queenage))
          ; Beepops potential egglaying Pt
    set NewWorkerEggs round (NewWorkerEggs * (potentialEggs / MAX_EGG_LAYING) )
  ]

  ; no egg-laying of young queen (also if QUEEN_AGEING = false!):
  if Queenage <= 10
  [
    set NewWorkerEggs 0
      ; Winston p. 203: 5-6d until sexually mature, 2-4d for orientation and mating flight, mating
      ; can be postponed for 4 weeks if weather is bad

    set NewDroneEggs 0
  ]
  if NewWorkerEggs < 0 [ set NewWorkerEggs 0 ]
  if NewDroneEggs < 0 [ set NewDroneEggs 0 ]
			}

 to NewEggsProc
  ; CALLED BY WorkerEggLayingProc   see: HoPoMo p.222 & p.230, ignoring ELRstoch
  let ELRt_HoPoMo (MAX_EGG_LAYING * (1 - HoPoMo_seasont))
  if EMERGING_AGE <= 0 [ set BugAlarm true show "EMERGING_AGE <= 0" ]
  let ELRt_IH (TotalIHbees
      + TotalForagers * FORAGER_NURSING_CONTRIBUTION)
      * MAX_BROOD_NURSE_RATIO / EMERGING_AGE
        ; EMERGING_AGE = 21: total developmental time of worker brood

  let ELRt ELRt_HoPoMo
    ; egg laying rate follows a seasonal pattern as described in
    ; HoPoMo (Schmickl & Crailsheim 2007)

  if EggLaying_IH = true and ELRt_IH < ELRt_HoPoMo
    ; if EggLaying_IH SWITCH is on and not enough nurse bees are available,
    ; the egg laying rate is reduced to ELRt_IH
  [
    set ELRt ELRt_IH
  ]

  if ELRt > MAX_EGG_LAYING
  [
    set ELRt MAX_EGG_LAYING
  ]

  ;   LIMITED BROOD NEST:
  if TotalWorkerAndDroneBrood + ELRt > MAX_BROODCELLS
  [
    set ELRt MAX_BROODCELLS - TotalWorkerAndDroneBrood
  ]

  set NewWorkerEggs round ELRt  ; ROUND! in contrast to HoPoMo

  ; CALCULATION OF DRONE EGGS:
  set NewDroneEggs floor(NewWorkerEggs * DRONE_EGGS_PROPORTION)
  if Day >= SEASON_STOP
     - ( DRONE_HATCHING_AGE
     -   DRONE_PUPATION_AGE
     -   DRONE_EMERGING_AGE )
  [
    set NewDroneEggs 0
  ] ; no more drone brood at end of season (however: Season set to day 1 - 365)

  ; AGEING OF QUEEN - based on deGrandi-Hofmann, BEEPOP:
  if QueenAgeing = true ; GUI: "switch"
  [
    let potentialEggs (MAX_EGG_LAYING
        + (-0.0027 * Queenage ^ 2)
        + (0.395 * Queenage))
          ; Beepops potential egglaying Pt
    set NewWorkerEggs round (NewWorkerEggs * (potentialEggs / MAX_EGG_LAYING) )
  ]

  ; no egg-laying of young queen (also if QUEEN_AGEING = false!):
  if Queenage <= 10
  [
    set NewWorkerEggs 0
      ; Winston p. 203: 5-6d until sexually mature, 2-4d for orientation and mating flight, mating
      ; can be postponed for 4 weeks if weather is bad

    set NewDroneEggs 0
  ]
  if NewWorkerEggs < 0 [ set NewWorkerEggs 0 ]
  if NewDroneEggs < 0 [ set NewDroneEggs 0 ]
end

			/*
			; Egg laying & development:
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
