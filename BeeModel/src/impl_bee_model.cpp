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

			/*
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
