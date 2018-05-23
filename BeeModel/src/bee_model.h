//bee_model.h

#ifndef __BEEMODEL_H__
#define __BEEMODEL_H__

#include <string>
#include <list>
#include <memory>

#include <My/Properties.h>

#define DAYS_IN_YEAR (365)
typedef unsigned int UINT;
typedef unsigned short USHORT;

namespace My {

	namespace BeeModel {

		struct date_struct {
			USHORT year = 2000;
			USHORT day = 1; //from 1 to DAYS_IN_YEAR //Day

			void inc();
		};

		struct entity {
			int64_t id = 0;
			int age = 0;
			int ploidy = 0;
			int number = 0;
			int numberDied = 0;
			int invadedByMiteOrganiserID = 0;
		};

		struct egg_group : entity {};

		struct egg_drone_group : entity {};

		struct larva_group : entity {};

		struct larva_drone_group : entity {};

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
			struct daily_data {
				UINT quantityMyl = 0; // [μl] microliters; quantity of available nectar on the specified day
				UINT amountPollen_g = 0; // [g] grams; quantity of available pollen on the specified day
				UINT nectarVisitsToday = 0; // number of bees collected nectar from that patch today
				UINT pollenVisitsToday = 0; // number of bees collected pollen from that patch today

				//Updates daily_data on the specified date
				virtual void update(const date_struct& date);
			};

			int patchType = 0;
			int distanceToColony = 0;
			int oldPatchID = 0;
			int size_sqm = 0;
			int nectarConcFlowerPatch = 0;
			int detectionProbability = 0;
			int flightCostsNectar = 0;
			int flightCostsPollen = 0;
			int EEF = 0;
			int danceCircuits = 0;
			int danceFollowersNectar = 0;
			int summedVisitors = 0;
			int tripDuration = 0;
			int tripDurationPollen = 0;
			int mortalityRisk = 0;
			int mortalityRiskPollen = 0;
			int handlingTimeNectar = 0;
			int handlingTimePollen = 0;

			std::shared_ptr<daily_data> dailyData;
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
			UINT CRITICAL_COLONY_SIZE_WINTER = 4000; // Martin (2001): 4000 adult workers during winter (from Free & Spencer-Booth 1958, Harbo 1983)
			float MORTALITY_EGGS = 0.03; // daily mortality rate of worker eggs

			std::list<egg_group> egg_groups;
			std::list<egg_drone_group> egg_drone_groups;
			std::list<larva_group> larva_groups;
			std::list<larva_drone_group> larva_drone_groups;
			std::list<base_bee_group> base_bee_groups;
			std::list<pupa_group> pupa_groups;
			std::list<pupa_drone_group> pupa_drone_groups;
			std::list<drone_group> drone_groups;
			std::list<in_hive_bee_group> in_hive_bee_groups;
			std::list<forager_squadron> forager_squadrons;
			std::list<flower_patch> flower_patchs;

			date_struct date; // all other data in model_data object setted for the end of this date (for the evening)

			unsigned Queenage = 1; // in days (min value is 1)
			
			prop<bool> colonyDied;
			std::string deathReason;
			bool clearOnDead = false;

			UINT TotalForagers = 0; // number of all foragers of the colony
			UINT TotalWorkerAndDroneBrood = 0; // number of all worker and drone eggs, larvae and pupae
			UINT TotalIHbees = 0; // number of in-hive bees present in the colony

			model_data(const model_data&) = default;
			model_data(model_data&&) = default;
			model_data& operator=(const model_data&) = default;
			model_data& operator=(model_data&&) = default;

			static model_data create_sample();

			//Gets Foraging Period For Today (using date field) in seconds
			virtual int getForagingPeriodForToday();

			virtual int FlowerPatchesMaxFoodAvailableTodayREP
		};

		struct internal_model_data;

		struct model {
			//sets sample model_data
			model();
			model(std::shared_ptr<model_data> data);

			const model_data& get_data();

			daily_step();
		private:
			std::unique_ptr<internal_model_data> idata;
		};

	} // namespace BeeModel

} // namespace My

#endif // __BEEMODEL_H__
