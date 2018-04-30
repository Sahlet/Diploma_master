//impl_bee_model.cpp

#include "bee_model.h"
#include "impl_bee_model.h"

namespace My {

	namespace BeeModel {

		model_data model_data::create_sample() {
			model_data res;
			res.day_number = 1;

			return std::move(res);
		}

		namespace Impl {

			bool verify_data(const model_data& data, const char*& info) {
				if (data.day_number > DAYS_IN_YEAR || data.day_number < 1) {
					info = "day_number";
					return false;
				}

				return true;
			}

			namespace {
				void daily_update_proc(internal_model_data& data) {
					data.day_number = (ticks + start_day - 1) % DAYS_IN_YEAR;
					data.DeathsAdultWorkers_t = 0;
					data.SumLifeSpanAdultWorkers_t = 0;
					data.DailyMiteFall = 0;
					data.Pupae_W&D_KilledByVirusToday = 0;
					data.NewReleasedMitesToday = 0;
						// all (healthy and infected) mites released from cells (mothers+offspring)
						// on current day (calculated after MiteFall!)

					for(auto& obj: forager_squadrons) {
						obj.km_today = 0;
					}

					set DailyForagingPeriod Foraging_PeriodREP
					set HoneyEnergyStoreYesterday HoneyEnergyStore
					set PollenStore_g_Yesterday PollenStore_g
					set LostBroodToday 0
					set Queenage Queenage + 1

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
			} // namespace

			void daily_step(internal_model_data& data) {
				ticks++;
				daily_update_proc(data);
			}

		} // namespace Impl

	} // namespace BeeModel

} // namespace My
