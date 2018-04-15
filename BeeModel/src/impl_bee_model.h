//impl_bee_model.h

#ifndef __IMPLBEEMODEL_H__
#define __IMPLBEEMODEL_H__

#include "bee_model.h"

namespace My {

	namespace BeeModel {

		struct internal_model_data {
			model_data data;
			internal_model_data(model_data data) : data(std::move(data)) {}
		};

		namespace Impl {

			bool verify_data(const model_data& data, const char*& info);

			void daily_step(internal_model_data& data);

		} // namespace Impl

	} // namespace BeeModel

} // namespace My

#endif // __IMPLBEEMODEL_H__