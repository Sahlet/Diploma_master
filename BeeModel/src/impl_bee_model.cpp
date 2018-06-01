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

		void flower_patch::daily_data::update(const date_struct& date) {
			LOGW("not overwrited flower_patch::daily_data::update function (const quantityMyl and amountPollen_g)");
			dailyData = daily_data();
		    dailyData.quantityMyl = 20 * 1000 * 1000; //see FlowerPatchesMaxFoodAvailableTodayREP in nlogo file;
		    dailyData.amountPollen_g = 1 * 1000; //see FlowerPatchesMaxFoodAvailableTodayREP in nlogo file;
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

				if (DRONE_EGGS_PROPORTION < 0 || DRONE_EGGS_PROPORTION > 1) {
					info = "invalid DRONE_EGGS_PROPORTION";
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
				if (data->colonyDied) {
					return;
				}
				ticks++;
				daily_update_proc();
				if (data->colonyDied) {
					return;
				}
				
				season_HoPoMo_proc();

				//Egg laying & development:
				worker_eggs_dev_proc();
				drone_eggs_dev_proc();
				new_eggs_proc();
				if (data->Swarming != swarming_type::NO_SWARMING) {
					swarming_proc();
				}
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

				if (data->honeyEnergyStore < 0) {
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
				if (!data->ClearOnDead) {
					return;
				}

				data->egg_groups.clear();
				data->egg_drone_groups.clear();
				data->larva_groups.clear();
				data->larva_drone_groups.clear();
				data->pupa_groups.clear();
				data->pupa_drone_groups.clear();
				data->drone_groups.clear();
				data->in_hive_bee_groups.clear();
				data->forager_squadrons.clear();
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

				newWorkerEggs = (UINT)(ELRt * (1 - data->DRONE_EGGS_PROPORTION));
				newDroneEggs = (UINT)(ELRt * data->DRONE_EGGS_PROPORTION);

				//no more drone brood at end of season
				if (
					data->date.day >= (
						data->SEASON_STOP - (data->DRONE_HATCHING_AGE + data->DRONE_PUPATION_AGE + data->DRONE_EMERGING_AGE)
					)
				) {
					newDroneEggs = 0;
				}

				//AGEING OF QUEEN - based on deGrandi-Hofmann, BEEPOP:
				if (data->QueenAgeing) {
					float potentialEggs = data->MAX_EGG_LAYING + (-0.0027 * data->Queenage ^ 2) + (0.395 * data->Queenage);
					newWorkerEggs = (UINT)(newWorkerEggs * potentialEggs / data->MAX_EGG_LAYING)
				}
				
				//no egg-laying of young queen (also if QUEEN_AGEING = false!):
				if (data->Queenage <= 10) {
					//Winston p. 203: 5-6d until sexually mature, 2-4d for orientation and mating flight, mating
      				//can be postponed for 4 weeks if weather is bad
					newWorkerEggs = 0;
					newDroneEggs = 0;
				}

				newWorkerEggs = std::max(0, newWorkerEggs);
				newDroneEggs = std::max(0, newDroneEggs);
			}

			void swarming_proc() {
				float fractionSwarm = 0.6; // Winston p. 187
				UINT broodSwarmingTH = 17000; // Fefferman & Starks 2006 (model)
				UINT lastSwarmingDate = 199; // Winston 1980: prime: 14.05.(134) after swarm: 18.07.(199)

				if (data->SwarmingDay == 0 && data->TotalWorkerAndDroneBrood > broodSwarmingTH and data->date.day <= (lastSwarmingDate - data->PRE_SWARMING_PERIOD)) {
					data->SwarmingDay = data->date.day + data->PRE_SWARMING_PERIOD;
				}

				if (data->date.day >= data->SwarmingDay - data->PRE_SWARMING_PERIOD && data->date.day <= data->SwarmingDay) {
					if (data->Swarming == swarming_type::PARENTAL_COLONY) {
						newDroneEggs = 0;
						newWorkerEggs = 0;
						if (data->date.day == data->SwarmingDay) {
							//SWARMING of PARENTAL colony:

							//a new queen is left in the hive, still in a capped cell, ca. 7 days before she emerges (Winston p. 187)
							data->Queenage = -7;

							//Winston p. 185: 36mg honey is taken by a swarming bee:
							data->honeyEnergyStore -= (data->TotalForagers + data->TotalIHbees) * 0.036 * data->ENERGY_HONEY_per_g * fractionSwarm;

							//(1 - fractionSwarm) of all healthy & infected in-hive bees stay in the hive
							for (auto& group : data->in_hive_bee_groups) {
								group.healthy = (USHORT)(group.healthy * (1 - fractionSwarm));
								group.infectedAsPupa = (USHORT)(group.infectedAsPupa * (1 - fractionSwarm));
								group.infectedAsAdult = (USHORT)(group.infectedAsAdult * (1 - fractionSwarm));
								group.number = group.healthy + group.infectedAsPupa + group.infectedAsAdult;
							}

							//(1 - fractionSwarm) of all healthy & infected drones bees stay in the hive
							for (auto& group : data->drone_groups) {
								group.healthy = (USHORT)(group.healthy * (1 - fractionSwarm));
								group.infectedAsPupa = (USHORT)(group.infectedAsPupa * (1 - fractionSwarm));
								group.number = group.healthy + group.infectedAsPupa;
							}

							std::mt19937 gen(time());

							//fractionSwarm foragers leave the colony
							for (auto iter = data->forager_squadrons.begin(), end = data->forager_squadrons.end(); iter != end;) {
								auto next = std::next(iter);
					
								if ((gen() % 100) * 0.01 < fractionSwarm) {
									data->forager_squadrons.erase(iter);
								}

								iter = next;
							}

							data->PhoreticMites = (UINT)(data->PhoreticMites * (1 - fractionSwarm)); //the phoretic mite population in the hive is reduced
							data->SwarmingDay = 0; //allows production of after swarms
						}
					}

					if (data->Swarming == swarming_type::PRIME_SWARM) {
						newDroneEggs = 0;
      					newWorkerEggs = 0;
      					if (data->date.day == data->SwarmingDay) {
      						data->egg_groups.clear();
							data->egg_drone_groups.clear();
							data->larva_groups.clear();
							data->larva_drone_groups.clear();
							data->pupa_groups.clear();
							data->pupa_drone_groups.clear();

							newWorkerLarvae = 0;
							newDroneLarvae = 0;
							newWorkerPupae = 0;
							newDronePupae = 0;

							//fractionSwarm of all healthy & infected in-hive bees join the swarm
							for (auto& group : data->in_hive_bee_groups) {
								group.healthy = (USHORT)(group.healthy * fractionSwarm);
								group.infectedAsPupa = (USHORT)(group.infectedAsPupa * fractionSwarm);
								group.infectedAsAdult = (USHORT)(group.infectedAsAdult * fractionSwarm);
								group.number = group.healthy + group.infectedAsPupa + group.infectedAsAdult;
							}

							//fractionSwarm of all healthy & infected drones bees join the swarm
							for (auto& group : data->drone_groups) {
								group.healthy = (USHORT)(group.healthy * fractionSwarm);
								group.infectedAsPupa = (USHORT)(group.infectedAsPupa * fractionSwarm);
								group.number = group.healthy + group.infectedAsPupa;
							}

							std::mt19937 gen(time());

							//(1 - fractionSwarm) foragers do not join the swarm
							for (auto iter = data->forager_squadrons.begin(), end = data->forager_squadrons.end(); iter != end;) {
								auto next = std::next(iter);
					
								if ((gen() % 100) * 0.01 < (1 - fractionSwarm)) {
									data->forager_squadrons.erase(iter);
								}

								iter = next;
							}

							mite_organisers
      					}
					}
				}
			}

to SwarmingProc

  if day >= SwarmingDate - PRE_SWARMING_PERIOD
     and day <= SwarmingDate
  [


    if Swarming = "Swarming (prime swarm)"
    [ ; Swarm PREPARATION of PRIME SWARM:
      set NewDroneEggs 0
      set NewWorkerEggs 0
      if  day = SwarmingDate
      [ ; Swarming of PRIME SWARM:
        ask (turtle-set eggCohorts larvaeCohorts droneEggCohorts droneLarvaeCohorts)
        [ ; all brod is left behind and hence removed from the smulation:
          set number 0
        ]
        ask (turtle-set pupaeCohorts dronePupaeCohorts)
        [
          set number 0
          set number_infectedAsPupa 0
          set number_healthy 0
        ]
        set NewWorkerLarvae 0
        set NewDroneLarvae 0
        set NewWorkerPupae 0
        set NewDronePupae 0
        ask IHbeeCohorts
        [ ; fractionSwarm of all healthy & infected in-hive bees join the swarm
          set number_Healthy round (number_Healthy * fractionSwarm)
          set number_infectedAsPupa round (number_infectedAsPupa * fractionSwarm)
          set number_infectedAsAdult round (number_infectedAsAdult * fractionSwarm)
          set number number_Healthy + number_infectedAsPupa + number_infectedAsAdult
        ]

        ask droneCohorts
        [ ; fractionSwarm of all healthy & infected drones join the swarm
          set number_Healthy round (number_Healthy * fractionSwarm)
          set number_infectedAsPupa round (number_infectedAsPupa * fractionSwarm)
          set number number_Healthy + number_infectedAsPupa
        ]

        ask foragerSquadrons
        [ ; (1 - fractionSwarm) foragers do not join the swarm and hence die (in the model):
          if random-float 1 < (1 - fractionSwarm) [ die ]
        ]

        ask miteOrganisers [ die ]
          ; mites in brood cells are left behind in the old colony

        ; the phoretic mite population in the swarm is reduced:
        set PhoreticMites round (PhoreticMites * fractionSwarm)
        set PollenStore_g 0
        set HoneyEnergyStore
              ((TotalForagers + TotalIHbees)
                * 36 * ENERGY_HONEY_per_g) / 1000
          ; Winston p. 185: 36mg honey per bee during swarming
        output-type "Swarming on day: "
        output-print day
        set SwarmingDate 0  ; allows production of after swarms
      ] ; if  day = SwarmingDate ..
    ] ; if Swarming = "Swarming (prime swarm)"   ,,
  ] ; if SwarmingDate > 0 and ..

  if Swarming = "Swarm (daughter colony)"
     and day > SwarmingDate
     and day <= SwarmingDate + POST_SWARMING_PERIOD   ; DAUGHTER COLONY AFTER SWARMING (0d period)
  [ ; no eggs can be laid, no food stored, as long as they have no new home..
    set NewDroneEggs 0
    set NewWorkerEggs 0
    set PollenStore_g 0
    set Aff MAX_AFF
    if HoneyEnergyStore >
      (((TotalForagers + TotalIHbees) * CROPVOLUME) / 1000)
         * 1.36 * ENERGY_HONEY_per_g
    [
      set HoneyEnergyStore (((TotalForagers + TotalIHbees) *
        CROPVOLUME) / 1000) * 1.36 * ENERGY_HONEY_per_g
    ]
  ]
  ; resetting SwarmingDate to zero at the end of a year:
  if day = 365 [ set SwarmingDate 0 ]
end

			/*
			; Egg laying & development:
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
