//bee_model.cpp

#include "bee_model.h"
#include "impl_bee_model.h"

namespace My {

	namespace BeeModel {

		struct internal_model_data {
			Impl::model_impl model;

			internal_model_data(std::shared_ptr<model_data> data) : model(std::move(data)) {}
		};

		model::model() : model(std::make_shared<model_data>(model_data::create_sample())) {}
		model::model(std::shared_ptr<model_data> data) {
			const char* info;
			if (!Impl::verify_data(*data, info)) {
				throw std::invalid_argument(info);
			}

			idata.reset(new internal_model_data(std::move(data)));
		}

		const model_data& model::get_data() {
			return idata->model.get_data();
		}

		model::daily_step() {
			idata->daily_step();
		}

	} // namespace BeeModel

} // namespace My