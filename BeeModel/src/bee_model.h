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
			USHORT age = 0;
			int ploidy = 0;
			USHORT number = 0;
			USHORT numberDied = 0;
			USHORT invadedByMiteOrganiserID = 0;
		};

		struct egg_group : entity {};

		struct egg_drone_group : entity {};

		struct larva_group : entity {};

		struct larva_drone_group : entity {};

		struct base_bee_group : entity {
			USHORT infectedAsPupa = 0;
			USHORT healthy = 0;
		};

		struct pupa_group : base_bee_group {};

		struct pupa_drone_group : base_bee_group {};

		struct drone_group : base_bee_group {};

		struct in_hive_bee_group : base_bee_group {
			USHORT infectedAsAdult = 0;
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
				//function has to be overwrited
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

		meke_enum(swarming_type,
			NO_SWARMING,
			SWARM_CONTROL, // by beekeeper
			PARENTAL_COLONY,
			DOUGHTER_COLONY, 
			PRIME_SWARM
		);

		struct model_data {
			//Model settings
			UINT CRITICAL_COLONY_SIZE_WINTER = 4000; // Martin (2001): 4000 adult workers during winter (from Free & Spencer-Booth 1958, Harbo 1983)
			float MORTALITY_EGGS = 0.03; // daily mortality rate of worker eggs (from Schmickl & Crailsheim (2007))
			UINT HATCHING_AGE = 3; // (days) age at which worker larvae hatch from eggs (from Winston (1987) p.50)
			float MORTALITY_DRONE_EGGS = 0.064; // daily mortality rate of drone eggs (from Fukuda, Ohtani (1977))
			UINT DRONE_HATCHING_AGE = 3; // (days) age at which drone larvae hatch from eggs (from Jay (1963), Hrassnig & Crailsheim (2005))
			UINT DRONE_PUPATION_AGE = 10; // (days) age of pupation for drone (from Winston (1987))
			UINT DRONE_EMERGING_AGE = 24; // age at which adult drones emerge from pupae (from Winston (1987))
			UINT MAX_EGG_LAYING = 1600; // maximum egg laying rate per day
			UINT EMERGING_AGE = 21; // age at which adult workers emerge from pupae (Winston (1987) p.50)
			float FORAGER_NURSING_CONTRIBUTION = 0.2; // contribution of foragers on brood care
			float MAX_BROOD_NURSE_RATIO = 3; // maximum amount of brood, nurse bees can care for (from Free & Racey (1968): 3; Becher et al. (2010): 2.65)
			bool EGG_LAYING_IH = true; // if true: egg laying is affected by available nurse bees
			UINT MAX_BROODCELLS = 2000099; // maximum brood space
			float DRONE_EGGS_PROPORTION = 0.04; // proportion of drone eggs (from Wilkinson & Smith (2002))
			UINT SEASON_START = 1; //edit // defines beginning of foraging period
			UINT SEASON_STOP = DAYS_IN_YEAR; //edit // end of foraging period & latest end of drone production
			UINT PRE_SWARMING_PERIOD = 3; // defines period during which colony prepares for swarming (from Schmickl & Crailsheim (2007): 3 days, Winston (1987) p. 184: "until the week before swarming")
			float ENERGY_HONEY_per_g = 12.78; // [kJ/g] energy content of 1g honey (from USDA: 304kcal/100g (http://www.nal.usda.gov/fnic/foodcomp/search/))

			//Process settings
			bool ClearOnDead = false; // if true: some data will delete
			bool QueenAgeing = false; // if true: egg laying rate decreases with queen age (following BEEPOP) and the queen is replaced every year
			swarming_type Swarming = swarming_type::NO_SWARMING;

			//Model data
			std::list<egg_group> egg_groups;
			std::list<egg_drone_group> egg_drone_groups;
			std::list<larva_group> larva_groups;
			std::list<larva_drone_group> larva_drone_groups;
			std::list<pupa_group> pupa_groups;
			std::list<pupa_drone_group> pupa_drone_groups;
			std::list<drone_group> drone_groups;
			std::list<in_hive_bee_group> in_hive_bee_groups;
			std::list<forager_squadron> forager_squadrons;
			std::list<flower_patch> flower_patchs;
			std::list<mite_organiser> mite_organisers;

			date_struct date; // all other data in model_data object setted for the end of this date (for the evening)

			int Queenage = 1; // in days (min value is 1)
			UINT SwarmingDay = 0; // day when swarming takes place; reset to 0 at the end of a year
			double honeyEnergyStore = 0; // in [kJ] (kilojoules) (honey store of the colony (recorded in energy not in weight))
			UINT PhoreticMites = 0;	 // all phoretic mites, healthy and infected
			
			prop<bool> colonyDied;
			std::string deathReason;
			
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
