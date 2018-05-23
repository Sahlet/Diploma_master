//impl_bee_model.h

#ifndef __IMPLBEEMODEL_H__
#define __IMPLBEEMODEL_H__

#include "bee_model.h"

namespace My {

	namespace BeeModel {

		namespace Impl {

			struct model_impl : public prop_handler {
				model_impl(std::shared_ptr<model_data> data);

				inline const model_data& get_data() {
					return *data;
				}

				void daily_step();

			private:
				void daily_update_proc();
				void season_HoPoMo_proc(); // calculates HoPoMo_seasont factor that influences egg laying rate.
				void worker_eggs_dev_proc(); // worker eggs ageing, deletion of oldest groups

				void clear_on_dead();
				static float season_HoPoMo(USHORT day, USHORT parameterList[5] = nullptr);

			private:
				std::shared_ptr<model_data> data;

				unsigned int ticks = 0;
				
				int ABANDON_POLLEN_PATCH_PROB_PER_S = 0;
				int AFF = 0;
				int AFF_BASE = 0;
				int AllDaysAllPatchesList = 0;
				int ColonyTripDurationSum = 0;
				int ColonyTripForagersSum = 0;
				int CROPVOLUME = 0;
				int CumulativeHoneyConsumption = 0;
				int foragingPeriodForToday = 0; // seconds per (current) day, bees are allowed to forage
				int DailyHoneyConsumption = 0;
				int DailyMiteFall = 0;
				int DailyPollenConsumption_g = 0;
				int DeathsAdultWorkers_t = 0;
				int DecentHoneyEnergyStore = 0;
				int DRONE_EGGLAYING_START = 0;
				int DRONE_EGGLAYING_STOP = 0;
				int DRONE_EMERGING_AGE = 0;
				int DRONE_HATCHING_AGE = 0;
				int DRONE_LIFESPAN = 0;
				int DRONE_PUPATION_AGE = 0;
				int DRONE_EGGS_PROPORTION = 0;
				int EMERGING_AGE = 0;
				int ENERGY_HONEY_per_g = 0;
				int ENERGY_SUCROSE = 0;
				int ExcessBrood = 0;
				int FIND_DANCED_PATCH_PROB = 0;
				int FLIGHT_VELOCITY = 0;
				int FLIGHTCOSTS_PER_m = 0;
				int FORAGER_NURSING_CONTRIBUTION = 0;
				int FORAGING_STOP_PROB = 0;
				int ForagingRounds = 0;
				int ForagingSpontaneousProb = 0;
				int HarvestedHoney_kg = 0;
				int HATCHING_AGE = 0;
				int HONEY_STORE_INIT = 0;
				double honeyEnergyStore = 0; // in [kJ] (kilojoules) (honey store of the colony (recorded in energy not in weight))
				int HoPoMo_seasont = 0;
				int IdealPollenStore_g = 0;
				int InhivebeesDiedToday = 0;
				int INVADING_DRONE_CELLS_AGE = 0;
				int INVADING_WORKER_CELLS_AGE = 0;
				int InvadingMitesDroneCellsReal = 0;
					// actual number of mites invading the cells, might be
					// lower than theor. number, if brood cells are crowded with mites
				int InvadingMitesDroneCellsTheo = 0;
				int 	// theoretical number of mites invading the cells
				int InvadingMitesWorkerCellsReal = 0;
				int InvadingMitesWorkerCellsTheo = 0;
				int LIFESPAN = 0;
				int LostBroodTotal = 0;	// brood that die due to lack of nursing or lack of pollen
				int MAX_AFF = 0;
				int MAX_BROOD_NURSE_RATIO = 0;
				int MAX_DANCE_CIRCUITS = 0;
				int MAX_EGG_LAYING = 0;
				int MAX_HONEY_ENERGY_STORE = 0;
				int MAX_INVADED_MITES_DRONECELL = 0;
				int MAX_INVADED_MITES_WORKERCELL = 0;
				int MAX_PROPORTION_POLLEN_FORAGERS = 0;
				int MAX_TOTAL_KM = 0;
				int MIN_AFF = 0;
				int MIN_IDEAL_POLLEN_STORE = 0;
				int MITE_FALL_DRONECELL = 0;
				int MITE_FALL_WORKERCELL = 0;
				int MITE_MORTALITY_BROODPERIOD = 0;
				int MITE_MORTALITY_WINTER = 0;
				int MORTALITY_DRONE_EGGS = 0;
				int MORTALITY_DRONE_LARVAE = 0;
				int MORTALITY_DRONE_PUPAE = 0;
				int MORTALITY_DRONES = 0;
				int MORTALITY_DRONES_INFECTED_AS_PUPAE = 0;
				int MORTALITY_EGGS = 0;
				int MORTALITY_FOR_PER_SEC = 0;
				int MORTALITY_INHIVE = 0;
				int MORTALITY_INHIVE_INFECTED_AS_ADULT = 0;
				int MORTALITY_INHIVE_INFECTED_AS_PUPA = 0;
				int MORTALITY_LARVAE = 0;
				int MORTALITY_PUPAE = 0;
				int N_FLOWERPATCHES = 0;
				int N_GENERIC_PLOTS = 0;
				int NewDroneEggs = 0;
				int NewDroneLarvae = 0;
				int NewDronePupae = 0;
				int NewDrones = 0;
				int NewDrones_healthy = 0;
				int NewForagerSquadronsHealthy = 0;
				int NewForagerSquadronsInfectedAsAdults = 0;
				int NewForagerSquadronsInfectedAsPupae = 0;
				int NewIHbees = 0;
				int NewIHbees_healthy = 0;
				int NewReleasedMitesToday = 0;
					// all (healthy and infected) mites released from cells (mothers+offspring)
					// on current day (calculated after MiteFall!)
				int NewWorkerEggs = 0;
				int NewWorkerLarvae = 0;
				int NewWorkerPupae = 0;
				int PATCHCOLOR = 0;
				int PhoreticMites = 0;	 // all phoretic mites, healthy and infected
				int PhoreticMitesHealthyRate = 0;
				int POLLEN_DANCE_FOLLOWERS = 0;
				int POLLEN_STORE_INIT = 0;
				int POLLENLOAD = 0;
				int PollenStore_g = 0; // in [g] (amount of pollen stored in the colony)
				int POST_SWARMING_PERIOD = 0;
				int PRE_SWARMING_PERIOD = 0;
				int ProbPollenCollection = 0;
				int PropNewToAllPhorMites = 0;
				int PROTEIN_STORE_NURSES_d = 0;
				int ProteinFactorNurses = 0;
				int Pupae_W&D_KilledByVirusToday = 0; // number of drone + worker pupae that were killed by the virus today
				int PUPATION_AGE = 0;
				int SaveInvadedMODroneLarvaeToPupae = 0;
				int SaveInvadedMOWorkerLarvaeToPupae = 0;
				int SaveWhoDroneLarvaeToPupae = 0;
				int SaveWhoWorkerLarvaeToPupae = 0;
				int SEARCH_LENGTH_M = 0;
				int SEASON_START = 0; // defines beginning of foraging period
				int SEASON_STOP = 0; // end of foraging period & latest end of drone production
				int SimpleDancing = 0;
				int STEPWIDTH = 0;
				int STEPWIDTHdrones = 0;
				int SumLifeSpanAdultWorkers_t = 0;
				int SummedForagerSquadronsOverTime = 0;
				int SwarmingDate = 0;
				int TIME_UNLOADING = 0;
				int TIME_UNLOADING_POLLEN = 0;
				int TotalBeesAdded = 0;
					// beekeeper can add bees in autumn, these are added up as long
					// as simulation runs
				int TotalDroneEggs = 0;
				int TotalDroneLarvae = 0;
				int TotalDronePupae = 0;
				int TotalDrones = 0;
				int TotalEggs = 0;
				int TotalForagers = 0;
				int TotalFPdetectionProb = 0;
				int TotalHoneyFed_kg = 0;
					// if "beekeeper" has to feed the colony, fed honey is added up as long
					// as simulation runs
				int TotalHoneyHarvested_kg = 0;
				int TotalIHbees = 0;
				int TotalLarvae = 0;
				int TotalMites = 0;
				int TotalPollenAdded = 0;
					// beekeeper can add pollen in spring, which is added up as long
					// as simulation runs
				int TotalPupae = 0;
				int TotalWeightBees_kg = 0; // weight of all bees (brood, adults, drones..)
				int TotalWorkerAndDroneBrood = 0;
				int VIRUS_KILLS_PUPA_PROB = 0;
				int VIRUS_TRANSMISSION_RATE_MITE_TO_PUPA = 0;
					// probability for an infected invaded mite to infect the bee pupa
				int VIRUS_TRANSMISSION_RATE_PUPA_TO_MITES = 0;
					// probability for an infected bee pupa to infect healthy invaded mites
				int WEIGHT_WORKER_g = 0;
			};

			bool verify_data(const model_data& data, const char*& info);

		} // namespace Impl

	} // namespace BeeModel

} // namespace My

#endif // __IMPLBEEMODEL_H__