//bee_model.h

#ifndef __BEEMODEL_H__
#define __BEEMODEL_H__

#include <string>

#define DAYS_IN_YEAR (365)

namespace My {

	namespace BeeModel {

		struct entity {
			int64_t id = 0;
			int age = 0;
			int ploidy = 0;
			int number = 0;
			int numberDied = 0;
			int invadedByMiteOrganiserID = 0;
		};

		struct base_bee_group : entity {
			int infectedAsPupa = 0;
			int healthy = 0;
		};

		struct pupa_group : base_bee_group {};

		struct pupa_drone_group : base_bee_group {};

		struct drone_group : base_bee_group {};

		struct in_hive_bee_group : base_bee_group {
			int infectedAsAdult = 0;
		};

		struct forager_squadron : entity { //bee group, which searching food
			int activity = 0;
			int activityList = 0;
			int knownNectarPatch = 0;
			int knownPollenPatch = 0;
			int pollenForager = 0;
			int cropEnergyLoad = 0;
			int collectedPollen = 0;
			int mileometer = 0;
			int km_today = 0;
			int infectionState = 0;
		};

		struct flower_patch : entity {
			int patchType = 0;
			int distanceToColony = 0;
			int xcorMap = 0;
			int ycorMap = 0;
			int oldPatchID = 0;
			int size_sqm = 0;
			int quantityMyl = 0;
			int amountPollen_g = 0;
			int nectarConcFlowerPatch = 0;
			int detectionProbability = 0;
			int flightCostsNectar = 0;
			int flightCostsPollen = 0;
			int EEF = 0;
			int danceCircuits = 0;
			int danceFollowersNectar = 0;
			int summedVisitors = 0;
			int nectarVisitsToday = 0;
			int pollenVisitsToday = 0;
			int tripDuration = 0;
			int tripDurationPollen = 0;
			int mortalityRisk = 0;
			int mortalityRiskPollen = 0;
			int handlingTimeNectar = 0;
			int handlingTimePollen = 0;
		};
		
		struct mite_organiser : entity {
			int workerCellListCondensed = 0;
			int droneCellListCondensed = 0;
			int cohortInvadedMitesSum = 0;
			int invadedMitesHealthyRate = 0;
			int invadedDroneCohortID = 0;
			int invadedWorkerCohortID = 0;
		};

		struct model_data {
			unsigned short day_number = 1; //from 1 to DAYS_IN_YEAR

			model_data(const model_data&) = default;
			model_data(model_data&&) = default;
			model_data& operator=(const model_data&) = default;
			model_data& operator=(model_data&&) = default;

			model_data create_sample();
		};

		struct internal_model_data;

		struct model {
			//sets sample model_data
			model();
			model(model_data data);
			~model();

			const model_data& get_data();

			daily_step();
		private:
			internal_model_data* idata;
		};

	} // namespace BeeModel

} // namespace My

#endif // __BEEMODEL_H__
