//bee_model.cpp

#include "bee_model.h"
#include "impl_bee_model.h"

namespace My {

	namespace BeeModel {

		model::model() : model(model_data::create_sample) {}
		model::model(model_data data) {
			const char* info;
			if (!Impl::verify_data(data, info)) {
				throw std::invalid_argument(info);
			}

			idata = new internal_model_data(std::move(data));
		}
		model::~model() {
			delete (internal_model_data*)idata;
		}

		const model_data& model::get_data() {
			return idata->data;
		}

		model::daily_step() {

		}

	} // namespace BeeModel

} // namespace My