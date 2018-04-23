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
				void daily_update_proc() {
					day_number = (day_number + 1) % DAYS_IN_YEAR;
					if(day_number == 0) {
						day_number = DAYS_IN_YEAR;
					}
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

			}

		} // namespace Impl

	} // namespace BeeModel

} // namespace My
