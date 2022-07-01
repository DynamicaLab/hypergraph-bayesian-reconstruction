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


rcParams["font.family"] = "CMU"
rcParams["font.size"] = 16
rcParams["text.usetex"] = True

rcParams["axes.titlesize"] = 16
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
rcParams["legend.fontsize"] = 12
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
            "pes": "^",
            "per": "s"
        }


edgetype_colors = [midblack, green, yellow]
edgetype_markers = ["x", ".", "^"]
edgetype_ls = ["--", "-", ":"]

get_figure_dir = lambda dataset: os.path.join(os.getcwd(), "figures", dataset)

inches_per_pt = 1/72.27
fig_width = 2*inches_per_pt*246 # width of article column
fig_height = fig_width/3
