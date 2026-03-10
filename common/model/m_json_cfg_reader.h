#ifndef CLT_OPTIC_M_JSON_CFG_READER_H
#define CLT_OPTIC_M_JSON_CFG_READER_H

#include <stdexcept>
#include <optional>
#include <string>
#include <jansson.h>

class jansson_cfg_obj_reader {
	const json_t *json_obj_;
	std::string obj_name_;
public:
	jansson_cfg_obj_reader(const json_t *json_up, const char *obj_name) : json_obj_{}, obj_name_{obj_name} {
		json_obj_ = json_object_get(json_up, obj_name);
		if (!json_obj_)
			throw std::logic_error(std::string("error on config file, there is no '") + obj_name_ + "' node");
	}

	explicit jansson_cfg_obj_reader(const json_t *json_obj):json_obj_{json_obj}, obj_name_{"???"}{}

	int read_int_param(const char *par_name, std::optional<int> def = {}) {
		json_t *json_par = json_object_get(json_obj_, par_name);
		if(!json_par){
			if(def.has_value())
				return def.value();
			else
				throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
									   "' is absent");
		}
		if (!json_is_integer(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is invalid (int)");
		int res = static_cast<int>(json_integer_value(json_par));
		return res;
	}

	float read_real_param(const char *par_name) {
		json_t *json_par = json_object_get(json_obj_, par_name);
		if (!json_is_real(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is absent or invalid (real)");
		float res = static_cast<float>(json_real_value(json_par));
		return res;
	}

	double read_double_param(const char *par_name, std::optional<double> def = {}) {
		json_t *json_par = json_object_get(json_obj_, par_name);
		if(!json_par){
			if(def.has_value())
				return def.value();
			else
				throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
									   "' is absent");
		}
		if (!json_is_number(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is invalid (not a number)");
		double res = json_number_value(json_par);
		return res;
	}

	std::string read_string_param(const char *par_name, std::optional<std::string> def = {}) {
		json_t *json_par = json_object_get(json_obj_, par_name);
		if(!json_par){
			if(def.has_value())
				return def.value();
			else
				throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
									   "' is absent");
		}
		if (!json_is_string(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is invalid (std::string)");
		std::string res{json_string_value(json_par)};
		return res;
	}

	bool read_bool_param(const char *par_name, std::optional<bool> def = {}) {
		json_t *json_par = json_object_get(json_obj_, par_name);
		if(!json_par){
			if(def.has_value())
				return def.value();
			else
				throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
									   "' is absent");
		}
		if (!json_is_boolean(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is invalid (bool)");
		bool res = json_boolean_value(json_par);
		return res;
	}

	const json_t * root(){return json_obj_;}

	const json_t * read_object(const char *par_name){
		json_t *json_par = json_object_get(json_obj_, par_name);
		if (!json_is_object(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is absent or invalid (object)");
		return json_par;
	}
	const json_t * read_array(const char *par_name){
		json_t *json_par = json_object_get(json_obj_, par_name);
		if (!json_is_array(json_par))
			throw std::logic_error(std::string("error on config file, '") + obj_name_ + "' -> '" + par_name +
								   "' is absent or invalid (array)");
		return json_par;
	}
};

#endif //CLT_OPTIC_M_JSON_CFG_READER_H
