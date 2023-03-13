from argparse import ArgumentParser
import os
import json


config_directory = "config/"
default_config_file = config_directory+"default.json"


class InvalidConfiguration(Exception):
    pass


class ConfigHandler:
    def __init__(self, default, config):
        self.default = default
        self.config = config

    def __getitem__(self, *keys):
        keys = keys[0]
        if isinstance(keys, tuple):
            try:
                value = self.config[keys[0]]
                for key in keys[1:]:
                    value = value[key]
            except KeyError:
                value = self.default[keys[0]]
                for key in keys[1:]:
                    value = value[key]
            return value

        else:
            try:
                return self.config[keys]
            except KeyError:
                return self.default[keys]


class GenericConfigurationParser:
    def __init__(self):
        self.parser = ArgumentParser()

        group = self.parser.add_mutually_exclusive_group(required=True)
        group.add_argument('-s', metavar="config", type=str,
                            help="Sampling config file used with synthetic graph and observations")
        group.add_argument('-g', metavar="config", type=str,
                            help="Sampling config file used with synthetic graph and observation data")
        group.add_argument('-o', metavar="config", type=str,
                            help="Sampling config file used with graph and observation data")

    def parse_args(self):
        return self.parser.parse_args()


class ConfigurationParserWithModels(GenericConfigurationParser):
    def __init__(self):
        super().__init__()
        self.parser.add_argument(type=str, dest="models",
                help="Models to process", nargs="+")


def get_dataset_name(args):
    return get_config_filename(args).split("/")[-1][:-5]


def get_config_filename(args):
    if args.s:
        config_location = os.path.join(config_directory, "synthetic", args.s)
    elif args.g:
        config_location = os.path.join(config_directory, "graph-data", args.g)
    elif args.o:
        config_location = os.path.join(config_directory, "observation-data", args.o)
    else:
        raise ValueError("Program arguments don't contain the config file name")

    return config_location


def get_config_filename_from_str(experiment_type, file_name):
    if experiment_type == "s":
        config_subdirectory_name = "synthetic"
    elif experiment_type == "g":
        config_subdirectory_name = "graph-data"
    elif experiment_type == "o":
        config_subdirectory_name = "observation-data"
    else:
        raise ValueError(f'Invalid type of data "{experiment_type}".')

    return os.path.join(config_directory, config_subdirectory_name, file_name)


def get_config(args):
    default_config = get_json(default_config_file)
    validate_default_config(default_config)

    dataset_config = get_json(get_config_filename(args))
    validate_dataset_config(dataset_config)

    return ConfigHandler(default_config, dataset_config)

def get_config_from_str(file_name, experiment_type):
    default_config = get_json(default_config_file)
    validate_default_config(default_config)

    dataset_config = get_json(get_config_filename_from_str(experiment_type, file_name))
    validate_dataset_config(dataset_config)

    return ConfigHandler(default_config, dataset_config)


def validate_default_config(config):
    pass


def validate_dataset_config(config):
    if "vertex number" not in config.keys():
        raise InvalidConfiguration("Missing entry \"vertex number\".")


def get_json(json_file):
    with open(json_file, 'r') as file_stream:
        json_object = json.load(file_stream)
    return json_object
