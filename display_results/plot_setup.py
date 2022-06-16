from matplotlib import rcParams
import os


darkblack = "#1a1a1a"
midblack ="#3d3d3d"
lightgray = "#ababab"

blue = "#386BA8"
darkblue = "#2A5180"
orange = "#FF9854"
darkorange = "#B0693A"
red = "#A4303F"
green = "#4E994B"
yellow = "#F6AE2D"


rcParams["font.family"] = "Computer Modern"
rcParams["font.size"] = 16
rcParams["text.usetex"] = True

rcParams["axes.labelsize"] = 16
rcParams["axes.facecolor"] = "white"
rcParams["axes.grid"] = False
rcParams["axes.edgecolor"] = lightgray
rcParams['axes.spines.right'] = False
rcParams['axes.spines.top'] = False

rcParams["xtick.labelsize"] = 12
rcParams["ytick.labelsize"] = 12
rcParams["xtick.color"] = midblack
rcParams["ytick.color"] = midblack

rcParams['legend.frameon'] = 'False'
rcParams["legend.edgecolor"] = "white"
rcParams["legend.fontsize"] = 14
rcParams["text.color"] = midblack


model_colors = {
            "phg": blue,
            "pes": orange,
            "per": green
        }
dark_model_colors = {
            "phg": darkblue,
            "pes": darkorange,
            "per": green
        }
model_markers = {
            "phg": "o",
            "pes": "s",
            "per": "^"
        }


edgetype_colors = [midblack, green, yellow]
edgetype_markers = ["x", ".", "^"]
edgetype_ls = ["--", "-", ":"]

get_figure_dir = lambda dataset: os.path.join(os.getcwd(), "figures", dataset)
