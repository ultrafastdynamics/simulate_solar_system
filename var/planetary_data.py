import numpy as np

# mass: kg, distance: km, a in AU
# data from https://nssdc.gsfc.nasa.gov/planetary/factsheet/
# a from: https://www.princeton.edu/~willman/planetary_systems/Sol/
# data for earth and moon can directly be used from Wikipedia (Erdbahn and Mondbahn)
data = [{"name": "mercury", "mass": 0.33e24,   "min_dist": 46e6,     "max_dist": 69.8e6,   "a": 0.3870993},
        {"name": "venus",   "mass": 4.87e24,   "min_dist": 107.5e6,  "max_dist": 108.9e6,  "a": 0.723336},
        {"name": "mars",    "mass": 0.642e24,  "min_dist": 206.7e6,  "max_dist": 249.3e6,  "a": 1.52371},
        {"name": "jupiter", "mass": 1898e24,   "min_dist": 750.6e6,  "max_dist": 816.4e6,  "a": 5.2029},
        {"name": "saturn",  "mass": 568e24,    "min_dist": 1357.6e6, "max_dist": 1506.5e6, "a": 9.537},
        {"name": "uranus",  "mass": 86.8e24,   "min_dist": 2732.7e6, "max_dist": 3001.4e6, "a": 19.189},
        {"name": "neptune", "mass": 102e24,    "min_dist": 4471.1e6, "max_dist": 4558.9e6, "a": 30.0699},
        {"name": "pluto",   "mass": 0.0130e24, "min_dist": 5906.4e6, "max_dist": 7375.9e6, "a": 39.4821},
        ]

mass_sun = 1.989e30
G = 6.6743e-11
au = 1.495978707e11

def velocity(a, r):
    # from https://de.wikipedia.org/wiki/Bahngeschwindigkeit_(Astronomie)
    return np.sqrt(G*mass_sun*(2/r-1/a))

print("name\tmin_dist\tmax_dist\tv_min\t\tv_max\t\tmass")
for i in data:
    v_min = velocity(i["a"]*au, i["min_dist"]*1e3)*1e-3
    v_max = velocity(i["a"]*au, i["max_dist"]*1e3)*1e-3
    print(i["name"]+"\t{:1.4e}".format(i["min_dist"])+"\t{:1.4e}".format(i["max_dist"])+"\t{:1.4e}".format(v_min)+"\t{:1.4e}".format(v_max)+"\t{:1.2e}".format(i["mass"]))
